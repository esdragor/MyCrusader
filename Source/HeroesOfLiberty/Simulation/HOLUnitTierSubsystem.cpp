// Copyright Epic Games, Inc. All Rights Reserved.

#include "Simulation/HOLUnitTierSubsystem.h"

#include "Engine/DataTable.h"
#include "Engine/World.h"
#include "Simulation/HOLSimulationSettings.h"
#include "Simulation/HOLUnitTypes.h"

UHOLUnitTierSubsystem* UHOLUnitTierSubsystem::Find(const UWorld* World)
{
	if (!World)
	{
		return nullptr;
	}
	const TArray<UHOLUnitTierSubsystem*> Boards = World->GetSubsystemArrayCopy<UHOLUnitTierSubsystem>();
	return Boards.Num() > 0 ? Boards[0] : nullptr;
}

int32 UHOLUnitTierSubsystem::CheckUnit(const UWorld* World, const FName UnitType, const int32 Tier, const bool bBuildingFinished, const bool bCanPay)
{
	if (UHOLUnitTierSubsystem* Board = Find(World))
	{
		return Board->CheckUnitTierNow(UnitType, Tier, bBuildingFinished, bCanPay);
	}
	// No script board: no C++ rule to fall back on (D836 point 3). The unit is refused, and the error is said once, not at every order or frame.
	static bool bLogged = false;
	if (!bLogged)
	{
		bLogged = true;
		UE_LOG(LogTemp, Error, TEXT("UnitTier: no script board, every unit is refused (tier too low)"));
	}
	return HOLUnitTierReason::TierTooLow;
}

int32 UHOLUnitTierSubsystem::GetMinTier(const FName UnitType)
{
	const UDataTable* Table = GetDefault<UHOLSimulationSettings>()->UnitTypesTable.LoadSynchronous();
	const FHOLUnitTypeRow* Row = Table ? Table->FindRow<FHOLUnitTypeRow>(UnitType, TEXT("unit tier"), /*bWarnIfRowMissing*/ false) : nullptr;
	return Row ? Row->MinTier : -1;
}

int32 UHOLUnitTierSubsystem::CheckUnitTierNow(const FName UnitType, const int32 Tier, const bool bBuildingFinished, const bool bCanPay)
{
	return CheckUnitTier(UnitType, Tier, bBuildingFinished, bCanPay);
}

bool UHOLUnitTierSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	// Never the abstract C++ class, nor a script class replaced by a reload (as UHOLChatSubsystem).
	if (IsUnreachable() || GetClass()->HasAnyClassFlags(CLASS_Abstract | CLASS_NewerVersionExists))
	{
		return false;
	}
	return Super::ShouldCreateSubsystem(Outer);
}

bool UHOLUnitTierSubsystem::DoesSupportWorldType(const EWorldType::Type WorldType) const
{
	return WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
}

int32 UHOLUnitTierSubsystem::CheckUnitTier_Implementation(const FName UnitType, const int32 Tier, const bool bBuildingFinished, const bool bCanPay)
{
	// No script subclass: refused, the rule is the script's alone (HOLUnitTierBoardScript.as overrides this).
	return HOLUnitTierReason::TierTooLow;
}
