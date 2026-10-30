// Copyright Epic Games, Inc. All Rights Reserved.

#include "AI/HOLAIOrders.h"
#include "Buildings/HOLBuilding.h"
#include "Buildings/HOLBuildingTypes.h"
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
#include "Simulation/HOLSimulationFragments.h"
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

namespace UE::HeroesOfLiberty::Tests::AIOrders
{
	/** The combat fragment of a unit (null if it has none). */
	const FHOLCombatFragment* GetCombat(const FWorld& Test, const uint32 UnitId)
	{
		const FMassEntityHandle Entity = Test.Units->FindEntity(UnitId);
		return Test.EntityManager->IsEntityValid(Entity) ? Test.EntityManager->GetFragmentDataPtr<FHOLCombatFragment>(Entity) : nullptr;
	}

	/** How many of Ids are in this combat mode. */
	int32 CountInMode(const FWorld& Test, const TArray<uint32>& Ids, const EHOLCombatMode Mode)
	{
		int32 Count = 0;
		for (const uint32 Id : Ids)
		{
			const FHOLCombatFragment* Combat = GetCombat(Test, Id);
			Count += (Combat && Combat->Mode == Mode) ? 1 : 0;
		}
		return Count;
	}

	/** How many of Ids were ordered to strike Building. */
	int32 CountStriking(const FWorld& Test, const TArray<uint32>& Ids, const AHOLBuilding* Building)
	{
		int32 Count = 0;
		for (const uint32 Id : Ids)
		{
			const FHOLCombatFragment* Combat = GetCombat(Test, Id);
			Count += (Combat && Combat->OrderedBuilding.Get() == Building) ? 1 : 0;
		}
		return Count;
	}

	int32 IndexOf(const TArray<FName>& Names, const TCHAR* Name) { return Names.IndexOfByKey(FName(Name)); }
}

// IA2.2: an AI slot's attack-move goes through the same queue as a human's (HOLPlayerOrders::AttackMove): it enters the
// queue, nothing changes in the frame, the next step puts slot 1's Spearmen walking in the attack-move mode; slot 0's
// Spearman slipped into the list is refused (D288); the cap and no slot are refused like a move's.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLAIOrdersAttackMoveTest, "HeroesOfLiberty.AI.Orders.AttackMove", UE::HeroesOfLiberty::Tests::AIOrders::Flags)
bool FHOLAIOrdersAttackMoveTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::AIOrders;
	FWorld Test;
	if (!TestTrue(TEXT("test world ready"), Test.IsReady()))
	{
		return false;
	}
	const UWorld& World = *Test.World;
	const TArray<uint32> Mine = PlaceUnits(*Test.Units, Spearman, 1, FVector(0.0, 800.0, 0.0), 2);
	const TArray<uint32> Other = PlaceUnits(*Test.Units, Spearman, 0, FVector(0.0, -800.0, 0.0), 1);
	const TArray<uint32> Crowd = PlaceUnits(*Test.Units, Peasant, 1, FVector(4000.0, 0.0, 0.0), HOLPlayerOrders::MaxOrderedUnits + 1);
	TestTrue(TEXT("two Spearmen of slot 1, one of slot 0, 201 Peasants of slot 1"),
		Mine.Num() == 2 && Other.Num() == 1 && Crowd.Num() == HOLPlayerOrders::MaxOrderedUnits + 1);

	TArray<uint32> Named = Mine;
	Named.Append(Other);
	TestTrue(TEXT("the attack-move enters the queue"), UHOLAIOrderLibrary::IssueAIAttackMoveOrder(Test.World, 1, ToScript(Named), FVector(-1500.0, 800.0, 0.0)));
	TestEqual(TEXT("not applied in the frame: slot 1's Spearmen idle"), CountDoing(World, Mine, EHOLUnitActivity::Idle), 2);
	TestEqual(TEXT("not applied in the frame: still in the normal mode"), CountInMode(Test, Mine, EHOLCombatMode::Normal), 2);
	TestFalse(TEXT("201 units: refused"), UHOLAIOrderLibrary::IssueAIAttackMoveOrder(Test.World, 1, ToScript(Crowd), FVector(0.0, 3000.0, 0.0)));
	TestFalse(TEXT("no slot: refused"), UHOLAIOrderLibrary::IssueAIAttackMoveOrder(Test.World, INDEX_NONE, ToScript(Mine), FVector(0.0, 3000.0, 0.0)));

	Test.RunStep();
	TestEqual(TEXT("next step: slot 1's Spearmen walking"), CountDoing(World, Mine, EHOLUnitActivity::Moving), 2);
	TestEqual(TEXT("next step: slot 1's Spearmen in the attack-move mode"), CountInMode(Test, Mine, EHOLCombatMode::AttackMove), 2);
	TestEqual(TEXT("slot 0's Spearman refused, idle"), CountDoing(World, Other, EHOLUnitActivity::Idle), 1);
	TestEqual(TEXT("slot 0's Spearman refused, normal mode"), CountInMode(Test, Other, EHOLCombatMode::Normal), 1);
	TestEqual(TEXT("the refused crowd did not move"), CountDoing(World, Crowd, EHOLUnitActivity::Idle), Crowd.Num());
	return true;
}

// IA2.2: an AI slot's order to attack a building, named by its ground point (any owner's building within 100 cm): the
// enemy's House is struck by slot 1's Spearmen after the step; slot 0's own Spearman slipped in is refused; a point with no
// building is refused; so is no slot.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLAIOrdersAttackBuildingTest, "HeroesOfLiberty.AI.Orders.AttackBuilding", UE::HeroesOfLiberty::Tests::AIOrders::Flags)
bool FHOLAIOrdersAttackBuildingTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::AIOrders;
	FWorld Test;
	if (!TestTrue(TEXT("test world ready"), Test.IsReady()))
	{
		return false;
	}
	const UWorld& World = *Test.World;
	AHOLBuilding* EnemyHouse = Test.World->SpawnActor<AHOLBuilding>(FVector(3000.0, 0.0, 0.0), FRotator::ZeroRotator);
	if (!TestNotNull(TEXT("an enemy House"), EnemyHouse))
	{
		return false;
	}
	EnemyHouse->Setup(HOLBuildingTypeNames::House, 0, 0);
	const TArray<uint32> Mine = PlaceUnits(*Test.Units, Spearman, 1, FVector(0.0, 800.0, 0.0), 2);
	const TArray<uint32> Other = PlaceUnits(*Test.Units, Spearman, 0, FVector(0.0, -800.0, 0.0), 1);
	TestTrue(TEXT("two Spearmen of slot 1, one of slot 0"), Mine.Num() == 2 && Other.Num() == 1);

	TArray<uint32> Named = Mine;
	Named.Append(Other);
	const FVector Near(3040.0, 30.0, 0.0);
	TestFalse(TEXT("no building at that point: refused"), UHOLAIOrderLibrary::IssueAIAttackBuildingOrder(Test.World, 1, ToScript(Mine), FVector(-5000.0, 0.0, 0.0)));
	TestFalse(TEXT("a point 300 cm off the House: refused"), UHOLAIOrderLibrary::IssueAIAttackBuildingOrder(Test.World, 1, ToScript(Mine), FVector(3300.0, 0.0, 0.0)));
	TestFalse(TEXT("no slot: refused"), UHOLAIOrderLibrary::IssueAIAttackBuildingOrder(Test.World, INDEX_NONE, ToScript(Mine), Near));
	TestTrue(TEXT("the order on the House enters the queue"), UHOLAIOrderLibrary::IssueAIAttackBuildingOrder(Test.World, 1, ToScript(Named), Near));
	TestEqual(TEXT("not applied in the frame: nobody ordered on the House"), CountStriking(Test, Mine, EnemyHouse), 0);

	Test.RunStep();
	TestEqual(TEXT("next step: slot 1's two Spearmen ordered to strike the House"), CountStriking(Test, Mine, EnemyHouse), 2);
	// The approach itself belongs to the combat processor (a played game, IA2.7): the test world only applies the order.
	TestEqual(TEXT("slot 0's Spearman (its own House) refused"), CountStriking(Test, Other, EnemyHouse), 0);
	TestEqual(TEXT("slot 0's Spearman idle"), CountDoing(World, Other, EHOLUnitActivity::Idle), 1);
	return true;
}

// IA4 (D572): an AI slot's order to attack one unit (a Peasant finishing a weakened soldier) goes through the same queue as a human's:
// after the step slot 1's Peasants have the enemy Spearman as their ordered target; a Spearman of their own camp is refused by the
// processor; a crowd over the cap and no slot are refused.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLAIOrdersAttackUnitTest, "HeroesOfLiberty.AI.Orders.AttackUnit", UE::HeroesOfLiberty::Tests::AIOrders::Flags)
bool FHOLAIOrdersAttackUnitTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::AIOrders;
	FWorld Test;
	if (!TestTrue(TEXT("test world ready"), Test.IsReady()))
	{
		return false;
	}
	const TArray<uint32> Mine = PlaceUnits(*Test.Units, Peasant, 1, FVector(0.0, 800.0, 0.0), 2);
	const TArray<uint32> Foe = PlaceUnits(*Test.Units, Spearman, 0, FVector(0.0, -800.0, 0.0), 1);
	const TArray<uint32> Crowd = PlaceUnits(*Test.Units, Peasant, 1, FVector(4000.0, 0.0, 0.0), HOLPlayerOrders::MaxOrderedUnits + 1);
	if (!TestTrue(TEXT("units placed"), Mine.Num() == 2 && Foe.Num() == 1 && Crowd.Num() == HOLPlayerOrders::MaxOrderedUnits + 1))
	{
		return false;
	}
	auto CountOrdered = [&Test](const TArray<uint32>& Ids)
	{
		int32 Count = 0;
		for (const uint32 Id : Ids)
		{
			const FHOLCombatFragment* Combat = GetCombat(Test, Id);
			Count += (Combat && Combat->bOrderedTarget && Combat->Target.IsSet()) ? 1 : 0;
		}
		return Count;
	};
	TestFalse(TEXT("no slot: refused"), UHOLAIOrderLibrary::IssueAIAttackUnitOrder(Test.World, INDEX_NONE, ToScript(Mine), static_cast<int32>(Foe[0])));
	TestFalse(TEXT("201 units: refused"), UHOLAIOrderLibrary::IssueAIAttackUnitOrder(Test.World, 1, ToScript(Crowd), static_cast<int32>(Foe[0])));
	TestTrue(TEXT("the order on the Spearman enters the queue"), UHOLAIOrderLibrary::IssueAIAttackUnitOrder(Test.World, 1, ToScript(Mine), static_cast<int32>(Foe[0])));
	TestEqual(TEXT("not applied in the frame"), CountOrdered(Mine), 0);
	Test.RunStep();
	TestEqual(TEXT("next step: both Peasants have the enemy Spearman as ordered target"), CountOrdered(Mine), 2);

	return true;
}

// IA2.2: the combat figures of a unit type, read from the same row the combat code reads (FHOLUnitTypeRow / HOLCombat::GetDamage):
// the numbers of the table, the bonuses as the flat damage a hit adds against a category.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLAIOrdersCombatFiguresTest, "HeroesOfLiberty.AI.Orders.CombatFigures", UE::HeroesOfLiberty::Tests::AIOrders::Flags)
bool FHOLAIOrdersCombatFiguresTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::AIOrders;
	FWorld Test;
	if (!TestTrue(TEXT("test world ready"), Test.IsReady()))
	{
		return false;
	}
	FHOLAITypeFigures Spear, Heavy, Bow, Bolt, Worker, Nothing;
	if (!TestTrue(TEXT("the Spearman is known"), UHOLAIOrderLibrary::ReadAIUnitFigures(Test.World, Spearman, Spear))
		|| !TestTrue(TEXT("the Man-at-arms is known"), UHOLAIOrderLibrary::ReadAIUnitFigures(Test.World, FName(TEXT("ManAtArms")), Heavy))
		|| !TestTrue(TEXT("the Archer is known"), UHOLAIOrderLibrary::ReadAIUnitFigures(Test.World, FName(TEXT("Archer")), Bow))
		|| !TestTrue(TEXT("the Crossbowman is known"), UHOLAIOrderLibrary::ReadAIUnitFigures(Test.World, FName(TEXT("Crossbowman")), Bolt))
		|| !TestTrue(TEXT("the Peasant is known"), UHOLAIOrderLibrary::ReadAIUnitFigures(Test.World, Peasant, Worker)))
	{
		return false;
	}
	TestFalse(TEXT("unknown unit"), UHOLAIOrderLibrary::ReadAIUnitFigures(Test.World, FName(TEXT("NoSuchUnit")), Nothing));

	TestTrue(TEXT("Spearman: health > 0, damage > 0, a short reach"), Spear.Health > 0 && Spear.Damage > 0 && Spear.AttackRange > 0.f && Spear.AttackRange < 500.f);
	TestTrue(TEXT("Spearman: a positive interval"), Spear.AttackInterval > 0.f);
	TestFalse(TEXT("Spearman: melee"), Spear.bRanged);
	TestTrue(TEXT("Spearman: LightInfantry"), IndexOf(Spear.ArmorCategories, TEXT("LightInfantry")) != INDEX_NONE);
	const int32 VsCavalry = IndexOf(Spear.BonusCategories, TEXT("Cavalry"));
	TestTrue(TEXT("Spearman: a bonus against Cavalry"), VsCavalry != INDEX_NONE && Spear.BonusDamage.IsValidIndex(VsCavalry) && Spear.BonusDamage[VsCavalry] > 0.f);
	TestEqual(TEXT("bonus arrays are parallel"), Spear.BonusCategories.Num(), Spear.BonusDamage.Num());

	TestTrue(TEXT("Man-at-arms: Heavy category"), IndexOf(Heavy.ArmorCategories, TEXT("Heavy")) != INDEX_NONE);
	TestTrue(TEXT("Man-at-arms: more health than the Spearman"), Heavy.Health > Spear.Health);
	TestTrue(TEXT("Man-at-arms: melee armour"), Heavy.MeleeArmor > Spear.MeleeArmor);

	TestTrue(TEXT("Archer: ranged"), Bow.bRanged);
	TestTrue(TEXT("Archer: a longer reach than the Spearman"), Bow.AttackRange > Spear.AttackRange);
	TestTrue(TEXT("Archer: not Heavy"), IndexOf(Bow.ArmorCategories, TEXT("Heavy")) == INDEX_NONE);

	TestTrue(TEXT("Crossbowman: ranged"), Bolt.bRanged);
	const int32 VsHeavy = IndexOf(Bolt.BonusCategories, TEXT("Heavy"));
	TestTrue(TEXT("Crossbowman: a bonus against Heavy"), VsHeavy != INDEX_NONE && Bolt.BonusDamage.IsValidIndex(VsHeavy) && Bolt.BonusDamage[VsHeavy] > 0.f);

	TestEqual(TEXT("Peasant: the cost as before"), Worker.CostFood, 50);
	TestTrue(TEXT("Peasant: the gathering as before"), Worker.CarryCapacity > 0 && Worker.GatherFood > 0.f);
	TestEqual(TEXT("Peasant: no bonus"), Worker.BonusCategories.Num(), 0);
	TestTrue(TEXT("Peasant: the Peasant category"), IndexOf(Worker.ArmorCategories, TEXT("Peasant")) != INDEX_NONE);

	FHOLAITypeFigures House;
	TestTrue(TEXT("a building: its figures"), UHOLAIOrderLibrary::ReadAIBuildingFigures(HOLBuildingTypeNames::House, House));
	TestTrue(TEXT("a building: no combat figures"), House.Health == 0 && House.Damage == 0 && House.ArmorCategories.IsEmpty() && House.BonusCategories.IsEmpty());
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
