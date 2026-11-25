// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "UObject/Object.h"
#include "HOLGridActionBridge.generated.h"

/**
 * G-4 (ELROND-1125ma, D839 binding to expose to script): the answer of the script's grid-action rule to the C++ caller
 * (AHOLPlayerController::ExecuteGridCommand). ENVELOPE ONLY, ints so that no C++ enum clashes with the script's own
 * EHOLGridActionKind / FHOLGridActionView (Script/Player/HOLCommandGridAction.as): the script bridge copies its view into this.
 * Kind values (HOLGridActionKind:: below, same order as the script's EHOLGridActionKind): 0 None, 1 Send, 2 Arm, 3 Place, 4 Refuse.
 */
USTRUCT(BlueprintType)
struct HEROESOFLIBERTY_API FHOLGridActionAnswer
{
	GENERATED_BODY()

	/** HOLGridActionKind value. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "HOL|GridAction")
	int32 Kind = 0;
	/** Arm: the armed mode (>= 0; faction power indices use 0.., the script picks the others) kept by AHOLPlayerController::SetArmedMode. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "HOL|GridAction")
	int32 AimMode = -1;
	/** Arm: the reach drawn around the hero while armed (cm). */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "HOL|GridAction")
	double RangeCm = 0.0;
	/** Arm: the unit that will cast (the Monk of the Seal), 0 none; read back with AHOLPlayerController::GetArmedUnitId. */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "HOL|GridAction")
	int32 UnitId = 0;
	/** The report name of the action (journal / hol.HUD.Report). */
	UPROPERTY(BlueprintReadWrite, EditAnywhere, Category = "HOL|GridAction")
	FName Report;
};

namespace HOLGridActionKind
{
	constexpr int32 None = 0;
	constexpr int32 Send = 1;
	constexpr int32 Arm = 2;
	constexpr int32 Place = 3;
	constexpr int32 Refuse = 4;
}

/**
 * The C++ callers' read of the script's grid-action rule (model: UHOLGameMenuRulesBridge). No rule here: one call. The rule's
 * single source is the script class deriving from this one (Script/Player/HOLCommandGridAction.as). Without a script class the
 * call answers false (the caller keeps its former path, nothing changes). Until G-5 the caller acts only on Kind == Arm; every
 * other answer leaves the existing switch of ExecuteGridCommand in charge.
 */
UCLASS(Abstract)
class HEROESOFLIBERTY_API UHOLGridActionBridge : public UObject
{
	GENERATED_BODY()

public:
	/** The script's instance (its class's default object), nullptr without a script class. Hot reload keeps older classes: the newest is looked up again. */
	UFUNCTION(BlueprintCallable, Category = "HOL|GridAction")
	static UHOLGridActionBridge* Get();

	/** True when a script class answered (OutAnswer is then its answer). Command = EHOLGridCommand, Ability = EHOLHeroAbility as ints; SelectedUnitIds = the own selection. Named Native* so the script override (BP_X seen as X) is not shadowed. */
	static bool NativeDecideGridAction(int32 Command, int32 Ability, bool bGreyed, int32 CooldownSeconds, FName TypeName, const TArray<int32>& SelectedUnitIds, FHOLGridActionAnswer& OutAnswer);

	/** The script override (Angelscript drops the BP_ prefix): DecideGridAction. */
	UFUNCTION(BlueprintImplementableEvent, Category = "HOL|GridAction")
	FHOLGridActionAnswer BP_DecideGridAction(int32 Command, int32 Ability, bool bGreyed, int32 CooldownSeconds, FName TypeName, const TArray<int32>& SelectedUnitIds);
};
