// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Buildings/HOLGarrisonRules.h"
#include "Subsystems/WorldSubsystem.h"
#include "HOLGarrisonBoard.generated.h"

/**
 * The garrison's rule where the server applies an order (F7, bascule Garrison, D830/D836 point 4: the script is the source). The
 * thinnest bridge, as UHOLChatSubsystem: UHOLGarrisonSubsystem reads the candidates (units, Mass fragments: C++) and asks the world's board
 * which of them enter; the script subclass (Script/Garrison/HOLGarrisonBoardScript.as) answers by HOLGarrison::Admitted of
 * Script/Garrison/HOLGarrisonRules.as. Abstract: only a script subclass is born. Without it (no script compiled) the server REFUSES every entry (R7:
 * no free passage); there is no C++ copy of the rule any more, and the default of the event refuses too.
 * Server and solo only; a client never decides.
 */
UCLASS(Abstract)
class HEROESOFLIBERTY_API UHOLGarrisonBoard : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	/** The world's garrison board (its script subclass), or null (no script compiled, or no world). */
	static UHOLGarrisonBoard* Find(const UWorld* World);

	/**
	 * What the server applies to an order: the board's answer when the script board exists, else nobody (refused).
	 * Kinds are HOLGarrison::EKind, the script's numbers.
	 */
	static TArray<int32> AdmittedForOrder(const UWorld* World, const TArray<int32>& UnitSlots, const TArray<HOLGarrison::EKind>& UnitKinds,
		int32 BuildingSlot, int32 Slots, int32 Occupants);

	/** The board's answer: indexes of the candidates that enter (the script's HOLGarrison::Admitted, default settings). */
	UFUNCTION(BlueprintCallable, Category = "HeroesOfLiberty|Garrison")
	TArray<int32> AdmittedNow(const TArray<int32>& UnitSlots, const TArray<int32>& UnitKinds, int32 BuildingSlot, int32 Slots, int32 Occupants);

	virtual bool ShouldCreateSubsystem(UObject* Outer) const override;

protected:
	virtual bool DoesSupportWorldType(const EWorldType::Type WorldType) const override;

	/** Overridden by the script board; the default refuses everybody (no C++ rule). */
	UFUNCTION(BlueprintNativeEvent, Category = "HeroesOfLiberty|Garrison")
	TArray<int32> GarrisonAdmitted(const TArray<int32>& UnitSlots, const TArray<int32>& UnitKinds, int32 BuildingSlot, int32 Slots, int32 Occupants);
	virtual TArray<int32> GarrisonAdmitted_Implementation(const TArray<int32>& UnitSlots, const TArray<int32>& UnitKinds, int32 BuildingSlot, int32 Slots,
		int32 Occupants);
};
