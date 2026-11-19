// Copyright Epic Games, Inc. All Rights Reserved.

#include "AI/HOLAIBrain.h"
#include "AI/HOLAIOrders.h"
#include "AI/HOLAITechnologyLibrary.h"
#include "AI/HOLAITools.h"
#include "Buildings/HOLBuilding.h"
#include "Buildings/HOLBuildingTypes.h"
#include "Buildings/HOLConstructionRules.h"
#include "Buildings/HOLConstructionSubsystem.h"
#include "Buildings/HOLProductionRules.h"
#include "Buildings/HOLProductionSubsystem.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "GameFramework/GameStateBase.h"
#include "Match/HOLDisconnectRules.h"
#include "Match/HOLGameState.h"
#include "Match/HOLMatchConfig.h"
#include "Match/HOLPlayerState.h"
#include "Misc/AutomationTest.h"
#include "Simulation/HOLGatherProcessor.h"
#include "Simulation/HOLGatherRules.h"
#include "Simulation/HOLHeroSubsystem.h"
#include "Simulation/HOLSimulationSubsystem.h"
#include "HAL/IConsoleManager.h"
#include "Misc/OutputDeviceRedirector.h"
#include "Technologies/HOLTechnologyRules.h"
#include "Technologies/HOLTechnologyTypes.h"

#if WITH_DEV_AUTOMATION_TESTS

// R7 (R-Q5, D71): a named player gone during the match keeps his slot for a window, then loses as by a surrender.
namespace UE::HeroesOfLiberty::Tests::Disconnect
{
	constexpr EAutomationTestFlags Flags = EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter;

	/** Alice (slot 0) and Bob (slot 1), humans of two camps; an AI in slot 2 with Bob; the rest closed. */
	FHOLMatchConfig MakeNamedConfig()
	{
		FHOLMatchConfig Config = FHOLMatchConfig::MakeDefault();
		for (FHOLMatchSlot& Slot : Config.Slots)
		{
			Slot.State = EHOLSlotState::Closed;
			Slot.Team = 0;
		}
		Config.Slots[0].State = EHOLSlotState::Human;
		Config.Slots[0].PlayerName = TEXT("Alice");
		Config.Slots[0].Team = 1;
		Config.Slots[1].State = EHOLSlotState::Human;
		Config.Slots[1].PlayerName = TEXT("Bob");
		Config.Slots[1].Team = 2;
		Config.Slots[2].State = EHOLSlotState::AI;
		Config.Slots[2].Team = 2;
		return Config;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLDisconnectRulesTest, "HeroesOfLiberty.Match.DisconnectRules",
	UE::HeroesOfLiberty::Tests::Disconnect::Flags)
bool FHOLDisconnectRulesTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::Disconnect;
	const FHOLMatchConfig Named = MakeNamedConfig();

	TestTrue(TEXT("Bob leaves during the match: his slot kept"), FHOLDisconnectRules::KeepsSlot(Named, 1, false, false, false));
	TestFalse(TEXT("still waiting for the players: not kept"), FHOLDisconnectRules::KeepsSlot(Named, 1, true, false, false));
	TestFalse(TEXT("the match over: not kept"), FHOLDisconnectRules::KeepsSlot(Named, 1, false, true, false));
	TestFalse(TEXT("his slot defeated: not kept"), FHOLDisconnectRules::KeepsSlot(Named, 1, false, false, true));
	TestFalse(TEXT("an AI slot: not kept"), FHOLDisconnectRules::KeepsSlot(Named, 2, false, false, false));
	TestFalse(TEXT("a closed slot: not kept"), FHOLDisconnectRules::KeepsSlot(Named, 4, false, false, false));
	TestFalse(TEXT("no slot (spectator): not kept"), FHOLDisconnectRules::KeepsSlot(Named, INDEX_NONE, false, false, false));
	TestFalse(TEXT("a configuration without names (solo, direct): not kept"),
		FHOLDisconnectRules::KeepsSlot(FHOLMatchConfig::MakeDefault(), 0, false, false, false));

	TestEqual(TEXT("the window by default: 3 min"), FHOLDisconnectRules::DefaultWindowSeconds, 180.);
	TestFalse(TEXT("left at 10, now 189.9: still in the window"), FHOLDisconnectRules::IsWindowOver(10., 189.9, 180.));
	TestTrue(TEXT("left at 10, now 190: over"), FHOLDisconnectRules::IsWindowOver(10., 190., 180.));
	TestEqual(TEXT("seconds left"), FHOLDisconnectRules::GetSecondsLeft(10., 31., 180.), 159.);
	TestEqual(TEXT("no seconds left after the end"), FHOLDisconnectRules::GetSecondsLeft(10., 500., 180.), 0.);

	TArray<FHOLAbsentSlot> Absent;
	Absent.Add({ 3, TEXT("Dan"), 0., FHOLResources() });
	Absent.Add({ 1, TEXT("Bob"), 50., FHOLResources(0, 300, 0, 0) });
	TestEqual(TEXT("Bob's slot found"), FHOLDisconnectRules::FindAbsent(Absent, 1), 1);
	TestEqual(TEXT("Alice not absent"), FHOLDisconnectRules::FindAbsent(Absent, 0), INDEX_NONE);
	TestEqual(TEXT("at 200: Dan's window over, not Bob's"), FHOLDisconnectRules::GetExpired(Absent, 200., 180.), TArray<int32>{ 3 });
	TestEqual(TEXT("at 230: both, in the order of the departures"), FHOLDisconnectRules::GetExpired(Absent, 230., 180.),
		(TArray<int32>{ 3, 1 }));

	const FHOLResources Starting(200, 200, 200, 200);
	TestTrue(TEXT("back: the stock kept"), FHOLDisconnectRules::GetStockOnTaking(&Absent[1], Starting) == FHOLResources(0, 300, 0, 0));
	TestTrue(TEXT("first login: the stock of the start"), FHOLDisconnectRules::GetStockOnTaking(nullptr, Starting) == Starting);
	return true;
}

// M6-Q16 (R7): what Bob gains while away is kept for him. Bob leaves (his player state destroyed, as after a Logout); a
// Peasant of his unloads 10 wood (the gathering's deposit), the enemy destroys his house's foundation (its cost back) and
// his centre with a Peasant queued (the queue back); back in the window, he finds his stock plus all of that.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLDisconnectGainsTest, "HeroesOfLiberty.Match.DisconnectGains",
	UE::HeroesOfLiberty::Tests::Disconnect::Flags)
bool FHOLDisconnectGainsTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::Disconnect;

	UWorld* World = UWorld::CreateWorld(EWorldType::Game, /*bInformEngineOfWorld*/ false);
	FWorldContext& Context = GEngine->CreateNewWorldContext(EWorldType::Game);
	Context.SetCurrentWorld(World);
	World->InitializeActorsForPlay(FURL());

	AHOLGameState* State = World->SpawnActor<AHOLGameState>();
	UHOLProductionSubsystem* Production = World->GetSubsystem<UHOLProductionSubsystem>();
	AHOLPlayerState* Bob = World->SpawnActor<AHOLPlayerState>();
	AHOLPlayerState* BobAgain = World->SpawnActor<AHOLPlayerState>();
	AHOLBuilding* Foundation = World->SpawnActor<AHOLBuilding>(FVector(3000.0, 0.0, 0.0), FRotator::ZeroRotator);
	AHOLBuilding* Centre = World->SpawnActor<AHOLBuilding>(FVector(0.0, 0.0, 0.0), FRotator::ZeroRotator);
	if (TestNotNull(TEXT("game state"), State) && TestNotNull(TEXT("production"), Production) && TestNotNull(TEXT("player states"), Bob)
		&& TestNotNull(TEXT("player states"), BobAgain) && TestNotNull(TEXT("buildings"), Foundation) && TestNotNull(TEXT("buildings"), Centre))
	{
		State->InitVictory(MakeNamedConfig());
		const FHOLResources Starting(500, 300, 100, 100);
		Bob->SetMatchSlot(1, 1);
		State->GiveSlotStock(*Bob, Starting);
		Foundation->Setup(HOLBuildingTypeNames::House, 1, 1, EHOLConstructionState::Foundation);
		Centre->Setup(HOLBuildingTypeNames::MainCentre, 1, 1);

		// Bob queues a Peasant (paid at once, as the production's command does).
		const FName Peasant(TEXT("Peasant"));
		const FHOLProductionCost PeasantCost = Production->GetCost(Peasant);
		FHOLResources Stock = Bob->GetResources();
		TestTrue(TEXT("a Peasant queued"), HOLProduction::TryEnqueue(Centre->GetMutableProductionQueue(), Peasant, PeasantCost, 5, Stock)
			== EHOLProductionRefusal::None);
		Bob->SetResources(Stock);
		const FHOLResources AtDeparture = Bob->GetResources();
		const FHOLBuildingTypeRow* HouseType = Foundation->GetType();
		const FHOLResources FoundationRefund = HouseType ? HOLConstruction::GetCancelRefund(HouseType->Cost, Foundation->GetConstructionState(),
			Foundation->GetConstructionWork(), Foundation->GetTotalWork()) : FHOLResources();
		TestTrue(TEXT("a house's foundation gives something back"), FoundationRefund.Wood > 0);
		TestTrue(TEXT("a Peasant costs something"), PeasantCost.Resources.Food > 0);

		TestTrue(TEXT("Bob leaves: slot kept"), State->KeepAbsentSlot(*Bob, 10.));
		Bob->Destroy();
		bool bBobListed = false;
		for (const APlayerState* Each : State->PlayerArray)
		{
			bBobListed |= Each == Bob;
		}
		TestFalse(TEXT("his player state gone, as after a Logout"), bBobListed);

		// A Peasant of his unloads 10 wood at his centre (the gathering's deposit).
		FHOLGatherLoad Load;
		Load.Resource = EHOLResource::Wood;
		Load.Amount = 10;
		FHOLResources AfterDeposit;
		TestEqual(TEXT("his Peasant unloads 10 wood"), UHOLGatherProcessor::DepositLoad(*World, 1, Load, AfterDeposit), 10);
		TestEqual(TEXT("its load emptied"), Load.Amount, 0);

		// The enemy destroys his foundation and his centre (the combat's path): the cost of the foundation and the queue back.
		UHOLConstructionSubsystem::DestroyBuilding(*Foundation);
		UHOLConstructionSubsystem::DestroyBuilding(*Centre);

		BobAgain->SetMatchSlot(1, 1);
		State->GiveSlotStock(*BobAgain, Starting);
		FHOLResources Expected = AtDeparture;
		Expected.Wood += 10;
		Expected += FoundationRefund;
		Expected += PeasantCost.Resources;
		AddInfo(FString::Printf(TEXT("at departure %s, back with %s, expected %s"), *AtDeparture.ToString(), *BobAgain->GetResources().ToString(),
			*Expected.ToString()));
		TestTrue(TEXT("back: his stock plus the 10 wood, the foundation's cost and the queue's"), BobAgain->GetResources() == Expected);
	}

	GEngine->DestroyWorldContext(World);
	World->DestroyWorld(/*bInformEngineOfWorld*/ false);
	return true;
}

namespace UE::HeroesOfLiberty::Tests::Disconnect
{
	/** What the game logs while a test runs (the journal of the research: « Technology: slot N researched X »). */
	class FCaptureLog : public FOutputDevice
	{
	public:
		TArray<FString> Lines;
		virtual void Serialize(const TCHAR* Text, ELogVerbosity::Type, const FName&) override { Lines.Add(Text); }
		int32 CountContaining(const FString& A, const FString& B) const
		{
			int32 Count = 0;
			for (const FString& Line : Lines)
			{
				Count += Line.Contains(A) && Line.Contains(B) ? 1 : 0;
			}
			return Count;
		}
	};
}

// D719 (T0): what the absent gains is kept for him, the technologies as the stock. Bob has the Plough I, queues the Plough II at
// his farm and leaves; the research ends while he is away (the real production step); the AI's reader still sees his Plough I
// (what a technology that asks for it needs); back in the window he finds the Plough I and the Plough II, and their effect.
// Alice, who never leaves, is as before; a research ending for a slot that exists nowhere is not logged as done.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLDisconnectTechnologiesTest, "HeroesOfLiberty.Match.DisconnectTechnologies",
	UE::HeroesOfLiberty::Tests::Disconnect::Flags)
bool FHOLDisconnectTechnologiesTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::Disconnect;

	const FName PloughI(TEXT("PloughI"));
	const FName PloughII(TEXT("PloughII"));
	if (!TestNotNull(TEXT("the technology table is loaded (Plough II)"), UHOLTechnologySettings::FindTechnology(PloughII)))
	{
		return false;
	}
	IConsoleVariable* BuildTime = IConsoleManager::Get().FindConsoleVariable(TEXT("hol.Prod.BuildTime"));
	if (!TestNotNull(TEXT("hol.Prod.BuildTime"), BuildTime))
	{
		return false;
	}
	BuildTime->Set(0.1f, ECVF_SetByCode);

	UWorld* World = UWorld::CreateWorld(EWorldType::Game, /*bInformEngineOfWorld*/ false);
	FWorldContext& Context = GEngine->CreateNewWorldContext(EWorldType::Game);
	Context.SetCurrentWorld(World);
	World->InitializeActorsForPlay(FURL());

	AHOLGameState* State = World->SpawnActor<AHOLGameState>();
	UHOLProductionSubsystem* Production = World->GetSubsystem<UHOLProductionSubsystem>();
	UHOLSimulationSubsystem* Simulation = World->GetSubsystem<UHOLSimulationSubsystem>();
	AHOLPlayerState* Alice = World->SpawnActor<AHOLPlayerState>();
	AHOLPlayerState* Bob = World->SpawnActor<AHOLPlayerState>();
	AHOLBuilding* BobFarm = World->SpawnActor<AHOLBuilding>(FVector(0.0, 0.0, 0.0), FRotator::ZeroRotator);
	AHOLBuilding* AliceFarm = World->SpawnActor<AHOLBuilding>(FVector(5000.0, 0.0, 0.0), FRotator::ZeroRotator);
	AHOLBuilding* NobodyFarm = World->SpawnActor<AHOLBuilding>(FVector(-5000.0, 0.0, 0.0), FRotator::ZeroRotator);
	if (TestNotNull(TEXT("game state"), State) && TestNotNull(TEXT("production"), Production) && TestNotNull(TEXT("simulation"), Simulation)
		&& TestNotNull(TEXT("player states"), Alice) && TestNotNull(TEXT("player states"), Bob) && TestNotNull(TEXT("farms"), BobFarm)
		&& TestNotNull(TEXT("farms"), AliceFarm) && TestNotNull(TEXT("farms"), NobodyFarm))
	{
		State->InitVictory(MakeNamedConfig());
		Alice->SetMatchSlot(0, 0);
		Bob->SetMatchSlot(1, 1);
		State->AddPlayerState(Alice);
		State->AddPlayerState(Bob);
		const FHOLResources Starting(2000, 2000, 500, 500);
		State->GiveSlotStock(*Alice, Starting);
		State->GiveSlotStock(*Bob, Starting);
		BobFarm->Setup(HOLBuildingTypeNames::Farm, 1, 1);
		AliceFarm->Setup(HOLBuildingTypeNames::Farm, 0, 0);
		// Slot 3 is nobody's (closed, no player state, not absent): its farm researches into the void.
		NobodyFarm->Setup(HOLBuildingTypeNames::Farm, 3, 3);
		Production->OnWorldBeginPlay(*World);

		const auto Queue = [&](AHOLBuilding* Farm, AHOLPlayerState* Payer, const int32 Slot, const FName Technology)
		{
			const FHOLProductionCost Cost = Production->GetCost(Technology, Slot);
			FHOLResources Stock = Payer ? Payer->GetResources() : Starting;
			const bool bQueued = HOLProduction::TryEnqueue(Farm->GetMutableProductionQueue(), Technology, Cost, 5, Stock) == EHOLProductionRefusal::None;
			if (Payer)
			{
				Payer->SetResources(Stock);
			}
			return bQueued;
		};
		int64 StepCount = 0;
		const auto RunSteps = [&](const int32 Steps)
		{
			for (int32 Each = 0; Each < Steps; ++Each)
			{
				Simulation->GetOnStepStarted().Broadcast(++StepCount);
			}
		};

		// Bob has the Plough I and queues the Plough II; Alice and slot 3 queue the Plough I.
		TestTrue(TEXT("Bob has the Plough I"), Bob->AddTechnology(PloughI));
		TestTrue(TEXT("Bob queues the Plough II"), Queue(BobFarm, Bob, 1, PloughII));
		TestTrue(TEXT("Alice queues the Plough I"), Queue(AliceFarm, Alice, 0, PloughI));
		TestTrue(TEXT("slot 3 queues the Plough I"), Queue(NobodyFarm, nullptr, 3, PloughI));

		// Bob leaves; his player state goes, as after a Logout.
		TestTrue(TEXT("Bob leaves: slot kept"), State->KeepAbsentSlot(*Bob, 10., 180.));
		Bob->Destroy();

		// What a technology that asks for the Plough I reads (the AI's reader): his Plough I is still his.
		TArray<FName> Done;
		UHOLAITechnologyLibrary::ReadAITechnologiesDone(World, 1, Done);
		AddInfo(FString::Printf(TEXT("while he is away, his technologies read: %d"), Done.Num()));
		TestTrue(TEXT("away: his Plough I is still read as done (the Plough II stays offered)"), Done.Contains(PloughI));

		// The researches end while he is away (the real production step).
		FCaptureLog Log;
		GLog->AddOutputDevice(&Log);
		RunSteps(8);
		GLog->FlushThreadedLogs();
		GLog->RemoveOutputDevice(&Log);

		TestTrue(TEXT("Alice's research landed in her player state, as ever"), Alice->GetTechnologies().Has(PloughI));
		TestEqual(TEXT("Alice's research logged once"), Log.CountContaining(TEXT("researched PloughI"), TEXT("slot 0")), 1);
		UHOLAITechnologyLibrary::ReadAITechnologiesDone(World, 1, Done);
		TestTrue(TEXT("away: the research ended while he was gone is his"), Done.Contains(PloughII));
		TestEqual(TEXT("slot 3's research, going nowhere, is not logged as researched"), Log.CountContaining(TEXT("researched PloughI"), TEXT("slot 3")), 0);

		// Back in the window with a new player state: everything is his.
		AHOLPlayerState* BobAgain = World->SpawnActor<AHOLPlayerState>();
		if (TestNotNull(TEXT("new player state"), BobAgain))
		{
			TestFalse(TEXT("a new player state starts without technologies"), BobAgain->GetTechnologies().Has(PloughI));
			BobAgain->SetMatchSlot(1, 1);
			State->AddPlayerState(BobAgain);
			TestTrue(TEXT("back at 100"), State->GiveSlotStock(*BobAgain, Starting, 100.));
			AddInfo(FString::Printf(TEXT("back: %d technologies, food gathering +%d %%"), BobAgain->GetTechnologies().Done.Num(),
				BobAgain->GetTechnologyBonusPercent(HOLTechnology::GetGatherStat(EHOLResource::Food))));
			TestTrue(TEXT("back: his Plough I"), BobAgain->GetTechnologies().Has(PloughI));
			TestTrue(TEXT("back: his Plough II, ended while he was away"), BobAgain->GetTechnologies().Has(PloughII));
			TestTrue(TEXT("back: their effect (food gathering) counts at once"), BobAgain->GetTechnologyBonusPercent(HOLTechnology::GetGatherStat(EHOLResource::Food)) > 0);
			TestFalse(TEXT("back: Alice's technologies are not his"), BobAgain->GetTechnologies().Done.Num() > 2);
		}
		// A slot never absent keeps what it had (nothing gained, nothing lost).
		TestEqual(TEXT("Alice never left: her technologies are hers"), Alice->GetTechnologies().Done.Num(), 1);
	}

	BuildTime->Set(0.f, ECVF_SetByCode);
	GEngine->DestroyWorldContext(World);
	World->DestroyWorld(/*bInformEngineOfWorld*/ false);
	return true;
}

// D718 (T1): the end of the window by the camps of the launch (D357). Alone in his camp with only two camps in all (the 1v1,
// the 1 against 2 too): defeat; otherwise the AI keeps the seat (teams, free for all of 3 and more, a camp with an AI seat
// beside him). The Diplomacy mode has every slot alone in its camp (GetCamp). The seat states and the return.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLDisconnectWindowEndTest, "HeroesOfLiberty.Match.DisconnectWindowEnd",
	UE::HeroesOfLiberty::Tests::Disconnect::Flags)
bool FHOLDisconnectWindowEndTest::RunTest(const FString& Parameters)
{
	using EEnd = FHOLDisconnectRules::EWindowEnd;
	// Each entry: the team of a human slot (0 = no team, free for all); a negative one is an AI of the team -value.
	const auto Make = [](const TArray<int32>& Seats, const EHOLMatchMode Mode = EHOLMatchMode::Standard)
	{
		FHOLMatchConfig Config = FHOLMatchConfig::MakeDefault();
		for (FHOLMatchSlot& Slot : Config.Slots)
		{
			Slot.State = EHOLSlotState::Closed;
			Slot.Team = 0;
		}
		for (int32 Index = 0; Index < Seats.Num(); ++Index)
		{
			Config.Slots[Index].State = Seats[Index] < 0 ? EHOLSlotState::AI : EHOLSlotState::Human;
			Config.Slots[Index].Team = FMath::Abs(Seats[Index]);
			Config.Slots[Index].PlayerName = Seats[Index] < 0 ? FString() : FString::Printf(TEXT("P%d"), Index);
		}
		Config.Mode = Mode;
		return Config;
	};
	const auto End = [&Make](const TArray<int32>& Seats, const int32 Slot, const EHOLMatchMode Mode = EHOLMatchMode::Standard)
	{
		return FHOLDisconnectRules::GetWindowEnd(Make(Seats, Mode), Slot);
	};

	TestTrue(TEXT("1v1: defeat"), End({ 1, 2 }, 0) == EEnd::Defeat && End({ 1, 2 }, 1) == EEnd::Defeat);
	TestTrue(TEXT("1v1 free for all (no teams): defeat"), End({ 0, 0 }, 0) == EEnd::Defeat);
	TestTrue(TEXT("2v2: the AI keeps the seat"), End({ 1, 1, 2, 2 }, 0) == EEnd::AIHolds && End({ 1, 1, 2, 2 }, 3) == EEnd::AIHolds);
	TestTrue(TEXT("free for all of 3: the AI keeps the seat"), End({ 0, 0, 0 }, 1) == EEnd::AIHolds);
	TestTrue(TEXT("free for all of 8: the AI keeps the seat"), End({ 0, 0, 0, 0, 0, 0, 0, 0 }, 7) == EEnd::AIHolds);
	TestTrue(TEXT("1 against 2: the one alone loses"), End({ 1, 2, 2 }, 0) == EEnd::Defeat);
	TestTrue(TEXT("1 against 2: one of the two: the AI keeps the seat"), End({ 1, 2, 2 }, 1) == EEnd::AIHolds);
	TestTrue(TEXT("1 against 3 and 1 against 1 against 1: three camps or the camp of two: the AI keeps"),
		End({ 1, 2, 2, 2 }, 2) == EEnd::AIHolds && End({ 1, 2, 3 }, 0) == EEnd::AIHolds);
	// A human with an AI seat in his camp (Bob and an AI against Alice, as the other tests' configuration): the camp is not alone.
	TestTrue(TEXT("a human with an AI of his camp against one: the AI keeps"), End({ 1, 2, -2 }, 1) == EEnd::AIHolds);
	TestTrue(TEXT("... the one alone against that camp of two: defeat"), End({ 1, 2, -2 }, 0) == EEnd::Defeat);
	TestTrue(TEXT("a human against an AI alone (two camps): defeat"), End({ 1, -2 }, 0) == EEnd::Defeat);
	// The camps are those of the launch configuration (D357): the lobby's teams do not count in the Diplomacy mode.
	TestTrue(TEXT("Diplomacy, 2v2 teams ignored: four camps, the AI keeps"), End({ 1, 1, 2, 2 }, 0, EHOLMatchMode::Diplomacy) == EEnd::AIHolds);
	TestTrue(TEXT("Diplomacy, two players: two camps alone, defeat"), End({ 1, 1 }, 0, EHOLMatchMode::Diplomacy) == EEnd::Defeat);
	TestTrue(TEXT("a closed or unknown slot holds nothing: defeat"), End({ 1, 2 }, 5) == EEnd::Defeat && End({ 1, 2 }, INDEX_NONE) == EEnd::Defeat);

	// The seat's states: back in the window (the AI holds it for him), back after it while the match lasts, never when defeated.
	using ESeat = FHOLDisconnectRules::ESeatState;
	TestTrue(TEXT("present: his name takes the seat again (a second connection)"), FHOLDisconnectRules::MayReturn(ESeat::Present, false));
	TestTrue(TEXT("away, the AI holds it (window): he may return"), FHOLDisconnectRules::MayReturn(ESeat::AbsentAIHolds, false));
	TestTrue(TEXT("the AI holds it after the window: he may return while the match lasts"), FHOLDisconnectRules::MayReturn(ESeat::AIHolds, false));
	TestFalse(TEXT("... not when the match is over"), FHOLDisconnectRules::MayReturn(ESeat::AIHolds, true));
	TestFalse(TEXT("defeated: he returns as a spectator only"), FHOLDisconnectRules::MayReturn(ESeat::Defeated, false));
	TestFalse(TEXT("a seat defeated is not kept (KeepsSlot)"), FHOLDisconnectRules::KeepsSlot(Make({ 1, 2 }), 0, false, false, true));
	return true;
}

#endif
