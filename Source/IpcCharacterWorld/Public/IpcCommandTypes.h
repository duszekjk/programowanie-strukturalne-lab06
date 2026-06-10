#pragma once

#include "CoreMinimal.h"
#include "IpcCommandTypes.generated.h"

UENUM(BlueprintType)
enum class ECommandType : uint8
{
    Forward,
    Backward,
    Left,
    Right,
    Jump,
    Stop,
    Reset,
    Ping,
    Unknown
};

USTRUCT(BlueprintType)
struct FQueuedCommand
{
    GENERATED_BODY()

    UPROPERTY()
    ECommandType Type = ECommandType::Unknown;

    UPROPERTY()
    FString RawText;

    UPROPERTY()
    double ReceivedAtSeconds = 0.0;
};
