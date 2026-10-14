// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class HeroesOfLiberty : ModuleRules
{
	public HeroesOfLiberty(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		// Code tree (folders created with their first file): Simulation (authoritative, server),
		// Presentation (client view), Commands (order queue), StressTest (load tests, D190 / D218),
		// Match (configuration, players), Map (start slots, bounds), Player (controller, RTS camera), Settings,
		// Buildings (replicated actors and their type table, D260), Net (unit replication, D269 to D271).
		PublicIncludePaths.Add(ModuleDirectory);

		PublicDependencyModuleNames.AddRange(new string[] { "Core", "CoreUObject", "Engine", "InputCore" });

		// Light units (D29, D215): Mass core + navmesh path following + instanced representation.
		PublicDependencyModuleNames.AddRange(new string[]
		{
			"MassEntity",
			"MassCore",
			"MassSignals",
			"MassCommon",
			"MassSpawner",
			"MassMovement",
			"MassNavigation",
			"MassNavMeshNavigation",
			"NavCorridor",
			"MassRepresentation",
			"MassLOD",
			"MassSimulation",
			"MassActors",
			"StateTreeModule",
			"MassAIBehavior",
			"NavigationSystem",
			"AIModule",
		});

		// SlateCore: the stress test checks the game window state before asking for a capture (D190).
		PrivateDependencyModuleNames.AddRange(new string[] { "NetCore", "DeveloperSettings", "RHI", "RenderCore", "SlateCore", "Json", "JsonUtilities" });

		// The match controller's camera keys (D255).
		PrivateDependencyModuleNames.Add("EnhancedInput");

		// Iris replication (D29, M2.5).
		SetupIrisSupport(Target);
	}
}
