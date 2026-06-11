#include "IpcCharacterWorldCharacter.h"

#include "Camera/CameraComponent.h"
#include "CommandReceiverComponent.h"
#include "Components/CapsuleComponent.h"
#include "Engine/Engine.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"

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
        GEngine->AddOnScreenDebugMessage(
            101,
            0.1f,
            FColor::Yellow,
            TEXT("IpcCharacterWorld: character class loaded")
        );
    }
}

void AIpcCharacterWorldCharacter::SetupPlayerInputComponent(UInputComponent* PlayerInputComponent)
{
    Super::SetupPlayerInputComponent(PlayerInputComponent);

    PlayerInputComponent->BindAxis(TEXT("MoveForward"), this, &AIpcCharacterWorldCharacter::MoveForward);
    PlayerInputComponent->BindAxis(TEXT("MoveRight"), this, &AIpcCharacterWorldCharacter::MoveRight);
    PlayerInputComponent->BindAxis(TEXT("Turn"), this, &AIpcCharacterWorldCharacter::Turn);
    PlayerInputComponent->BindAxis(TEXT("LookUp"), this, &AIpcCharacterWorldCharacter::LookUp);
}

void AIpcCharacterWorldCharacter::MoveForward(float Value)
{
    if (Value != 0.0f)
    {
        AddMovementInput(GetActorForwardVector(), Value);
    }
}

void AIpcCharacterWorldCharacter::MoveRight(float Value)
{
    if (Value != 0.0f)
    {
        AddMovementInput(GetActorRightVector(), Value);
    }
}

void AIpcCharacterWorldCharacter::Turn(float Value)
{
    AddControllerYawInput(Value);
}

void AIpcCharacterWorldCharacter::LookUp(float Value)
{
    AddControllerPitchInput(Value);
}
