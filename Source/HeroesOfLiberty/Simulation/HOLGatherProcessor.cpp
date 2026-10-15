// Copyright Epic Games, Inc. All Rights Reserved.

#include "Simulation/HOLGatherProcessor.h"

#include "Buildings/HOLBuilding.h"
#include "Buildings/HOLBuildingTypes.h"
#include "Buildings/HOLProductionSubsystem.h"
#include "Commands/HOLCommandSubsystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "HAL/IConsoleManager.h"
#include "HeroesOfLibertyLog.h"
#include "Match/HOLPlayerState.h"
#include "MassCommonFragments.h"
#include "MassCommonTypes.h"
#include "MassEntitySubsystem.h"
#include "MassExecutionContext.h"
#include "NavigationSystem.h"
#include "Resources/HOLSourceSubsystem.h"
#include "Simulation/HOLSimulationFragments.h"
#include "Simulation/HOLSimulationSettings.h"
#include "Simulation/HOLUnitSubsystem.h"
#include "Simulation/HOLUnitTypes.h"

namespace UE::HeroesOfLiberty::Gather
{
	int32 LogDeposits = 0;
	FAutoConsoleVariableRef CVarLogDeposits(TEXT("hol.Gather.LogDeposits"), LogDeposits,
		TEXT("1: every unload at a deposit is logged (scripted checks of J5: the stock rises at the deposit only)"));

	/**
	 * Gap (cm) between the bodies of a gatherer standing at its spot and its source: the navmesh ends one agent radius
	 * from a source, a spot a little inside it is reached exactly (on its edge, the unit stopped 12 cm short).
	 */
	constexpr double SpotGap = 15.;
	/** Gap (cm) between two gatherers' bodies at their spots around a source. */
	constexpr double SpotSpacingGap = 20.;
	/**
	 * A gatherer pushed farther than this (cm, body to body) from its source walks back to its spot (S7: at 60 cm, the
	 * passers-by made the gatherers walk back and forth).
	 */
	constexpr double KeepGatheringGap = 120.;
	/** A walk to the source ends in contact within this gap (cm, body to body): one stopped short by a neighbour still gathers. */
	constexpr double ArrivalGap = 60.;
	/** Distance (cm) the navmesh must be open outward from a spot, away from its source. */
	constexpr double OpenOutwardDistance = 300.;
	/**
	 * Largest angle (radians) between two spots tried around a source, from the side facing the gatherer: 30 degrees
	 * around a tree or a bush; around a vein (J5b.1), as close as two gatherers stand, so that more of them fit.
	 */
	constexpr double MaxSpotStep = UE_DOUBLE_PI / 6.;
	/** Arrivals that missed their spot in a row before a gatherer tries a neighbour of its source. */
	constexpr int32 MaxSourceMisses = 3;
	/** Arrivals that missed the deposit in a row before a carrier gives up and stands, its load kept. */
	constexpr int32 MaxDepositMisses = 6;
	/** Steps a gatherer waits when no spot is free at its source nor at its neighbours (3 s: each try looks around them all). */
	constexpr uint16 NoSpotWaitSteps = 60;
	/** Gap (cm) between a carrier's body and the deposit at its unloading spot. */
	constexpr double DepositSpotGap = 20.;
	/** Slide (cm) along the deposit's wall of each new try after a missed unloading spot. */
	constexpr double DepositSlide = 150.;
	/**
	 * Spread of the gatherers' unloading spots along the wall, one per unit, as a share of the half side: they do not all
	 * aim at one point (S7: 250 gatherers crowded the middle of a side).
	 */
	constexpr double DepositSpread = 0.85;

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

	RefreshDeposits(*World);
	ApplyGatherOrders(Step);

	EntityQuery.ForEachEntityChunk(Context, [this, &Step](FMassExecutionContext& Context)
	{
		const TArrayView<FHOLGatherFragment> Gathers = Context.GetMutableFragmentView<FHOLGatherFragment>();
		const TConstArrayView<FHOLMoveOrderFragment> Moves = Context.GetFragmentView<FHOLMoveOrderFragment>();
		const TArrayView<FHOLUnitStateFragment> States = Context.GetMutableFragmentView<FHOLUnitStateFragment>();
		const TConstArrayView<FTransformFragment> Transforms = Context.GetFragmentView<FTransformFragment>();
		const FHOLUnitTypeConstants& Constants = Context.GetConstSharedFragment<FHOLUnitTypeConstants>();
		for (FMassExecutionContext::FEntityIterator EntityIt = Context.CreateEntityIterator(); EntityIt; ++EntityIt)
		{
			if (!Gathers[EntityIt].IsGathering())
			{
				continue;
			}
			FGatherer Gatherer{ Context.GetEntity(EntityIt), Gathers[EntityIt], Moves[EntityIt], States[EntityIt],
				Transforms[EntityIt].GetTransform().GetLocation(), States[EntityIt].OwnerPlayer, Constants };
			Advance(Step, Gatherer);
		}
	});
}

void UHOLGatherProcessor::RefreshDeposits(UWorld& World)
{
	// A few buildings per player: read again at each step, a new one or a lost one counts at once.
	Deposits.Reset();
	for (TActorIterator<AHOLBuilding> It(&World); It; ++It)
	{
		const FHOLBuildingTypeRow* Type = It->GetType();
		if (!Type || Type->Accepts.IsEmpty())
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
		int32 NumLost = 0;
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
			// Another resource: what it carries is lost, as in AoE (D296).
			NumLost += HOLGather::StartGathering(Gather.Load, Resource);
			Gather.SourceIndex = Order.SourceIndex;
			Gather.Phase = EHOLGatherPhase::ToSource;
			Gather.bWalkIssued = false;
			Gather.Misses = 0;
			Gather.WaitSteps = 0;
			++NumGatherers;
		}
		UE_LOG(LogHOLSimulation, Display, TEXT("Gather order of slot %d: %d units to source %d (%s)%s%s"), Order.PlayerIndex, NumGatherers,
			Order.SourceIndex, GetResourceName(Resource), NumRefused > 0 ? *FString::Printf(TEXT(", %d refused"), NumRefused) : TEXT(""),
			NumLost > 0 ? *FString::Printf(TEXT(", %d carried lost"), NumLost) : TEXT(""));
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

	switch (Gather.Phase)
	{
	case EHOLGatherPhase::ToSource:
	{
		if (!Source || !Source->IsStanding())
		{
			if (Source)
			{
				OnSourceGone(Step, Gatherer, Source->Location);
			}
			else
			{
				Stop(Gatherer);
			}
			return;
		}
		if (HOLGather::IsFull(Gather.Load, Gatherer.Constants.CarryCapacity) && Gather.Load.Resource == Source->Resource)
		{
			WalkToDeposit(Step, Gatherer);
			return;
		}
		if (!Gather.bWalkIssued)
		{
			if (Gather.WaitSteps > 0)
			{
				--Gather.WaitSteps;
				return;
			}
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
		// Short of its spot (taken by another body, or pushed): another spot, then a neighbour.
		Gather.bWalkIssued = false;
		++Counters.MissedSpots;
		if (++Gather.Misses >= MaxSourceMisses)
		{
			TArray<int32> Neighbours;
			FindNeighbours(Step, Source->Resource, Source->Location, Gather.SourceIndex, Neighbours);
			if (Neighbours.Num() > 0)
			{
				Gather.SourceIndex = Neighbours[0];
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
				OnSourceGone(Step, Gatherer, Source->Location);
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
		const FVector GoneLocation = Source->Location;
		const int32 Taken = HOLGather::Gather(Gather.Load, Gatherer.Constants.GatherRates[Source->Resource], Step.StepSeconds,
			Gatherer.Constants.CarryCapacity, Remaining);
		if (Taken > 0)
		{
			// The registry's own count (it leaves the map at 0); Source is changed in place.
			Step.Sources->Take(Gather.SourceIndex, Taken);
		}
		if (HOLGather::IsFull(Gather.Load, Gatherer.Constants.CarryCapacity))
		{
			WalkToDeposit(Step, Gatherer);
		}
		else if (Remaining <= 0)
		{
			OnSourceGone(Step, Gatherer, GoneLocation);
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
				OnSourceGone(Step, Gatherer, Source->Location);
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
		&& !SpotClaims.FindRef(Gather.SourceIndex).ContainsByPredicate([&Gatherer, BodyRadius](const FSpotClaim& Claim)
		{
			return Claim.Entity != Gatherer.Entity && FVector2D::Distance(Claim.Spot, FVector2D(Gatherer.Location)) < 2. * BodyRadius;
		}))
	{
		Gather.Spot = Gatherer.Location;
		SpotClaims.FindOrAdd(Gather.SourceIndex).Add({ Gatherer.Entity, FVector2D(Gatherer.Location) });
		StartGathering(Gatherer);
		return;
	}

	FVector Spot;
	if (FindSourceSpot(Step, Gather.SourceIndex, Gatherer.Location, BodyRadius, Gatherer.Entity, Spot))
	{
		IssueWalk(Step, Gatherer, Spot);
		return;
	}
	// Its source is surrounded: the nearest neighbour with a free spot (the gatherers spread along the wood's edge).
	TArray<int32> Neighbours;
	FindNeighbours(Step, Source.Resource, Source.Location, Gather.SourceIndex, Neighbours);
	for (const int32 Neighbour : Neighbours)
	{
		if (FindSourceSpot(Step, Neighbour, Gatherer.Location, BodyRadius, Gatherer.Entity, Spot))
		{
			Gather.SourceIndex = Neighbour;
			IssueWalk(Step, Gatherer, Spot);
			return;
		}
	}
	Gather.WaitSteps = NoSpotWaitSteps;
	++Counters.NoSpotWaits;
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
	const FVector2D Spot = HOLGather::GetDepositSpot(FVector2D(Gatherer.Location), Deposit.Footprint, Gatherer.Constants.Radius + DepositSpotGap, Along);
	IssueWalk(Step, Gatherer, FVector(Spot, Deposit.GroundZ));
}

void UHOLGatherProcessor::OnSourceGone(const FStepContext& Step, FGatherer& Gatherer, const FVector& GoneLocation)
{
	FHOLGatherFragment& Gather = Gatherer.Gather;
	TArray<int32> Neighbours;
	FindNeighbours(Step, Gather.Load.Resource, GoneLocation, Gather.SourceIndex, Neighbours);
	Gather.bWalkIssued = false;
	Gather.Misses = 0;
	if (Neighbours.Num() > 0)
	{
		Gather.SourceIndex = Neighbours[0];
		Gather.Phase = EHOLGatherPhase::ToSource;
		return;
	}
	Gather.SourceIndex = INDEX_NONE;
	if (Gather.Load.Amount > 0)
	{
		// Nothing left to gather near: it unloads what it carries, then stands (D296).
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
	Gatherer.State.Activity = EHOLUnitActivity::Gathering;
}

void UHOLGatherProcessor::Stop(FGatherer& Gatherer)
{
	if (Gatherer.State.Activity == EHOLUnitActivity::Gathering)
	{
		Gatherer.State.Activity = EHOLUnitActivity::Idle;
	}
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
	Gather.OrderSerialBefore = Gatherer.Move.OrderSerial;
	Gather.bWalkIssued = true;
	const uint32 UnitIds[] = { UnitId };
	Step.Commands->IssueMoveOrder(UnitIds, Spot, INDEX_NONE, /*bFromTask*/ true);
}

bool UHOLGatherProcessor::FindSourceSpot(const FStepContext& Step, const int32 SourceIndex, const FVector& From, const float BodyRadius,
	const FMassEntityHandle Self, FVector& OutSpot)
{
	using namespace UE::HeroesOfLiberty::Gather;
	const FHOLSource& Source = Step.Sources->GetSources()[SourceIndex];
	const FVector2D Centre(Source.Location);
	const double Distance = Source.Radius + BodyRadius + SpotGap;
	const double MinSpacing = 2. * BodyRadius + SpotSpacingGap;

	// The spots still held: their gatherers still walk to them or gather there.
	TArray<FSpotClaim>& Claims = SpotClaims.FindOrAdd(SourceIndex);
	const FMassEntityManager& EntityManager = *Step.EntityManager;
	Claims.RemoveAll([&EntityManager, SourceIndex, Self](const FSpotClaim& Claim)
	{
		if (Claim.Entity == Self || !EntityManager.IsEntityValid(Claim.Entity))
		{
			return true;
		}
		const FHOLGatherFragment* Gather = EntityManager.GetFragmentDataPtr<FHOLGatherFragment>(Claim.Entity);
		return !Gather || Gather->SourceIndex != SourceIndex
			|| (Gather->Phase != EHOLGatherPhase::ToSource && Gather->Phase != EHOLGatherPhase::Gathering)
			|| FVector2D::DistSquared(FVector2D(Gather->Spot), Claim.Spot) > 1.;
	});

	FVector2D Towards = FVector2D(From) - Centre;
	if (!Towards.Normalize())
	{
		Towards = FVector2D(1., 0.);
	}
	const double BaseAngle = FMath::Atan2(Towards.Y, Towards.X);
	const double SpotStep = FMath::Min(MaxSpotStep, 2. * FMath::Asin(FMath::Min(1., MinSpacing / (2. * Distance))));
	const int32 SpotTries = FMath::FloorToInt32(2. * UE_DOUBLE_PI / SpotStep + 1e-6);
	const FVector Extent(25., 25., 300.);
	for (int32 Try = 0; Try < SpotTries; ++Try)
	{
		// 0, +1, -1, +2, -2... steps from the side facing it.
		const int32 Turn = (Try + 1) / 2;
		const double Angle = BaseAngle + (Try % 2 == 1 ? 1. : -1.) * Turn * SpotStep;
		const FVector2D Spot = Centre + Distance * FVector2D(FMath::Cos(Angle), FMath::Sin(Angle));
		if (Claims.ContainsByPredicate([&Spot, MinSpacing](const FSpotClaim& Claim) { return FVector2D::DistSquared(Claim.Spot, Spot) < MinSpacing * MinSpacing; }))
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
		}
		Claims.Add({ Self, Spot });
		OutSpot = Found;
		return true;
	}
	return false;
}

void UHOLGatherProcessor::FindNeighbours(const FStepContext& Step, const EHOLResource Resource, const FVector& Location, const int32 Exclude,
	TArray<int32>& OutSources) const
{
	OutSources.Reset();
	const TArray<FHOLSource>& Sources = Step.Sources->GetSources();
	const double MaxDistanceSquared = FMath::Square(HOLGather::NeighbourSourceDistance);
	TArray<TPair<double, int32>> Found;
	for (int32 Index = 0; Index < Sources.Num(); ++Index)
	{
		const FHOLSource& Source = Sources[Index];
		if (Index == Exclude || !Source.IsStanding() || Source.Resource != Resource)
		{
			continue;
		}
		const double DistanceSquared = FVector::DistSquared2D(Location, Source.Location);
		if (DistanceSquared <= MaxDistanceSquared)
		{
			Found.Emplace(DistanceSquared, Index);
		}
	}
	Found.Sort([](const TPair<double, int32>& A, const TPair<double, int32>& B) { return A.Key < B.Key || (A.Key == B.Key && A.Value < B.Value); });
	for (const TPair<double, int32>& Pair : Found)
	{
		OutSources.Add(Pair.Value);
	}
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
			}
		}));
}
