// One of the hero's block's three abilities' boxes (J8c.4, J8c-Q4): its icon lit (the aura on, an ability ready) or dark;
// locked below its level (the level in its tooltip); cooling down, the seconds left on it and a dark curtain over the
// share of the cool-down left (J8-Q5: the owner's private sheet).
struct FHOLHeroAbilityBox
{
	EHOLHeroAbility Ability = EHOLHeroAbility::Aura;
	bool bLit = false;
	bool bLocked = false;
	int UnlockLevel = 1;
	int SecondsLeft = 0;
	float CooldownFraction = 0.0;
	// "aura on", "aura off", "cry ready", "cry 42 s", "cry off", "banner locked 4", "banner 118 s".
	FString Describe;
}

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
	// The XP (J8b.4, J8b-Q4), to its owner alone: -1 unknown, its bar empty and greyed. Its bar filled within the level
	// (from its threshold to the next one's, full at the last level); XPText its tooltip ("420 / 600", "3400" at the last
	// level); XPDescribe the stable line ("xp 420/600", "xp 3400 max", "xp unknown").
	int XP = -1;
	float XPFraction = 0.0;
	bool bXPGreyed = true;
	int XPNext = -1;
	FString XPText;
	FString XPDescribe = "xp unknown";
	int NumAbilities = 3;
	// The abilities' boxes, aura, cry, banner (HOLHeroBlock::SetAbilities; empty: all dark).
	TArray<FHOLHeroAbilityBox> Abilities;
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
		SetXP(View, Level, XP);
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

	// The XP's bar of a hero of Level with XP (-1 unknown), alive or dead (development goes on while dead, D06).
	void SetXP(FHOLHeroBlockView& View, int Level, int XP)
	{
		View.XP = XP;
		if (XP < 0)
			return;
		View.bXPGreyed = false;
		if (Level >= HOLHero::GetMaxLevel())
		{
			View.XPFraction = 1.0;
			View.XPText = f"{XP}";
			View.XPDescribe = f"xp {XP} max";
			return;
		}
		const int Start = HOLHero::GetLevelXP(Level);
		View.XPNext = HOLHero::GetLevelXP(Level + 1);
		View.XPFraction = View.XPNext > Start ? Math::Clamp(float(XP - Start) / float(View.XPNext - Start), 0.0, 1.0) : 0.0;
		View.XPText = f"{XP} / {View.XPNext}";
		View.XPDescribe = f"xp {XP}/{View.XPNext}";
	}

	// The box of Ability for a hero alive or not (bAlive), of Level, the ability ready at ReadyStep (0 never cast), the
	// server at ServerStep (-1 unknown: nothing counted), RateHz steps a second, CooldownSeconds its whole cool-down.
	FHOLHeroAbilityBox MakeAbility(EHOLHeroAbility Ability, bool bAlive, int Level, int ReadyStep, int ServerStep, int RateHz, int CooldownSeconds)
	{
		FHOLHeroAbilityBox Box;
		Box.Ability = Ability;
		const FString Name = AbilityWord(Ability);
		if (Ability == EHOLHeroAbility::Aura)
		{
			// Passive, on while he lives (§ 9.8).
			Box.bLit = bAlive;
			Box.Describe = Name + (bAlive ? " on" : " off");
			return Box;
		}
		Box.UnlockLevel = HOLHero::GetHeroAbilityUnlockLevel(Ability);
		if (Level < Box.UnlockLevel)
		{
			Box.bLocked = true;
			Box.Describe = f"{Name} locked {Box.UnlockLevel}";
			return Box;
		}
		// The steps left, rounded up to the second (one step left: 1 s); the server's step unknown: nothing counted.
		const int StepsLeft = ServerStep >= 0 ? Math::Max(0, ReadyStep - ServerStep) : 0;
		const int Rate = Math::Max(1, RateHz);
		Box.SecondsLeft = Math::IntegerDivisionTrunc(StepsLeft + Rate - 1, Rate);
		Box.CooldownFraction = CooldownSeconds > 0 ? Math::Clamp(float(StepsLeft) / float(CooldownSeconds * Rate), 0.0, 1.0) : 0.0;
		Box.bLit = bAlive && Box.SecondsLeft == 0;
		Box.Describe = Box.SecondsLeft > 0 ? f"{Name} {Box.SecondsLeft} s" : Name + (bAlive ? " ready" : " off");
		return Box;
	}

	// The three boxes of the view (aura, cry, banner), ReadySteps indexed by EHOLHeroAbility.
	void SetAbilities(FHOLHeroBlockView& View, bool bAlive, int Level, TArray<int> ReadySteps, int ServerStep, int RateHz)
	{
		View.Abilities.Reset();
		for (int Index = 0; Index < 3; ++Index)
		{
			const EHOLHeroAbility Ability = AbilityAt(Index);
			const int ReadyStep = ReadySteps.IsValidIndex(Index) ? ReadySteps[Index] : 0;
			View.Abilities.Add(MakeAbility(Ability, bAlive, Level, ReadyStep, ServerStep, RateHz, HOLHero::GetHeroAbilityCooldownSeconds(Ability)));
		}
	}

	// The ability of the box at Index: aura, cry, banner (EHOLHeroAbility's order).
	EHOLHeroAbility AbilityAt(int Index)
	{
		return Index == 0 ? EHOLHeroAbility::Aura : Index == 1 ? EHOLHeroAbility::CloseRanks : EHOLHeroAbility::Banner;
	}

	// The word of the stable lines: aura, cry, banner.
	FString AbilityWord(EHOLHeroAbility Ability)
	{
		return Ability == EHOLHeroAbility::Aura ? "aura" : Ability == EHOLHeroAbility::CloseRanks ? "cry" : "banner";
	}

	// The abilities' icons (Tools/Editor/make_ui_icons.py, cooked with /Game/UI/Icons), aura, cry, banner.
	UTexture2D LoadAbilityIcon(EHOLHeroAbility Ability)
	{
		const FString Name = Ability == EHOLHeroAbility::Aura ? "T_Icon_Aura" : Ability == EHOLHeroAbility::CloseRanks ? "T_Icon_CloseRanks" : "T_Icon_Banner";
		return Cast<UTexture2D>(LoadObject(nullptr, f"/Game/UI/Icons/{Name}.{Name}"));
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
	const FString AuraName = "HeroBlock.AuraName";
	const FString AuraEffect = "HeroBlock.AuraEffect";
	const FString CloseRanksName = "HeroBlock.CloseRanksName";
	const FString CloseRanksEffect = "HeroBlock.CloseRanksEffect";
	const FString BannerName = "HeroBlock.BannerName";
	const FString BannerEffect = "HeroBlock.BannerEffect";
	const FString AbilityLocked = "HeroBlock.AbilityLocked";
	const FString AbilityCooldown = "HeroBlock.AbilityCooldown";

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
		if (Key == AuraName) return NSLOCTEXT("HOLHeroBlock", "AuraName", "Aura de l'Aube");
		if (Key == AuraEffect) return NSLOCTEXT("HOLHeroBlock", "AuraEffect", "Passive : −15 % de dégâts reçus pour vos unités à 12 m du héros");
		if (Key == CloseRanksName) return NSLOCTEXT("HOLHeroBlock", "CloseRanksName", "Serrez les rangs (A)");
		if (Key == CloseRanksEffect) return NSLOCTEXT("HOLHeroBlock", "CloseRanksEffect", "−25 % de dégâts reçus pendant 15 s pour vos unités à 12 m du héros ; recharge 60 s");
		if (Key == BannerName) return NSLOCTEXT("HOLHeroBlock", "BannerName", "Bannière de l'Aube (Z)");
		if (Key == BannerEffect) return NSLOCTEXT("HOLHeroBlock", "BannerEffect", "Plantée à 16 m du héros au plus : son aura pendant 45 s ; recharge 120 s");
		if (Key == AbilityLocked) return NSLOCTEXT("HOLHeroBlock", "AbilityLocked", "Niveau {0} requis");
		if (Key == AbilityCooldown) return NSLOCTEXT("HOLHeroBlock", "AbilityCooldown", "Prête dans {0} s");
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

	// An ability's box's tooltip: its name, its effect, then locked or the seconds left.
	FString AbilityTooltip(FHOLHeroAbilityBox Box)
	{
		const bool bAura = Box.Ability == EHOLHeroAbility::Aura;
		const bool bCry = Box.Ability == EHOLHeroAbility::CloseRanks;
		FString Text = Format(bAura ? AuraName : bCry ? CloseRanksName : BannerName) + "\n" + Format(bAura ? AuraEffect : bCry ? CloseRanksEffect : BannerEffect);
		if (Box.bLocked)
			Text += "\n" + Format(AbilityLocked, f"{Box.UnlockLevel}");
		else if (Box.SecondsLeft > 0)
			Text += "\n" + Format(AbilityCooldown, f"{Box.SecondsLeft}");
		return Text;
	}
}
