#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Character.h"
#include "IpcCharacterWorldCharacter.generated.h"

class USpringArmComponent;
class UCameraComponent;
class UCommandReceiverComponent;

UCLASS()
class IPCCHARACTERWORLD_API AIpcCharacterWorldCharacter : public ACharacter
{
    GENERATED_BODY()

public:
    AIpcCharacterWorldCharacter();

    virtual void Tick(float DeltaSeconds) override;

protected:
    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Camera")
    USpringArmComponent* CameraBoom;

    UPROPERTY(VisibleAnywhere, BlueprintReadOnly, Category="Camera")
    UCameraComponent* FollowCamera;

    U