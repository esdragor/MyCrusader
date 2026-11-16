// Copyright Epic Games, Inc. All Rights Reserved.

#include "Simulation/HOLGarrisonBoard.h"

#include "Engine/World.h"

UHOLGarrisonBoard* UHOLGarrisonBoard::Find(const UWorld* World)
{
	if (!World)
	{
		return nullptr;
	}
	const TArray<UHOLGarrisonBoard*> Boards = World->GetSubsystemArrayCopy<UHOLGarrisonBoard>();
	return Boards.Num() > 0 ? Boards[0] : nullptr;
}

bool UHOLGarrisonBoard::ShouldCreateSubsystem(UObject* Outer) const
{
	// Never the abstract C++ class, nor a script class replaced by a reload (as UHOLChatSubsystem).
	if (IsUnreachable() || GetClass()->HasAnyClassFlags(CLASS_Abstract | CLASS_NewerVersionExists))
	{
		return false;
	}
	return Super::ShouldCreateSubsystem(Outer);
}

bool UHOLGarrisonBoard::DoesSupportWorldType(const EWorldType::Type WorldType) const
{
	return WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
}

TArray<int32> UHOLGarrisonBoard::AdmittedForOrder(const UWorld* World, const TArray<int32>& UnitSlots, const TArray<HOLGarrison::EKind>& UnitKinds,
	const int32 BuildingSlot, const int32 Slots, const int32 Occupants)
{
	UHOLGarrisonBoard* Board = Find(World);
	if (!Board)
	{
		return HOLGarrison::AdmittedBySettings(UnitSlots, UnitKinds, BuildingSlot, Slots, Occupants);
	}
	TArray<int32> Kinds;
	Kinds.Reserve(UnitKinds.Num());
	for (const HOLGarrison::EKind Kind : UnitKinds)
	{
		Kinds.Add(static_cast<int32>(Kind));
	}
	return Board->AdmittedNow(UnitSlots, Kinds, BuildingSlot, Slots, Occupants);
}

TArray<int32> UHOLGarrisonBoard::AdmittedNow(const TArray<int32>& UnitSlots, const TArray<int32>& UnitKinds, const int32 BuildingSlot, const int32 Slots,
	const int32 Occupants)
{
	return GarrisonAdmitted(UnitSlots, UnitKinds, BuildingSlot, Slots, Occupants);
}

TArray<int32> UHOLGarrisonBoard::GarrisonAdmitted_Implementation(const TArray<int32>& UnitSlots, const TArray<int32>& UnitKinds, const int32 BuildingSlot,
	const int32 Slots, const int32 Occupants)
{
	// The C++ rule, the same list as the script's (a kind outside 0..4 never enters, as in the script).
	TArray<HOLGarrison::EKind> Kinds;
	Kinds.Reserve(UnitKinds.Num());
	for (const int32 Kind : UnitKinds)
	{
		Kinds.Add(Kind >= 0 && Kind <= static_cast<int32>(HOLGarrison::EKind::Hero) ? static_cast<HOLGarrison::EKind>(Kind) : HOLGarrison::EKind::Siege);
	}
	return HOLGarrison::AdmittedBySettings(UnitSlots, Kinds, BuildingSlot, Slots, Occupants);
}
