#include "IpcCharacterWorldCharacter.h"

#include "Camera/CameraComponent.h"
#include "CommandReceiverComponent.h"
#include "Components/CapsuleComponent.h"
#include "Engine/Engine.h"
#include "GameFramework/CharacterMovementComponent.h"
#include "GameFramework/SpringArmComponent.h"

AIpcCharacterWorldCharacter::AIpcCharacterWorldCharacter()
{
    PrimaryActorTick.bCanEverTick =