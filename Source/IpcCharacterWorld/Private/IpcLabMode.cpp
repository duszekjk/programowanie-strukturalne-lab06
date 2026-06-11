#include "IpcLabMode.h"

#include "Camera/CameraActor.h"
#include "Engine/World.h"
#include "GameFramework/DefaultPawn.h"
#include "Kismet/GameplayStatics.h"

AIpcLabMode::AIpcLabMode()
{
    DefaultPawnClass = ADefaultPawn::StaticClass();
}

void AIpcLabMode::StartPlay()
{
    Super::StartPlay();

    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    ACameraActor* Camera = World->SpawnActor<AC