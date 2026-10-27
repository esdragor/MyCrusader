// Copyright Epic Games, Inc. All Rights Reserved.

#include "Match/HOLDisconnectRules.h"

#include "Match/HOLMatchConfig.h"

bool FHOLDisconnectRules::KeepsSlot(const FHOLMatchConfig& Config, const int32 Slot, const bool bWaitingForPlayers,
	const bool bMatchOver, const bool bSlotDefeated)
{
	if (!Config.HasPlayerNames() || !Config.Slots.IsValidIndex(Slot) || bWaitingForPlayers || bMatchOver || bSlotDefeated)
	{
		return false;
	}
	const FHOLMatchSlot& SlotConfig = Config.Slots[Slot];
	return SlotConfig.State == EHOLSlotState::Human && !SlotConfig.PlayerName.TrimStartAndEnd().IsEmpty();
}

bool FHOLDisconnectRules::IsWindowOver(const double LeftAt, const double Now, const double WindowSeconds)
{
	return Now - LeftAt >= WindowSeconds;
}

double FHOLDisconnectRules::GetSecondsLeft(const double LeftAt, const double Now, const double WindowSeconds)
{
	return FMath::Max(0., LeftAt + WindowSeconds - Now);
}

int32 FHOLDisconnectRules::FindAbsent(const TConstArrayView<FHOLAbsentSlot> Absent, const int32 Slot)
{
	for (int32 Index = 0; Index < Absent.Num(); ++Index)
	{
		if (Absent[Index].Slot == Slot)
		{
			return Index;
		}
	}
	return INDEX_NONE;
}

TArray<int32> FHOLDisconnectRules::GetExpired(const TConstArrayView<FHOLAbsentSlot> Absent, const double Now, const double WindowSeconds)
{
	TArray<int32> Expired;
	for (const FHOLAbsentSlot& Each : Absent)
	{
		if (IsWindowOver(Each.LeftAt, Now, WindowSeconds))
		{
			Expired.Add(Each.Slot);
		}
	}
	return Expired;
}

FHOLResources FHOLDisconnectRules::GetStockOnTaking(const FHOLAbsentSlot* Absent, const FHOLResources& Starting)
{
	return Absent ? Absent->Resources : Starting;
}

#define LOCTEXT_NAMESPACE "HOLGameScreen"

FText FHOLDisconnectRules::FormatAbsentLine(const FString& Name, const double SecondsLeft, const double WindowSeconds)
{
	int32 Left = FMath::Max(0, FMath::CeilToInt(SecondsLeft));
	if (WindowSeconds > 0.)
	{
		Left = FMath::Min(Left, FMath::FloorToInt(WindowSeconds));
	}
	FFormatNamedArguments Arguments;
	Arguments.Add(TEXT("Name"), FText::AsCultureInvariant(Name));
	Arguments.Add(TEXT("Time"), FText::AsCultureInvariant(FString::Printf(TEXT("%d:%02d"), Left / 60, Left % 60)));
	return FText::Format(LOCTEXT("AbsentLine", "{Name} : connexion perdue — {Time}"), Arguments);
}

FText FHOLDisconnectRules::FormatBackLine(const FString& Name)
{
	FFormatNamedArguments Arguments;
	Arguments.Add(TEXT("Name"), FText::AsCultureInvariant(Name));
	return FText::Format(LOCTEXT("BackLine", "{Name} : de retour"), Arguments);
}

#undef LOCTEXT_NAMESPACE
