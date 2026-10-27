// Copyright Epic Games, Inc. All Rights Reserved.

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

// R7.2-R7.3 on the game state: Bob (300 wood) leaves, comes back with his 300 wood; leaves again, and at the end of the
// window his slot is defeated (Disconnected), collapsed as by a surrender, and Alice wins.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLDisconnectGameStateTest, "HeroesOfLiberty.Match.DisconnectGameState",
	UE::HeroesOfLiberty::Tests::Disconnect::Flags)
bool FHOLDisconnectGameStateTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::Disconnect;

	UWorld* World = UWorld::CreateWorld(EWorldType::Game, /*bInformEngineOfWorld*/ false);
	FWorldContext& Context = GEngine->CreateNewWorldContext(EWorldType::Game);
	Context.SetCurrentWorld(World);
	World->InitializeActorsForPlay(FURL());

	AHOLGameState* State = World->SpawnActor<AHOLGameState>();
	AHOLPlayerState* Bob = World->SpawnActor<AHOLPlayerState>();
	AHOLPlayerState* BobAgain = World->SpawnActor<AHOLPlayerState>();
	if (TestNotNull(TEXT("game state spawned"), State) && TestNotNull(TEXT("player states spawned"), Bob) && BobAgain)
	{
		State->InitVictory(MakeNamedConfig());
		const FHOLResources Starting(200, 200, 200, 200);
		Bob->SetMatchSlot(1, 1);
		TestFalse(TEXT("first login: not a return (no back to the manager)"), State->GiveSlotStock(*Bob, Starting));
		TestTrue(TEXT("first login: the stock of the start"), Bob->GetResources() == Starting);
		Bob->SetResources(FHOLResources(0, 300, 0, 0));

		TestTrue(TEXT("Bob leaves: slot kept"), State->KeepAbsentSlot(*Bob, 10.));
		TestTrue(TEXT("slot 1 absent"), State->IsSlotAbsent(1));
		TestEqual(TEXT("no defeat 179 s later"), State->DefeatExpiredAbsences(189., 180.).Num(), 0);
		BobAgain->SetMatchSlot(1, 1);
		TestTrue(TEXT("back in the window: a return (M6-Q15, back to the manager)"), State->GiveSlotStock(*BobAgain, Starting));
		TestTrue(TEXT("back in the window: his 300 wood, not the stock of the start"), BobAgain->GetResources() == FHOLResources(0, 300, 0, 0));
		TestFalse(TEXT("slot 1 no longer absent"), State->IsSlotAbsent(1));
		TestEqual(TEXT("back: no defeat after the window"), State->DefeatExpiredAbsences(1000., 180.).Num(), 0);

		TestTrue(TEXT("Bob leaves again"), State->KeepAbsentSlot(*BobAgain, 1000.));
		TestEqual(TEXT("the end of the window: slot 1 defeated"), State->DefeatExpiredAbsences(1180., 180.), TArray<int32>{ 1 });
		TestTrue(TEXT("reason Disconnected"), State->GetDefeatReason(1) == EHOLDefeatReason::Disconnected);
		TestEqual(TEXT("reason's name"), FString(AHOLGameState::GetReasonName(EHOLDefeatReason::Disconnected)), FString(TEXT("Disconnected")));
		TestFalse(TEXT("no longer absent once defeated"), State->IsSlotAbsent(1));
		TestFalse(TEXT("not over: the AI of Bob's camp plays on"), State->IsMatchOver());
		TestFalse(TEXT("a defeated slot is not kept"), State->KeepAbsentSlot(*BobAgain, 1200.));

		// B6b: Bob joins again after his defeat: he watches (no slot, no stock of the start on a beaten camp).
		TestEqual(TEXT("Alice, not defeated: her slot"), State->FindSlotOnLogin(MakeNamedConfig(), TEXT("Alice"), {}), 0);
		TestEqual(TEXT("Bob after his defeat: a spectator"), State->FindSlotOnLogin(MakeNamedConfig(), TEXT("Bob"), {}), INDEX_NONE);
	}

	// Without names (solo, a direct match): no window, the slot freed at once as before.
	AHOLGameState* Solo = World->SpawnActor<AHOLGameState>();
	AHOLPlayerState* Player = World->SpawnActor<AHOLPlayerState>();
	if (TestNotNull(TEXT("solo game state spawned"), Solo) && Player)
	{
		Solo->InitVictory(FHOLMatchConfig::MakeDefault());
		Player->SetMatchSlot(0, 0);
		TestFalse(TEXT("solo: not kept"), Solo->KeepAbsentSlot(*Player, 0.));
	}

	GEngine->DestroyWorldContext(World);
	World->DestroyWorld(/*bInformEngineOfWorld*/ false);
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

// R7.5: the banner of the others. Bob leaves at 10 (window 180 s): every machine gets his notice (replicated), its end at
// 190; at 29, « Bob : connexion perdue — 2:41 »; back at 100, « Bob : de retour » until 105, then nothing. Gone again and not
// back: his notice goes with his defeat (no more text here).
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLDisconnectBannerTest, "HeroesOfLiberty.Match.DisconnectBanner",
	UE::HeroesOfLiberty::Tests::Disconnect::Flags)
bool FHOLDisconnectBannerTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::Disconnect;

	const FString At161 = FHOLDisconnectRules::FormatAbsentLine(TEXT("Bob"), 161.).ToString();
	const FString Back = FHOLDisconnectRules::FormatBackLine(TEXT("Bob")).ToString();
	AddInfo(FString::Printf(TEXT("161 s: %s; back: %s"), *At161, *Back));
	// Neutral wording (the game does not know the players' gender), 2026-10-26.
	TestEqual(TEXT("161 s: neutral text"), At161, FString(TEXT("Bob : connexion perdue — 2:41")));
	TestEqual(TEXT("back: neutral text"), Back, FString(TEXT("Bob : de retour")));
	TestTrue(TEXT("160.2 s rounded up: 2:41"), FHOLDisconnectRules::FormatAbsentLine(TEXT("Bob"), 160.2).ToString().Contains(TEXT("2:41")));
	TestTrue(TEXT("59 s: 0:59"), FHOLDisconnectRules::FormatAbsentLine(TEXT("Bob"), 59.).ToString().Contains(TEXT("0:59")));
	TestTrue(TEXT("past the end: 0:00"), FHOLDisconnectRules::FormatAbsentLine(TEXT("Bob"), -3.).ToString().Contains(TEXT("0:00")));
	TestTrue(TEXT("back: Bob, no time"), Back.Contains(TEXT("Bob")) && !Back.Contains(TEXT("0:")) && !Back.Contains(TEXT("—")));
	TestFalse(TEXT("gone and back: two texts"), At161 == Back);

	const FProperty* Notices = AHOLGameState::StaticClass()->FindPropertyByName(TEXT("AbsentNotices"));
	TestTrue(TEXT("the notices replicated (players and spectators see them)"), Notices && Notices->HasAnyPropertyFlags(CPF_Net));
	const FProperty* Kept = AHOLGameState::StaticClass()->FindPropertyByName(TEXT("AbsentSlots"));
	TestTrue(TEXT("the stock kept is not (server only)"), !Kept || !Kept->HasAnyPropertyFlags(CPF_Net));

	UWorld* World = UWorld::CreateWorld(EWorldType::Game, /*bInformEngineOfWorld*/ false);
	FWorldContext& Context = GEngine->CreateNewWorldContext(EWorldType::Game);
	Context.SetCurrentWorld(World);
	World->InitializeActorsForPlay(FURL());

	AHOLGameState* State = World->SpawnActor<AHOLGameState>();
	AHOLPlayerState* Bob = World->SpawnActor<AHOLPlayerState>();
	AHOLPlayerState* BobAgain = World->SpawnActor<AHOLPlayerState>();
	if (TestNotNull(TEXT("game state spawned"), State) && TestNotNull(TEXT("player states spawned"), Bob) && BobAgain)
	{
		State->InitVictory(MakeNamedConfig());
		const FHOLResources Starting(200, 200, 200, 200);
		Bob->SetMatchSlot(1, 1);
		State->GiveSlotStock(*Bob, Starting, 0.);
		TestEqual(TEXT("first login: no notice"), State->GetAbsentNotices().Num(), 0);

		TestTrue(TEXT("Bob leaves at 10"), State->KeepAbsentSlot(*Bob, 10., 180.));
		if (TestEqual(TEXT("one notice"), State->GetAbsentNotices().Num(), 1))
		{
			const FHOLAbsentNotice& Notice = State->GetAbsentNotices()[0];
			TestEqual(TEXT("his slot"), Notice.Slot, 1);
			TestEqual(TEXT("his name"), Notice.Name, FString(TEXT("Bob")));
			TestEqual(TEXT("the window's end at 190"), Notice.EndsAt, 190.);
			TestEqual(TEXT("not back"), Notice.BackAt, 0.);
		}
		// A client's clock of the server a little behind at the departure: never more than the window (3:01 seen in
		// lobby_match -Rejoin, 2026-10-26).
		const TArray<FText> At9 = State->GetAbsenceBannerLines(9.6);
		if (TestEqual(TEXT("at 9.6: one line"), At9.Num(), 1))
		{
			TestEqual(TEXT("at 9.6 (clock behind): 3:00, not 3:01"), At9[0].ToString(), FString(TEXT("Bob : connexion perdue — 3:00")));
		}
		const TArray<FText> At29 = State->GetAbsenceBannerLines(29.);
		if (TestEqual(TEXT("at 29: one line"), At29.Num(), 1))
		{
			TestEqual(TEXT("at 29: Bob, 2:41"), At29[0].ToString(), At161);
		}

		BobAgain->SetMatchSlot(1, 1);
		TestTrue(TEXT("back at 100"), State->GiveSlotStock(*BobAgain, Starting, 100.));
		const TArray<FText> At101 = State->GetAbsenceBannerLines(101.);
		if (TestEqual(TEXT("at 101: one line"), At101.Num(), 1))
		{
			TestEqual(TEXT("at 101: Bob is back"), At101[0].ToString(), Back);
		}
		TestEqual(TEXT("at 104.9: still back"), State->GetAbsenceBannerLines(104.9).Num(), 1);
		TestEqual(TEXT("at 105: nothing (even before the server drops it)"), State->GetAbsenceBannerLines(105.).Num(), 0);
		State->DefeatExpiredAbsences(105., 180.);
		TestEqual(TEXT("at 105: the notice dropped"), State->GetAbsentNotices().Num(), 0);

		TestTrue(TEXT("Bob leaves again at 200"), State->KeepAbsentSlot(*BobAgain, 200., 180.));
		TestEqual(TEXT("his notice again"), State->GetAbsentNotices().Num(), 1);
		TestEqual(TEXT("not back by 380: defeated"), State->DefeatExpiredAbsences(380., 180.), TArray<int32>{ 1 });
		TestEqual(TEXT("defeated: no notice left (the defeat says the rest)"), State->GetAbsentNotices().Num(), 0);
		TestEqual(TEXT("defeated: no line"), State->GetAbsenceBannerLines(381.).Num(), 0);
	}

	GEngine->DestroyWorldContext(World);
	World->DestroyWorld(/*bInformEngineOfWorld*/ false);
	return true;
}

#endif
