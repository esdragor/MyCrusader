// Copyright Epic Games, Inc. All Rights Reserved.

#include "Match/HOLDiplomacyReadLibrary.h"

#include "Buildings/HOLBuilding.h"
#include "Buildings/HOLBuildingTypes.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "HAL/IConsoleManager.h"
#include "Match/HOLDiplomacyRules.h"
#include "Match/HOLForumRules.h"
#include "Match/HOLGameState.h"
#include "Match/HOLMarketRules.h"
#include "Match/HOLMatchConfig.h"
#include "Match/HOLPlayerColors.h"

namespace
{
	/** hol.UI.DiplomacyLayout: the three presentations of the panel, behind a setting until the user chooses (DiplomacyInProgress.md, D7). */
	TAutoConsoleVariable<int32> CVarDiplomacyLayout(
		TEXT("hol.UI.DiplomacyLayout"), 0,
		TEXT("The Diplomacy panel's presentation: 0 stance in three buttons (wide, centred), 1 stance cycled on a colour chip (narrow, right), 2 stance in a drop-down menu (bottom left, compact)."),
		ECVF_Default);

	const AHOLGameState* GetGameStateOf(const UObject* WorldContextObject)
	{
		const UWorld* World = GEngine ? GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull) : nullptr;
		return World ? World->GetGameState<AHOLGameState>() : nullptr;
	}

	/** The row of the players' table of Slot, seen from Viewer (as UHOLGameScreen::RefreshPlayersTable builds it). */
	bool FindRow(const AHOLGameState& GameState, const int32 Viewer, const int32 Slot, FHOLPlayersTableRow& OutRow)
	{
		TArray<FHOLForumState> Forums;
		TArray<int32> Tiers;
		for (int32 Seat = 0; Seat < FHOLMatchConfig::MaxSlots; ++Seat)
		{
			Forums.Add(GameState.GetForumState(Seat));
			Tiers.Add(GameState.GetForumTuning(Seat).Tier);
		}
		const TArray<FHOLPlayersTableRow> Rows = HOLForumRules::GetPlayersTable(GameState.GetLaunchConfig(), Forums, GameState.GetDefeatedMask(),
			Viewer, FHOLForumTuning::FromSettings(), Tiers);
		for (const FHOLPlayersTableRow& Row : Rows)
		{
			if (Row.Slot == Slot)
			{
				OutRow = Row;
				return true;
			}
		}
		return false;
	}
}

bool UHOLDiplomacyReadLibrary::IsDiplomacyMode(const UObject* WorldContextObject)
{
	const AHOLGameState* GameState = GetGameStateOf(WorldContextObject);
	return GameState && GameState->IsDiplomacyMode();
}

bool UHOLDiplomacyReadLibrary::IsSlotInMatch(const UObject* WorldContextObject, const int32 Slot)
{
	const AHOLGameState* GameState = GetGameStateOf(WorldContextObject);
	return GameState && GameState->GetLaunchConfig().Slots.IsValidIndex(Slot) && GameState->GetLaunchConfig().Slots[Slot].State != EHOLSlotState::Closed;
}

bool UHOLDiplomacyReadLibrary::IsSlotAI(const UObject* WorldContextObject, const int32 Slot)
{
	const AHOLGameState* GameState = GetGameStateOf(WorldContextObject);
	return GameState && GameState->GetLaunchConfig().Slots.IsValidIndex(Slot) && GameState->GetLaunchConfig().Slots[Slot].State == EHOLSlotState::AI;
}

int32 UHOLDiplomacyReadLibrary::GetStance(const UObject* WorldContextObject, const int32 From, const int32 To)
{
	const AHOLGameState* GameState = GetGameStateOf(WorldContextObject);
	return GameState ? HOLDiplomacy::StanceOf(GameState->GetDiplomacyBook(), From, To) : HOLDiplomacy::Enemy;
}

int32 UHOLDiplomacyReadLibrary::GetRelation(const UObject* WorldContextObject, const int32 A, const int32 B)
{
	const AHOLGameState* GameState = GetGameStateOf(WorldContextObject);
	return GameState ? GameState->GetDiplomacyRelation(A, B) : HOLDiplomacy::Enemy;
}

bool UHOLDiplomacyReadLibrary::GetVision(const UObject* WorldContextObject, const int32 From, const int32 To)
{
	const AHOLGameState* GameState = GetGameStateOf(WorldContextObject);
	return GameState && HOLDiplomacy::SharesVision(GameState->GetDiplomacyBook(), From, To);
}

FLinearColor UHOLDiplomacyReadLibrary::GetSlotColor(const UObject* WorldContextObject, const int32 Slot)
{
	const AHOLGameState* GameState = GetGameStateOf(WorldContextObject);
	if (!GameState || !GameState->GetLaunchConfig().Slots.IsValidIndex(Slot))
	{
		return FLinearColor(0.5f, 0.5f, 0.5f, 1.f);
	}
	return FLinearColor(HOLPlayerColors::GetColor(GameState->GetLaunchConfig().Slots[Slot].ColorIndex));
}

FString UHOLDiplomacyReadLibrary::GetSlotNameText(const UObject* WorldContextObject, const int32 Viewer, const int32 Slot)
{
	const AHOLGameState* GameState = GetGameStateOf(WorldContextObject);
	FHOLPlayersTableRow Row;
	return GameState && FindRow(*GameState, Viewer, Slot, Row) ? Row.Name.ToString() : FString();
}

FString UHOLDiplomacyReadLibrary::GetSlotLivesText(const UObject* WorldContextObject, const int32 Viewer, const int32 Slot)
{
	const AHOLGameState* GameState = GetGameStateOf(WorldContextObject);
	FHOLPlayersTableRow Row;
	return GameState && FindRow(*GameState, Viewer, Slot, Row) ? Row.Lives.ToString() : FString();
}

FString UHOLDiplomacyReadLibrary::GetSlotStateText(const UObject* WorldContextObject, const int32 Viewer, const int32 Slot)
{
	const AHOLGameState* GameState = GetGameStateOf(WorldContextObject);
	FHOLPlayersTableRow Row;
	return GameState && FindRow(*GameState, Viewer, Slot, Row) ? Row.State.ToString() : FString();
}

FString UHOLDiplomacyReadLibrary::GetSlotFactionText(const UObject* WorldContextObject, const int32 Slot)
{
	// The new game screen's own table (namespace HOLNewGameScreen, same keys and sources: one entry per name in the gathered texts).
	const AHOLGameState* GameState = GetGameStateOf(WorldContextObject);
	if (!GameState || !GameState->GetLaunchConfig().Slots.IsValidIndex(Slot))
	{
		return FString();
	}
	switch (GameState->GetLaunchConfig().Slots[Slot].Faction)
	{
	case EHOLFaction::DawnOrder: return NSLOCTEXT("HOLNewGameScreen", "DawnOrder", "Ordre de l'Aube").ToString();
	case EHOLFaction::BlackLegions: return NSLOCTEXT("HOLNewGameScreen", "BlackLegions", "Légions Noires").ToString();
	case EHOLFaction::DragonChildren: return NSLOCTEXT("HOLNewGameScreen", "DragonChildren", "Enfants du Dragon").ToString();
	case EHOLFaction::ShadowCircle: return NSLOCTEXT("HOLNewGameScreen", "ShadowCircle", "Cercle de l'Ombre").ToString();
	case EHOLFaction::FireHeirs: return NSLOCTEXT("HOLNewGameScreen", "FireHeirs", "Héritiers du Feu").ToString();
	case EHOLFaction::Templars: return NSLOCTEXT("HOLNewGameScreen", "Templars", "Templiers").ToString();
	}
	return FString();
}

bool UHOLDiplomacyReadLibrary::HasFinishedMarket(const UObject* WorldContextObject, const int32 Slot)
{
	const UWorld* World = GEngine ? GEngine->GetWorldFromContextObject(WorldContextObject, EGetWorldErrorMode::ReturnNull) : nullptr;
	if (!World || Slot < 0)
	{
		return false;
	}
	for (FHOLBuildingIterator It(World); It; ++It)
	{
		if (It->GetOwnerPlayer() == Slot && It->GetTypeName() == HOLBuildingTypeNames::Market && It->IsFinished())
		{
			return true;
		}
	}
	return false;
}

int32 UHOLDiplomacyReadLibrary::GetTributeTaxPercent()
{
	return HOLTribute::DefaultTaxPercent;
}

int32 UHOLDiplomacyReadLibrary::GetPanelLayout()
{
	return FMath::Clamp(CVarDiplomacyLayout.GetValueOnGameThread(), 0, 2);
}
