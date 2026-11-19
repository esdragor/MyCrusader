// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Kismet/BlueprintFunctionLibrary.h"
#include "HOLWaitBannerReadLibrary.generated.h"

/**
 * H-1 (the wait and absence banners, Script\UI\HOLWaitBanner.as; HOLWaitBannerRead:: in script): what the game state says of the wait for named
 * players (R3.10) and of the disconnection notices (R7.5), in simple types. The rules and the view are in script; the localized absence lines
 * (AHOLGameState::GetAbsenceBannerLines, shared with the server) stay in C++ and come back here already formatted.
 */
UCLASS()
class HEROESOFLIBERTY_API UHOLWaitBannerReadLibrary : public UBlueprintFunctionLibrary
{
	GENERATED_BODY()

public:
	/** R3.10: the game state's wait for named players. True: waiting. OutMissing: the names still missing; OutSecondsLeft: whole seconds before the wait's end by the server's clock, rounded up, never under 0. False and empty without a game state. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Match", meta = (WorldContext = "WorldContextObject"))
	static bool GetWaitState(const UObject* WorldContextObject, TArray<FString>& OutMissing, int32& OutSecondsLeft);

	/** R7.5: the disconnection banner's lines at the server's time (AHOLGameState::GetAbsenceBannerLines, formatted FText::ToString) and ALL the notices (OutNames, OutBack = BackAt > 0), parallel arrays. Empty without a game state. */
	UFUNCTION(BlueprintPure, Category = "HeroesOfLiberty|Match", meta = (WorldContext = "WorldContextObject"))
	static void GetAbsenceState(const UObject* WorldContextObject, TArray<FString>& OutLines, TArray<FString>& OutNames, TArray<bool>& OutBack);
};
