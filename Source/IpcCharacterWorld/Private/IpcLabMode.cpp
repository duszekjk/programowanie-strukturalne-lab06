#include "IpcLabMode.h"

#include "Camera/CameraActor.h"
#include "Engine/DirectionalLight.h"
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

    ACameraActor* Camera = World->SpawnActor<ACameraActor>(
        FVector(-600.0f, 0.0f, 350.0f),
        FRotator(-25.0f, 0.0f, 0.0f)
    );

    APlayerController* PlayerController = UGameplayStatics::GetPlayerController(World, 0);
    if (Camera && PlayerController)
    {
        PlayerController->SetViewTarget(Camera);
    }

    World->SpawnActor<ADirectionalLight>(
        FVector(0.0f, 0.0f, 400.0f),
        FRotator(-45.0f, 0.0f, 0.0f)
    );
}
