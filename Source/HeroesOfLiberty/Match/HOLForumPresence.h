// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Match/HOLForumRules.h"
#include "Match/HOLGameState.h"

class UHOLUnitSubsystem;

/**
 * V3.2 (D348, V3-Q1, V3-Q2): who stands in the zone of each main forum down, from the living units (not dying, health
 * above 0; buildings and bodies never count) within FHOLForumTuning::HoldRadius of the forum's edge (the distance to its
 * body, not to its middle). By team: a unit of the forum's owner or of an ally defends; any other (an adversary, a
 * Neutral owner, D401) attacks. AttackerSlot: the attacking owner with the most units in the zone, the lowest on a tie.
 *
 * Cost: nothing while every forum stands; else one walk of the units per simulation step for all the forums down (the
 * first ask of a step counts them all, the others read it).
 */
class HEROESOFLIBERTY_API FHOLForumPresenceCounter
{
public:
	/** The provider the server gives the game state at the launch (AHOLGameState::SetForumPresenceProvider). */
	static AHOLGameState::FForumPresenceProvider MakeProvider(const TSharedRef<FHOLForumPresenceCounter>& Counter);

	/** Slot's presence this step (AHOLGameState::GetCurrentStep). */
	FHOLForumPresence Get(const AHOLGameState& GameState, int32 Slot);

	/** The cost so far: walks of the units, units walked (the dying included). */
	int32 GetWalks() const { return Walks; }
	int64 GetUnitsWalked() const { return UnitsWalked; }

private:
	void Count(const AHOLGameState& GameState);

	int64 CountedStep = -1;
	TArray<FHOLForumPresence> Presences;
	int32 Walks = 0;
	int64 UnitsWalked = 0;
};
