// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "HOLAITechnologyLibrary.generated.h"

/**
 * One row of the technology table as the AI's rule reads it (T7, GDD 11): the three conditions (tier, building, cost) and what the
 * line asks first. Resources by name here, the script indexes them 0 food, 1 wood, 2 stone, 3 gold.
 */
USTRUCT(BlueprintType)
struct HEROESOFLIBERTY_API FHOLAITechnology
{
	GENERATED_BODY()

	/** The row name (Tools/Data/Technologies.csv): the name a research order carries. */
	UPROPERTY(BlueprintReadOnly, ScriptReadWrite, Category = "AI")
	FName Name;

	/** The building type that researches it. */
	UPROPERTY(BlueprintReadOnly, ScriptReadWrite, Category = "AI")
	FName Building;

	/** EHOLTechCategory as an integer: 0 economy, 1 military, 2 infrastructure, 3 hero, 4 faction. */
	UPROPERTY(BlueprintReadOnly, ScriptReadWrite, Category = "AI")
	int32 Category = 0;

	UPROPERTY(BlueprintReadOnly, ScriptReadWrite, Category = "AI")
	int32 MinTier = 1;

	UPROPERTY(BlueprintReadOnly, ScriptReadWrite, Category = "AI")
	int32 MinHeroLevel = 0;

	/** The technology to have first (level I of a line), none for a first level. */
	UPROPERTY(BlueprintReadOnly, ScriptReadWrite, Category = "AI")
	FName Requires;

	UPROPERTY(BlueprintReadOnly, ScriptReadWrite, Category = "AI")
	int32 CostFood = 0;

	UPROPERTY(BlueprintReadOnly, ScriptReadWrite, Category = "AI")
	int32 CostWood = 0;

	UPROPERTY(BlueprintReadOnly, ScriptReadWrite, Category = "AI")
	int32 CostStone = 0;

	UPROPERTY(BlueprintReadOnly, ScriptReadWrite, Category = "AI")
	int32 CostGold = 0;

	/** Some code applies its effect (the table's Implemented column, D601). */
	UPROPERTY(BlueprintReadOnly, ScriptReadWrite, Category = "AI")
	bool bImplemented = false;
};

/**
 * T7: what the AI's brain reads of the technologies, for Angelscript (namespace HOLAITechnology there). The research order itself is the
 * production order the AI already has (UHOLAIOrderLibrary::IssueAIProduceOrder with the technology's name at the building: a research is an
 * item of the same queue, T-Q6, D583).
 */
UCLASS()
class HEROESOFLIBERTY_API UHOLAITechnologyLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** The table's rows, in the table's order (false: no table). Needs no world: the same rows the simulation reads. */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|AI")
	static bool ReadAITechnologyTable(TArray<FHOLAITechnology>& OutRows);

	/** The technologies PlayerIndex has researched (the server's player state), in the order they were finished. */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|AI", meta = (WorldContext = "WorldContextObject"))
	static void ReadAITechnologiesDone(const UObject* WorldContextObject, int32 PlayerIndex, TArray<FName>& OutDone);

	/**
	 * D648: the game seconds before the AI of a level (0 easy, 1 normal, 2 hard, as played now: an Evolving AI is resolved by the caller)
	 * researches its first technology. The console variables hol.AI.TechDelayEasy / TechDelayNormal / TechDelayHard (840, 360, 0: the user's answer A, D657).
	 */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|AI")
	static float GetAITechDelay(int32 Level);
};
