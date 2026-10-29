// Copyright Epic Games, Inc. All Rights Reserved.

#include "Simulation/HOLSimulationProcessors.h"

#include "Buildings/HOLBuilding.h"
#include "Buildings/HOLBuildingTypes.h"
#include "Buildings/HOLConstructionRules.h"
#include "Components/StaticMeshComponent.h"
#include "Engine/StaticMeshActor.h"
#include "EngineUtils.h"
#include "HAL/IConsoleManager.h"
#include "HeroesOfLibertyLog.h"
#include "MassCommonFragments.h"
#include "MassCommonTypes.h"
#include "MassExecutionContext.h"
#include "MassMovementFragments.h"
#include "MassNavMeshNavigationFragments.h"
#include "MassNavigationFragments.h"
#include "NavigationSystem.h"
#include "Misc/ScopeExit.h"
#include "Simulation/HOLAvoidanceHoldFragments.h"
#include "Simulation/HOLAvoidRules.h"
#include "Simulation/HOLCombatRules.h"
#include "Simulation/HOLGatherProcessor.h"
#include "Simulation/HOLSimulationFragments.h"
#include "Simulation/HOLUnitTypes.h"

#include <atomic>

namespace UE::HeroesOfLiberty::Simulation
{
	/** Buckets of the look-ahead's cell table (a power of two), and the width (cm) of its cells. */
	constexpr int32 LookAheadBuckets = 4096;
	constexpr double LookAheadCellSize = 250.0;
	/** Angle (degrees) between two ways a walker tries around a body ahead (D293). */
	constexpr double AvoidAngleStep = 10.0;
	/** Below this speed (cm/s) a unit keeps its facing (UHOLInstantOrientationProcessor). */
	constexpr double MinFacingSpeed = 20.0;
	/** Part of the two bodies' radii a walker keeps between them when it looks ahead (D293, D294). */
	float AvoidRadiusScale = 1.1f;
	FAutoConsoleVariableRef CVarAvoidClearance(TEXT("hol.Units.AvoidClearance"), AvoidRadiusScale,
		TEXT("Distance a walker keeps from the bodies it looks ahead for, as a part of the two radii (D293, D294): above 1, a margin"));
	/**
	 * Perf (hol.Units.FastNear): cells of the look-ahead's dense table at most, per unit (beyond, the hashed buckets): a
	 * crowd's box only; units spread over the map (S7's gatherers) would clear a large table each step for nothing.
	 */
	constexpr int64 LookAheadDenseCellsPerUnit = 8;
	/** Perf (2026-10-24): the avoidance's near bodies found by the cheapest tests first, their terms read once (the same ways). */
	int32 FastNear = 1;
	FAutoConsoleVariableRef CVarFastNear(TEXT("hol.Units.FastNear"), FastNear,
		TEXT("1 (default): the full-speed avoidance's near bodies by the same tests, the cheapest first, and their contact terms read once a step; 0: as before, the same ways"));

	int32 AvoidEnabled = 1;
	FAutoConsoleVariableRef CVarAvoid(TEXT("hol.Units.Avoid"), AvoidEnabled,
		TEXT("1: a walker turns aside from the bodies it would meet within hol.Units.AvoidTime, and pushes only when no way within hol.Units.AvoidMaxAngle is clear (D293); 2: the same, and its average cost logged every 200 steps; 0: off, for A / B measures"));
	int32 DetourMemory = 1;
	FAutoConsoleVariableRef CVarDetourMemory(TEXT("hol.Units.DetourMemory"), DetourMemory,
		TEXT("1: a walker remembers its detour and its part at a crossing from step to step (MV.2); 0: forgotten each step, for A / B measures (the heading flipped every step)"));

	/**
	 * The walkers' heading flips (MV.4.3): a turn of more than TurnFlipDegrees between the last step's velocity and this
	 * one's, both moving at least TurnFlipMinStep cm in the step; counted until hol.Units.TurnReport (no log a step).
	 */
	constexpr double TurnFlipDegrees = 25.;
	constexpr double TurnFlipMinStep = 3.;
	std::atomic<int64> TurnFlips{0};
	std::atomic<int64> TurnWalkerSteps{0};
	FAutoConsoleCommand TurnReportCommand(TEXT("hol.Units.TurnReport"),
		TEXT("Logs the walkers' heading flips over 25 degrees between two steps since the last report, then counts again from zero (MV.4.3)"),
		FConsoleCommandDelegate::CreateLambda([]
		{
			const int64 Flips = TurnFlips.exchange(0);
			const int64 Steps = TurnWalkerSteps.exchange(0);
			UE_LOG(LogHOLSimulation, Display, TEXT("Turn report: %lld flips over %.0f deg in %lld walker-steps"), Flips, TurnFlipDegrees, Steps);
		}));

	/**
	 * The walkers' contacts with still bodies (MV.4.4): a walker out of a fight, at a step, closer (centre to centre) than
	 * the two radii to a body moving less than StillBodySpeed; counted until hol.Units.ContactReport, with the closest gap.
	 */
	constexpr double StillBodySpeed = 10.;
	std::atomic<int64> StillContacts{0};
	std::atomic<int64> StillWalkerSteps{0};
	FCriticalSection StillClosestLock;
	double StillClosestGap = TNumericLimits<double>::Max();
	FVector2D StillClosestAt = FVector2D::ZeroVector;
	FAutoConsoleCommand ContactReportCommand(TEXT("hol.Units.ContactReport"),
		TEXT("Logs the walkers' contacts with still bodies (closer than the two radii) since the last report, with the closest gap, then counts again from zero (MV.4.4)"),
		FConsoleCommandDelegate::CreateLambda([]
		{
			const int64 Contacts = StillContacts.exchange(0);
			const int64 Steps = StillWalkerSteps.exchange(0);
			FScopeLock Lock(&StillClosestLock);
			const double Closest = StillClosestGap == TNumericLimits<double>::Max() ? 9999. : StillClosestGap;
			UE_LOG(LogHOLSimulation, Display, TEXT("Contact report: %lld contacts with still bodies in %lld walker-steps, closest gap %.0f cm at (%.0f, %.0f)"),
				Contacts, Steps, Closest, StillClosestAt.X, StillClosestAt.Y);
			StillClosestGap = TNumericLimits<double>::Max();
			StillClosestAt = FVector2D::ZeroVector;
		}));

	float AvoidTime = 0.5f;
	FAutoConsoleVariableRef CVarAvoidTime(TEXT("hol.Units.AvoidTime"), AvoidTime,
		TEXT("Seconds a walker looks ahead for bodies on its way (D293)"));
	float AvoidStillTime = 1.0f;
	FAutoConsoleVariableRef CVarAvoidStillTime(TEXT("hol.Units.AvoidStillTime"), AvoidStillTime,
		TEXT("Seconds a walker looks ahead for still bodies on its way, at least hol.Units.AvoidTime (MV.4.4: seen at 0.5 s, a wall of still ones was touched before being gone round)"));
	float AvoidStillMaxAngle = 90.f;
	FAutoConsoleVariableRef CVarAvoidStillMaxAngle(TEXT("hol.Units.AvoidStillMaxAngle"), AvoidStillMaxAngle,
		TEXT("Largest turn (degrees) a walker makes along still bodies near it, at least hol.Units.AvoidMaxAngle (MV.4.4)"));
	float AvoidMaxAngle = 60.f;
	FAutoConsoleVariableRef CVarAvoidMaxAngle(TEXT("hol.Units.AvoidMaxAngle"), AvoidMaxAngle,
		TEXT("Largest turn (degrees) a walker makes around a body ahead; beyond, the detour is too long and it pushes its way (D293)"));

	/** J7c.4, loop 19: the ranks behind the front slide along their friends instead of walking into them. */
	/** J7c.4 loop 29: the melee rules' bookkeeping (army middles, lines of contact) skipped or done without maps when it cannot change anything. */
	int32 MeleeRuleSkips = 1;
	FAutoConsoleVariableRef CVarMeleeRuleSkips(TEXT("hol.Simulation.MeleeRuleSkips"), MeleeRuleSkips,
		TEXT("1 (default): the army middles of the rear slide only on a step where a unit may slide, the lines of contact summed per owner without maps (the same results); 0: as before (loop 29)"));
	int32 RearSlide = 1;
	FAutoConsoleVariableRef CVarRearSlide(TEXT("hol.Combat.RearSlide"), RearSlide,
		TEXT("1 (default): a melee walking to fight, off the front, its way barred by a friend ahead, slides along that friend (towards its target's side, else the side with fewer friends: a gap or the line's end), stops gently when both sides are shut, its velocity smoothed; 0: as before"));
	/** RearSlide: a friend this close (cm, edge to edge) and ahead bars the way; an enemy this close makes a unit of the front. */
	constexpr double RearSlideContact = 15.0;
	constexpr double RearSlideFrontGap = 10.0;
	/** RearSlide: reach (cm, from the unit's centre) of the friends counted on each side; the slide's speed (part of the walk's). */
	constexpr double RearSlideSideReach = 400.0;
	constexpr double RearSlideSpeed = 0.7;
	/** RearSlide: the largest change of velocity a step, as a part of the walk's speed (a U-turn over ~7 steps). */
	constexpr double RearSlideMaxChange = 0.3;

	/** J7c.4, loop 20: a rear rank keeps the side it slides to, chosen with an aim, instead of choosing again each step. */
	int32 RearSlideKeep = 1;
	FAutoConsoleVariableRef CVarRearSlideKeep(TEXT("hol.Combat.RearSlideKeep"), RearSlideKeep,
		TEXT("1 (default, with hol.Combat.RearSlide): a sliding melee keeps its side (its target's side, else away from the middle of its army: the nearer end of the line) while it slides; turned only after that side has been shut hol.Combat.RearSlideShutTime s or its target moved clearly to the other side; 0: the side chosen again each step (loop 19)"));
	float RearSlideShutTime = 1.f;
	FAutoConsoleVariableRef CVarRearSlideShutTime(TEXT("hol.Combat.RearSlideShutTime"), RearSlideShutTime,
		TEXT("s (default 1, with hol.Combat.RearSlideKeep): a kept side shut this long is turned"));
	float RearSlideCreep = 0.f;
	FAutoConsoleVariableRef CVarRearSlideCreep(TEXT("hol.Combat.RearSlideCreep"), RearSlideCreep,
		TEXT("Part of the walk's speed (default 0, with hol.Combat.RearSlideKeep): a sliding melee whose side is shut creeps on towards its goal at this speed instead of standing"));
	/** RearSlideKeep: the kept way is forgotten after this long (s) walking on unbarred; the target's side that turns it. */
	constexpr float RearSlideForgetTime = 0.5f;
	constexpr double RearSlideTurnGoalSide = 0.5;
	/** RearSlideKeep: across the line, a unit this close (cm) to its army's middle takes its side by its id. */
	constexpr double RearSlideMiddle = 100.0;
	/** J7c.4 loop 21: a melee held behind its own line goes round its end (read by UHOLCombatProcessor). */
	int32 RearFlank = 1;
	FAutoConsoleVariableRef CVarRearFlank(TEXT("hol.Combat.RearFlank"), RearFlank,
		TEXT("1 (default): a melee of an attack-move held behind its own line of contact for hol.Combat.RearFlankDelay s walks round the line's end on its side (its slide's side, else the nearer end), and takes on a free enemy met there; 0: as before"));
	float RearFlankDelay = 0.5f;
	FAutoConsoleVariableRef CVarRearFlankDelay(TEXT("hol.Combat.RearFlankDelay"), RearFlankDelay,
		TEXT("s (default 0.5, loop 21: 1.5, with hol.Combat.RearFlank): how long a melee is held behind its line before it goes round"));
	/** J7c.4 loop 22: the departures spread by unit id, the side the nearer end, the shift capped, a point behind first. */
	float RearFlankStagger = 0.4f;
	FAutoConsoleVariableRef CVarRearFlankStagger(TEXT("hol.Combat.RearFlankStagger"), RearFlankStagger,
		TEXT("s (default 0.4, with hol.Combat.RearFlank): added to the delay by unit id (id % 8 / 8 of it): not all leave on the same step; 0: as loop 21"));
	int32 RearFlankNearest = 1;
	FAutoConsoleVariableRef CVarRearFlankNearest(TEXT("hol.Combat.RearFlankNearest"), RearFlankNearest,
		TEXT("1 (default, with hol.Combat.RearFlank): the flank's side is always the line's end nearer the unit (along the line's axis); 0: its slide's side first (loop 21)"));
	float RearFlankMaxShift = 1200.f;
	FAutoConsoleVariableRef CVarRearFlankMaxShift(TEXT("hol.Combat.RearFlankMaxShift"), RearFlankMaxShift,
		TEXT("cm (default 1200, with hol.Combat.RearFlank): a unit whose flank's goal lies further along the line does not go round (it waits or slides); 0: no cap (loop 21)"));
	float RearFlankBack = 250.f;
	FAutoConsoleVariableRef CVarRearFlankBack(TEXT("hol.Combat.RearFlankBack"), RearFlankBack,
		TEXT("cm (default 250, with hol.Combat.RearFlank): the flank first walks to a point this far behind the unit's own rank, at the end's level, then round the end; 0: straight to the end (loop 21)"));
	float RearTargetMaxShift = 1200.f;
	FAutoConsoleVariableRef CVarRearTargetMaxShift(TEXT("hol.Combat.RearTargetMaxShift"), RearTargetMaxShift,
		TEXT("cm (default 1200, with hol.Combat.RearFlank and hol.Combat.PreferFreeTarget): a melee out of reach does not take as free an enemy further than this along its line (it counts as full); 0: no cap (loop 21)"));
	/** J7c.4 loop 21 (hol.Combat.RearFlank): each side's line of contact this step (read by UHOLCombatProcessor). */
	TMap<uint8, FHOLFrontLine> FrontLines;

	int32 PathAim = 1;
	FAutoConsoleVariableRef CVarPathAim(TEXT("hol.Units.PathAim"), PathAim,
		TEXT("1: a walker following a path heads at once for the point the path follow moves ahead of it (D308); 0: the steering's direction, which turns at ~50 degrees a second and swings wide at the corners"));
	/** How far beyond the path follow's point a walker aims (cm), along the path (Mass' steering aims ahead the same way). */
	constexpr double PathAimLead = 100.;

	int32 GhostTrips = 1;
	FAutoConsoleVariableRef CVarGhostTrips(TEXT("hol.Units.GhostTrips"), GhostTrips,
		TEXT("1: a gatherer on its round trip passes through every unit, none turns aside for it nor pushes it (D309); 0: bodies as any unit's"));

	/**
	 * Where a walker following its short path heads (D308): Aim, PathDistance along the path, unless the path has a point
	 * between the walker and Aim: then that point, the next one once within Reached (cm) of it. The path follow's point
	 * runs up to ~5 m ahead of the walker (Mass' look-ahead at its speed): round a building's corner, the straight line to
	 * it cut through the building (D310, J6a.1: 1.9 m into the main centre).
	 */
	FVector GetAimOnPath(const FMassNavMeshShortPathFragment& ShortPath, const FVector& Location, const FVector& Aim, const double PathDistance,
		const double Reached)
	{
		if (ShortPath.NumPoints < 2)
		{
			return Aim;
		}
		// The walker's segment: the nearest one, as Mass' path follow finds it.
		uint8 Segment = 0;
		double Nearest = TNumericLimits<double>::Max();
		for (uint8 Index = 0; Index + 1 < ShortPath.NumPoints; ++Index)
		{
			const double DistanceSquared = FMath::PointDistToSegmentSquared(Location, ShortPath.Points[Index].Position, ShortPath.Points[Index + 1].Position);
			if (DistanceSquared < Nearest)
			{
				Nearest = DistanceSquared;
				Segment = Index;
			}
		}
		uint8 Next = Segment + 1;
		while (Next + 1 < ShortPath.NumPoints && FVector::DistSquared2D(Location, ShortPath.Points[Next].Position) <= FMath::Square(Reached))
		{
			++Next;
		}
		return PathDistance > ShortPath.Points[Next].Distance.Get() ? ShortPath.Points[Next].Position : Aim;
	}

	/** A gatherer on its round trip (D309): no look-ahead, no separation, for it or against it. */
	bool IsGhost(const FHOLUnitStateFragment& State)
	{
		return GhostTrips != 0 && State.Activity == EHOLUnitActivity::GatherTrip;
	}

	/** Time spent and steps run since the last log (hol.Units.Avoid 2). */
	double AvoidSeconds = 0.;
	int32 AvoidRuns = 0;

	/** Time (s) to contact with a body ahead (pure rule, tested on its own: HOLAvoid, MV.4). */
	using HOLAvoid::TimeToContact;
}

UHOLStorePreviousTransformProcessor::UHOLStorePreviousTransformProcessor()
	: EntityQuery(*this)
{
	bAutoRegisterWithProcessingPhases = false; // fixed step only (D219)
	ExecutionFlags = static_cast<int32>(EProcessorExecutionFlags::Server | EProcessorExecutionFlags::Standalone); // authoritative simulation (D231)
	ExecutionOrder.ExecuteBefore.Add(UE::Mass::ProcessorGroupNames::Tasks);
}

void UHOLStorePreviousTransformProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FHOLPreviousTransformFragment>(EMassFragmentAccess::ReadWrite);
}

void UHOLStorePreviousTransformProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	EntityQuery.ParallelForEachEntityChunk(Context, [](FMassExecutionContext& Context)
	{
		const TConstArrayView<FTransformFragment> Transforms = Context.GetFragmentView<FTransformFragment>();
		const TArrayView<FHOLPreviousTransformFragment> PreviousTransforms = Context.GetMutableFragmentView<FHOLPreviousTransformFragment>();

		for (FMassExecutionContext::FEntityIterator EntityIt = Context.CreateEntityIterator(); EntityIt; ++EntityIt)
		{
			PreviousTransforms[EntityIt].Transform = Transforms[EntityIt].GetTransform();
		}
	});
}

UHOLFullSpeedProcessor::UHOLFullSpeedProcessor()
	: EntityQuery(*this)
{
	bAutoRegisterWithProcessingPhases = false; // fixed step only (D219)
	ExecutionFlags = static_cast<int32>(EProcessorExecutionFlags::Server | EProcessorExecutionFlags::Standalone); // authoritative simulation (D231)
	// Once the forces are in the desired velocity, before it moves the unit.
	ExecutionOrder.ExecuteAfter.Add(TEXT("MassApplyForceProcessor"));
	ExecutionOrder.ExecuteBefore.Add(TEXT("MassApplyMovementProcessor"));
}

void UHOLFullSpeedProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddRequirement<FMassDesiredMovementFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FMassMoveTargetFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FHOLMoveOrderFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FMassVelocityFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FAgentRadiusFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FHOLUnitStateFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FMassNavMeshShortPathFragment>(EMassFragmentAccess::ReadOnly);
	// ReadWrite for the rear slide's way (J7c.4 loop 20, FHOLCombatFragment::SlideWay).
	EntityQuery.AddRequirement<FHOLCombatFragment>(EMassFragmentAccess::ReadWrite, EMassFragmentPresence::Optional);
	EntityQuery.AddRequirement<FHOLDetourFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddTagRequirement<FHOLLightUnitTag>(EMassFragmentPresence::All);
	EntityQuery.AddConstSharedRequirement<FHOLUnitTypeConstants>(EMassFragmentPresence::Optional);
}

bool UHOLFullSpeedProcessor::BuildLookAhead(FMassExecutionContext& Context)
{
	using namespace UE::HeroesOfLiberty::Simulation;

	Entities.Reset();
	Positions.Reset();
	Velocities.Reset();
	Stills.Reset();
	Radii.Reset();
	FormUpSerials.Reset();
	FormUpKinds.Reset();
	Owners.Reset();
	CampTable = FHOLCampTable::FromWorld(Context.GetWorld());
	MaxRadius = 1.f;
	MaxSpeed = 0.f;
	bool bAnyWalking = false;
	bool bAnySlider = false;
	EntityQuery.ForEachEntityChunk(Context, [this, &bAnyWalking, &bAnySlider](FMassExecutionContext& Context)
	{
		const TConstArrayView<FHOLCombatFragment> Combats = Context.GetFragmentView<FHOLCombatFragment>();
		const TConstArrayView<FTransformFragment> Transforms = Context.GetFragmentView<FTransformFragment>();
		const TConstArrayView<FMassVelocityFragment> UnitVelocities = Context.GetFragmentView<FMassVelocityFragment>();
		const TConstArrayView<FAgentRadiusFragment> AgentRadii = Context.GetFragmentView<FAgentRadiusFragment>();
		const TConstArrayView<FMassMoveTargetFragment> MoveTargets = Context.GetFragmentView<FMassMoveTargetFragment>();
		const TConstArrayView<FHOLUnitStateFragment> States = Context.GetFragmentView<FHOLUnitStateFragment>();
		const TConstArrayView<FHOLMoveOrderFragment> Orders = Context.GetFragmentView<FHOLMoveOrderFragment>();
		for (FMassExecutionContext::FEntityIterator EntityIt = Context.CreateEntityIterator(); EntityIt; ++EntityIt)
		{
			// Nobody turns aside for a gatherer on its round trip (D309).
			if (IsGhost(States[EntityIt]))
			{
				continue;
			}
			const FVector Location = Transforms[EntityIt].GetTransform().GetLocation();
			const FVector2D Velocity(UnitVelocities[EntityIt].Value.X, UnitVelocities[EntityIt].Value.Y);
			Entities.Add(Context.GetEntity(EntityIt));
			Positions.Add(FVector2D(Location.X, Location.Y));
			Velocities.Add(Velocity);
			Stills.Add(MoveTargets[EntityIt].GetCurrentAction() != EMassMovementAction::Move && Velocity.SizeSquared() < FMath::Square(StillBodySpeed));
			Radii.Add(AgentRadii[EntityIt].Radius);
			FormUpSerials.Add(GetPassThroughSerial(Orders[EntityIt]));
			FormUpKinds.Add(GetPassThroughKind(Orders[EntityIt]));
			Owners.Add(States[EntityIt].OwnerPlayer);
			MaxRadius = FMath::Max(MaxRadius, AgentRadii[EntityIt].Radius);
			MaxSpeed = FMath::Max(MaxSpeed, static_cast<float>(Velocity.Size()));
			bAnyWalking |= MoveTargets[EntityIt].GetCurrentAction() == EMassMovementAction::Move;
			// Loop 29: a unit that may slide this step (more than the slide's own test, below: melee, walking, not striking).
			bAnySlider |= !Combats.IsEmpty() && (Combats[EntityIt].bApproaching || Combats[EntityIt].Mode == EHOLCombatMode::AttackMove);
		}
	});
	if (!bAnyWalking)
	{
		return false;
	}
	// The buildings' bodies (EtapeDefini E): a few per player, read again at each step as the keep-out's.
	Footprints.Reset();
	FootprintRadii.Reset();
	if (const UWorld* World = Context.GetWorld())
	{
		for (TActorIterator<AHOLBuilding> It(World); It; ++It)
		{
			const FHOLBuildingTypeRow* Type = It->GetType();
			// A bare foundation is walked over (D312).
			if (!Type || !It->BlocksNavigation())
			{
				continue;
			}
			HOLAvoid::FFootprintBox& Box = Footprints.AddDefaulted_GetRef();
			Box.Centre = FVector2D(It->GetActorLocation());
			Box.HalfExtent = UHOLBuildingSettings::GetBodySize(Type->Footprint) * 0.5;
			FMath::SinCos(&Box.Sin, &Box.Cos, FMath::DegreesToRadians(It->GetActorRotation().Yaw));
			FootprintRadii.Add(Box.HalfExtent.Size());
		}
	}
	// J7c.4 loop 20: the middle of each owner's army (a rear rank slides away from it, towards the nearer end of the line).
	OwnerCentres.Reset();
	if (RearSlide != 0 && RearSlideKeep != 0 && MeleeRuleSkips != 0)
	{
		// Loop 29: only read by a sliding unit (SlideAlongFriends); summed per owner in the units' order, as below.
		if (bAnySlider)
		{
			FVector2D Sums[256];
			int32 Counts[256] = {};
			for (int32 Index = 0; Index < Positions.Num(); ++Index)
			{
				const uint8 Owner = Owners[Index];
				Sums[Owner] = Counts[Owner] == 0 ? Positions[Index] : Sums[Owner] + Positions[Index];
				++Counts[Owner];
			}
			for (int32 Owner = 0; Owner < 256; ++Owner)
			{
				if (Counts[Owner] > 0)
				{
					FVector2D& Centre = OwnerCentres.Add(static_cast<uint8>(Owner), Sums[Owner]);
					Centre /= FMath::Max(1, Counts[Owner]); // as below, to the bit
				}
			}
		}
	}
	else if (RearSlide != 0 && RearSlideKeep != 0)
	{
		TMap<uint8, int32> OwnerCounts;
		for (int32 Index = 0; Index < Positions.Num(); ++Index)
		{
			OwnerCentres.FindOrAdd(Owners[Index]) += Positions[Index];
			++OwnerCounts.FindOrAdd(Owners[Index]);
		}
		for (TPair<uint8, FVector2D>& Centre : OwnerCentres)
		{
			Centre.Value /= FMath::Max(1, OwnerCounts[Centre.Key]);
		}
	}
	CellHeads.Init(INDEX_NONE, LookAheadBuckets);
	NextInCell.SetNumUninitialized(Positions.Num());
	Cells.SetNumUninitialized(Positions.Num());
	FIntPoint MinCell(MAX_int32, MAX_int32);
	FIntPoint MaxCell(MIN_int32, MIN_int32);
	for (int32 Index = 0; Index < Positions.Num(); ++Index)
	{
		Cells[Index] = FIntPoint(FMath::FloorToInt32(Positions[Index].X / LookAheadCellSize), FMath::FloorToInt32(Positions[Index].Y / LookAheadCellSize));
		int32& Head = CellHeads[LookAheadBucketOf(Cells[Index])];
		NextInCell[Index] = Head;
		Head = Index;
		MinCell = FIntPoint(FMath::Min(MinCell.X, Cells[Index].X), FMath::Min(MinCell.Y, Cells[Index].Y));
		MaxCell = FIntPoint(FMath::Max(MaxCell.X, Cells[Index].X), FMath::Max(MaxCell.Y, Cells[Index].Y));
	}
	// Perf (hol.Units.FastNear): the dense table, as the separation's (J7c.4): counted per cell, each range filled from its
	// end with the units in ascending order, so read from its start in descending order, as a bucket's chain.
	const int64 Width = Positions.Num() > 0 ? static_cast<int64>(MaxCell.X) - MinCell.X + 1 : 0;
	const int64 Height = Positions.Num() > 0 ? static_cast<int64>(MaxCell.Y) - MinCell.Y + 1 : 0;
	bLookAheadDense = FastNear != 0 && Positions.Num() > 0 && Width * Height <= LookAheadDenseCellsPerUnit * Positions.Num();
	if (bLookAheadDense)
	{
		LookAheadMin = MinCell;
		LookAheadWidth = static_cast<int32>(Width);
		LookAheadHeight = static_cast<int32>(Height);
		const int32 CellCount = LookAheadWidth * LookAheadHeight;
		LookAheadStarts.Init(0, CellCount + 1);
		for (int32 Index = 0; Index < Positions.Num(); ++Index)
		{
			++LookAheadStarts[(Cells[Index].Y - MinCell.Y) * LookAheadWidth + (Cells[Index].X - MinCell.X) + 1];
		}
		for (int32 Cell = 0; Cell < CellCount; ++Cell)
		{
			LookAheadStarts[Cell + 1] += LookAheadStarts[Cell];
		}
		LookAheadBodies.SetNumUninitialized(Positions.Num());
		for (int32 Index = 0; Index < Positions.Num(); ++Index)
		{
			FLookAheadBody& Body = LookAheadBodies[--LookAheadStarts[(Cells[Index].Y - MinCell.Y) * LookAheadWidth + (Cells[Index].X - MinCell.X) + 1]];
			Body.Position = Positions[Index];
			Body.Radius = Radii[Index];
			Body.Index = Index;
		}
		for (int32 Cell = 0; Cell < CellCount; ++Cell)
		{
			LookAheadStarts[Cell] = LookAheadStarts[Cell + 1];
		}
		LookAheadStarts[CellCount] = Positions.Num();
	}
	return true;
}

int32 UHOLFullSpeedProcessor::LookAheadBucketOf(const FIntPoint& Cell)
{
	return static_cast<int32>((static_cast<uint32>(Cell.X) * 73856093u ^ static_cast<uint32>(Cell.Y) * 19349663u) & (UE::HeroesOfLiberty::Simulation::LookAheadBuckets - 1));
}

FVector UHOLFullSpeedProcessor::AvoidBodies(const FMassEntityHandle Entity, const FVector& Location, const float Radius, const FVector& Direction, const float Speed, const double Horizon,
	const double StillHorizonIn, const uint32 FormUpSerial, const uint8 FormUpKind, const uint8 Owner, FHOLDetourFragment& Detour, const bool bAroundEnemies, const FVector2D* Goal) const
{
	using namespace UE::HeroesOfLiberty::Simulation;

	// A still body is seen further (MV.4.4: the walkers touched a wall of still ones before going round it).
	const double StillHorizon = FMath::Max(Horizon, StillHorizonIn);
	const auto IsStill = [this](const int32 Other) { return Stills[Other]; };
	// The bodies that could be met within the horizon, whatever way this one takes: in the reach of both speeds, and not
	// behind it (no turn reaches them; one catching up from behind is its own to avoid).
	const FVector2D Position(Location.X, Location.Y);
	const FVector2D Ahead(Direction.X, Direction.Y);
	// Only the cells the square around that reach overlaps (D302: 9 to 16 instead of 25 at 3 m/s), in the same order.
	const double Reach = FMath::Max((Speed + MaxSpeed) * Horizon, Speed * StillHorizon) + (Radius + MaxRadius) * FMath::Max(1.f, AvoidRadiusScale);
	const FIntPoint MinCell(FMath::FloorToInt32((Position.X - Reach) / LookAheadCellSize), FMath::FloorToInt32((Position.Y - Reach) / LookAheadCellSize));
	const FIntPoint MaxCell(FMath::FloorToInt32((Position.X + Reach) / LookAheadCellSize), FMath::FloorToInt32((Position.Y + Reach) / LookAheadCellSize));
	TArray<int32, TInlineAllocator<192>> Near; // perf: a dense crowd keeps up to ~150 (no heap allocation in the threads)
	// Perf (hol.Units.FastNear): the same tests, all of them pure filters, the cheapest first: a body further than Reach
	// (which bounds every OtherReach below, the 1 cm covering MaxSpeed's float rounding), then one behind, is out before the
	// others; the same bodies kept, in the same order.
	const bool bFastNear = FastNear != 0 && AvoidRadiusScale >= 0.f;
	const double PrefilterSquared = FMath::Square(Reach + 1.0);
	// A whole cell (widened by 1 cm for the rounding of its units' cells) beyond the prefilter, or behind by more than any
	// contact distance: none of its units would be kept.
	const double BehindLimit = (Radius + MaxRadius) * AvoidRadiusScale + 1.0;
	for (int32 Y = MinCell.Y; Y <= MaxCell.Y; ++Y)
	{
		for (int32 X = MinCell.X; X <= MaxCell.X; ++X)
		{
			const FIntPoint Cell(X, Y);
			if (bFastNear)
			{
				const double X0 = X * LookAheadCellSize - 1.0 - Position.X;
				const double X1 = (X + 1) * LookAheadCellSize + 1.0 - Position.X;
				const double Y0 = Y * LookAheadCellSize - 1.0 - Position.Y;
				const double Y1 = (Y + 1) * LookAheadCellSize + 1.0 - Position.Y;
				const double NearX = FMath::Clamp(0.0, X0, X1);
				const double NearY = FMath::Clamp(0.0, Y0, Y1);
				const double FarAhead = (Ahead.X > 0. ? X1 : X0) * Ahead.X + (Ahead.Y > 0. ? Y1 : Y0) * Ahead.Y;
				if (NearX * NearX + NearY * NearY >= PrefilterSquared || FarAhead < -BehindLimit)
				{
					continue;
				}
			}
			// The fast tests on one body, from its position and radius (the same values as Positions and Radii hold).
			const auto TestNear = [&](const int32 Other, const FVector2D& OtherPosition, const float OtherRadius)
			{
				const FVector2D ToOther = OtherPosition - Position;
				const double DistanceSquared = ToOther.SizeSquared();
				if (DistanceSquared >= PrefilterSquared)
				{
					return;
				}
				const double ContactDistance = (Radius + OtherRadius) * AvoidRadiusScale;
				if (!(FVector2D::DotProduct(ToOther, Ahead) > -ContactDistance) || Entities[Other] == Entity
					|| CanPassThrough(FormUpSerial, FormUpKind, FormUpSerials[Other], FormUpKinds[Other]))
				{
					return;
				}
				if (Goal && IsStill(Other) && FVector2D::DistSquared(*Goal, OtherPosition) < FMath::Square(ContactDistance))
				{
					return;
				}
				const double OtherReach = (IsStill(Other) ? FMath::Max(Speed * Horizon, Speed * StillHorizon) : (Speed + Velocities[Other].Size()) * Horizon) + ContactDistance;
				if (DistanceSquared < FMath::Square(OtherReach))
				{
					Near.Add(Other);
				}
			};
			// The dense table's range of the cell (none out of the box), its bodies side by side, in the bucket's order.
			if (bFastNear && bLookAheadDense)
			{
				const int32 DenseX = X - LookAheadMin.X;
				const int32 DenseY = Y - LookAheadMin.Y;
				if (DenseX < 0 || DenseX >= LookAheadWidth || DenseY < 0 || DenseY >= LookAheadHeight)
				{
					continue;
				}
				const int32 SlotEnd = LookAheadStarts[DenseY * LookAheadWidth + DenseX + 1];
				for (int32 Slot = LookAheadStarts[DenseY * LookAheadWidth + DenseX]; Slot < SlotEnd; ++Slot)
				{
					const FLookAheadBody& Body = LookAheadBodies[Slot];
					TestNear(Body.Index, Body.Position, Body.Radius);
				}
				continue;
			}
			for (int32 Other = CellHeads[LookAheadBucketOf(Cell)]; Other != INDEX_NONE; Other = NextInCell[Other])
			{
				if (bFastNear)
				{
					if (Cells[Other] == Cell)
					{
						TestNear(Other, Positions[Other], Radii[Other]);
					}
					continue;
				}
				if (Cells[Other] != Cell || Entities[Other] == Entity || CanPassThrough(FormUpSerial, FormUpKind, FormUpSerials[Other], FormUpKinds[Other]))
				{
					continue;
				}
				const FVector2D ToOther = Positions[Other] - Position;
				const double ContactDistance = (Radius + Radii[Other]) * AvoidRadiusScale;
				// A still body touching its goal is no wall (MV.4.5.3: a rank of a held line walking back to its place, just
				// behind a still friend of the rank ahead, saw it as a wall at 1 s and stopped or went round, 1.5 m off its
				// place): it is to stand beside it, the separation keeping them apart.
				if (Goal && IsStill(Other) && FVector2D::DistSquared(*Goal, Positions[Other]) < FMath::Square(ContactDistance))
				{
					continue;
				}
				const double OtherReach = (IsStill(Other) ? FMath::Max(Speed * Horizon, Speed * StillHorizon) : (Speed + Velocities[Other].Size()) * Horizon) + ContactDistance;
				if (FVector2D::DotProduct(ToOther, Ahead) > -ContactDistance && ToOther.SizeSquared() < FMath::Square(OtherReach))
				{
					Near.Add(Other);
				}
			}
		}
	}
	// Perf (hol.Units.FastNear): each near body's terms of TimeToContact, read once (the same values at every way tried).
	TArray<FVector2D, TInlineAllocator<192>> NearOffsets;
	TArray<FVector2D, TInlineAllocator<192>> NearVelocities;
	TArray<double, TInlineAllocator<192>> NearContacts;
	TArray<bool, TInlineAllocator<192>> NearPressed;
	TArray<bool, TInlineAllocator<192>> NearStills;
	if (bFastNear)
	{
		NearOffsets.SetNumUninitialized(Near.Num());
		NearVelocities.SetNumUninitialized(Near.Num());
		NearContacts.SetNumUninitialized(Near.Num());
		NearPressed.SetNumUninitialized(Near.Num());
		NearStills.SetNumUninitialized(Near.Num());
		for (int32 Slot = 0; Slot < Near.Num(); ++Slot)
		{
			const int32 Other = Near[Slot];
			NearOffsets[Slot] = Positions[Other] - Position;
			NearVelocities[Slot] = Velocities[Other];
			NearContacts[Slot] = (Radius + Radii[Other]) * AvoidRadiusScale;
			NearPressed[Slot] = bAroundEnemies && CampTable.AreHostile(Owners[Other], Owner);
			NearStills[Slot] = Stills[Other];
		}
	}
	// The detour is forgotten once eased back onto the straight course (MV.2).
	const auto Forget = [&Detour]
	{
		Detour.Blocker.Reset();
		Detour.Side = 0;
		Detour.Turn = 0;
		Detour.ClearSteps = 0;
	};
	// A / B (hol.Units.DetourMemory 0): every step from the straight course, as before MV.2.
	if (DetourMemory == 0)
	{
		Forget();
		Detour.Crosser.Reset();
	}
	// The part at a crossing is kept while that walker is near (MV.2).
	if (!Detour.Crosser.IsSet() || !Near.ContainsByPredicate([this, &Detour](const int32 Other) { return Entities[Other] == Detour.Crosser; }))
	{
		Detour.Crosser.Reset();
		Detour.CrossRole = 0;
		Detour.CrossSide = 0;
	}
	if (Near.IsEmpty() && Detour.Turn == 0)
	{
		return Direction;
	}
	// One keeping its course at a crossing does not turn from the other but for a contact very close, as a safety.
	const double KeepCourseHorizon = HOLAvoid::KeepCourseSafetyScale * Horizon;
	const auto ContactLimit = [this, &Detour, Horizon, StillHorizon, KeepCourseHorizon, &IsStill](const int32 Other)
	{
		return Detour.CrossRole == static_cast<uint8>(HOLAvoid::ECrossingRole::KeepCourse) && Entities[Other] == Detour.Crosser ? KeepCourseHorizon
			: IsStill(Other) ? StillHorizon : Horizon;
	};
	// The same limit by the near body's slot (its stillness read once).
	const auto SlotLimit = [this, &Detour, &Near, &NearStills, Horizon, StillHorizon, KeepCourseHorizon](const int32 Slot)
	{
		return Detour.CrossRole == static_cast<uint8>(HOLAvoid::ECrossingRole::KeepCourse) && Entities[Near[Slot]] == Detour.Crosser ? KeepCourseHorizon
			: NearStills[Slot] ? StillHorizon : Horizon;
	};

	// The first body met going this way, and when.
	const auto FirstContact = [this, &Near, &Position, Radius, Speed, &ContactLimit, &SlotLimit, Owner, bAroundEnemies, bFastNear, &NearOffsets, &NearVelocities, &NearContacts, &NearPressed](const FVector2D& Way, int32& OutOther)
	{
		double First = TNumericLimits<double>::Max();
		OutOther = INDEX_NONE;
		if (bFastNear)
		{
			const FVector2D Own = Way * Speed;
			for (int32 Slot = 0; Slot < Near.Num(); ++Slot)
			{
				const double Time = TimeToContact(NearOffsets[Slot], Own - NearVelocities[Slot], NearContacts[Slot], NearPressed[Slot]);
				if (Time < First && Time < SlotLimit(Slot))
				{
					First = Time;
					OutOther = Near[Slot];
				}
			}
			return First;
		}
		for (const int32 Other : Near)
		{
			const double Time = TimeToContact(Positions[Other] - Position, Way * Speed - Velocities[Other], (Radius + Radii[Other]) * AvoidRadiusScale,
				bAroundEnemies && CampTable.AreHostile(Owners[Other], Owner));
			if (Time < First && Time < ContactLimit(Other))
			{
				First = Time;
				OutOther = Other;
			}
		}
		return First;
	};
	// Whether nothing is met going this way within the horizon: the first contact found is enough (D302).
	const auto IsClear = [this, &Near, &Position, Radius, Speed, &ContactLimit, &SlotLimit, Owner, bAroundEnemies, bFastNear, &NearOffsets, &NearVelocities, &NearContacts, &NearPressed](const FVector2D& Way)
	{
		if (bFastNear)
		{
			const FVector2D Own = Way * Speed;
			for (int32 Slot = 0; Slot < Near.Num(); ++Slot)
			{
				if (TimeToContact(NearOffsets[Slot], Own - NearVelocities[Slot], NearContacts[Slot], NearPressed[Slot]) < SlotLimit(Slot))
				{
					return false;
				}
			}
			return true;
		}
		for (const int32 Other : Near)
		{
			if (TimeToContact(Positions[Other] - Position, Way * Speed - Velocities[Other], (Radius + Radii[Other]) * AvoidRadiusScale,
				bAroundEnemies && CampTable.AreHostile(Owners[Other], Owner)) < ContactLimit(Other))
			{
				return false;
			}
		}
		return true;
	};
	// A turn into a building's body within the still horizon is no way (EtapeDefini E: the avoidance did not see them, a
	// follower held its remembered turn into a house's wall, MV.2, and a walker turned wide from still builders into the
	// site's wall, MV.4; the keep-out put them back). The straight course is the path's, along the navmesh.
	const double WallDistance = Speed * StillHorizon;
	TArray<int32, TInlineAllocator<8>> Walls;
	for (int32 Index = 0; Index < Footprints.Num(); ++Index)
	{
		if (FVector2D::DistSquared(Footprints[Index].Centre, Position) < FMath::Square(FootprintRadii[Index] + Radius + WallDistance))
		{
			Walls.Add(Index);
		}
	}
	const auto IntoWall = [this, &Walls, &Position, Radius, WallDistance](const FVector2D& Way)
	{
		return Walls.ContainsByPredicate([this, &Position, Radius, WallDistance, &Way](const int32 Index)
		{
			return HOLAvoid::LeadsIntoFootprint(Footprints[Index], Radius, Position, Way, WallDistance);
		});
	};
	int32 Blocker = INDEX_NONE;
	// A body met within its own limit (a still one's further, MV.4.4): not clear.
	FirstContact(Ahead, Blocker);
	bool bStraightClear = Blocker == INDEX_NONE;
	// At a crossing, the second to the crossing point passes behind the first, which keeps its course (MV.2, user's choice
	// A: both turned away from each other the same way and ran side by side). Chosen once, at the first contact met on
	// the straight course, and kept while that walker is near; not against one of its own order.
	if (!bStraightClear && Detour.Turn == 0 && Entities[Blocker] != Detour.Crosser && (FormUpSerial == 0u || FormUpSerials[Blocker] != FormUpSerial))
	{
		// None (face to face) is kept too: once both turned, their ways looked like a crossing (MV.2: Men-at-arms face to face).
		const HOLAvoid::FCrossingChoice Choice = HOLAvoid::ChooseCrossingRole(Position, Ahead * Speed, Positions[Blocker], Velocities[Blocker]);
		Detour.Crosser = Entities[Blocker];
		Detour.CrossRole = static_cast<uint8>(Choice.Role);
		Detour.CrossSide = static_cast<int8>(Choice.Side);
		if (Choice.Role == HOLAvoid::ECrossingRole::KeepCourse)
		{
			FirstContact(Ahead, Blocker);
			bStraightClear = Blocker == INDEX_NONE;
		}
	}
	// Its body passed (no longer near and ahead): the detour eases back one turn a step while the way is clear, without
	// waiting for a run of clear steps (MV.2: forgotten at once, the heading jumped back up to 60 degrees in a step).
	if (Detour.Turn > 0 && !Near.ContainsByPredicate([this, &Detour](const int32 Other) { return Entities[Other] == Detour.Blocker; }))
	{
		Detour.ClearSteps = static_cast<uint8>(FMath::Max<int32>(Detour.ClearSteps, HOLAvoid::EaseBackSteps - 1));
	}
	if (bStraightClear && Detour.Turn == 0)
	{
		return Direction;
	}

	// Along a still body up to hol.Units.AvoidStillMaxAngle (MV.4.4: at 60 degrees the walkers still closed in on a wide
	// wall of still ones and touched it); the wider turns tried only once none up to the usual largest is clear.
	const bool bStillNear = Near.ContainsByPredicate(IsStill);
	const int32 Turns = FMath::FloorToInt32(FMath::Max3(0.f, AvoidMaxAngle, bStillNear ? AvoidStillMaxAngle : 0.f) / AvoidAngleStep);
	const auto WayOf = [&Ahead](const double Hand, const int32 Turn) { return Ahead.GetRotated(Hand * Turn * AvoidAngleStep); };
	if (Detour.Turn > 0)
	{
		// Without memory, each step started again from the straight course: both walkers turned, the way looked clear,
		// both went straight and met again, the heading flipping every 2 steps (MV.2). On the remembered hand from the
		// remembered turn: one turn back towards the place (the last one onto the straight course) after a run of clear
		// steps, else held, else wider.
		const double Hand = Detour.Side;
		const int32 Turn = HOLAvoid::NextDetourTurn(Detour.Turn, Turns, Detour.ClearSteps, [&WayOf, &IsClear, &IntoWall, Hand](const int32 Each)
			{
				const FVector2D Way = WayOf(Hand, Each);
				return IsClear(Way) && (Each == 0 || !IntoWall(Way));
			});
		// No turn clear up to the largest: on its hand, one turn a step back towards its place, pushing (MV.2: forgetting
		// it flipped the hand, the heading jumping up to 80 degrees in a step at a crossing; held, it circled its slot).
		// An enemy is not pushed (D293): against one, one turn a step wider up to the largest, held along it (MV.4: eased
		// back, the Paysans walked straight into a wall of enemies and stuck in it).
		int32 EnemyAhead = INDEX_NONE;
		const bool bHoldAlongEnemy = Turn == INDEX_NONE && bAroundEnemies && (FirstContact(WayOf(Hand, Turns), EnemyAhead), EnemyAhead != INDEX_NONE)
			&& CampTable.AreHostile(Owners[EnemyAhead], Owner);
		Detour.Turn = static_cast<uint8>(Turn != INDEX_NONE ? Turn : HOLAvoid::TurnWithoutClearWay(Detour.Turn, Turns, bHoldAlongEnemy));
		if (Detour.Turn == 0)
		{
			Forget();
			return Direction;
		}
		const FVector2D Way = WayOf(Hand, Detour.Turn);
		return FVector(Way.X, Way.Y, 0.);
	}

	// Around it on the side away from it, face to face on its own right (two walkers face to face take opposite sides
	// and pass each other, MV.2), the smallest turn first; a turn beyond the largest is too long a detour: on, pushing.
	// Passing behind one at a crossing: on the side it comes from, every turn on that hand before the other hand.
	const FVector2D ToBlocker = Positions[Blocker] - Position;
	const bool bPassBehind = Entities[Blocker] == Detour.Crosser && Detour.CrossRole == static_cast<uint8>(HOLAvoid::ECrossingRole::PassBehind);
	const double Side = bPassBehind ? static_cast<double>(Detour.CrossSide) : HOLAvoid::ChooseDetourSide(Ahead, ToBlocker, (Radius + Radii[Blocker]) * AvoidRadiusScale);
	for (int32 Index = 0; Index < 2 * Turns; ++Index)
	{
		const int32 Turn = bPassBehind ? Index % Turns + 1 : Index / 2 + 1;
		const double Hand = bPassBehind ? (Index < Turns ? Side : -Side) : (Index % 2 == 0 ? Side : -Side);
		{
			const FVector2D Way = WayOf(Hand, Turn);
			if (IsClear(Way) && !IntoWall(Way))
			{
				// Not remembered around one of its own order (a member of its block, one forming up): its file pulls it
				// back at the next step (MV.2: remembered, members drifted up to 2.4 m off their files on the march).
				if (FormUpSerial == 0u || FormUpSerials[Blocker] != FormUpSerial)
				{
					Detour.Blocker = Entities[Blocker];
					Detour.Side = static_cast<int8>(Hand);
					Detour.Turn = static_cast<uint8>(Turn);
				}
				return FVector(Way.X, Way.Y, 0.);
			}
		}
	}
	// No way clear against an enemy (it is not pushed, D293): along it at the largest turn on its side, remembered (MV.4).
	if (bAroundEnemies && Turns > 0 && CampTable.AreHostile(Owners[Blocker], Owner) && !bPassBehind)
	{
		Detour.Blocker = Entities[Blocker];
		Detour.Side = static_cast<int8>(Side);
		Detour.Turn = static_cast<uint8>(Turns);
		const FVector2D Way = WayOf(Side, Turns);
		return FVector(Way.X, Way.Y, 0.);
	}
	return Direction;
}

bool UHOLFullSpeedProcessor::SlideAlongFriends(const FMassEntityHandle Entity, const FVector2D& Position, const float Radius, const FVector2D& Goal, const FVector2D& Wanted,
	const float Speed, const FVector2D& LastVelocity, const uint8 Owner, const float DeltaTime, FHOLCombatFragment& Combat, FVector2D& OutVelocity, bool& bOutSliding) const
{
	using namespace UE::HeroesOfLiberty::Simulation;

	bOutSliding = false;
	const FVector2D Ahead = Wanted.GetSafeNormal();
	const auto Forget = [&Combat]()
	{
		Combat.SlideWay = FVector2f::ZeroVector;
		Combat.SlideShutSeconds = 0.f;
		Combat.SlideFreeSeconds = 0.f;
	};
	if (Ahead.IsZero())
	{
		Forget();
		return false;
	}
	// The bodies around: an enemy at contact makes it of the front (left as before); the friend barring the way, the
	// nearest at contact in the cone ahead (60 degrees each side) not walking away as fast; the friends on each side.
	const double Reach = RearSlideSideReach + MaxRadius;
	const FIntPoint MinCell(FMath::FloorToInt32((Position.X - Reach) / LookAheadCellSize), FMath::FloorToInt32((Position.Y - Reach) / LookAheadCellSize));
	const FIntPoint MaxCell(FMath::FloorToInt32((Position.X + Reach) / LookAheadCellSize), FMath::FloorToInt32((Position.Y + Reach) / LookAheadCellSize));
	TArray<int32, TInlineAllocator<64>> Touching;
	int32 Blocker = INDEX_NONE;
	double BlockerEdge = 0.;
	// Perf (hol.Units.FastNear): only a body within the contact gaps counts here (an enemy within RearSlideFrontGap, a
	// friend within RearSlideContact); the cells and the bodies further than both (and 1 cm) are skipped, the rest met in
	// the same order.
	const bool bFastNear = FastNear != 0;
	const double ContactReach = FMath::Max(RearSlideContact, RearSlideFrontGap) + Radius + MaxRadius + 1.0;
	for (int32 Y = MinCell.Y; Y <= MaxCell.Y; ++Y)
	{
		for (int32 X = MinCell.X; X <= MaxCell.X; ++X)
		{
			const FIntPoint Cell(X, Y);
			if (bFastNear)
			{
				const double NearX = FMath::Clamp(Position.X, X * LookAheadCellSize - 1.0, (X + 1) * LookAheadCellSize + 1.0) - Position.X;
				const double NearY = FMath::Clamp(Position.Y, Y * LookAheadCellSize - 1.0, (Y + 1) * LookAheadCellSize + 1.0) - Position.Y;
				if (NearX * NearX + NearY * NearY > FMath::Square(ContactReach))
				{
					continue;
				}
			}
			for (int32 Other = CellHeads[LookAheadBucketOf(Cell)]; Other != INDEX_NONE; Other = NextInCell[Other])
			{
				if (Cells[Other] != Cell || Entities[Other] == Entity)
				{
					continue;
				}
				if (bFastNear && FVector2D::DistSquared(Positions[Other], Position) > FMath::Square(ContactReach))
				{
					continue;
				}
				const FVector2D ToOther = Positions[Other] - Position;
				const double Distance = ToOther.Size();
				const double Edge = Distance - Radius - Radii[Other];
				if (CampTable.AreHostile(Owners[Other], Owner))
				{
					if (Edge <= RearSlideFrontGap)
					{
						Forget();
						return false;
					}
					continue;
				}
				if (Edge > RearSlideContact || Distance <= UE_KINDA_SMALL_NUMBER)
				{
					continue;
				}
				Touching.Add(Other);
				const FVector2D Way = ToOther / Distance;
				if (FVector2D::DotProduct(Ahead, Way) > 0.5 && FVector2D::DotProduct(Velocities[Other], Ahead) < 0.5 * Speed
					&& (Blocker == INDEX_NONE || Edge < BlockerEdge || (Edge == BlockerEdge && Entities[Other].Index < Entities[Blocker].Index)))
				{
					Blocker = Other;
					BlockerEdge = Edge;
				}
			}
		}
	}
	FVector2D Target = Wanted;
	const bool bKeep = RearSlideKeep != 0;
	if (Blocker == INDEX_NONE)
	{
		// Walking on unbarred (into a gap): the kept way is forgotten after a while, not at once (a step between two
		// friends barring the way would choose again).
		Combat.SlideFreeSeconds += DeltaTime;
		if (!bKeep || Combat.SlideFreeSeconds >= RearSlideForgetTime)
		{
			Forget();
		}
	}
	else
	{
		Combat.SlideFreeSeconds = 0.f;
		const FVector2D Normal = (Positions[Blocker] - Position).GetSafeNormal();
		const FVector2D Left(-Normal.Y, Normal.X);
		// The side: the target's when it is clearly to one side; else the side with fewer friends within reach (a gap or the
		// line's end, where the enemy is not surrounded: the rear ranks wrap round its flank); else the way it already goes.
		// Loop 20 (RearSlideKeep): the side kept while it slides; turned only by its target gone clearly the other way (or
		// that side shut a while, below); a new one: the target's side, else away from its army's middle (the nearer end of
		// the line: the neighbours all take the same way, a wave, and wrap round the flank).
		double Hand = 0.;
		const double GoalSide = FVector2D::DotProduct(Goal, Left);
		const double KeptSide = FVector2D::DotProduct(FVector2D(Combat.SlideWay.X, Combat.SlideWay.Y), Left);
		if (bKeep && FMath::Abs(KeptSide) > 0.2)
		{
			Hand = KeptSide > 0. ? 1. : -1.;
			if (GoalSide * Hand < -RearSlideTurnGoalSide)
			{
				Hand = -Hand;
				Combat.SlideShutSeconds = 0.f;
			}
		}
		else if (FMath::Abs(GoalSide) > 0.25)
		{
			Hand = GoalSide > 0. ? 1. : -1.;
		}
		else if (bKeep)
		{
			const FVector2D* const Centre = OwnerCentres.Find(Owner);
			const double Across = Centre ? FVector2D::DotProduct(Position - *Centre, Left) : 0.;
			Hand = Across != 0. ? (Across > 0. ? 1. : -1.) : ((Entity.Index & 1) ? 1. : -1.);
		}
		else
		{
			int32 Friends[2] = {};
			for (int32 Y = MinCell.Y; Y <= MaxCell.Y; ++Y)
			{
				for (int32 X = MinCell.X; X <= MaxCell.X; ++X)
				{
					const FIntPoint Cell(X, Y);
					for (int32 Other = CellHeads[LookAheadBucketOf(Cell)]; Other != INDEX_NONE; Other = NextInCell[Other])
					{
						if (Cells[Other] != Cell || Entities[Other] == Entity || CampTable.AreHostile(Owners[Other], Owner))
						{
							continue;
						}
						const FVector2D ToOther = Positions[Other] - Position;
						if (ToOther.SizeSquared() <= FMath::Square(RearSlideSideReach))
						{
							const double Across = FVector2D::DotProduct(ToOther, Left);
							if (FMath::Abs(Across) > Radius)
							{
								++Friends[Across > 0. ? 0 : 1];
							}
						}
					}
				}
			}
			const double LastSide = FVector2D::DotProduct(LastVelocity, Left);
			Hand = Friends[0] != Friends[1] ? (Friends[0] < Friends[1] ? 1. : -1.)
				: FMath::Abs(LastSide) > 1. ? (LastSide > 0. ? 1. : -1.) : ((Entity.Index & 1) ? 1. : -1.);
		}
		// A side shut by a body at contact that way: the other one; both shut, it stops (no push).
		const auto IsShut = [&](const FVector2D& Way)
		{
			for (const int32 Other : Touching)
			{
				if (Other != Blocker && FVector2D::DotProduct((Positions[Other] - Position).GetSafeNormal(), Way) > 0.7)
				{
					return true;
				}
			}
			return false;
		};
		Target = FVector2D::ZeroVector;
		if (bKeep)
		{
			// Its side shut: it waits (or creeps on towards its goal), and turns only once shut a while.
			const FVector2D Way = Left * Hand;
			if (!IsShut(Way))
			{
				Target = Way * (Speed * RearSlideSpeed);
				Combat.SlideShutSeconds = 0.f;
			}
			else
			{
				Target = Ahead * (Speed * FMath::Clamp(RearSlideCreep, 0.f, 1.f));
				// Turned only towards an open side: both shut, it keeps its side and waits (no turn each second).
				Combat.SlideShutSeconds += DeltaTime;
				if (Combat.SlideShutSeconds >= RearSlideShutTime && !IsShut(-Way))
				{
					Hand = -Hand;
					Combat.SlideShutSeconds = 0.f;
				}
			}
			Combat.SlideWay = FVector2f(Left * Hand);
		}
		else
		{
			Combat.SlideWay = FVector2f::ZeroVector;
			for (const double Side : { Hand, -Hand })
			{
				const FVector2D Way = Left * Side;
				if (!IsShut(Way))
				{
					Target = Way * (Speed * RearSlideSpeed);
					Combat.SlideWay = FVector2f(Way);
					break;
				}
			}
		}
		bOutSliding = true;
	}
	// Smoothed: the last velocity, its part into the friend barring the way dropped, moves towards the wanted one by a
	// part of the speed a step.
	FVector2D Last = LastVelocity;
	if (Blocker != INDEX_NONE)
	{
		const FVector2D Normal = (Positions[Blocker] - Position).GetSafeNormal();
		Last -= Normal * FMath::Max(0., FVector2D::DotProduct(Last, Normal));
	}
	FVector2D Change = Target - Last;
	const double MaxChange = Speed * RearSlideMaxChange;
	if (Change.SizeSquared() > FMath::Square(MaxChange))
	{
		Change = Change.GetSafeNormal() * MaxChange;
	}
	OutVelocity = Last + Change;
	return true;
}

void UHOLFullSpeedProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	using namespace UE::HeroesOfLiberty::Simulation;

	const double StartSeconds = FPlatformTime::Seconds();
	ON_SCOPE_EXIT
	{
		if (AvoidEnabled == 2)
		{
			AvoidSeconds += FPlatformTime::Seconds() - StartSeconds;
			if (++AvoidRuns == 200)
			{
				UE_LOG(LogHOLSimulation, Display, TEXT("Avoid: %.3f ms per step on average (%d units)"), AvoidSeconds * 1000. / AvoidRuns, Positions.Num());
				AvoidSeconds = 0.;
				AvoidRuns = 0;
			}
		}
	};
	const bool bLookAhead = AvoidEnabled != 0 && BuildLookAhead(Context);
	const double LookAheadTime = FMath::Max(0.f, AvoidTime);
	const double StillLookAheadTime = FMath::Max<double>(LookAheadTime, AvoidStillTime);

	// Chunks taken by the threads as they come free: a chunk of a dense crowd costs more (D302).
	EntityQuery.ParallelForEachEntityChunk(Context, [this, bLookAhead, LookAheadTime, StillLookAheadTime](FMassExecutionContext& Context)
	{
		// As UMassApplyMovementProcessor, which moves the unit by this velocity next.
		const float DeltaTime = FMath::Min(0.1f, Context.GetDeltaTimeSeconds());
		const TArrayView<FMassDesiredMovementFragment> Movements = Context.GetMutableFragmentView<FMassDesiredMovementFragment>();
		const TConstArrayView<FMassMoveTargetFragment> MoveTargets = Context.GetFragmentView<FMassMoveTargetFragment>();
		const TConstArrayView<FTransformFragment> Transforms = Context.GetFragmentView<FTransformFragment>();
		const TConstArrayView<FHOLMoveOrderFragment> Orders = Context.GetFragmentView<FHOLMoveOrderFragment>();
		const TConstArrayView<FAgentRadiusFragment> AgentRadii = Context.GetFragmentView<FAgentRadiusFragment>();
		const TConstArrayView<FHOLUnitStateFragment> States = Context.GetFragmentView<FHOLUnitStateFragment>();
		const TConstArrayView<FMassNavMeshShortPathFragment> ShortPaths = Context.GetFragmentView<FMassNavMeshShortPathFragment>();
		const TArrayView<FHOLCombatFragment> Combats = Context.GetMutableFragmentView<FHOLCombatFragment>();
		const TConstArrayView<FMassVelocityFragment> LastVelocities = Context.GetFragmentView<FMassVelocityFragment>();
		const FHOLUnitTypeConstants* const TypeConstants = Context.GetConstSharedFragmentPtr<FHOLUnitTypeConstants>();
		// J7c.4 loop 19 (hol.Combat.RearSlide): a melee only (a ranged unit stops at its range, far behind).
		const bool bMayRearSlide = bLookAhead && RearSlide != 0 && !Combats.IsEmpty()
			&& (!TypeConstants || !HOLCombat::IsRangedAttack(TypeConstants->AttackRange));
		const TArrayView<FHOLDetourFragment> Detours = Context.GetMutableFragmentView<FHOLDetourFragment>();
		// The heading flips of this chunk (MV.4.3), added once at its end.
		int64 ChunkFlips = 0;
		int64 ChunkWalkerSteps = 0;
		const double MinStepSpeed = DeltaTime > 0.f ? TurnFlipMinStep / DeltaTime : TNumericLimits<double>::Max();
		const double FlipCosine = FMath::Cos(FMath::DegreesToRadians(TurnFlipDegrees));
		const auto CountTurn = [&ChunkFlips, &ChunkWalkerSteps, MinStepSpeed, FlipCosine](const FVector& Last, const FVector& Now)
		{
			const FVector2D Before(Last.X, Last.Y);
			const FVector2D After(Now.X, Now.Y);
			if (Before.SizeSquared() >= FMath::Square(MinStepSpeed) && After.SizeSquared() >= FMath::Square(MinStepSpeed))
			{
				++ChunkWalkerSteps;
				ChunkFlips += FVector2D::DotProduct(Before.GetSafeNormal(), After.GetSafeNormal()) < FlipCosine ? 1 : 0;
			}
		};
		// The contacts with still bodies of this chunk (MV.4.4), added once at its end.
		int64 ChunkContacts = 0;
		int64 ChunkStillSteps = 0;
		double ChunkClosest = TNumericLimits<double>::Max();
		FVector2D ChunkClosestAt = FVector2D::ZeroVector;
		const auto CountStillContact = [this, &ChunkContacts, &ChunkStillSteps, &ChunkClosest, &ChunkClosestAt](const FMassEntityHandle Entity, const FVector2D& Position, const float Radius)
		{
			const double Reach = Radius + MaxRadius;
			const FIntPoint MinCell(FMath::FloorToInt32((Position.X - Reach) / LookAheadCellSize), FMath::FloorToInt32((Position.Y - Reach) / LookAheadCellSize));
			const FIntPoint MaxCell(FMath::FloorToInt32((Position.X + Reach) / LookAheadCellSize), FMath::FloorToInt32((Position.Y + Reach) / LookAheadCellSize));
			double Gap = TNumericLimits<double>::Max();
			for (int32 Y = MinCell.Y; Y <= MaxCell.Y; ++Y)
			{
				for (int32 X = MinCell.X; X <= MaxCell.X; ++X)
				{
					const FIntPoint Cell(X, Y);
					for (int32 Other = CellHeads[LookAheadBucketOf(Cell)]; Other != INDEX_NONE; Other = NextInCell[Other])
					{
						if (Cells[Other] == Cell && Entities[Other] != Entity && Stills[Other])
						{
							Gap = FMath::Min(Gap, FVector2D::Distance(Positions[Other], Position) - (Radius + Radii[Other]));
						}
					}
				}
			}
			++ChunkStillSteps;
			ChunkContacts += Gap < 0. ? 1 : 0;
			if (Gap < ChunkClosest)
			{
				ChunkClosest = Gap;
				ChunkClosestAt = Position;
			}
		};
		ON_SCOPE_EXIT
		{
			TurnFlips += ChunkFlips;
			TurnWalkerSteps += ChunkWalkerSteps;
			StillContacts += ChunkContacts;
			StillWalkerSteps += ChunkStillSteps;
			if (ChunkClosest < TNumericLimits<double>::Max())
			{
				FScopeLock Lock(&StillClosestLock);
				if (ChunkClosest < StillClosestGap)
				{
					StillClosestGap = ChunkClosest;
					StillClosestAt = ChunkClosestAt;
				}
			}
		};

		for (FMassExecutionContext::FEntityIterator EntityIt = Context.CreateEntityIterator(); EntityIt; ++EntityIt)
		{
			const FMassMoveTargetFragment& MoveTarget = MoveTargets[EntityIt];
			const float Speed = MoveTarget.DesiredSpeed.Get();
			FVector& Velocity = Movements[EntityIt].DesiredVelocity;
			if (MoveTarget.GetCurrentAction() != EMassMovementAction::Move || Speed <= 0.f || DeltaTime <= 0.f)
			{
				// A standing unit does not move: Mass' standing avoidance would drift it by a few cm/s for seconds. The
				// walkers go around it (moving avoidance).
				Velocity = FVector::ZeroVector;
				if (!Combats.IsEmpty())
				{
					Combats[EntityIt].SlideWay = FVector2f::ZeroVector;
					Combats[EntityIt].SlideShutSeconds = 0.f;
				}
				continue;
			}
			const FVector Location = Transforms[EntityIt].GetTransform().GetLocation();
			const FHOLMoveOrderFragment& Order = Orders[EntityIt];

			const FVector ToSlot(Order.Slot.X - Location.X, Order.Slot.Y - Location.Y, 0.);
			const double SlotDistance = ToSlot.Size();
			const bool bToSlot = Order.Phase == EHOLMovePhase::FinalLeg || Order.Phase == EHOLMovePhase::FormUp || Order.Phase == EHOLMovePhase::Block;
			if (bToSlot && SlotDistance <= Speed * DeltaTime)
			{
				// The last step lands on the slot: the order processor stands the unit there at the next step.
				Velocity = ToSlot / DeltaTime;
				continue;
			}
			FVector Direction;
			if (Order.Phase == EHOLMovePhase::Path && PathAim != 0)
			{
				// Along its path, as straight to its slot below: the steering turned the velocity at ~50 degrees a second, at
				// full speed it swung 5 to 10 m wide of each corner of the path (the user's trial of J5b.2: gatherers going
				// round the centre or out of the wood made huge detours). The path follow's point runs ahead at the unit's
				// speed and waits for it when it falls behind; never past the path's next point (D310), the step's walk from it.
				const FVector Tangent = FVector(MoveTarget.Forward.X, MoveTarget.Forward.Y, 0.).GetSafeNormal();
				const FMassNavMeshShortPathFragment& ShortPath = ShortPaths[EntityIt];
				const FVector Aim = GetAimOnPath(ShortPath, Location, MoveTarget.Center + Tangent * PathAimLead,
					ShortPath.MoveTargetProgressDistance + PathAimLead, Speed * DeltaTime);
				const FVector ToAim(Aim.X - Location.X, Aim.Y - Location.Y, 0.);
				Direction = ToAim.SizeSquared() > 1. ? ToAim.GetSafeNormal() : Tangent.IsZero() ? ToSlot / FMath::Max(SlotDistance, 1.) : Tangent;
			}
			else if (bToSlot)
			{
				// Straight to its slot, a straight way on the navmesh: the steering turns the velocity at ~50 degrees a
				// second (its acceleration), at full speed it circled the slot (D290), and after each turn of the
				// look-ahead it circled the units standing beside the slot without ever coming in (D294).
				Direction = ToSlot / SlotDistance;
			}
			else
			{
				// The steering's direction at full speed; from a standstill it has barely any, so towards the move target.
				FVector Goal = FVector(MoveTarget.Center.X - Location.X, MoveTarget.Center.Y - Location.Y, 0.).GetSafeNormal();
				if (Goal.IsZero())
				{
					Goal = FVector(MoveTarget.Forward.X, MoveTarget.Forward.Y, 0.).GetSafeNormal();
				}
				Direction = FVector(Velocity.X, Velocity.Y, 0.).GetSafeNormal();
				if (Direction.IsZero())
				{
					Direction = Goal;
				}
				else if (FVector::DotProduct(Direction, Goal) < 0.)
				{
					// Never away from the goal at full speed (D292): units pressed together, the avoidance's push sent them
					// running backwards. Sideways only, or on towards the goal.
					const FVector Sideways = (Direction - Goal * FVector::DotProduct(Direction, Goal)).GetSafeNormal();
					Direction = Sideways.IsZero() ? Goal : Sideways;
				}
			}
			// A gatherer on its round trip goes through the others (D309).
			if (bLookAhead && !IsGhost(States[EntityIt]))
			{
				// Around the bodies on its way (D293), those beyond its slot aside: Mass' avoidance runs one step out of 6
				// (D220) and turns too late at full speed, so the walker pushed through them.
				const double Horizon = bToSlot ? FMath::Min(LookAheadTime, SlotDistance / Speed) : LookAheadTime;
				// A still body further (MV.4.4), but none beyond its slot either.
				const double StillHorizon = bToSlot ? FMath::Min(StillLookAheadTime, SlotDistance / Speed) : StillLookAheadTime;
				// A unit forming up for a block march goes through the others of its order forming up in other kinds of rows
				// (two of one kind keep clear of each other, the user's trial of playtest note 1 of J7b); one re-forming at a
				// block's arrival, or a straggler walking into its slot, through all of its order (J7a.4, J7b note 1: CanPassThrough),
				// as the separation lets it.
				// A detour is kept for one order's leg (MV.2).
				FHOLDetourFragment& Detour = Detours[EntityIt];
				if (Detour.OrderSerial != Order.OrderSerial || Detour.Phase != static_cast<uint8>(Order.Phase))
				{
					Detour = FHOLDetourFragment();
					Detour.OrderSerial = Order.OrderSerial;
					Detour.Phase = static_cast<uint8>(Order.Phase);
				}
				const bool bOutOfFight = Combats.IsEmpty() || (!Combats[EntityIt].bAttacking && !Combats[EntityIt].bApproaching && Combats[EntityIt].Mode != EHOLCombatMode::AttackMove);
				const FVector2D Goal(Order.Slot.X, Order.Slot.Y);
				Direction = AvoidBodies(Context.GetEntity(EntityIt), Location, AgentRadii[EntityIt].Radius, Direction, Speed, Horizon, StillHorizon,
					GetPassThroughSerial(Order), GetPassThroughKind(Order), States[EntityIt].OwnerPlayer, Detour, bOutOfFight, Order.Phase != EHOLMovePhase::None ? &Goal : nullptr);
				if (bOutOfFight)
				{
					CountStillContact(Context.GetEntity(EntityIt), FVector2D(Location.X, Location.Y), AgentRadii[EntityIt].Radius);
				}
				// On a block's march, never against it (playtest note 1, loop 20: a unit forming up, going round one of
				// its kind, took a step back): the turn's part against the march dropped, sideways or on.
				if ((Order.Phase == EHOLMovePhase::FormUp || Order.Phase == EHOLMovePhase::Block) && !Order.bReforming && !Order.MarchDirection.IsNearlyZero())
				{
					const FVector March(Order.MarchDirection.X, Order.MarchDirection.Y, 0.);
					const double Along = FVector::DotProduct(Direction, March);
					if (Along < 0.)
					{
						Direction = (Direction - March * Along).GetSafeNormal();
					}
				}
			}
			// J7c.4 loop 19 (hol.Combat.RearSlide): a melee walking to fight (an attack-move or a walk to its target), off the
			// front, never walks into the friend barring its way: it slides along it, or stops, its velocity smoothed.
			if (bMayRearSlide && !IsGhost(States[EntityIt]) && !Combats[EntityIt].bAttacking
				&& (Combats[EntityIt].bApproaching || Combats[EntityIt].Mode == EHOLCombatMode::AttackMove))
			{
				const FVector2D Wanted(Direction.X * Speed, Direction.Y * Speed);
				const FVector2D Goal = FVector2D(MoveTarget.Center.X - Location.X, MoveTarget.Center.Y - Location.Y).GetSafeNormal();
				const FVector2D Last(LastVelocities[EntityIt].Value.X, LastVelocities[EntityIt].Value.Y);
				FVector2D Slid;
				bool bSliding = false;
				if (SlideAlongFriends(Context.GetEntity(EntityIt), FVector2D(Location.X, Location.Y), AgentRadii[EntityIt].Radius, Goal.IsZero() ? Wanted.GetSafeNormal() : Goal,
					Wanted, Speed, Last, States[EntityIt].OwnerPlayer, DeltaTime, Combats[EntityIt], Slid, bSliding))
				{
					Velocity = FVector(Slid.X, Slid.Y, 0.);
					CountTurn(LastVelocities[EntityIt].Value, Velocity);
					continue;
				}
			}
			else if (!Combats.IsEmpty() && !Combats[EntityIt].SlideWay.IsZero())
			{
				// Striking, or off a fight's walk: the slide's way forgotten (J7c.4 loop 20).
				Combats[EntityIt].SlideWay = FVector2f::ZeroVector;
				Combats[EntityIt].SlideShutSeconds = 0.f;
				Combats[EntityIt].SlideFreeSeconds = 0.f;
			}
			Velocity = Direction * Speed;
			CountTurn(LastVelocities[EntityIt].Value, Velocity);
		}
	}, FMassEntityQuery::EParallelExecutionFlags::AutoBalance);
}

namespace UE::HeroesOfLiberty::Simulation
{
	/** Buckets of the separation's cell table (a power of two): a few per unit at 2 000 units, 32 KB to clear each step. */
	constexpr int32 SeparationBuckets = 8192;
	/** Rounds of the separation a step (D292). */
	constexpr int32 SeparationRounds = 2;
	/**
	 * What (cm) a gatherer at its source and a unit standing beside it may come closer than the spots' spacing (D309): the
	 * arrival tolerance of an order (OrderArrivalTolerance), so that two gatherers on their spots are never nudged.
	 */
	constexpr double AtSourceTolerance = 5.;

	int32 SeparationEnabled = 1;
	FAutoConsoleVariableRef CVarSeparation(TEXT("hol.Units.Separation"), SeparationEnabled,
		TEXT("1: no body overlaps another once the units have moved (D292); 2: the same, and its average cost logged every 200 steps; 0: off, for A / B measures"));

	/**
	 * Cells of the separation's dense table at most (J7c.4): beyond, the units are spread too far apart for a table over
	 * their bounding box and the hashed buckets are used (1 M cells: 4 MB to clear a step).
	 */
	constexpr int64 SeparationDenseMaxCells = 1 << 20;

	int32 FastSeparation = 1;
	FAutoConsoleVariableRef CVarFastSeparation(TEXT("hol.Simulation.FastSeparation"), FastSeparation,
		TEXT("1: the separation's cells in a dense table over the units' bounding box, the same pairs met in the same order as in the hashed buckets (J7c.4); 0: the hashed buckets only, for A / B measures"));

	float SeparationSlop = 0.f;
	FAutoConsoleVariableRef CVarSeparationSlop(TEXT("hol.Units.SeparationSlop"), SeparationSlop,
		TEXT("Overlap (cm) of two bodies the separation leaves (D294): only what goes beyond is undone"));

	/** J7c.4, loop 18: the separation never takes a unit through an enemy (the pass probe of loop 17: the jumps came from it). */
	int32 EnemySideGuard = 1;
	FAutoConsoleVariableRef CVarEnemySideGuard(TEXT("hol.Simulation.EnemySideGuard"), EnemySideGuard,
		TEXT("1 (default): two enemies part along their axis at the step's start (before the corrections), by the least move that frees them, and a unit moves at most hol.Simulation.SeparationMaxStep a step; 0: as before"));
	/** With EnemySideGuard: a unit's whole move by the separation in one step at most (cm); below 0: its radius; 0: no cap. */
	float SeparationMaxStep = -1.f;
	FAutoConsoleVariableRef CVarSeparationMaxStep(TEXT("hol.Simulation.SeparationMaxStep"), SeparationMaxStep,
		TEXT("cm (with hol.Simulation.EnemySideGuard): a unit's whole move by the separation in one step at most; -1 (default): its radius; 0: no cap"));

	/** J7c.4, loop 18: the fighting front is a wall for its friends' pushes; the ranks behind press against it. */
	int32 FrontHoldsFriends = 1;
	FAutoConsoleVariableRef CVarFrontHoldsFriends(TEXT("hol.Simulation.FrontHoldsFriends"), FrontHoldsFriends,
		TEXT("1 (default): a unit that strikes, or touches an enemy, is never pushed by a friend (the friend takes the whole overlap); 0: as before"));
	/** Perf (2026-10-24): the front's flags found in parallel (the same flags). */
	int32 ParallelFront = 1;
	FAutoConsoleVariableRef CVarParallelFront(TEXT("hol.Simulation.ParallelFront"), ParallelFront,
		TEXT("1 (default): the separation's front (FrontHoldsFriends) found in parallel; 0: in one pass, the same flags"));
	/** An enemy this close (cm, edge to edge) makes a unit of the front. */
	constexpr double FrontContactGap = 10.0;

	/** J7c.4 loop 30 (the rule since loop 32): the front only where a fight is; a unit brushing an enemy on a walk is not of it. */
	int32 FrontNeedsCombat = 1;
	FAutoConsoleVariableRef CVarFrontNeedsCombat(TEXT("hol.Simulation.FrontNeedsCombat"), FrontNeedsCombat,
		TEXT("1 (default): a unit touching an enemy is of the front only when it or that enemy is concerned by a fight (target, strike, approach, attack-move, flank); no such unit: no front pass at all; 0: any enemy contact makes the front (before loop 30)"));

	/** Time spent and steps run since the last log (hol.Units.Separation 2). */
	double SeparationSeconds = 0.;
	int32 SeparationRuns = 0;
	/** Of that time, the positions gathered into the grid, and the rounds (D302). */
	double SeparationGridSeconds = 0.;
	double SeparationRoundsSeconds = 0.;
}

UHOLUnitSeparationProcessor::UHOLUnitSeparationProcessor()
	: EntityQuery(*this)
{
	bAutoRegisterWithProcessingPhases = false; // fixed step only (D219)
	ExecutionFlags = static_cast<int32>(EProcessorExecutionFlags::Server | EProcessorExecutionFlags::Standalone); // authoritative simulation (D231)
	// Once the units have moved, before the obstacle grid records where they are.
	ExecutionOrder.ExecuteAfter.Add(TEXT("MassApplyMovementProcessor"));
	ExecutionOrder.ExecuteBefore.Add(TEXT("MassNavigationObstacleGridProcessor"));
}

void UHOLUnitSeparationProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FAgentRadiusFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FMassMoveTargetFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FHOLUnitStateFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FHOLGatherFragment>(EMassFragmentAccess::ReadOnly, EMassFragmentPresence::Optional);
	EntityQuery.AddRequirement<FHOLMoveOrderFragment>(EMassFragmentAccess::ReadOnly, EMassFragmentPresence::Optional);
	EntityQuery.AddRequirement<FHOLCombatFragment>(EMassFragmentAccess::ReadOnly, EMassFragmentPresence::Optional);
	EntityQuery.AddTagRequirement<FHOLLightUnitTag>(EMassFragmentPresence::All);
	EntityQuery.AddConstSharedRequirement<FHOLUnitTypeConstants>(EMassFragmentPresence::Optional);
}

void UHOLUnitSeparationProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	using namespace UE::HeroesOfLiberty::Simulation;

	if (SeparationEnabled == 0)
	{
		return;
	}
	const double StartSeconds = FPlatformTime::Seconds();
	ON_SCOPE_EXIT
	{
		if (SeparationEnabled == 2)
		{
			SeparationSeconds += FPlatformTime::Seconds() - StartSeconds;
			if (++SeparationRuns == 200)
			{
				UE_LOG(LogHOLSimulation, Display, TEXT("Separation: %.3f ms per step on average (%d units; grid %.3f, rounds %.3f)"), SeparationSeconds * 1000. / SeparationRuns,
					Positions.Num(), SeparationGridSeconds * 1000. / SeparationRuns, SeparationRoundsSeconds * 1000. / SeparationRuns);
				SeparationSeconds = 0.;
				SeparationGridSeconds = 0.;
				SeparationRoundsSeconds = 0.;
				SeparationRuns = 0;
			}
		}
	};
	Positions.Reset();
	Radii.Reset();
	Walking.Reset();
	Ghosts.Reset();
	FormUpSerials.Reset();
	FormUpKinds.Reset();
	Moving.Reset();
	Owners.Reset();
	CampTable = FHOLCampTable::FromWorld(Context.GetWorld());
	AtSource.Reset();
	Soldiers.Reset();
	UnitIds.Reset();
	Front.Reset();
	Engaged.Reset();
	float MaxRadius = 1.f;
	bool bAnyWalking = false;
	bool bAnyEngaged = false;
	EntityQuery.ForEachEntityChunk(Context, [this, &MaxRadius, &bAnyWalking, &bAnyEngaged](FMassExecutionContext& Context)
	{
		const TConstArrayView<FTransformFragment> Transforms = Context.GetFragmentView<FTransformFragment>();
		const TConstArrayView<FAgentRadiusFragment> AgentRadii = Context.GetFragmentView<FAgentRadiusFragment>();
		const TConstArrayView<FMassMoveTargetFragment> MoveTargets = Context.GetFragmentView<FMassMoveTargetFragment>();
		const TConstArrayView<FHOLUnitStateFragment> States = Context.GetFragmentView<FHOLUnitStateFragment>();
		const TConstArrayView<FHOLGatherFragment> Gathers = Context.GetFragmentView<FHOLGatherFragment>();
		const TConstArrayView<FHOLMoveOrderFragment> Orders = Context.GetFragmentView<FHOLMoveOrderFragment>();
		const TConstArrayView<FHOLCombatFragment> Combats = Context.GetFragmentView<FHOLCombatFragment>();
		const FHOLUnitTypeConstants* TypeConstants = Context.GetConstSharedFragmentPtr<FHOLUnitTypeConstants>();
		const bool bSoldiers = TypeConstants && TypeConstants->CarryCapacity <= 0;
		for (FMassExecutionContext::FEntityIterator EntityIt = Context.CreateEntityIterator(); EntityIt; ++EntityIt)
		{
			const FVector Location = Transforms[EntityIt].GetTransform().GetLocation();
			Soldiers.Add(bSoldiers);
			FormUpSerials.Add(!Orders.IsEmpty() ? GetPassThroughSerial(Orders[EntityIt]) : 0u);
			FormUpKinds.Add(!Orders.IsEmpty() ? GetPassThroughKind(Orders[EntityIt]) : uint8(0));
			Positions.Add(FVector2D(Location.X, Location.Y));
			Radii.Add(AgentRadii[EntityIt].Radius);
			// A gatherer on its round trip neither pushes nor is pushed (D309): it never looks, nobody finds it. Landed at
			// its spot, it looks once, as a walker: a unit standing there and itself part (the user's 3rd trial of J5b.2).
			const bool bGhost = IsGhost(States[EntityIt]);
			Ghosts.Add(bGhost);
			const bool bSettling = !Gathers.IsEmpty() && Gathers[EntityIt].bSettling;
			const bool bMoving = !bGhost && MoveTargets[EntityIt].GetCurrentAction() == EMassMovementAction::Move;
			const bool bWalking = bMoving || (!bGhost && bSettling);
			Walking.Add(bWalking);
			Moving.Add(bMoving);
			Owners.Add(States[EntityIt].OwnerPlayer);
			AtSource.Add(States[EntityIt].Activity == EHOLUnitActivity::Gathering);
			UnitIds.Add(States[EntityIt].UnitId);
			Front.Add(!Combats.IsEmpty() && Combats[EntityIt].bAttacking); // the contacts are added once the grid is built
			bool bEngaged = false;
			if (!Combats.IsEmpty() && !bGhost)
			{
				const FHOLCombatFragment& Combat = Combats[EntityIt];
				bEngaged = Combat.Target.IsSet() || Combat.bAttacking || Combat.bApproaching || Combat.Mode == EHOLCombatMode::AttackMove
					|| Combat.bAttackMoveDetour || Combat.bFlanking;
			}
			Engaged.Add(bEngaged);
			bAnyEngaged |= bEngaged;
			MaxRadius = FMath::Max(MaxRadius, AgentRadii[EntityIt].Radius);
			bAnyWalking |= bWalking;
		}
	});
	if (!bAnyWalking)
	{
		return;
	}

	// J7c.4, determinism: the units in identifier order, as the combat's fighters (unique identifiers, the unit subsystem's
	// table): the chunks' order differs between runs for the same state, and everything below follows the units' indices
	// (the cells' lists, the units to look, the corrections applied in place). Nothing to do when the chunks gave it already.
	bool bSorted = true;
	for (int32 Index = 1; Index < UnitIds.Num() && bSorted; ++Index)
	{
		bSorted = UnitIds[Index - 1] < UnitIds[Index];
	}
	if (!bSorted)
	{
		SortKeys.Reset(UnitIds.Num());
		for (int32 Index = 0; Index < UnitIds.Num(); ++Index)
		{
			SortKeys.Add(static_cast<uint64>(UnitIds[Index]) << 32 | static_cast<uint32>(Index));
		}
		SortKeys.Sort();
		SortedIndexOf.SetNumUninitialized(UnitIds.Num());
		for (int32 Sorted = 0; Sorted < SortKeys.Num(); ++Sorted)
		{
			SortedIndexOf[static_cast<int32>(SortKeys[Sorted] & 0xFFFFFFFFull)] = Sorted;
		}
		// Each array moved once, through one scratch buffer (all of them plain values).
		const auto Permute = [this](auto& Array)
		{
			using FElement = typename TRemoveReference<decltype(Array)>::Type::ElementType;
			PermuteScratch.SetNumUninitialized(Array.Num() * sizeof(FElement), EAllowShrinking::No);
			FElement* const Moved = reinterpret_cast<FElement*>(PermuteScratch.GetData());
			for (int32 Sorted = 0; Sorted < SortKeys.Num(); ++Sorted)
			{
				Moved[Sorted] = Array[static_cast<int32>(SortKeys[Sorted] & 0xFFFFFFFFull)];
			}
			FMemory::Memcpy(Array.GetData(), Moved, Array.Num() * sizeof(FElement));
		};
		Permute(Positions);
		Permute(Radii);
		Permute(Walking);
		Permute(Ghosts);
		Permute(FormUpSerials);
		Permute(FormUpKinds);
		Permute(Moving);
		Permute(Owners);
		Permute(AtSource);
		Permute(Soldiers);
		Permute(Front);
		Permute(Engaged);
	}

	// Cells as wide as the largest pair of bodies, whatever the map's size: the overlaps of a unit are all in the 3 x 3
	// cells around it. The cells go into a fixed table by a hash; a bucket shared by two cells keeps each unit's own.
	const double CellSize = 2.0 * MaxRadius;
	const auto BucketOf = [](const FIntPoint& Cell)
	{
		return static_cast<int32>((static_cast<uint32>(Cell.X) * 73856093u ^ static_cast<uint32>(Cell.Y) * 19349663u) & (SeparationBuckets - 1));
	};
	Cells.SetNumUninitialized(Positions.Num());
	FIntPoint MinCell(MAX_int32, MAX_int32);
	FIntPoint MaxCell(MIN_int32, MIN_int32);
	for (int32 Index = 0; Index < Positions.Num(); ++Index)
	{
		Cells[Index] = FIntPoint(FMath::FloorToInt32(Positions[Index].X / CellSize), FMath::FloorToInt32(Positions[Index].Y / CellSize));
		MinCell = FIntPoint(FMath::Min(MinCell.X, Cells[Index].X), FMath::Min(MinCell.Y, Cells[Index].Y));
		MaxCell = FIntPoint(FMath::Max(MaxCell.X, Cells[Index].X), FMath::Max(MaxCell.Y, Cells[Index].Y));
	}
	// Or a dense table (J7c.4): every cell of the units' bounding box, its units side by side. A bucket's chain is walked
	// from the last unit put in to the first, in descending index order; each cell's range is laid out the same way, so
	// the pairs are met in the same order as through the buckets (the corrections are applied in place: the order matters).
	const int64 GridWidth = static_cast<int64>(MaxCell.X) - MinCell.X + 1;
	const int64 GridHeight = static_cast<int64>(MaxCell.Y) - MinCell.Y + 1;
	const bool bDense = FastSeparation != 0 && GridWidth * GridHeight <= SeparationDenseMaxCells;
	if (bDense)
	{
		const int32 CellCount = static_cast<int32>(GridWidth * GridHeight);
		// Counted per cell (into the next cell's entry), then summed up: CellStarts[Cell + 1] is the end of the cell's range.
		CellStarts.Init(0, CellCount + 1);
		DenseCells.SetNumUninitialized(Positions.Num());
		for (int32 Index = 0; Index < Positions.Num(); ++Index)
		{
			DenseCells[Index] = (Cells[Index].Y - MinCell.Y) * static_cast<int32>(GridWidth) + (Cells[Index].X - MinCell.X);
			++CellStarts[DenseCells[Index] + 1];
		}
		for (int32 Cell = 0; Cell < CellCount; ++Cell)
		{
			CellStarts[Cell + 1] += CellStarts[Cell];
		}
		// Each range filled from its end, the units taken in ascending order: read from its start, in descending order.
		// The end moves down to the range's start, which CellStarts[Cell + 1] then holds.
		CellUnits.SetNumUninitialized(Positions.Num());
		for (int32 Index = 0; Index < Positions.Num(); ++Index)
		{
			CellUnits[--CellStarts[DenseCells[Index] + 1]] = Index;
		}
		// Back by one entry: CellStarts[Cell] the start of the cell's range, CellStarts[Cell + 1] its end.
		for (int32 Cell = 0; Cell < CellCount; ++Cell)
		{
			CellStarts[Cell] = CellStarts[Cell + 1];
		}
		CellStarts[CellCount] = Positions.Num();
	}
	else
	{
		CellHeads.Init(INDEX_NONE, SeparationBuckets);
		NextInCell.SetNumUninitialized(Positions.Num());
		for (int32 Index = 0; Index < Positions.Num(); ++Index)
		{
			int32& Head = CellHeads[BucketOf(Cells[Index])];
			NextInCell[Index] = Head;
			Head = Index;
		}
	}

	// The walkers look for their neighbours, then each standing unit they pushed looks for its own (it may now overlap a
	// third one), and so on: units standing together and left alone cost nothing. Each unit looks once a round; a second
	// round undoes most of what the pushes of the first made (two walking groups meeting head-on: 36 cm left with one).
	const double GridSeconds = FPlatformTime::Seconds();
	SeparationGridSeconds += GridSeconds - StartSeconds;
	const double Slop = FMath::Max(0.f, SeparationSlop);
	// A gatherer at its source and a unit standing beside it: the spots' spacing (D309), less what an arrival may stop
	// short of its spot; two gatherers on their spots are not pushed apart, a unit on a gatherer's spot is.
	const double AtSourceDistance = FMath::Max(0., UHOLGatherProcessor::GetSpotSpacing() - AtSourceTolerance);
	// J7c.4, loop 18 (hol.Simulation.FrontHoldsFriends): a unit touching an enemy is of the front as one that strikes.
	// Loop 30 (hol.Simulation.FrontNeedsCombat): only a contact where one of the two is concerned by a fight; none such
	// this step, no pass at all (a striking unit is concerned: the front is then empty).
	const bool bFrontNeedsCombat = FrontNeedsCombat != 0;
	if (FrontHoldsFriends != 0 && (!bFrontNeedsCombat || bAnyEngaged))
	{
		// Perf (hol.Simulation.ParallelFront): each unit sets only its own flag and reads only the others' fixed data (the
		// same flags in any order): in parallel.
		const auto FrontOf = [&](const int32 Index)
		{
			if (Front[Index] || Ghosts[Index])
			{
				return;
			}
			const bool bSelfEngaged = !bFrontNeedsCombat || Engaged[Index];
			for (int32 DY = -1; DY <= 1 && !Front[Index]; ++DY)
			{
				for (int32 DX = -1; DX <= 1 && !Front[Index]; ++DX)
				{
					const FIntPoint Cell(Cells[Index].X + DX, Cells[Index].Y + DY);
					const auto Touches = [&](const int32 Other)
					{
						return Other != Index && !Ghosts[Other] && CampTable.AreHostile(Owners[Other], Owners[Index]) && (bSelfEngaged || Engaged[Other])
							&& FVector2D::DistSquared(Positions[Index], Positions[Other]) <= FMath::Square(Radii[Index] + Radii[Other] + FrontContactGap);
					};
					if (bDense)
					{
						const int32 X = Cell.X - MinCell.X;
						const int32 Y = Cell.Y - MinCell.Y;
						if (X < 0 || X >= GridWidth || Y < 0 || Y >= GridHeight)
						{
							continue;
						}
						const int32 DenseCell = Y * static_cast<int32>(GridWidth) + X;
						for (int32 Slot = CellStarts[DenseCell]; Slot < CellStarts[DenseCell + 1] && !Front[Index]; ++Slot)
						{
							Front[Index] = Touches(CellUnits[Slot]);
						}
						continue;
					}
					for (int32 Other = CellHeads[BucketOf(Cell)]; Other != INDEX_NONE && !Front[Index]; Other = NextInCell[Other])
					{
						Front[Index] = Cells[Other] == Cell && Touches(Other);
					}
				}
			}
		};
		if (ParallelFront != 0)
		{
			ParallelFor(TEXT("HOLSepFront"), Positions.Num(), 128, FrontOf);
		}
		else
		{
			for (int32 Index = 0; Index < Positions.Num(); ++Index)
			{
				FrontOf(Index);
			}
		}
	}
	// J7c.4 loop 21 (hol.Combat.RearFlank): each side's line of contact, O(N): its front's units across the way to the
	// other armies' middle (their span, their mean place).
	FrontLines.Reset();
	if (FrontHoldsFriends != 0 && MeleeRuleSkips != 0)
	{
		// Loop 29: as below, summed per owner in the units' order without maps (the same sums, the same lines).
		FVector2D OwnSums[256];
		FVector2D LineSums[256];
		int32 OwnCounts[256] = {};
		int32 LineCounts[256] = {};
		for (int32 Owner = 0; Owner < 256; ++Owner)
		{
			OwnSums[Owner] = FVector2D::ZeroVector;
			LineSums[Owner] = FVector2D::ZeroVector;
		}
		FVector2D AllSum = FVector2D::ZeroVector;
		int32 AllCount = 0;
		for (int32 Index = 0; Index < Positions.Num(); ++Index)
		{
			if (Ghosts[Index] || !Soldiers[Index])
			{
				continue;
			}
			const uint8 Owner = Owners[Index];
			OwnSums[Owner] += Positions[Index];
			++OwnCounts[Owner];
			AllSum += Positions[Index];
			++AllCount;
			if (Front[Index])
			{
				LineSums[Owner] += Positions[Index];
				++LineCounts[Owner];
			}
		}
		FHOLFrontLine* LineOf[256] = {};
		bool bAnyLine = false;
		for (int32 Owner = 0; Owner < 256; ++Owner)
		{
			if (LineCounts[Owner] == 0 || AllCount <= OwnCounts[Owner])
			{
				continue;
			}
			const FVector2D EnemyMiddle = (AllSum - OwnSums[Owner]) / (AllCount - OwnCounts[Owner]);
			const FVector2D Charge = (EnemyMiddle - OwnSums[Owner] / OwnCounts[Owner]).GetSafeNormal();
			if (Charge.IsNearlyZero())
			{
				continue;
			}
			FHOLFrontLine& Line = FrontLines.Add(static_cast<uint8>(Owner));
			Line.Charge = Charge;
			Line.Axis = FVector2D(-Charge.Y, Charge.X);
			Line.Origin = LineSums[Owner];
			Line.Count = LineCounts[Owner];
			Line.Origin /= Line.Count;
			Line.Min = TNumericLimits<double>::Max();
			Line.Max = TNumericLimits<double>::Lowest();
			bAnyLine = true;
		}
		// The map's entries stay put once all are added.
		if (bAnyLine)
		{
			for (TPair<uint8, FHOLFrontLine>& Pair : FrontLines)
			{
				LineOf[Pair.Key] = &Pair.Value;
			}
			for (int32 Index = 0; Index < Positions.Num(); ++Index)
			{
				if (!Front[Index] || Ghosts[Index] || !Soldiers[Index])
				{
					continue;
				}
				if (FHOLFrontLine* const Line = LineOf[Owners[Index]])
				{
					const double Along = FVector2D::DotProduct(Positions[Index] - Line->Origin, Line->Axis);
					Line->Min = FMath::Min(Line->Min, Along);
					Line->Max = FMath::Max(Line->Max, Along);
				}
			}
		}
	}
	else if (FrontHoldsFriends != 0)
	{
		TMap<uint8, FVector2D> OwnerSums;
		TMap<uint8, int32> OwnerCounts;
		FVector2D AllSum = FVector2D::ZeroVector;
		int32 AllCount = 0;
		for (int32 Index = 0; Index < Positions.Num(); ++Index)
		{
			if (Ghosts[Index] || !Soldiers[Index])
			{
				continue;
			}
			OwnerSums.FindOrAdd(Owners[Index]) += Positions[Index];
			++OwnerCounts.FindOrAdd(Owners[Index]);
			AllSum += Positions[Index];
			++AllCount;
			if (Front[Index])
			{
				FHOLFrontLine& Line = FrontLines.FindOrAdd(Owners[Index]);
				Line.Origin += Positions[Index];
				++Line.Count;
			}
		}
		for (auto It = FrontLines.CreateIterator(); It; ++It)
		{
			FHOLFrontLine& Line = It.Value();
			const int32 OwnCount = OwnerCounts[It.Key()];
			const FVector2D OwnSum = OwnerSums[It.Key()];
			if (AllCount <= OwnCount)
			{
				It.RemoveCurrent();
				continue;
			}
			const FVector2D EnemyMiddle = (AllSum - OwnSum) / (AllCount - OwnCount);
			Line.Charge = (EnemyMiddle - OwnSum / OwnCount).GetSafeNormal();
			if (Line.Charge.IsNearlyZero())
			{
				It.RemoveCurrent();
				continue;
			}
			Line.Axis = FVector2D(-Line.Charge.Y, Line.Charge.X);
			Line.Origin /= Line.Count;
			Line.Min = TNumericLimits<double>::Max();
			Line.Max = TNumericLimits<double>::Lowest();
		}
		for (int32 Index = 0; Index < Positions.Num(); ++Index)
		{
			if (!Front[Index] || Ghosts[Index] || !Soldiers[Index])
			{
				continue;
			}
			if (FHOLFrontLine* const Line = FrontLines.Find(Owners[Index]))
			{
				const double Along = FVector2D::DotProduct(Positions[Index] - Line->Origin, Line->Axis);
				Line->Min = FMath::Min(Line->Min, Along);
				Line->Max = FMath::Max(Line->Max, Along);
			}
		}
	}
	const bool bHoldFront = FrontHoldsFriends != 0;
	const bool bSideGuard = EnemySideGuard != 0;
	Corrections = Positions;
	Queued = Walking;
	ToLook.Reset();
	for (int32 Index = 0; Index < Positions.Num(); ++Index)
	{
		if (Walking[Index])
		{
			ToLook.Add(Index);
		}
	}
	// A pair met by the unit that looks (Index), in its cell or one around it.
	const auto Undo = [this, Slop, AtSourceDistance, bHoldFront, bSideGuard](const int32 Index, const int32 Other)
	{
		// Two units of one order forming up in different kinds of rows, of one block's re-forming, or a
		// straggler walking into its slot (J7a.4, J7b note 1: CanPassThrough): through each other, the ways to the
		// new rows cross (three bodies pressed together, each walking into the next, held each other still).
		// Two forming up in one kind of row keep apart (the user's trial of playtest note 1 of J7b).
		// Perf (hol.Units.FastNear): the overlap tested first, the pass-through after (both only end the pair, in any order).
		const bool bOverlapFirst = FastNear != 0;
		if (!bOverlapFirst && CanPassThrough(FormUpSerials[Index], FormUpKinds[Index], FormUpSerials[Other], FormUpKinds[Other]))
		{
			return;
		}
		const FVector2D Delta = Corrections[Index] - Corrections[Other];
		double MinDistance = Radii[Index] + Radii[Other] - Slop;
		if ((AtSource[Index] || AtSource[Other]) && !Moving[Index] && !Moving[Other])
		{
			MinDistance = FMath::Min(MinDistance, AtSourceDistance);
		}
		const double DistanceSquared = Delta.SizeSquared();
		if (DistanceSquared >= FMath::Square(MinDistance))
		{
			return;
		}
		if (bOverlapFirst && CanPassThrough(FormUpSerials[Index], FormUpKinds[Index], FormUpSerials[Other], FormUpKinds[Other]))
		{
			return;
		}
		const double Distance = FMath::Sqrt(DistanceSquared);
		const bool bEnemies = CampTable.AreHostile(Owners[Index], Owners[Other]);
		// Two units on the same point: apart along a direction of their own, the same at every run.
		FVector2D Normal = Distance > UE_KINDA_SMALL_NUMBER ? Delta / Distance
			: FVector2D(FMath::Cos(Index * 2.399), FMath::Sin(Index * 2.399));
		// Two units of one player take half each: a walker pushes a standing unit out of its way as it would push a
		// walker. Two enemies never push each other back (the user's trial of J7b): a walker stops against a
		// standing enemy, which stays where it is; the one that looks is a walker or pushed this step, so it moves.
		double Overlap = MinDistance - Distance;
		// J7c.4, loop 18 (hol.Simulation.EnemySideGuard): two enemies part along their axis at the step's start (before any
		// push of this step), each sent back to its own side, never out on the enemy's side when a friend pushed it past
		// the enemy's centre; by the least move along that axis that frees them (the whole way back when it crossed).
		if (bEnemies && bSideGuard)
		{
			const FVector2D StartDelta = Positions[Index] - Positions[Other];
			const double StartDistance = StartDelta.Size();
			if (StartDistance > UE_KINDA_SMALL_NUMBER)
			{
				Normal = StartDelta / StartDistance;
			}
			const double Along = FVector2D::DotProduct(Delta, Normal);
			const double Across = FVector2D::DotProduct(Delta, FVector2D(-Normal.Y, Normal.X));
			Overlap = FMath::Max(0., FMath::Sqrt(FMath::Max(0., FMath::Square(MinDistance) - FMath::Square(Across))) - Along);
			if (Overlap <= 0.)
			{
				return;
			}
		}
		double Share = HOLCombat::GetOverlapShare(bEnemies, Walking[Index], Walking[Other]);
		// J7c.4, loop 18 (hol.Simulation.FrontHoldsFriends): a friend of the front (striking or touching an enemy) is a wall:
		// the other one takes the whole overlap, the front is neither moved nor queued (two of the front: as before).
		if (!bEnemies && bHoldFront && Front[Index] != Front[Other])
		{
			Share = Front[Other] ? 1. : 0.;
		}
		if (Share > 0.)
		{
			Corrections[Index] += Normal * (Overlap * Share);
		}
		if (Share >= 1.)
		{
			return;
		}
		Corrections[Other] -= Normal * (Overlap * (1. - Share));
		if (!Queued[Other])
		{
			Queued[Other] = true;
			ToLook.Add(Other);
		}
	};
	for (int32 Round = 0; Round < SeparationRounds; ++Round)
	{
		Looked.Init(false, Positions.Num());
		for (int32 Next = 0; Next < ToLook.Num(); ++Next)
		{
			const int32 Index = ToLook[Next];
			Looked[Index] = true;
			for (int32 DY = -1; DY <= 1; ++DY)
			{
				for (int32 DX = -1; DX <= 1; ++DX)
				{
					const FIntPoint Cell(Cells[Index].X + DX, Cells[Index].Y + DY);
					if (bDense)
					{
						// A cell out of the bounding box holds no unit (nor did its bucket hold any of this cell).
						const int32 X = Cell.X - MinCell.X;
						const int32 Y = Cell.Y - MinCell.Y;
						if (X < 0 || X >= GridWidth || Y < 0 || Y >= GridHeight)
						{
							continue;
						}
						const int32 DenseCell = Y * static_cast<int32>(GridWidth) + X;
						for (int32 Slot = CellStarts[DenseCell]; Slot < CellStarts[DenseCell + 1]; ++Slot)
						{
							// A pair is undone once a round, by the first of the two to look.
							const int32 Other = CellUnits[Slot];
							if (Other != Index && !Looked[Other] && !Ghosts[Other])
							{
								Undo(Index, Other);
							}
						}
						continue;
					}
					for (int32 Other = CellHeads[BucketOf(Cell)]; Other != INDEX_NONE; Other = NextInCell[Other])
					{
						// A pair is undone once a round, by the first of the two to look; another cell of the same bucket is not this one.
						if (Other == Index || Looked[Other] || Ghosts[Other] || Cells[Other] != Cell)
						{
							continue;
						}
						Undo(Index, Other);
					}
				}
			}
		}
	}

	// J7c.4, loop 18 (hol.Simulation.EnemySideGuard): a unit moves at most SeparationMaxStep a step (its radius by default),
	// the pushes of a step added up never a jump through a rank.
	if (bSideGuard && SeparationMaxStep != 0.f)
	{
		for (int32 Index = 0; Index < Positions.Num(); ++Index)
		{
			const double MaxStep = SeparationMaxStep > 0.f ? static_cast<double>(SeparationMaxStep) : static_cast<double>(Radii[Index]);
			const FVector2D Move = Corrections[Index] - Positions[Index];
			if (Move.SizeSquared() > FMath::Square(MaxStep))
			{
				Corrections[Index] = Positions[Index] + Move.GetSafeNormal() * MaxStep;
			}
		}
	}

	SeparationRoundsSeconds += FPlatformTime::Seconds() - GridSeconds;

	// The chunks visited in the gathering's order: each unit's sorted index (J7c.4, determinism).
	int32 Gathered = 0;
	EntityQuery.ForEachEntityChunk(Context, [this, &Gathered, bSorted](FMassExecutionContext& Context)
	{
		const TArrayView<FTransformFragment> Transforms = Context.GetMutableFragmentView<FTransformFragment>();
		for (FMassExecutionContext::FEntityIterator EntityIt = Context.CreateEntityIterator(); EntityIt; ++EntityIt, ++Gathered)
		{
			const int32 Index = bSorted ? Gathered : SortedIndexOf[Gathered];
			if (Corrections[Index] != Positions[Index])
			{
				FTransform& Transform = Transforms[EntityIt].GetMutableTransform();
				const FVector Location = Transform.GetLocation();
				Transform.SetLocation(FVector(Corrections[Index].X, Corrections[Index].Y, Location.Z));
			}
		}
	});
}

namespace UE::HeroesOfLiberty::Simulation
{
	/** Times a body is put back out of the footprints a step, at most: two buildings side by side, out of one into the other. */
	constexpr int32 KeepOutPasses = 3;
	/** The largest body the footprints' reach allows for (cm): above any unit's radius (60 cm, the Peasant drawn x1.5). */
	constexpr double KeepOutMaxRadius = 200.;
	/** Gap (cm) a body put back is left from the wall: the rounding of the move back does not leave it a hair inside. */
	constexpr double KeepOutSlack = 1.;
	/** EtapeDefini C.1: a unit standing still goes out of a visible wall this much a step at most (200 cm/s at 20 steps a second). */
	constexpr double KeepOutWallStep = 10.;
	/** Side (cm) of the cells of the keep-out's grid, and cells at most (a larger side past that). */
	constexpr double KeepOutCellSize = 2000.;
	constexpr int32 KeepOutMaxCells = 4096;
	/**
	 * The map's fixed obstacles (ridge_army): a static mesh wider than this (cm) is the ground, not an obstacle; one whose
	 * bottom is higher than this (cm) stands above the units' heads (the grove's crowns, from 4.75 m).
	 */
	constexpr double ObstacleMaxSize = 5000.;
	constexpr double ObstacleMaxBottom = 250.;
	/** A body in an obstacle is put back onto the navmesh's nearest point within this reach across (cm) and height. */
	constexpr double ObstacleProbeReach = 400.;
	constexpr double ObstacleProbeHeight = 60.;
	/** A put back of less than this (cm) is none: a body on the navmesh's edge stays where it is. */
	constexpr double ObstacleSlack = 1.;

	int32 KeepOutEnabled = 1;
	FAutoConsoleVariableRef CVarKeepOut(TEXT("hol.Units.KeepOut"), KeepOutEnabled,
		TEXT("1: a body in a building's footprint is put back against its wall (D310); 2: the same, each one logged; 0: off, for A / B measures"));
}

std::atomic<int64> UHOLFootprintKeepOutProcessor::NumPushedOut{ 0 };
std::atomic<int64> UHOLFootprintKeepOutProcessor::NumEasedOut{ 0 };
std::atomic<int64> UHOLFootprintKeepOutProcessor::NumPutOutOfObstacles{ 0 };

UHOLFootprintKeepOutProcessor::UHOLFootprintKeepOutProcessor()
	: EntityQuery(*this)
{
	bAutoRegisterWithProcessingPhases = false; // fixed step only (D219)
	ExecutionFlags = static_cast<int32>(EProcessorExecutionFlags::Server | EProcessorExecutionFlags::Standalone); // authoritative simulation (D231)
	// Once the units moved and were pushed apart, before the obstacle grid records where they are.
	ExecutionOrder.ExecuteAfter.Add(TEXT("HOLUnitSeparationProcessor"));
	ExecutionOrder.ExecuteBefore.Add(TEXT("MassNavigationObstacleGridProcessor"));
	bRequiresGameThreadExecution = true; // the buildings are read from the world
}

void UHOLFootprintKeepOutProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FAgentRadiusFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FHOLMoveOrderFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FHOLUnitStateFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FHOLCombatFragment>(EMassFragmentAccess::ReadOnly, EMassFragmentPresence::Optional);
	EntityQuery.AddTagRequirement<FHOLLightUnitTag>(EMassFragmentPresence::All);
}

void UHOLFootprintKeepOutProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	using namespace UE::HeroesOfLiberty::Simulation;
	const UWorld* World = EntityManager.GetWorld();
	if (!World || KeepOutEnabled == 0)
	{
		return;
	}
	// A few buildings per player: read again at each step, as the gathering's deposits.
	Footprints.Reset();
	for (TActorIterator<AHOLBuilding> It(World); It; ++It)
	{
		const FHOLBuildingTypeRow* Type = It->GetType();
		// A bare foundation is walked over (D312): only a footprint the navmesh forbids keeps bodies out.
		if (!Type || !It->BlocksNavigation())
		{
			continue;
		}
		FFootprint& Footprint = Footprints.AddDefaulted_GetRef();
		Footprint.Centre = FVector2D(It->GetActorLocation());
		// The building's body, inset in its tiles (D324): a unit walks between two neighbours.
		Footprint.HalfExtent = UHOLBuildingSettings::GetBodySize(Type->Footprint) * 0.5;
		FMath::SinCos(&Footprint.Sin, &Footprint.Cos, FMath::DegreesToRadians(It->GetActorRotation().Yaw));
		// The bounds of the turned rectangle, widened.
		const FVector2D Half(FMath::Abs(Footprint.Cos) * Footprint.HalfExtent.X + FMath::Abs(Footprint.Sin) * Footprint.HalfExtent.Y,
			FMath::Abs(Footprint.Sin) * Footprint.HalfExtent.X + FMath::Abs(Footprint.Cos) * Footprint.HalfExtent.Y);
		const FVector2D Margin(KeepOutMaxRadius);
		Footprint.Reach = FBox2D(Footprint.Centre - Half - Margin, Footprint.Centre + Half + Margin);
	}
	// The map's fixed obstacles, once per world (ridge_army: a block's files walked into the ridge's rocks, nothing held
	// them out but their paths). The navmesh, cut by them, is the rule: a body whose centre is off it is in one.
	if (ObstacleWorld.Get() != World)
	{
		ObstacleWorld = World;
		ObstacleReaches.Reset();
		ObstacleBounds = FBox2D(ForceInit);
		for (TActorIterator<AStaticMeshActor> It(World); It; ++It)
		{
			const UStaticMeshComponent* Mesh = It->GetStaticMeshComponent();
			if (!Mesh || !Mesh->CanEverAffectNavigation() || !Mesh->IsCollisionEnabled())
			{
				continue;
			}
			const FBox Box = Mesh->Bounds.GetBox();
			if (Box.GetSize().X > ObstacleMaxSize || Box.GetSize().Y > ObstacleMaxSize || Box.Min.Z > ObstacleMaxBottom)
			{
				continue;
			}
			const FBox2D Reach(FVector2D(Box.Min) - FVector2D(KeepOutMaxRadius), FVector2D(Box.Max) + FVector2D(KeepOutMaxRadius));
			ObstacleReaches.Add(Reach);
			ObstacleBounds += Reach;
		}
		UE_LOG(LogHOLSimulation, Display, TEXT("Keep-out: %d fixed obstacles of the map"), ObstacleReaches.Num());
	}
	const UNavigationSystemV1* NavSystem = ObstacleReaches.IsEmpty() ? nullptr : FNavigationSystem::GetCurrent<UNavigationSystemV1>(World);
	const ANavigationData* NavData = NavSystem ? NavSystem->GetDefaultNavDataInstance() : nullptr;
	if (Footprints.IsEmpty() && NavData == nullptr)
	{
		return;
	}
	// The grid over the footprints' reaches: a unit far from every building costs one box test.
	GridBounds = FBox2D(ForceInit);
	for (const FFootprint& Footprint : Footprints)
	{
		GridBounds += Footprint.Reach;
	}
	if (Footprints.IsEmpty())
	{
		// No building: an empty cell no unit is in.
		GridBounds = FBox2D(FVector2D(TNumericLimits<double>::Max()), FVector2D(TNumericLimits<double>::Max()));
	}
	double CellSize = KeepOutCellSize;
	const FVector2D Extent = Footprints.IsEmpty() ? FVector2D(1.) : GridBounds.GetSize();
	while (FMath::CeilToDouble(Extent.X / CellSize) * FMath::CeilToDouble(Extent.Y / CellSize) > KeepOutMaxCells)
	{
		CellSize *= 2.;
	}
	GridSize = FIntPoint(FMath::Max(1, FMath::CeilToInt32(Extent.X / CellSize)), FMath::Max(1, FMath::CeilToInt32(Extent.Y / CellSize)));
	const auto GetCell = [this, CellSize](const FVector2D& Point)
	{
		return FIntPoint(FMath::Clamp(FMath::FloorToInt32((Point.X - GridBounds.Min.X) / CellSize), 0, GridSize.X - 1),
			FMath::Clamp(FMath::FloorToInt32((Point.Y - GridBounds.Min.Y) / CellSize), 0, GridSize.Y - 1));
	};
	CellStarts.Init(0, GridSize.X * GridSize.Y + 1);
	for (int32 Pass = 0; Pass < 2; ++Pass)
	{
		// First the count of each cell, then the footprints in place.
		TArray<int32> Fill;
		if (Pass == 1)
		{
			for (int32 Cell = 1; Cell < CellStarts.Num(); ++Cell)
			{
				CellStarts[Cell] += CellStarts[Cell - 1];
			}
			CellFootprints.SetNumUninitialized(CellStarts.Last());
			Fill = CellStarts;
		}
		for (int32 Index = 0; Index < Footprints.Num(); ++Index)
		{
			const FIntPoint Min = GetCell(Footprints[Index].Reach.Min);
			const FIntPoint Max = GetCell(Footprints[Index].Reach.Max);
			for (int32 Y = Min.Y; Y <= Max.Y; ++Y)
			{
				for (int32 X = Min.X; X <= Max.X; ++X)
				{
					const int32 Cell = Y * GridSize.X + X;
					if (Pass == 0)
					{
						++CellStarts[Cell + 1];
					}
					else
					{
						CellFootprints[Fill[Cell]++] = Index;
					}
				}
			}
		}
	}

	// From a body to the navmesh's edge (D310) and to the visible wall (D324).
	const double Clearance = UHOLBuildingSettings::GetBodyClearance();
	const double VisualGap = FMath::Max(0., static_cast<double>(UHOLBuildingSettings::GetBodyInset() - UHOLBuildingSettings::GetVisualInset()));

	// A point put out of the bodies near it (a few passes: two neighbours); then, if no body was put back, a unit standing
	// still out of their visible walls, one bounded step a step, unless wedged between two (C.3). bPutBack: a body put back (C.2).
	const auto PushOut = [this, GetCell, VisualGap](FVector2D& Position, const double Radius, const double KeepInside, const double Keep, const bool bStill,
		bool& bPutBack, const bool bSquareCorners = false)
	{
		bool bMoved = false;
		bPutBack = false;
		if (Footprints.IsEmpty())
		{
			return false;
		}
		for (int32 Pass = 0; Pass < KeepOutPasses; ++Pass)
		{
			bool bMovedThisPass = false;
			if (!GridBounds.IsInsideOrOn(Position))
			{
				break;
			}
			const FIntPoint Cell = GetCell(Position);
			const int32 CellIndex = Cell.Y * GridSize.X + Cell.X;
			for (int32 Item = CellStarts[CellIndex]; Item < CellStarts[CellIndex + 1]; ++Item)
			{
				const FFootprint& Footprint = Footprints[CellFootprints[Item]];
				if (!Footprint.Reach.IsInsideOrOn(Position))
				{
					continue;
				}
				// In the footprint's frame.
				const FVector2D Offset = Position - Footprint.Centre;
				FVector2D Local(Offset.X * Footprint.Cos + Offset.Y * Footprint.Sin, -Offset.X * Footprint.Sin + Offset.Y * Footprint.Cos);
				if (!HOLConstruction::KeepOutOfBody(Footprint.HalfExtent, KeepInside, Keep, Local))
				{
					continue;
				}
				bPutBack = true;
				Position = Footprint.Centre + FVector2D(Local.X * Footprint.Cos - Local.Y * Footprint.Sin, Local.X * Footprint.Sin + Local.Y * Footprint.Cos);
				bMovedThisPass = true;
			}
			bMoved |= bMovedThisPass;
			if (!bMovedThisPass)
			{
				break;
			}
		}
		if (bMoved || !bStill || !GridBounds.IsInsideOrOn(Position))
		{
			return bMoved;
		}
		// The walls near it, all at once: out of one alone, never into another (C.3).
		TArray<HOLConstruction::FWallBody, TInlineAllocator<8>> Near;
		const FIntPoint Cell = GetCell(Position);
		const int32 CellIndex = Cell.Y * GridSize.X + Cell.X;
		for (int32 Item = CellStarts[CellIndex]; Item < CellStarts[CellIndex + 1]; ++Item)
		{
			const FFootprint& Footprint = Footprints[CellFootprints[Item]];
			if (Footprint.Reach.ExpandBy(KeepOutMaxRadius).IsInsideOrOn(Position))
			{
				Near.Add({ Footprint.Centre, Footprint.HalfExtent, Footprint.Cos, Footprint.Sin });
			}
		}
		return HOLConstruction::EaseOutOfVisibleWalls(Near, Radius, VisualGap, KeepOutWallStep, Position, bSquareCorners);
	};

	// One thread: a light loop (S7, 2 000 units and 32 buildings: ~0.11 ms a step, as much as on the workers).
	// Out of the map's fixed obstacles (ridge_army): a body near one whose centre is off the navmesh is put back on its
	// nearest point (the navmesh is eroded by the units' radius: its body then touches the rock at most). Only near them:
	// the buildings keep their own rule above (a crowd against a wall touches it, closer than the navmesh's edge).
	const auto PutOutOfObstacles = [this, NavSystem, NavData](FVector2D& Position, const double Z)
	{
		if (NavData == nullptr || !ObstacleBounds.IsInsideOrOn(Position))
		{
			return false;
		}
		const bool bNear = ObstacleReaches.ContainsByPredicate([&Position](const FBox2D& Reach) { return Reach.IsInsideOrOn(Position); });
		FNavLocation OnNav;
		if (!bNear || !NavSystem->ProjectPointToNavigation(FVector(Position, Z), OnNav, FVector(ObstacleProbeReach, ObstacleProbeReach, ObstacleProbeHeight), NavData)
			|| FVector2D::Distance(FVector2D(OnNav.Location), Position) <= ObstacleSlack)
		{
			return false;
		}
		Position = FVector2D(OnNav.Location);
		return true;
	};

	EntityQuery.ForEachEntityChunk(Context, [this, PushOut, PutOutOfObstacles, Clearance, VisualGap](FMassExecutionContext& Context)
	{
		const TArrayView<FTransformFragment> Transforms = Context.GetMutableFragmentView<FTransformFragment>();
		const TConstArrayView<FAgentRadiusFragment> AgentRadii = Context.GetFragmentView<FAgentRadiusFragment>();
		const TArrayView<FHOLMoveOrderFragment> Orders = Context.GetMutableFragmentView<FHOLMoveOrderFragment>();
		const TConstArrayView<FHOLUnitStateFragment> States = Context.GetFragmentView<FHOLUnitStateFragment>();
		const TConstArrayView<FHOLCombatFragment> Combats = Context.GetFragmentView<FHOLCombatFragment>();
		int64 Pushed = 0;
		int64 Eased = 0;
		int64 OutOfObstacles = 0;
		for (FMassExecutionContext::FEntityIterator EntityIt = Context.CreateEntityIterator(); EntityIt; ++EntityIt)
		{
			FTransform& Transform = Transforms[EntityIt].GetMutableTransform();
			const FVector Location = Transform.GetLocation();
			FVector2D Position(Location.X, Location.Y);
			// The body against the wall at most: the navmesh keeps the walkers the clearance away (D310); a crowd pressed
			// against a building touches it. Not the clearance: on the turned buildings of S7 the navmesh's cells leave the
			// walkers' paths 11 cm from the wall, a keep-out at 25 cm would put them back at every step.
			const double Keep = AgentRadii[EntityIt].Radius + KeepOutSlack;
			// Its centre inside (a site started under it, J6a.1): out beyond the navmesh's margin and the visible wall.
			const double Radius = AgentRadii[EntityIt].Radius;
			const double KeepInside = HOLConstruction::GetKeepOutInsideGap(Radius, Clearance, VisualGap);
			// Standing still (EtapeDefini C.1): wholly out of the visible wall, a step at a time; a walker, a worker or a
			// fighter keeps D324's shoulder.
			FHOLMoveOrderFragment& Order = Orders[EntityIt];
			const FHOLCombatFragment* Combat = Combats.Num() > 0 ? &Combats[EntityIt] : nullptr;
			const bool bFighting = Combat && (Combat->Target.IsSet() || Combat->TargetBuilding.IsValid() || Combat->bAttacking);
			const bool bStill = HOLConstruction::StandsClearOfVisibleWall(States[EntityIt].Activity, Order.Phase, bFighting);
			// EtapeDefini J loop 8: a unit striking a building whose place it was given (kept or forgotten) is kept out of the
			// visible wall too, the same step at a time (the user's trial: units half in the House). Any other fighter, walker
			// or worker keeps D324's shoulder.
			const bool bStrikesByPlace = Combat && Combat->bAttacking && Combat->TargetBuilding.IsValid()
				&& (Combat->PlaceBuilding == Combat->TargetBuilding || Combat->PlaceGivenUpBuilding == Combat->TargetBuilding);
			const bool bKeptOut = bStill || bStrikesByPlace;
			bool bPutBack = false;
			// Loop 9: those out of the square slab, its corners too (the round rule round the body left them ~30 cm in a corner);
			// EtapeDefini R: the units standing still as well (their bodies stayed ~30 cm in the slab's corners).
			const bool bMoved = PushOut(Position, Radius, KeepInside, Keep, bKeptOut, bPutBack, bKeptOut);
			// Its formation place goes out with it: held in the wall, it would walk back in (C.1).
			if (bKeptOut && Order.PlaceHold.bHeld)
			{
				FVector2D Place(Order.Slot.X, Order.Slot.Y);
				bool bPlacePutBack = false;
				if (PushOut(Place, Radius, KeepInside, Keep, true, bPlacePutBack, true))
				{
					Order.Slot = FVector(Place.X, Place.Y, Order.Slot.Z);
				}
			}
			if (bMoved)
			{
				// Put back (a body in a building's, counted as D310's) or eased out of the visible wall alone (C.2, apart).
				UE_CLOG(KeepOutEnabled == 2, LogHOLSimulation, Display, TEXT("Keep-out: #%u %s/%s at (%.0f, %.0f) %s at (%.0f, %.0f), its slot (%.0f, %.0f)%s"),
					States[EntityIt].UnitId, *StaticEnum<EHOLUnitActivity>()->GetNameStringByValue(static_cast<int64>(States[EntityIt].Activity)),
					*StaticEnum<EHOLMovePhase>()->GetNameStringByValue(static_cast<int64>(Orders[EntityIt].Phase)), Location.X, Location.Y,
					bPutBack ? TEXT("put back") : TEXT("eased out of the visible wall"), Position.X, Position.Y,
					Orders[EntityIt].Slot.X, Orders[EntityIt].Slot.Y, Order.PlaceHold.bHeld ? TEXT(" held") : TEXT(" not held"));
				Transform.SetLocation(FVector(Position.X, Position.Y, Location.Z));
				++(bPutBack ? Pushed : Eased);
			}
			// Out of a rock or a trunk (ridge_army), after the buildings: never into one of them.
			if (!bPutBack && PutOutOfObstacles(Position, Location.Z))
			{
				UE_CLOG(KeepOutEnabled == 2, LogHOLSimulation, Display, TEXT("Keep-out: #%u at (%.0f, %.0f) put out of a fixed obstacle at (%.0f, %.0f)"),
					States[EntityIt].UnitId, Location.X, Location.Y, Position.X, Position.Y);
				Transform.SetLocation(FVector(Position.X, Position.Y, Location.Z));
				++OutOfObstacles;
				Order.ObstaclePushes = static_cast<uint8>(FMath::Min<int32>(Order.ObstaclePushes + 1, MAX_uint8));
			}
			else if (Order.ObstaclePushes != 0)
			{
				Order.ObstaclePushes = 0;
			}
			// A walker put back step after step: its way crosses the building, the order processor finds it a new path.
			if (bMoved && (Order.Phase == EHOLMovePhase::Path || Order.Phase == EHOLMovePhase::FinalLeg))
			{
				Order.BuildingPushes = static_cast<uint8>(FMath::Min<int32>(Order.BuildingPushes + 1, MAX_uint8));
			}
			else if (Order.BuildingPushes != 0)
			{
				Order.BuildingPushes = 0;
			}
		}
		if (Pushed > 0)
		{
			NumPushedOut += Pushed;
		}
		if (Eased > 0)
		{
			NumEasedOut += Eased;
		}
		if (OutOfObstacles > 0)
		{
			NumPutOutOfObstacles += OutOfObstacles;
		}
	});
}

UHOLInstantOrientationProcessor::UHOLInstantOrientationProcessor()
	: EntityQuery(*this)
{
	bAutoRegisterWithProcessingPhases = false; // fixed step only (D219)
	ExecutionFlags = static_cast<int32>(EProcessorExecutionFlags::Server | EProcessorExecutionFlags::Standalone); // authoritative simulation (D231)
	// Once the units moved and were pushed apart.
	ExecutionOrder.ExecuteAfter.Add(TEXT("HOLUnitSeparationProcessor"));
}

void UHOLInstantOrientationProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddRequirement<FMassVelocityFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FHOLMoveOrderFragment>(EMassFragmentAccess::ReadOnly, EMassFragmentPresence::Optional);
	EntityQuery.AddTagRequirement<FHOLLightUnitTag>(EMassFragmentPresence::All);
}

void UHOLInstantOrientationProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	using namespace UE::HeroesOfLiberty::Simulation;

	EntityQuery.ParallelForEachEntityChunk(Context, [](FMassExecutionContext& Context)
	{
		const TConstArrayView<FMassVelocityFragment> Velocities = Context.GetFragmentView<FMassVelocityFragment>();
		const TArrayView<FTransformFragment> Transforms = Context.GetMutableFragmentView<FTransformFragment>();
		const TConstArrayView<FHOLMoveOrderFragment> Orders = Context.GetFragmentView<FHOLMoveOrderFragment>();

		for (FMassExecutionContext::FEntityIterator EntityIt = Context.CreateEntityIterator(); EntityIt; ++EntityIt)
		{
			// The walk's velocity (a standing unit's is zero, D290), not the step's move: a push does not turn it.
			const FVector& Velocity = Velocities[EntityIt].Value;
			if (Velocity.SizeSquared2D() > FMath::Square(MinFacingSpeed))
			{
				Transforms[EntityIt].GetMutableTransform().SetRotation(FQuat(FVector::UpVector, FMath::Atan2(Velocity.Y, Velocity.X)));
			}
			// Standing at the end of a formation move (J7a.4): turned towards the front of its rows, at once as a walker.
			else if (!Orders.IsEmpty() && Orders[EntityIt].bFacesFront && Orders[EntityIt].Phase == EHOLMovePhase::None)
			{
				Transforms[EntityIt].GetMutableTransform().SetRotation(FQuat(FVector::UpVector, FMath::DegreesToRadians(Orders[EntityIt].FrontYaw)));
			}
		}
	});
}
