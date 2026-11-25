// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "HOLGridFaceBridge.generated.h"

/**
 * G-5b (SAURON-1125md, D839 binding to expose to script): the answer of the script's cell-face rule (HOLCommandGridFace::GetCellFace,
 * Script/Player/HOLCommandGridFace.as) to the C++ caller (UHOLGameScreen::RefreshGrid). ENVELOPE ONLY: the script's own type is
 * FHOLGridCellLookView (a script struct, not visible from C++), the script bridge copies it into this one.
 * State values (HOLGridFaceState:: below, same order as the script's EHOLGridFaceState): 0 Ready, 1 Greyed.
 */
USTRUCT(BlueprintType)
struct HEROESOFLIBERTY_API FHOLGridFaceAnswer
{
	GENERATED_BODY()

	/** The label drawn on the cell (with the seconds left under it when cooling down). */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "HOL|GridFace")
	FText Label;
	/** HOLGridFaceState value. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "HOL|GridFace")
	int32 State = 0;
	/** The tooltip's title. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "HOL|GridFace")
	FText TooltipName;
	/** The tooltip's effect line. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "HOL|GridFace")
	FText TooltipEffect;
	/** The report name of the cell (journal / hol.HUD.Report). */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "HOL|GridFace")
	FName ReportName;
};

namespace HOLGridFaceState
{
	constexpr int32 Ready = 0;
	constexpr int32 Greyed = 1;
}

/**
 * The C++ caller's read of the script's cell-face rule (model: UHOLGridActionBridge, UHOLGameMenuRulesBridge). No rule here: one call.
 * The rule's single source is the script class deriving from this one (Script/Player/HOLGridFaceBridge.as). Without a script class the
 * call answers false (the caller leaves the cell empty: no C++ twin of the rule).
 */
UCLASS(Abstract)
class HEROESOFLIBERTY_API UHOLGridFaceBridge : public UObject
{
	GENERATED_BODY()

public:
	/** The script's instance (its class's default object), nullptr without a script class. Hot reload keeps older classes: the newest is looked up again. */
	UFUNCTION(BlueprintCallable, Category = "HOL|GridFace")
	static UHOLGridFaceBridge* Get();

	/**
	 * True when a script class answered (OutAnswer is then its answer). Command = EHOLGridCommand, Ability = EHOLHeroAbility as ints.
	 * CommandCooldownSeconds = the command's own seconds (hero ability); FactCooldownSeconds = the seconds left read from the game state
	 * (Seal: the slot's power cool-down, rage: the selection's); UnlockLevel = the hero level an ability needs, 0 when unlocked;
	 * Numbers = the tooltip's numbers (Seal: radius m, duration s, slow %; rage: duration s, +damage %, +rate %, HP per hit, refused under HP, stops at HP, recharge s).
	 * Named Native* so the script override (BP_X seen as X) is not shadowed.
	 */
	static bool NativeGetCellFace(int32 Command, int32 Ability, bool bGreyed, int32 CommandCooldownSeconds, int32 FactCooldownSeconds,
		int32 UnlockLevel, const TArray<int32>& Numbers, FHOLGridFaceAnswer& OutAnswer);

	/** The script override (Angelscript drops the BP_ prefix): GetCellFace. */
	UFUNCTION(BlueprintImplementableEvent, Category = "HOL|GridFace")
	FHOLGridFaceAnswer BP_GetCellFace(int32 Command, int32 Ability, bool bGreyed, int32 CommandCooldownSeconds, int32 FactCooldownSeconds,
		int32 UnlockLevel, const TArray<int32>& Numbers);
};
