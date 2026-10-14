// Copyright Epic Games, Inc. All Rights Reserved.

#include "Engine/Engine.h"
#include "HAL/IConsoleManager.h"
#include "HeroesOfLibertyLog.h"
#include "NavMesh/RecastNavMesh.h"
#include "NavigationSystem.h"
#include "Simulation/HOLUnitView.h"

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

	/** Scripted checks of the orders (J4.7, D289): no unit stands in a rock or a building, where the navmesh has a hole. */
	FAutoConsoleCommand NavUnitsCommand(
		TEXT("hol.Nav.Units"),
		TEXT("hol.Nav.Units: logs the units whose centre is off the navmesh (by more than 10 cm), on the machines that have one"),
		FConsoleCommandDelegate::CreateLambda([]()
		{
			for (const FWorldContext& Context : GEngine->GetWorldContexts())
			{
				UWorld* World = Context.World();
				UNavigationSystemV1* NavSystem = World && World->IsGameWorld() ? FNavigationSystem::GetCurrent<UNavigationSystemV1>(World) : nullptr;
				const ARecastNavMesh* NavMesh = NavSystem ? Cast<ARecastNavMesh>(NavSystem->GetDefaultNavDataInstance()) : nullptr;
				if (!NavMesh)
				{
					continue;
				}
				TArray<FHOLUnitView> Units;
				HOLUnitView::Gather(*World, Units);
				Units.Sort([](const FHOLUnitView& A, const FHOLUnitView& B) { return A.UnitId < B.UnitId; });
				TArray<FString> Off;
				for (const FHOLUnitView& Unit : Units)
				{
					FNavLocation OnNav;
					if (!NavSystem->ProjectPointToNavigation(Unit.Location, OnNav, FVector(10., 10., 1000.), NavMesh))
					{
						Off.Add(FString::Printf(TEXT("#%u at (%.0f, %.0f)"), Unit.UnitId, Unit.Location.X, Unit.Location.Y));
					}
				}
				UE_LOG(LogHOLMatch, Display, TEXT("Nav units: %d units, %d off the navmesh%s%s"), Units.Num(), Off.Num(), Off.IsEmpty() ? TEXT("") : TEXT(": "),
					*FString::Join(Off, TEXT("; ")));
			}
		}));
}
