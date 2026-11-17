// The C++ callers' read of the in-game menu's rules (MIG-C-GameMenu, HALDIR-1117h, D836: the menu's rules go to script). UHOLGameMenuRulesBridge
// (Source/HeroesOfLiberty/Player/HOLGameMenuRulesBridge.h) finds this class and calls its overrides from HOLPlayerController (Escape),
// AHOLGameHUD and UHOLGameScreen (end banner, « Statistiques »), UHOLGameMenuScreen (« Abandonner », the way out, the menu input). Envelopes only:
// each override is one call of HOLGameMenuRules.as or HOLEndBannerScript.as, the single source of the rule; enumerations travel as ints.
class UHOLGameMenuRulesScript : UHOLGameMenuRulesBridge
{
	UFUNCTION(BlueprintOverride)
	int GetEscapeAction(bool bMenuOpen, bool bModeArmed, bool bHasSelection)
	{
		return int(HOLGameMenuRules::GetEscapeAction(bMenuOpen, bModeArmed, bHasSelection));
	}

	UFUNCTION(BlueprintOverride)
	bool CanSurrender(bool bDefeated, bool bMatchOver)
	{
		return HOLGameMenuRules::CanSurrender(bDefeated, bMatchOver);
	}

	UFUNCTION(BlueprintOverride)
	bool CanQuitToMenu(bool bDefeated, bool bMatchOver)
	{
		return HOLGameMenuRules::CanQuitToMenu(bDefeated, bMatchOver);
	}

	UFUNCTION(BlueprintOverride)
	int GetEndBanner(int MySlot, int DefeatedMask, bool bMatchOver, int WinnerMask)
	{
		return int(HOLEndBannerScript::GetEndBanner(MySlot, DefeatedMask, bMatchOver, WinnerMask));
	}

	UFUNCTION(BlueprintOverride)
	bool OffersReturnToLobby(int Banner, int Origin)
	{
		return HOLGameMenuRules::OffersReturnToLobby(EHOLGameMenuBanner(Banner), EHOLGameMenuOrigin(Origin));
	}

	UFUNCTION(BlueprintOverride)
	bool QuitsToList(int Origin)
	{
		return HOLGameMenuRules::QuitsToList(EHOLGameMenuOrigin(Origin));
	}

	UFUNCTION(BlueprintOverride)
	bool ShouldOpenEndBanner(int Due, int Shown, int Closed)
	{
		return HOLEndBannerScript::ShouldOpenEndBanner(EHOLGameMenuBanner(Due), EHOLGameMenuBanner(Shown), EHOLGameMenuBanner(Closed));
	}

	UFUNCTION(BlueprintOverride)
	bool KeepsMatchMenuInput(bool bOpening, bool bOtherScreenOpen)
	{
		return HOLGameMenuRules::KeepsMatchMenuInput(bOpening, bOtherScreenOpen);
	}

	UFUNCTION(BlueprintOverride)
	bool ShowsStatsButton(bool bSpectator, bool bMatchOver)
	{
		return HOLGameMenuRules::ShowsStatsButton(bSpectator, bMatchOver);
	}
}
