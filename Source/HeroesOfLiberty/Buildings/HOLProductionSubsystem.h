// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Buildings/HOLProductionRules.h"
#include "Match/HOLMarketRules.h"
#include "Subsystems/WorldSubsystem.h"
#include "HOLProductionSubsystem.generated.h"

class AHOLBuilding;
class AHOLPlayerState;
struct FHOLProductionCommand;
struct FHOLUnitTypeRow;

/**
 * Production of units at the buildings (J3, D282), on the server (or a solo match) only, at the start of each
 * simulation step: applies the production orders of the order queue (owner, producible type, queue length and cost
 * checked by the rules of HOLProductionRules), counts each player's population (its units and the units in
 * production) and cap (its buildings), advances every queue, and asks the unit subsystem for the units that are done,
 * in an arc below their building as seen from the camera, until the rally point of J4 (D283, specified in J3). The
 * births asked at the start of a step are served by that same step.
 */
UCLASS()
class HEROESOFLIBERTY_API UHOLProductionSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	/** Simulation step to show the progress at, with its fraction: the server's clock as estimated by a client (D271), the simulation's own in solo. */
	static double GetDisplayStep(const UWorld* World);

	/** Production time of a unit type in simulation steps (its table time at the step rate), at least 1. */
	static int32 GetBuildSteps(const FHOLUnitTypeRow& Type, float StepSeconds);

	/**
	 * What producing one unit of TypeName takes (its row of the unit type table); a missing type costs nothing and takes 1 step. PlayerIndex: the
	 * player who pays, whose technologies lower the cost (the Vow of Poverty: the Monk, MonkCostDiscount); INDEX_NONE: the table's.
	 */
	FHOLProductionCost GetCost(FName TypeName, int32 PlayerIndex = INDEX_NONE) const;

	/** Player state of a slot, or null (an AI slot has none yet: no economy). */
	AHOLPlayerState* FindPlayerState(int32 PlayerIndex) const;

	/** Development cheat (never in a Shipping build): places added to a player's cap, still at most 175 (D03). */
	void AddCheatPopulation(int32 PlayerIndex, int32 Places);

	virtual void OnWorldBeginPlay(UWorld& InWorld) override;
	virtual void Deinitialize() override;

protected:
	virtual bool DoesSupportWorldType(const EWorldType::Type WorldType) const override;

private:
	void OnStepStarted(int64 StepIndex);
	void ApplyOrders();
	void AdvanceQueues(int64 StepIndex);
	/** M4c: one Trade order (owner, finished Market, resource, lots checked here; the price and the stock by HOLMarket::Apply). */
	void ApplyMarketTrade(const FHOLProductionCommand& Order, const AHOLBuilding& Building, AHOLPlayerState& PlayerState);
	/** D372: one Tribute order (owner and finished Market checked by ApplyOrders; the receiver, the resource and the stock here). */
	void ApplyTribute(const FHOLProductionCommand& Order, const AHOLBuilding& Building, AHOLPlayerState& Giver);
	/** M4c: the common book one step nearer to rest, one HOLMarket::Recover every RecoverEverySeconds. */
	void RecoverMarketBook();
	/** The Standard row of the Market table, read once at the first use. */
	bool EnsureMarketSettings();
	/** Where the Index-th unit born at Building stands: an arc below it on the screen (the camera's side). */
	FVector GetBirthLocation(const AHOLBuilding& Building, int32 Index) const;

	FDelegateHandle StepStartedHandle;
	float StepSeconds = 0.05f;
	/** Units born at each building so far: spreads them along the arc. */
	TMap<TWeakObjectPtr<AHOLBuilding>, int32> NumBirths;
	/** Places each player's cheats added to its cap (AddCheatPopulation). */
	TMap<int32, int32> CheatPopulation;
	/** M4c: the Market's numbers (loaded at the first trade or return to rest), and the steps since the book last moved back. */
	HOLMarket::FSettings MarketSettings;
	bool bMarketSettingsLoaded = false;
	int32 StepsSinceMarketRecover = 0;
};
