// Copyright Epic Games, Inc. All Rights Reserved.

#include "Buildings/HOLBuilding.h"
#include "Buildings/HOLBuildingTypes.h"
#include "Buildings/HOLConstructionSubsystem.h"
#include "Buildings/HOLProductionSubsystem.h"
#include "Commands/HOLCommandSubsystem.h"
#include "Commands/HOLProductionCommand.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/GameStateBase.h"
#include "HeroesOfLibertyGameModeBase.h"
#include "Match/HOLGameState.h"
#include "Match/HOLMatchConfig.h"
#include "Match/HOLMatchSettings.h"
#include "Match/HOLPlayerState.h"
#include "Misc/AutomationTest.h"
#include "Resources/HOLSourceSubsystem.h"
#include "Simulation/HOLSimulationSubsystem.h"
#include "Simulation/HOLUnitSubsystem.h"
#include "Simulation/HOLUnitTierRules.h"
#include "Simulation/HOLUnitTypes.h"

#if WITH_DEV_AUTOMATION_TESTS

// G1 (Palier3InProgress.md, D711; GDD 11.1): a unit opens at the tier of its column MinTier, by ONE rule (HOLUnitTier), for every unit. On the
// real chain: the production order of a slot goes through the command queue and the production subsystem (server), the stock and the
// buildings' queues are what is measured. Two slots of the Dawn each own a Barracks, an Archery range, a Stable, a Chapel and a Siege
// workshop; slot 0 is at tier 0, slot 1 at tier 1 (forced, hol.Cheat.Tier's own state), and the same orders are sent for both.
namespace UE::HeroesOfLiberty::Tests::UnitTier
{
	constexpr EAutomationTestFlags Flags = EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter;

	const FName Spearman(TEXT("Spearman"));
	const FName ManAtArms(TEXT("ManAtArms"));
	const FName Archer(TEXT("Archer"));
	const FName Crossbowman(TEXT("Crossbowman"));
	const FName LightCavalry(TEXT("LightCavalry"));
	const FName Monk(TEXT("Monk"));
	const FName Ram(TEXT("Ram"));
	const FName Mangonel(TEXT("Mangonel"));

	FHOLMatchConfig MakeConfig()
	{
		FHOLMatchConfig Config = FHOLMatchConfig::MakeDefault();
		for (int32 Index = 0; Index < Config.Slots.Num(); ++Index)
		{
			Config.Slots[Index].State = Index <= 1 ? EHOLSlotState::AI : EHOLSlotState::Closed;
			Config.Slots[Index].ColorIndex = Index;
			Config.Slots[Index].Faction = EHOLFaction::DawnOrder;
		}
		return Config;
	}

	int32 Total(const FHOLResources& Stock) { return Stock.Food + Stock.Wood + Stock.Stone + Stock.Gold; }

	struct FWorld
	{
		UWorld* World = nullptr;
		AHOLGameState* State = nullptr;
		UHOLUnitSubsystem* Units = nullptr;
		UHOLCommandSubsystem* Commands = nullptr;
		AHOLPlayerState* Payer[2] = { nullptr, nullptr };
		AHOLBuilding* Barracks[2] = { nullptr, nullptr };
		AHOLBuilding* Range[2] = { nullptr, nullptr };
		AHOLBuilding* Stable[2] = { nullptr, nullptr };
		AHOLBuilding* Chapel[2] = { nullptr, nullptr };
		AHOLBuilding* Workshop[2] = { nullptr, nullptr };
		bool bReady = false;

		FWorld()
		{
			World = UWorld::CreateWorld(EWorldType::Game, /*bInformEngineOfWorld*/ false);
			FWorldContext& Context = GEngine->CreateNewWorldContext(EWorldType::Game);
			Context.SetCurrentWorld(World);
			World->InitializeActorsForPlay(FURL());
			State = World->SpawnActor<AHOLGameState>();
			Units = World->GetSubsystem<UHOLUnitSubsystem>();
			Commands = World->GetSubsystem<UHOLCommandSubsystem>();
			UHOLProductionSubsystem* Production = World->GetSubsystem<UHOLProductionSubsystem>();
			UHOLConstructionSubsystem* Construction = World->GetSubsystem<UHOLConstructionSubsystem>();
			UHOLSourceSubsystem* Sources = World->GetSubsystem<UHOLSourceSubsystem>();
			if (!State || !Units || !Commands || !Production || !Construction || !Sources || !World->GetSubsystem<UHOLSimulationSubsystem>())
			{
				return;
			}
			const FHOLMatchConfig Config = MakeConfig();
			State->InitVictory(Config);
			AHeroesOfLibertyGameModeBase::SpawnAIPlayerStates(*World, Config, GetDefault<UHOLMatchSettings>()->StartingResources);
			Sources->OnWorldBeginPlay(*World);
			Construction->OnWorldBeginPlay(*World);
			Production->OnWorldBeginPlay(*World);
			bReady = true;
			for (int32 Slot = 0; Slot < 2; ++Slot)
			{
				Payer[Slot] = FindPlayerState(Slot);
				bReady = bReady && Payer[Slot];
				const auto Put = [&](AHOLBuilding*& Out, const FName Type, const double Y)
				{
					Out = World->SpawnActor<AHOLBuilding>(FVector(Slot * -20000.0 + 4000.0, Y, 0.0), FRotator::ZeroRotator);
					bReady = bReady && Out;
					if (Out)
					{
						Out->Setup(Type, static_cast<uint8>(Slot), static_cast<uint8>(Slot));
					}
				};
				Put(Barracks[Slot], TEXT("Barracks"), 0.0);
				Put(Range[Slot], TEXT("ArcheryRange"), 3000.0);
				Put(Stable[Slot], TEXT("Stable"), 6000.0);
				Put(Chapel[Slot], TEXT("Chapel"), 9000.0);
				Put(Workshop[Slot], TEXT("SiegeWorkshop"), 12000.0);
			}
		}

		~FWorld()
		{
			GEngine->DestroyWorldContext(World);
			World->DestroyWorld(false);
		}

		AHOLPlayerState* FindPlayerState(const int32 Slot) const
		{
			for (APlayerState* Each : State->PlayerArray)
			{
				AHOLPlayerState* HOLEach = Cast<AHOLPlayerState>(Each);
				if (HOLEach && HOLEach->GetPlayerIndex() == Slot)
				{
					return HOLEach;
				}
			}
			return nullptr;
		}

		void RunStep(const int64 StepIndex = 1) { World->GetSubsystem<UHOLSimulationSubsystem>()->GetOnStepStarted().Broadcast(StepIndex); }

		void Order(const int32 Slot, AHOLBuilding* Building, const FName Type)
		{
			FHOLProductionCommand Command;
			Command.PlayerIndex = Slot;
			Command.Building = Building;
			Command.TypeName = Type;
			Commands->IssueProductionOrder(Command);
		}

		/** Every unit of the roster ordered at its own building, for the slot. */
		void OrderAll(const int32 Slot)
		{
			Order(Slot, Barracks[Slot], Spearman);
			Order(Slot, Barracks[Slot], ManAtArms);
			Order(Slot, Range[Slot], Archer);
			Order(Slot, Range[Slot], Crossbowman);
			Order(Slot, Stable[Slot], LightCavalry);
			Order(Slot, Chapel[Slot], Monk);
			Order(Slot, Workshop[Slot], Ram);
			Order(Slot, Workshop[Slot], Mangonel);
		}

		/** What the slot's buildings hold in queue, as "B/R/S/C/W" counts. */
		FString Queues(const int32 Slot) const
		{
			return FString::Printf(TEXT("%d/%d/%d/%d/%d"), Barracks[Slot]->GetProductionQueue().Num(), Range[Slot]->GetProductionQueue().Num(),
				Stable[Slot]->GetProductionQueue().Num(), Chapel[Slot]->GetProductionQueue().Num(), Workshop[Slot]->GetProductionQueue().Num());
		}

		int32 Cost(const FName Type) const
		{
			const FHOLUnitTypeRow* Row = Units->FindTypeRow(Type);
			return Row ? Total(Row->Cost) : -1;
		}
	};
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLUnitTierProductionTest, "HeroesOfLiberty.Production.UnitTier", UE::HeroesOfLiberty::Tests::UnitTier::Flags)
bool FHOLUnitTierProductionTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::UnitTier;
	FWorld Test;
	if (!TestTrue(TEXT("the test world stands"), Test.bReady))
	{
		return false;
	}
	const FHOLResources Rich(5000, 5000, 5000, 5000);
	Test.Payer[0]->SetResources(Rich);
	Test.Payer[1]->SetResources(Rich);
	Test.State->SetTierOverride(0, 0);
	Test.State->SetTierOverride(1, 1);
	TestEqual(TEXT("slot 0 is at tier 0"), Test.State->GetTier(0), 0);
	TestEqual(TEXT("slot 1 is at tier 1"), Test.State->GetTier(1), 1);

	// 1. Tier 0, the same orders for both: the server refuses the Man-at-arms, the Crossbowman, the Light cavalry, the Monk, the Ram and the
	// Mangonel of slot 0 (even a client that asked for them), and queues what tier 0 opens (Spearman, Archer). Slot 1 (tier 1) queues all but
	// the Mangonel (tier 2). The stock moves by exactly the price of what was queued.
	Test.OrderAll(0);
	Test.OrderAll(1);
	Test.RunStep(1);
	TestEqual(TEXT("tier 0: Barracks / range / stable / chapel / workshop queues: the Spearman and the Archer only"), Test.Queues(0), FString(TEXT("1/1/0/0/0")));
	TestEqual(TEXT("tier 1: the Man-at-arms, the Crossbowman, the Light cavalry, the Monk and the Ram come out too (not the Mangonel)"),
		Test.Queues(1), FString(TEXT("2/2/1/1/1")));
	TestEqual(TEXT("tier 0 paid the Spearman and the Archer only"), 20000 - Total(Test.Payer[0]->GetResources()), Test.Cost(Spearman) + Test.Cost(Archer));
	TestEqual(TEXT("tier 1 paid all its queued units"), 20000 - Total(Test.Payer[1]->GetResources()),
		Test.Cost(Spearman) + Test.Cost(ManAtArms) + Test.Cost(Archer) + Test.Cost(Crossbowman) + Test.Cost(LightCavalry) + Test.Cost(Monk) + Test.Cost(Ram));

	// 2. The hero reaches tier 1 (the forced tier lifted to 1): the same orders now pass for slot 0, the Mangonel still waits for tier 2.
	Test.State->SetTierOverride(0, 1);
	const int32 Before = Total(Test.Payer[0]->GetResources());
	Test.OrderAll(0);
	Test.RunStep(2);
	TestEqual(TEXT("tier 1 for slot 0: what tier 1 opens is queued on top of the first lot; the workshop holds the Ram only (the Mangonel waits for tier 2)"),
		Test.Queues(0), FString(TEXT("3/3/1/1/1")));
	TestEqual(TEXT("slot 0 paid the Man-at-arms, the Crossbowman, the Light cavalry, the Monk, the Ram and one more Spearman and Archer"),
		Before - Total(Test.Payer[0]->GetResources()),
		Test.Cost(Spearman) + Test.Cost(ManAtArms) + Test.Cost(Archer) + Test.Cost(Crossbowman) + Test.Cost(LightCavalry) + Test.Cost(Monk) + Test.Cost(Ram));

	// 3. Tier 2: the Mangonel is queued (and the Ram, Mangonel keep their table: the engines behave as before).
	Test.State->SetTierOverride(0, 2);
	Test.Order(0, Test.Workshop[0], Mangonel);
	Test.RunStep(3);
	TestEqual(TEXT("tier 2: the Mangonel is queued at the workshop"), Test.Workshop[0]->GetProductionQueue().Num(), 2);
	return true;
}

#endif
