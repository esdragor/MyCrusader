// Copyright Epic Games, Inc. All Rights Reserved.

#include "Match/HOLChatSubsystem.h"

#include "Engine/World.h"
#include "Match/HOLDiplomacyReadLibrary.h"
#include "Match/HOLDiplomacyRules.h"
#include "Match/HOLGameState.h"
#include "Match/HOLMatchConfig.h"

UHOLChatSubsystem* UHOLChatSubsystem::Find(const UWorld* World)
{
	if (!World)
	{
		return nullptr;
	}
	const TArray<UHOLChatSubsystem*> Boards = World->GetSubsystemArrayCopy<UHOLChatSubsystem>();
	return Boards.Num() > 0 ? Boards[0] : nullptr;
}

bool UHOLChatSubsystem::ShouldCreateSubsystem(UObject* Outer) const
{
	// Never the abstract C++ class, nor a script class replaced by a reload (as UHOLSignalSubsystem).
	if (IsUnreachable() || GetClass()->HasAnyClassFlags(CLASS_Abstract | CLASS_NewerVersionExists))
	{
		return false;
	}
	return Super::ShouldCreateSubsystem(Outer);
}

bool UHOLChatSubsystem::DoesSupportWorldType(const EWorldType::Type WorldType) const
{
	return WorldType == EWorldType::Game || WorldType == EWorldType::PIE;
}

FHOLChatSeats UHOLChatSubsystem::ReadSeats(const UWorld* World)
{
	FHOLChatSeats Seats;
	if (!World)
	{
		return Seats;
	}
	for (int32 Slot = 0; Slot < FHOLMatchConfig::MaxSlots; ++Slot)
	{
		if (UHOLDiplomacyReadLibrary::IsSlotInMatch(World, Slot))
		{
			Seats.Slots = Slot + 1;
		}
	}
	Seats.bTeamGame = !UHOLDiplomacyReadLibrary::IsDiplomacyMode(World);
	const AHOLGameState* GameState = World->GetGameState<AHOLGameState>();
	for (int32 From = 0; From < Seats.Slots; ++From)
	{
		Seats.SpectatorBySlot.Add(GameState && GameState->IsSlotDefeated(From));
		for (int32 To = 0; To < Seats.Slots; ++To)
		{
			Seats.MateByPair.Add(From != To && UHOLDiplomacyReadLibrary::IsSlotInMatch(World, From) && UHOLDiplomacyReadLibrary::IsSlotInMatch(World, To)
				&& UHOLDiplomacyReadLibrary::GetRelation(World, From, To) == HOLDiplomacy::Ally);
		}
	}
	return Seats;
}

int32 UHOLChatSubsystem::ParseChannel(const FString& Name)
{
	if (Name.Equals(TEXT("all"), ESearchCase::IgnoreCase))
	{
		return ChannelAll;
	}
	if (Name.Equals(TEXT("allies"), ESearchCase::IgnoreCase))
	{
		return ChannelAllies;
	}
	if (Name.Equals(TEXT("private"), ESearchCase::IgnoreCase))
	{
		return ChannelPrivate;
	}
	return INDEX_NONE;
}

FHOLChatGateAnswer UHOLChatSubsystem::CheckPostNow(const FHOLChatSeats& Seats, const int32 Channel, const int32 Sender, const int32 Target, const FString& Text,
	const TArray<int32>& RecentMs, const int32 NowMs, const int32 MaxLength, const int32 MaxPerWindow, const int32 WindowMs)
{
	return ChatCheckPost(Seats, Channel, Sender, Target, Text, RecentMs, NowMs, MaxLength, MaxPerWindow, WindowMs);
}

bool UHOLChatSubsystem::ReachesNow(const FHOLChatSeats& Seats, const int32 Channel, const int32 Sender, const int32 Target, const int32 Receiver)
{
	return ChatReaches(Seats, Channel, Sender, Target, Receiver);
}

FHOLChatGateAnswer UHOLChatSubsystem::ChatCheckPost_Implementation(const FHOLChatSeats& Seats, int32 Channel, int32 Sender, int32 Target, const FString& Text,
	const TArray<int32>& RecentMs, int32 NowMs, int32 MaxLength, int32 MaxPerWindow, int32 WindowMs)
{
	FHOLChatGateAnswer Gate;
	Gate.Answer = NoScript;
	Gate.RecentMs = RecentMs;
	return Gate;
}
