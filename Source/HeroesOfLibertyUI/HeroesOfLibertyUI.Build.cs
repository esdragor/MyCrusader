using UnrealBuildTool;

// Interface module (D247): Common UI, styles and screens. ClientOnly in the .uproject, so a dedicated server (D231)
// neither builds nor loads it. Depends on HeroesOfLiberty, never the other way round.
public class HeroesOfLibertyUI : ModuleRules
{
	public HeroesOfLibertyUI(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		PublicIncludePaths.Add(ModuleDirectory);

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore" });

		PrivateDependencyModuleNames.AddRange(new string[]
		{
			"HeroesOfLiberty",
			"Slate",
			"SlateCore",
			"UMG",
			"CommonUI",
			"CommonInput",
			"EnhancedInput",
		});
	}
}
