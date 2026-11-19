// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Match/HOLMarketBridgeTypes.h"
#include "Match/HOLMarketRules.h"
#include "Subsystems/WorldSubsystem.h"
#include "HOLMarketSubsystem.generated.h"

/**
 * The Market's low-frequency rule where the match runs (MIG-CPP-Market, D836 point 4: the script is the source of the rule). The thinnest bridge, as
 * UHOLUnitTierSubsystem: the server (AHOLProductionSubsystem) asks its script subclass (Script/Economy/HOLMarketBoardScript.as, the rules
 * Script/Economy/HOLMarketRules.as and HOLTributeRules.as) for the calls that are made rarely: the return of the book to rest (every
 * RecoverEverySeconds, HOLMarket::Recover) and the tribute's settlement (once per Tribute order, HOLTribute::Settle, which applies HOLTribute::Received). Abstract: only a script
 * subclass is born. There is no C++ copy of these rules any more: without a script board (no script compiled) the static calls are inert and
 * safe (the book is returned unchanged, a tribute is refused) and log an error. Left in C++: the Slate quote
 * (HOLGameScreen), the taxes of the diplomacy library, HOLPlayerController and HOLAIOrders. The trade itself is now the script's too (UHOLMarketSubsystem::Apply -> HOLMarket::Apply of the script, once per
 * Trade order / Market click, MIG-C++-FINAL); the C++ copies HOLMarket::Apply and Buy are gone (MIG-CPP-Market); Sell/BuyWithGold remain
 * for the Slate quote (HOLGameScreen) and the C++ tests of the table chain; NewBook is the script's too (UHOLMarketSubsystem::NewBook, once per match, HOLGameState::ResetMarketBook).
 */
UCLASS(Abstract)
class HEROESOFLIBERTY_API UHOLMarketSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	/** The world's market board (its script subclass), or null (no script compiled, or no world). */
	static UHOLMarketSubsystem* Find(const UWorld* World);

	/** The book after Intervals intervals back toward rest: the board's answer; the book unchanged (and an error logged) when the world has no script board. */
	static HOLMarket::FBook Recover(const UWorld* World, const HOLMarket::FSettings& Settings, const HOLMarket::FBook& Book, int32 Intervals);

	/**
	 * One trade on a player's stock, the script's rule (HOLMarket::Apply of Script/Economy/HOLMarketRules.as, MIG-C++-FINAL) with the stock moved here as the
	 * C++ Apply did: OutStock is Stock changed (Stock itself when refused). A refused trade with Reason "board" (and an error logged) when the world has no script board.
	 */
	static HOLMarket::FTrade Apply(const UWorld* World, const HOLMarket::FSettings& Settings, const HOLMarket::FBook& Book, const FHOLResources& Stock, EHOLResource Resource, int32 Lots, bool bBuy, FHOLResources& OutStock);

	/** The book at rest from these settings (every price at its base): the board's answer (HOLMarket::NewBook of the script); an all-zero book (and an error logged) when the world has no script board. */
	static HOLMarket::FBook NewBook(const UWorld* World, const HOLMarket::FSettings& Settings);

	/**
	 * One tribute on the two stocks, the script's rule (HOLTribute::Settle of Script/Economy/HOLTributeRules.as, D851 step 5 R1): the result carries both stocks
	 * after the tribute (as given when refused). A refusal with Reason -1 (and an error logged) when the world has no script board. The caller still applies the stocks.
	 */
	static FHOLTributeBridgeResult SettleTribute(const UWorld* World, bool bSenderHasMarket, int32 Sender, int32 Receiver, bool bReceiverInMatch, const FHOLResources& GiverStock, const FHOLResources& TakerStock, int32 Resource, int32 Sent, int32 TaxPercent);

	/** The board's answers (the native events below, i.e. the script rules in the script subclass). */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|Market")
	FHOLMarketBridgeTrade TradeNow(const FHOLMarketBridgeSettings& Settings, const FHOLMarketBridgeBook& Book, int32 Resource, int32 Lots, bool bBuy, int32 Stock);

	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|Market")
	FHOLMarketBridgeBook NewBookNow(const FHOLMarketBridgeSettings& Settings);

	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|Market")
	FHOLMarketBridgeBook RecoverNow(const FHOLMarketBridgeSettings& Settings, const FHOLMarketBridgeBook& Book, int32 Intervals);

	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|Market")
	FHOLTributeBridgeResult SettleNow(bool bSenderHasMarket, int32 Sender, int32 Receiver, bool bReceiverInMatch, const FHOLResources& GiverStock, const FHOLResources& TakerStock, int32 Resource, int32 Sent, int32 TaxPercent);

	/** The conversions between the C++ rule's structures and the bridge's (a missing price counts as 0 in a short array). */
	static FHOLMarketBridgeSettings ToBridge(const HOLMarket::FSettings& Settings);
	static HOLMarket::FSettings FromBridge(const FHOLMarketBridgeSettings& Settings);
	static FHOLMarketBridgeBook ToBridge(const HOLMarket::FBook& Book);
	static HOLMarket::FBook FromBridge(const FHOLMarketBridgeBook& Book);

	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

protected:
	virtual bool DoesSupportWorldType(const EWorldType::Type WorldType) const override;

	/** The book after Intervals intervals toward rest. The script subclass overrides it (HOLMarket::Recover of the script); the default is inert: the book unchanged, an error logged. */
	UFUNCTION(BlueprintNativeEvent, Category = "HeroesOfLiberty|Market")
	FHOLMarketBridgeBook RecoverPrices(const FHOLMarketBridgeSettings& Settings, const FHOLMarketBridgeBook& Book, int32 Intervals);
	virtual FHOLMarketBridgeBook RecoverPrices_Implementation(const FHOLMarketBridgeSettings& Settings, const FHOLMarketBridgeBook& Book, int32 Intervals);

	/** One trade at the settings' own tax; Stock is what is given up (the goods of Resource for a sale, the gold for a purchase). The script subclass overrides it (HOLMarket::Apply of the script); the default refuses with Reason "board" and logs an error. */
	UFUNCTION(BlueprintNativeEvent, Category = "HeroesOfLiberty|Market")
	FHOLMarketBridgeTrade TradeOnce(const FHOLMarketBridgeSettings& Settings, const FHOLMarketBridgeBook& Book, int32 Resource, int32 Lots, bool bBuy, int32 Stock);
	virtual FHOLMarketBridgeTrade TradeOnce_Implementation(const FHOLMarketBridgeSettings& Settings, const FHOLMarketBridgeBook& Book, int32 Resource, int32 Lots, bool bBuy, int32 Stock);

	/** The book at rest from Settings. The script subclass overrides it (HOLMarket::NewBook of the script); the default is inert: an empty book (read as all zero), an error logged. */
	UFUNCTION(BlueprintNativeEvent, Category = "HeroesOfLiberty|Market")
	FHOLMarketBridgeBook FreshBook(const FHOLMarketBridgeSettings& Settings);
	virtual FHOLMarketBridgeBook FreshBook_Implementation(const FHOLMarketBridgeSettings& Settings);

	/** The tribute applied to the two stocks. The script subclass overrides it (HOLTribute::Settle of the script); the default is inert: a refusal with Reason -1 and the stocks as given, an error logged. */
	UFUNCTION(BlueprintNativeEvent, Category = "HeroesOfLiberty|Market")
	FHOLTributeBridgeResult TributeSettle(bool bSenderHasMarket, int32 Sender, int32 Receiver, bool bReceiverInMatch, const FHOLResources& GiverStock, const FHOLResources& TakerStock, int32 Resource, int32 Sent, int32 TaxPercent);
	virtual FHOLTributeBridgeResult TributeSettle_Implementation(bool bSenderHasMarket, int32 Sender, int32 Receiver, bool bReceiverInMatch, const FHOLResources& GiverStock, const FHOLResources& TakerStock, int32 Resource, int32 Sent, int32 TaxPercent);
};