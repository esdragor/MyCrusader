// Copyright Epic Games, Inc. All Rights Reserved.

#include "Misc/AutomationTest.h"
#include "Simulation/HOLCombatRules.h"
#include "Simulation/HOLOrderChainRules.h"

#if WITH_DEV_AUTOMATION_TESTS

// J7b.5b-1a checks (D328): the chains of queued orders, one per common queue.
namespace UE::HeroesOfLiberty::Tests::OrderChains
{
	constexpr EAutomationTestFlags Flags = EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLOrderChainsTest, "HeroesOfLiberty.Orders.Chains", UE::HeroesOfLiberty::Tests::OrderChains::Flags)

bool FHOLOrderChainsTest::RunTest(const FString& Parameters)
{
	using namespace HOLOrderChain;
	const FStep MoveA = FStep::MakeMove(FVector(1000., 0., 0.));
	const FStep MoveB = FStep::MakeMove(FVector(2000., 500., 0.));
	const FStep AttackMoveC = FStep::MakeMove(FVector(3000., 0., 0.), true);
	TArray<FChain> Chains;

	// A group sharing a queue: one chain, its units in order.
	{
		TArray<FUnitSteps> Units;
		for (uint32 Id = 1; Id <= 4; ++Id)
		{
			Units.Add({Id, {MoveA, MoveB, AttackMoveC}});
		}
		MakeChains(Units, Chains);
		TestEqual(TEXT("Group: one chain"), Chains.Num(), 1);
		TestEqual(TEXT("Group: 4 units"), Chains.Num() == 1 ? Chains[0].UnitIds.Num() : 0, 4);
		TestEqual(TEXT("Group: 3 steps"), Chains.Num() == 1 ? Chains[0].Steps.Num() : 0, 3);
	}
	// Peasants with different queues: one chain each, units without steps left out, order stable.
	{
		const TArray<FUnitSteps> Units = {
			{10, {FStep::MakeBuild(7), FStep::MakeGather(3)}},
			{11, {}},
			{12, {FStep::MakeBuild(7), FStep::MakeGather(4)}},
			{13, {FStep::MakeBuild(7), FStep::MakeGather(3)}},
		};
		MakeChains(Units, Chains);
		TestEqual(TEXT("Peasants: two chains"), Chains.Num(), 2);
		if (Chains.Num() == 2)
		{
			TestTrue(TEXT("Peasants: first chain #10 #13"), Chains[0].UnitIds == TArray<uint32>{10, 13});
			TestTrue(TEXT("Peasants: second chain #12"), Chains[1].UnitIds == TArray<uint32>{12});
		}
	}
	// A mix, in order: move, attack, attack-move, build, gather.
	{
		const TArray<FStep> Mix = {MoveA, FStep::MakeAttack(42), AttackMoveC, FStep::MakeBuild(9), FStep::MakeGather(5)};
		MakeChains({FUnitSteps{1, Mix}}, Chains);
		TestEqual(TEXT("Mix: one chain"), Chains.Num(), 1);
		if (Chains.Num() == 1 && Chains[0].Steps.Num() == 5)
		{
			const EStepKind Expected[] = {EStepKind::Move, EStepKind::Attack, EStepKind::AttackMove, EStepKind::Build, EStepKind::Gather};
			for (int32 Index = 0; Index < 5; ++Index)
			{
				TestEqual(*FString::Printf(TEXT("Mix: step %d is %s"), Index, KindName(Expected[Index])), static_cast<int32>(Chains[0].Steps[Index].Kind), static_cast<int32>(Expected[Index]));
			}
			TestEqual(TEXT("Mix: attack target"), Chains[0].Steps[1].TargetId, 42u);
			TestEqual(TEXT("Mix: gather source"), Chains[0].Steps[4].SourceIndex, 5);
		}
		else
		{
			AddError(TEXT("Mix: 5 steps expected"));
		}
		// The same points but an attack-move instead of a move: another chain.
		MakeChains({FUnitSteps{1, {MoveA}}, FUnitSteps{2, {FStep::MakeMove(MoveA.Point, true)}}}, Chains);
		TestEqual(TEXT("Move and attack-move to the same point: two chains"), Chains.Num(), 2);
	}
	// A rally (D328): units born together share one white step to their point; a move to the same point is another chain.
	{
		const FStep Rally = FStep::MakeRally(FVector(500., 200., 0.));
		MakeChains({FUnitSteps{3, {Rally}}, FUnitSteps{4, {Rally}}, FUnitSteps{5, {FStep::MakeMove(Rally.Point)}}}, Chains);
		TestEqual(TEXT("Rally: two chains"), Chains.Num(), 2);
		if (Chains.Num() == 2)
		{
			TestTrue(TEXT("Rally: born together share it"), Chains[0].UnitIds == TArray<uint32>{3, 4});
			TestEqual(TEXT("Rally: its kind"), static_cast<int32>(Chains[0].Steps[0].Kind), static_cast<int32>(EStepKind::Rally));
			TestEqual(TEXT("Rally: its name"), FString(KindName(EStepKind::Rally)), FString(TEXT("Rally")));
		}
	}
	// The bound: at most MaxSteps steps; queues alike up to the bound share a chain.
	{
		TArray<FStep> Long;
		for (int32 Index = 0; Index < MaxSteps + 5; ++Index)
		{
			Long.Add(FStep::MakeMove(FVector(100. * Index, 0., 0.)));
		}
		TArray<FStep> Longer = Long;
		Longer.Add(FStep::MakeGather(1));
		MakeChains({FUnitSteps{1, Long}, FUnitSteps{2, Longer}}, Chains);
		TestEqual(TEXT("Bound: one chain"), Chains.Num(), 1);
		TestEqual(TEXT("Bound: MaxSteps steps"), Chains.Num() == 1 ? Chains[0].Steps.Num() : 0, MaxSteps);
	}
	return true;
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
		FBuildingRef Target;
		FBuildingRef Ordered;
		TArray<HOLOrderChain::FStep> Queue;

		bool IsOver() const { return HOLOrderChain::IsCurrentOrderOver(bWalking, bOrderedTarget, !Ordered.IsExplicitlyNull(), false); }

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

	// A building attack queued after a move: it waits for the walk, then strikes; the building falls: its order is over.
	{
		FUnitModel Unit;
		Unit.bWalking = true;
		Unit.Queue = {FStep::MakeAttackBuilding(7)};
		TestFalse(TEXT("After a move: waits while it walks"), Unit.Advance());
		Unit.bWalking = false;
		TestTrue(TEXT("After a move: starts at the arrival"), Unit.Advance());
		TestEqual(TEXT("After a move: strikes building 7"), Unit.Target.Id, 7);
		TestFalse(TEXT("After a move: its order goes on while the building stands"), Unit.IsOver());
		EndBuildingTarget(Unit.Target, Unit.Ordered);
		TestTrue(TEXT("After a move: the building fallen, its order is over"), Unit.IsOver());
	}
	// Two buildings queued (Shift): both fall, in order; never dropped for an enemy unit meanwhile (D345).
	{
		FUnitModel Unit;
		Unit.Queue = {FStep::MakeAttackBuilding(1), FStep::MakeAttackBuilding(2)};
		TestTrue(TEXT("Two buildings: the first starts"), Unit.Advance());
		TestFalse(TEXT("Two buildings: the first never dropped for an enemy unit"), Unit.MaySeek());
		TestFalse(TEXT("Two buildings: the second waits"), Unit.Advance());
		TestEqual(TEXT("Two buildings: still on the first"), Unit.Ordered.Id, 1);
		EndBuildingTarget(Unit.Target, Unit.Ordered);
		TestTrue(TEXT("Two buildings: the first fallen, the second starts"), Unit.Advance());
		TestEqual(TEXT("Two buildings: strikes the second"), Unit.Target.Id, 2);
		TestFalse(TEXT("Two buildings: the second never dropped for an enemy unit"), Unit.MaySeek());
		TestEqual(TEXT("Two buildings: queue empty"), Unit.Queue.Num(), 0);
		EndBuildingTarget(Unit.Target, Unit.Ordered);
		TestTrue(TEXT("Two buildings: both fallen, its order over"), Unit.IsOver());
		TestTrue(TEXT("Two buildings: both fallen, it takes on enemy units again"), Unit.MaySeek());
	}
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
	// A building attack without Shift, then a move with Shift: the move waits for the fall.
	{
		FUnitModel Unit;
		Unit.Target.Id = Unit.Ordered.Id = 3;
		Unit.Queue = {FStep::MakeMove(FVector(100., 0., 0.))};
		TestFalse(TEXT("Then a move: waits while the building stands"), Unit.Advance());
		EndBuildingTarget(Unit.Target, Unit.Ordered);
		TestTrue(TEXT("Then a move: starts at the fall"), Unit.Advance());
		TestTrue(TEXT("Then a move: walks"), Unit.bWalking);
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
		// A building taken on by itself ends without an order to end.
		EndBuildingTarget(Rest.Target, Rest.Ordered);
		TestTrue(TEXT("At rest: its building fallen, nothing left"), Rest.Target.IsExplicitlyNull() && Rest.Ordered.IsExplicitlyNull());
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
	return true;
}

#endif
