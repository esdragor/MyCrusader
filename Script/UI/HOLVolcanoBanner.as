// The volcano on the HUD (V10, GDD 12.1 and 12.2 bis, D28, D377, D380): the line of each site, in script. What the player may read: the alert's countdown
// (visible to all from the alert on, D377, D381), the lava while it advances and cools, and the ancient altar while someone holds or contests it
// (D380: held ~60 s without an adversary in the zone, like the ruin, D348). Nothing before the alert: a site that is only armed gives no warning (12.2).
// The view is pure (from the numbers UHOLVolcanoLibrary hands over); UHOLGameScreenScript puts the lines in a panel. Describe is the stable line of the tests.
struct FHOLVolcanoBannerView
{
	bool bVisible = false;
	// One line per site worth a line, "\n" apart.
	FString Text = "";
	// volcano <n>: <phase> ...; or hidden.
	FString Describe = "hidden";
}

// EHOLVolcanoPhase as the library gives it (HOLVolcanoRules.h): 0 Dormant, 1 Armed, 2 Warning, 3 Erupting, 4 Cooling, 5 Rest.
namespace HOLVolcanoBannerPhase
{
	const int Armed = 1;
	const int Warning = 2;
	const int Erupting = 3;
	const int Cooling = 4;
}

// The texts by key (French source here, gathered from the .as files by GatherText; English and Japanese in Tools/Localization/fill_po.py, keys
// "HOLVolcanoBanner,<Name>"). Literals only in these calls. Provisional wording (the GDD names no label): to confirm with the user.
namespace HOLVolcanoBannerText
{
	const FString Alert = "Volcano.Alert";
	const FString Erupting = "Volcano.Erupting";
	const FString Cooling = "Volcano.Cooling";
	const FString AltarHeld = "Volcano.AltarHeld";
	const FString AltarContested = "Volcano.AltarContested";
	const FString AltarIdle = "Volcano.AltarIdle";
	const FString SiteNumber = "Volcano.SiteNumber";

	FText PatternText(FString Key)
	{
		if (Key == Alert) return NSLOCTEXT("HOLVolcanoBanner", "Alert", "Volcan : éruption dans {0}");
		if (Key == Erupting) return NSLOCTEXT("HOLVolcanoBanner", "Erupting", "Volcan : la lave avance");
		if (Key == Cooling) return NSLOCTEXT("HOLVolcanoBanner", "Cooling", "Volcan : la lave refroidit");
		if (Key == AltarHeld) return NSLOCTEXT("HOLVolcanoBanner", "AltarHeld", "Autel ancien : tenu {0} % ({1} s)");
		if (Key == AltarContested) return NSLOCTEXT("HOLVolcanoBanner", "AltarContested", "Autel ancien : disputé ({0} %)");
		if (Key == AltarIdle) return NSLOCTEXT("HOLVolcanoBanner", "AltarIdle", "Autel ancien : abandonné ({0} %)");
		if (Key == SiteNumber) return NSLOCTEXT("HOLVolcanoBanner", "SiteNumber", "Site {0} - {1}");
		return FText();
	}

	FString Format(FString Key, FString First = "", FString Second = "")
	{
		return PatternText(Key).ToString().Replace("{0}", First).Replace("{1}", Second);
	}
}

namespace HOLVolcanoBanner
{
	// The line of one site, "" when it gives nothing to read.
	FString SiteLine(FHOLVolcanoSiteView Site, FString& OutDescribe)
	{
		FString Line = "";
		if (Site.Phase == HOLVolcanoBannerPhase::Warning)
		{
			Line = HOLVolcanoBannerText::Format(HOLVolcanoBannerText::Alert, HOLHeroBlock::Clock(Site.SecondsToErupt));
			OutDescribe = f"alert {Site.SecondsToErupt}";
		}
		else if (Site.Phase == HOLVolcanoBannerPhase::Erupting)
		{
			Line = HOLVolcanoBannerText::Format(HOLVolcanoBannerText::Erupting);
			OutDescribe = "erupting";
		}
		else if (Site.Phase == HOLVolcanoBannerPhase::Cooling)
		{
			Line = HOLVolcanoBannerText::Format(HOLVolcanoBannerText::Cooling);
			OutDescribe = "cooling";
		}
		else if (Site.bAltarPresent && Site.bAltarLit && (Site.AltarHold != 0 || Site.AltarPercent > 0))
		{
			// Lit and someone at work on it (or the count not yet run down): held, contested (2) or left (0).
			if (Site.AltarHold == 2)
			{
				Line = HOLVolcanoBannerText::Format(HOLVolcanoBannerText::AltarContested, f"{Site.AltarPercent}");
				OutDescribe = f"altar contested {Site.AltarPercent}";
			}
			else if (Site.AltarHold == 1)
			{
				Line = HOLVolcanoBannerText::Format(HOLVolcanoBannerText::AltarHeld, f"{Site.AltarPercent}", f"{Site.AltarSecondsLeft}");
				OutDescribe = f"altar held {Site.AltarPercent} left {Site.AltarSecondsLeft}";
			}
			else
			{
				Line = HOLVolcanoBannerText::Format(HOLVolcanoBannerText::AltarIdle, f"{Site.AltarPercent}");
				OutDescribe = f"altar idle {Site.AltarPercent}";
			}
		}
		return Line;
	}

	// The panel's lines for the sites given (one view each). With several sites each line says which one.
	FHOLVolcanoBannerView Make(TArray<FHOLVolcanoSiteView> Sites)
	{
		FHOLVolcanoBannerView View;
		FString Describe = "";
		for (int Index = 0; Index < Sites.Num(); Index++)
		{
			FString Part = "";
			FString Line = SiteLine(Sites[Index], Part);
			if (Line.IsEmpty())
				continue;
			if (Sites.Num() > 1)
				Line = HOLVolcanoBannerText::Format(HOLVolcanoBannerText::SiteNumber, f"{Index + 1}", Line);
			View.Text += (View.Text.IsEmpty() ? "" : "\n") + Line;
			Describe += (Describe.IsEmpty() ? "" : "; ") + f"volcano {Index + 1}: {Part}";
		}
		View.bVisible = !View.Text.IsEmpty();
		if (View.bVisible)
			View.Describe = Describe;
		return View;
	}
}
