// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "HOLUnitTierSubsystem.generated.h"

/** The reasons a unit cannot be queued now (the numbers of the script rule Script/Units/HOLUnitTierRules.as, namespace HOLUnitTier). */
namespace HOLUnitTierReason
{
	constexpr int32 Available = 0;
	/** The unit has no tier (a type outside the table). */
	constexpr int32 NotListed = 1;
	constexpr int32 TierTooLow = 2;
	constexpr int32 NoBuilding = 3;
	constexpr int32 CannotPay = 4;
}

/**
 * The unit tier rule where the match runs (MIG-CPP-UnitTier, D836 point 3): the script is the ONLY source of the rule (no C++ twin). The thinnest
 * bridge, as UHOLChatSubsystem: the server (AHOLProductionSubsystem, each queued order) and the client's grid (HOLGameScreen.cpp, to grey a unit
 * of a tier too high) ask its script subclass (Script/Units/HOLUnitTierBoardScript.as, the rule Script/Units/HOLUnitTierRules.as,
 * HOLUnitTier::Check) why a unit cannot be queued now. The subsystem is a world subsystem of Game and PIE worlds with no net mode test: it is
 * born on the dedicated server, on every client and in solo. Abstract: only a script subclass is born. Without it (no script compiled) CheckUnit
 * REFUSES (TierTooLow) and logs an error once: there is no C++ rule to fall back on (as the Garrison and Camp bridges).
 */
UCLASS(Abstract)
class HEROESOFLIBERTY_API UHOLUnitTierSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	/** The world's unit tier board (its script subclass), or null (no script compiled, or no world). */
	static UHOLUnitTierSubsystem* Find(const UWorld* World);

	/**
	 * Why a unit of UnitType cannot be queued now at Tier (a HOLUnitTierReason): the script board's answer, or TierTooLow (refused, error logged
	 * once) when the world has no script board. The server's and the grid's call.
	 */
	static int32 CheckUnit(const UWorld* World, FName UnitType, int32 Tier, bool bBuildingFinished, bool bCanPay);

	/** The MinTier of the type's row in the unit type table (the column is the one truth, D836 point 6), -1 when the type has no row. */
	static int32 GetMinTier(FName UnitType);

	/** The board's answer for a unit (the native event below, i.e. the script rule in its script subclass). */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|UnitTier")
	int32 CheckUnitTierNow(FName UnitType, int32 Tier, bool bBuildingFinished, bool bCanPay);

	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

protected:
	virtual bool DoesSupportWorldType(const EWorldType::Type WorldType) const override;

	/** Why the unit cannot be queued now (a HOLUnitTierReason). The default implementation refuses (TierTooLow): the rule is the script subclass's alone. */
	UFUNCTION(BlueprintNativeEvent, Category = "HeroesOfLiberty|UnitTier")
	int32 CheckUnitTier(FName UnitType, int32 Tier, bool bBuildingFinished, bool bCanPay);
	virtual int32 CheckUnitTier_Implementation(FName UnitType, int32 Tier, bool bBuildingFinished, bool bCanPay);
};
