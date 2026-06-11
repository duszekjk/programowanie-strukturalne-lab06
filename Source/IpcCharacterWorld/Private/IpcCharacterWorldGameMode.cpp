#include "IpcCharacterWorldGameMode.h"

#include "IpcCharacterWorldCharacter.h"

AIpcCharacterWorldGameMode::AIpcCharacterWorldGameMode()
{
    DefaultPawnClass = AIpcCharacterWorldCharacter::StaticClass();
}

void AIpcCharacterWorldGameMode::StartPlay()
{
    Super::StartPlay();
}
