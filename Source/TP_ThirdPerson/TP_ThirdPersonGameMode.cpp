// Copyright Epic Games, Inc. All Rights Reserved.

#include "TP_ThirdPersonGameMode.h"

#include "DiamondCollectible.h"
#include "Engine/World.h"

ATP_ThirdPersonGameMode::ATP_ThirdPersonGameMode()
{
	// stub
}

void ATP_ThirdPersonGameMode::BeginPlay()
{
	Super::BeginPlay();
	SpawnDiamonds();
}

void ATP_ThirdPersonGameMode::SpawnDiamonds()
{
	if (!GetWorld())
	{
		return;
	}

	struct FDiamondSpawnSpec
	{
		int32 Index;
		FVector Location;
		FRotator Rotation;
		FString SecretCode;
	};

	const FDiamondSpawnSpec Specs[] = {
		{0, FVector(450.f, 0.f, 120.f), FRotator(0.f, 0.f, 0.f), TEXT("PS-DIAMOND-01-4F9A2C")},
		{1, FVector(700.f, 140.f, 120.f), FRotator(0.f, 15.f, 0.f), TEXT("PS-DIAMOND-02-7B18D1")},
		{2, FVector(950.f, -170.f, 120.f), FRotator(0.f, -20.f, 0.f), TEXT("PS-DIAMOND-03-2E6C8F")},
		{3, FVector(1200.f, 220.f, 120.f), FRotator(0.f, 35.f, 0.f), TEXT("PS-DIAMOND-04-91A3E4")},
		{4, FVector(1450.f, -260.f, 120.f), FRotator(0.f, -45.f, 0.f), TEXT("PS-DIAMOND-05-5D0B7A")},
		{5, FVector(1700.f, 320.f, 120.f), FRotator(0.f, 60.f, 0.f), TEXT("PS-DIAMOND-06-C38F10")},
		{6, FVector(1950.f, -390.f, 120.f), FRotator(0.f, -75.f, 0.f), TEXT("PS-DIAMOND-07-A6D4E2")},
		{7, FVector(2200.f, 480.f, 120.f), FRotator(0.f, 90.f, 0.f), TEXT("PS-DIAMOND-08-1F97B5")},
		{8, FVector(2450.f, -560.f, 120.f), FRotator(0.f, -105.f, 0.f), TEXT("PS-DIAMOND-09-8C2D6E")},
		{9, FVector(2700.f, 640.f, 120.f), FRotator(0.f, 120.f, 0.f), TEXT("PS-DIAMOND-10-DA5E31")},
	};

	for (const FDiamondSpawnSpec& Spec : Specs)
	{
		FActorSpawnParameters SpawnParameters;
		SpawnParameters.SpawnCollisionHandlingOverride = ESpawnActorCollisionHandlingMethod::AdjustIfPossibleButAlwaysSpawn;

		if (ADiamondCollectible* Diamond = GetWorld()->SpawnActor<ADiamondCollectible>(ADiamondCollectible::StaticClass(), Spec.Location, Spec.Rotation, SpawnParameters))
		{
			Diamond->InitializeDiamond(Spec.Index, Spec.SecretCode);
		}
	}
}
