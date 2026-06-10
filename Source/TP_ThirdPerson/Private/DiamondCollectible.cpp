#include "DiamondCollectible.h"

#include "Components/BoxComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMesh.h"
#include "Engine/CollisionProfile.h"
#include "GameFramework/Character.h"
#include "UObject/ConstructorHelpers.h"
#include "TP_ThirdPersonCharacter.h"

ADiamondCollectible::ADiamondCollectible()
	: DiamondIndex(-1)
	, bCollected(false)
{
	PrimaryActorTick.bCanEverTick = false;

	TriggerVolume = CreateDefaultSubobject<UBoxComponent>(TEXT("TriggerVolume"));
	TriggerVolume->SetBoxExtent(FVector(70.f, 70.f, 70.f));
	TriggerVolume->SetCollisionEnabled(ECollisionEnabled::QueryOnly);
	TriggerVolume->SetCollisionObjectType(ECC_WorldDynamic);
	TriggerVolume->SetCollisionResponseToAllChannels(ECR_Ignore);
	TriggerVolume->SetCollisionResponseToChannel(ECC_Pawn, ECR_Overlap);
	TriggerVolume->SetGenerateOverlapEvents(true);
	RootComponent = TriggerVolume;

	CubeMesh = CreateDefaultSubobject<UStaticMeshComponent>(TEXT("CubeMesh"));
	CubeMesh->SetupAttachment(RootComponent);
	CubeMesh->SetCollisionEnabled(ECollisionEnabled::NoCollision);
	CubeMesh->SetRelativeScale3D(FVector(0.75f));
	CubeMesh->SetRelativeRotation(FRotator(45.f, 45.f, 0.f));

	static ConstructorHelpers::FObjectFinder<UStaticMesh> CubeMeshAsset(TEXT("/Engine/BasicShapes/Cube.Cube"));
	if (CubeMeshAsset.Succeeded())
	{
		CubeMesh->SetStaticMesh(CubeMeshAsset.Object);
	}

	SetActorEnableCollision(true);
}

void ADiamondCollectible::BeginPlay()
{
	Super::BeginPlay();
	TriggerVolume->OnComponentBeginOverlap.AddDynamic(this, &ADiamondCollectible::HandleBeginOverlap);
}

void ADiamondCollectible::InitializeDiamond(int32 InDiamondIndex, const FString& InSecretCode)
{
	DiamondIndex = InDiamondIndex;
	SecretCode = InSecretCode;
#if WITH_EDITOR
	SetActorLabel(FString::Printf(TEXT("Diamond_%02d"), DiamondIndex + 1));
#endif
}

void ADiamondCollectible::HandleBeginOverlap(
	UPrimitiveComponent* OverlappedComponent,
	AActor* OtherActor,
	UPrimitiveComponent* OtherComp,
	int32 OtherBodyIndex,
	bool bFromSweep,
	const FHitResult& SweepResult)
{
	if (bCollected || OtherActor == nullptr)
	{
		return;
	}

	if (ATP_ThirdPersonCharacter* Character = Cast<ATP_ThirdPersonCharacter>(OtherActor))
	{
		bCollected = true;
		Character->RegisterDiamondCollected(DiamondIndex, SecretCode);
		Destroy();
	}
}

void ADiamondCollectible::MarkCollected()
{
	bCollected = true;
}
