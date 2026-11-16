// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Match/HOLRelayView.h"

/**
 * One neutral relay (R3; GDD 9.4, D388, D348, D380): its owner (a slot), who is filling its count and the count of steps held toward its
 * capture. The rules (Step, TerritoryXP) are written and tested in Script/Relays/HOLRelayRules.as only (bascule Relay, D836 point 3: the script is the
 * source, asked by UHOLRelayBoard); this struct, FillPercent and MakeView stay C++ because they fill the replicated FHOLRelayView.
 */
struct FHOLRelayState
{
	/** The owner's slot, INDEX_NONE for nobody. */
	int32 Owner = INDEX_NONE;
	/** The slot with the most units among the adverse side filling the count, INDEX_NONE while nobody fills it. */
	int32 Taker = INDEX_NONE;
	/** Steps held toward the capture. */
	int32 Held = 0;

	bool operator==(const FHOLRelayState& Other) const { return Owner == Other.Owner && Taker == Other.Taker && Held == Other.Held; }
};

namespace HOLRelay
{
	/** Values the server holds (D388), the same as Script/Relays/HOLRelayRules.as (Test_RelayConstantsAgreeWithTheSimulation compares them): the capture time and the recession. The XP by the minute lives in the script only. */
	constexpr float DefaultHoldSeconds = 60.f;
	constexpr int32 DefaultDecaySteps = 2;

	/** R5: the share of the capture held, 0 to 99 (the ring is full only by the change of owner); 0 with no hold steps. */
	HEROESOFLIBERTY_API int32 FillPercent(const FHOLRelayState& State, int32 HoldSteps);

	/** R5: what the server replicates of a relay at Location with a zone of Radius cm. */
	HEROESOFLIBERTY_API FHOLRelayView MakeView(const FVector2D& Location, float Radius, const FHOLRelayState& State, int32 HoldSteps);
}
