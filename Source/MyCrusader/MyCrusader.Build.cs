// Copyright Epic Games, Inc. All Rights Reserved.

using UnrealBuildTool;

public class MyCrusader : ModuleRules
{
	public MyCrusader(ReadOnlyTargetRules Target) : base(Target)
	{
		PCHUsage = PCHUsageMode.UseExplicitOrSharedPCHs;

		// Code tree (folders created with their first file): Simulation (authoritative, server),
		// Presentation (client view), Commands (order queue), StressTest (load tests, D190 / D218).
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
			"MassRepresentation",
			"MassLOD",
			"MassSimulation",
			"MassActors",
			"StateTreeModule",
			"MassAIBehavior",
			"NavigationSystem",
			"AIModule",
		});

		PrivateDependencyModuleNames.AddRange(new string[] { "NetCore" });

		// Iris replication (D29, M2.5).
		SetupIrisSupport(Target);
	}
}
