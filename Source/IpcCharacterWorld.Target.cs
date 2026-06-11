using UnrealBuildTool;
using System.Collections.Generic;

public class IpcCharacterWorldTarget : TargetRules
{
    public IpcCharacterWorldTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Game;
        DefaultBuildSettings = BuildSettingsVersion.V5;
        IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
        ExtraModuleNames.Add("IpcCharacterWorld");
    }
}
