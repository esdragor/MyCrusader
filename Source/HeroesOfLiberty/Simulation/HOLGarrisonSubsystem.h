// Copyright Epic Games, Inc. All Rights Reserved.

#pragma once

#include "CoreMinimal.h"
#include "Subsystems/WorldSubsystem.h"
#include "HOLGarrisonSubsystem.generated.h"

class AHOLBuilding;

/**
 * F7 lot 2 (FortificationsInProgress.md, D693): the garrison orders, server and solo. At the start of each step it takes the orders the
 * command subsystem holds (UHOLCommandSubsystem::IssueGarrisonOrder / IssueUngarrisonAllOrder) and applies them with the rule
 * (the script's HOLGarrisonRules.as, asked through UHOLGarrisonBoard): only the building's owner orders its garrison, only its own units of an admitted kind enter (siege
 * never, D793), up to the places of its type (GarrisonSlots, Tools/Data/BuildingTypes.csv). The garrisoned state lives in UHOLUnitSubsystem
 * (SetGarrison): off the views, kept in the population. Not here yet (lots 3 and 4): the body taken off the field (drawing, footprint, combat),
 * the ejection at the building's fall, the network, the interface.
 */
UCLASS()
class HEROESOFLIBERTY_API UHOLGarrisonSubsystem : public UWorldSubsystem
{
	GENERATED_BODY()

public:
	/** Applies the garrison orders waiting; returns how many units went in or out. Nothing on a client. */
	int32 ApplyOrders();
	/**
	 * F7 lots 3b and 3c (D693): every occupant of Building back on the field, each on its own free point at the building's foot (where its
	 * births stand), health unchanged: exit all, and the building's fall (UHOLConstructionSubsystem::DestroyBuilding). Server only; returns how many.
	 */
	static int32 EjectAll(AHOLBuilding& Building);
	/** The places of a building: its type's GarrisonSlots (0 for a type without a row). */
	static int32 GetSlots(const AHOLBuilding& Building);

protected:
	virtual bool DoesSupportWorldType(const EWorldType::Type WorldType) const override;
	virtual void Initialize(FSubsystemCollectionBase& Collection) override;
	virtual void Deinitialize() override;

private:
	void OnStepStarted(int64 StepIndex);
	FDelegateHandle StepStartedHandle;
};
