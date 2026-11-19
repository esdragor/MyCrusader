// Copyright Epic Games, Inc. All Rights Reserved.

#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Match/HOLMarketRules.h"
#include "Match/HOLMarketSubsystem.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

// MIG-CPP-Market (D836 point 4): the book's return to rest and the tribute's tax are the script's (Script/Economy/HOLMarketRules.as, HOLTributeRules.as),
// reached by the server through the bridge UHOLMarketSubsystem (Script/Economy/HOLMarketBoardScript.as). The rules are tested in script
// (HOLMarketRules_Test.as, HOLTributeRules_Test.as); here the real chain C++ bridge -> script board, on the figures of the standard row (option C).
namespace UE::HeroesOfLiberty::Tests::MarketBridge
{
	constexpr EAutomationTestFlags Flags = EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter;

	/** The standard row as settings (food 80, wood 80, stone 120, gold 100; step 3, tax 30 %, 1 point back every 10 s; prices in 20..300). */
	HOLMarket::FSettings StandardSettings()
	{
		HOLMarket::FSettings Settings;
		Settings.BasePrices[0] = 80;
		Settings.BasePrices[1] = 80;
		Settings.BasePrices[2] = 120;
		Settings.BasePrices[3] = 100;
		Settings.Step = 3;
		Settings.FeePercent = 30;
		Settings.MinPrice = 20;
		Settings.MaxPrice = 300;
		Settings.RecoverPoints = 1;
		Settings.RecoverEverySeconds = 10;
		return Settings;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLMarketServerBoardTest, "HeroesOfLiberty.Market.ServerBoard", UE::HeroesOfLiberty::Tests::MarketBridge::Flags)

bool FHOLMarketServerBoardTest::RunTest(const FString& Parameters)
{
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, /*bInformEngineOfWorld*/ false);
	FWorldContext& Context = GEngine->CreateNewWorldContext(EWorldType::Game);
	Context.SetCurrentWorld(World);
	World->InitializeActorsForPlay(FURL());

	UHOLMarketSubsystem* Board = UHOLMarketSubsystem::Find(World);
	if (TestNotNull(TEXT("the script board of the market"), Board))
	{
		const HOLMarket::FSettings Settings = UE::HeroesOfLiberty::Tests::MarketBridge::StandardSettings();

		// The book at rest, through the board (HOLMarket::NewBook of the script): every price at its base (food 80, wood 80, stone 120, gold 100).
		const HOLMarket::FBook Rest0 = UHOLMarketSubsystem::NewBook(World, Settings);
		for (int32 Index = 0; Index < HOLMarket::BookSize; ++Index)
		{
			TestEqual(*FString::Printf(TEXT("the fresh book has price %d at its base"), Index), Rest0.Prices[Index], Settings.BasePrices[Index]);
		}

		// The return to rest, in whole intervals, through the board (bridge structures in, bridge structures out).
		HOLMarket::FBook Raised;
		Raised.Prices[0] = 110;
		Raised.Prices[1] = 70;
		Raised.Prices[2] = 125;
		Raised.Prices[3] = 100;
		const FHOLMarketBridgeBook Three = Board->RecoverNow(UHOLMarketSubsystem::ToBridge(Settings), UHOLMarketSubsystem::ToBridge(Raised), 3);
		if (TestEqual(TEXT("the board returns the four prices"), Three.Prices.Num(), 4))
		{
			TestEqual(TEXT("Food 110 is 3 points nearer its base 80: 107"), Three.Prices[0], 107);
			TestEqual(TEXT("Wood 70 is 3 points nearer its base 80: 73"), Three.Prices[1], 73);
			TestEqual(TEXT("Stone 125 is 3 points nearer its base 120: 122"), Three.Prices[2], 122);
			TestEqual(TEXT("Gold at its base stays at 100"), Three.Prices[3], 100);
		}
		const FHOLMarketBridgeBook Rest = Board->RecoverNow(UHOLMarketSubsystem::ToBridge(Settings), UHOLMarketSubsystem::ToBridge(Raised), 1000);
		if (TestEqual(TEXT("a long time later the board returns the four prices"), Rest.Prices.Num(), 4))
		{
			TestEqual(TEXT("Food never goes past its base"), Rest.Prices[0], 80);
			TestEqual(TEXT("Wood never goes past its base"), Rest.Prices[1], 80);
			TestEqual(TEXT("Stone never goes past its base"), Rest.Prices[2], 120);
		}

		// The server's call (the static one) is the board's answer, at figures worked by hand (1 point per interval toward the base, never past it).
		struct FRecoverCase { int32 Intervals; int32 Prices[HOLMarket::BookSize]; };
		const FRecoverCase Cases[] = { { 0, { 110, 70, 125, 100 } }, { 5, { 105, 75, 120, 100 } }, { 10, { 100, 80, 120, 100 } }, { 40, { 80, 80, 120, 100 } } };
		for (const FRecoverCase& Case : Cases)
		{
			const HOLMarket::FBook ViaBridge = UHOLMarketSubsystem::Recover(World, Settings, Raised, Case.Intervals);
			for (int32 Index = 0; Index < HOLMarket::BookSize; ++Index)
			{
				TestEqual(*FString::Printf(TEXT("%d intervals, resource %d: the bridge gives the script's price"), Case.Intervals, Index), ViaBridge.Prices[Index], Case.Prices[Index]);
			}
		}

		// The order's two stocks (D851 step 5 R2, the call ApplyTribute makes): the script's Settle, through the static bridge call.
		const FHOLResources GiverStock(10, 500, 20, 30);
		const FHOLResources TakerStock(1, 5, 2, 3);
		const FHOLTributeBridgeResult Done = UHOLMarketSubsystem::SettleTribute(World, true, 0, 1, true, GiverStock, TakerStock, static_cast<int32>(EHOLResource::Wood), 100, HOLTribute::DefaultTaxPercent);
		TestTrue(TEXT("100 wood given: accepted"), Done.bOk && Done.Reason == 0);
		TestTrue(TEXT("the giver pays 100, the receiver gets 70"), Done.Giver == FHOLResources(10, 400, 20, 30) && Done.Taker == FHOLResources(1, 75, 2, 3) && Done.Received == 70);
		const FHOLTributeBridgeResult Short = UHOLMarketSubsystem::SettleTribute(World, true, 0, 1, true, FHOLResources(0, 99, 0, 0), TakerStock, static_cast<int32>(EHOLResource::Wood), 100, HOLTribute::DefaultTaxPercent);
		TestTrue(TEXT("99 cannot give 100: refused for the stock (4), nothing moves"), !Short.bOk && Short.Reason == 4 && Short.Giver == FHOLResources(0, 99, 0, 0) && Short.Taker == TakerStock);
	}
	GEngine->DestroyWorldContext(World);
	World->DestroyWorld(false);
	return true;
}

#endif