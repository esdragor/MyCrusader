// Copyright Epic Games, Inc. All Rights Reserved.

#include "AI/HOLAIOrders.h"
#include "Buildings/HOLBuilding.h"
#include "Buildings/HOLBuildingTypes.h"
#include "Buildings/HOLProductionSubsystem.h"
#include "Commands/HOLCommandSubsystem.h"
#include "Commands/HOLProductionCommand.h"
#include "Engine/DataTable.h"
#include "Engine/Engine.h"
#include "Match/HOLMatchConfig.h"
#include "Match/HOLPlayerState.h"
#include "Simulation/HOLSimulationSettings.h"
#include "Simulation/HOLSimulationSubsystem.h"
#include "Engine/World.h"
#include "Net/UnrealNetwork.h"
#include "Match/HOLForumMessages.h"
#include "Match/HOLGameState.h"
#include "Match/HOLMarketRules.h"
#include "Misc/AutomationTest.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "UObject/Package.h"

#if WITH_DEV_AUTOMATION_TESTS

// HeroesOfLiberty.Market.Rules is gone: its CSV import, settings and exchange figures are Test_TableImportMarketSettings (Script/Tests/HOLTableImport_Test.as, seen red then green); the round trip is also Test_MarketRulesVectors (script).

// HeroesOfLiberty.Market.Apply (the trade on a stock) is gone: the rule is the script's (Test_MarketApplySellIsSell / Test_MarketApplyBuyIsBuyWithGold / Test_MarketApplyRefusals), the stock it moves is held by Market.Chain.
// M4c (first part): the common book lives in the game state and is replicated to everyone (D84: one list of prices, allies and
// enemies alike); a start puts every price at its base from the table, a change is read back.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLMarketBookStateTest, "HeroesOfLiberty.Market.BookState",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FHOLMarketBookStateTest::RunTest(const FString& Parameters)
{
	const FProperty* Prices = AHOLGameState::StaticClass()->FindPropertyByName(TEXT("MarketPrices"));
	if (!TestNotNull(TEXT("the game state has the MarketPrices property"), Prices))
	{
		return false;
	}
	TestTrue(TEXT("MarketPrices is replicated"), Prices->HasAnyPropertyFlags(CPF_Net));

	TArray<FLifetimeProperty> Lifetime;
	GetDefault<AHOLGameState>()->GetLifetimeReplicatedProps(Lifetime);
	TestTrue(TEXT("MarketPrices is in the replication list"), Lifetime.ContainsByPredicate([Prices](const FLifetimeProperty& Item)
	{
		return Item.RepIndex == Prices->RepIndex;
	}));

	UWorld* World = UWorld::CreateWorld(EWorldType::Game, /*bInformEngineOfWorld*/ false);
	FWorldContext& Context = GEngine->CreateNewWorldContext(EWorldType::Game);
	Context.SetCurrentWorld(World);
	World->InitializeActorsForPlay(FURL());
	AHOLGameState* State = World->SpawnActor<AHOLGameState>();
	if (TestNotNull(TEXT("game state spawned"), State))
	{
		State->ResetMarketBook();
		HOLMarket::FSettings S;
		if (TestTrue(TEXT("the Standard settings load"), HOLMarket::LoadStandardSettings(S)))
		{
			const HOLMarket::FBook Fresh = State->GetMarketBook();
			for (int32 Resource = 0; Resource < HOLMarket::BookSize; ++Resource)
			{
				TestEqual(*FString::Printf(TEXT("after a reset, resource %d at its base"), Resource), Fresh.Prices[Resource], S.BasePrices[Resource]);
			}
			HOLMarket::FBook Lowered = Fresh;
			Lowered.Prices[1] = S.BasePrices[1] - S.Step; // wood after one lot sold (80 -> 77)
			State->SetMarketBook(Lowered);
			TestEqual(TEXT("a set book is read back: wood at 77"), State->GetMarketBook().Prices[1], 77);
			TestEqual(TEXT("... the others untouched: stone at 120"), State->GetMarketBook().Prices[2], 120);
		}
	}
	GEngine->DestroyWorldContext(World);
	World->DestroyWorld(/*bInformEngineOfWorld*/ false);
	return true;
}

// M4c: the exchange order's whole chain on the server's real path: the order queue (UHOLCommandSubsystem, as ServerMarketTrade
// fills it), the production subsystem's step (owner, finished Market, resource, lots), the player's stock, the game state's
// common book, and the book's way back to rest on the fixed step. The figures are the table's (Market.csv, MK-Q1 = C).
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLMarketChainTest, "HeroesOfLiberty.Market.Chain",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FHOLMarketChainTest::RunTest(const FString& Parameters)
{
	HOLMarket::FSettings S;
	if (!TestTrue(TEXT("the Standard row of /Game/Match/MarketRates loads"), HOLMarket::LoadStandardSettings(S)))
	{
		return false;
	}
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, /*bInformEngineOfWorld*/ false);
	FWorldContext& Context = GEngine->CreateNewWorldContext(EWorldType::Game);
	Context.SetCurrentWorld(World);
	World->InitializeActorsForPlay(FURL());
	AHOLGameState* State = World->SpawnActor<AHOLGameState>();
	AHOLPlayerState* Player = World->SpawnActor<AHOLPlayerState>();
	UHOLCommandSubsystem* Commands = World->GetSubsystem<UHOLCommandSubsystem>();
	UHOLProductionSubsystem* Production = World->GetSubsystem<UHOLProductionSubsystem>();
	UHOLSimulationSubsystem* Simulation = World->GetSubsystem<UHOLSimulationSubsystem>();
	AHOLBuilding* Market = World->SpawnActor<AHOLBuilding>(FVector(0.0, 0.0, 0.0), FRotator::ZeroRotator);
	AHOLBuilding* Site = World->SpawnActor<AHOLBuilding>(FVector(5000.0, 0.0, 0.0), FRotator::ZeroRotator);
	AHOLBuilding* Forum = World->SpawnActor<AHOLBuilding>(FVector(0.0, 5000.0, 0.0), FRotator::ZeroRotator);
	AHOLBuilding* Enemy = World->SpawnActor<AHOLBuilding>(FVector(0.0, -5000.0, 0.0), FRotator::ZeroRotator);
	const bool bReady = State && Player && Commands && Production && Simulation && Market && Site && Forum && Enemy;
	if (TestTrue(TEXT("test world ready"), bReady))
	{
		FHOLMatchConfig Config = FHOLMatchConfig::MakeDefault();
		for (int32 Index = 0; Index < Config.Slots.Num(); ++Index)
		{
			Config.Slots[Index].State = Index <= 2 ? EHOLSlotState::Human : EHOLSlotState::Closed;
			Config.Slots[Index].Team = 0;
		}
		State->InitVictory(Config);
		State->ResetMarketBook();
		Player->SetMatchSlot(0, 0);
		State->AddPlayerState(Player);
		Market->Setup(HOLBuildingTypeNames::Market, 0, 0);
		Site->Setup(HOLBuildingTypeNames::Market, 0, 0, EHOLConstructionState::Foundation);
		Forum->Setup(HOLBuildingTypeNames::MainCentre, 0, 0);
		Enemy->Setup(HOLBuildingTypeNames::Market, 1, 1);
		Production->OnWorldBeginPlay(*World);

		int64 StepCount = 0;
		const auto RunStep = [&]() { Simulation->GetOnStepStarted().Broadcast(++StepCount); };
		const auto Order = [&](AHOLBuilding* At, const int32 Resource, const int32 Lots, const bool bBuy)
		{
			FHOLProductionCommand Command;
			Command.Kind = FHOLProductionCommand::EKind::Trade;
			Command.PlayerIndex = 0;
			Command.Building = At;
			Command.Resource = Resource;
			Command.Lots = Lots;
			Command.bBuy = bBuy;
			Commands->IssueProductionOrder(Command);
		};
		const int32 Food = static_cast<int32>(EHOLResource::Food);
		const int32 Wood = static_cast<int32>(EHOLResource::Wood);
		const int32 Stone = static_cast<int32>(EHOLResource::Stone);
		const int32 Gold = static_cast<int32>(EHOLResource::Gold);

		// 1. Two lots of wood sold: queued, applied at the next step (stock and common book).
		Player->SetResources(FHOLResources(500, 300, 0, 50));
		Order(Market, Wood, 2, false);
		TestTrue(TEXT("queued only: the stock has not moved"), Player->GetResources() == FHOLResources(500, 300, 0, 50));
		RunStep();
		TestTrue(TEXT("sold at the next step: wood 100, gold 50 + 56 + 53"), Player->GetResources() == FHOLResources(500, 100, 0, 159));
		TestEqual(TEXT("the common wood price falls to 74"), State->GetMarketBook().Prices[Wood], 74);
		TestEqual(TEXT("... stone untouched at 120"), State->GetMarketBook().Prices[Stone], 120);

		// 2. A lot of stone bought with that gold, at the lowered book's other price.
		Order(Market, Stone, 1, true);
		RunStep();
		TestTrue(TEXT("bought with 100 gold: stone 64, gold 159 - 100"), Player->GetResources() == FHOLResources(500, 100, 64, 59));
		TestEqual(TEXT("the common stone price rises to 123"), State->GetMarketBook().Prices[Stone], 123);

		// 3. Every refusal changes nothing: the stock, the book.
		const FHOLResources Before = Player->GetResources();
		const HOLMarket::FBook BookBefore = State->GetMarketBook();
		Order(Market, Wood, 4, false);       // not enough wood (100)
		Order(Market, Stone, 1, true);       // not enough gold (59 for a lot of 100)
		Order(Market, Gold, 1, false);       // gold is the money
		Order(Market, Gold, 1, true);
		Order(Market, INDEX_NONE, 1, false); // no such resource
		Order(Market, 7, 1, false);
		Order(Market, Wood, 0, false);       // no lots
		Order(Market, Wood, -3, false);
		Order(Market, Wood, 100000, false);  // far too many at once
		Order(Site, Wood, 1, false);         // a Market not finished
		Order(Enemy, Wood, 1, false);        // another player's Market
		Order(Forum, Wood, 1, false);        // not a Market
		Order(nullptr, Wood, 1, false);      // no building
		RunStep();
		TestTrue(TEXT("thirteen refused orders: the stock has not moved"), Player->GetResources() == Before);
		bool bBookSame = true;
		for (int32 Resource = 0; Resource < HOLMarket::BookSize; ++Resource)
		{
			bBookSame &= State->GetMarketBook().Prices[Resource] == BookBefore.Prices[Resource];
		}
		TestTrue(TEXT("... nor the book"), bBookSame);

		// 4. The way back to rest: one point toward the base every RecoverEverySeconds, none earlier.
		const float StepSeconds = GetDefault<UHOLSimulationSettings>()->GetStepSeconds();
		const int64 RecoverSteps = FMath::Max(1, FMath::RoundToInt(static_cast<float>(S.RecoverEverySeconds) / StepSeconds));
		while (StepCount < RecoverSteps - 1)
		{
			RunStep();
		}
		TestEqual(TEXT("one step before the interval: wood still 74"), State->GetMarketBook().Prices[Wood], 74);
		RunStep();
		TestEqual(TEXT("at the interval: wood back to 75"), State->GetMarketBook().Prices[Wood], 75);
		TestEqual(TEXT("... stone back to 122"), State->GetMarketBook().Prices[Stone], 122);
		TestEqual(TEXT("... food (at its base) unmoved"), State->GetMarketBook().Prices[Food], S.BasePrices[Food]);
		for (int64 Index = 0; Index < RecoverSteps; ++Index)
		{
			RunStep();
		}
		TestEqual(TEXT("one interval later: wood 76"), State->GetMarketBook().Prices[Wood], 76);
		TestEqual(TEXT("... stone 121"), State->GetMarketBook().Prices[Stone], 121);
		for (int64 Index = 0; Index < RecoverSteps * 10; ++Index)
		{
			RunStep();
		}
		TestEqual(TEXT("long after: wood at its base 80, not past it"), State->GetMarketBook().Prices[Wood], S.BasePrices[Wood]);
		TestEqual(TEXT("... stone at its base 120"), State->GetMarketBook().Prices[Stone], S.BasePrices[Stone]);

		// 5. The AI's way (M6): the book read by the brain's binding, the order through UHOLAIOrderLibrary at the Market's place.
		TArray<int32> Prices;
		if (TestTrue(TEXT("the AI reads the book"), UHOLAIOrderLibrary::ReadAIMarketBook(World, Prices)) && TestEqual(TEXT("... four prices"), Prices.Num(), 4))
		{
			TestEqual(TEXT("... wood at 80, stone at 120"), Prices[Wood] * 1000 + Prices[Stone], 80 * 1000 + 120);
		}
		const FHOLResources BeforeAI = Player->GetResources();
		TestTrue(TEXT("the AI's order for a lot of food is queued"), UHOLAIOrderLibrary::IssueAIMarketOrder(World, 0, Market->GetActorLocation(), Food, 1, false));
		TestFalse(TEXT("an AI order at a place with no Market of the slot is refused"), UHOLAIOrderLibrary::IssueAIMarketOrder(World, 0, FVector(900000.0, 0.0, 0.0), Food, 1, false));
		RunStep();
		TestTrue(TEXT("... sold at the next step: food 100 less, gold 56 more"),
			Player->GetResources() == FHOLResources(BeforeAI.Food - 100, BeforeAI.Wood, BeforeAI.Stone, BeforeAI.Gold + 56));
	}
	GEngine->DestroyWorldContext(World);
	World->DestroyWorld(/*bInformEngineOfWorld*/ false);
	return true;
}
// D372 (tributes): the order's whole chain on the server's real path, like Market.Chain: the order queue, the production step (owner,
// finished Market), the giver's stock, the receiver's stock, the tax. The 30 % is the rule's (HOLTribute::DefaultTaxPercent), as the script's.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLTributeChainTest, "HeroesOfLiberty.Market.Tribute",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FHOLTributeChainTest::RunTest(const FString& Parameters)
{
	TestEqual(TEXT("the tax is 30 % by default"), HOLTribute::DefaultTaxPercent, 30);
	// What a gift reaches the receiver as is the script's rule now: Test_MarketTributeVectors (Script/Economy/HOLMarketRules_Test.as) holds the four figures; the chain below reads it through the bridge.

	UWorld* World = UWorld::CreateWorld(EWorldType::Game, /*bInformEngineOfWorld*/ false);
	FWorldContext& Context = GEngine->CreateNewWorldContext(EWorldType::Game);
	Context.SetCurrentWorld(World);
	World->InitializeActorsForPlay(FURL());
	AHOLGameState* State = World->SpawnActor<AHOLGameState>();
	AHOLPlayerState* Giver = World->SpawnActor<AHOLPlayerState>();
	AHOLPlayerState* Taker = World->SpawnActor<AHOLPlayerState>();
	UHOLCommandSubsystem* Commands = World->GetSubsystem<UHOLCommandSubsystem>();
	UHOLProductionSubsystem* Production = World->GetSubsystem<UHOLProductionSubsystem>();
	UHOLSimulationSubsystem* Simulation = World->GetSubsystem<UHOLSimulationSubsystem>();
	AHOLBuilding* Market = World->SpawnActor<AHOLBuilding>(FVector(0.0, 0.0, 0.0), FRotator::ZeroRotator);
	AHOLBuilding* Site = World->SpawnActor<AHOLBuilding>(FVector(5000.0, 0.0, 0.0), FRotator::ZeroRotator);
	AHOLBuilding* Forum = World->SpawnActor<AHOLBuilding>(FVector(0.0, 5000.0, 0.0), FRotator::ZeroRotator);
	AHOLBuilding* Enemy = World->SpawnActor<AHOLBuilding>(FVector(0.0, -5000.0, 0.0), FRotator::ZeroRotator);
	if (TestTrue(TEXT("test world ready"), State && Giver && Taker && Commands && Production && Simulation && Market && Site && Forum && Enemy))
	{
		FHOLMatchConfig Config = FHOLMatchConfig::MakeDefault();
		for (int32 Index = 0; Index < Config.Slots.Num(); ++Index)
		{
			Config.Slots[Index].State = Index <= 2 ? EHOLSlotState::Human : EHOLSlotState::Closed;
			Config.Slots[Index].Team = 0;
		}
		State->InitVictory(Config);
		State->ResetMarketBook();
		Giver->SetMatchSlot(0, 0);
		Taker->SetMatchSlot(1, 1);
		State->AddPlayerState(Giver);
		State->AddPlayerState(Taker);
		Market->Setup(HOLBuildingTypeNames::Market, 0, 0);
		Site->Setup(HOLBuildingTypeNames::Market, 0, 0, EHOLConstructionState::Foundation);
		Forum->Setup(HOLBuildingTypeNames::MainCentre, 0, 0);
		Enemy->Setup(HOLBuildingTypeNames::Market, 1, 1);
		Production->OnWorldBeginPlay(*World);

		int64 StepCount = 0;
		const auto RunStep = [&]() { Simulation->GetOnStepStarted().Broadcast(++StepCount); };
		const auto Order = [&](AHOLBuilding* At, const int32 From, const int32 To, const int32 Resource, const int32 Amount)
		{
			FHOLProductionCommand Command;
			Command.Kind = FHOLProductionCommand::EKind::Tribute;
			Command.PlayerIndex = From;
			Command.Building = At;
			Command.Receiver = To;
			Command.Resource = Resource;
			Command.Amount = Amount;
			Commands->IssueProductionOrder(Command);
		};
		const int32 Food = static_cast<int32>(EHOLResource::Food);
		const int32 Wood = static_cast<int32>(EHOLResource::Wood);
		const int32 Gold = static_cast<int32>(EHOLResource::Gold);

		// TB-Q1 (2026-11-01): only the receiver is told, with what arrives after the tax; the giver and the others read nothing.
		TArray<int32> MessageReceivers;
		TArray<FString> MessageTexts;
		State->OnTributeMessage.AddLambda([&](const int32 Receiver, const FHOLForumMessage& Message)
		{
			MessageReceivers.Add(Receiver);
			MessageTexts.Add(Message.Text.ToString());
		});
		const FHOLForumMessage Pure = HOLForumMessages::GetTributeMessage(0, static_cast<int32>(EHOLResource::Gold), 70, 30, Config, 1);
		TestTrue(TEXT("the pure message names what arrives and the tax"), Pure.Text.ToString().Contains(TEXT("70")) && Pure.Text.ToString().Contains(TEXT("30")));
		TestTrue(TEXT("the pure message names the giver"), Pure.NamedSlots.Contains(0));

		// 1. 200 wood given from the finished Market: queued, applied at the next step; 140 arrive (30 % kept by the Market).
		Giver->SetResources(FHOLResources(500, 300, 0, 50));
		Taker->SetResources(FHOLResources(10, 20, 30, 40));
		Order(Market, 0, 1, Wood, 200);
		TestTrue(TEXT("queued only: nothing has moved"), Giver->GetResources() == FHOLResources(500, 300, 0, 50) && Taker->GetResources() == FHOLResources(10, 20, 30, 40));
		RunStep();
		TestTrue(TEXT("the giver pays 200 wood"), Giver->GetResources() == FHOLResources(500, 100, 0, 50));
		TestTrue(TEXT("the receiver gets 140 wood"), Taker->GetResources() == FHOLResources(10, 160, 30, 40));
		TestEqual(TEXT("one message, for the receiver alone"), MessageReceivers.Num(), 1);
		TestTrue(TEXT("it is for slot 1"), MessageReceivers.Num() == 1 && MessageReceivers[0] == 1);
		TestTrue(TEXT("it says 140 arrived and 60 was taxed"), MessageTexts.Num() == 1 && MessageTexts[0].Contains(TEXT("140")) && MessageTexts[0].Contains(TEXT("60")));

		// 2. Gold is given too (a tribute is not an exchange: no book, no lots).
		Order(Market, 0, 1, Gold, 50);
		RunStep();
		TestTrue(TEXT("50 gold given: 35 arrive"), Giver->GetResources() == FHOLResources(500, 100, 0, 0) && Taker->GetResources() == FHOLResources(10, 160, 30, 75));

		// 3. Every refusal changes nothing.
		const FHOLResources GiverBefore = Giver->GetResources();
		const FHOLResources TakerBefore = Taker->GetResources();
		Order(Market, 0, 1, Wood, 101);      // more than the stock
		Order(Market, 0, 1, Wood, 0);        // nothing
		Order(Market, 0, 1, Wood, -5);
		Order(Market, 0, 0, Food, 10);       // oneself
		Order(Market, 0, 7, Food, 10);       // nobody at that slot
		Order(Market, 0, INDEX_NONE, Food, 10);
		Order(Market, 0, 1, 9, 10);          // no such resource
		Order(Market, 0, 1, INDEX_NONE, 10);
		Order(Site, 0, 1, Food, 10);         // a Market not finished
		Order(Enemy, 0, 1, Food, 10);        // another player's Market
		Order(Forum, 0, 1, Food, 10);        // not a Market
		Order(nullptr, 0, 1, Food, 10);      // no building
		RunStep();
		TestTrue(TEXT("twelve refused orders: the giver's stock has not moved"), Giver->GetResources() == GiverBefore);
		TestTrue(TEXT("... nor the receiver's"), Taker->GetResources() == TakerBefore);
	}
	GEngine->DestroyWorldContext(World);
	World->DestroyWorld(/*bInformEngineOfWorld*/ false);
	return true;
}
#endif // WITH_DEV_AUTOMATION_TESTS