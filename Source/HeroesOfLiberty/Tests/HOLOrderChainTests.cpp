// Copyright Epic Games, Inc. All Rights Reserved.

#include "Misc/AutomationTest.h"
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

#endif
