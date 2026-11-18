// Copyright Epic Games, Inc. All Rights Reserved.

#include "Simulation/HOLUnitTierSubsystem.h"

#include "Engine/World.h"
#include "Simulation/HOLUnitTierRules.h"

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
	return static_cast<int32>(HOLUnitTier::Check(HOLUnitTier::GetMinTier(UnitType), Tier, bBuildingFinished, bCanPay));
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
	return static_cast<int32>(HOLUnitTier::Check(HOLUnitTier::GetMinTier(UnitType), Tier, bBuildingFinished, bCanPay));
}
