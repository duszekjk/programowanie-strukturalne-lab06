#include "IpcLabMode.h"

#include "Camera/CameraActor.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/World.h"
#include "GameFramework/DefaultPawn.h"
#include "Kismet/GameplayStatics.h"
#include "UObject/ConstructorHelpers.h"

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

    AActor* Floor = World->SpawnActor<AActor>();
    if (Floor)
    {
        UStaticMeshComponent* Mesh = NewObject<UStaticMeshComponent>(Floor);
        Mesh->RegisterComponent();
        Floor->SetRootComponent(Mesh);
        static ConstructorHelpers::FObjectFinder<UStaticMesh> Plane(TEXT("/Engine/BasicShapes/Plane.Plane"));
        if (Plane.Succeeded())
        {
            Mesh->SetStaticMesh(Plane.Object);
        }
        Floor->SetActorScale3D(FVector(20.0f, 20.0f, 1.0f));
    }

    ACameraActor* Camera = World->SpawnActor<ACameraActor>(FVector(-600.0f, 0.0f, 400.0f), FRotator(-35.0f, 0.0f, 0.0f));
    if (Camera)
    {
        UGameplayStatics::GetPlayerController(World, 0)->SetViewTarget(Camera);
    }
}
