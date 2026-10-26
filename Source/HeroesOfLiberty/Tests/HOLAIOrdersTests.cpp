// Copyright Epic Games, Inc. All Rights Reserved.

#include "AI/HOLAIOrders.h"
#include "Commands/HOLCommandSubsystem.h"
#include "Commands/HOLPlayerOrders.h"
#include "Commands/HOLUnitOrderProcessor.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "MassEntitySubsystem.h"
#include "MassExecutor.h"
#include "MassProcessingContext.h"
#include "Match/HOLGameState.h"
#include "Match/HOLMatchConfig.h"
#include "Misc/AutomationTest.h"
#include "Resources/HOLSourceSubsystem.h"
#include "Resources/HOLSourceTypes.h"
#include "Simulation/HOLGatherProcessor.h"
#include "Simulation/HOLSimulationSettings.h"
#include "Simulation/HOLSimulationSubsystem.h"
#include "Simulation/HOLUnitSubsystem.h"
#include "Simulation/HOLUnitView.h"

#if WITH_DEV_AUTOMATION_TESTS

// IA0.2 checks: an AI slot's orders without a controller (UHOLAIOrderLibrary) go through the same rule as a human's
// (HOLPlayerOrders, which AHOLPlayerController's RPCs call): the queue, applied at the next step by the real processors,
// to the slot's own units only, at most MaxOrderedUnits, the soldiers of an order to gather walking to its source.
namespace UE::HeroesOfLiberty::Tests::AIOrders
{
	constexpr EAutomationTestFlags Flags = EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter;

	const FName Peasant(TEXT("Peasant"));
	const FName Spearman(TEXT("Spearman"));

	/** Units of Type born for Owner around Location (no walk: no movement processor runs); their identifiers. */
	TArray<uint32> PlaceUnits(UHOLUnitSubsystem& Units, const FName Type, const int32 Owner, const FVector& Location, const int32 Count)
	{
		TArray<uint32> Before;
		Units.GetAllUnitIds(Before);
		FHOLUnitSpawnRequest Request;
		Request.TypeName = Type;
		Request.Count = Count;
		Request.OwnerPlayer = static_cast<uint8>(Owner);
		Request.Center = Location;
		Units.RequestUnits(Request);
		Units.ServeBirthRequests();
		TArray<uint32> After;
		Units.GetAllUnitIds(After);
		After.RemoveAll([&Before](const uint32 Id) { return Before.Contains(Id); });
		After.Sort();
		return After;
	}

	TArray<int32> ToScript(const TArray<uint32>& Ids)
	{
		TArray<int32> Out;
		for (const uint32 Id : Ids)
		{
			Out.Add(static_cast<int32>(Id));
		}
		return Out;
	}

	EHOLUnitActivity GetActivity(const UWorld& World, const uint32 UnitId)
	{
		FHOLUnitView View;
		return HOLUnitView::Find(World, UnitId, View) ? View.Activity : EHOLUnitActivity::Dying;
	}

	/** How many of Ids do Activity. */
	int32 CountDoing(const UWorld& World, const TArray<uint32>& Ids, const EHOLUnitActivity Activity)
	{
		int32 Count = 0;
		for (const uint32 Id : Ids)
		{
			Count += GetActivity(World, Id) == Activity ? 1 : 0;
		}
		return Count;
	}

	/**
	 * A test world as a match's server: a game state (slot 0 human, slot 1 AI: no PlayerState, no controller), the
	 * subsystems, a tree, and the step's processors that apply the queue, in the pipeline's order (orders, then gathering).
	 */
	struct FWorld
	{
		UWorld* World = nullptr;
		UHOLUnitSubsystem* Units = nullptr;
		UHOLSourceSubsystem* Sources = nullptr;
		UHOLCommandSubsystem* Commands = nullptr;
		TSharedPtr<FMassEntityManager> EntityManager;
		UHOLGatherProcessor* Gather = nullptr;
		UHOLUnitOrderProcessor* Orders = nullptr;
		int32 Tree = INDEX_NONE;

		FWorld()
		{
			World = UWorld::CreateWorld(EWorldType::Game, /*bInformEngineOfWorld*/ false);
			FWorldContext& Context = GEngine->CreateNewWorldContext(EWorldType::Game);
			Context.SetCurrentWorld(World);
			World->InitializeActorsForPlay(FURL());
			AHOLGameState* State = World->SpawnActor<AHOLGameState>();
			Units = World->GetSubsystem<UHOLUnitSubsystem>();
			Sources = World->GetSubsystem<UHOLSourceSubsystem>();
			Commands = World->GetSubsystem<UHOLCommandSubsystem>();
			UHOLSimulationSubsystem* Simulation = World->GetSubsystem<UHOLSimulationSubsystem>();
			UMassEntitySubsystem* EntitySubsystem = World->GetSubsystem<UMassEntitySubsystem>();
			if (!State || !Units || !Sources || !Commands || !Simulation || !EntitySubsystem)
			{
				return;
			}
			FHOLMatchConfig Config = FHOLMatchConfig::MakeDefault();
			for (int32 Index = 0; Index < Config.Slots.Num(); ++Index)
			{
				Config.Slots[Index].State = Index == 0 ? EHOLSlotState::Human : (Index == 1 ? EHOLSlotState::AI : EHOLSlotState::Closed);
			}
			State->InitVictory(Config);
			Sources->OnWorldBeginPlay(*World);
			Tree = Sources->AddSource(HOLSourceTypeNames::Tree, FVector(2000.0, 0.0, 0.0));
			EntityManager = EntitySubsystem->GetMutableEntityManager().AsShared();
			Gather = NewObject<UHOLGatherProcessor>(Simulation);
			Gather->CallInitialize(Simulation, EntityManager.ToSharedRef());
			Orders = NewObject<UHOLUnitOrderProcessor>(Simulation);
			Orders->CallInitialize(Simulation, EntityManager.ToSharedRef());
		}

		~FWorld()
		{
			GEngine->DestroyWorldContext(World);
			World->DestroyWorld(false);
		}

		bool IsReady() const { return Gather && Orders && Tree != INDEX_NONE; }

		/** One fixed step of the queue's processors (the test world runs no movement): the order processor, then the gathering. */
		void RunStep()
		{
			const float StepSeconds = GetDefault<UHOLSimulationSettings>()->GetStepSeconds();
			{
				FMassProcessingContext ProcessingContext(EntityManager, StepSeconds);
				UE::Mass::Executor::Run(*Orders, ProcessingContext);
			}
			{
				FMassProcessingContext ProcessingContext(EntityManager, StepSeconds);
				UE::Mass::Executor::Run(*Gather, ProcessingContext);
			}
		}
	};
}

// The rule alone: a player's slot (never INDEX_NONE, the server's own orders every unit obeys), 200 units at most.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLAIOrdersRuleTest, "HeroesOfLiberty.AI.Orders.Rule", UE::HeroesOfLiberty::Tests::AIOrders::Flags)
bool FHOLAIOrdersRuleTest::RunTest(const FString& Parameters)
{
	TestEqual(TEXT("the cap: a player's whole population (D03)"), HOLPlayerOrders::MaxOrderedUnits, 200);
	TestTrue(TEXT("slot 1, 200 units: accepted"), HOLPlayerOrders::IsAccepted(1, 200));
	TestTrue(TEXT("slot 0, one unit: accepted"), HOLPlayerOrders::IsAccepted(0, 1));
	TestTrue(TEXT("slot 7, no unit (a foundation placed alone): accepted"), HOLPlayerOrders::IsAccepted(FHOLMatchConfig::MaxSlots - 1, 0));
	TestFalse(TEXT("slot 1, 201 units: refused"), HOLPlayerOrders::IsAccepted(1, 201));
	TestFalse(TEXT("no slot (INDEX_NONE): refused"), HOLPlayerOrders::IsAccepted(INDEX_NONE, 1));
	TestFalse(TEXT("slot 8: refused"), HOLPlayerOrders::IsAccepted(FHOLMatchConfig::MaxSlots, 1));
	return true;
}

// An AI slot (1, no PlayerState nor controller) sends its Peasants to a tree: the order enters the queue, nobody moves in
// this frame, the next step sets them gathering; slot 0's Peasant slipped into the list is refused (D288). A move order
// of its Spearman likewise.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLAIOrdersGatherTest, "HeroesOfLiberty.AI.Orders.Gather", UE::HeroesOfLiberty::Tests::AIOrders::Flags)
bool FHOLAIOrdersGatherTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::AIOrders;
	FWorld Test;
	if (!TestTrue(TEXT("test world ready"), Test.IsReady()))
	{
		return false;
	}
	const UWorld& World = *Test.World;
	const TArray<uint32> Mine = PlaceUnits(*Test.Units, Peasant, 1, FVector(1000.0, 0.0, 0.0), 3);
	const TArray<uint32> Other = PlaceUnits(*Test.Units, Peasant, 0, FVector(1000.0, 800.0, 0.0), 1);
	const TArray<uint32> Soldier = PlaceUnits(*Test.Units, Spearman, 1, FVector(0.0, 800.0, 0.0), 1);
	TestTrue(TEXT("three Peasants of slot 1, one of slot 0, a Spearman of slot 1"), Mine.Num() == 3 && Other.Num() == 1 && Soldier.Num() == 1);

	AddExpectedMessage(FString::Printf(TEXT("Gather order of slot 1: 3 units to source %d \\(Wood\\), 1 refused"), Test.Tree),
		EAutomationExpectedMessageFlags::Contains, 1);
	TArray<uint32> Named = Mine;
	Named.Append(Other);
	TestTrue(TEXT("the gather order enters the queue"), UHOLAIOrderLibrary::IssueAIGatherOrder(Test.World, 1, ToScript(Named), Test.Tree));
	TestTrue(TEXT("the move order enters the queue"), UHOLAIOrderLibrary::IssueAIMoveOrder(Test.World, 1, ToScript(Soldier), FVector(-1500.0, 800.0, 0.0)));
	TestEqual(TEXT("not applied in the frame that issued them: slot 1's Peasants idle"), CountDoing(World, Mine, EHOLUnitActivity::Idle), 3);
	TestEqual(TEXT("not applied in the frame: the Spearman idle"), CountDoing(World, Soldier, EHOLUnitActivity::Idle), 1);

	Test.RunStep();
	TestEqual(TEXT("next step: slot 1's three Peasants on their way to the tree"), CountDoing(World, Mine, EHOLUnitActivity::GatherTrip), 3);
	TestEqual(TEXT("slot 0's Peasant refused, idle"), CountDoing(World, Other, EHOLUnitActivity::Idle), 1);
	TestEqual(TEXT("slot 1's Spearman walking"), CountDoing(World, Soldier, EHOLUnitActivity::Moving), 1);
	return true;
}

// The cap of a human's order holds for an AI slot: 201 units refused whole (nobody moves), 200 accepted; no slot
// (INDEX_NONE, which every unit would obey) refused; the rule the controller calls (HOLPlayerOrders) refuses the same.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLAIOrdersCapTest, "HeroesOfLiberty.AI.Orders.Cap", UE::HeroesOfLiberty::Tests::AIOrders::Flags)
bool FHOLAIOrdersCapTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::AIOrders;
	FWorld Test;
	if (!TestTrue(TEXT("test world ready"), Test.IsReady()))
	{
		return false;
	}
	const UWorld& World = *Test.World;
	const TArray<uint32> Crowd = PlaceUnits(*Test.Units, Peasant, 1, FVector(0.0, 0.0, 0.0), HOLPlayerOrders::MaxOrderedUnits + 1);
	if (!TestEqual(TEXT("201 Peasants of slot 1"), Crowd.Num(), HOLPlayerOrders::MaxOrderedUnits + 1))
	{
		return false;
	}

	TestFalse(TEXT("201 units: the AI's order refused"), UHOLAIOrderLibrary::IssueAIGatherOrder(Test.World, 1, ToScript(Crowd), Test.Tree));
	TestFalse(TEXT("201 units: the human's rule refuses it too"), HOLPlayerOrders::Gather(*Test.World, 1, Crowd, Test.Tree));
	TestFalse(TEXT("201 units: a move refused too"), UHOLAIOrderLibrary::IssueAIMoveOrder(Test.World, 1, ToScript(Crowd), FVector(0.0, 3000.0, 0.0)));
	TestFalse(TEXT("no slot: refused"), UHOLAIOrderLibrary::IssueAIGatherOrder(Test.World, INDEX_NONE, ToScript(TArray<uint32>(Crowd.GetData(), 10)), Test.Tree));
	Test.RunStep();
	TestEqual(TEXT("nobody moved"), CountDoing(World, Crowd, EHOLUnitActivity::Idle), Crowd.Num());

	// The gathering takes the 200 (its log, the real count: one tree has room for a few, the others find no spot and stand).
	AddExpectedMessage(FString::Printf(TEXT("Gather order of slot 1: 200 units to source %d \\(Wood\\)"), Test.Tree), EAutomationExpectedMessageFlags::Contains, 1);
	const TArray<uint32> Most(Crowd.GetData(), HOLPlayerOrders::MaxOrderedUnits);
	TestTrue(TEXT("200 units: accepted"), UHOLAIOrderLibrary::IssueAIGatherOrder(Test.World, 1, ToScript(Most), Test.Tree));
	Test.RunStep();
	TestTrue(TEXT("some of the 200 on their way to the tree"), CountDoing(World, Most, EHOLUnitActivity::GatherTrip) > 0);
	TestEqual(TEXT("the last one idle"), GetActivity(World, Crowd.Last()), EHOLUnitActivity::Idle);
	return true;
}

// The soldiers of an AI's order to gather walk to the source (J7a.2), as a human's: the slot's own only, slot 0's
// Spearman slipped in stays in the gather order, refused (no CarryCapacity); an order of soldiers alone is a walk.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLAIOrdersNonWorkersTest, "HeroesOfLiberty.AI.Orders.NonWorkers", UE::HeroesOfLiberty::Tests::AIOrders::Flags)
bool FHOLAIOrdersNonWorkersTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::AIOrders;
	FWorld Test;
	if (!TestTrue(TEXT("test world ready"), Test.IsReady()))
	{
		return false;
	}
	const UWorld& World = *Test.World;
	const TArray<uint32> Workers = PlaceUnits(*Test.Units, Peasant, 1, FVector(1000.0, 0.0, 0.0), 2);
	const TArray<uint32> Soldiers = PlaceUnits(*Test.Units, Spearman, 1, FVector(0.0, 800.0, 0.0), 2);
	const TArray<uint32> OtherSoldier = PlaceUnits(*Test.Units, Spearman, 0, FVector(0.0, -800.0, 0.0), 1);
	const TArray<uint32> Lone = PlaceUnits(*Test.Units, Spearman, 1, FVector(-800.0, 0.0, 0.0), 1);
	TestTrue(TEXT("two Peasants and three Spearmen of slot 1, a Spearman of slot 0"), Workers.Num() == 2 && Soldiers.Num() == 2
		&& OtherSoldier.Num() == 1 && Lone.Num() == 1);

	AddExpectedMessage(TEXT("Order of slot 1: 2 units that do not work walk to \\(2000, 0\\)"), EAutomationExpectedMessageFlags::Contains, 1);
	AddExpectedMessage(TEXT("Order of slot 1: 1 units that do not work walk to \\(2000, 0\\)"), EAutomationExpectedMessageFlags::Contains, 1);
	AddExpectedMessage(FString::Printf(TEXT("Gather order of slot 1: 2 units to source %d \\(Wood\\), 1 refused"), Test.Tree),
		EAutomationExpectedMessageFlags::Contains, 1);
	TArray<uint32> Named = Workers;
	Named.Append(Soldiers);
	Named.Append(OtherSoldier);
	TestTrue(TEXT("the order enters the queue"), UHOLAIOrderLibrary::IssueAIGatherOrder(Test.World, 1, ToScript(Named), Test.Tree));
	TestTrue(TEXT("soldiers alone: a walk enters the queue"), UHOLAIOrderLibrary::IssueAIGatherOrder(Test.World, 1, ToScript(Lone), Test.Tree));

	Test.RunStep();
	TestEqual(TEXT("the Peasants on their way to the tree"), CountDoing(World, Workers, EHOLUnitActivity::GatherTrip), 2);
	TestEqual(TEXT("slot 1's Spearmen walking to it"), CountDoing(World, Soldiers, EHOLUnitActivity::Moving), 2);
	TestEqual(TEXT("the lone Spearman walking to it"), CountDoing(World, Lone, EHOLUnitActivity::Moving), 1);
	TestEqual(TEXT("slot 0's Spearman idle"), CountDoing(World, OtherSoldier, EHOLUnitActivity::Idle), 1);
	return true;
}

#endif
