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

		// The tribute: the tax is kept (30 %, rounded down), a gift of 1 is not eaten.
		TestEqual(TEXT("100 sent at 30 %: 70 arrive"), Board->ReceivedNow(100, HOLTribute::DefaultTaxPercent), 70);
		TestEqual(TEXT("250 sent at 30 %: 175 arrive"), Board->ReceivedNow(250, HOLTribute::DefaultTaxPercent), 175);
		TestEqual(TEXT("1 sent at 30 %: 1 arrives"), Board->ReceivedNow(1, HOLTribute::DefaultTaxPercent), 1);
		// The server's call (the static one), at figures worked by hand: Sent minus Sent * 30 / 100 rounded down.
		const int32 SentAndArrives[][2] = { { 1, 1 }, { 7, 5 }, { 10, 7 }, { 99, 70 }, { 100, 70 }, { 250, 175 }, { 1000, 700 } };
		for (const int32 (&Case)[2] : SentAndArrives)
		{
			TestEqual(*FString::Printf(TEXT("%d sent: the bridge gives the script's amount"), Case[0]),
				UHOLMarketSubsystem::Received(World, Case[0], HOLTribute::DefaultTaxPercent), Case[1]);
		}
	}
	GEngine->DestroyWorldContext(World);
	World->DestroyWorld(false);
	return true;
}

#endif