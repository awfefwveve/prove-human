using UnrealBuildTool;

public class Pangaea : ModuleRules
{
	public Pangaea(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicDependencyModuleNames.AddRange(new string[]
		{
			"Core", "CoreUObject", "Engine", "InputCore", "EnhancedInput", "Slate", "SlateCore"
		});
	}
}
