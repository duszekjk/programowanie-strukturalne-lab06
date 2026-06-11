#pragma once

#include "CoreMinimal.h"
#include "GameFramework/GameModeBase.h"
#include "IpcLabMode.generated.h"

UCLASS()
class IPCCHARACTERWORLD_API AIpcLabMode : public AGameModeBase
{
    GENERATED_BODY()

public:
    AIpcLabMode();
    virtual void StartPlay() override;
};
