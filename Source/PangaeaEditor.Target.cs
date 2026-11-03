using UnrealBuildTool;
using System.Collections.Generic;

public class PangaeaEditorTarget : TargetRules
{
	public PangaeaEditorTarget(TargetInfo Target) : base(Target)
	{
		Type = TargetType.Editor;
		DefaultBuildSettings = BuildSettingsVersion.Latest;
		IncludeOrderVersion = EngineIncludeOrderVersion.Latest;
		ExtraModuleNames.Add("Pangaea");
	}
}
