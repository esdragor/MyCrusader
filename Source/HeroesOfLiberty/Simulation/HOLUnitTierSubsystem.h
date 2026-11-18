// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "HOLUnitTierSubsystem.generated.h"

/**
 * The unit tier rule where the match runs (MIG-CPP-UnitTier, D836 point 3: the script is the source of the rule). The thinnest bridge, as
 * UHOLChatSubsystem: the server (AHOLProductionSubsystem, each queued order) asks its script subclass (Script/Units/HOLUnitTierBoardScript.as,
 * the rule Script/Units/HOLUnitTierRules.as, HOLUnitTier::Check) why a unit cannot be queued now. Abstract: only a script subclass is born.
 * Without it (no script compiled) CheckUnit falls back to the C++ rule HOLUnitTier::Check, so the behaviour is the same as before the bridge;
 * the C++ rule is kept in this lot (its removal is the next one). The Slate grid (HOLGameScreen.cpp) still calls the C++ rule: Slate stays C++
 * for this lot. The reason numbers are HOLUnitTier's (Available 0, NotListed 1, TierTooLow 2, NoBuilding 3, CannotPay 4).
 */
UCLASS(Abstract)
class HEROESOFLIBERTY_API UHOLUnitTierSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	/** The world's unit tier board (its script subclass), or null (no script compiled, or no world). */
	static UHOLUnitTierSubsystem* Find(const UWorld* World);

	/**
	 * Why a unit of UnitType cannot be queued now at Tier: the script board's answer, or the C++ rule's when the world has no script board
	 * (the same numbers, see above). The server's call.
	 */
	static int32 CheckUnit(const UWorld* World, FName UnitType, int32 Tier, bool bBuildingFinished, bool bCanPay);

	/** The board's answer for a unit (the native event below, i.e. the script rule in its script subclass). */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|UnitTier")
	int32 CheckUnitTierNow(FName UnitType, int32 Tier, bool bBuildingFinished, bool bCanPay);

	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

protected:
	virtual bool DoesSupportWorldType(const EWorldType::Type WorldType) const override;

	/** Why the unit cannot be queued now (HOLUnitTier::EBlock as an int). The default implementation is the C++ rule HOLUnitTier::Check on the column MinTier. */
	UFUNCTION(BlueprintNativeEvent, Category = "HeroesOfLiberty|UnitTier")
	int32 CheckUnitTier(FName UnitType, int32 Tier, bool bBuildingFinished, bool bCanPay);
	virtual int32 CheckUnitTier_Implementation(FName UnitType, int32 Tier, bool bBuildingFinished, bool bCanPay);
};
