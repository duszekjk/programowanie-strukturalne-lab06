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
            Floor->SetActorScale3D(FVector(