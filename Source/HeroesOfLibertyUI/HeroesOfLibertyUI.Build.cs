using UnrealBuildTool;

// Interface module (D247): Common UI, styles and screens. Runtime in the .uproject (J8a5-Q3c): a dedicated server (D231)
// builds and loads it without showing it, so the scripts naming its classes compile there too. Depends on
// HeroesOfLiberty, never the other way round.
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
			// Common UI's action bindings (FBindUIActionArgs) are keyed by gameplay tags.
			"GameplayTags",
			// The lobby manager's client (R4.1).
			"HTTP",
			"Json",
		});
	}
}
