// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "HOLSiteRulesLibrary.generated.h"

/**
 * The simulation's sacred-site rules (HOLSite::Step and HOLSite::AddUnit, C++) for Angelscript (namespace HOLSiteRules there): only so that
 * the script's own rules, Script/Sites/HOLSiteRules.as (namespace HOLSite), are checked against them (Test_SiteScriptAndSimulationRulesAgree).
 */
UCLASS()
class HEROESOFLIBERTY_API UHOLSiteRulesLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * HOLSite::Step on one site fed through HOLSite::AddUnit: its state in, its state out. The units are parallel arrays (the slot of each
	 * living unit in the zone, 1 when it can take, 1 when it is a Peasant); the slots are TeamBySlot's.
	 */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Site")
	static void StepSite(int32 Owner, int32 Taker, int32 Held, const TArray<int32>& UnitSlots, const TArray<int32>& UnitCanTake,
		const TArray<int32>& UnitIsPeasant, const TArray<int32>& TeamBySlot, int32 HoldSteps, int32 DecaySteps, int32& OutOwner, int32& OutTaker,
		int32& OutHeld);

	/** The counts HOLSite::AddUnit makes of the same parallel arrays, for Slots slots. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Site")
	static void CountSiteUnits(int32 Slots, const TArray<int32>& UnitSlots, const TArray<int32>& UnitCanTake, const TArray<int32>& UnitIsPeasant,
		TArray<int32>& OutTakers, TArray<int32>& OutDisturbers);

	/** The provisional values of the simulation's rules (HOLSite), so that the script's are checked against them. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Site")
	static int32 GetHoldSeconds();
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Site")
	static int32 GetDecaySteps();

	/**
	 * S10 (D431): the victory count of the sacred sites as this machine knows it (the game state's replicated count, so the same on a client and on the
	 * server): the slot standing for the camp in the lead (-1 nobody), the whole seconds left (-1 without a leader), frozen by an adverse unit, the winner (-1 none).
	 */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Site", meta = (WorldContext = "WorldContextObject"))
	static void GetVictoryCount(const UObject* WorldContextObject, int32& OutLeader, int32& OutSecondsLeft, bool& bOutFrozen, int32& OutWinner);

	/**
	 * S10: HOLSite::VictoryStep (the simulation's victory count) for the script, so that Script/Sites/HOLSiteVictory.as is checked against it
	 * (Test_SiteVictoryScriptAndSimulationAgree): the state in (leader, steps held, winner, owners at the last step), the step's inputs, the state out.
	 */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Site")
	static void VictoryStepSite(int32 Leader, int32 Held, int32 Winner, const TArray<int32>& LastOwners, const TArray<int32>& OwnerBySite,
		const TArray<int32>& ContestedBySite, const TArray<int32>& TeamBySlot, int32 VictorySteps, int32& OutLeader, int32& OutHeld, int32& OutWinner,
		TArray<int32>& OutLastOwners);
};
