// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "HOLSiegeCheckLibrary.generated.h"

/**
 * The simulation's rule of the siege engines (HOLSiege, Siege/HOLSiegeRules.h, C++, the one the server applies) for Angelscript
 * (namespace HOLSiegeCheck there): D836 (2) keeps the rule in C++ and the AI in script reads it here, so that its own copy,
 * Script/Siege/HOLSiegeRules.as, can go (socle step 1 of the migration, D830). Wrappers only: no rule here. The numbers are the
 * script's: engine 0 Ram, 1 Mangonel, 2 Trebuchet, 3 Cannon (EHOLSiegeEngine); block 0 none, 1 not in the prototype, 2 tier too low,
 * 3 no workshop, 4 cannot pay (HOLSiege::Available...).
 */
UCLASS()
class HEROESOFLIBERTY_API UHOLSiegeCheckLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** HOLSiege::Check: why the engine cannot be queued now (block number above), 0 when nothing stops it; -1 for an engine out of range. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Siege")
	static int32 Check(int32 Engine, int32 HeroTier, bool bWorkshopFinished, bool bCanPay);

	/** HOLSiege::FindEngine: the engine a unit type is, by the name of its row; false (OutEngine -1) for any other type. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Siege")
	static bool FindEngine(FName TypeName, int32& OutEngine);

	/** HOLSiege::DescribeBlock: the words of a block in the production's log ("tier too low"). */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Siege")
	static FString DescribeBlock(int32 Block);

	/** HOLSiege::GetTier: the tier an engine opens at; 0 for an engine out of range. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Siege")
	static int32 GetTier(int32 Engine);

	/** HOLSiege::InPrototype: the Ram and the Mangonel (D77); false for an engine out of range. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Siege")
	static bool InPrototype(int32 Engine);

	/** HOLSiege::GetHealthStat: the technology statistic that raises this type's health, NAME_None for any other type. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Siege")
	static FName GetHealthStat(FName TypeName);

	/** HOLSiege::GetDamageStat: the technology statistic that raises this type's damage, NAME_None for any other type. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Siege")
	static FName GetDamageStat(FName TypeName);
};
