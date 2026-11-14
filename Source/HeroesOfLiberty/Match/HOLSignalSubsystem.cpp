// Copyright Epic Games, Inc. All Rights Reserved.

#include "Match/HOLSignalSubsystem.h"

#include "Engine/World.h"
#include "Match/HOLDiplomacyReadLibrary.h"
#include "Match/HOLGameState.h"
#include "Match/HOLMatchConfig.h"

UHOLSignalSubsystem* UHOLSignalSubsystem::Find(const UWorld* World)
{
	if (!World)
	{
		return nullptr;
	}
	const TArray<UHOLSignalSubsystem*> Boards = World->GetSubsystemArrayCopy<UHOLSignalSubsystem>();
	return Boards.Num() > 0 ? Boards[0] : nullptr;
}

bool UHOLSignalSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	// Never the abstract C++ class, nor a script class replaced by a reload (as UHOLDuelSubsystem).
	if (IsUnreachable() || GetClass()->HasAnyClassFlags(CLASS_Abstract | CLASS_NewerVersionExists))
	{
		return false;
	}
	return Super::ShouldCreateSubsystem(Outer);
}

bool UHOLSignalSubsystem::DoesSupportWorldType(const EWorldType::Type WorldType) const
{
	return WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
}

FHOLSignalBookInput UHOLSignalSubsystem::ReadBook(const UWorld* World)
{
	FHOLSignalBookInput Book;
	if (!World)
	{
		return Book;
	}
	for (int32 Slot = 0; Slot < FHOLMatchConfig::MaxSlots; ++Slot)
	{
		if (UHOLDiplomacyReadLibrary::IsSlotInMatch(World, Slot))
		{
			Book.Slots = Slot + 1;
		}
	}
	Book.bLocked = !UHOLDiplomacyReadLibrary::IsDiplomacyMode(World);
	for (int32 Slot = 0; Slot < Book.Slots; ++Slot)
	{
		Book.IsAI.Add(UHOLDiplomacyReadLibrary::IsSlotAI(World, Slot));
		for (int32 To = 0; To < Book.Slots; ++To)
		{
			Book.Stances.Add(UHOLDiplomacyReadLibrary::GetStance(World, Slot, To));
		}
	}
	return Book;
}

bool UHOLSignalSubsystem::IsSignalSpectator(const UWorld* World, const int32 Viewer, const bool bNoSeat)
{
	if (bNoSeat || Viewer < 0)
	{
		return true;
	}
	const AHOLGameState* GameState = World ? World->GetGameState<AHOLGameState>() : nullptr;
	if (!GameState || !GameState->IsSlotDefeated(Viewer))
	{
		return false;
	}
	// A defeated teammate of a team match keeps its team's signals only (D784); defeated alone, it is a spectator (D376).
	for (int32 Other = 0; Other < FHOLMatchConfig::MaxSlots; ++Other)
	{
		if (Other != Viewer && UHOLDiplomacyReadLibrary::IsSlotInMatch(World, Other) && UHOLDiplomacyReadLibrary::GetRelation(World, Viewer, Other) == 2)
		{
			return false;
		}
	}
	return true;
}

int32 UHOLSignalSubsystem::PostNow(const FHOLSignalBookInput& Book, const int32 Sender, const bool bDefeated, const int32 Kind, const FVector& Location, const int32 NowMs)
{
	return PostSignal(Book, Sender, bDefeated, Kind, Location, NowMs);
}

bool UHOLSignalSubsystem::SeesNow(const FHOLSignalBookInput& Book, const int32 Sender, const int32 Viewer, const bool bViewerSpectator)
{
	return SeesSignal(Book, Sender, Viewer, bViewerSpectator);
}
