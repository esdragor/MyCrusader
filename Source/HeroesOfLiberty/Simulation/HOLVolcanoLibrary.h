// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "HOLVolcanoLibrary.generated.h"

/**
 * What the HUD (V10, script) reads of one volcano site: its phase, the alert's countdown and the altar (D380). Whole numbers only, the
 * rules and the texts are the script's (Script/UI/HOLVolcanoBanner.as). Phase: EHOLVolcanoPhase as an integer (0 Dormant, 1 Armed,
 * 2 Warning, 3 Erupting, 4 Cooling, 5 Rest). AltarHold: EHOLAltarHold (0 nobody in the zone, 1 held, 2 contested).
 */
USTRUCT(BlueprintType)
struct HEROESOFLIBERTY_API FHOLVolcanoSiteView
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadWrite, Category = "HeroesOfLiberty|Volcano")
	int32 Phase = 0;

	/** Whole seconds left before the lava starts, during the alert only; -1 otherwise. */
	UPROPERTY(BlueprintReadWrite, Category = "HeroesOfLiberty|Volcano")
	int32 SecondsToErupt = -1;

	/** The site has an altar. */
	UPROPERTY(BlueprintReadWrite, Category = "HeroesOfLiberty|Volcano")
	bool bAltarPresent = false;

	/** The altar can be held now (the site is Armed and not woken). */
	UPROPERTY(BlueprintReadWrite, Category = "HeroesOfLiberty|Volcano")
	bool bAltarLit = false;

	/** The altar's count as a percentage of the time to hold, 0 to 100. */
	UPROPERTY(BlueprintReadWrite, Category = "HeroesOfLiberty|Volcano")
	int32 AltarPercent = 0;

	/** The seconds still to hold, rounded up (the countdown at the altar). */
	UPROPERTY(BlueprintReadWrite, Category = "HeroesOfLiberty|Volcano")
	int32 AltarSecondsLeft = 0;

	UPROPERTY(BlueprintReadWrite, Category = "HeroesOfLiberty|Volcano")
	int32 AltarHold = 0;

	/** This cycle's alert was begun by the altar held. */
	UPROPERTY(BlueprintReadWrite, Category = "HeroesOfLiberty|Volcano")
	bool bWoken = false;
};

/** The volcano as the HUD sees it (V10). Server and solo only: a client has no site (the map gives them to the server), so its list is empty. */
UCLASS()
class HEROESOFLIBERTY_API UHOLVolcanoLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** How many sites the match has now (0 without Catastrophe, on a client or in a map without site). */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Volcano", meta = (WorldContext = "WorldContextObject"))
	static int32 GetSiteCount(const UObject* WorldContextObject);

	/** The site's view now. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Volcano", meta = (WorldContext = "WorldContextObject"))
	static FHOLVolcanoSiteView GetSiteView(const UObject* WorldContextObject, int32 SiteIndex);
};
