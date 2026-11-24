// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "HOLSpecializationReadLibrary.generated.h"

/**
 * ELROND-1123pf (D839: a binding to expose): what the script reads of the specialization board that is not a UFUNCTION of the board itself.
 * Script namespace: HOLSpecializationRead. A library over UHOLSpecializationSubsystem (no central header touched).
 */
UCLASS()
class HEROESOFLIBERTY_API UHOLSpecializationReadLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/**
	 * Whether Slot's seat has its complete Forum: the very calculation the server refuses an unlock with (UHOLSpecializationSubsystem::IsForumComplete,
	 * NoForumComplete): a standard seat is always true, a Nomad seat once its first Forum is complete (the bits replicated to every machine). False
	 * without a world or for a negative slot. The SP3 button reads it (HOLSpecializationSlot::ReadView).
	 */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Specialization", meta = (WorldContext = "WorldContextObject"))
	static bool IsForumCompleteForSlot(const UObject* WorldContextObject, int32 Slot);

	/**
	 * GLORFINDEL-1124b (ELROND-1124b point 3, D893): the option the SEAT of Slot applied at Tier (1 to 3), read on its AHOLPlayerState's specialization view
	 * (GetPlayerIndex is no UFUNCTION: the seat -> player state step lives here, no central header touched). -1 for no option yet, a tier out of range, a
	 * slot without a player state (an AI slot) and without a world. The view is owner-only: a client reads its own seat, -1 for the others.
	 */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Specialization", meta = (WorldContext = "WorldContextObject"))
	static int32 GetChosenOptionForSlot(const UObject* WorldContextObject, int32 Slot, int32 Tier);
};
