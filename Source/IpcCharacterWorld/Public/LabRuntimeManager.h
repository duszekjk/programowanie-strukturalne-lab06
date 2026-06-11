#pragma once

#include "CoreMinimal.h"
#include "GameFramework/Actor.h"
#include "LabRuntimeManager.generated.h"

UCLASS()
class IPCCHARACTERWORLD_API ALabRuntimeManager : public AActor
{
    GENERATED_BODY()

public:
    ALabRuntimeManager();

protected:
    virtual void BeginPlay() override;

private:
    UFUNCTION()
    void QuitGame();

    UFUNCTION()
    void HandleCollectibleOverlap(AActor* OverlappedActor, AActor* OtherActor);

    void BindQuitKeys();
    void RegisterCollectibles();
    bool IsCollectibleActor(const AActor* Actor) const;

    int32 TotalCollectibles = 0;
    int32 CollectedCollectibles = 0;
};
