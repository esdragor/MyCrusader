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

/**
 * One lit altar as the AI reads it for its slot (V11, D639, D640): where, how wide, whether an adversary stands in its zone and how strong, and how far
 * the nearest possible flow's line is from the AI's positions (which positions: the console variable hol.AI.AltarTraceFrom, Q-ia-autel-distance).
 */
USTRUCT(BlueprintType)
struct HEROESOFLIBERTY_API FHOLAIAltar
{
	GENERATED_BODY()

	UPROPERTY(BlueprintReadOnly, ScriptReadWrite, Category = "Volcano")
	int32 SiteIndex = INDEX_NONE;

	UPROPERTY(BlueprintReadOnly, ScriptReadWrite, Category = "Volcano")
	FVector Location = FVector::ZeroVector;

	/** The zone's radius, cm. */
	UPROPERTY(BlueprintReadOnly, ScriptReadWrite, Category = "Volcano")
	float Radius = 1200.f;

	/** A living unit of another camp (a neutral too) stands in the zone. */
	UPROPERTY(BlueprintReadOnly, ScriptReadWrite, Category = "Volcano")
	bool bAdversary = false;

	/** The adversary's strength in the zone: one per soldier, a Peasant counting less (FHOLVolcanoSettings::AiAltarWorkerWeight). */
	UPROPERTY(BlueprintReadOnly, ScriptReadWrite, Category = "Volcano")
	float AdversaryStrength = 0.f;

	/** The distance (cm) from the AI's positions to the nearest possible flow's line. */
	UPROPERTY(BlueprintReadOnly, ScriptReadWrite, Category = "Volcano")
	float TraceDistance = 0.f;
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

	/**
	 * V11: the lit altars of the match as slot PlayerIndex measures them (server and solo; empty on a client, without site, without Catastrophe,
	 * or when every altar is held and woken).
	 */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|Volcano", meta = (WorldContext = "WorldContextObject"))
	static void ReadAIAltars(const UObject* WorldContextObject, int32 PlayerIndex, TArray<FHOLAIAltar>& OutAltars);

	/** Q-ia-autel-diversion = B: the console variable hol.AI.AltarStrikeBack (default on): Hard strikes the bare camp of an adversary holding the altar with a big army. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Volcano")
	static bool GetAIAltarStrikeBack();

	/**
	 * V11: the size of the squad the AI keeps at an altar: the contest rule when an adversary stands in the zone (HOLVolcano::AltarSend, D639), the
	 * hold of a free altar otherwise (HOLVolcano::AltarHoldSquad, D640). Level: 0 Easy, 1 Normal, 2 Hard. The simulation's own settings.
	 */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Volcano", meta = (WorldContext = "WorldContextObject"))
	static void DecideAltarSquad(const UObject* WorldContextObject, const FHOLAIAltar& Altar, int32 FreeSoldiers, int32 AlreadySent, int32 Level, int32& OutSquad, bool& bOutDiversion);
};
