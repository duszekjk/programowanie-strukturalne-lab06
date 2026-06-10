#include "CommandReceiverComponent.h"

#include "Containers/StringConv.h"
#include "Engine/Engine.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/Character.h"
#include "HAL/PlatformProcess.h"
#include "HAL/Runnable.h"
#include "HAL/RunnableThread.h"
#include "Misc/DateTime.h"
#include "Misc/Paths.h"

#if PLATFORM_LINUX
#include <cerrno>
#include <fcntl.h>
#include <sys/stat.h>
#include <unistd.h>
#endif

class FFifoCommandWorker final : public FRunnable
{
public:
    FFifoCommandWorker(
        const FString& InPipePath,
        TQueue<FQueuedCommand, EQueueMode::Mpsc>* InPendingCommands,
        std::atomic<bool>* InShouldStop)
        : PipePath(InPipePath)
        , PendingCommands(InPendingCommands)
        , bShouldStop(InShouldStop)
    {
    }

    virtual uint32 Run() override
    {
#if PLATFORM_LINUX
        EnsurePipeExists();

        while (!bShouldStop->load(std::memory_order_acquire))
        {
            if (PipeFd < 0 && !OpenPipe())
            {
                FPlatformProcess::Sleep(0.05f);
                continue;
            }

            char Buffer[256];
            const ssize_t BytesRead = read(PipeFd, Buffer, sizeof(Buffer));

            if (BytesRead > 0)
            {
                FUTF8ToTCHAR Converter(Buffer, static_cast<int32>(BytesRead));
                PendingText.AppendChars(Converter.Get(), Converter.Length());
                DrainPendingLines(false);
                continue;
            }

            if (BytesRead == 0)
            {
                DrainPendingLines(true);
                ClosePipe();
                FPlatformProcess::Sleep(0.02f);
                continue;
            }

            if (errno == EAGAIN || errno == EWOULDBLOCK)
            {
                FPlatformProcess::Sleep(0.01f);
                continue;
            }

            ClosePipe();
            FPlatformProcess::Sleep(0.05f);
        }

        DrainPendingLines(true);
        ClosePipe();
#else
        while (!bShouldStop->load(std::memory_order_acquire))
        {
            FPlatformProcess::Sleep(0.1f);
        }
#endif
        return 0;
    }

    virtual void Stop() override
    {
        bShouldStop->store(true, std::memory_order_release);
    }

private:
#if PLATFORM_LINUX
    bool EnsurePipeExists() const
    {
        FTCHARToUTF8 PipePathUtf8(*PipePath);
        if (access(PipePathUtf8.Get(), F_OK) == 0)
        {
            return true;
        }

        if (mkfifo(PipePathUtf8.Get(), 0666) == 0)
        {
            return true;
        }

        if (errno == EEXIST)
        {
            return true;
        }

        return false;
    }

    bool OpenPipe()
    {
        FTCHARToUTF8 PipePathUtf8(*PipePath);
        PipeFd = open(PipePathUtf8.Get(), O_RDONLY | O_NONBLOCK);
        return PipeFd >= 0;
    }

    void ClosePipe()
    {
        if (PipeFd >= 0)
        {
            close(PipeFd);
            PipeFd = -1;
        }
    }

    void DrainPendingLines(bool bFlushPartial)
    {
        int32 NewlineIndex = INDEX_NONE;
        while (PendingText.FindChar(TEXT('\n'), NewlineIndex))
        {
            const FString RawLine = PendingText.Left(NewlineIndex).TrimStartAndEnd();
            PendingText.RightChopInline(NewlineIndex + 1, false);
            EnqueueLine(RawLine);
            NewlineIndex = INDEX_NONE;
        }

        if (bFlushPartial)
        {
            const FString Tail = PendingText.TrimStartAndEnd();
            PendingText.Empty();
            if (!Tail.IsEmpty())
            {
                EnqueueLine(Tail);
            }
        }
    }

    void EnqueueLine(const FString& RawLine)
    {
        if (RawLine.IsEmpty() || PendingCommands == nullptr)
        {
            return;
        }

        FQueuedCommand Command;
        Command.RawText = RawLine;
        Command.ReceivedAtSeconds = FPlatformTime::Seconds();
        Command.Type = UCommandReceiverComponent::ParseCommandType(RawLine);
        PendingCommands->Enqueue(MoveTemp(Command));
    }
#endif

    FString PipePath;
    TQueue<FQueuedCommand, EQueueMode::Mpsc>* PendingCommands = nullptr;
    std::atomic<bool>* bShouldStop = nullptr;
    int32 PipeFd = -1;
    FString PendingText;
};

UCommandReceiverComponent::UCommandReceiverComponent()
    : bShouldStop(false)
    , Worker(nullptr)
    , WorkerThread(nullptr)
    , ReceivedCommandCount(0)
    , InvalidCommandCount(0)
    , RemainingForwardTime(0.0f)
    , RemainingBackwardTime(0.0f)
{
    PrimaryComponentTick.bCanEverTick = true;
}

void UCommandReceiverComponent::BeginPlay()
{
    Super::BeginPlay();

    CharacterOwner = Cast<ACharacter>(GetOwner());
    if (CharacterOwner.IsValid())
    {
        InitialTransform = CharacterOwner->GetActorTransform();
    }

    EnsurePipeExists();
    StartWorker();
}

void UCommandReceiverComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    StopWorker();
    Super::EndPlay(EndPlayReason);
}

void UCommandReceiverComponent::TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction)
{
    Super::TickComponent(DeltaTime, TickType, ThisTickFunction);

    FQueuedCommand Command;
    while (PendingCommands.Dequeue(Command))
    {
        ExecuteCommand(Command);
    }

    if (!CharacterOwner.IsValid())
    {
        return;
    }

    if (RemainingForwardTime > 0.0f)
    {
        CharacterOwner->AddMovementInput(CharacterOwner->GetActorForwardVector(), MoveStrength);
        RemainingForwardTime = FMath::Max(0.0f, RemainingForwardTime - DeltaTime);
    }

    if (RemainingBackwardTime > 0.0f)
    {
        CharacterOwner->AddMovementInput(-CharacterOwner->GetActorForwardVector(), MoveStrength);
        RemainingBackwardTime = FMath::Max(0.0f, RemainingBackwardTime - DeltaTime);
    }

    UpdateDebugOverlay();
}

void UCommandReceiverComponent::StartWorker()
{
#if PLATFORM_LINUX
    if (WorkerThread != nullptr)
    {
        return;
    }

    bShouldStop.store(false, std::memory_order_release);
    Worker = new FFifoCommandWorker(PipePath, &PendingCommands, &bShouldStop);
    WorkerThread = FRunnableThread::Create(Worker, TEXT("IpcCharacterWorld.FifoReader"));

    if (WorkerThread == nullptr)
    {
        delete Worker;
        Worker = nullptr;
        bShouldStop.store(true, std::memory_order_release);
    }
#endif
}

void UCommandReceiverComponent::StopWorker()
{
    bShouldStop.store(true, std::memory_order_release);

    if (WorkerThread != nullptr)
    {
        WorkerThread->WaitForCompletion();
        delete WorkerThread;
        WorkerThread = nullptr;
    }

    if (Worker != nullptr)
    {
        delete Worker;
        Worker = nullptr;
    }
}

void UCommandReceiverComponent::EnsurePipeExists() const
{
#if PLATFORM_LINUX
    FTCHARToUTF8 PipePathUtf8(*PipePath);
    if (access(PipePathUtf8.Get(), F_OK) != 0)
    {
        if (mkfifo(PipePathUtf8.Get(), 0666) != 0 && errno != EEXIST)
        {
            UE_LOG(LogTemp, Warning, TEXT("IPC: failed to create FIFO at %s (errno=%d)"), *PipePath, errno);
        }
    }
#endif
}

ECommandType UCommandReceiverComponent::ParseCommandType(const FString& RawText)
{
    const FString Normalized = RawText.TrimStartAndEnd().ToUpper();
    if (Normalized == TEXT("FORWARD"))
    {
        return ECommandType::Forward;
    }
    if (Normalized == TEXT("BACKWARD"))
    {
        return ECommandType::Backward;
    }
    if (Normalized == TEXT("LEFT"))
    {
        return ECommandType::Left;
    }
    if (Normalized == TEXT("RIGHT"))
    {
        return ECommandType::Right;
    }
    if (Normalized == TEXT("JUMP"))
    {
        return ECommandType::Jump;
    }
    if (Normalized == TEXT("STOP"))
    {
        return ECommandType::Stop;
    }
    if (Normalized == TEXT("RESET"))
    {
        return ECommandType::Reset;
    }
    if (Normalized == TEXT("PING"))
    {
        return ECommandType::Ping;
    }
    return ECommandType::Unknown;
}

void UCommandReceiverComponent::ExecuteCommand(const FQueuedCommand& Command)
{
    if (!CharacterOwner.IsValid())
    {
        return;
    }

    ++ReceivedCommandCount;
    LastRawCommand = Command.RawText;

    if (bLogCommands)
    {
        UE_LOG(LogTemp, Display, TEXT("IPC command: %s"), *Command.RawText);
    }

    switch (Command.Type)
    {
    case ECommandType::Forward:
        RemainingForwardTime += MoveImpulseSeconds;
        break;
    case ECommandType::Backward:
        RemainingBackwardTime += MoveImpulseSeconds;
        break;
    case ECommandType::Left:
        CharacterOwner->AddActorWorldRotation(FRotator(0.0f, -TurnDegrees, 0.0f));
        break;
    case ECommandType::Right:
        CharacterOwner->AddActorWorldRotation(FRotator(0.0f, TurnDegrees, 0.0f));
        break;
    case ECommandType::Jump:
        CharacterOwner->Jump();
        break;
    case ECommandType::Stop:
        RemainingForwardTime = 0.0f;
        RemainingBackwardTime = 0.0f;
        if (UCharacterMovementComponent* Movement = CharacterOwner->GetCharacterMovement())
        {
            Movement->StopMovementImmediately();
        }
        break;
    case ECommandType::Reset:
        ResetCharacter();
        break;
    case ECommandType::Ping:
        UE_LOG(LogTemp, Display, TEXT("IPC PING received"));
        break;
    case ECommandType::Unknown:
    default:
        ++InvalidCommandCount;
        UE_LOG(LogTemp, Warning, TEXT("IPC: ignored invalid command '%s'"), *Command.RawText);
        break;
    }
}

void UCommandReceiverComponent::ResetCharacter()
{
    if (!CharacterOwner.IsValid())
    {
        return;
    }

    CharacterOwner->TeleportTo(InitialTransform.GetLocation(), InitialTransform.Rotator());
    if (UCharacterMovementComponent* Movement = CharacterOwner->GetCharacterMovement())
    {
        Movement->StopMovementImmediately();
    }

    RemainingForwardTime = 0.0f;
    RemainingBackwardTime = 0.0f;
}

void UCommandReceiverComponent::UpdateDebugOverlay()
{
    if (!GEngine)
    {
        return;
    }

    const FString StatusLine = FString::Printf(
        TEXT("FIFO: %s | last: %s | received: %d | invalid: %d"),
        *PipePath,
        LastRawCommand.IsEmpty() ? TEXT("-") : *LastRawCommand,
        ReceivedCommandCount,
        InvalidCommandCount);
    LastStatusLine = StatusLine;

    GEngine->AddOnScreenDebugMessage(
        reinterpret_cast<uint64>(this),
        0.1f,
        FColor::Green,
        LastStatusLine);
}
