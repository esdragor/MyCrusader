// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Buildings/HOLProductionReadLibrary.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "HOLTechnologyCheckLibrary.generated.h"

/**
 * The simulation's rule of the technologies (HOLTechnology::Check, C++, the one the server applies) for Angelscript (namespace
 * HOLTechnologyCheck there): only so that the script's own rule, Script/Technologies/HOLTechnologyRules.as, is checked against it
 * (Test_TechnologyScriptAndSimulationRulesAgree). MIG2-S4 (D830, D839): also the reading wrappers of HOLTechnologyTableTests.cpp
 * (Playable, Rules) so that those two tests are written in script (Script/Tests/HOLTechnologyTable_Test.as); wrappers only,
 * HOLTechnology:: stays the one source of every rule.
 */
UCLASS()
class HEROESOFLIBERTY_API UHOLTechnologyCheckLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * HOLTechnology::Check on a row with these facts, as the script's numbers (HOLTechnologyRules.as): 0 available, 1 researched,
	 * 2 tier too low, 3 hero level too low, 4 requires missing, 5 no building, 6 cannot pay, 7 already queued, 8 not implemented.
	 */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Technology")
	static int32 Check(bool bImplemented, int32 HeroTier, int32 HeroLevel, int32 MinTier, int32 MinHeroLevel, bool bRequiresDone,
		bool bBuildingFinished, bool bCanPay, bool bResearched, bool bQueued);

	/** HOLTechnology::DescribeBlock of the script's reason number (as Check above): "soon", "tier too low"... */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Technology")
	static FString DescribeBlock(int32 Reason);

	/** HOLTechnology::IsPlayable of the table's row (its building is a type of BuildingTypes.csv AND its effect is coded); false for an unknown technology. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Technology")
	static bool IsPlayable(FName Technology);

	/** Every playable technology of the table (IsPlayable), in the table's order. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Technology")
	static TArray<FName> GetPlayableTechnologies();

	/** The MaxQueue of the building type that researches the technology (BuildingTypes.csv); -1 when the technology or its building type is not in a table. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Technology")
	static int32 GetResearchQueueSize(FName Technology);

	/** HOLTechnology::MakeCost of the table's row: the cost as an item of the production queue. False (outputs untouched) for an unknown technology. */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|Technology")
	static bool GetResearchCost(FName Technology, float StepSeconds, FHOLResources& OutCost, int32& OutBuildSteps, int32& OutPopulation);

	/** HOLTechnology::GetBonusPercent of the technologies Done on a statistic, the rows being the table's. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Technology")
	static int32 GetBonusPercent(const TArray<FName>& Done, FName Stat);

	/** HOLTechnology::ScaleByPercent. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Technology")
	static int32 ScaleByPercent(int32 Base, int32 Percent);

	/** HOLTechnology::GetAttackStat of an EHOLAttackType as an integer (0 Melee, 1 Ranged, 2 Siege, 3 Hero). */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Technology")
	static FName GetAttackStat(int32 AttackType);

	/** HOLTechnology::GetArmorStat of a mask of armour categories (bit i = EHOLArmorCategory i: 0 Light, 1 Heavy infantry, 2 Ranged, 3 Cavalry, 4 Siege, 5 Peasant, 6 Gunpowder, 7 Heavy, 8 King, 9 Hero). */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Technology")
	static FName GetArmorStat(int32 ArmorCategoryMask);

	/** HOLTechnology::ArmorCoversAttack of an EHOLAttackType as an integer. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Technology")
	static bool ArmorCoversAttack(int32 AttackType);

	/** HOLTechnology::IsQueued on the items of a script queue (its Types). */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Technology")
	static bool IsQueued(const FHOLProductionQueueState& Queue, FName Technology);

	/** FHOLTechnologyState::Has on a list of names. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Technology")
	static bool StateHas(const TArray<FName>& Done, FName Technology);

	/** FHOLTechnologyState::Add on a list of names: true when it was added (a technology is researched once). */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|Technology")
	static bool StateAdd(UPARAM(ref) TArray<FName>& Done, FName Technology);
};
