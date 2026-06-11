using UnrealBuildTool;
using System.Collections.Generic;

public class IpcCharacterWorldTarget : TargetRules
{
    public IpcCharacterWorldTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Game;
        DefaultBuildSettings = BuildSettingsVersion.V6;
        IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
        ExtraModuleNames.Add("IpcCharacterWorld");
    }
}
