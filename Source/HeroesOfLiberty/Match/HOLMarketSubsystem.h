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
 * Script/Economy/HOLMarketRules.as and HOLTributeRules.as) for the two calls that are made rarely: the return of the book to rest (every
 * RecoverEverySeconds, HOLMarket::Recover) and what a tribute delivers (once per Tribute order, HOLTribute::Received). Abstract: only a script
 * subclass is born. Without it (no script compiled) the static calls fall back to the C++ rules, so the behaviour is the same as before the bridge;
 * the C++ rules are kept in this lot. Left in C++ this lot: NewBook (HOLGameState), the Slate quote (HOLGameScreen), the taxes of the diplomacy
 * library, HOLPlayerController and HOLAIOrders. The trade itself (HOLMarket::Apply) stays C++ for now (the price of each order, higher frequency).
 */
UCLASS(Abstract)
class HEROESOFLIBERTY_API UHOLMarketSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	/** The world's market board (its script subclass), or null (no script compiled, or no world). */
	static UHOLMarketSubsystem* Find(const UWorld* World);

	/** The book after Intervals intervals back toward rest: the board's answer, or HOLMarket::Recover's when the world has no script board. */
	static HOLMarket::FBook Recover(const UWorld* World, const HOLMarket::FSettings& Settings, const HOLMarket::FBook& Book, int32 Intervals);

	/** What reaches the receiver of a tribute of Sent at TaxPercent: the board's answer, or HOLTribute::Received's when the world has no script board. */
	static int32 Received(const UWorld* World, int32 Sent, int32 TaxPercent);

	/** The board's answers (the native events below, i.e. the script rules in the script subclass). */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|Market")
	FHOLMarketBridgeBook RecoverNow(const FHOLMarketBridgeSettings& Settings, const FHOLMarketBridgeBook& Book, int32 Intervals);

	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|Market")
	int32 ReceivedNow(int32 Sent, int32 TaxPercent);

	/** The conversions between the C++ rule's structures and the bridge's (a missing price counts as 0 in a short array). */
	static FHOLMarketBridgeSettings ToBridge(const HOLMarket::FSettings& Settings);
	static HOLMarket::FSettings FromBridge(const FHOLMarketBridgeSettings& Settings);
	static FHOLMarketBridgeBook ToBridge(const HOLMarket::FBook& Book);
	static HOLMarket::FBook FromBridge(const FHOLMarketBridgeBook& Book);

	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

protected:
	virtual bool DoesSupportWorldType(const EWorldType::Type WorldType) const override;

	/** The book after Intervals intervals toward rest. The default implementation is the C++ rule HOLMarket::Recover. */
	UFUNCTION(BlueprintNativeEvent, Category = "HeroesOfLiberty|Market")
	FHOLMarketBridgeBook RecoverPrices(const FHOLMarketBridgeSettings& Settings, const FHOLMarketBridgeBook& Book, int32 Intervals);
	virtual FHOLMarketBridgeBook RecoverPrices_Implementation(const FHOLMarketBridgeSettings& Settings, const FHOLMarketBridgeBook& Book, int32 Intervals);

	/** What reaches the receiver of Sent at TaxPercent. The default implementation is the C++ rule HOLTribute::Received. */
	UFUNCTION(BlueprintNativeEvent, Category = "HeroesOfLiberty|Market")
	int32 TributeReceived(int32 Sent, int32 TaxPercent);
	virtual int32 TributeReceived_Implementation(int32 Sent, int32 TaxPercent);
};