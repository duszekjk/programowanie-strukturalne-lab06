#include "IpcCharacterWorldGameMode.h"

#include "Components/StaticMeshComponent.h"
#include "Engine/DirectionalLight.h"
#include "Engine/StaticMeshActor.h"
#include "Engine/World.h"
#include "IpcCharacterWorldCharacter.h"
#include "UObject/ConstructorHelpers.h"

AIpcCharacterWorldGameMode::AIpcCharacterWorldGameMode()
{
    DefaultPawnClass = AIpcCharacterWorldCharacter::StaticClass();
}

void AIpcCharacterWorldGameMode::StartPlay()
{
    Super::StartPlay();

    UWorld* World = GetWorld();
    if (!World)
    {
        return;
    }

    static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeAsset(TEXT("/Engine/BasicShapes/Cube.Cube"));
    UStaticMesh* CubeMesh = CubeAsset.Succeeded() ? CubeAsset.Object : nullptr;

    if (CubeMesh)
    {
        AStaticMeshActor* Floor = World->SpawnActor<AStaticMeshActor>(FVector(0.0f, 0.0f, -55.0f), FRotator::ZeroRotator);
        if (Floor)
        {
            Floor->GetStaticMeshComponent()->SetStaticMesh(CubeMesh);
            Floor->SetActorScale3D(FVector(20.0f, 20.0f, 0.1f));
            Floor->SetActorLabel(TEXT("Lab Floor"));
        }

        const FVector BoxLocations[] =
        {
            FVector(300.0f, 0.0f, 30.0f),
            FVector(300.0f, 300.0f, 30.0f),
            FVector(0.0f, 300.0f, 30.0f),
            FVector(-300.0f, 150.0f, 30.0f)
        };

        for (int32 Index = 0; Index < 4; ++Index)
        {
            AStaticMeshActor* Box = World->SpawnActor<AStaticMeshActor>(BoxLocations[Index], FRotator::ZeroRotator);
            if (Box)
            {
                Box->GetStaticMeshComponent()->SetStaticMesh(CubeMesh);
                Box->SetActorScale3D(FVector(0.6f, 0.6f, 0.6f));
                Box->SetActorLabel(FString::Printf(TEXT("Collect Box %d"), Index + 1));
            }
        }
    }

    World->SpawnActor<ADirectionalLight>(FVector(0.0f, 0.0f, 500.0f), FRotator(-45.0f, 30.0f, 0.0f));
}
