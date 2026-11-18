// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "HOLSiteRulesLibrary.generated.h"

/**
 * Sacred-site values and the game state's count for Angelscript (namespace HOLSiteRules there). The rules themselves (the step, the victory count)
 * are the script's alone (Script/Sites/HOLSiteRules.as, HOLSiteVictory.as), reached by the server through UHOLSiteBoardSubsystem.
 */
UCLASS()
class HEROESOFLIBERTY_API UHOLSiteRulesLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
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
};
