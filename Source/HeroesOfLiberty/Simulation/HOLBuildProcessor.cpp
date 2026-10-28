// Copyright Epic Games, Inc. All Rights Reserved.

#include "Simulation/HOLBuildProcessor.h"

#include "Buildings/HOLBuilding.h"
#include "Buildings/HOLBuildingTypes.h"
#include "Buildings/HOLConstructionRules.h"
#include "Commands/HOLCommandSubsystem.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "HAL/IConsoleManager.h"
#include "HeroesOfLibertyLog.h"
#include "MassCommonFragments.h"
#include "MassCommonTypes.h"
#include "MassExecutionContext.h"
#include "Match/HOLGameState.h"
#include "NavigationSystem.h"
#include "Resources/HOLSourceSubsystem.h"
#include "Resources/HOLSourceTypes.h"
#include "Simulation/HOLBuildFragments.h"
#include "Simulation/HOLSimulationFragments.h"
#include "Simulation/HOLSimulationSubsystem.h"
#include "Simulation/HOLUnitSubsystem.h"
#include "Simulation/HOLUnitTypes.h"

namespace UE::HeroesOfLiberty::Build
{
	int32 Speed = 1;
	FAutoConsoleVariableRef CVarSpeed(TEXT("hol.Build.Speed"), Speed,
		TEXT("Construction this many times as fast, for every player (development and scripted checks, J6a.4): ")
		TEXT("each step's work added this many times; 1: the table's times"));

#if !UE_BUILD_SHIPPING
	int32 Instant = 0;
	FAutoConsoleVariableRef CVarInstant(TEXT("hol.Build.Instant"), Instant,
		TEXT("1: every site is finished at its first blow, a builder still has to reach it (development only; hol.Cheat.FastBuild sets it, ")
		TEXT("the user's choice at the trial of J6a.4, as the aegis of AoE II)"));
#endif

	float NextSiteRange = 2000.f;
	FAutoConsoleVariableRef CVarNextSiteRange(TEXT("hol.Build.NextSiteRange"), NextSiteRange,
		TEXT("Distance (cm, footprint to footprint) at most from a site just finished to its player's nearest site not finished, bare foundations too, ")
		TEXT("where its builders with nothing left in their queue go on (the user's trial of J6a.4: ~20 m); 0: they stand idle"));

	float CampGatherRange = 2500.f;
	FAutoConsoleVariableRef CVarCampGatherRange(TEXT("hol.Build.CampGatherRange"), CampGatherRange,
		TEXT("Distance (cm, footprint to the source's edge) at most from a camp just finished to the nearest standing source it accepts, which its ")
		TEXT("builders with nothing left in their queue go on to gather (D312, J6b.1: 25 m); none that near: they go on as after a house ")
		TEXT("(hol.Build.NextSiteRange, else idle); a negative value: never"));

	/**
	 * Gap (cm) between a builder's body at its spot and the footprint, beyond the clearance every body keeps from a footprint
	 * (D310): the spot is on the navmesh once the site blocks it (it ends 60 + 25 cm from the footprint, ~10 cm more on the
	 * cells), as the carriers' unloading spots (D296).
	 */
	constexpr double SpotGap = 20.;
	/** Gap (cm) between two builders' bodies at their spots: they never overlap (the separation leaves them be). */
	constexpr double SpotSpacingGap = 10.;
	/** A walk to the site ends at contact within this gap (cm, body to footprint): one stopped short by a neighbour still works. */
	constexpr double ArrivalGap = 100.;
	/** A builder pushed farther than this (cm, body to footprint) walks back to its spot. */
	constexpr double KeepBuildingGap = 150.;
	/** Arrivals that missed the site in a row before a builder gives up and stands idle. */
	constexpr int32 MaxMisses = 6;
	/** A spot is on the navmesh when the navmesh is this near it (cm). */
	constexpr double NavTolerance = 20.;
	/** The builders' queues of units that no longer build are swept every so many steps. */
	constexpr int32 SweepSteps = 200;
	/** hol.Build.Speed at most. */
	constexpr int32 MaxSpeed = 100;
}

double UHOLBuildProcessor::GetCampGatherRange()
{
	return static_cast<double>(UE::HeroesOfLiberty::Build::CampGatherRange);
}

int32 UHOLBuildProcessor::FindCampSource(const UHOLSourceSubsystem& Sources, const AHOLBuilding& Camp, double* OutGap)
{
	const double Range = GetCampGatherRange();
	if (Range < 0. || !Camp.IsCamp())
	{
		return INDEX_NONE;
	}
	// Every source by its index (a camp finished now and then: a copy of a few hundred is nothing).
	// A farm field (a linked source) is never one (IA1.2-Q6: a mill takes the berries, not a farmer's place); a free field
	// of their own is its builders' second choice (GatherForCamp).
	const TArray<FHOLSource>& All = Sources.GetSources();
	TArray<FHOLCampSource> Candidates;
	Candidates.Reserve(All.Num());
	for (int32 Index = 0; Index < All.Num(); ++Index)
	{
		const FHOLSource& Source = All[Index];
		Candidates.Add({ FVector2D(Source.Location), static_cast<double>(Source.Radius), Source.Resource, Source.IsStanding() && !Sources.IsLinked(Index) });
	}
	return HOLConstruction::FindCampSource(Camp.GetBodyBounds(), Camp.GetType()->Accepts, Candidates, Range, OutGap);
}

UHOLBuildProcessor::UHOLBuildProcessor()
	: EntityQuery(*this)
{
	bAutoRegisterWithProcessingPhases = false; // fixed step only (D219)
	ExecutionFlags = static_cast<int32>(EProcessorExecutionFlags::Server | EProcessorExecutionFlags::Standalone);
	ExecutionOrder.ExecuteInGroup = UE::Mass::ProcessorGroupNames::Tasks;
	// After the orders of the step (it sees the arrivals, a player's order already ended a building) and the gathering.
	ExecutionOrder.ExecuteAfter.Add(TEXT("HOLUnitOrderProcessor"));
	ExecutionOrder.ExecuteAfter.Add(TEXT("HOLGatherProcessor"));
	ExecutionOrder.ExecuteBefore.Add(TEXT("MassNavMeshPathFollowProcessor"));
	bRequiresGameThreadExecution = true; // the order queue, the buildings, the navigation
}

void UHOLBuildProcessor::ConfigureQueries(const TSharedRef<FMassEntityManager>& EntityManager)
{
	EntityQuery.AddRequirement<FHOLBuildFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FHOLMoveOrderFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddRequirement<FHOLUnitStateFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FHOLGatherFragment>(EMassFragmentAccess::ReadWrite);
	EntityQuery.AddRequirement<FTransformFragment>(EMassFragmentAccess::ReadOnly);
	EntityQuery.AddConstSharedRequirement<FHOLUnitTypeConstants>(EMassFragmentPresence::All);
	EntityQuery.AddTagRequirement<FHOLLightUnitTag>(EMassFragmentPresence::All);
}

void UHOLBuildProcessor::GetChains(TArray<FChainOfBuilder>& OutChains) const
{
	OutChains.Reset();
	for (const TPair<FMassEntityHandle, FBuilderQueue>& Pair : Queues)
	{
		const bool bRallyHead = Pair.Value.IsRallyHead();
		if ((!Pair.Value.bChain && !bRallyHead) || Pair.Value.Items.IsEmpty())
		{
			continue;
		}
		FChainOfBuilder& Chain = OutChains.AddDefaulted_GetRef();
		Chain.Entity = Pair.Key;
		for (const FWorkItem& Item : Pair.Value.Items)
		{
			if (Item.IsGather())
			{
				Chain.Steps.Add(HOLOrderChain::FStep::MakeGather(Item.SourceIndex));
			}
			else if (Item.IsDeposit())
			{
				// D485: drawn as a gathering (blue), to the deposit.
				if (const AHOLBuilding* Deposit = Item.Site.Get())
				{
					Chain.Steps.Add(HOLOrderChain::FStep::MakeDeposit(Deposit->GetUniqueID()));
				}
			}
			else if (const AHOLBuilding* Site = Item.Site.Get())
			{
				// Born for a rally point on this site (D328): a white step to it until the unit starts building.
				Chain.Steps.Add(bRallyHead && Chain.Steps.IsEmpty() ? HOLOrderChain::FStep::MakeRally(Site->GetActorLocation())
					: HOLOrderChain::FStep::MakeBuild(Site->GetUniqueID()));
			}
		}
	}
}

bool UHOLBuildProcessor::HasChains() const
{
	for (const TPair<FMassEntityHandle, FBuilderQueue>& Pair : Queues)
	{
		if ((Pair.Value.bChain || Pair.Value.IsRallyHead()) && !Pair.Value.Items.IsEmpty())
		{
			return true;
		}
	}
	return false;
}

void UHOLBuildProcessor::Execute(FMassEntityManager& EntityManager, FMassExecutionContext& Context)
{
	UWorld* World = EntityManager.GetWorld();
	LastWorld = World;
	FStepContext Step;
	Step.EntityManager = &EntityManager;
	Step.Commands = World ? World->GetSubsystem<UHOLCommandSubsystem>() : nullptr;
	Step.Sources = World ? World->GetSubsystem<UHOLSourceSubsystem>() : nullptr;
	Step.Units = World ? World->GetSubsystem<UHOLUnitSubsystem>() : nullptr;
	if (!Step.Commands || !Step.Sources || !Step.Units)
	{
		return;
	}
	const UHOLSimulationSubsystem* Simulation = World->GetSubsystem<UHOLSimulationSubsystem>();
	Step.StepIndex = Simulation ? Simulation->GetStepIndex() : 0;
	ApplyBuildOrders(Step);

	// The queues of the units a player sent elsewhere (or gone), from time to time.
	if (++StepsSinceSweep >= UE::HeroesOfLiberty::Build::SweepSteps)
	{
		StepsSinceSweep = 0;
		for (auto It = Queues.CreateIterator(); It; ++It)
		{
			const FHOLBuildFragment* Build = EntityManager.IsEntityValid(It.Key()) ? EntityManager.GetFragmentDataPtr<FHOLBuildFragment>(It.Key()) : nullptr;
			if (!Build || !Build->IsBuilding())
			{
				It.RemoveCurrent();
			}
		}
	}
	// No builder anywhere (S7, most of a match): nothing more to do.
	if (Queues.IsEmpty())
	{
		// The last builders gone: their sites show none (J6a.5); nothing to do once it is written.
		for (const TPair<TWeakObjectPtr<AHOLBuilding>, FSiteStats>& Pair : SiteStats)
		{
			if (AHOLBuilding* Site = Pair.Key.Get())
			{
				Site->SetBuildersAtWork(0);
			}
		}
		// Nobody at a forum down any more (V2.2).
		if (ForumBuilderSlots != 0)
		{
			PublishForumBuilders(World, TMap<int32, int32>());
		}
		return;
	}
	Step.NavSystem = FNavigationSystem::GetCurrent<UNavigationSystemV1>(World);
	Step.NavData = Step.NavSystem ? Step.NavSystem->GetDefaultNavDataInstance() : nullptr;

	// Every building and foundation: no spot on one (a house touching the site), whoever it belongs to; their bodies (D324:
	// no spot in the way between two neighbours either, too narrow for a builder and a passer-by).
	Footprints.Reset();
	FootprintBuildings.Reset();
	for (TActorIterator<AHOLBuilding> It(World); It; ++It)
	{
		if (It->GetType())
		{
			Footprints.Add(It->GetBodyBounds());
			FootprintBuildings.Add(*It);
		}
	}

	// The spots held: the builders walking to theirs or working there.
	SpotClaims.Reset();
	EntityQuery.ForEachEntityChunk(Context, [this](FMassExecutionContext& Context)
	{
		const TConstArrayView<FHOLBuildFragment> Builds = Context.GetFragmentView<FHOLBuildFragment>();
		for (FMassExecutionContext::FEntityIterator EntityIt = Context.CreateEntityIterator(); EntityIt; ++EntityIt)
		{
			const FHOLBuildFragment& Build = Builds[EntityIt];
			if (Build.IsBuilding() && (Build.bWalkIssued || Build.Phase == EHOLBuildPhase::Building))
			{
				SpotClaims.Add({ Context.GetEntity(EntityIt), FVector2D(Build.Spot) });
			}
		}
	});

	WorkersBySite.Reset();
	EntityQuery.ForEachEntityChunk(Context, [this, &Step](FMassExecutionContext& Context)
	{
		const TArrayView<FHOLBuildFragment> Builds = Context.GetMutableFragmentView<FHOLBuildFragment>();
		const TConstArrayView<FHOLMoveOrderFragment> Moves = Context.GetFragmentView<FHOLMoveOrderFragment>();
		const TArrayView<FHOLUnitStateFragment> States = Context.GetMutableFragmentView<FHOLUnitStateFragment>();
		const TArrayView<FHOLGatherFragment> Gathers = Context.GetMutableFragmentView<FHOLGatherFragment>();
		const TConstArrayView<FTransformFragment> Transforms = Context.GetFragmentView<FTransformFragment>();
		const FHOLUnitTypeConstants& Constants = Context.GetConstSharedFragment<FHOLUnitTypeConstants>();
		for (FMassExecutionContext::FEntityIterator EntityIt = Context.CreateEntityIterator(); EntityIt; ++EntityIt)
		{
			if (!Builds[EntityIt].IsBuilding() || States[EntityIt].CurrentHealth <= 0)
			{
				// Dying (J7b.4): its task is over.
				continue;
			}
			FBuilder Builder{ Context.GetEntity(EntityIt), Builds[EntityIt], Moves[EntityIt], States[EntityIt], Gathers[EntityIt],
				Transforms[EntityIt].GetTransform().GetLocation(), Constants };
			Advance(Step, Builder);
			// Gathering at the head of its queue: the gathering shows its activity (D307).
			if (Builder.Build.IsBuilding() && Builder.Build.Phase != EHOLBuildPhase::Gathering)
			{
				SyncActivity(Builder);
			}
		}
	});
	LogCampSends(Step);
	ApplyWork(Step);
}

void UHOLBuildProcessor::ApplyBuildOrders(const FStepContext& Step)
{
	TArray<FHOLBuildOrder> Orders;
	Step.Commands->ConsumeBuildOrders(Orders);
	FMassEntityManager& EntityManager = *Step.EntityManager;
	const TArray<FHOLSource>& Sources = Step.Sources->GetSources();
	for (const FHOLBuildOrder& Order : Orders)
	{
		// A source to gather in its turn (Shift + right click on a resource, the user's trial of J6a.4), as a gather order:
		// a source still standing, only the player's units that gather its resource (D288).
		const bool bGather = Order.SourceIndex != INDEX_NONE;
		// Or a deposit to unload at in its turn (Shift + right click on it, D485, EtapeDefini L.3).
		const bool bDeposit = !bGather && Order.bDeposit;
		AHOLBuilding* Site = Order.Building.Get();
		if (bGather && (!Sources.IsValidIndex(Order.SourceIndex) || !Sources[Order.SourceIndex].IsStanding()))
		{
			UE_LOG(LogHOLCommands, Display, TEXT("Build order of slot %d ignored: source %d is not standing"), Order.PlayerIndex, Order.SourceIndex);
			continue;
		}
		// A camp finished meanwhile (a unit born with its rally point on it a step too late, a right click on a site finishing
		// at once): the nearest source it accepts, as its builders (D312, J6b.1), by a gather order of its owner's.
		if (!bGather && !bDeposit && Site && !Site->NeedsBuilders() && !Order.bQueue && (Order.PlayerIndex == INDEX_NONE || Site->GetOwnerPlayer() == Order.PlayerIndex))
		{
			double Gap = 0.;
			const int32 CampSource = FindCampSource(*Step.Sources, *Site, &Gap);
			if (CampSource != INDEX_NONE)
			{
				Step.Commands->IssueGatherOrder(Order.UnitIds, CampSource, Site->GetOwnerPlayer());
				const FVector Location = Site->GetActorLocation();
				UE_LOG(LogHOLCommands, Display, TEXT("Build order of slot %d on the finished camp %s at (%.0f, %.0f): %d units sent to gather %s (source %d, %.1f m)"),
					Order.PlayerIndex, *Site->GetTypeName().ToString(), Location.X, Location.Y, Order.UnitIds.Num(),
					*StaticEnum<EHOLResource>()->GetNameStringByValue(static_cast<int64>(Sources[CampSource].Resource)), CampSource, Gap / 100.);
				continue;
			}
		}
		// A deposit: only one of the player's finished buildings taking deliveries (D288, D485).
		const FHOLBuildingTypeRow* DepositType = bDeposit && Site ? Site->GetType() : nullptr;
		if (bDeposit && (!DepositType || DepositType->Accepts.IsEmpty() || Site->NeedsBuilders() || !Site->TakesDeliveries()
			|| (Order.PlayerIndex != INDEX_NONE && Site->GetOwnerPlayer() != Order.PlayerIndex)))
		{
			UE_LOG(LogHOLCommands, Display, TEXT("Build order of slot %d ignored: not one of its deposits"), Order.PlayerIndex);
			continue;
		}
		// Only the player's own site, not finished yet, or its main forum down (D288, V2.2): a client could name any building.
		if (!bGather && !bDeposit && (!Site || !Site->NeedsBuilders() || (Order.PlayerIndex != INDEX_NONE && Site->GetOwnerPlayer() != Order.PlayerIndex)))
		{
			UE_LOG(LogHOLCommands, Display, TEXT("Build order of slot %d ignored: not its site under construction"), Order.PlayerIndex);
			continue;
		}
		const EHOLResource Resource = bGather ? Sources[Order.SourceIndex].Resource : EHOLResource::Food;
		const FWorkItem Item = bGather ? FWorkItem::MakeGather(Order.SourceIndex)
			: bDeposit ? FWorkItem::MakeDeposit(TWeakObjectPtr<AHOLBuilding>(Site)) : FWorkItem::MakeBuild(TWeakObjectPtr<AHOLBuilding>(Site));
		// Whose units obey: the site's (or the deposit's) owner's; a source's, the player's who gave it.
		const int32 Owner = bGather ? Order.PlayerIndex : static_cast<int32>(Site->GetOwnerPlayer());
		int32 NumBuilders = 0;
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
			FHOLBuildFragment* Build = EntityManager.GetFragmentDataPtr<FHOLBuildFragment>(Entity);
			FHOLGatherFragment* Gather = EntityManager.GetFragmentDataPtr<FHOLGatherFragment>(Entity);
			// Only the owner's units, and only the workers (those that gather: the Peasant, D313); a source: those that
			// gather its resource.
			if (!Build || !Gather || (Owner != INDEX_NONE && State.OwnerPlayer != Owner) || Constants.CarryCapacity <= 0
				|| (bGather && Constants.GatherRates[Resource] <= 0.f))
			{
				++NumRefused;
				continue;
			}
			FBuilderQueue& Queue = Queues.FindOrAdd(Entity);
			const bool bHadQueue = Build->IsBuilding();
			if (!bHadQueue)
			{
				Queue.Items.Reset();
				Queue.Current = FWorkItem();
			}
			HOLConstruction::AddToBuildQueue(Queue.Items, Item, Order.bQueue);
			// D328 (the user's trial of J7b.5b): a player's order makes a chain, with Shift or not; the server's own (a unit born
			// with its rally point on a site) only once queued after another.
			// A unit born with its rally point on a site (D328): a Rally step until it starts building, no chain after.
			Queue.bChain = (Order.bChain && !Order.bRally) || (Order.bQueue && (Queue.bChain || Queue.Items.Num() > 1));
			if (!bHadQueue || !Order.bQueue)
			{
				Queue.RallyItem.Reset();
			}
			if (Order.bRally)
			{
				Queue.RallyItem = Item;
			}
			// A new head (no queue before, or no Shift): it stops gathering, its load kept (as a player's move order, D296),
			// and its head is looked at at once; with Shift, its head so far goes on first.
			if (!bHadQueue || !Order.bQueue)
			{
				if (Gather->IsGathering())
				{
					Gather->Phase = EHOLGatherPhase::None;
					Gather->SourceIndex = INDEX_NONE;
					Gather->bWalkIssued = false;
				}
				Build->Phase = EHOLBuildPhase::ToSite;
				Build->bWalkIssued = false;
				Build->Misses = 0;
				Queue.Current = FWorkItem();
			}
			// A new head without Shift: the moves its player queued after its work are forgotten with it (EtapeDefini M.1).
			if (!Order.bQueue)
			{
				Build->bMovesAfter = false;
			}
			Build->bQueueNext = Queue.Items.Num() > 1;
			++NumBuilders;
		}
		const TCHAR* Queued = Order.bQueue ? TEXT(", queued") : TEXT("");
		const FString Refused = NumRefused > 0 ? FString::Printf(TEXT(", %d refused"), NumRefused) : FString();
		if (bGather)
		{
			const FVector Location = Sources[Order.SourceIndex].Location;
			UE_LOG(LogHOLCommands, Display, TEXT("Build order of slot %d: %d units to gather source %d (%s) at (%.0f, %.0f)%s%s"), Order.PlayerIndex,
				NumBuilders, Order.SourceIndex, *StaticEnum<EHOLResource>()->GetNameStringByValue(static_cast<int64>(Resource)), Location.X, Location.Y,
				Queued, *Refused);
			continue;
		}
		const FVector Location = Site->GetActorLocation();
		UE_LOG(LogHOLCommands, Display, TEXT("Build order of slot %d: %d units to %s%s at (%.0f, %.0f)%s%s"), Order.PlayerIndex, NumBuilders,
			bDeposit ? TEXT("unload at ") : TEXT(""), *Site->GetTypeName().ToString(), Location.X, Location.Y, Queued, *Refused);
	}
}

void UHOLBuildProcessor::Advance(const FStepContext& Step, FBuilder& Builder)
{
	using namespace UE::HeroesOfLiberty::Build;
	FHOLBuildFragment& Build = Builder.Build;
	FBuilderQueue* Queue = Queues.Find(Builder.Entity);
	if (!Queue)
	{
		Stop(Builder, TEXT("no site left"));
		return;
	}
	// A source at the head of its queue: the gathering carries it out until its massif is used up (D309), then the next item;
	// a deposit (D485): until it has unloaded.
	const bool bGatherHead = Build.Phase == EHOLBuildPhase::Gathering && !Queue->Items.IsEmpty() && Queue->Items[0] == Queue->Current;
	if (bGatherHead && Builder.Gather.IsGathering())
	{
		return;
	}
	// Its site finished or cancelled, its gathering over: the next item of its queue (D312), else, a camp finished, the
	// nearest source it accepts (J6b.1), else its player's nearest site near the one it finished (the user's trial of
	// J6a.4), else idle.
	const FWorkItem Before = Queue->Current;
	if (!HOLConstruction::AdvanceWorkQueue<TWeakObjectPtr<AHOLBuilding>>(Queue->Items, bGatherHead,
		[](const TWeakObjectPtr<AHOLBuilding>& Site) { return Site.IsValid() && Site->NeedsBuilders(); }))
	{
		// A main forum rebuilt (V2.2) ends as a site finished. A deposit unloaded at is no site finished (D485): idle after it,
		// neither its camp's source nor a nearby site.
		// Moves its player queued after its work (Shift, EtapeDefini M.1): idle, so that they start (UHOLUnitOrderProcessor).
		if (Build.bMovesAfter)
		{
			Stop(Builder, TEXT("its queue done, moves queued after it"));
			return;
		}
		AHOLBuilding* Finished = Before.IsBuild() ? Before.Site.Get() : nullptr;
		const bool bFinished = Finished && !Finished->NeedsBuilders();
		if (bFinished && (GatherForFarm(Step, Builder, *Finished) || GatherForCamp(Step, Builder, *Finished)))
		{
			return;
		}
		AHOLBuilding* Next = bFinished ? FindNextSite(Builder, *Finished) : nullptr;
		if (!Next)
		{
			Stop(Builder, bGatherHead ? TEXT("its queue done") : TEXT("no site left"));
			return;
		}
		const FVector Location = Next->GetActorLocation();
		UE_LOG(LogHOLSimulation, Display, TEXT("Build: a builder of slot %d goes on to the nearby site %s at (%.0f, %.0f), %.0f cm from the one finished"),
			Builder.State.OwnerPlayer, *Next->GetTypeName().ToString(), Location.X, Location.Y,
			HOLConstruction::GetGap(Finished->GetBodyBounds(), Next->GetBodyBounds()));
		Queue->Items.Add(FWorkItem::MakeBuild(Next));
		// A site taken on its own, no player's order (D328): no chain.
		Queue->bChain = false;
	}
	Build.bQueueNext = Queue->Items.Num() > 1;
	if (Queue->Current != Queue->Items[0])
	{
		Queue->Current = Queue->Items[0];
		if (Queue->Current.IsGather())
		{
			StartGathering(Step, Builder, Queue->Current.SourceIndex);
			return;
		}
		if (Queue->Current.IsDeposit())
		{
			StartUnloading(Builder, Queue->Current);
			return;
		}
		Build.Phase = EHOLBuildPhase::ToSite;
		Build.bWalkIssued = false;
		Build.Misses = 0;
	}
	AHOLBuilding* Site = Queue->Items[0].Site.Get();
	if (!Site || Build.Phase == EHOLBuildPhase::Gathering || Queue->Items[0].IsDeposit())
	{
		return;
	}
	// The builders stand at the wall of its body (D324), not at the edge of its tiles.
	const FBox2D Footprint = Site->GetBodyBounds();
	const FVector2D Location(Builder.Location);
	// Its walk is over: the order it gave itself was applied (a new serial), then ended (no phase).
	const bool bArrived = Build.bWalkIssued && Builder.Move.OrderSerial != Build.OrderSerialBefore && Builder.Move.Phase == EHOLMovePhase::None;
	if (Build.Phase == EHOLBuildPhase::ToSite)
	{
		if (!Build.bWalkIssued)
		{
			WalkToSite(Step, Builder, *Site, Footprint);
			return;
		}
		if (!bArrived)
		{
			return;
		}
		if (HOLConstruction::IsAtSite(Footprint, Location, Builder.Constants.Radius, ArrivalGap))
		{
			StartBuilding(Builder, Footprint);
			return;
		}
		// Short of the site (pushed away, its spot taken by a body): another spot.
		Build.bWalkIssued = false;
		if (++Build.Misses >= MaxMisses)
		{
			Stop(Builder, TEXT("its site out of reach"));
		}
		return;
	}
	if (!HOLConstruction::IsAtSite(Footprint, Location, Builder.Constants.Radius, KeepBuildingGap))
	{
		// Pushed away by a passer-by (D292): back to its spot.
		Build.Phase = EHOLBuildPhase::ToSite;
		Build.bWalkIssued = false;
		return;
	}
	++WorkersBySite.FindOrAdd(Site);
}

void UHOLBuildProcessor::WalkToSite(const FStepContext& Step, FBuilder& Builder, const AHOLBuilding& Site, const FBox2D& Footprint)
{
	using namespace UE::HeroesOfLiberty::Build;
	const float Radius = Builder.Constants.Radius;
	const double Spacing = 2. * Radius + SpotSpacingGap;
	// Already at contact on a place of its own (sent to the site beside it): it works at once.
	if (HOLConstruction::IsAtSite(Footprint, FVector2D(Builder.Location), Radius, ArrivalGap)
		&& HOLConstruction::GetDistance(Footprint, FVector2D(Builder.Location)) >= Radius
		&& !IsSpotTaken(FVector2D(Builder.Location), Spacing, Builder.Entity))
	{
		Builder.Build.Spot = Builder.Location;
		SpotClaims.Add({ Builder.Entity, FVector2D(Builder.Location) });
		StartBuilding(Builder, Footprint);
		return;
	}
	FVector Spot;
	if (FindSpot(Step, Site, Footprint, Builder.Location, Radius, Builder.Entity, Spot))
	{
		IssueWalk(Step, Builder, Spot);
		return;
	}
	Stop(Builder, TEXT("no spot around its site"));
}

bool UHOLBuildProcessor::FindSpot(const FStepContext& Step, const AHOLBuilding& Site, const FBox2D& Footprint, const FVector& From, const float BodyRadius,
	const FMassEntityHandle Self, FVector& OutSpot)
{
	using namespace UE::HeroesOfLiberty::Build;
	const double Clearance = UHOLBuildingSettings::GetBodyClearance();
	const double Offset = BodyRadius + Clearance + SpotGap;
	const double Spacing = 2. * BodyRadius + SpotSpacingGap;
	TArray<FVector2D> Spots;
	HOLConstruction::GetBuildSpots(Footprint, Offset, Spacing, FVector2D(From), Spots);
	const FVector Extent(25., 25., 300.);
	const double GroundZ = Site.GetActorLocation().Z;
	// Free first; every spot taken (more builders than spots), the nearest one on the navmesh: they work shoulder to shoulder.
	for (int32 Pass = 0; Pass < 2; ++Pass)
	{
		for (const FVector2D& Spot : Spots)
		{
			if (Pass == 0 && IsSpotTaken(Spot, Spacing, Self))
			{
				continue;
			}
			// Not on another building nor against it (a house touching the site: no spot on its side).
			bool bOnBuilding = false;
			for (int32 Index = 0; Index < Footprints.Num() && !bOnBuilding; ++Index)
			{
				bOnBuilding = FootprintBuildings[Index].Get() != &Site && HOLConstruction::GetDistance(Footprints[Index], Spot) < BodyRadius + Clearance;
			}
			if (bOnBuilding)
			{
				continue;
			}
			FVector Found(Spot, GroundZ);
			if (Step.NavSystem && Step.NavData)
			{
				FNavLocation OnNav;
				if (!Step.NavSystem->ProjectPointToNavigation(Found, OnNav, Extent, Step.NavData) || FVector::Dist2D(OnNav.Location, Found) > NavTolerance)
				{
					continue;
				}
				Found = OnNav.Location;
			}
			SpotClaims.Add({ Self, FVector2D(Found) });
			OutSpot = Found;
			return true;
		}
	}
	return false;
}

bool UHOLBuildProcessor::IsSpotTaken(const FVector2D& Spot, const double MinSpacing, const FMassEntityHandle Self) const
{
	const double MinSquared = FMath::Square(MinSpacing - 1.);
	return SpotClaims.ContainsByPredicate([&Spot, MinSquared, Self](const FSpotClaim& Claim)
	{
		return Claim.Owner != Self && FVector2D::DistSquared(Claim.Spot, Spot) < MinSquared;
	});
}

void UHOLBuildProcessor::StartBuilding(FBuilder& Builder, const FBox2D& Footprint)
{
	Builder.Build.Phase = EHOLBuildPhase::Building;
	Builder.Build.SiteFootprint = Footprint;
	Builder.Build.Misses = 0;
	Builder.Build.bWalkIssued = false;
	Builder.State.Activity = EHOLUnitActivity::Building;
	// At its site (D328): the line of the rally point it was born for is over.
	if (FBuilderQueue* Queue = Queues.Find(Builder.Entity))
	{
		Queue->RallyItem.Reset();
	}
}

void UHOLBuildProcessor::StartGathering(const FStepContext& Step, FBuilder& Builder, const int32 SourceIndex)
{
	FHOLBuildFragment& Build = Builder.Build;
	Build.Phase = EHOLBuildPhase::Gathering;
	Build.bWalkIssued = false;
	Build.Misses = 0;
	const TArray<FHOLSource>& Sources = Step.Sources->GetSources();
	if (!Sources.IsValidIndex(SourceIndex))
	{
		// Not a source (never met: checked with the order): over at the next step, the next item.
		return;
	}
	// As a gather order (UHOLGatherProcessor::ApplyGatherOrders), from the gathering's next step: a source used up meanwhile
	// sends it to the nearest one of its massif with a free spot, or of a nearby grove (D309).
	const FHOLSource& Source = Sources[SourceIndex];
	Builder.Gather.StartTask(SourceIndex, Source.Resource);
	Builder.State.Activity = EHOLUnitActivity::GatherTrip;
	UE_LOG(LogHOLSimulation, Display, TEXT("Build: a builder of slot %d goes on to gather source %d (%s) at (%.0f, %.0f)%s"), Builder.State.OwnerPlayer,
		SourceIndex, *StaticEnum<EHOLResource>()->GetNameStringByValue(static_cast<int64>(Source.Resource)), Source.Location.X, Source.Location.Y,
		Build.bQueueNext ? TEXT(", more queued") : TEXT(""));
}

void UHOLBuildProcessor::StartUnloading(FBuilder& Builder, const FWorkItem& Item)
{
	FHOLBuildFragment& Build = Builder.Build;
	Build.Phase = EHOLBuildPhase::Gathering;
	Build.bWalkIssued = false;
	Build.Misses = 0;
	FHOLGatherFragment& Gather = Builder.Gather;
	const AHOLBuilding* Deposit = Item.Site.Get();
	const FHOLBuildingTypeRow* Type = Deposit ? Deposit->GetType() : nullptr;
	// Still one of its player's deposits (finished, taking deliveries), and it carries a resource it takes (D485: its load
	// judged now, at its turn).
	const bool bDeposit = Type && !Type->Accepts.IsEmpty() && !Deposit->NeedsBuilders() && Deposit->TakesDeliveries()
		&& Deposit->GetOwnerPlayer() == Builder.State.OwnerPlayer;
	if (!bDeposit || Gather.Load.Amount <= 0 || !Type->Accepts.Contains(Gather.Load.Resource))
	{
		// Nothing to unload there: over at the next step, the next item (idle if none).
		UE_LOG(LogHOLSimulation, Display, TEXT("Build: a builder of slot %d carries nothing %s takes, it skips the unload%s"), Builder.State.OwnerPlayer,
			Deposit ? *Deposit->GetTypeName().ToString() : TEXT("(gone)"), Build.bQueueNext ? TEXT(", more queued") : TEXT(""));
		return;
	}
	// As a drop-off order (UHOLGatherProcessor::ApplyDropOffOrder), from the gathering's next step: unloaded, it stops.
	Gather.StartDropOff(Item.Site);
	Builder.State.Activity = EHOLUnitActivity::GatherTrip;
	const FVector Location = Deposit->GetActorLocation();
	UE_LOG(LogHOLSimulation, Display, TEXT("Build: a builder of slot %d goes on to unload %d %s at %s (%.0f, %.0f)%s"), Builder.State.OwnerPlayer,
		static_cast<int32>(Gather.Load.Amount), *StaticEnum<EHOLResource>()->GetNameStringByValue(static_cast<int64>(Gather.Load.Resource)),
		*Deposit->GetTypeName().ToString(), Location.X, Location.Y, Build.bQueueNext ? TEXT(", more queued") : TEXT(""));
}

AHOLBuilding* UHOLBuildProcessor::FindNextSite(const FBuilder& Builder, const AHOLBuilding& Finished) const
{
	using namespace UE::HeroesOfLiberty::Build;
	TArray<FBox2D, TInlineAllocator<16>> Boxes;
	TArray<AHOLBuilding*, TInlineAllocator<16>> Sites;
	for (int32 Index = 0; Index < FootprintBuildings.Num(); ++Index)
	{
		AHOLBuilding* Site = FootprintBuildings[Index].Get();
		if (Site && Site != &Finished && Site->NeedsBuilders() && Site->GetOwnerPlayer() == Builder.State.OwnerPlayer)
		{
			Boxes.Add(Footprints[Index]);
			Sites.Add(Site);
		}
	}
	const int32 Found = NextSiteRange > 0.f ? HOLConstruction::FindNextSite(Finished.GetBodyBounds(), Boxes, NextSiteRange) : INDEX_NONE;
	return Found != INDEX_NONE ? Sites[Found] : nullptr;
}

bool UHOLBuildProcessor::GatherForCamp(const FStepContext& Step, FBuilder& Builder, AHOLBuilding& Camp)
{
	if (!Camp.IsCamp())
	{
		return false;
	}
	// Its source chosen once for all its builders (they all go on at the same step, the one after its end).
	const TWeakObjectPtr<AHOLBuilding> Key(&Camp);
	FCampSend* Send = CampSends.Find(Key);
	if (!Send)
	{
		Send = &CampSends.Add(Key);
		Send->SourceIndex = FindCampSource(*Step.Sources, Camp, &Send->Gap);
	}
	const TArray<FHOLSource>& Sources = Step.Sources->GetSources();
	// IA1.2-Q6: a camp of food (the mill) with no berries near: a free field of its player's, one builder each (one place).
	const int32 Field = !Sources.IsValidIndex(Send->SourceIndex) && Camp.GetType()->Accepts.Contains(EHOLResource::Food)
		&& Builder.Constants.GatherRates[EHOLResource::Food] > 0.f
		? FindFreeFarmField(Step, Builder.State.OwnerPlayer, Camp.GetBodyBounds(), GetCampGatherRange(), Builder.Entity) : INDEX_NONE;
	if (Field != INDEX_NONE)
	{
		Builder.Build.Clear();
		Queues.Remove(Builder.Entity);
		Builder.Gather.StartTask(Field, Sources[Field].Resource);
		Builder.State.Activity = EHOLUnitActivity::GatherTrip;
		++Send->FieldsSent;
		return true;
	}
	if (!Sources.IsValidIndex(Send->SourceIndex) || Builder.Constants.GatherRates[Sources[Send->SourceIndex].Resource] <= 0.f)
	{
		++Send->NotSent;
		return false;
	}
	// As a gather order (UHOLGatherProcessor::ApplyGatherOrders): no more building, its queue forgotten; from the gathering's
	// next step, a free spot at the source (on the camp's side), else in its massif or a nearby grove (D309), its load kept
	// until its first unit of another resource (D306).
	const FHOLSource& Source = Sources[Send->SourceIndex];
	Builder.Build.Clear();
	Queues.Remove(Builder.Entity);
	Builder.Gather.StartTask(Send->SourceIndex, Source.Resource);
	Builder.State.Activity = EHOLUnitActivity::GatherTrip;
	++Send->Sent;
	return true;
}

bool UHOLBuildProcessor::GatherForFarm(const FStepContext& Step, FBuilder& Builder, const AHOLBuilding& Farm)
{
	// Its field born when it was finished (F6a, SetConstructionState at the step before: its builders go on at this one).
	const int32 OwnField = Farm.GetLinkedSource();
	const TArray<FHOLSource>& Sources = Step.Sources->GetSources();
	if (!Sources.IsValidIndex(OwnField) || !Sources[OwnField].IsStanding() || !Sources[OwnField].IsGatherableBy(Builder.State.OwnerPlayer)
		|| Builder.Constants.GatherRates[Sources[OwnField].Resource] <= 0.f)
	{
		return false;
	}
	// Its one place to the first of its builders (they all go on at the same step); the others to their player's nearest
	// free field near it (IA1.2-Q4, as the massif of F3), else as after a house (the nearby site, else idle).
	const int32 Field = HasFreePlace(Step, OwnField, Builder.Entity) ? OwnField
		: FindFreeFarmField(Step, Builder.State.OwnerPlayer, Farm.GetBodyBounds(), GetCampGatherRange(), Builder.Entity);
	if (Field == INDEX_NONE)
	{
		UE_LOG(LogHOLSimulation, Display, TEXT("Build: a builder of slot %d finds its farm's field taken and no free one near, goes on as after a house"),
			Builder.State.OwnerPlayer);
		return false;
	}
	// As a gather order (UHOLGatherProcessor::ApplyGatherOrders): no more building, its queue forgotten.
	const FHOLSource& Source = Sources[Field];
	Builder.Build.Clear();
	Queues.Remove(Builder.Entity);
	Builder.Gather.StartTask(Field, Source.Resource);
	Builder.State.Activity = EHOLUnitActivity::GatherTrip;
	UE_LOG(LogHOLSimulation, Display, TEXT("Build: a builder of slot %d goes on to gather %s field (source %d) at (%.0f, %.0f)"), Builder.State.OwnerPlayer,
		Field == OwnField ? TEXT("its farm's") : TEXT("a free"), Field, Source.Location.X, Source.Location.Y);
	return true;
}

int32 UHOLBuildProcessor::FindFreeFarmField(const FStepContext& Step, const uint8 Slot, const FBox2D& Near, const double Range, const FMassEntityHandle Self) const
{
	const TArray<FHOLSource>& Sources = Step.Sources->GetSources();
	int32 Found = INDEX_NONE;
	double Nearest = Range;
	for (int32 Index = 0; Index < Sources.Num(); ++Index)
	{
		const FHOLSource& Source = Sources[Index];
		// A field (a linked source of food) of Slot's own: never another player's (F3).
		if (!Source.IsStanding() || Source.OwnerSlot != Slot || Source.Resource != EHOLResource::Food || !Step.Sources->IsLinked(Index))
		{
			continue;
		}
		const double Gap = HOLConstruction::GetSourceGap(Near, FVector2D(Source.Location), static_cast<double>(Source.Radius));
		// Strictly nearer than the one kept (the first of two as near); its place looked at last (a walk of the units).
		if (Gap <= Range && (Found == INDEX_NONE || Gap < Nearest) && HasFreePlace(Step, Index, Self))
		{
			Found = Index;
			Nearest = Gap;
		}
	}
	return Found;
}

bool UHOLBuildProcessor::HasFreePlace(const FStepContext& Step, const int32 SourceIndex, const FMassEntityHandle Self) const
{
	const TArray<FHOLSource>& Sources = Step.Sources->GetSources();
	const FHOLSourceTypeRow* Type = Sources.IsValidIndex(SourceIndex) ? Step.Sources->GetType(Sources[SourceIndex].TypeIndex) : nullptr;
	if (!Type || Type->Places <= 0)
	{
		return true;
	}
	// As UHOLGatherProcessor::HasFreePlace: a holder keeps its place while its task is this source (on its way, at work, or
	// carrying its food); the builders sent this step hold theirs already (their task started). A farm finished now and
	// then: one walk of the units is nothing.
	const FMassEntityManager& EntityManager = *Step.EntityManager;
	TArray<uint32> UnitIds;
	Step.Units->GetAllUnitIds(UnitIds);
	int32 Holders = 0;
	for (const uint32 UnitId : UnitIds)
	{
		const FMassEntityHandle Entity = Step.Units->FindEntity(UnitId);
		if (Entity == Self || !EntityManager.IsEntityValid(Entity))
		{
			continue;
		}
		const FHOLGatherFragment* Gather = EntityManager.GetFragmentDataPtr<FHOLGatherFragment>(Entity);
		const FHOLUnitStateFragment* State = EntityManager.GetFragmentDataPtr<FHOLUnitStateFragment>(Entity);
		if (Gather && State && State->CurrentHealth > 0 && Gather->IsGathering() && Gather->SourceIndex == SourceIndex && ++Holders >= Type->Places)
		{
			return false;
		}
	}
	return true;
}

void UHOLBuildProcessor::LogCampSends(const FStepContext& Step)
{
	const TArray<FHOLSource>& Sources = Step.Sources->GetSources();
	for (const TPair<TWeakObjectPtr<AHOLBuilding>, FCampSend>& Pair : CampSends)
	{
		const AHOLBuilding* Camp = Pair.Key.Get();
		if (!Camp)
		{
			continue;
		}
		const FCampSend& Send = Pair.Value;
		const FVector Location = Camp->GetActorLocation();
		const FString Name = Camp->GetTypeName().ToString();
		if (!Sources.IsValidIndex(Send.SourceIndex) && Send.FieldsSent > 0)
		{
			UE_LOG(LogHOLCommands, Display, TEXT("Construction: slot %d camp %s at (%.0f, %.0f) finished: no berries within %.0f m, %d builders sent to free farm fields, %d go on as after a house"),
				Camp->GetOwnerPlayer(), *Name, Location.X, Location.Y, GetCampGatherRange() / 100., Send.FieldsSent, Send.NotSent);
			continue;
		}
		if (!Sources.IsValidIndex(Send.SourceIndex))
		{
			UE_LOG(LogHOLCommands, Display, TEXT("Construction: slot %d camp %s at (%.0f, %.0f) finished: no accepted source within %.0f m, %d builders go on as after a house"),
				Camp->GetOwnerPlayer(), *Name, Location.X, Location.Y, GetCampGatherRange() / 100., Send.NotSent);
			continue;
		}
		const FString Resource = StaticEnum<EHOLResource>()->GetNameStringByValue(static_cast<int64>(Sources[Send.SourceIndex].Resource));
		const FString NotSent = Send.NotSent > 0 ? FString::Printf(TEXT(", %d that cannot gather it go on as after a house"), Send.NotSent) : FString();
		UE_LOG(LogHOLCommands, Display, TEXT("Construction: slot %d camp %s at (%.0f, %.0f) finished: %d builders sent to gather %s (source %d, %.1f m)%s"),
			Camp->GetOwnerPlayer(), *Name, Location.X, Location.Y, Send.Sent, *Resource, Send.SourceIndex, Send.Gap / 100., *NotSent);
	}
	CampSends.Reset();
}

void UHOLBuildProcessor::Stop(FBuilder& Builder, const TCHAR* Why)
{
	if (Builder.State.Activity == EHOLUnitActivity::Building || Builder.Build.bWalkIssued)
	{
		// Stopped on a walk it gave itself, it ends that walk first.
		Builder.State.Activity = Builder.Move.Phase != EHOLMovePhase::None ? EHOLUnitActivity::Moving : EHOLUnitActivity::Idle;
	}
	Builder.Build.Clear();
	Queues.Remove(Builder.Entity);
	UE_LOG(LogHOLSimulation, Display, TEXT("Build: a builder of slot %d stops (%s), %s"), Builder.State.OwnerPlayer, Why,
		Builder.State.Activity == EHOLUnitActivity::Idle ? TEXT("idle") : TEXT("ending its walk"));
}

void UHOLBuildProcessor::IssueWalk(const FStepContext& Step, FBuilder& Builder, const FVector& Spot)
{
	const uint32 UnitId = Step.Units->FindUnitId(Builder.Entity);
	if (UnitId == 0)
	{
		Stop(Builder, TEXT("unknown unit"));
		return;
	}
	FHOLBuildFragment& Build = Builder.Build;
	Build.Spot = Spot;
	Build.OrderSerialBefore = Builder.Move.OrderSerial;
	Build.bWalkIssued = true;
	const uint32 UnitIds[] = { UnitId };
	Step.Commands->IssueMoveOrder(UnitIds, Spot, INDEX_NONE, /*bFromTask*/ true);
}

void UHOLBuildProcessor::SyncActivity(FBuilder& Builder)
{
	// Working at its site, or on its way there: never idle (the HUD's count, the "." key), even between two of its walks.
	Builder.State.Activity = Builder.Build.Phase == EHOLBuildPhase::Building ? EHOLUnitActivity::Building : EHOLUnitActivity::Moving;
}

void UHOLBuildProcessor::ApplyWork(const FStepContext& Step)
{
	using namespace UE::HeroesOfLiberty::Build;
	const float Exponent = GetDefault<UHOLBuildingSettings>()->BuilderExponent;
	const int32 Times = FMath::Clamp(Speed, 1, MaxSpeed);
	bool bInstant = false;
#if !UE_BUILD_SHIPPING
	bInstant = Instant != 0;
#endif
	UWorld* World = LastWorld.IsValid() ? const_cast<UWorld*>(LastWorld.Get()) : nullptr;
	const AHOLGameState* GameState = World ? World->GetGameState<AHOLGameState>() : nullptr;
	TMap<int32, int32> ForumBuilders;
	for (const TPair<TWeakObjectPtr<AHOLBuilding>, int32>& Pair : WorkersBySite)
	{
		AHOLBuilding* Site = Pair.Key.Get();
		const int32 Builders = Pair.Value;
		// A main forum down (V2.2, D352): the Peasants at their places go to its rules (which count them only while it
		// rebuilds: in ruin they wait), no work of a site; its block shows those at work.
		if (Site && GameState && Site->IsForumDown())
		{
			const int32 Slot = Site->GetOwnerPlayer();
			const bool bRebuilding = GameState->GetForumState(Slot).Phase == EHOLForumPhase::Rebuilding;
			ForumBuilders.Add(Slot, Builders);
			Site->SetBuildersAtWork(bRebuilding ? Builders : 0);
			FSiteStats& Stats = SiteStats.FindOrAdd(Pair.Key);
			if (bRebuilding && Stats.LastBuilders != Builders)
			{
				const FVector Location = Site->GetActorLocation();
				UE_LOG(LogHOLCommands, Display, TEXT("Construction: slot %d rebuilds its forum at (%.0f, %.0f), %d builders at work"), Slot, Location.X, Location.Y,
					Builders);
				Stats.LastBuilders = Builders;
			}
			continue;
		}
		if (!Site || Site->IsFinished() || Builders <= 0)
		{
			continue;
		}
		const FVector Location = Site->GetActorLocation();
		FSiteStats& Stats = SiteStats.FindOrAdd(Pair.Key);
		// The first blow (D312): seen by everyone, the footprint blocks the navmesh (units inside are put out by the
		// footprints' guard, D310), it starts rising.
		if (Site->IsFoundation())
		{
			Stats = FSiteStats();
			Stats.FirstStep = Step.StepIndex;
			Site->SetConstructionState(EHOLConstructionState::UnderConstruction);
			UE_LOG(LogHOLCommands, Display, TEXT("Construction: slot %d first work on %s at (%.0f, %.0f), %d builders"), Site->GetOwnerPlayer(),
				*Site->GetTypeName().ToString(), Location.X, Location.Y, Builders);
		}
		if (Stats.LastBuilders != Builders && Stats.WorkSteps > 0)
		{
			UE_LOG(LogHOLCommands, Display, TEXT("Construction: slot %d %s at (%.0f, %.0f), %d builders at work"), Site->GetOwnerPlayer(),
				*Site->GetTypeName().ToString(), Location.X, Location.Y, Builders);
		}
		Stats.LastBuilders = Builders;
		// The site's block shows them to its owner (J6a.5, D313).
		Site->SetBuildersAtWork(Builders);
		const int64 Total = Site->GetTotalWork();
		int64 Work = Site->GetConstructionWork();
		bool bFinished = Total <= 0;
		if (bInstant)
		{
			// Finished at its first blow (hol.Build.Instant, development).
			Work = FMath::Max<int64>(Total, 0);
			bFinished = true;
		}
		for (int32 Time = 0; Time < Times && !bFinished; ++Time)
		{
			bFinished = HOLConstruction::AddWork(Work, Total, Builders, Exponent);
		}
		Site->SetConstructionWork(Work);
		++Stats.WorkSteps;
		++Stats.StepsByBuilders[FMath::Min(Builders, static_cast<int32>(UE_ARRAY_COUNT(Stats.StepsByBuilders))) - 1];
		if (!bFinished)
		{
			continue;
		}
		// Finished: it serves (population, deposit; D312); its builders go on at the next step.
		Site->SetConstructionState(EHOLConstructionState::Finished);
		Site->SetBuildersAtWork(0);
		TArray<FString> ByBuilders;
		for (int32 Index = 0; Index < UE_ARRAY_COUNT(Stats.StepsByBuilders); ++Index)
		{
			if (Stats.StepsByBuilders[Index] > 0)
			{
				ByBuilders.Add(FString::Printf(TEXT("%d x%d"), Index + 1, Stats.StepsByBuilders[Index]));
			}
		}
		UE_LOG(LogHOLCommands, Display, TEXT("Construction: slot %d finished %s at (%.0f, %.0f) after %d steps of work (by builders: %s), speed %d%s"),
			Site->GetOwnerPlayer(), *Site->GetTypeName().ToString(), Location.X, Location.Y, Stats.WorkSteps, *FString::Join(ByBuilders, TEXT(", ")), Times,
			bInstant ? TEXT(", instant") : TEXT(""));
		SiteStats.Remove(Pair.Key);
	}
	// The stats of sites cancelled or gone; a site nobody works on any more shows no builder (J6a.5).
	for (auto It = SiteStats.CreateIterator(); It; ++It)
	{
		if (!It.Key().IsValid())
		{
			It.RemoveCurrent();
		}
		else if (!WorkersBySite.Contains(It.Key()))
		{
			It.Key()->SetBuildersAtWork(0);
			// A main forum rebuilt (V2.2): nothing more to keep.
			if (!It.Key()->NeedsBuilders())
			{
				It.RemoveCurrent();
			}
		}
	}
	PublishForumBuilders(World, ForumBuilders);
}

void UHOLBuildProcessor::PublishForumBuilders(UWorld* World, const TMap<int32, int32>& BuildersBySlot)
{
	AHOLGameState* GameState = World ? World->GetGameState<AHOLGameState>() : nullptr;
	if (!GameState)
	{
		ForumBuilderSlots = 0;
		return;
	}
	uint32 Slots = 0;
	for (const TPair<int32, int32>& Pair : BuildersBySlot)
	{
		if (Pair.Key >= 0 && Pair.Key < 32)
		{
			GameState->SetForumBuilders(Pair.Key, Pair.Value);
			Slots |= 1u << Pair.Key;
		}
	}
	// The slots that had Peasants at their forum and have none any more.
	for (int32 Slot = 0; Slot < 32; ++Slot)
	{
		if ((ForumBuilderSlots & ~Slots) & (1u << Slot))
		{
			GameState->SetForumBuilders(Slot, 0);
		}
	}
	ForumBuilderSlots = Slots;
}
