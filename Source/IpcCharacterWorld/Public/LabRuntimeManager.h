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

    virtual void Tick(float DeltaSeconds) override;

protected:
    virtual void BeginPlay() override;

private:
    void QuitGame();
    void RegisterCollectibles();
    void CollectActor(AActor