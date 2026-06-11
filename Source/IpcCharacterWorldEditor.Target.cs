using UnrealBuildTool;
using System.Collections.Generic;

public class IpcCharacterWorldEditorTarget : TargetRules
{
    public IpcCharacterWorldEditorTarget(TargetInfo Target) : base(Target)
    {
        Type = TargetType.Editor;
        DefaultBuildSettings = BuildSettingsVersion.V5;
        IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
        BuildEnvironment = TargetBuildEnvironment.Unique;
        ExtraModuleNames.Add("IpcCharacterWorld");
    }
}
