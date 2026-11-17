// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Player/HOLGameMenuRules.h"
#include "UObject/Object.h"
#include "HOLGameMenuRulesBridge.generated.h"

/**
 * MIG-C-GameMenu (HALDIR-1117h, D836: the menu's rules go to script): the C++ callers' read of Script/UI/HOLGameMenuRules.as and
 * Script/UI/HOLEndBannerScript.as, ENVELOPES ONLY (no rule here: a call each, enumerations as the script's int values). The rules'
 * single source is the script class deriving from this one (Script/UI/HOLGameMenuRulesBridge.as); without it (no script compiled)
 * every call logs a warning and answers its neutral value (nothing offered, no banner, Escape opens the menu).
 */
UCLASS(Abstract)
class HEROESOFLIBERTY_API UHOLGameMenuRulesBridge : public UObject
{
	GENERATED_BODY()

public:
	/** The script's instance (its class's default object), nullptr without a script class. Hot reload keeps older classes: the newest is looked up again. */
	UFUNCTION(BlueprintCallable, Category = "HOL|GameMenu")
	static UHOLGameMenuRulesBridge* Get();

	// The callers' typed face, the old HOLGameMenuRules:: signatures. Named Native* so the script overrides (BP_X seen as X) are not shadowed when script calls them.
	static EHOLEscapeAction NativeGetEscapeAction(bool bMenuOpen, bool bModeArmed, bool bHasSelection);
	static bool NativeCanSurrender(bool bDefeated, bool bMatchOver);
	static bool NativeCanQuitToMenu(bool bDefeated, bool bMatchOver);
	static EHOLEndBanner NativeGetEndBanner(int32 MySlot, uint8 DefeatedMask, bool bMatchOver, uint8 WinnerMask);
	static bool NativeOffersReturnToLobby(EHOLEndBanner Banner, EHOLMatchOrigin Origin);
	static bool NativeQuitsToList(EHOLMatchOrigin Origin);
	static bool NativeShouldOpenEndBanner(EHOLEndBanner Due, EHOLEndBanner Shown, EHOLEndBanner Closed);
	static bool NativeKeepsMatchMenuInput(bool bOpening, bool bOtherScreenOpen);
	static bool NativeShowsStatsButton(bool bSpectator, bool bMatchOver);

	// The script overrides (Angelscript drops the BP_ prefix). Values of EHOLEscapeAction, EHOLEndBanner, EHOLMatchOrigin (the script's EHOLGameMenuEscape, EHOLGameMenuBanner, EHOLGameMenuOrigin).
	UFUNCTION(BlueprintImplementableEvent, Category = "HOL|GameMenu")
	int32 BP_GetEscapeAction(bool bMenuOpen, bool bModeArmed, bool bHasSelection);

	UFUNCTION(BlueprintImplementableEvent, Category = "HOL|GameMenu")
	bool BP_CanSurrender(bool bDefeated, bool bMatchOver);

	UFUNCTION(BlueprintImplementableEvent, Category = "HOL|GameMenu")
	bool BP_CanQuitToMenu(bool bDefeated, bool bMatchOver);

	/** Masks as int32 (script has no uint8 parameter worth the cast). */
	UFUNCTION(BlueprintImplementableEvent, Category = "HOL|GameMenu")
	int32 BP_GetEndBanner(int32 MySlot, int32 DefeatedMask, bool bMatchOver, int32 WinnerMask);

	UFUNCTION(BlueprintImplementableEvent, Category = "HOL|GameMenu")
	bool BP_OffersReturnToLobby(int32 Banner, int32 Origin);

	UFUNCTION(BlueprintImplementableEvent, Category = "HOL|GameMenu")
	bool BP_QuitsToList(int32 Origin);

	UFUNCTION(BlueprintImplementableEvent, Category = "HOL|GameMenu")
	bool BP_ShouldOpenEndBanner(int32 Due, int32 Shown, int32 Closed);

	UFUNCTION(BlueprintImplementableEvent, Category = "HOL|GameMenu")
	bool BP_KeepsMatchMenuInput(bool bOpening, bool bOtherScreenOpen);

	UFUNCTION(BlueprintImplementableEvent, Category = "HOL|GameMenu")
	bool BP_ShowsStatsButton(bool bSpectator, bool bMatchOver);
};
