#include "IpcCharacterWorldCharacter.h"

#include "Camera/CameraComponent.h"
#include "CommandReceiverComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/Engine.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"
#include "UObject/ConstructorHelpers.h"

AIpcCharacterWorldCharacter::AIpcCharacterWorldCharacter()
{
    PrimaryActorTick.bCanEverTick = true;

    GetCapsuleComponent()->InitCapsuleSize(42.0f, 96.0f);

    bUseControllerRotationPitch = false;
    bUseControllerRotationYaw = true;
    bUseControllerRotationRoll = false;

    GetCharacterMovement()->bOrientRotationToMovement = false;
    GetCharacterMovement()->RotationRate = FRotator(0.0f, 540.0f, 0.0f);
    GetCharacterMovement()->JumpZVelocity = 700.0f;
    GetCharacterMovement()->AirControl = 0.35f;

    BodyMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("VisibleBody"));
    BodyMesh->SetupAttachment(RootComponent);
    BodyMesh->SetRelativeLocation(FVector(0.0f, 0.0f, -45.0f));
    BodyMesh->SetRelativeScale3D(FVector(0.55f, 0.55f, 1.4f));
    BodyMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);

    static ConstructorHelpers::FObjectFinder<UStaticMesh> BodyAsset(TEXT("/Engine/BasicShapes/Capsule.Capsule"));
    if (BodyAsset.Succeeded())
    {
        BodyMesh->SetStaticMesh(BodyAsset.Object);
    }

    CameraBoom = CreateDefaultSubobject<USpringArmComponent>(TEXT("CameraBoom"));
    CameraBoom->SetupAttachment(RootComponent);
    CameraBoom->TargetArmLength = 450.0f;
    CameraBoom->SetRelativeRotation(FRotator(-20.0f, 0.0f, 0.0f));
    CameraBoom->bUsePawnControlRotation = true;

    FollowCamera = CreateDefaultSubobject<UCameraComponent>(TEXT("FollowCamera"));
    FollowCamera->SetupAttachment(CameraBoom, USpringArmComponent::SocketName);
    FollowCamera->bUsePawnControlRotation = false;

    CommandReceiver = CreateDefaultSubobject<UCommandReceiverComponent>(TEXT("CommandReceiver"));
    AutoPossessPlayer = EAutoReceiveInput::Player0;
}

void AIpcCharacterWorldCharacter::Tick(float DeltaSeconds)
{
    Super::Tick(DeltaSeconds);

    if (GEngine)
    {
        GEngine->AddOnScreenDebugMessage(101, 0.1f, FColor::Yellow, TEXT("Collect boxes to reveal code: PS-LAB06-OK"));
    }
}
