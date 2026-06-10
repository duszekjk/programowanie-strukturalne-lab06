#include "IpcCharacterWorldGameMode.h"

#include "IpcCharacterWorldCharacter.h"

AIpcCharacterWorldGameMode::AIpcCharacterWorldGameMode()
{
    DefaultPawnClass = AIpcCharacterWorldCharacter::StaticClass();
}
