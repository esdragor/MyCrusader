// GLORFINDEL-1125mc (ENONCES-1125m G-6, D839): the FACE of a command grid cell (label, greyed or ready, tooltip, report name), decided in script.
// Today HOLGameScreen.cpp 1300-1372 decides it (HeroAbility, LightSeal, NecroticRage); the C++ hook that asks this rule and keeps only
// GridCell->SetCommand / SetTooltip comes in a later C++ lot (no twin: the C++ cases go when the hook is wired). NOT COMPILED when written.
// The other commands (Produce, Build, Research, Trade, Deploy, ...) are not decided here yet: their face stays empty (the C++ switch still draws them).

enum EHOLGridFaceState
{
	Ready,
	Greyed,
}

// What the cell's command does not carry (filled by the hook from the game state): the seconds left of the cool-down shown under the label,
// the hero level an ability needs (0 when it is unlocked), and the numbers of the tooltip (HOLZoneEffect's for the Seal: radius m, duration s, slow %;
// the simulation's for the rage: duration s, +damage %, +rate %, HP lost per hit, refused under HP, stops at HP, recharge s).
struct FHOLGridFaceFacts
{
	int CooldownSeconds = 0;
	int UnlockLevel = 0;
	TArray<int> Numbers;
}

struct FHOLGridCellLookView
{
	FText Label;
	EHOLGridFaceState State = EHOLGridFaceState::Ready;
	FText TooltipName;
	FText TooltipEffect;
	FName ReportName;
}

namespace HOLCommandGridFace
{
	// HALDIR-1127ar3: the pattern of a label with its seconds under it (fr / en "{0}\n{1} s", ja "{0}\n{1}秒"). Key of the former C++ face
	// (HOLGameScreen.cpp, kept so that its translations stay: Tools/Localization/fill_po.py).
	FText CooldownPattern()
	{
		return NSLOCTEXT("HOLGameScreen", "HeroAbilityCooldown", "{0}\n{1} s");
	}

	// HALDIR-1127ar3: the pattern of the rage's tooltip, {0}..{6} = FHOLGridFaceFacts::Numbers (key of the former C++ face).
	FText RageEffectPattern()
	{
		return NSLOCTEXT("HOLGameScreen", "NecroticRageEffect", "{0} s : +{1} % de dégâts, +{2} % de cadence, {3} PV perdus par coup porté. Refusée sous {4} PV, elle s'arrête à {5} PV ; recharge {6} s");
	}

	// A localized pattern filled: {0} and {1} replaced (the culture's order, separators and units kept; HOLDiplomacyPanelText::Format's way).
	FString Fill(const FString& Pattern, const FString& First, const FString& Second = "")
	{
		return Pattern.Replace("{0}", First).Replace("{1}", Second);
	}

	// A localized pattern filled with the facts' numbers: {N} = Facts.Numbers[N].
	FString FillNumbers(const FString& Pattern, const FHOLGridFaceFacts& Facts)
	{
		FString Out = Pattern;
		for (int Index = 0; Index < Facts.Numbers.Num(); Index++)
		{
			Out = Out.Replace("{" + Index + "}", "" + Facts.Numbers[Index]);
		}
		return Out;
	}

	// The label with the seconds left under it ("Sceau\n12 s"), HOLGameScreen.cpp 1337 / 1364 / 1317.
	FText WithCooldown(const FText& Short, int Seconds)
	{
		if (Seconds <= 0)
		{
			return Short;
		}
		return FText::FromString(Fill(CooldownPattern().ToString(), Short.ToString(), "" + Seconds));
	}

	int NumberAt(const FHOLGridFaceFacts& Facts, int Index)
	{
		if (Index < 0 || Index >= Facts.Numbers.Num())
		{
			return 0;
		}
		return Facts.Numbers[Index];
	}

	// G-6: the face of the cell of this command. Seal / rage: the seconds left (Facts.CooldownSeconds) grey the cell; hero ability: the seconds are the
	// command's own (CooldownSeconds), a locked one shows the level it needs instead of its effect and no seconds.
	FHOLGridCellLookView GetCellFace(const FHOLGridCommandView& Command, const FHOLGridFaceFacts& Facts)
	{
		FHOLGridCellLookView Result;
		Result.State = Command.bGreyed ? EHOLGridFaceState::Greyed : EHOLGridFaceState::Ready;
		if (Command.Command == EHOLGridCommandKind::LightSeal)
		{
			Result.Label = WithCooldown(NSLOCTEXT("HOLCommandGridFace", "LightSealShort", "Sceau"), Facts.CooldownSeconds);
			if (Facts.CooldownSeconds > 0)
			{
				Result.State = EHOLGridFaceState::Greyed;
			}
			Result.ReportName = n"LightSeal";
			Result.TooltipName = NSLOCTEXT("HOLCommandGridFace", "LightSealName", "Sceau de lumière");
			Result.TooltipEffect = FText::FromString(FillNumbers(NSLOCTEXT("HOLCommandGridFace", "LightSealEffect", "Puis clic au sol, à 8 m du Moine au plus : une zone de {0} m de rayon pendant {1} s ; les ennemis dedans sont ralentis de {2} % ; recharge 40 s").ToString(), Facts));
		}
		else if (Command.Command == EHOLGridCommandKind::KnightChallenge)
		{
			// D923: label "Defi" with the seconds left under it, full name in the tooltip; the numbers (8 m, 8 s, 45 s) are the table row KnightChallenge (UnitAbilities.csv).
			Result.Label = WithCooldown(NSLOCTEXT("HOLCommandGridFace", "KnightChallengeShort", "Défi"), Facts.CooldownSeconds);
			if (Facts.CooldownSeconds > 0)
			{
				Result.State = EHOLGridFaceState::Greyed;
			}
			Result.ReportName = n"KnightChallenge";
			Result.TooltipName = NSLOCTEXT("HOLCommandGridFace", "KnightChallengeName", "Défi du Chevalier");
			Result.TooltipEffect = HOLUnitAbilityTable::Description(HOLUnitAbilityTable::KnightChallenge);
		}
		else if (Command.Command == EHOLGridCommandKind::NecroticRage)
		{
			Result.Label = WithCooldown(NSLOCTEXT("HOLCommandGridFace", "NecroticRageShort", "Rage"), Facts.CooldownSeconds);
			if (Facts.CooldownSeconds > 0)
			{
				Result.State = EHOLGridFaceState::Greyed;
			}
			Result.ReportName = n"NecroticRage";
			Result.TooltipName = NSLOCTEXT("HOLCommandGridFace", "NecroticRageName", "Rage nécrotique");
			Result.TooltipEffect = FText::FromString(FillNumbers(RageEffectPattern().ToString(), Facts));
		}
		else if (Command.Command == EHOLGridCommandKind::HeroAbility)
		{
			// Ability numbers: Aura 0, CloseRanks 1, Banner 2 (FHOLGridCommandView::Ability).
			bool bBanner = Command.Ability == 2;
			bool bLocked = Facts.UnlockLevel > 0;
			FText Short = bBanner ? NSLOCTEXT("HOLCommandGridFace", "BannerShort", "Bannière") : NSLOCTEXT("HOLCommandGridFace", "CloseRanksShort", "Rangs");
			Result.Label = bLocked ? Short : WithCooldown(Short, Command.CooldownSeconds);
			Result.ReportName = bBanner ? n"Banner" : n"CloseRanks";
			Result.TooltipName = bBanner ? NSLOCTEXT("HOLCommandGridFace", "BannerName", "Bannière de l'Aube") : NSLOCTEXT("HOLCommandGridFace", "CloseRanksName", "Serrez les rangs");
			if (bLocked)
			{
				Result.TooltipEffect = FText::FromString(Fill(NSLOCTEXT("HOLGameScreen", "HeroAbilityLocked", "Niveau {0} requis").ToString(), "" + Facts.UnlockLevel));
			}
			else if (bBanner)
			{
				Result.TooltipEffect = NSLOCTEXT("HOLGameScreen", "BannerEffect", "Puis clic au sol, à 16 m du héros au plus : son aura (−15 % de dégâts reçus à 12 m) pendant 45 s ; recharge 120 s");
			}
			else
			{
				Result.TooltipEffect = NSLOCTEXT("HOLGameScreen", "CloseRanksEffect", "Immobilise 15 s les unités à 12 m du héros (formation défensive) : elles restent sur place et reçoivent −25 % de dégâts ; recharge 60 s");
			}
		}
		return Result;
	}
}
