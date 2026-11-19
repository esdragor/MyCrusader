// Copyright Epic Games, Inc. All Rights Reserved.

#include "Match/HOLPlayersTableReadLibrary.h"

#include "Engine/World.h"
#include "Match/HOLDiplomacyRules.h"
#include "Match/HOLForumRules.h"
#include "Match/HOLGameState.h"
#include "Match/HOLMatchConfig.h"
#include "Match/HOLPlayerColors.h"

int32 UHOLPlayersTableReadLibrary::GetMaxSlots()
{
	return FHOLMatchConfig::MaxSlots;
}

TArray<FHOLPlayersTableRow> UHOLPlayersTableReadLibrary::BuildRows(const AHOLGameState* GameState, const int32 LocalSlot)
{
	TArray<FHOLPlayersTableRow> Rows;
	if (!GameState)
	{
		return Rows;
	}
	TArray<FHOLForumState> Forums;
	TArray<int32> Tiers;
	for (int32 Seat = 0; Seat < FHOLMatchConfig::MaxSlots; ++Seat)
	{
		Forums.Add(GameState->GetForumState(Seat));
		Tiers.Add(GameState->GetForumTuning(Seat).Tier);
	}
	// D6: in the Diplomacy mode the allies and enemies are the book's, not the lobby's teams.
	const HOLDiplomacy::FBook Book = GameState->IsDiplomacyMode() ? GameState->GetDiplomacyBook() : HOLDiplomacy::FBook();
	return HOLForumRules::GetPlayersTable(GameState->GetLaunchConfig(), Forums, GameState->GetDefeatedMask(), LocalSlot,
		FHOLForumTuning::FromSettings(), Tiers, GameState->IsDiplomacyMode() ? &Book : nullptr, GameState->GetAbsentMask(), GameState->GetAIHeldMask());
}

void UHOLPlayersTableReadLibrary::GetRows(const UObject* WorldContextObject, const int32 LocalSlot, TArray<FHOLPlayersTableRowRead>& OutRows)
{
	OutRows.Reset();
	const UWorld* World = WorldContextObject ? WorldContextObject->GetWorld() : nullptr;
	const AHOLGameState* GameState = World ? World->GetGameState<AHOLGameState>() : nullptr;
	for (const FHOLPlayersTableRow& Row : BuildRows(GameState, LocalSlot))
	{
		FHOLPlayersTableRowRead& Out = OutRows.AddDefaulted_GetRef();
		Out.Slot = Row.Slot;
		Out.ColorIndex = Row.ColorIndex;
		Out.Name = Row.Name.ToString();
		Out.Lives = Row.Lives.ToString();
		Out.State = Row.State.ToString();
		Out.Color = FLinearColor(HOLPlayerColors::GetColor(Row.ColorIndex));
		Out.bGreyed = Row.bGreyed;
		Out.bStruck = Row.bStruck;
		Out.bAway = Row.bAway;
		Out.bAIHeld = Row.bAIHeld;
		Out.bNewGroup = Row.bNewGroup;
		Out.Relation = static_cast<int32>(Row.Relation);
		Out.Shown = Row.Shown;
	}
}