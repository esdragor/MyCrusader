// Copyright Epic Games, Inc. All Rights Reserved.

#include "Engine/DataTable.h"
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

#endif // WITH_DEV_AUTOMATION_TESTS