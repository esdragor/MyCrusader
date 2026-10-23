// Copyright Epic Games, Inc. All Rights Reserved.

#include "Buildings/HOLBuilding.h"
#include "Buildings/HOLBuildingTypes.h"
#include "Commands/HOLUnitOrderProcessor.h"
#include "Engine/Engine.h"
#include "EngineUtils.h"
#include "HAL/IConsoleManager.h"
#include "HeroesOfLibertyLog.h"
#include "MassCommonFragments.h"
#include "MassEntitySubsystem.h"
#include "NavMesh/RecastNavMesh.h"
#include "NavigationSystem.h"
#include "Simulation/HOLSimulationFragments.h"
#include "Simulation/HOLSimulationProcessors.h"
#include "Simulation/HOLSimulationSubsystem.h"
#include "Simulation/HOLUnitSubsystem.h"
#include "Simulation/HOLUnitTypes.h"
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
				FPathFindingResult Result = NavSystem->FindPathSync(Query);
				// What an order walks: around two neighbouring buildings, perhaps between them (D324).
				if (Result.IsSuccessful())
				{
					Result.Path = UHOLUnitOrderProcessor::FindPathThroughWays(*World, *NavSystem, *NavMesh, StartOnNav.Location, EndOnNav.Location, Result.Path);
				}
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
		TEXT("hol.Nav.Units [Tolerance=10]: logs the units whose centre is off the navmesh (by more than Tolerance cm), on the machines that have one"),
		FConsoleCommandWithArgsDelegate::CreateLambda([](const TArray<FString>& Args)
		{
			// The part "rocks" passes 25 (ridge_army): a unit as drawn, between two fixed steps, cuts across the corner of the
			// navmesh's voxel edge round a trunk, 27 cm clear of the trunk itself.
			const double Tolerance = Args.IsEmpty() ? 10. : FMath::Max(1., FCString::Atod(*Args[0]));
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
					if (!NavSystem->ProjectPointToNavigation(Unit.Location, OnNav, FVector(Tolerance, Tolerance, 1000.), NavMesh))
					{
						Off.Add(FString::Printf(TEXT("#%u at (%.0f, %.0f)"), Unit.UnitId, Unit.Location.X, Unit.Location.Y));
					}
				}
				UE_LOG(LogHOLMatch, Display, TEXT("Nav units: %d units, %d off the navmesh%s%s"), Units.Num(), Off.Num(), Off.IsEmpty() ? TEXT("") : TEXT(": "),
					*FString::Join(Off, TEXT("; ")));
			}
		}));

	/** A building's footprint on the ground plane (D310): a rectangle turned by its yaw. */
	struct FWatchedFootprint
	{
		FString Name;
		FVector2D Centre = FVector2D::ZeroVector;
		FVector2D HalfExtent = FVector2D::ZeroVector;
		double Yaw = 0.;
	};

	void GetFootprints(UWorld& World, TArray<FWatchedFootprint>& OutFootprints)
	{
		OutFootprints.Reset();
		for (TActorIterator<AHOLBuilding> It(&World); It; ++It)
		{
			// A bare foundation is walked over (D312): watched once it blocks the navmesh.
			const FHOLBuildingTypeRow* Type = It->BlocksNavigation() ? It->GetType() : nullptr;
			if (Type)
			{
				OutFootprints.Add({ FString::Printf(TEXT("%s of slot %d"), *It->GetTypeName().ToString(), It->GetOwnerPlayer()),
					FVector2D(It->GetActorLocation()), UHOLBuildingSettings::GetBodySize(Type->Footprint) * 0.5, It->GetActorRotation().Yaw });
			}
		}
	}

	/** Distance (cm) from Point to the footprint, negative inside it (to its nearest side). */
	double GetFootprintDistance(const FVector2D& Point, const FWatchedFootprint& Footprint)
	{
		const FVector2D Local = (Point - Footprint.Centre).GetRotated(-Footprint.Yaw);
		const FVector2D Outside(FMath::Abs(Local.X) - Footprint.HalfExtent.X, FMath::Abs(Local.Y) - Footprint.HalfExtent.Y);
		if (Outside.X <= 0. && Outside.Y <= 0.)
		{
			return FMath::Max(Outside.X, Outside.Y);
		}
		return FVector2D(FMath::Max(Outside.X, 0.), FMath::Max(Outside.Y, 0.)).Size();
	}

	/**
	 * Scripted check of D310 (J6a.1): no unit's body in a building's footprint. Every fixed step of the authoritative
	 * worlds (solo, server), each unit's gap between its body and the nearest footprint; the closest one, and the bodies
	 * that entered a footprint, with what they were doing. Development only, switched on by hol.Nav.Footprints on.
	 */
	struct FFootprintWatch
	{
		TWeakObjectPtr<UWorld> World;
		FDelegateHandle StepHandle;
		int64 Steps = 0;
		int64 Samples = 0;
		/** Unit-steps with a body in a footprint (gap below 0), and the units that did. */
		int64 Inside = 0;
		TSet<uint32> UnitsInside;
		double Closest = TNumericLimits<double>::Max();
		FString ClosestWhat;
		/** Closest gap per activity (EHOLUnitActivity). */
		TMap<FString, double> ClosestByActivity;
		/** Entries logged as they happen (a unit is logged again only after it left the footprint). */
		int32 Logged = 0;
		TSet<uint32> InsideNow;
		/** UHOLFootprintKeepOutProcessor's count when the watch started. */
		int64 PushedOutAtStart = 0;
		/** Units standing still eased out of a visible wall (C.1), counted apart (C.2). */
		int64 EasedOutAtStart = 0;
		/** UHOLUnitOrderProcessor's new paths for a building in the way, when the watch started. */
		int64 RepathsAtStart = 0;
	};
	TArray<TUniquePtr<FFootprintWatch>> FootprintWatches;
	/** Entries logged as they happen, per watch, at most. */
	constexpr int32 MaxFootprintEntriesLogged = 30;

	FString DescribeUnit(const FMassEntityManager& EntityManager, const FMassEntityHandle Entity, const FHOLUnitStateFragment& State)
	{
		FString Text = StaticEnum<EHOLUnitActivity>()->GetNameStringByValue(static_cast<int64>(State.Activity));
		if (const FHOLMoveOrderFragment* Order = EntityManager.GetFragmentDataPtr<FHOLMoveOrderFragment>(Entity))
		{
			Text += TEXT("/") + StaticEnum<EHOLMovePhase>()->GetNameStringByValue(static_cast<int64>(Order->Phase));
		}
		const FHOLGatherFragment* Gather = EntityManager.GetFragmentDataPtr<FHOLGatherFragment>(Entity);
		if (Gather && Gather->IsGathering())
		{
			Text += TEXT("/") + StaticEnum<EHOLGatherPhase>()->GetNameStringByValue(static_cast<int64>(Gather->Phase));
		}
		return Text;
	}

	void WatchFootprints(FFootprintWatch& Watch)
	{
		UWorld* World = Watch.World.Get();
		const UHOLUnitSubsystem* Units = World ? World->GetSubsystem<UHOLUnitSubsystem>() : nullptr;
		const UMassEntitySubsystem* EntitySubsystem = World ? World->GetSubsystem<UMassEntitySubsystem>() : nullptr;
		if (!Units || !EntitySubsystem)
		{
			return;
		}
		TArray<FWatchedFootprint> Footprints;
		GetFootprints(*World, Footprints);
		if (Footprints.IsEmpty())
		{
			return;
		}
		++Watch.Steps;
		const FMassEntityManager& EntityManager = EntitySubsystem->GetEntityManager();
		TArray<uint32> UnitIds;
		Units->GetAllUnitIds(UnitIds);
		TSet<uint32> InsideNow;
		for (const uint32 UnitId : UnitIds)
		{
			const FMassEntityHandle Entity = Units->FindEntity(UnitId);
			const FTransformFragment* Transform = EntityManager.IsEntityValid(Entity) ? EntityManager.GetFragmentDataPtr<FTransformFragment>(Entity) : nullptr;
			const FHOLUnitStateFragment* State = Transform ? EntityManager.GetFragmentDataPtr<FHOLUnitStateFragment>(Entity) : nullptr;
			const FHOLUnitTypeConstants* Constants = State ? EntityManager.GetConstSharedFragmentDataPtr<FHOLUnitTypeConstants>(Entity) : nullptr;
			if (!Constants)
			{
				continue;
			}
			const FVector2D Location(Transform->GetTransform().GetLocation());
			double Gap = TNumericLimits<double>::Max();
			const FWatchedFootprint* Nearest = nullptr;
			for (const FWatchedFootprint& Footprint : Footprints)
			{
				const double FootprintGap = GetFootprintDistance(Location, Footprint) - Constants->Radius;
				if (FootprintGap < Gap)
				{
					Gap = FootprintGap;
					Nearest = &Footprint;
				}
			}
			++Watch.Samples;
			const FString Activity = StaticEnum<EHOLUnitActivity>()->GetNameStringByValue(static_cast<int64>(State->Activity));
			double& ClosestOfActivity = Watch.ClosestByActivity.FindOrAdd(Activity, TNumericLimits<double>::Max());
			ClosestOfActivity = FMath::Min(ClosestOfActivity, Gap);
			if (Gap < Watch.Closest)
			{
				Watch.Closest = Gap;
				Watch.ClosestWhat = FString::Printf(TEXT("#%u %s at (%.0f, %.0f), %s"), UnitId, *DescribeUnit(EntityManager, Entity, *State), Location.X, Location.Y,
					*Nearest->Name);
			}
			if (Gap < 0.)
			{
				++Watch.Inside;
				Watch.UnitsInside.Add(UnitId);
				InsideNow.Add(UnitId);
				if (!Watch.InsideNow.Contains(UnitId) && Watch.Logged < MaxFootprintEntriesLogged)
				{
					++Watch.Logged;
					UE_LOG(LogHOLMatch, Display, TEXT("Footprint entered: #%u %s at (%.0f, %.0f), body %.0f cm into %s"), UnitId,
						*DescribeUnit(EntityManager, Entity, *State), Location.X, Location.Y, -Gap, *Nearest->Name);
				}
			}
		}
		Watch.InsideNow = MoveTemp(InsideNow);
	}

	/**
	 * How far the navmesh stays from each footprint (D310): navmesh rays from 5 m out toward 5 points of each side and
	 * toward each corner; the gap between the ray's stop and the footprint.
	 */
	void ReportFootprintNavmesh(UWorld& World)
	{
		UNavigationSystemV1* NavSystem = FNavigationSystem::GetCurrent<UNavigationSystemV1>(&World);
		const ANavigationData* NavData = NavSystem ? NavSystem->GetDefaultNavDataInstance() : nullptr;
		if (!NavData)
		{
			return;
		}
		TArray<FWatchedFootprint> Footprints;
		GetFootprints(World, Footprints);
		for (const FWatchedFootprint& Footprint : Footprints)
		{
			double SideMin = TNumericLimits<double>::Max();
			double SideMax = 0.;
			double CornerMin = TNumericLimits<double>::Max();
			double CornerMax = 0.;
			int32 Rays = 0;
			auto Probe = [&](const FVector2D& LocalEdge, const FVector2D& LocalOut, double& Min, double& Max)
			{
				// The map's ground is at Z = 0; the start projected on the navmesh with a tall box.
				const FVector Start(Footprint.Centre + (LocalEdge + LocalOut * 500.).GetRotated(Footprint.Yaw), 0.);
				const FVector End(Footprint.Centre + LocalEdge.GetRotated(Footprint.Yaw), 0.);
				FNavLocation StartOnNav;
				if (!NavSystem->ProjectPointToNavigation(Start, StartOnNav, FVector(10., 10., 1000.), NavData))
				{
					return;
				}
				++Rays;
				FVector Hit;
				const double Gap = NavData->Raycast(StartOnNav.Location, FVector(FVector2D(End), StartOnNav.Location.Z), Hit, NavData->GetDefaultQueryFilter())
					? GetFootprintDistance(FVector2D(Hit), Footprint) : 0.;
				Min = FMath::Min(Min, Gap);
				Max = FMath::Max(Max, Gap);
			};
			const FVector2D& Half = Footprint.HalfExtent;
			for (const double Along : { -0.8, -0.4, 0., 0.4, 0.8 })
			{
				Probe(FVector2D(Half.X, Along * Half.Y), FVector2D(1., 0.), SideMin, SideMax);
				Probe(FVector2D(-Half.X, Along * Half.Y), FVector2D(-1., 0.), SideMin, SideMax);
				Probe(FVector2D(Along * Half.X, Half.Y), FVector2D(0., 1.), SideMin, SideMax);
				Probe(FVector2D(Along * Half.X, -Half.Y), FVector2D(0., -1.), SideMin, SideMax);
			}
			for (const FVector2D& Corner : { FVector2D(1., 1.), FVector2D(1., -1.), FVector2D(-1., 1.), FVector2D(-1., -1.) })
			{
				Probe(Half * Corner, Corner.GetSafeNormal(), CornerMin, CornerMax);
			}
			UE_LOG(LogHOLMatch, Display, TEXT("Footprint navmesh: %s, %d rays: sides %.0f to %.0f cm, corners %.0f to %.0f cm"), *Footprint.Name, Rays,
				SideMin, SideMax, CornerMin, CornerMax);
		}
	}

	void ReportFootprintWatch(const FFootprintWatch& Watch)
	{
		TArray<FString> ByActivity;
		for (const TPair<FString, double>& Pair : Watch.ClosestByActivity)
		{
			ByActivity.Add(FString::Printf(TEXT("%s %.0f"), *Pair.Key, Pair.Value));
		}
		ByActivity.Sort();
		UE_LOG(LogHOLMatch, Display, TEXT("Footprint watch: %lld steps, %lld samples, closest body gap %.0f cm (%s); bodies in a footprint: %lld unit-steps, %d units; closest by activity: %s; eased out of a visible wall: %lld; put back out: %lld, new paths: %lld"),
			Watch.Steps, Watch.Samples, Watch.Samples > 0 ? Watch.Closest : 0., *Watch.ClosestWhat, Watch.Inside, Watch.UnitsInside.Num(),
			*FString::Join(ByActivity, TEXT(", ")), UHOLFootprintKeepOutProcessor::GetNumEasedOut() - Watch.EasedOutAtStart,
			UHOLFootprintKeepOutProcessor::GetNumPushedOut() - Watch.PushedOutAtStart,
			UHOLUnitOrderProcessor::GetTotalBuildingRepaths() - Watch.RepathsAtStart);
	}

	FAutoConsoleCommand NavFootprintsCommand(
		TEXT("hol.Nav.Footprints"),
		TEXT("hol.Nav.Footprints on|off|report|navmesh: watches at every fixed step the gap between each unit's body and the buildings' footprints ")
		TEXT("(D310: never inside), reports it (off also ends the watch), or how far the navmesh stays from each footprint (server or solo)"),
		FConsoleCommandWithArgsDelegate::CreateLambda([](const TArray<FString>& Args)
		{
			const FString Mode = Args.IsEmpty() ? FString(TEXT("report")) : Args[0];
			FootprintWatches.RemoveAll([](const TUniquePtr<FFootprintWatch>& Watch) { return !Watch->World.IsValid(); });
			for (const FWorldContext& Context : GEngine->GetWorldContexts())
			{
				UWorld* World = Context.World();
				UHOLSimulationSubsystem* Simulation = World && World->IsGameWorld() && World->GetNetMode() != NM_Client
					? World->GetSubsystem<UHOLSimulationSubsystem>() : nullptr;
				if (!Simulation)
				{
					continue;
				}
				const int32 Index = FootprintWatches.IndexOfByPredicate([World](const TUniquePtr<FFootprintWatch>& Watch) { return Watch->World.Get() == World; });
				if (Mode == TEXT("navmesh"))
				{
					ReportFootprintNavmesh(*World);
				}
				else if (Mode == TEXT("on"))
				{
					if (Index != INDEX_NONE)
					{
						Simulation->GetOnStepEnded().Remove(FootprintWatches[Index]->StepHandle);
						FootprintWatches.RemoveAt(Index);
					}
					FFootprintWatch* Watch = FootprintWatches.Add_GetRef(MakeUnique<FFootprintWatch>()).Get();
					Watch->World = World;
					Watch->PushedOutAtStart = UHOLFootprintKeepOutProcessor::GetNumPushedOut();
					Watch->EasedOutAtStart = UHOLFootprintKeepOutProcessor::GetNumEasedOut();
					Watch->RepathsAtStart = UHOLUnitOrderProcessor::GetTotalBuildingRepaths();
					Watch->StepHandle = Simulation->GetOnStepEnded().AddLambda([Watch](int64) { WatchFootprints(*Watch); });
					UE_LOG(LogHOLMatch, Display, TEXT("Footprint watch: on"));
				}
				else if (Index != INDEX_NONE)
				{
					ReportFootprintWatch(*FootprintWatches[Index]);
					if (Mode == TEXT("off"))
					{
						Simulation->GetOnStepEnded().Remove(FootprintWatches[Index]->StepHandle);
						FootprintWatches.RemoveAt(Index);
					}
				}
			}
		}));
}
