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
};
