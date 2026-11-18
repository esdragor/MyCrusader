// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "HOLSiteBoardTypes.generated.h"

/**
 * One sacred site (S2; GDD 14.3, D429 to D431, D697 to D703): its owner (a slot), who is filling its count and the count of steps held
 * toward its capture. A USTRUCT since MIG-CPP-Site (D836 point 3) so that the script board (Script/Sites/HOLSiteBoardScript.as) takes and gives
 * it; the fields are the same as before (HOLSiteRules.h includes this header). The rule is HOLSite::Step of
 * Script/Sites/HOLSiteRules.as.
 */
USTRUCT(BlueprintType)
struct HEROESOFLIBERTY_API FHOLSiteState
{
	GENERATED_BODY()

	/** The owner's slot, INDEX_NONE for nobody. */
	UPROPERTY(BlueprintReadWrite, Category = "HeroesOfLiberty|Site")
	int32 Owner = INDEX_NONE;
	/** The slot with the most takers among the adverse camp filling the count, INDEX_NONE while nobody fills it. */
	UPROPERTY(BlueprintReadWrite, Category = "HeroesOfLiberty|Site")
	int32 Taker = INDEX_NONE;
	/** Steps held toward the capture. */
	UPROPERTY(BlueprintReadWrite, Category = "HeroesOfLiberty|Site")
	int32 Held = 0;

	bool operator==(const FHOLSiteState& Other) const { return Owner == Other.Owner && Taker == Other.Taker && Held == Other.Held; }
};

/**
 * S10 (D431, D703): the victory count of the sacred sites (HOLSiteVictory::FHOLSiteVictoryState of Script/Sites/HOLSiteVictory.as).
 * Leader: the camp holding every site now (INDEX_NONE for none); Held: the steps it held them; Winner: the camp once the count is full (it stays).
 */
USTRUCT(BlueprintType)
struct HEROESOFLIBERTY_API FHOLSiteVictoryState
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "HeroesOfLiberty|Site")
	int32 Leader = INDEX_NONE;
	UPROPERTY(BlueprintReadWrite, Category = "HeroesOfLiberty|Site")
	int32 Held = 0;
	UPROPERTY(BlueprintReadWrite, Category = "HeroesOfLiberty|Site")
	int32 Winner = INDEX_NONE;
	/** The owners of the sites at the last step, to see a site change hands. */
	UPROPERTY(BlueprintReadWrite, Category = "HeroesOfLiberty|Site")
	TArray<int32> LastOwners;
};
