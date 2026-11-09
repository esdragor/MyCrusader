// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "HOLSiteCount.generated.h"

/**
 * S10 (D431, D699, D703, D747): the victory count of the sacred sites as EVERY machine knows it (public, like the volcano's alert: the count is shown to
 * all, SacredSitesInProgress.md S10). The server writes it each step from the real count (UHOLSiteSubsystem::StepVictory), only when it changed (the
 * seconds are whole), a client reads it. Leader: the slot that stands for the camp holding every site now (INDEX_NONE: nobody), SecondsLeft: whole
 * seconds before the victory (-1 without a leader, 0 once won), bFrozen: an adverse unit stands in a zone (the count waits), Winner: the camp's slot once won.
 */
USTRUCT()
struct FHOLSiteCountNet
{
	GENERATED_BODY()

	UPROPERTY()
	int32 Leader = INDEX_NONE;

	UPROPERTY()
	int32 SecondsLeft = -1;

	UPROPERTY()
	bool bFrozen = false;

	UPROPERTY()
	int32 Winner = INDEX_NONE;

	bool operator==(const FHOLSiteCountNet& Other) const
	{
		return Leader == Other.Leader && SecondsLeft == Other.SecondsLeft && bFrozen == Other.bFrozen && Winner == Other.Winner;
	}

	bool operator!=(const FHOLSiteCountNet& Other) const { return !(*this == Other); }

	/** "leader 0 left 580 s frozen 0 winner -1": the report's line. */
	FString Describe() const
	{
		return FString::Printf(TEXT("leader %d left %d s frozen %d winner %d"), Leader, SecondsLeft, bFrozen ? 1 : 0, Winner);
	}
};
