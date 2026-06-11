#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Pawn.h"
#include "LabCommand.h"
#include "LabPawn.generated.h"

class UCapsuleComponent;
class UCameraComponent;
class UFloatingPawnMovement;

UCLASS()
class IPCCHARACTERWORLD_API ALabPawn : public APawn
{
    GENERATED_BODY()

public:
    ALabPawn();
    virtual void Tick(float DeltaSeconds) override;
    void ApplyLabCommand(ELabCommand Command);

protected:
    virtual void BeginPlay() override;

private:
    UPROPERTY(VisibleAnywhere)
    UCapsuleComponent* Capsule;

    UPROPERTY(VisibleAnywhere)
    UStaticMeshComponent* BodyMesh;

    UPROPERTY(VisibleAnywhere)
    UCameraComponent* Camera;

    UPROPERTY(VisibleAnywhere)
    UFloatingPawnMovement* Movement;

    FVector StartLocation;
    FRotator StartRotation;
    float VerticalVelocity;
};
