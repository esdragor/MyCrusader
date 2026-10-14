// Copyright Epic Games, Inc. All Rights Reserved.

#include "Engine/Engine.h"
#include "HAL/IConsoleManager.h"
#include "HeroesOfLibertyLog.h"
#include "NavMesh/RecastNavMesh.h"
#include "NavigationSystem.h"

namespace
{
	const TCHAR* RuntimeGenerationName(const ERuntimeGenerationType Type)
	{
		switch (Type)
		{
		case ERuntimeGenerationType::Static: return TEXT("static");
		case ERuntimeGenerationType::DynamicModifiersOnly: return TEXT("modifiers only");
		case ERuntimeGenerationType::Dynamic: return TEXT("dynamic");
		default: return TEXT("unknown");
		}
	}

	/** Paths of D286 tried by script (J4.1): a path between two points of the ground, on the machines that have a navmesh
	 *  (the server, or a solo game; a client has none). The length against the straight line shows whether it goes around. */
	FAutoConsoleCommand NavPathCommand(
		TEXT("hol.Nav.Path"),
		TEXT("hol.Nav.Path X1 Y1 X2 Y2: logs the navmesh path between two ground points (cm): points, length, straight distance"),
		FConsoleCommandWithArgsDelegate::CreateLambda([](const TArray<FString>& Args)
		{
			if (Args.Num() < 4)
			{
				UE_LOG(LogHOLMatch, Warning, TEXT("Nav path: usage hol.Nav.Path X1 Y1 X2 Y2"));
				return;
			}
			const FVector Start(FCString::Atod(*Args[0]), FCString::Atod(*Args[1]), 0.);
			const FVector End(FCString::Atod(*Args[2]), FCString::Atod(*Args[3]), 0.);
			for (const FWorldContext& Context : GEngine->GetWorldContexts())
			{
				UWorld* World = Context.World();
				UNavigationSystemV1* NavSystem = World && World->IsGameWorld() ? FNavigationSystem::GetCurrent<UNavigationSystemV1>(World) : nullptr;
				const ARecastNavMesh* NavMesh = NavSystem ? Cast<ARecastNavMesh>(NavSystem->GetDefaultNavDataInstance()) : nullptr;
				if (!NavMesh)
				{
					continue;
				}
				// Ground points: projected on the navmesh with a tall box (the map's ground is at Z = 0).
				const FVector Extent(50., 50., 1000.);
				FNavLocation StartOnNav;
				FNavLocation EndOnNav;
				if (!NavSystem->ProjectPointToNavigation(Start, StartOnNav, Extent, NavMesh)
					|| !NavSystem->ProjectPointToNavigation(End, EndOnNav, Extent, NavMesh))
				{
					UE_LOG(LogHOLMatch, Display, TEXT("Nav path: a point is off the navmesh (%s, %s)"), *Start.ToCompactString(), *End.ToCompactString());
					continue;
				}
				const FPathFindingQuery Query(World, *NavMesh, StartOnNav.Location, EndOnNav.Location);
				const FPathFindingResult Result = NavSystem->FindPathSync(Query);
				const double Straight = FVector::Dist2D(StartOnNav.Location, EndOnNav.Location);
				if (!Result.IsSuccessful() || !Result.Path.IsValid())
				{
					UE_LOG(LogHOLMatch, Display, TEXT("Nav path: none (%s), straight %.1f m"), Result.IsPartial() ? TEXT("partial") : TEXT("failed"), Straight / 100.);
					continue;
				}
				const double Length = Result.Path->GetLength();
				UE_LOG(LogHOLMatch, Display, TEXT("Nav path: %d points, length %.1f m, straight %.1f m, ratio %.2f%s (navmesh %s, agent %.0f cm)"),
					Result.Path->GetPathPoints().Num(), Length / 100., Straight / 100., Straight > 1. ? Length / Straight : 1.,
					Result.IsPartial() ? TEXT(", partial") : TEXT(""), RuntimeGenerationName(NavMesh->GetRuntimeGenerationMode()), NavMesh->AgentRadius);
			}
		}));
}
