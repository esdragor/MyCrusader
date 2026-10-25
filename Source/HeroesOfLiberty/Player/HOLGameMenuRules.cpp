// Copyright Epic Games, Inc. All Rights Reserved.

#include "Player/HOLGameMenuRules.h"

EHOLEscapeAction HOLGameMenuRules::GetEscapeAction(const bool bMenuOpen, const bool bModeArmed, const bool bHasSelection)
{
	if (bMenuOpen)
	{
		return EHOLEscapeAction::CloseMenu;
	}
	if (bModeArmed)
	{
		return EHOLEscapeAction::CancelMode;
	}
	return bHasSelection ? EHOLEscapeAction::Deselect : EHOLEscapeAction::OpenMenu;
}

bool HOLGameMenuRules::CanSurrender(const bool bDefeated, const bool bMatchOver)
{
	return !bDefeated && !bMatchOver;
}

bool HOLGameMenuRules::CanQuitToMenu(const bool bDefeated, const bool bMatchOver)
{
	return !CanSurrender(bDefeated, bMatchOver);
}

EHOLEndBanner HOLGameMenuRules::GetEndBanner(const int32 MySlot, const uint8 DefeatedMask, const bool bMatchOver, const uint8 WinnerMask)
{
	if (MySlot < 0 || MySlot >= 8)
	{
		// M6-Q10: a spectator's banner, neutral, once the match is over.
		return bMatchOver ? EHOLEndBanner::MatchOver : EHOLEndBanner::None;
	}
	const uint8 Me = 1 << MySlot;
	if (bMatchOver)
	{
		// D357: a camp wins with its fallen members; no winner is everybody's defeat.
		return (WinnerMask & Me) ? EHOLEndBanner::Victory : EHOLEndBanner::Defeat;
	}
	return (DefeatedMask & Me) ? EHOLEndBanner::Defeat : EHOLEndBanner::None;
}

bool HOLGameMenuRules::OffersReturnToLobby(const EHOLEndBanner Banner, const EHOLMatchOrigin Origin)
{
	return Origin == EHOLMatchOrigin::Lobby && (Banner == EHOLEndBanner::Defeat || Banner == EHOLEndBanner::Victory);
}

bool HOLGameMenuRules::QuitsToList(const EHOLMatchOrigin Origin)
{
	return Origin != EHOLMatchOrigin::Local;
}

bool HOLGameMenuRules::ShouldOpenEndBanner(const EHOLEndBanner Due, const EHOLEndBanner Shown, const EHOLEndBanner Closed)
{
	return Due != EHOLEndBanner::None && Due != Shown && Due != Closed;
}

bool HOLGameMenuRules::KeepsMatchMenuInput(const bool bOpening, const bool bOtherScreenOpen)
{
	return bOpening || bOtherScreenOpen;
}

bool HOLGameMenuRules::ShowsStatsButton(const bool bSpectator, const bool bMatchOver)
{
	return bSpectator || bMatchOver;
}
