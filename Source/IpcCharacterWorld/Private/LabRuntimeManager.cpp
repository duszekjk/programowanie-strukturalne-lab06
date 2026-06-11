#include "LabRuntimeManager.h"

#include "Components/PrimitiveComponent.h"
#include "GameFramework/Pawn.h"
#include "GameFramework/PlayerController.h"
#include "InputCoreTypes.h"
#include "Kismet/GameplayStatics.h"
#include "Kismet/KismetSystemLibrary.h"

namespace
{
    const FName CollectibleTag(TEXT("Collectible"));
    const FName DiamondTag(TEXT("Diamond"));
}

ALabRuntimeManager::ALabRuntimeManager()
{
   