// Copyright Epic Games, Inc. All Rights Reserved.

#include "Player/HOLGameMenuRulesBridge.h"

#include "UObject/UObjectIterator.h"

UHOLGameMenuRulesBridge* UHOLGameMenuRulesBridge::Get()
{
	static TWeakObjectPtr<UClass> Cached;
	if (!Cached.IsValid() || Cached->HasAnyClassFlags(CLASS_NewerVersionExists))
	{
		Cached = nullptr;
		// Angelscript's hot reload keeps the older classes, flagged CLASS_NewerVersionExists.
		for (TObjectIterator<UClass> It; It; ++It)
		{
			if (It->IsChildOf(StaticClass()) && !It->HasAnyClassFlags(CLASS_Abstract | CLASS_NewerVersionExists | CLASS_Deprecated))
			{
				Cached = *It;
				break;
			}
		}
	}
	return Cached.IsValid() ? Cast<UHOLGameMenuRulesBridge>(Cached->GetDefaultObject()) : nullptr;
}

namespace
{
	/** The script instance, or nullptr after one warning (the neutral values answer). */
	UHOLGameMenuRulesBridge* Script()
	{
		UHOLGameMenuRulesBridge* Bridge = UHOLGameMenuRulesBridge::Get();
		UE_CLOG(!Bridge, LogTemp, Warning, TEXT("Game menu rules: no script class (Script/UI/HOLGameMenuRulesBridge.as), neutral answers"));
		return Bridge;
	}
}

EHOLEscapeAction UHOLGameMenuRulesBridge::GetEscapeAction(const bool bMenuOpen, const bool bModeArmed, const bool bHasSelection)
{
	UHOLGameMenuRulesBridge* Bridge = Script();
	return Bridge ? static_cast<EHOLEscapeAction>(Bridge->BP_GetEscapeAction(bMenuOpen, bModeArmed, bHasSelection)) : EHOLEscapeAction::OpenMenu;
}

bool UHOLGameMenuRulesBridge::CanSurrender(const bool bDefeated, const bool bMatchOver)
{
	UHOLGameMenuRulesBridge* Bridge = Script();
	return Bridge && Bridge->BP_CanSurrender(bDefeated, bMatchOver);
}

bool UHOLGameMenuRulesBridge::CanQuitToMenu(const bool bDefeated, const bool bMatchOver)
{
	UHOLGameMenuRulesBridge* Bridge = Script();
	return Bridge && Bridge->BP_CanQuitToMenu(bDefeated, bMatchOver);
}

EHOLEndBanner UHOLGameMenuRulesBridge::GetEndBanner(const int32 MySlot, const uint8 DefeatedMask, const bool bMatchOver, const uint8 WinnerMask)
{
	UHOLGameMenuRulesBridge* Bridge = Script();
	return Bridge ? static_cast<EHOLEndBanner>(Bridge->BP_GetEndBanner(MySlot, DefeatedMask, bMatchOver, WinnerMask)) : EHOLEndBanner::None;
}

bool UHOLGameMenuRulesBridge::OffersReturnToLobby(const EHOLEndBanner Banner, const EHOLMatchOrigin Origin)
{
	UHOLGameMenuRulesBridge* Bridge = Script();
	return Bridge && Bridge->BP_OffersReturnToLobby(static_cast<int32>(Banner), static_cast<int32>(Origin));
}

bool UHOLGameMenuRulesBridge::QuitsToList(const EHOLMatchOrigin Origin)
{
	UHOLGameMenuRulesBridge* Bridge = Script();
	return Bridge && Bridge->BP_QuitsToList(static_cast<int32>(Origin));
}

bool UHOLGameMenuRulesBridge::ShouldOpenEndBanner(const EHOLEndBanner Due, const EHOLEndBanner Shown, const EHOLEndBanner Closed)
{
	UHOLGameMenuRulesBridge* Bridge = Script();
	return Bridge && Bridge->BP_ShouldOpenEndBanner(static_cast<int32>(Due), static_cast<int32>(Shown), static_cast<int32>(Closed));
}

bool UHOLGameMenuRulesBridge::KeepsMatchMenuInput(const bool bOpening, const bool bOtherScreenOpen)
{
	UHOLGameMenuRulesBridge* Bridge = Script();
	return Bridge && Bridge->BP_KeepsMatchMenuInput(bOpening, bOtherScreenOpen);
}

bool UHOLGameMenuRulesBridge::ShowsStatsButton(const bool bSpectator, const bool bMatchOver)
{
	UHOLGameMenuRulesBridge* Bridge = Script();
	return Bridge && Bridge->BP_ShowsStatsButton(bSpectator, bMatchOver);
}
