// Copyright Epic Games, Inc. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "Simulation/HOLCombatRules.h"
#include "Simulation/HOLOrderChainRules.h"

#if WITH_DEV_AUTOMATION_TESTS
// J7b.5b-1a: the Orders.Chains test moved to script (HOLOrderChain_Test.as, L17-09); the shared Flags stay here.
// J7b.5b-1a checks (D328): the chains of queued orders, one per common queue.
namespace UE::HeroesOfLiberty::Tests::OrderChains
{
	constexpr EAutomationTestFlags Flags = EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter;
}

// D345, Shift checks: Shift + right click on an enemy building, its attack in the queue (the order processor's and the combat
// processor's rules, on a model of one unit: its walk, its ordered target, its ordered building, its queue).
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLOrderBuildingQueueTest, "HeroesOfLiberty.Orders.BuildingQueue", UE::HeroesOfLiberty::Tests::OrderChains::Flags)

namespace UE::HeroesOfLiberty::Tests::OrderChains
{
	/** A building reference as TWeakObjectPtr is used by the rules (0: none). */
	struct FBuildingRef
	{
		int32 Id = 0;
		bool IsExplicitlyNull() const { return Id == 0; }
		void Reset() { Id = 0; }
		bool operator==(const FBuildingRef& Other) const { return Id == Other.Id; }
	};

	/** One unit: its current order and its queue, advanced as UHOLUnitOrderProcessor::StartQueuedMoves does. */
	struct FUnitModel
	{
		bool bWalking = false;
		bool bOrderedTarget = false;
		/** On its way to unload by its player's order (D485, L.5b). */
		bool bDroppingOff = false;
		FBuildingRef Target;
		FBuildingRef Ordered;
		TArray<HOLOrderChain::FStep> Queue;

		bool IsOver() const { return HOLOrderChain::IsCurrentOrderOver(bWalking, bOrderedTarget, !Ordered.IsExplicitlyNull(), false, bDroppingOff); }

		/** It takes on an enemy unit by itself (search, riposte, zone of control), dropping a building it strikes (the combat processor's rule). */
		bool MaySeek(const bool bAttackMove = false) const { return HOLCombat::MaySeek(bWalking, bAttackMove, bOrderedTarget, !Ordered.IsExplicitlyNull()); }

		/** The next queued order starts if the current one is over; true if one started. */
		bool Advance()
		{
			if (!IsOver() || Queue.IsEmpty())
			{
				return false;
			}
			const HOLOrderChain::FStep Next = Queue[0];
			Queue.RemoveAt(0);
			switch (Next.Kind)
			{
			case HOLOrderChain::EStepKind::Attack: bOrderedTarget = true; break;
			case HOLOrderChain::EStepKind::AttackBuilding: Target.Id = Ordered.Id = static_cast<int32>(Next.TargetId); break;
			case HOLOrderChain::EStepKind::Deposit: bDroppingOff = true; break;
			default: bWalking = true; break;
			}
			return true;
		}
	};
}

bool FHOLOrderBuildingQueueTest::RunTest(const FString& Parameters)
{
	using namespace HOLOrderChain;
	using UE::HeroesOfLiberty::Tests::OrderChains::FUnitModel;

	// Shift on a building after an attack on a unit: it waits for the target's death.
	{
		FUnitModel Unit;
		Unit.bOrderedTarget = true;
		Unit.Queue = {FStep::MakeAttackBuilding(5)};
		TestFalse(TEXT("After a unit attack: waits while the target lives"), Unit.Advance());
		Unit.bOrderedTarget = false;
		TestTrue(TEXT("After a unit attack: starts at the target's death"), Unit.Advance());
		TestEqual(TEXT("After a unit attack: strikes building 5"), Unit.Ordered.Id, 5);
	}
	// The drop (D345): only a building taken on by itself (at rest, on an attack-move) is dropped for an enemy unit; an ordered
	// one (a right click, Shift or not) never, its queue waiting for its fall.
	{
		FUnitModel Ordered;
		Ordered.Target.Id = Ordered.Ordered.Id = 4;
		Ordered.Queue = {FStep::MakeMove(FVector(100., 0., 0.))};
		TestFalse(TEXT("Ordered: never dropped for an enemy unit"), Ordered.MaySeek());
		TestFalse(TEXT("Ordered: its queue waits while it stands"), Ordered.Advance());
		TestEqual(TEXT("Ordered: still struck"), Ordered.Target.Id, 4);
		FUnitModel Rest;
		Rest.Target.Id = 9;
		TestTrue(TEXT("At rest: its own building dropped for an enemy unit"), Rest.MaySeek());
		FUnitModel AttackMove;
		AttackMove.bWalking = true;
		AttackMove.Target.Id = 9;
		TestTrue(TEXT("Attack-move: its own building dropped for an enemy unit"), AttackMove.MaySeek(/*bAttackMove*/ true));
		TestFalse(TEXT("A plain walk: nobody taken on"), AttackMove.MaySeek(/*bAttackMove*/ false));
	}
	// Its chain step (D328): red as an attack, to the building; not the same step as a build of the same id.
	{
		const FStep Step = FStep::MakeAttackBuilding(11);
		TestEqual(TEXT("Step: its name"), FString(KindName(Step.Kind)), FString(TEXT("AttackBuilding")));
		TestEqual(TEXT("Step: its building"), Step.TargetId, 11u);
		TArray<FChain> Chains;
		MakeChains({FUnitSteps{1, {Step}}, FUnitSteps{2, {Step}}, FUnitSteps{3, {FStep::MakeBuild(11)}}}, Chains);
		TestEqual(TEXT("Step: two chains (attack, build)"), Chains.Num(), 2);
	}
	// Shift: a move, a drop-off, a move (D485, EtapeDefini L.5b): the drop-off waits for the first walk, the last move for the
	// unload (its walk to the deposit is the gathering's, not a player's).
	{
		FUnitModel Unit;
		Unit.bWalking = true;
		Unit.Queue = {FStep::MakeDeposit(4), FStep::MakeMove(FVector(100., 0., 0.), false)};
		TestFalse(TEXT("Drop-off: waits while it walks"), Unit.Advance());
		Unit.bWalking = false;
		TestTrue(TEXT("Drop-off: starts at the arrival"), Unit.Advance());
		TestTrue(TEXT("Drop-off: on its way to unload"), Unit.bDroppingOff);
		TestFalse(TEXT("Drop-off: the next move waits for the unload"), Unit.Advance());
		Unit.bDroppingOff = false;
		TestTrue(TEXT("Drop-off: unloaded, the next move starts"), Unit.Advance());
		TestEqual(TEXT("Drop-off: queue empty"), Unit.Queue.Num(), 0);
		TestEqual(TEXT("Drop-off: its step's name"), FString(KindName(EStepKind::Deposit)), FString(TEXT("Deposit")));
	}
	return true;
}

#endif
