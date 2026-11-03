// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "HOLVolcanoView.generated.h"

/**
 * V9a: what every machine knows of a volcano site (GDD 12, D377 to D380): its phase, the flow drawn for the cycle, how far the front has run, the
 * crater and the altar. Public (the alert is seen by all, V-Q3 = A); replicated by the game state like the relays' views (R5): the server writes
 * it each step from the site's real state (UHOLVolcanoSubsystem), a client reads it where the server reads the subsystem. The sites themselves
 * (the map's actor) are not replicated: the drawing of V9b and V9c will read this and nothing else.
 */
USTRUCT()
struct FHOLVolcanoSiteNet
{
	GENERATED_BODY()

	/** EHOLVolcanoPhase as a number (0 Dormant, 1 Armed, 2 Warning, 3 Erupting, 4 Cooling, 5 Rest). */
	UPROPERTY()
	uint8 Phase = 0;

	/** The cycle in progress. */
	UPROPERTY()
	int32 Cycle = 0;

	/** The flow drawn for the cycle (an index of the site's flows), INDEX_NONE for a site that sleeps. */
	UPROPERTY()
	int32 FlowIndex = INDEX_NONE;

	/** Whole seconds left before the lava starts, during the alert only; -1 otherwise. */
	UPROPERTY()
	int32 SecondsToErupt = -1;

	/** The front along the flow, cm (0 before the eruption, the flow's length once it has arrived). */
	UPROPERTY()
	int32 FrontDistance = 0;

	/** The crater (the first point of the first flow), cm. */
	UPROPERTY()
	float CraterX = 0.f;

	UPROPERTY()
	float CraterY = 0.f;

	/** The length of the drawn flow, cm (0 when none). */
	UPROPERTY()
	int32 FlowLength = 0;

	/** The altar: present, lit now, woken this cycle, place, zone radius, count in percent, seconds left, what its zone does (EHOLAltarHold). */
	UPROPERTY()
	bool bAltarPresent = false;

	UPROPERTY()
	bool bAltarLit = false;

	UPROPERTY()
	bool bWoken = false;

	UPROPERTY()
	float AltarX = 0.f;

	UPROPERTY()
	float AltarY = 0.f;

	UPROPERTY()
	float AltarRadius = 0.f;

	UPROPERTY()
	uint8 AltarPercent = 0;

	UPROPERTY()
	int32 AltarSecondsLeft = 0;

	UPROPERTY()
	uint8 AltarHold = 0;

	bool operator==(const FHOLVolcanoSiteNet& Other) const
	{
		return Phase == Other.Phase && Cycle == Other.Cycle && FlowIndex == Other.FlowIndex && SecondsToErupt == Other.SecondsToErupt && FrontDistance == Other.FrontDistance
			&& CraterX == Other.CraterX && CraterY == Other.CraterY && FlowLength == Other.FlowLength && bAltarPresent == Other.bAltarPresent && bAltarLit == Other.bAltarLit
			&& bWoken == Other.bWoken && AltarX == Other.AltarX && AltarY == Other.AltarY && AltarRadius == Other.AltarRadius && AltarPercent == Other.AltarPercent
			&& AltarSecondsLeft == Other.AltarSecondsLeft && AltarHold == Other.AltarHold;
	}

	/** "phase 2 cycle 0 flow 1 (6000 cm) front 0 to-erupt 57 s crater (1500, -2500) altar lit 1 woken 0 (3000, 100) r1200 held 0% (60 s left) hold 0": the report's line. */
	FString Describe() const
	{
		return FString::Printf(TEXT("phase %d cycle %d flow %d (%d cm) front %d cm to-erupt %d s crater (%.0f, %.0f) altar present %d lit %d woken %d (%.0f, %.0f) r%.0f held %d%% (%d s left) hold %d"),
			static_cast<int32>(Phase), Cycle, FlowIndex, FlowLength, FrontDistance, SecondsToErupt, CraterX, CraterY, bAltarPresent ? 1 : 0, bAltarLit ? 1 : 0, bWoken ? 1 : 0,
			AltarX, AltarY, AltarRadius, static_cast<int32>(AltarPercent), AltarSecondsLeft, static_cast<int32>(AltarHold));
	}
};
