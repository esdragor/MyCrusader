// The hero's block of the HUD as a view (J8a.5 loop 3): what it shows, from what it knows (pure, no world), applied to the
// widgets by UHOLGameScreenScript. Texts are keys and numbers (HOLHeroBlockText turns them into the current language's
// texts), Describe the stable line of the tests and hol.HUD.Report.
struct FHOLHeroBlockView
{
	bool bVisible = false;
	bool bPortraitDark = false;
	int Level = 0;
	// The HP: 0 while dead, whatever the sheet says.
	int Health = 0;
	int MaxHealth = 0;
	float HealthFraction = 0.0;
	// The XP: -1 unknown; its bar empty and greyed until the levels have thresholds.
	int XP = -1;
	float XPFraction = 0.0;
	bool bXPGreyed = true;
	int NumAbilities = 3;
	bool bAbilitiesGreyed = true;
	// HOLHeroBlockText::ReadyIn ("Prêt dans m:ss", DelayClock the m:ss), Unknown ("…"), "" none.
	FString DelayKey;
	FString DelayClock;
	int SecondsToReady = -1;
	// The return's button (ready): its text HOLHeroBlockText::Return with Price, greyed with a tooltip when not buyable.
	bool bButtonVisible = false;
	bool bButtonEnabled = false;
	FString ButtonKey;
	int Price = -1;
	bool bPriceRed = false;
	// HOLHeroBlockText::NoPlace, NoGold, "" none.
	FString TooltipKey;
	// hidden, alive level L hp H/M, dead ready in S s, ready price P buyable / no gold / no place, unknown.
	FString Describe = "hidden";
}

namespace HOLHeroBlock
{
	// The portrait's emblem (Tools/Editor/make_ui_icons.py, cooked with /Game/UI/Icons): a white shield edged in gold with a
	// gold sun, transparent around it (the player's colour under it).
	const FString EmblemPath = "/Game/UI/Icons/T_Icon_HeroEmblem.T_Icon_HeroEmblem";

	UTexture2D LoadEmblem()
	{
		return Cast<UTexture2D>(LoadObject(nullptr, EmblemPath));
	}

	// The view from what the block knows: bSpectator (no slot), bDefeated, State, Level, Health and MaxHealth of the public
	// sheet, XP (-1 unknown), Quote the return's (HOLHeroReturnQuote::Read). The quote, not the sheet, says ready: an order
	// sent now is priced at the next step.
	FHOLHeroBlockView Make(bool bSpectator, bool bDefeated, EHOLHeroState State, int Level, int Health, int MaxHealth, int XP,
		FHOLHeroReturnQuote Quote)
	{
		FHOLHeroBlockView View;
		if (bSpectator || bDefeated || State == EHOLHeroState::None)
			return View;
		View.bVisible = true;
		View.Level = Level;
		View.MaxHealth = Math::Max(0, MaxHealth);
		View.XP = XP;
		if (State == EHOLHeroState::Alive)
		{
			View.Health = Math::Clamp(Health, 0, View.MaxHealth);
			View.HealthFraction = View.MaxHealth > 0 ? float(View.Health) / float(View.MaxHealth) : 0.0;
			View.Describe = f"alive level {Level} hp {View.Health}/{View.MaxHealth}";
			return View;
		}
		View.bPortraitDark = true;
		if (Quote.State == EHOLHeroReturnQuoteState::Waiting)
		{
			View.DelayKey = HOLHeroBlockText::ReadyIn;
			View.SecondsToReady = Quote.SecondsToReady;
			View.DelayClock = Clock(Quote.SecondsToReady);
			View.Describe = f"dead ready in {Quote.SecondsToReady} s";
		}
		else if (Quote.State == EHOLHeroReturnQuoteState::Ready)
		{
			View.SecondsToReady = 0;
			View.bButtonVisible = true;
			View.bButtonEnabled = Quote.bCanBuy;
			View.ButtonKey = HOLHeroBlockText::Return;
			View.Price = Quote.Price;
			if (!Quote.bHasPlace)
			{
				View.TooltipKey = HOLHeroBlockText::NoPlace;
				View.Describe = f"ready price {Quote.Price} no place";
			}
			else if (!Quote.bCanBuy)
			{
				// Gold short (or not known here: never said buyable).
				View.bPriceRed = true;
				View.TooltipKey = HOLHeroBlockText::NoGold;
				View.Describe = f"ready price {Quote.Price} no gold";
			}
			else
				View.Describe = f"ready price {Quote.Price} buyable";
		}
		else
		{
			// Unknown here (no death step, no server step yet): nothing invented.
			View.DelayKey = HOLHeroBlockText::Unknown;
			View.Describe = "unknown";
		}
		return View;
	}

	// Seconds as m:ss (0:23, 1:01, 12:05).
	FString Clock(int Seconds)
	{
		const int Total = Math::Max(0, Seconds);
		const int Minutes = Math::IntegerDivisionTrunc(Total, 60);
		const int Rest = Total - Minutes * 60;
		return Rest < 10 ? f"{Minutes}:0{Rest}" : f"{Minutes}:{Rest}";
	}
}

// The block's texts by key, in French until a string table holds them (each key its entry, {0} and {1} its arguments).
namespace HOLHeroBlockText
{
	const FString Level = "HeroBlock.Level";
	const FString Health = "HeroBlock.Health";
	const FString ReadyIn = "HeroBlock.ReadyIn";
	const FString Unknown = "HeroBlock.Unknown";
	const FString Return = "HeroBlock.Return";
	const FString NoGold = "HeroBlock.NoGold";
	const FString NoPlace = "HeroBlock.NoPlace";

	// The pattern of a key in the current language ("" for none or an unknown key). Localized texts of the game:
	// French source here, gathered from the .as files by GatherText (Config/Localization/Game.ini, Script/), English and
	// Japanese in Tools/Localization/fill_po.py (keys "HOLHeroBlock,<Name>"). Literals only in these calls.
	FText PatternText(FString Key)
	{
		if (Key == Level) return NSLOCTEXT("HOLHeroBlock", "Level", "Niveau {0}");
		if (Key == Health) return NSLOCTEXT("HOLHeroBlock", "Health", "PV {0} / {1}");
		if (Key == ReadyIn) return NSLOCTEXT("HOLHeroBlock", "ReadyIn", "Prêt dans {0}");
		if (Key == Unknown) return NSLOCTEXT("HOLHeroBlock", "Unknown", "…");
		if (Key == Return) return NSLOCTEXT("HOLHeroBlock", "Return", "Ramener le héros — {0} or");
		if (Key == NoGold) return NSLOCTEXT("HOLHeroBlock", "NoGold", "Pas assez d'or");
		if (Key == NoPlace) return NSLOCTEXT("HOLHeroBlock", "NoPlace", "Aucun forum pour revenir");
		return FText();
	}

	FString Pattern(FString Key)
	{
		return PatternText(Key).ToString();
	}

	// {0} and {1} replaced by the numbers as they are (no grouping by the culture: "1000", as the tests read them).
	FString Format(FString Key, FString First = "", FString Second = "")
	{
		return Pattern(Key).Replace("{0}", First).Replace("{1}", Second);
	}

	FString LevelText(FHOLHeroBlockView View) { return View.bVisible ? Format(Level, f"{View.Level}") : ""; }
	FString HealthText(FHOLHeroBlockView View) { return View.bVisible ? Format(Health, f"{View.Health}", f"{View.MaxHealth}") : ""; }
	FString DelayText(FHOLHeroBlockView View) { return Format(View.DelayKey, View.DelayClock); }
	FString ButtonText(FHOLHeroBlockView View) { return View.bButtonVisible ? Format(View.ButtonKey, f"{View.Price}") : ""; }
	FString TooltipText(FHOLHeroBlockView View) { return Format(View.TooltipKey); }
}
