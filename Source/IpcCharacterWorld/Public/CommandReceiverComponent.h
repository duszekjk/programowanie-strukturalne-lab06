#pragma once

#include "CoreMinimal.h"
#include "Components/ActorComponent.h"
#include "IpcCommandTypes.h"
#include <atomic>

#include "CommandReceiverComponent.generated.h"

class ACharacter;
class FRunnableThread;

UCLASS(ClassGroup=(IPC), meta=(BlueprintSpawnableComponent))
class IPCCHARACTERWORLD_API UCommandReceiverComponent : public UActorComponent
{
    GENERATED_BODY()

public:
    UCommandReceiverComponent();

    virtual void BeginPlay() override;
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    virtual void TickComponent(float DeltaTime, ELevelTick TickType, FActorComponentTickFunction* ThisTickFunction) override;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="IPC")
    FString PipePath = TEXT("/tmp/ue_character_commands");

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Movement")
    float MoveImpulseSeconds = 0.25f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Movement")
    float TurnDegrees = 15.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Movement")
    float MoveStrength = 1.0f;

    UPROPERTY(EditAnywhere, BlueprintReadWrite, Category="Debug")
    bool bLogCommands = true;

private:
    friend class FFifoCommandWorker;

    void StartWorker();
    void StopWorker();
    void EnsurePipeExists() const;
    void ExecuteCommand(const FQueuedCommand& Command);
    void ResetCharacter();
    void UpdateDebugOverlay();
    static ECommandType ParseCommandType(const FString& RawText);

    TWeakObjectPtr<ACharacter> CharacterOwner;
    FTransform InitialTransform;
    TQueue<FQueuedCommand, EQueueMode::Mpsc> PendingCommands;
    std::atomic<bool> bShouldStop;
    class FFifoCommandWorker* Worker;
    FRunnableThread* WorkerThread;
    int32 ReceivedCommandCount;
    int32 InvalidCommandCount;
    float RemainingForwardTime;
    float RemainingBackwardTime;
    FString LastRawCommand;
    FString LastStatusLine;
};
