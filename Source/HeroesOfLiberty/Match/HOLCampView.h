// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "HOLCampView.generated.h"

/**
 * N7 (NeutralsInProgress.md; Retours\Q-neutrals-camp-indicator.md answered 2026-11-11: Q1 C, Q2 B): what one player knows of a neutral
 * camp: its number, its post, alive or cleared, its living members. Like FHOLSiteView, NOT public: the server (UHOLCampSubsystem) writes
 * to each player's state only the camps his camp has had in sight, read live while in sight and frozen as last seen out of it, and
 * replicates them to that player only; a camp never seen is not on his machine. The minimap's camp dots read this and nothing else.
 */
USTRUCT()
struct FHOLCampView
{
	GENERATED_BODY()

	/** The camp's id in the match (its index in UHOLCampSubsystem). */
	UPROPERTY()
	uint8 Index = 0;

	/** The post, cm. */
	UPROPERTY()
	float X = 0.f;

	UPROPERTY()
	float Y = 0.f;

	/** Cleared for good (as last seen). */
	UPROPERTY()
	bool bCleared = false;

	/** Members alive (as last seen), at most 255. */
	UPROPERTY()
	uint8 Living = 0;

	bool operator==(const FHOLCampView& Other) const
	{
		return Index == Other.Index && X == Other.X && Y == Other.Y && bCleared == Other.bCleared && Living == Other.Living;
	}
};
