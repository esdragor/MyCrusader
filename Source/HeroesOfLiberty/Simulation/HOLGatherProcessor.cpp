// Copyright Epic Games, Inc. All Rights Reserved.

#include "Simulation/HOLGatherProcessor.h"

#include "Buildings/HOLBuilding.h"
#include "Buildings/HOLBuildingTypes.h"
#include "Buildings/HOLProductionSubsystem.h"
#include "Commands/HOLCommandSubsystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "GameFramework/GameStateBase.h"
#include "HAL/IConsoleManager.h"
#include "HeroesOfLibertyLog.h"
#include "Match/HOLPlayerState.h"
#include "MassCommonFragments.h"
#include "MassCommonTypes.h"
#include "MassEntitySubsystem.h"
#include "MassExecutionContext.h"
#include "NavigationSystem.h"
#include "Resources/HOLSourceSubsystem.h"
#include "Simulation/HOLBuildFragments.h"
#include "Simulation/HOLSimulationFragments.h"
#include "Simulation/HOLSimulationSettings.h"
#include "Simulation/HOLUnitSubsystem.h"
#include "Simulation/HOLUnitTypes.h"

namespace UE::HeroesOfLiberty::Gather
{
	int32 LogWalks = 0;
	FAutoConsoleVariableRef CVarLogWalks(TEXT("hol.Gather.LogWalks"), LogWalks,
		TEXT("1: every gathering walk much longer than the straight line is logged at its end (D308: what was walked, from where to where)"));

	int32 LogDeposits = 0;
	FAutoConsoleVariableRef CVarLogDeposits(TEXT("hol.Gather.LogDeposits"), LogDeposits,
		TEXT("1: every unload at a deposit is logged (scripted checks of J5: the stock rises at the deposit only)"));

	float SpeedScale = 1.f;
	FAutoConsoleVariableRef CVarSpeedScale(TEXT("hol.Gather.SpeedScale"), SpeedScale,
		TEXT("Every gatherer gathers this many times as fast, on this server (development: hol.Cheat.GatherSpeed and the cheats' panel set it; D291); ")
		TEXT("1: the table's rates"));

	float SpotSpacing = 100.f;
	FAutoConsoleVariableRef CVarSpotSpacing(TEXT("hol.Gather.SpotSpacing"), SpotSpacing,
		TEXT("Distance (cm) between two gatherers' spots, at one source or two side by side (D309): two Peasants' bodies (60 cm radius) overlapping a little, ")
		TEXT("pressed against a wood's edge as in AoE IV (the user's trial of D309: 70 overlapped by half, too tight); 140: two bodies and a gap, as before"));

	float NearbyMassifRange = static_cast<float>(HOLGather::NearbyMassifDistance);
	FAutoConsoleVariableRef CVarNearbyMassifRange(TEXT("hol.Gather.NearbyMassifRange"), NearbyMassifRange,
		TEXT("Distance (cm) at most between a source of a full massif and a source of a nearby grove of the same resource, where its gatherers ")
		TEXT("look for a free spot next (D309 point 6, the user's choice: ~20 m); 0: none, idle in front of the source at once"));

	/**
	 * Gap (cm) between the bodies of a gatherer standing at its spot and its source: the navmesh ends one agent radius
	 * from a source, a spot a little inside it is reached exactly (on its edge, the unit stopped 12 cm short).
	 */
	constexpr double SpotGap = 15.;
	/**
	 * Gap (cm) between two idle gatherers' bodies at their points in front of a full source, and between those points and
	 * the gatherers' spots (D309): they never overlap, nor stand on a spot.
	 */
	constexpr double StandSpacingGap = 20.;
	/**
	 * A gatherer pushed farther than this (cm, body to body) from its source walks back to its spot (S7: at 60 cm, the
	 * passers-by made the gatherers walk back and forth).
	 */
	constexpr double KeepGatheringGap = 120.;
	/** A walk to the source ends in contact within this gap (cm, body to body): one stopped short by a neighbour still gathers. */
	constexpr double ArrivalGap = 60.;
	/** Distance (cm) the navmesh must be open outward from a spot, away from its source. */
	constexpr double OpenOutwardDistance = 300.;
	/** Arrivals that missed their spot in a row before a gatherer tries a neighbour of its source. */
	constexpr int32 MaxSourceMisses = 3;
	/** Arrivals that missed the deposit in a row before a carrier gives up and stands, its load kept. */
	constexpr int32 MaxDepositMisses = 6;
	/**
	 * Gap (cm) between the body of a gatherer with no room and its source, where it stands idle when no point of the rows
	 * in front is free (D309): the place of an order, among the units standing there.
	 */
	constexpr double StandBeforeGap = 150.;
	/**
	 * Rows of points in front of a source where its idle gatherers stand, one per unit (D309, 3rd trial of J5b.2), looked
	 * through so many at a time, farther out while none is free (D309 point 6: rows without limit).
	 */
	constexpr int32 StandRows = 6;
	/**
	 * Rows at most (some 6,500 points, 90 m out): a bound against a source the navmesh never reaches around, not a limit
	 * met by a match (175 Peasants on one wood stand in the first 10 or so).
	 */
	constexpr int32 MaxStandRows = 64;
	/**
	 * Gap (cm) between a carrier's body and the deposit at its unloading spot, beyond the clearance every body keeps from a
	 * footprint (D310): the spot is on the navmesh, which ends that far from the footprint (and up to ~17 cm more).
	 */
	constexpr double DepositSpotGap = 20.;
	/** Slide (cm) along the deposit's wall of each new try after a missed unloading spot. */
	constexpr double DepositSlide = 150.;
	/**
	 * Spread of the gatherers' unloading spots along the wall, one per unit, as a share of the half side: they do not all
	 * aim at one point (S7: 250 gatherers crowded the middle of a side).
	 */
	constexpr double DepositSpread = 0.85;
	/**
	 * Unloads of one player sent as « +10 » per step at most (J5c.3): 8 at 20 steps a second is more than its HUD shows at
	 * once (AHOLPlayerController::MaxDepositPopups); the rest of a wave of unloads is not shown.
	 */
	constexpr int32 MaxDepositPopupsPerStep = 8;

	const TCHAR* GetResourceName(const EHOLResource Resource)
	{
		switch (Resource)
		{
		case EHOLResource::Food: return TEXT("Food");
		case EHOLResource::Wood: return TEXT("Wood");
		case EHOLResource::Stone: return TEXT("Stone");
		case EHOLResource::Gold: return TEXT("Gold");
		}
		return TEXT("?");
	}
}

UHOLGatherProcessor::FCounters UHOLGatherProcessor::Counters;

double UHOLGatherProcessor::GetSpotSpacing()
{
	return FMath::Max(1., static_cast<double>(UE::HeroesOfLiberty::Gather::SpotSpacing));
}

UHOLGatherProcessor::UHOLGatherProcessor()
	: EntityQuery(*this)
{
	bAutoRegisterWithProcessingPhases = false; // fixed step only (D219)
	ExecutionFlags = static_cast<int32>(EProcessorExecutionFlags::Server | EProcessorExecutionFlags::Standalone);
	ExecutionOrder.ExecuteInGroup = UE::Mass::ProcessorGroupNames::Tasks;
	// After the orders of the step: it sees the arrivals, and a player's order already ended a gathering.
	ExecutionOrder.ExecuteAfter.Add(TEXT("HOLUnitOrderProcessor"));
	ExecutionOrder.ExecuteBefore.Add(TEXT("MassNavMeshPathFollowProcessor"));
	bRequiresGameThreadExecution = true; // the order queue, the sources, the navigation and the players' stocks
}

void UHOLGatherProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddRequirement<FHOLGatherFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FHOLMoveOrderFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FHOLUnitStateFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddConstSharedRequirement<FHOLUnitTypeConstants>(EMassFragmentPresence::All);
	EntityQuery.AddTagRequirement<FHOLLightUnitTag>(EMassFragmentPresence::All);
}

void UHOLGatherProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	UWorld* World = EntityManager.GetWorld();
	FStepContext Step;
	Step.EntityManager = &EntityManager;
	Step.Sources = World ? World->GetSubsystem<UHOLSourceSubsystem>() : nullptr;
	Step.Commands = World ? World->GetSubsystem<UHOLCommandSubsystem>() : nullptr;
	Step.Units = World ? World->GetSubsystem<UHOLUnitSubsystem>() : nullptr;
	Step.Production = World ? World->GetSubsystem<UHOLProductionSubsystem>() : nullptr;
	if (!Step.Sources || !Step.Commands || !Step.Units)
	{
		return;
	}
	Step.NavSystem = FNavigationSystem::GetCurrent<UNavigationSystemV1>(World);
	Step.NavData = Step.NavSystem ? Step.NavSystem->GetDefaultNavDataInstance() : nullptr;
	Step.StepSeconds = GetDefault<UHOLSimulationSettings>()->GetStepSeconds();
	// Spots taken since the last step, and spots left: every source may have room again (D309).
	FullSources.Init(false, Step.Sources->GetSources().Num());

	RefreshDeposits(*World);
	ApplyGatherOrders(Step);

	// Each player's gatherers by resource, counted again at every step (J5b, D307).
	GathererCounts.Reset();
	EntityQuery.ForEachEntityChunk(Context, [this, &Step](FMassExecutionContext& Context)
	{
		const TArrayView<FHOLGatherFragment> Gathers = Context.GetMutableFragmentView<FHOLGatherFragment>();
		const TConstArrayView<FHOLMoveOrderFragment> Moves = Context.GetFragmentView<FHOLMoveOrderFragment>();
		const TArrayView<FHOLUnitStateFragment> States = Context.GetMutableFragmentView<FHOLUnitStateFragment>();
		const TConstArrayView<FTransformFragment> Transforms = Context.GetFragmentView<FTransformFragment>();
		const FHOLUnitTypeConstants& Constants = Context.GetConstSharedFragment<FHOLUnitTypeConstants>();
		for (FMassExecutionContext::FEntityIterator EntityIt = Context.CreateEntityIterator(); EntityIt; ++EntityIt)
		{
			// Settled at the last step's separation (D309).
			Gathers[EntityIt].bSettling = false;
			if (!Gathers[EntityIt].IsGathering())
			{
				continue;
			}
			FGatherer Gatherer{ Context.GetEntity(EntityIt), Gathers[EntityIt], Moves[EntityIt], States[EntityIt],
				Transforms[EntityIt].GetTransform().GetLocation(), States[EntityIt].OwnerPlayer, Constants };
			Advance(Step, Gatherer);
			if (Gatherer.Gather.IsGathering())
			{
				SyncActivity(Step, Gatherer);
				CountGatherer(Step, Gatherer);
			}
		}
	});
	PublishGathererCounts(Step);
	SendDepositPopups(Step);
}

void UHOLGatherProcessor::RefreshDeposits(UWorld& World)
{
	// A few buildings per player: read again at each step, a new one or a lost one counts at once.
	Deposits.Reset();
	for (TActorIterator<AHOLBuilding> It(&World); It; ++It)
	{
		const FHOLBuildingTypeRow* Type = It->GetType();
		// Only a finished building takes deliveries (D312): a camp's foundation is no deposit yet.
		if (!Type || Type->Accepts.IsEmpty() || !It->IsFinished())
		{
			continue;
		}
		FDeposit& Deposit = Deposits.AddDefaulted_GetRef();
		Deposit.Footprint.Centre = FVector2D(It->GetActorLocation());
		Deposit.Footprint.HalfExtent = Type->Footprint * 0.5;
		Deposit.Footprint.Yaw = static_cast<float>(It->GetActorRotation().Yaw);
		Deposit.GroundZ = static_cast<float>(It->GetActorLocation().Z);
		Deposit.OwnerPlayer = It->GetOwnerPlayer();
		Deposit.Accepts = Type->Accepts;
	}
}

void UHOLGatherProcessor::ApplyGatherOrders(const FStepContext& Step)
{
	using namespace UE::HeroesOfLiberty::Gather;
	TArray<FHOLGatherOrder> Orders;
	Step.Commands->ConsumeGatherOrders(Orders);
	FMassEntityManager& EntityManager = *Step.EntityManager;
	for (const FHOLGatherOrder& Order : Orders)
	{
		const TArray<FHOLSource>& Sources = Step.Sources->GetSources();
		if (!Sources.IsValidIndex(Order.SourceIndex) || !Sources[Order.SourceIndex].IsStanding())
		{
			UE_LOG(LogHOLSimulation, Display, TEXT("Gather order: source %d is not standing, ignored"), Order.SourceIndex);
			continue;
		}
		const EHOLResource Resource = Sources[Order.SourceIndex].Resource;
		int32 NumGatherers = 0;
		int32 NumRefused = 0;
		for (const uint32 UnitId : Order.UnitIds)
		{
			const FMassEntityHandle Entity = Step.Units->FindEntity(UnitId);
			if (!EntityManager.IsEntityValid(Entity))
			{
				continue;
			}
			const FHOLUnitStateFragment& State = EntityManager.GetFragmentDataChecked<FHOLUnitStateFragment>(Entity);
			const FHOLUnitTypeConstants& Constants = EntityManager.GetConstSharedFragmentDataChecked<FHOLUnitTypeConstants>(Entity);
			// Only a player's own units obey it (D288), and only those that gather this resource.
			if ((Order.PlayerIndex != INDEX_NONE && State.OwnerPlayer != Order.PlayerIndex)
				|| Constants.CarryCapacity <= 0 || Constants.GatherRates[Resource] <= 0.f)
			{
				++NumRefused;
				continue;
			}
			FHOLGatherFragment& Gather = EntityManager.GetFragmentDataChecked<FHOLGatherFragment>(Entity);
			// It stops building, its queue of sites forgotten (J6a.4, D312).
			if (FHOLBuildFragment* Build = EntityManager.GetFragmentDataPtr<FHOLBuildFragment>(Entity))
			{
				Build->Clear();
			}
			// Another resource: what it carries is kept until its first unit of this one (D306).
			Gather.StartTask(Order.SourceIndex, Resource);
			++NumGatherers;
		}
		UE_LOG(LogHOLSimulation, Display, TEXT("Gather order of slot %d: %d units to source %d (%s)%s"), Order.PlayerIndex, NumGatherers,
			Order.SourceIndex, GetResourceName(Resource), NumRefused > 0 ? *FString::Printf(TEXT(", %d refused"), NumRefused) : TEXT(""));
	}
}

void UHOLGatherProcessor::Advance(const FStepContext& Step, FGatherer& Gatherer)
{
	using namespace UE::HeroesOfLiberty::Gather;
	FHOLGatherFragment& Gather = Gatherer.Gather;
	const TArray<FHOLSource>& Sources = Step.Sources->GetSources();
	const bool bHasSource = Sources.IsValidIndex(Gather.SourceIndex);
	const FHOLSource* Source = bHasSource ? &Sources[Gather.SourceIndex] : nullptr;
	// Its walk is over: the order it gave itself was applied (a new serial), then ended (no phase).
	const bool bArrived = Gather.bWalkIssued && Gatherer.Move.OrderSerial != Gather.OrderSerialBefore && Gatherer.Move.Phase == EHOLMovePhase::None;
	if (Gather.bWalkIssued)
	{
		Gather.Walked += static_cast<float>(FVector::Dist2D(Gatherer.Location, Gather.LastLocation));
		Gather.LastLocation = Gatherer.Location;
		Gather.WalkSteps = static_cast<uint16>(FMath::Min<int32>(Gather.WalkSteps + 1, MAX_uint16));
		const FVector2D Line = FVector2D(Gather.Spot - Gather.WalkStart).GetSafeNormal();
		const FVector2D FromStart(Gatherer.Location - Gather.WalkStart);
		Gather.MaxLateral = FMath::Max(Gather.MaxLateral, static_cast<float>(FMath::Abs(FVector2D::CrossProduct(Line, FromStart))));
		const double Straight = FVector::Dist2D(Gather.WalkStart, Gatherer.Location);
		// Once, at the end of a walk (gathering at its source, it still counts as arrived).
		if (bArrived && Gather.Phase != EHOLGatherPhase::Gathering && LogWalks != 0 && Gather.Walked > Straight * 1.3 + 300.)
		{
			UE_LOG(LogHOLSimulation, Display, TEXT("Gather walk: %s, %.0f cm walked for %.0f cm straight in %.1f s, %.0f cm off the line at most, from (%.0f, %.0f) to (%.0f, %.0f), spot (%.0f, %.0f)"),
				Gather.Phase == EHOLGatherPhase::ToDeposit ? TEXT("to deposit") : TEXT("to source"), Gather.Walked, Straight,
				Gather.WalkSteps * Step.StepSeconds, Gather.MaxLateral, Gather.WalkStart.X,
				Gather.WalkStart.Y, Gatherer.Location.X, Gatherer.Location.Y, Gather.Spot.X, Gather.Spot.Y);
		}
	}

	switch (Gather.Phase)
	{
	case EHOLGatherPhase::ToSource:
	{
		if (!Source || !Source->IsStanding())
		{
			if (Source)
			{
				OnSourceGone(Step, Gatherer);
			}
			else
			{
				Stop(Gatherer);
			}
			return;
		}
		if (HOLGather::IsFullOf(Gather.Load, Source->Resource, Gatherer.Constants.CarryCapacity))
		{
			WalkToDeposit(Step, Gatherer);
			return;
		}
		if (!Gather.bWalkIssued)
		{
			WalkToSource(Step, Gatherer);
			return;
		}
		if (!bArrived)
		{
			return;
		}
		const FHOLGatherSite Site{ FVector2D(Source->Location), Source->Radius };
		if (HOLGather::GetEdgeDistance(FVector2D(Gatherer.Location), Site) - Gatherer.Constants.Radius <= ArrivalGap)
		{
			StartGathering(Gatherer);
			return;
		}
		// Short of its spot (taken by another body, or pushed): another spot, then the nearest source of its massif with a
		// free spot, claimed until its walk there (D309); its massif full, its own source again.
		Gather.bWalkIssued = false;
		++Counters.MissedSpots;
		if (++Gather.Misses >= MaxSourceMisses)
		{
			FVector Spot;
			const int32 Other = FindMassifSpot(Step, Gatherer, Gather.SourceIndex, Spot);
			if (Other != INDEX_NONE)
			{
				Gather.SourceIndex = Other;
				Gather.Spot = Spot;
			}
			Gather.Misses = 0;
		}
		return;
	}
	case EHOLGatherPhase::Gathering:
	{
		if (!Source || !Source->IsStanding())
		{
			if (Source)
			{
				OnSourceGone(Step, Gatherer);
			}
			else
			{
				Stop(Gatherer);
			}
			return;
		}
		const double Gap = HOLGather::GetEdgeDistance(FVector2D(Gatherer.Location), FHOLGatherSite{ FVector2D(Source->Location), Source->Radius })
			- Gatherer.Constants.Radius;
		if (Gap > KeepGatheringGap)
		{
			// Pushed away by a passer-by (D292): back to its spot.
			Gather.Phase = EHOLGatherPhase::ToSource;
			Gather.bWalkIssued = false;
			return;
		}
		int32 Remaining = Source->Remaining;
		const EHOLResource Resource = Source->Resource;
		const EHOLResource Carried = Gather.Load.Resource;
		int32 Lost = 0;
		const float Rate = Gatherer.Constants.GatherRates[Resource] * FMath::Clamp(SpeedScale, 0.1f, 10.f);
		const int32 Taken = HOLGather::Gather(Gather.Load, Resource, Rate, Step.StepSeconds,
			Gatherer.Constants.CarryCapacity, Remaining, &Lost);
		if (Taken > 0)
		{
			// The registry's own count (it leaves the map at 0); Source is changed in place.
			Step.Sources->Take(Gather.SourceIndex, Taken);
		}
		if (Lost > 0 && LogDeposits != 0)
		{
			UE_LOG(LogHOLSimulation, Display, TEXT("Gather: slot %d gathered its first %s, %d %s carried lost"), Gatherer.OwnerPlayer,
				GetResourceName(Resource), Lost, GetResourceName(Carried));
		}
		if (HOLGather::IsFullOf(Gather.Load, Resource, Gatherer.Constants.CarryCapacity))
		{
			WalkToDeposit(Step, Gatherer);
		}
		else if (Remaining <= 0)
		{
			OnSourceGone(Step, Gatherer);
		}
		return;
	}
	case EHOLGatherPhase::ToDeposit:
	{
		if (!Gather.bWalkIssued)
		{
			WalkToDeposit(Step, Gatherer);
			return;
		}
		if (!bArrived)
		{
			return;
		}
		const int32 DepositIndex = FindDeposit(Gatherer.OwnerPlayer, Gather.Load.Resource, Gatherer.Location);
		if (DepositIndex != INDEX_NONE && HOLGather::IsAtDeposit(FVector2D(Gatherer.Location), Gatherer.Constants.Radius, Deposits[DepositIndex].Footprint))
		{
			Unload(Step, Gatherer);
			++Counters.Unloads;
			Gather.Misses = 0;
			if (Source && Source->IsStanding())
			{
				Gather.Phase = EHOLGatherPhase::ToSource;
				Gather.bWalkIssued = false;
			}
			else if (Source)
			{
				OnSourceGone(Step, Gatherer);
			}
			else
			{
				Stop(Gatherer);
			}
			return;
		}
		Gather.bWalkIssued = false;
		++Counters.MissedDeposits;
		if (LogDeposits != 0)
		{
			UE_LOG(LogHOLSimulation, Display, TEXT("Gather: slot %d missed the deposit at (%.0f, %.0f), %.0f cm from its spot, %.0f cm from the footprint"),
				Gatherer.OwnerPlayer, Gatherer.Location.X, Gatherer.Location.Y, FVector::Dist2D(Gatherer.Location, Gather.Spot),
				DepositIndex != INDEX_NONE ? HOLGather::GetDepositDistance(FVector2D(Gatherer.Location), Deposits[DepositIndex].Footprint) : -1.);
		}
		if (++Gather.Misses >= MaxDepositMisses)
		{
			UE_LOG(LogHOLSimulation, Display, TEXT("Gather: a unit of slot %d could not reach a deposit, it stands with its load"), Gatherer.OwnerPlayer);
			Stop(Gatherer);
		}
		return;
	}
	default:
		return;
	}
}

void UHOLGatherProcessor::WalkToSource(const FStepContext& Step, FGatherer& Gatherer)
{
	using namespace UE::HeroesOfLiberty::Gather;
	FHOLGatherFragment& Gather = Gatherer.Gather;
	const TArray<FHOLSource>& Sources = Step.Sources->GetSources();
	const FHOLSource& Source = Sources[Gather.SourceIndex];
	const float BodyRadius = Gatherer.Constants.Radius;

	// Already touching it (a player ordered it to the tree beside it): no walk.
	if (HOLGather::IsInContact(FVector2D(Gatherer.Location), BodyRadius, FHOLGatherSite{ FVector2D(Source.Location), Source.Radius })
		&& !IsSpotTaken(Step, FVector2D(Gatherer.Location), GetSpotSpacing(), Gatherer.Entity))
	{
		Gather.Spot = Gatherer.Location;
		SpotClaims.Add(Gatherer.Entity.AsNumber(), Gather.SourceIndex, FVector2D(Gatherer.Location));
		StartGathering(Gatherer);
		return;
	}

	// Its spot on the side facing the deposit it will unload at (the user's trial of D309, after AoE IV).
	FVector Spot;
	if (FindSourceSpot(Step, Gather.SourceIndex, Gatherer.Location, BodyRadius, Gatherer.Entity, Spot, FindSourceDeposit(Gatherer.OwnerPlayer, Source)))
	{
		IssueWalk(Step, Gatherer, Spot);
		return;
	}
	// Full for it, full for the others too (its own claim was no obstacle).
	if (FullSources.IsValidIndex(Gather.SourceIndex))
	{
		FullSources[Gather.SourceIndex] = true;
	}
	// Its source is surrounded: the nearest source of its massif with a free spot, from it outward (the user's rule at
	// the 3rd trial of J5b.2: the gatherers fill the wood from the tree clicked and surround it, round the patch of
	// berries; a vein alone has no other, D309); the whole massif full, a nearby grove of the same resource (point 6).
	const int32 Other = FindMassifSpot(Step, Gatherer, Gather.SourceIndex, Spot);
	if (Other != INDEX_NONE)
	{
		Gather.SourceIndex = Other;
		IssueWalk(Step, Gatherer, Spot);
		return;
	}
	// The whole massif full, its nearby groves too: idle in front of the source it was sent to, on a point of its own.
	++Counters.NoSpotStops;
	StandBeforeSource(Step, Gatherer, Source);
}

void UHOLGatherProcessor::StandBeforeSource(const FStepContext& Step, FGatherer& Gatherer, const FHOLSource& Source)
{
	using namespace UE::HeroesOfLiberty::Gather;
	// The head of its queue with more after it (Shift, the user's trial of J6a.4): no standing idle here, its massif is
	// used up for it; the builders' step sends it to the next item of its queue, its load kept.
	if (const FHOLBuildFragment* Build = Step.EntityManager->GetFragmentDataPtr<FHOLBuildFragment>(Gatherer.Entity); Build && Build->HasQueuedNext())
	{
		Stop(Gatherer);
		if (LogDeposits != 0)
		{
			UE_LOG(LogHOLSimulation, Display, TEXT("Gather: slot %d found no room in its massif, the next item of its queue"), Gatherer.OwnerPlayer);
		}
		return;
	}
	// In front of it as seen from the gatherer, its load kept: an idle worker (the HUD's counter, the "." key) the player
	// sends elsewhere; it no longer waits there for a spot, unseen (D309).
	FVector2D Away = FVector2D(Gatherer.Location) - FVector2D(Source.Location);
	if (!Away.Normalize())
	{
		Away = FVector2D(1., 0.);
	}
	const float BodyRadius = Gatherer.Constants.Radius;
	FVector Front = Source.Location + FVector(Away * (Source.Radius + BodyRadius + StandBeforeGap), 0.);
	// Its own point, claimed as the gatherers' spots are: two idle gatherers never stand on one point, nor on a
	// gatherer's spot, of this source or of one beside it (the user's 3rd trial of J5b.2: gatherers walking through
	// every unit landed on the idle ones standing there, and two units at rest never part).
	const bool bClaimed = FindStandPoint(Step, Source, Gatherer.Location, BodyRadius, Gatherer.Entity, Front);
	const uint32 UnitId = Step.Units->FindUnitId(Gatherer.Entity);
	Stop(Gatherer);
	if (bClaimed)
	{
		// The claim lasts while it walks there or stands there (IsSpotTaken).
		Gatherer.Gather.Spot = Front;
		Gatherer.Gather.OrderSerialBefore = Gatherer.Move.OrderSerial;
	}
	if (UnitId != 0)
	{
		const uint32 UnitIds[] = { UnitId };
		// To its point exactly; with none free, among the units standing in front as for a player's order.
		Step.Commands->IssueMoveOrder(UnitIds, Front, INDEX_NONE, /*bFromTask*/ bClaimed);
	}
	if (LogDeposits != 0)
	{
		UE_LOG(LogHOLSimulation, Display, TEXT("Gather: slot %d found no room in its massif, it stands idle in front of it at (%.0f, %.0f)%s"),
			Gatherer.OwnerPlayer, Front.X, Front.Y, bClaimed ? TEXT("") : TEXT(", no point of its own free"));
	}
}

bool UHOLGatherProcessor::FindStandPoint(const FStepContext& Step, const FHOLSource& Source, const FVector& From, const float BodyRadius,
	const FMassEntityHandle Self, FVector& OutPoint)
{
	using namespace UE::HeroesOfLiberty::Gather;
	const FVector2D Centre(Source.Location);
	const double SpotOffset = BodyRadius + SpotGap;
	// Clear of the circle of spots, however close they stand on it (hol.Gather.SpotSpacing): that rule is the idle ones'.
	const double Spacing = 2. * BodyRadius + StandSpacingGap;
	const double SpotDistance = Source.Radius + SpotOffset;

	// The sources whose spots a row may meet: none of their spots is stood on, used or not. Gathered as the rows go farther
	// out, those beyond the last reach added to the ones already met.
	TArray<FHOLGatherSite, TInlineAllocator<32>> Sites;
	double SitesReach = -1.;
	TArray<FVector2D> Points;
	const FVector Extent(25., 25., 300.);
	for (int32 FirstRow = 0; FirstRow < MaxStandRows; FirstRow += StandRows)
	{
		const double Reach = SpotDistance + (FirstRow + StandRows + 1) * Spacing;
		for (const FHOLSource& Other : Step.Sources->GetSources())
		{
			const double Within = Reach + Other.Radius + SpotOffset + Spacing;
			const double Before = SitesReach + Other.Radius + SpotOffset + Spacing;
			const double DistanceSquared = FVector::DistSquared2D(Other.Location, Source.Location);
			if (Other.IsStanding() && DistanceSquared <= FMath::Square(Within) && (SitesReach < 0. || DistanceSquared > FMath::Square(Before)))
			{
				Sites.Add(FHOLGatherSite{ FVector2D(Other.Location), Other.Radius });
			}
		}
		SitesReach = Reach;
		HOLGather::GetStandPoints(Centre, SpotDistance, Spacing, FVector2D(From) - Centre, StandRows, Points, FirstRow);
		for (const FVector2D& Point : Points)
		{
			if (!HOLGather::IsClearOfSpots(Point, Sites, SpotOffset, Spacing) || IsSpotTaken(Step, Point, Spacing, Self))
			{
				continue;
			}
			FVector Found(Point, Source.Location.Z);
			if (Step.NavSystem && Step.NavData)
			{
				FNavLocation OnNav;
				if (!Step.NavSystem->ProjectPointToNavigation(Found, OnNav, Extent, Step.NavData) || FVector::Dist2D(OnNav.Location, Found) > 20.)
				{
					continue;
				}
				// Not on one of the wood's islands no path reaches (as the spots, FindSourceSpot).
				const FVector2D Out = (Point - Centre).GetSafeNormal() * OpenOutwardDistance;
				FVector Hit;
				if (Step.NavData->Raycast(OnNav.Location, OnNav.Location + FVector(Out, 0.), Hit, Step.NavData->GetDefaultQueryFilter()))
				{
					continue;
				}
				Found = OnNav.Location;
				if (!HOLGather::IsClearOfSpots(FVector2D(Found), Sites, SpotOffset, Spacing) || IsSpotTaken(Step, FVector2D(Found), Spacing, Self))
				{
					continue;
				}
			}
			SpotClaims.Add(Self.AsNumber(), INDEX_NONE, FVector2D(Found));
			OutPoint = Found;
			return true;
		}
	}
	return false;
}

void UHOLGatherProcessor::WalkToDeposit(const FStepContext& Step, FGatherer& Gatherer)
{
	using namespace UE::HeroesOfLiberty::Gather;
	FHOLGatherFragment& Gather = Gatherer.Gather;
	Gather.Phase = EHOLGatherPhase::ToDeposit;
	const int32 DepositIndex = FindDeposit(Gatherer.OwnerPlayer, Gather.Load.Resource, Gatherer.Location);
	if (DepositIndex == INDEX_NONE)
	{
		UE_LOG(LogHOLSimulation, Display, TEXT("Gather: slot %d has no deposit for %s, its gatherer stands with its load"), Gatherer.OwnerPlayer,
			GetResourceName(Gather.Load.Resource));
		Stop(Gatherer);
		return;
	}
	const FDeposit& Deposit = Deposits[DepositIndex];
	// A first try in front of it; a missed one slides along the wall, one side then the other.
	const int32 Slide = (Gather.Misses + 1) / 2;
	const FVector2D Local = (FVector2D(Gatherer.Location) - Deposit.Footprint.Centre).GetRotated(-Deposit.Footprint.Yaw);
	const double HalfSide = FMath::Abs(Local.X) - Deposit.Footprint.HalfExtent.X >= FMath::Abs(Local.Y) - Deposit.Footprint.HalfExtent.Y
		? Deposit.Footprint.HalfExtent.Y : Deposit.Footprint.HalfExtent.X;
	const double Spread = (static_cast<double>(GetTypeHash(Gatherer.Entity) % 1001u) / 500. - 1.) * DepositSpread * HalfSide;
	const double Along = Spread + (Gather.Misses % 2 == 0 ? 1. : -1.) * Slide * DepositSlide;
	const FVector2D Spot = HOLGather::GetDepositSpot(FVector2D(Gatherer.Location), Deposit.Footprint, Gatherer.Constants.Radius + UHOLBuildingSettings::GetBodyClearance() + DepositSpotGap,
		Along);
	IssueWalk(Step, Gatherer, FVector(Spot, Deposit.GroundZ));
}

void UHOLGatherProcessor::OnSourceGone(const FStepContext& Step, FGatherer& Gatherer)
{
	FHOLGatherFragment& Gather = Gatherer.Gather;
	Gather.bWalkIssued = false;
	Gather.Misses = 0;
	// The nearest source of its massif with a free spot, from the used-up one outward (a tree felled: the nearest free tree
	// of the wood), else of a nearby grove (D309 point 6), claimed until its walk there, its spot found again then (D309).
	// The source's resource: it may still carry another one (D306).
	FVector Spot;
	int32 NearestStanding = INDEX_NONE;
	const int32 Other = FindMassifSpot(Step, Gatherer, Gather.SourceIndex, Spot, &NearestStanding);
	if (Other != INDEX_NONE)
	{
		Gather.SourceIndex = Other;
		Gather.Spot = Spot;
		Gather.Phase = EHOLGatherPhase::ToSource;
		return;
	}
	if (NearestStanding != INDEX_NONE)
	{
		// The whole massif full: idle in front of the nearest source still standing, its load kept.
		++Counters.NoSpotStops;
		StandBeforeSource(Step, Gatherer, Step.Sources->GetSources()[NearestStanding]);
		return;
	}
	Gather.SourceIndex = INDEX_NONE;
	if (Gather.Load.Amount > 0)
	{
		// Nothing left to gather in its massif: it unloads what it carries, then stands (D296).
		WalkToDeposit(Step, Gatherer);
		return;
	}
	Stop(Gatherer);
}

void UHOLGatherProcessor::Unload(const FStepContext& Step, FGatherer& Gatherer)
{
	using namespace UE::HeroesOfLiberty::Gather;
	FHOLGatherFragment& Gather = Gatherer.Gather;
	const EHOLResource Resource = Gather.Load.Resource;
	AHOLPlayerState* PlayerState = Step.Production ? Step.Production->FindPlayerState(Gatherer.OwnerPlayer) : nullptr;
	FHOLResources Stock = PlayerState ? PlayerState->GetResources() : FHOLResources();
	const int32 Amount = HOLGather::Deposit(Gather.Load, Stock);
	if (PlayerState && Amount > 0)
	{
		PlayerState->SetResources(Stock);
	}
	Step.Sources->RecordDeposit(Gatherer.OwnerPlayer, Resource, Amount);
	if (Amount > 0)
	{
		// Its « +10 », for its owner alone, sent at the end of the step with the owner's other unloads (J5c.3).
		if (!DepositPopups.IsValidIndex(Gatherer.OwnerPlayer))
		{
			DepositPopups.SetNum(Gatherer.OwnerPlayer + 1);
		}
		TArray<FHOLDepositPopup>& Popups = DepositPopups[Gatherer.OwnerPlayer];
		if (Popups.Num() < MaxDepositPopupsPerStep)
		{
			FHOLDepositPopup& Popup = Popups.AddDefaulted_GetRef();
			Popup.Location = Gatherer.Location;
			Popup.Resource = Resource;
			Popup.Amount = static_cast<uint8>(FMath::Min(Amount, 255));
		}
	}
	if (LogDeposits != 0)
	{
		UE_LOG(LogHOLSimulation, Display, TEXT("Gather: slot %d unloaded %d %s (stock %s)"), Gatherer.OwnerPlayer, Amount, GetResourceName(Resource),
			PlayerState ? *Stock.ToString() : TEXT("none"));
	}
}

void UHOLGatherProcessor::StartGathering(FGatherer& Gatherer)
{
	Gatherer.Gather.Phase = EHOLGatherPhase::Gathering;
	Gatherer.Gather.Misses = 0;
	Gatherer.Gather.bSettling = true;
	Gatherer.State.Activity = EHOLUnitActivity::Gathering;
}

void UHOLGatherProcessor::SyncActivity(const FStepContext& Step, FGatherer& Gatherer)
{
	// Its round trip shows as one activity from the order to the end (D307): the walks it gives itself start at the next
	// step and end idle (UHOLUnitOrderProcessor), the waits for a spot stand; none of that is idleness.
	Gatherer.State.Activity = Gatherer.Gather.Phase == EHOLGatherPhase::Gathering ? EHOLUnitActivity::Gathering : EHOLUnitActivity::GatherTrip;
	if (Gatherer.State.Activity == EHOLUnitActivity::Gathering)
	{
		// What its gesture shows (J5c.1): its source's resource, the one it takes now (its load may still be another, D306).
		const TArray<FHOLSource>& Sources = Step.Sources->GetSources();
		Gatherer.State.GatherResource = Sources.IsValidIndex(Gatherer.Gather.SourceIndex) ? Sources[Gatherer.Gather.SourceIndex].Resource
			: Gatherer.Gather.Load.ProgressResource;
	}
}

void UHOLGatherProcessor::CountGatherer(const FStepContext& Step, const FGatherer& Gatherer)
{
	// Its source's resource; on its last way to a deposit (its source gone), what it carries.
	const TArray<FHOLSource>& Sources = Step.Sources->GetSources();
	const EHOLResource Resource = Sources.IsValidIndex(Gatherer.Gather.SourceIndex) ? Sources[Gatherer.Gather.SourceIndex].Resource
		: Gatherer.Gather.Load.Resource;
	if (!GathererCounts.IsValidIndex(Gatherer.OwnerPlayer))
	{
		GathererCounts.SetNum(Gatherer.OwnerPlayer + 1);
	}
	++GathererCounts[Gatherer.OwnerPlayer][Resource];
}

void UHOLGatherProcessor::PublishGathererCounts(const FStepContext& Step) const
{
	const AGameStateBase* GameState = Step.Production ? Step.Production->GetWorld()->GetGameState() : nullptr;
	if (!GameState)
	{
		return;
	}
	// Every player, those with no gatherer left too (each state sends only when its counts changed).
	for (APlayerState* PlayerState : GameState->PlayerArray)
	{
		AHOLPlayerState* HOLPlayerState = Cast<AHOLPlayerState>(PlayerState);
		const int32 Slot = HOLPlayerState ? HOLPlayerState->GetPlayerIndex() : INDEX_NONE;
		if (Slot != INDEX_NONE)
		{
			HOLPlayerState->SetGatherers(GathererCounts.IsValidIndex(Slot) ? GathererCounts[Slot] : FHOLResources());
		}
	}
}

void UHOLGatherProcessor::SendDepositPopups(const FStepContext& Step)
{
	for (int32 Slot = 0; Slot < DepositPopups.Num(); ++Slot)
	{
		TArray<FHOLDepositPopup>& Popups = DepositPopups[Slot];
		if (Popups.IsEmpty())
		{
			continue;
		}
		// Its own controller only: a client call goes to the owning connection (in solo, it runs here). An AI has none.
		const AHOLPlayerState* PlayerState = Step.Production ? Step.Production->FindPlayerState(Slot) : nullptr;
		if (AHOLPlayerController* Controller = PlayerState ? Cast<AHOLPlayerController>(PlayerState->GetPlayerController()) : nullptr)
		{
			Controller->ClientShowDepositPopups(Popups);
		}
		Popups.Reset();
	}
}

void UHOLGatherProcessor::Stop(FGatherer& Gatherer)
{
	if (Gatherer.State.Activity == EHOLUnitActivity::Gathering || Gatherer.State.Activity == EHOLUnitActivity::GatherTrip)
	{
		// Stopped on a walk it gave itself, it ends that walk first.
		Gatherer.State.Activity = Gatherer.Move.Phase != EHOLMovePhase::None ? EHOLUnitActivity::Moving : EHOLUnitActivity::Idle;
	}
	// Stopped where its round trip left it, having walked through every unit (D309; its walk's end already made it idle,
	// UHOLUnitOrderProcessor): it looks once, as a walker, and parts from a unit it stands on (J5b.3: Peasants idle at the
	// centre 29 cm apart once their berries were used up).
	Gatherer.Gather.bSettling = true;
	Gatherer.Gather.Phase = EHOLGatherPhase::None;
	Gatherer.Gather.SourceIndex = INDEX_NONE;
	Gatherer.Gather.bWalkIssued = false;
	Gatherer.Gather.Misses = 0;
}

void UHOLGatherProcessor::IssueWalk(const FStepContext& Step, FGatherer& Gatherer, const FVector& Spot)
{
	const uint32 UnitId = Step.Units->FindUnitId(Gatherer.Entity);
	if (UnitId == 0)
	{
		Stop(Gatherer);
		return;
	}
	FHOLGatherFragment& Gather = Gatherer.Gather;
	++(Gather.Phase == EHOLGatherPhase::ToDeposit ? Counters.WalksToDeposit : Counters.WalksToSource);
	Gather.Spot = Spot;
	Gather.WalkStart = Gatherer.Location;
	Gather.LastLocation = Gatherer.Location;
	Gather.Walked = 0.f;
	Gather.MaxLateral = 0.f;
	Gather.WalkSteps = 0;
	Gather.OrderSerialBefore = Gatherer.Move.OrderSerial;
	Gather.bWalkIssued = true;
	const uint32 UnitIds[] = { UnitId };
	Step.Commands->IssueMoveOrder(UnitIds, Spot, INDEX_NONE, /*bFromTask*/ true);
}

bool UHOLGatherProcessor::FindSourceSpot(const FStepContext& Step, const int32 SourceIndex, const FVector& From, const float BodyRadius,
	const FMassEntityHandle Self, FVector& OutSpot, const FHOLGatherDeposit* Deposit)
{
	using namespace UE::HeroesOfLiberty::Gather;
	const FHOLSource& Source = Step.Sources->GetSources()[SourceIndex];
	const FVector2D Centre(Source.Location);
	const double Distance = Source.Radius + BodyRadius + SpotGap;
	// 100 cm (hol.Gather.SpotSpacing): two gatherers side by side overlap a little, never on one point.
	const double MinSpacing = GetSpotSpacing();

	// The side facing the deposit first (the point of its footprint nearest the source); with none, the gatherer's side.
	FVector2D Towards = Deposit ? HOLGather::GetDepositSpot(Centre, *Deposit, 0.) - Centre : FVector2D(From) - Centre;
	if (!Towards.Normalize())
	{
		Towards = FVector2D(1., 0.);
	}
	TArray<FVector2D> Spots;
	HOLGather::GetSourceSpots(Centre, Distance, MinSpacing, Towards, Spots);
	const FVector Extent(25., 25., 300.);
	for (const FVector2D& Spot : Spots)
	{
		// Spots of this source and of its neighbours alike (two trees side by side, D309).
		if (IsSpotTaken(Step, Spot, MinSpacing, Self))
		{
			continue;
		}
		FVector Found(Spot, Source.Location.Z);
		if (Step.NavSystem && Step.NavData)
		{
			// Between the trees of a wood, the navmesh is closed: only the spots on its edge are open (as in AoE).
			FNavLocation OnNav;
			if (!Step.NavSystem->ProjectPointToNavigation(Found, OnNav, Extent, Step.NavData) || FVector::Dist2D(OnNav.Location, Found) > 20.)
			{
				continue;
			}
			// Inside a wood, between four trees, the navmesh keeps small islands no path reaches: a spot is taken only
			// if the navmesh is open outward from the source (a unit pushed into an island could not leave it).
			const FVector2D Out = (Spot - Centre).GetSafeNormal() * OpenOutwardDistance;
			FVector Hit;
			if (Step.NavData->Raycast(OnNav.Location, OnNav.Location + FVector(Out, 0.), Hit, Step.NavData->GetDefaultQueryFilter()))
			{
				continue;
			}
			Found = OnNav.Location;
			// Where it will stand, up to 20 cm off: the spacing holds there too.
			if (IsSpotTaken(Step, FVector2D(Found), MinSpacing, Self))
			{
				continue;
			}
		}
		// The point it walks to and stands on (Gather.Spot): the claim lasts as long as that one (D309; the point off
		// the navmesh was claimed before, its claim lost once the gatherer's spot differed by more than 1 cm).
		SpotClaims.Add(Self.AsNumber(), SourceIndex, FVector2D(Found));
		OutSpot = Found;
		return true;
	}
	return false;
}

bool UHOLGatherProcessor::IsSpotTaken(const FStepContext& Step, const FVector2D& Spot, const double MinSpacing, const FMassEntityHandle Self)
{
	// A claim holds while its gatherer walks to that spot or gathers there, at the same source; an idle gatherer's point
	// in front of a full source, while it walks there or stands there, sent nowhere else (D309).
	const FMassEntityManager& EntityManager = *Step.EntityManager;
	return SpotClaims.IsTaken(Spot, MinSpacing, Self.AsNumber(), [&EntityManager](const FHOLSpotClaims::FClaim& Claim)
	{
		const FMassEntityHandle Entity = FMassEntityHandle::FromNumber(Claim.Owner);
		if (!EntityManager.IsEntityValid(Entity))
		{
			return true;
		}
		const FHOLGatherFragment* Gather = EntityManager.GetFragmentDataPtr<FHOLGatherFragment>(Entity);
		if (Gather && Claim.SourceIndex == INDEX_NONE)
		{
			// Its walk there not applied yet (the same order serial), or applied: its slot is that point.
			const FHOLMoveOrderFragment* Move = EntityManager.GetFragmentDataPtr<FHOLMoveOrderFragment>(Entity);
			return !Move || Gather->Phase != EHOLGatherPhase::None || FVector2D::DistSquared(FVector2D(Gather->Spot), Claim.Spot) > 1.
				|| (Move->OrderSerial != Gather->OrderSerialBefore && FVector2D::DistSquared(FVector2D(Move->Slot), Claim.Spot) > 1.);
		}
		return !Gather || Gather->SourceIndex != Claim.SourceIndex
			|| (Gather->Phase != EHOLGatherPhase::ToSource && Gather->Phase != EHOLGatherPhase::Gathering)
			|| FVector2D::DistSquared(FVector2D(Gather->Spot), Claim.Spot) > 1.;
	});
}

int32 UHOLGatherProcessor::FindMassifSpot(const FStepContext& Step, const FGatherer& Gatherer, const int32 FromSource, FVector& OutSpot,
	int32* OutNearestStanding)
{
	const TArray<FHOLSource>& Sources = Step.Sources->GetSources();
	if (OutNearestStanding)
	{
		*OutNearestStanding = INDEX_NONE;
	}
	if (!Sources.IsValidIndex(FromSource))
	{
		return INDEX_NONE;
	}
	// The edge facing the deposit it will unload at first, from FromSource along it (the user's trial of D309, after AoE
	// IV: the wood's other side only once that edge is full), and each source's spots facing that deposit.
	// Its whole massif full, its nearby groves next (D309 point 6), in the same order.
	const FHOLGatherDeposit* Deposit = FindSourceDeposit(Gatherer.OwnerPlayer, Sources[FromSource]);
	const FHOLSourceMassifs& SourceMassifs = GetMassifs(Step);
	const int32 OwnMassif = SourceMassifs.GetMassif(FromSource);
	const int32 Found = SourceMassifs.Search(FromSource, [this, &Step, &Gatherer, &Sources, &OutSpot, OutNearestStanding, Deposit, &SourceMassifs, OwnMassif](const int32 Index)
	{
		// A used-up source leads on to the ones beyond it: the massif is not cut.
		const FHOLSource& Source = Sources[Index];
		if (!Source.IsStanding())
		{
			return false;
		}
		// The one to stand in front of is of its own massif: a massif used up still sends its gatherers home, its loads
		// unloaded, not to stand at a full grove.
		if (OutNearestStanding && *OutNearestStanding == INDEX_NONE && SourceMassifs.GetMassif(Index) == OwnMassif)
		{
			*OutNearestStanding = Index;
		}
		if (FullSources.IsValidIndex(Index) && FullSources[Index])
		{
			return false;
		}
		if (FindSourceSpot(Step, Index, Gatherer.Location, Gatherer.Constants.Radius, Gatherer.Entity, OutSpot, Deposit))
		{
			return true;
		}
		if (FullSources.IsValidIndex(Index))
		{
			FullSources[Index] = true;
		}
		return false;
	}, Deposit);
	if (Found != INDEX_NONE && SourceMassifs.GetMassif(Found) != OwnMassif)
	{
		++Counters.NearbyGroves;
		if (UE::HeroesOfLiberty::Gather::LogDeposits != 0)
		{
			UE_LOG(LogHOLSimulation, Display, TEXT("Gather: slot %d found no room in the massif of source %d, sent to source %d of a nearby grove, %.0f cm away"),
				Gatherer.OwnerPlayer, FromSource, Found, FVector::Dist2D(Sources[FromSource].Location, Sources[Found].Location));
		}
	}
	return Found;
}

const FHOLGatherDeposit* UHOLGatherProcessor::FindSourceDeposit(const uint8 OwnerPlayer, const FHOLSource& Source) const
{
	const int32 DepositIndex = FindDeposit(OwnerPlayer, Source.Resource, Source.Location);
	return DepositIndex == INDEX_NONE ? nullptr : &Deposits[DepositIndex].Footprint;
}

const FHOLSourceMassifs& UHOLGatherProcessor::GetMassifs(const FStepContext& Step)
{
	// Sources are only added, never moved, a used-up one kept in the list: the same count, the same links (the nearby
	// groves linked again when hol.Gather.NearbyMassifRange changes).
	const TArray<FHOLSource>& Sources = Step.Sources->GetSources();
	const double NearbyRange = FMath::Max(0., static_cast<double>(UE::HeroesOfLiberty::Gather::NearbyMassifRange));
	if (Massifs.Num() != Sources.Num() || MassifsNearbyRange != NearbyRange)
	{
		MassifsNearbyRange = NearbyRange;
		TArray<FHOLGatherSource> Sites;
		Sites.Reserve(Sources.Num());
		for (const FHOLSource& Source : Sources)
		{
			FHOLGatherSource& Site = Sites.AddDefaulted_GetRef();
			Site.Site = FHOLGatherSite{ FVector2D(Source.Location), Source.Radius };
			Site.Resource = Source.Resource;
			Site.Remaining = Source.Remaining;
		}
		Massifs.Build(Sites, HOLGather::NoRoomSourceDistance, NearbyRange);
	}
	return Massifs;
}

int32 UHOLGatherProcessor::FindDeposit(const uint8 OwnerPlayer, const EHOLResource Resource, const FVector& From) const
{
	TArray<FHOLGatherDeposit> Candidates;
	TArray<int32> Indices;
	for (int32 Index = 0; Index < Deposits.Num(); ++Index)
	{
		if (Deposits[Index].OwnerPlayer == OwnerPlayer && Deposits[Index].Accepts.Contains(Resource))
		{
			Candidates.Add(Deposits[Index].Footprint);
			Indices.Add(Index);
		}
	}
	const int32 Nearest = HOLGather::FindNearestDeposit(FVector2D(From), Candidates);
	return Nearest == INDEX_NONE ? INDEX_NONE : Indices[Nearest];
}

namespace UE::HeroesOfLiberty::Gather
{
	/** Scripted tries of J5 (D299): units sent to gather the source nearest a ground point, as one order. */
	FAutoConsoleCommand GatherCommand(TEXT("hol.Units.Gather"),
		TEXT("hol.Units.Gather X Y [Slot]: the units (of one slot, or all) gather the standing source nearest the ground point X Y (cm) (server or solo)"),
		FConsoleCommandWithArgsDelegate::CreateLambda([](const TArray<FString>& Args)
		{
			if (Args.Num() < 2)
			{
				UE_LOG(LogHOLSimulation, Warning, TEXT("hol.Units.Gather X Y [Slot]"));
				return;
			}
			const FVector Point(FCString::Atod(*Args[0]), FCString::Atod(*Args[1]), 0.);
			const int32 Slot = Args.Num() >= 3 ? FCString::Atoi(*Args[2]) : INDEX_NONE;
			for (const FWorldContext& WorldContext : GEngine->GetWorldContexts())
			{
				UWorld* World = WorldContext.World();
				UHOLUnitSubsystem* Units = World && World->IsGameWorld() && World->GetNetMode() != NM_Client ? World->GetSubsystem<UHOLUnitSubsystem>() : nullptr;
				UHOLCommandSubsystem* Commands = World ? World->GetSubsystem<UHOLCommandSubsystem>() : nullptr;
				UHOLSourceSubsystem* Sources = World ? World->GetSubsystem<UHOLSourceSubsystem>() : nullptr;
				UMassEntitySubsystem* EntitySubsystem = World ? World->GetSubsystem<UMassEntitySubsystem>() : nullptr;
				if (!Units || !Commands || !Sources || !EntitySubsystem)
				{
					continue;
				}
				const int32 SourceIndex = Sources->FindNearestStanding(Point);
				if (SourceIndex == INDEX_NONE)
				{
					UE_LOG(LogHOLSimulation, Display, TEXT("Gather order: no standing source"));
					continue;
				}
				const FMassEntityManager& EntityManager = EntitySubsystem->GetEntityManager();
				TArray<uint32> UnitIds;
				Units->GetAllUnitIds(UnitIds);
				UnitIds.RemoveAll([&](const uint32 UnitId)
				{
					const FHOLUnitStateFragment* State = EntityManager.GetFragmentDataPtr<FHOLUnitStateFragment>(Units->FindEntity(UnitId));
					return State == nullptr || (Slot != INDEX_NONE && State->OwnerPlayer != Slot);
				});
				Commands->IssueGatherOrder(UnitIds, SourceIndex);
			}
		}));

	/** Where every gatherer is in its round trip, per slot (server or solo). */
	FAutoConsoleCommand ReportCommand(TEXT("hol.Gather.Report"),
		TEXT("hol.Gather.Report [Slot]: per slot, the gatherers to their source, gathering, to the deposit, what they carry and what they unloaded (server or solo)"),
		FConsoleCommandWithArgsDelegate::CreateLambda([](const TArray<FString>& Args)
		{
			const int32 OnlySlot = Args.IsEmpty() ? INDEX_NONE : FCString::Atoi(*Args[0]);
			for (const FWorldContext& WorldContext : GEngine->GetWorldContexts())
			{
				UWorld* World = WorldContext.World();
				const UHOLUnitSubsystem* Units = World && World->IsGameWorld() && World->GetNetMode() != NM_Client ? World->GetSubsystem<UHOLUnitSubsystem>() : nullptr;
				const UHOLSourceSubsystem* Sources = World ? World->GetSubsystem<UHOLSourceSubsystem>() : nullptr;
				const UMassEntitySubsystem* EntitySubsystem = World ? World->GetSubsystem<UMassEntitySubsystem>() : nullptr;
				if (!Units || !Sources || !EntitySubsystem)
				{
					continue;
				}
				struct FSlotCounts
				{
					int32 Phases[4] = {};
					int32 IdleCarrying = 0;
					int32 Carried = 0;
				};
				TMap<int32, FSlotCounts> Counts;
				const FMassEntityManager& EntityManager = EntitySubsystem->GetEntityManager();
				TArray<uint32> UnitIds;
				Units->GetAllUnitIds(UnitIds);
				for (const uint32 UnitId : UnitIds)
				{
					const FMassEntityHandle Entity = Units->FindEntity(UnitId);
					const FHOLUnitStateFragment* State = EntityManager.GetFragmentDataPtr<FHOLUnitStateFragment>(Entity);
					const FHOLGatherFragment* Gather = EntityManager.GetFragmentDataPtr<FHOLGatherFragment>(Entity);
					if (!State || !Gather || (OnlySlot != INDEX_NONE && State->OwnerPlayer != OnlySlot))
					{
						continue;
					}
					FSlotCounts& Slot = Counts.FindOrAdd(State->OwnerPlayer);
					++Slot.Phases[static_cast<int32>(Gather->Phase)];
					Slot.Carried += Gather->Load.Amount;
					Slot.IdleCarrying += !Gather->IsGathering() && Gather->Load.Amount > 0 ? 1 : 0;
				}
				Counts.KeySort(TLess<int32>());
				for (const TPair<int32, FSlotCounts>& Pair : Counts)
				{
					const FSlotCounts& Slot = Pair.Value;
					UE_LOG(LogHOLSimulation, Display, TEXT("Gather: slot %d to source %d, gathering %d, to deposit %d, not gathering %d (%d with a load), carrying %d, unloaded %s"),
						Pair.Key, Slot.Phases[static_cast<int32>(EHOLGatherPhase::ToSource)], Slot.Phases[static_cast<int32>(EHOLGatherPhase::Gathering)],
						Slot.Phases[static_cast<int32>(EHOLGatherPhase::ToDeposit)], Slot.Phases[static_cast<int32>(EHOLGatherPhase::None)], Slot.IdleCarrying,
						Slot.Carried, *Sources->GetDeposited(static_cast<uint8>(Pair.Key)).ToString());
				}
				// Since the program started, every slot (D309): sent to a nearby grove, stopped idle in front of a full source.
				const UHOLGatherProcessor::FCounters& Counters = UHOLGatherProcessor::GetCounters();
				UE_LOG(LogHOLSimulation, Display, TEXT("Gather: since the start, %lld sent to a nearby grove, %lld stopped idle with no room"),
					Counters.NearbyGroves, Counters.NoSpotStops);
			}
		}));
}
