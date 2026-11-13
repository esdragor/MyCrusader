// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "HOLSpecializationView.generated.h"

/**
 * SP2 (SpecializationInProgress.md; GDD 13, D203, D684): what the owner of a seat reads of its specialization, copied by the server from
 * the script's FHOLSpecializationState (Script/Specialization/HOLSpecializationRules.as) onto AHOLPlayerState, replicated to the owner
 * only (the opponent reads nothing of an unlock in progress). The client draws it (SP3) and never decides.
 */
USTRUCT(BlueprintType)
struct FHOLSpecializationView
{
	GENERATED_BODY()

	/** The lowest tier whose unlock runs at the Forum (the bar's), 0 when none runs. */
	UPROPERTY(BlueprintReadOnly, Category = "HeroesOfLiberty|Specialization")
	int32 PendingTier = 0;

	/** The option of PendingTier that runs, -1 when none. */
	UPROPERTY(BlueprintReadOnly, Category = "HeroesOfLiberty|Specialization")
	int32 PendingOption = -1;

	/** The match time left before PendingTier's option applies (HOLSpecialization::RemainingMs), 0 when none runs. */
	UPROPERTY(BlueprintReadOnly, Category = "HeroesOfLiberty|Specialization")
	int32 RemainingMs = 0;

	/** Per tier (index = tier, 1 to 3; index 0 unused): the applied option, or -1. */
	UPROPERTY(BlueprintReadOnly, Category = "HeroesOfLiberty|Specialization")
	TArray<int32> Chosen;

	int32 GetChosen(const int32 Tier) const { return Chosen.IsValidIndex(Tier) ? Chosen[Tier] : -1; }

	bool operator==(const FHOLSpecializationView& Other) const
	{
		return PendingTier == Other.PendingTier && PendingOption == Other.PendingOption && RemainingMs == Other.RemainingMs && Chosen == Other.Chosen;
	}
};
