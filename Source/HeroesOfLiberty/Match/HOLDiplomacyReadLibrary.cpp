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
	/** hol.UI.DiplomacyLayout: the three presentations of the panel, 1 is the user's choice of 2026-11-03 (D650), the others stay behind the setting for comparison. */
	TAutoConsoleVariable<int32> CVarDiplomacyLayout(
		TEXT("hol.UI.DiplomacyLayout"), 1,
		TEXT("The Diplomacy panel's presentation: 0 stance in three buttons (wide, centred), 1 stance cycled on a colour chip (narrow, right), 2 stance in a drop-down menu (bottom left, compact)."),
		ECVF_Default);

	const AHOLGameState* GetDiplomacyGameStateOf(const UObject* WorldContextObject)
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
		const HOLDiplomacy::FBook Book = GameState.IsDiplomacyMode() ? GameState.GetDiplomacyBook() : HOLDiplomacy::FBook();
		const TArray<FHOLPlayersTableRow> Rows = HOLForumRules::GetPlayersTable(GameState.GetLaunchConfig(), Forums, GameState.GetDefeatedMask(),
			Viewer, FHOLForumTuning::FromSettings(), Tiers, GameState.IsDiplomacyMode() ? &Book : nullptr,
			GameState.GetAbsentMask(), GameState.GetAIHeldMask());
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
	const AHOLGameState* GameState = GetDiplomacyGameStateOf(WorldContextObject);
	return GameState && GameState->IsDiplomacyMode();
}

bool UHOLDiplomacyReadLibrary::IsSlotInMatch(const UObject* WorldContextObject, const int32 Slot)
{
	const AHOLGameState* GameState = GetDiplomacyGameStateOf(WorldContextObject);
	return GameState && GameState->GetLaunchConfig().Slots.IsValidIndex(Slot) && GameState->GetLaunchConfig().Slots[Slot].State != EHOLSlotState::Closed;
}

bool UHOLDiplomacyReadLibrary::IsSlotAI(const UObject* WorldContextObject, const int32 Slot)
{
	const AHOLGameState* GameState = GetDiplomacyGameStateOf(WorldContextObject);
	return GameState && GameState->GetLaunchConfig().Slots.IsValidIndex(Slot) && GameState->GetLaunchConfig().Slots[Slot].State == EHOLSlotState::AI;
}

int32 UHOLDiplomacyReadLibrary::GetStance(const UObject* WorldContextObject, const int32 From, const int32 To)
{
	const AHOLGameState* GameState = GetDiplomacyGameStateOf(WorldContextObject);
	return GameState ? HOLDiplomacy::StanceOf(GameState->GetDiplomacyBook(), From, To) : HOLDiplomacy::Enemy;
}

int32 UHOLDiplomacyReadLibrary::GetRelation(const UObject* WorldContextObject, const int32 A, const int32 B)
{
	const AHOLGameState* GameState = GetDiplomacyGameStateOf(WorldContextObject);
	return GameState ? GameState->GetDiplomacyRelation(A, B) : HOLDiplomacy::Enemy;
}

bool UHOLDiplomacyReadLibrary::GetVision(const UObject* WorldContextObject, const int32 From, const int32 To)
{
	const AHOLGameState* GameState = GetDiplomacyGameStateOf(WorldContextObject);
	return GameState && HOLDiplomacy::SharesVision(GameState->GetDiplomacyBook(), From, To);
}

FLinearColor UHOLDiplomacyReadLibrary::GetSlotColor(const UObject* WorldContextObject, const int32 Slot)
{
	const AHOLGameState* GameState = GetDiplomacyGameStateOf(WorldContextObject);
	if (!GameState || !GameState->GetLaunchConfig().Slots.IsValidIndex(Slot))
	{
		return FLinearColor(0.5f, 0.5f, 0.5f, 1.f);
	}
	return FLinearColor(HOLPlayerColors::GetColor(GameState->GetLaunchConfig().Slots[Slot].ColorIndex));
}

FString UHOLDiplomacyReadLibrary::GetSlotNameText(const UObject* WorldContextObject, const int32 Viewer, const int32 Slot)
{
	const AHOLGameState* GameState = GetDiplomacyGameStateOf(WorldContextObject);
	FHOLPlayersTableRow Row;
	return GameState && FindRow(*GameState, Viewer, Slot, Row) ? Row.Name.ToString() : FString();
}

FString UHOLDiplomacyReadLibrary::GetSlotLivesText(const UObject* WorldContextObject, const int32 Viewer, const int32 Slot)
{
	const AHOLGameState* GameState = GetDiplomacyGameStateOf(WorldContextObject);
	FHOLPlayersTableRow Row;
	return GameState && FindRow(*GameState, Viewer, Slot, Row) ? Row.Lives.ToString() : FString();
}

FString UHOLDiplomacyReadLibrary::GetSlotStateText(const UObject* WorldContextObject, const int32 Viewer, const int32 Slot)
{
	const AHOLGameState* GameState = GetDiplomacyGameStateOf(WorldContextObject);
	FHOLPlayersTableRow Row;
	return GameState && FindRow(*GameState, Viewer, Slot, Row) ? Row.State.ToString() : FString();
}

FString UHOLDiplomacyReadLibrary::GetSlotFactionText(const UObject* WorldContextObject, const int32 Slot)
{
	// The new game screen's own table (namespace HOLNewGameScreen, same keys and sources: one entry per name in the gathered texts).
	const AHOLGameState* GameState = GetDiplomacyGameStateOf(WorldContextObject);
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

// Socle step 1 (D830, D836 (5)): the C++ rule on a book, for the script. Wrappers only.

namespace
{
	HOLDiplomacy::FBook ToBook(const FHOLDiplomacyStances& Book)
	{
		HOLDiplomacy::FBook Out;
		Out.Slots = Book.Slots;
		Out.bLocked = Book.bLocked;
		Out.Stances = Book.Stances;
		Out.bAI = Book.bAI;
		Out.Vision = Book.Vision;
		return Out;
	}

	FHOLDiplomacyStances FromBook(const HOLDiplomacy::FBook& Book)
	{
		FHOLDiplomacyStances Out;
		Out.Slots = Book.Slots;
		Out.bLocked = Book.bLocked;
		Out.Stances = Book.Stances;
		Out.bAI = Book.bAI;
		Out.Vision = Book.Vision;
		return Out;
	}
}

FHOLDiplomacyStances UHOLDiplomacyReadLibrary::GetBook(const UObject* WorldContextObject)
{
	const AHOLGameState* GameState = GetDiplomacyGameStateOf(WorldContextObject);
	return GameState ? FromBook(GameState->GetDiplomacyBook()) : FHOLDiplomacyStances();
}

FHOLDiplomacyStances UHOLDiplomacyReadLibrary::NewBook(const int32 Slots, const TArray<bool>& IsAI)
{
	return FromBook(HOLDiplomacy::NewBook(Slots, IsAI));
}

FHOLDiplomacyStances UHOLDiplomacyReadLibrary::NewTeamBook(const TArray<int32>& TeamBySlot, const TArray<bool>& IsAI)
{
	return FromBook(HOLDiplomacy::NewTeamBook(TeamBySlot, IsAI));
}

int32 UHOLDiplomacyReadLibrary::SetStance(FHOLDiplomacyStances& Book, const int32 From, const int32 To, const int32 Stance)
{
	HOLDiplomacy::FBook Rule = ToBook(Book);
	const HOLDiplomacy::FStanceChange Change = HOLDiplomacy::SetStance(Rule, From, To, Stance);
	Book = FromBook(Rule);
	return static_cast<int32>(Change.Result);
}

int32 UHOLDiplomacyReadLibrary::SetVision(FHOLDiplomacyStances& Book, const int32 From, const int32 To, const bool bShare)
{
	HOLDiplomacy::FBook Rule = ToBook(Book);
	const HOLDiplomacy::FVisionChange Change = HOLDiplomacy::SetVision(Rule, From, To, bShare);
	Book = FromBook(Rule);
	return static_cast<int32>(Change.Result);
}

int32 UHOLDiplomacyReadLibrary::StanceOf(const FHOLDiplomacyStances& Book, const int32 From, const int32 To)
{
	return HOLDiplomacy::StanceOf(ToBook(Book), From, To);
}

int32 UHOLDiplomacyReadLibrary::Relation(const FHOLDiplomacyStances& Book, const int32 A, const int32 B)
{
	return HOLDiplomacy::Relation(ToBook(Book), A, B);
}

bool UHOLDiplomacyReadLibrary::IsAlly(const FHOLDiplomacyStances& Book, const int32 A, const int32 B)
{
	return HOLDiplomacy::IsAlly(ToBook(Book), A, B);
}

bool UHOLDiplomacyReadLibrary::IsAdversary(const FHOLDiplomacyStances& Book, const int32 A, const int32 B)
{
	return HOLDiplomacy::IsAdversary(ToBook(Book), A, B);
}

bool UHOLDiplomacyReadLibrary::FiresAtOnce(const FHOLDiplomacyStances& Book, const int32 A, const int32 B)
{
	return HOLDiplomacy::FiresAtOnce(ToBook(Book), A, B);
}

bool UHOLDiplomacyReadLibrary::SharesVision(const FHOLDiplomacyStances& Book, const int32 From, const int32 To)
{
	return HOLDiplomacy::SharesVision(ToBook(Book), From, To);
}

bool UHOLDiplomacyReadLibrary::CanLeaveRampart(const FHOLDiplomacyStances& Book, const int32 UnitOwner, const int32 RampartOwner)
{
	return HOLDiplomacy::CanLeaveRampart(ToBook(Book), UnitOwner, RampartOwner);
}

// Constants and codes of HOLDiplomacy, and the SetStance / SetVision fiches (D836 (5)). Wrappers only.

int32 UHOLDiplomacyReadLibrary::GetStanceEnemy() { return HOLDiplomacy::Enemy; }
int32 UHOLDiplomacyReadLibrary::GetStanceNeutral() { return HOLDiplomacy::Neutral; }
int32 UHOLDiplomacyReadLibrary::GetStanceAlly() { return HOLDiplomacy::Ally; }
int32 UHOLDiplomacyReadLibrary::GetStanceDefault() { return HOLDiplomacy::DefaultStance; }
int32 UHOLDiplomacyReadLibrary::GetResultOk() { return static_cast<int32>(HOLDiplomacy::EResult::Ok); }
int32 UHOLDiplomacyReadLibrary::GetResultNoSuchSlot() { return static_cast<int32>(HOLDiplomacy::EResult::NoSuchSlot); }
int32 UHOLDiplomacyReadLibrary::GetResultSelf() { return static_cast<int32>(HOLDiplomacy::EResult::Self); }
int32 UHOLDiplomacyReadLibrary::GetResultBadStance() { return static_cast<int32>(HOLDiplomacy::EResult::BadStance); }
int32 UHOLDiplomacyReadLibrary::GetResultSenderIsAI() { return static_cast<int32>(HOLDiplomacy::EResult::SenderIsAI); }
int32 UHOLDiplomacyReadLibrary::GetResultLocked() { return static_cast<int32>(HOLDiplomacy::EResult::Locked); }
int32 UHOLDiplomacyReadLibrary::GetResultNotAlly() { return static_cast<int32>(HOLDiplomacy::EResult::NotAlly); }

FHOLDiplomacyStanceChange UHOLDiplomacyReadLibrary::ChangeStance(FHOLDiplomacyStances& Book, const int32 From, const int32 To, const int32 Stance)
{
	HOLDiplomacy::FBook Rule = ToBook(Book);
	const HOLDiplomacy::FStanceChange Change = HOLDiplomacy::SetStance(Rule, From, To, Stance);
	Book = FromBook(Rule);
	FHOLDiplomacyStanceChange Out;
	Out.Result = static_cast<int32>(Change.Result);
	Out.bAnnounce = Change.bAnnounce;
	Out.From = Change.From;
	Out.To = Change.To;
	Out.Stance = Change.Stance;
	Out.Before = Change.Before;
	Out.After = Change.After;
	Out.bBroke = Change.bBroke;
	Out.bFormed = Change.bFormed;
	return Out;
}

FHOLDiplomacyVisionChange UHOLDiplomacyReadLibrary::ChangeVision(FHOLDiplomacyStances& Book, const int32 From, const int32 To, const bool bShare)
{
	HOLDiplomacy::FBook Rule = ToBook(Book);
	const HOLDiplomacy::FVisionChange Change = HOLDiplomacy::SetVision(Rule, From, To, bShare);
	Book = FromBook(Rule);
	FHOLDiplomacyVisionChange Out;
	Out.Result = static_cast<int32>(Change.Result);
	Out.bAnnounce = Change.bAnnounce;
	Out.From = Change.From;
	Out.To = Change.To;
	Out.bShare = Change.bShare;
	return Out;
}