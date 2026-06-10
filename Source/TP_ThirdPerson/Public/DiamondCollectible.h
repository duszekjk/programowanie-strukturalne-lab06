#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "DiamondCollectible.generated.h"

class UBoxComponent;
class UStaticMeshComponent;

UCLASS()
class TP_THIRDPERSON_API ADiamondCollectible : public AActor
{
	GENERATED_BODY()

public:
	ADiamondCollectible();

	void InitializeDiamond(int32 InDiamondIndex, const FString& InSecretCode);

protected:
	virtual void BeginPlay() override;

private:
	UFUNCTION()
	void HandleBeginOverlap(
		UPrimitiveComponent* OverlappedComponent,
		AActor* OtherActor,
		UPrimitiveComponent* OtherComp,
		int32 OtherBodyIndex,
		bool bFromSweep,
		const FHitResult& SweepResult);

	void MarkCollected();

	UPROPERTY(VisibleAnywhere)
	UBoxComponent* TriggerVolume;

	UPROPERTY(VisibleAnywhere)
	UStaticMeshComponent* CubeMesh;

	UPROPERTY(EditAnywhere, Category="Diamond")
	int32 DiamondIndex;

	UPROPERTY(EditAnywhere, Category="Diamond")
	FString SecretCode;

	bool bCollected;
};
