// The sacred sites on the HUD (S10, D431, D703, D747): the banner "Victoire sacrée" and its announcements at 60 s and 10 s, in script. Public, shown to all
// from the moment a camp holds every site: the camp in the lead, the clock before its victory, "en attente" while an adverse unit stands in a zone, and the
// win once done. The view is pure (from the numbers HOLSite::GetVictoryCount hands over, written by the server each step); UHOLGameScreenScript puts the
// line under the volcano's. Describe is the stable line of the tests.
struct FHOLSiteBannerView
{
	bool bVisible = false;
	FString Text = "";
	// sacred leader <slot> left <s> [frozen]; sacred won <slot>; or hidden.
	FString Describe = "hidden";
}

// The texts by key (French source here, gathered from the .as files by GatherText; English and Japanese in Tools/Localization/fill_po.py, keys
// "HOLSiteBanner,<Name>"). Literals only in these calls. Provisional wording (the GDD names no label): to confirm with the user.
namespace HOLSiteBannerText
{
	const FString Countdown = "Sites.Countdown";
	const FString Frozen = "Sites.Frozen";
	const FString Won = "Sites.Won";
	const FString Announce60 = "Sites.Announce60";
	const FString Announce10 = "Sites.Announce10";
	const FString Gold = "Sites.Gold";

	FText PatternText(FString Key)
	{
		if (Key == Gold) return NSLOCTEXT("HOLSiteBanner", "Gold", "+{0} or/min");
		if (Key == Countdown) return NSLOCTEXT("HOLSiteBanner", "Countdown", "Victoire sacrée : {0} dans {1}");
		if (Key == Frozen) return NSLOCTEXT("HOLSiteBanner", "Frozen", "Victoire sacrée : {0} dans {1} (en attente)");
		if (Key == Won) return NSLOCTEXT("HOLSiteBanner", "Won", "Victoire sacrée : {0}");
		if (Key == Announce60) return NSLOCTEXT("HOLSiteBanner", "Announce60", "Sites sacrés : {0} gagne dans une minute");
		if (Key == Announce10) return NSLOCTEXT("HOLSiteBanner", "Announce10", "Sites sacrés : {0} gagne dans 10 secondes");
		return FText();
	}

	FString Format(FString Key, FString First = "", FString Second = "")
	{
		return PatternText(Key).ToString().Replace("{0}", First).Replace("{1}", Second);
	}
}

namespace HOLSiteBanner
{
	// The banner for the count as the game state gives it (Leader INDEX_NONE: nobody holds every site; Winner >= 0: won).
	FHOLSiteBannerView Make(int Leader, int SecondsLeft, bool bFrozen, int Winner, FString LeaderName)
	{
		FHOLSiteBannerView View;
		if (Winner >= 0)
		{
			View.bVisible = true;
			View.Text = HOLSiteBannerText::Format(HOLSiteBannerText::Won, LeaderName);
			View.Describe = f"sacred won {Winner}";
		}
		else if (Leader >= 0 && SecondsLeft >= 0)
		{
			View.bVisible = true;
			View.Text = HOLSiteBannerText::Format(bFrozen ? HOLSiteBannerText::Frozen : HOLSiteBannerText::Countdown, LeaderName, HOLHeroBlock::Clock(SecondsLeft));
			View.Describe = f"sacred leader {Leader} left {SecondsLeft}" + (bFrozen ? " frozen" : "");
		}
		return View;
	}

	// 60 or 10 when the count passes that many seconds between two reads (once each, a jump over the threshold counts), else 0. Without a previous
	// number (-1: the count has just appeared) or on the win (0): nothing.
	int Announce(int PreviousSeconds, int Seconds)
	{
		if (PreviousSeconds < 0 || Seconds < 1)
			return 0;
		if (PreviousSeconds > 60 && Seconds <= 60)
			return 60;
		if (PreviousSeconds > 10 && Seconds <= 10)
			return 10;
		return 0;
	}

	// S6: the income of the sites held, gold a minute (each held site pays the rate to its owner alone, D699, D748; nothing held or a zero rate pays 0).
	int GoldPerMinute(int OwnedSites, int RatePerMinute)
	{
		return Math::Max(0, OwnedSites) * Math::Max(0, RatePerMinute);
	}

	// S6: "+N or/min" (N the total of the sites held), or empty when nothing is held or paid.
	FString GoldLine(int OwnedSites, int RatePerMinute)
	{
		const int Total = GoldPerMinute(OwnedSites, RatePerMinute);
		if (Total <= 0)
			return "";
		return HOLSiteBannerText::Format(HOLSiteBannerText::Gold, f"{Total}");
	}

	FString AnnounceText(int Threshold, FString LeaderName)
	{
		if (Threshold == 60)
			return HOLSiteBannerText::Format(HOLSiteBannerText::Announce60, LeaderName);
		if (Threshold == 10)
			return HOLSiteBannerText::Format(HOLSiteBannerText::Announce10, LeaderName);
		return "";
	}
}
