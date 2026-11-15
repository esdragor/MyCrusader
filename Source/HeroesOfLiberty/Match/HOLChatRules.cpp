// Copyright Epic Games, Inc. All Rights Reserved.

#include "Match/HOLChatRules.h"

#include "Engine/World.h"
#include "Match/HOLDiplomacyReadLibrary.h"
#include "Match/HOLDiplomacyRules.h"
#include "Match/HOLGameState.h"
#include "Match/HOLMatchConfig.h"

bool HOLChat::FSeats::AreMates(const int32 A, const int32 B) const
{
	return A >= 0 && B >= 0 && A < Slots && B < Slots && Mates.IsValidIndex(A * Slots + B) && Mates[A * Slots + B];
}

bool HOLChat::FSeats::IsSpectator(const int32 Slot) const
{
	return Slot < 0 || Slot >= Slots || (Spectator.IsValidIndex(Slot) && Spectator[Slot]);
}

bool HOLChat::Reaches(const FSeats& Seats, const int32 Channel, const int32 Sender, const int32 Target, const int32 Receiver)
{
	if (Receiver == Sender)
	{
		return true;
	}
	// D376: a spectator talks with the spectators, and with his teammates in a team game (D357). A living player may reach anyone.
	if (Seats.IsSpectator(Sender) && !Seats.IsSpectator(Receiver) && !(Seats.bTeamGame && Seats.AreMates(Sender, Receiver)))
	{
		return false;
	}
	if (Channel == All)
	{
		return true;
	}
	if (Channel == Allies)
	{
		// Mates; a spectator and a living player are not on the same side of the Allies channel outside a team game (D792 2 A).
		return Seats.AreMates(Sender, Receiver) && (Seats.bTeamGame || Seats.IsSpectator(Sender) == Seats.IsSpectator(Receiver));
	}
	if (Channel == Private)
	{
		return Receiver == Target;
	}
	return false;
}

TArray<int32> HOLChat::Recipients(const FSeats& Seats, const int32 Channel, const int32 Sender, const int32 Target)
{
	TArray<int32> Slots;
	Slots.Add(Sender);
	for (int32 Slot = 0; Slot < Seats.Slots; ++Slot)
	{
		if (Slot != Sender && Reaches(Seats, Channel, Sender, Target, Slot))
		{
			Slots.Add(Slot);
		}
	}
	return Slots;
}

int32 HOLChat::CheckPost(const FSeats& Seats, const int32 Channel, const int32 Sender, const int32 Target, const FString& Text, TArray<int32>& RecentMs,
	const int32 NowMs, const int32 MaxLength, const int32 InMaxPerWindow, const int32 InWindowMs)
{
	if (Channel != All && Channel != Allies && Channel != Private)
	{
		return BadChannel;
	}
	if (Channel == Private && (Target < 0 || Target >= Seats.Slots || Target == Sender))
	{
		return BadTarget;
	}
	if (Text.TrimStartAndEnd().IsEmpty())
	{
		return Empty;
	}
	if (Text.Len() > MaxLength)
	{
		return TooLong;
	}
	RecentMs.RemoveAll([NowMs, InWindowMs](const int32 PostedMs) { return NowMs - PostedMs >= InWindowMs; });
	if (RecentMs.Num() >= InMaxPerWindow)
	{
		return TooFast;
	}
	RecentMs.Add(NowMs);
	return Ok;
}

HOLChat::FSeats HOLChat::ReadSeats(const UWorld* World)
{
	FSeats Seats;
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
		Seats.Spectator.Add(GameState && GameState->IsSlotDefeated(From));
		for (int32 To = 0; To < Seats.Slots; ++To)
		{
			Seats.Mates.Add(From != To && UHOLDiplomacyReadLibrary::IsSlotInMatch(World, From) && UHOLDiplomacyReadLibrary::IsSlotInMatch(World, To)
				&& UHOLDiplomacyReadLibrary::GetRelation(World, From, To) == HOLDiplomacy::Ally);
		}
	}
	return Seats;
}

int32 HOLChat::ParseChannel(const FString& Name)
{
	if (Name.Equals(TEXT("all"), ESearchCase::IgnoreCase))
	{
		return All;
	}
	if (Name.Equals(TEXT("allies"), ESearchCase::IgnoreCase))
	{
		return Allies;
	}
	if (Name.Equals(TEXT("private"), ESearchCase::IgnoreCase))
	{
		return Private;
	}
	return INDEX_NONE;
}
