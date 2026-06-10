using UnrealBuildTool;
using System.Collections.Generic;

public class IpcCharacterWorldEditorTarget : TargetRules
{
    public IpcCharacterWorldEditorTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Editor;
        DefaultBuildSettings = BuildSettingsVersion.V6;
        IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
        bOverrideBuildEnvironment = true;
        ExtraModuleNames.Add("TP_ThirdPerson");
        ExtraModuleNames.Add("IpcCharacterWorld");
    }
}
