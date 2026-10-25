// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"

struct FHOLMatchConfig;

/**
 * R3.9: when a match launched by the lobby manager (a configuration with names) starts: once every named player is
 * here, or at the end of the wait limit, whoever is missing. Pure rules, for the tests.
 */
struct HEROESOFLIBERTY_API FHOLStartRules
{
	/** The expected names missing from the present ones, in the expected order (trimmed, case ignored). */
	static TArray<FString> GetMissing(TConstArrayView<FString> Expected, TConstArrayView<FString> Present);

	/** Whether the match starts now: nobody missing, or the wait limit reached. */
	static bool ShouldStart(int32 NumMissing, double ElapsedSeconds, double MaxWaitSeconds);

	/** V5.1: the King's unit type (Tools/Data/UnitTypes.csv), produced nowhere, only born at the start of a Regicide match. */
	static const FName KingTypeName;

	/** V5.1 (Regicide, D206): the slot gets a King at the start: the option on, the slot occupied (a human or an AI). */
	static bool ShouldSpawnKing(const FHOLMatchConfig& Config, int32 Slot);

	/**
	 * V5.1: where the King is born, by its main forum (at CentreLocation, its body HalfBody cm from the centre to its edge):
	 * below it on the screen (-X, the side the births come out of, D254), one row further than the first births, so it stands
	 * beside them rather than on them.
	 */
	static FVector GetKingLocation(const FVector& CentreLocation, float HalfBody);

	/** V5.1: the King's distance from its forum's body edge (cm). */
	static constexpr float KingGap = 450.f;
};
