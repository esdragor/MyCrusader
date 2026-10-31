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
#include "Match/HOLGameState.h"
#include "Match/HOLMarketRules.h"
#include "Misc/AutomationTest.h"
#include "Misc/FileHelper.h"
#include "Misc/Paths.h"
#include "UObject/Package.h"

#if WITH_DEV_AUTOMATION_TESTS

// M4a (Market, GDD 5.5): the exchange rule where the simulation runs, on the real chain: Tools/Data/Market.csv imported as
// the editor does, turned into settings by SettingsFromRow, then sales, purchases and the way back. The figures are those of
// the script test of Script/Economy/HOLMarketRules.as (MK-Q1 = C, D582): the two rules must agree.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLMarketRulesTest, "HeroesOfLiberty.Market.Rules",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FHOLMarketRulesTest::RunTest(const FString& Parameters)
{
	constexpr int32 Food = 0, Wood = 1, Stone = 2, Gold = 3;

	FString Text;
	const FString Path = FPaths::Combine(FPaths::ProjectDir(), TEXT("Tools"), TEXT("Data"), TEXT("Market.csv"));
	if (!TestTrue(TEXT("Market.csv is there"), FFileHelper::LoadFileToString(Text, *Path)))
	{
		return false;
	}
	UDataTable* Table = NewObject<UDataTable>(GetTransientPackage());
	Table->RowStruct = FHOLMarketRow::StaticStruct();
	TestTrue(TEXT("Market.csv imports without a problem"), Table->CreateTableFromCSVString(Text).IsEmpty());
	const FHOLMarketRow* Row = Table->FindRow<FHOLMarketRow>(TEXT("Standard"), TEXT("market rates"), false);
	if (!TestNotNull(TEXT("the Standard row"), Row))
	{
		return false;
	}

	const HOLMarket::FSettings S = HOLMarket::SettingsFromRow(*Row);
	TestEqual(TEXT("settings: food base"), S.BasePrices[Food], 80);
	TestEqual(TEXT("settings: stone base"), S.BasePrices[Stone], 120);
	TestEqual(TEXT("settings: gold base"), S.BasePrices[Gold], 100);
	TestEqual(TEXT("settings: tax"), S.FeePercent, 30);
	TestEqual(TEXT("settings: step"), S.Step, 3);
	TestEqual(TEXT("settings: floor"), S.MinPrice, 20);
	TestEqual(TEXT("settings: cap"), S.MaxPrice, 300);

	const HOLMarket::FBook Book = HOLMarket::NewBook(S);
	TestEqual(TEXT("fresh book: wood at 80"), Book.Prices[Wood], 80);
	TestEqual(TEXT("fresh book: stone at 120"), Book.Prices[Stone], 120);

	// The round trip of the script test: wood sold at 80 pays 56 and falls to 77; bought back at 77 costs 101 and returns to 80.
	const HOLMarket::FTrade Sold = HOLMarket::Sell(S, Book, Wood, 1, 100, S.FeePercent);
	TestTrue(TEXT("a lot of wood sold"), Sold.bOk);
	TestEqual(TEXT("it pays 56"), Sold.Gold, 56);
	TestEqual(TEXT("100 wood leave the stock"), Sold.Units, 100);
	TestEqual(TEXT("wood falls to 77"), Sold.Book.Prices[Wood], 77);
	TestEqual(TEXT("the book given is not changed"), Book.Prices[Wood], 80);
	const HOLMarket::FTrade Bought = HOLMarket::Buy(S, Sold.Book, Wood, 1, 1000, S.FeePercent);
	TestTrue(TEXT("a lot of wood bought"), Bought.bOk);
	TestEqual(TEXT("it costs 101"), Bought.Gold, 101);
	TestEqual(TEXT("wood returns to 80"), Bought.Book.Prices[Wood], 80);

	// Stone: 120 sold pays 84, bought back at 117 costs 153.
	const HOLMarket::FTrade StoneSold = HOLMarket::Sell(S, Book, Stone, 1, 100, S.FeePercent);
	TestEqual(TEXT("a lot of stone pays 84"), StoneSold.Gold, 84);
	TestEqual(TEXT("a lot of stone bought back costs 153"), HOLMarket::Buy(S, StoneSold.Book, Stone, 1, 1000, S.FeePercent).Gold, 153);

	// Lot by lot: three lots of wood at 80, 77, 74.
	const HOLMarket::FTrade Three = HOLMarket::Sell(S, Book, Wood, 3, 300, S.FeePercent);
	TestEqual(TEXT("three lots, each at the price left by the one before"), Three.Gold, 56 + 53 + 51);
	TestEqual(TEXT("three lots: wood at 71"), Three.Book.Prices[Wood], 71);

	// The refusals change nothing.
	TestEqual(TEXT("zero lots"), HOLMarket::Sell(S, Book, Wood, 0, 500, S.FeePercent).Reason, FName(TEXT("lots")));
	TestEqual(TEXT("gold is not traded"), HOLMarket::Sell(S, Book, Gold, 1, 500, S.FeePercent).Reason, FName(TEXT("resource")));
	TestEqual(TEXT("two lots from 150"), HOLMarket::Sell(S, Book, Wood, 2, 150, S.FeePercent).Reason, FName(TEXT("stock")));
	const HOLMarket::FTrade Poor = HOLMarket::Buy(S, Book, Stone, 1, 10, S.FeePercent);
	TestEqual(TEXT("a lot with 10 gold"), Poor.Reason, FName(TEXT("gold")));
	TestTrue(TEXT("a refusal moves nothing"), !Poor.bOk && Poor.Gold == 0 && Poor.Units == 0 && Poor.Book.Prices[Stone] == 120);

	// The bounds, and the way back.
	HOLMarket::FBook Low = Book;
	Low.Prices[Wood] = S.MinPrice + 1;
	TestEqual(TEXT("the price stops at the floor"), HOLMarket::Sell(S, Low, Wood, 3, 1000, S.FeePercent).Book.Prices[Wood], S.MinPrice);
	HOLMarket::FBook High = Book;
	High.Prices[Wood] = S.MaxPrice - 1;
	TestEqual(TEXT("the price stops at the cap"), HOLMarket::Buy(S, High, Wood, 3, 100000, S.FeePercent).Book.Prices[Wood], S.MaxPrice);
	TestEqual(TEXT("4 intervals: wood 71 to 75"), HOLMarket::Recover(S, Three.Book, 4).Prices[Wood], 75);
	TestEqual(TEXT("9 intervals: back to 80"), HOLMarket::Recover(S, Three.Book, 9).Prices[Wood], 80);
	TestEqual(TEXT("no further than the base"), HOLMarket::Recover(S, Three.Book, 50).Prices[Wood], 80);
	TestEqual(TEXT("a price above the base comes down"), HOLMarket::Recover(S, High, 2).Prices[Wood], S.MaxPrice - 3);
	return true;
}

// M4b: one trade on a stock, the end of the order's chain: a sale takes the goods and gives the gold, a purchase the opposite,
// the book moves, a refusal changes nothing. The figures are the round trip's (wood 80 sells 56 then 53).
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLMarketApplyTest, "HeroesOfLiberty.Market.Apply",
	EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter)
bool FHOLMarketApplyTest::RunTest(const FString& Parameters)
{
	// The settings come from the table asset the game reads (make_market.py): the real chain, no number typed in the rule.
	HOLMarket::FSettings S;
	if (!TestTrue(TEXT("the Standard row of /Game/Match/MarketRates loads"), HOLMarket::LoadStandardSettings(S)))
	{
		return false;
	}
	TestEqual(TEXT("loaded: wood base 80"), S.BasePrices[1], 80);
	TestEqual(TEXT("loaded: stone base 120"), S.BasePrices[2], 120);
	TestEqual(TEXT("loaded: tax 30"), S.FeePercent, 30);

	const HOLMarket::FBook Book = HOLMarket::NewBook(S);
	const FHOLResources Stock(500, 300, 0, 50);
	FHOLResources After;

	const HOLMarket::FTrade Sale = HOLMarket::Apply(S, Book, Stock, EHOLResource::Wood, 2, false, S.FeePercent, After);
	TestTrue(TEXT("two lots of wood sold"), Sale.bOk);
	TestTrue(TEXT("the stock after the sale: wood 100, gold 50 + 56 + 53"), After == FHOLResources(500, 100, 0, 159));
	TestEqual(TEXT("wood falls to 74"), Sale.Book.Prices[1], 74);

	FHOLResources Later;
	const HOLMarket::FTrade Purchase = HOLMarket::Apply(S, Sale.Book, After, EHOLResource::Stone, 1, true, S.FeePercent, Later);
	TestTrue(TEXT("a lot of stone bought with that gold"), Purchase.bOk);
	TestEqual(TEXT("it costs 156 (120 plus 30 %)"), Purchase.Gold, 156);
	TestTrue(TEXT("the stock after the purchase: stone 100, gold 3"), Later == FHOLResources(500, 100, 100, 3));
	TestEqual(TEXT("stone rises to 123"), Purchase.Book.Prices[2], 123);

	// Refusals: the stock out is the stock in.
	FHOLResources Same;
	const HOLMarket::FTrade TooMuch = HOLMarket::Apply(S, Book, Stock, EHOLResource::Wood, 4, false, S.FeePercent, Same);
	TestTrue(TEXT("four lots from 300 wood: refused (stock)"), !TooMuch.bOk && TooMuch.Reason == FName(TEXT("stock")) && Same == Stock);
	const HOLMarket::FTrade TooPoor = HOLMarket::Apply(S, Book, Stock, EHOLResource::Stone, 1, true, S.FeePercent, Same);
	TestTrue(TEXT("a lot of stone with 50 gold: refused (gold)"), !TooPoor.bOk && TooPoor.Reason == FName(TEXT("gold")) && Same == Stock);
	const HOLMarket::FTrade ForGold = HOLMarket::Apply(S, Book, Stock, EHOLResource::Gold, 1, false, S.FeePercent, Same);
	TestTrue(TEXT("gold sold: refused (resource)"), !ForGold.bOk && ForGold.Reason == FName(TEXT("resource")) && Same == Stock);
	return true;
}
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
			const HOLMarket::FTrade Sold = HOLMarket::Sell(S, Fresh, 1, 1, 100, S.FeePercent);
			State->SetMarketBook(Sold.Book);
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
		TestTrue(TEXT("bought: stone 100, gold 159 - 156"), Player->GetResources() == FHOLResources(500, 100, 100, 3));
		TestEqual(TEXT("the common stone price rises to 123"), State->GetMarketBook().Prices[Stone], 123);

		// 3. Every refusal changes nothing: the stock, the book.
		const FHOLResources Before = Player->GetResources();
		const HOLMarket::FBook BookBefore = State->GetMarketBook();
		Order(Market, Wood, 4, false);       // not enough wood (100)
		Order(Market, Stone, 1, true);       // not enough gold (3)
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
#endif // WITH_DEV_AUTOMATION_TESTS