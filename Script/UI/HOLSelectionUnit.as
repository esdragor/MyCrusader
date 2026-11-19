// NOT COMPILED YET (HALDIR-1119q, H-9 lot B of the HUD plan): this file calls bindings that do not exist before the next C++ lot (HOLSelectionUnitRead::
// GetUnitSelectionView, GetUnitStatsRow, GetUnitStatsMode, GetUnitStatsBuildingMode, GetUnitName, FormatStat: Source\HeroesOfLibertyUI\HUD\HOLSelectionUnitReadLibrary.h/.cpp)
// and the class UHOLSelectionPanel of the lot A (Script\UI\HOLSelection.as: ChipsHost, StatsHost, SetName, SetOwner, SetHealth, SetActivity, SetShownKind,
// SetShownActivity, SetShownHealth). Do NOT build with this file present and without the two lots.
//
// The selection block's units and statistics, in script: the unit's name, owner, health "250 / 250", the six statistic chips SelectionStat0..5 (each with its own
// tooltip), the activity of a worker or of a unit that strikes, and the count by state of a group's workers. The rules are pure (namespace HOLSelectionUnitRules:
// the sort soldiers then workers, "Peasant x3 (own)", the one-decimal formats, the attack with the technologies' bonus, the three modes of the console
// variables, the separator, the report words read by the tools, the workers' count); UHOLSelectionUnitPanel builds the chips in the lot A's StatsHost
// (same widget names as the C++ screen: SelectionStats, SelectionStat0..5) and refreshes them. ShownSelection / ShownActivity / ShownHealth have ONE writer, the
// lot A (UHOLSelectionPanel): this panel passes its fragments by SetShownKind / SetShownActivity / SetShownHealth and never writes those fields.
//
// CROCHET : what the switch pastes into HOLGameScreenScript.as (the lot A builds this panel in its Build, in one line):
//   member:  UHOLSelectionUnitPanel SelectionUnit;     (in UHOLSelectionPanel's Build:  SelectionUnit = Cast<UHOLSelectionUnitPanel>(NewObject(this, UHOLSelectionUnitPanel));  SelectionUnit.Build(Screen, this);)
//   by change (UHOLSelectionPanel's Refresh, the units' branch, the C++ RefreshUnitSelection):  SelectionUnit.Refresh();
//   every frame, one unit selected (the C++ NativeTick 548-557, RefreshUnitActivity + RefreshUnitHealth):  SelectionUnit.RefreshFrame();
//   every 4 Hz, a group (the C++ NativeTick 558-561): SelectionUnit.Refresh();
//   no unit selected / a building, a source: SelectionUnit.Hide();
//   the report read by "HUD stats:" and "HUD stats tip:" (HOLUIHUD.cpp:63-64): after Refresh / RefreshFrame, if SelectionUnit.TakeStatsReportChange() then
//     Screen.SetStatsReport(SelectionUnit.ShownStats, SelectionUnit.ShownStatTipList);   (UHOLScriptScreen::SetStatsReport to ADD by the switch, see the report)
//   HoverStat (STAYS C++, unit_stats_attack.txt:54-63): its chip i is the widget named "SelectionStat<i>" (i = 0..5) found in the script's tree (as HoverIncome finds
//   FoodAmount), visible when its Visibility is Visible; DescribeStatsTip lists the tips of the chips visible with a non-empty tooltip under the widget named
//   "SelectionStats" (not Collapsed): the tip texts are SelectionUnit.ShownStatTipList[i].

namespace HOLSelectionUnitText
{
	// Same namespace and keys as the C++ LOCTEXT of UHOLGameScreen ("HOLGameScreen,<key>"): the translations stay.
	FString Replace1(FText Pattern, FString A)
	{
		return Pattern.ToString().Replace("{0}", A);
	}

	FString Replace2(FText Pattern, FString A, FString B)
	{
		return Pattern.ToString().Replace("{0}", A).Replace("{1}", B);
	}

	FString Replace4(FText Pattern, FString A, FString B, FString C, FString D)
	{
		return Pattern.ToString().Replace("{0}", A).Replace("{1}", B).Replace("{2}", C).Replace("{3}", D);
	}

	FText StatBuildingAttack() { return NSLOCTEXT("HOLGameScreen", "StatBuildingAttack", "Attaque bâtiments {0}"); }
	FText TipBuildingAttack() { return NSLOCTEXT("HOLGameScreen", "TipBuildingAttack", "Attaque bâtiments : {0} par coup"); }
	FText StatBuildingAttackExtra() { return NSLOCTEXT("HOLGameScreen", "StatBuildingAttackExtra", "Attaque bâtiments {0}"); }
	FText TipBuildingAttackExtra() { return NSLOCTEXT("HOLGameScreen", "TipBuildingAttackExtra", "Attaque bâtiments : {0} par coup"); }
	FText TipBaseAttack() { return NSLOCTEXT("HOLGameScreen", "TipBaseAttack", "Attaque : {0}"); }
	FText TipTechnology() { return NSLOCTEXT("HOLGameScreen", "TipTechnology", "{0} % technologies"); }
	FText StatAttackBonus() { return NSLOCTEXT("HOLGameScreen", "StatAttackBonus", "Attaque {0} (+{1})"); }
	FText StatAttack() { return NSLOCTEXT("HOLGameScreen", "StatAttack", "Attaque {0}"); }
	FText StatMeleeArmor() { return NSLOCTEXT("HOLGameScreen", "StatMeleeArmor", "Armure mêlée {0}"); }
	FText TipMeleeArmor() { return NSLOCTEXT("HOLGameScreen", "TipMeleeArmor", "Armure mêlée : -{0} par coup reçu"); }
	FText StatRangedArmor() { return NSLOCTEXT("HOLGameScreen", "StatRangedArmor", "Armure distance {0}"); }
	FText TipRangedArmor() { return NSLOCTEXT("HOLGameScreen", "TipRangedArmor", "Armure distance : -{0} par tir reçu"); }
	FText StatHeroArmor() { return NSLOCTEXT("HOLGameScreen", "StatHeroArmor", "Armure héros {0}"); }
	FText TipHeroArmor() { return NSLOCTEXT("HOLGameScreen", "TipHeroArmor", "Armure héros : -{0} par coup de héros reçu\nArmure siège : -{1} par coup de siège"); }
	FText StatSpeed() { return NSLOCTEXT("HOLGameScreen", "StatSpeed", "Vitesse {0} m/s"); }
	FText TipSpeed() { return NSLOCTEXT("HOLGameScreen", "TipSpeed", "Vitesse : {0} m/s"); }

	FText ActivityGathering() { return NSLOCTEXT("HOLGameScreen", "ActivityGathering", "Récolte"); }
	FText ActivityBuilding() { return NSLOCTEXT("HOLGameScreen", "ActivityBuilding", "Construction"); }
	FText ActivityAttacking() { return NSLOCTEXT("HOLGameScreen", "ActivityAttacking", "Attaque"); }
	FText ActivityMoving() { return NSLOCTEXT("HOLGameScreen", "ActivityMoving", "En marche"); }
	FText ActivityIdle() { return NSLOCTEXT("HOLGameScreen", "ActivityIdle", "Inactif"); }
	FText ActivityWithLoad() { return NSLOCTEXT("HOLGameScreen", "ActivityWithLoad", "{0} — {1} {2}/{3}"); }
	FText GroupGathering() { return NSLOCTEXT("HOLGameScreen", "GroupGathering", "{0} en récolte"); }
	FText GroupBuilding() { return NSLOCTEXT("HOLGameScreen", "GroupBuilding", "{0} en construction"); }
	FText GroupFighting() { return NSLOCTEXT("HOLGameScreen", "GroupFighting", "{0} au combat"); }
	FText GroupMoving() { return NSLOCTEXT("HOLGameScreen", "GroupMoving", "{0} en marche"); }
	// The C++ pattern "{0} {0}|plural(one=inactif,other=inactifs)" cannot be formatted in script: two keys (NEW keys, to translate: see the report).
	FText GroupIdleOne() { return NSLOCTEXT("HOLGameScreen", "GroupIdleOne", "{0} inactif"); }
	FText GroupIdleOther() { return NSLOCTEXT("HOLGameScreen", "GroupIdleOther", "{0} inactifs"); }
}

// What the six statistic chips show: the text of each chip, the report words, the tooltips and their report words (four parallel lists).
struct FHOLSelectionStatLines
{
	TArray<FString> Lines;
	TArray<FString> Plain;
	TArray<FString> Tips;
	TArray<FString> TipPlain;
}

// The workers of a group by state (the C++ RefreshUnitSelection's counters).
struct FHOLSelectionWorkerCounts
{
	int Gathering = 0;
	int Building = 0;
	int Fighting = 0;
	int Moving = 0;
	int Idle = 0;
}

namespace HOLSelectionUnitRules
{
	// The activity kinds of the binding (FHOLUnitSelView::ActivityKind). Functions, not constants (a constant of a namespace is not visible from another file).
	int KindIdle() { return 0; }
	int KindMoving() { return 1; }
	int KindGathering() { return 2; }
	int KindBuilding() { return 3; }
	int KindAttacking() { return 4; }

	// The C++ constants of the screen (HOLGameScreen.cpp): the separator between two chips of the one-line mode.
	FString Separator() { return "   ·   "; }

	// A statistic for the report: at most one decimal, none when whole ("13.2", "12", "7"), culture-free (FormatStatPlain: round to a tenth, "." always).
	FString FormatPlain(float Value)
	{
		const int Tenths = Math::RoundToInt(Value * 10.0);
		const int Whole = Math::Abs(Tenths) / 10;
		const int Frac = Math::Abs(Tenths) % 10;
		const FString Sign = Tenths < 0 ? "-" : "";
		if (Frac == 0)
			return f"{Sign}{Whole}";
		return f"{Sign}{Whole}.{Frac}";
	}

	// The number as the chip writes it: the game's culture (FText::AsNumber, "13,2" in French) through the binding, or the plain one (tests).
	FString Number(float Value, bool bLocal)
	{
		if (bLocal)
			return HOLSelectionUnitRead::FormatStat(Value).ToString();
		return FormatPlain(Value);
	}

	// The attack with the technologies' bonus: Base * (100 + Percent) / 100 (12 at +10 % = 13.2).
	float AttackWithBonus(int Base, int Percent)
	{
		return float(Base) * float(100 + Percent) / 100.0;
	}

	// Soldiers first, the workers (the types that carry) last, each part by name (J7a.5; the C++ KeySort): the indices of the types in the order shown.
	TArray<int> SortedTypeOrder(const TArray<FString>& Names, const TArray<bool>& Workers)
	{
		TArray<int> Order;
		for (int Index = 0; Index < Names.Num(); Index++)
		{
			int Place = Order.Num();
			while (Place > 0 && TypeBefore(Names, Workers, Index, Order[Place - 1]))
				Place--;
			Order.Insert(Index, Place);
		}
		return Order;
	}

	// True when type A is shown before type B: a soldier before a worker, else by name (case-insensitive, FName::LexicalLess).
	bool TypeBefore(const TArray<FString>& Names, const TArray<bool>& Workers, int A, int B)
	{
		if (Workers[A] != Workers[B])
			return !Workers[A];
		return Names[A].ToLower() < Names[B].ToLower();
	}

	// One type's label in ShownSelection: "Peasant x3", the key alone for a single unit.
	FString TypeLabel(FString TypeName, int Count, int UnitCount)
	{
		if (UnitCount == 1)
			return TypeName;
		return f"{TypeName} x{Count}";
	}

	// ShownSelection of units: "Peasant x3 (own)", "Archer x2, Peasant x3 (other)", "King (own)".
	FString SelectionKind(const TArray<FString>& Labels, bool bOwn)
	{
		FString Joined = "";
		for (int Index = 0; Index < Labels.Num(); Index++)
		{
			if (Index > 0)
				Joined += ", ";
			Joined += Labels[Index];
		}
		return Joined + (bOwn ? " (own)" : " (other)");
	}

	// The statistics show for one unit that is not a hero, in a mode above 0 (the C++ RefreshUnitStats' opening test).
	bool StatsShown(int Mode, bool bHero)
	{
		return Mode > 0 && !bHero;
	}

	// Q-unit-stats-building: the blow against buildings alone (a unit with no ordinary attack: the Ram) from mode 1, next to an ordinary attack from mode 2 when it differs.
	bool BuildingOnly(int BuildingMode, int Attack, int BuildingAttack)
	{
		return BuildingMode >= 1 && Attack <= 0 && BuildingAttack > 0;
	}

	bool BuildingExtra(int BuildingMode, int Attack, int BuildingAttack)
	{
		return BuildingMode >= 2 && Attack > 0 && BuildingAttack > 0 && BuildingAttack != Attack;
	}

	void AddStat(FHOLSelectionStatLines& Out, FString Line, FString PlainLine, FString Tip, FString PlainTip)
	{
		Out.Lines.Add(Line);
		Out.Plain.Add(PlainLine);
		Out.Tips.Add(Tip);
		Out.TipPlain.Add(PlainTip);
	}

	// The percent of the attack's tooltip: "+10", "-5" (%+d).
	FString SignedPercent(int Percent)
	{
		return Percent >= 0 ? f"+{Percent}" : f"{Percent}";
	}

	// RefreshUnitStats of the C++ screen, as a pure rule: the chips' texts, their report words, their tooltips and their report words, in the C++ order (building
	// blow alone, attack, building blow beside it, melee armour, ranged armour, hero armour, speed). Speed in cm/s as in the row. bLocal: the numbers in the game's
	// culture (the view) or plain (the tests, the report words are always plain).
	FHOLSelectionStatLines BuildStatLines(int Attack, int BuildingAttack, int MeleeArmor, int RangedArmor, int HeroArmor, int SiegeArmor, float Speed, int Percent, int BuildingMode, bool bLocal)
	{
		FHOLSelectionStatLines Out;
		const float Base = float(Attack);
		const float Strike = AttackWithBonus(Attack, Percent);
		const float Bonus = Strike - Base;
		const bool bStrikes = Attack > 0;
		const FString BuildingNumber = Number(float(BuildingAttack), bLocal);
		const FString BuildingPlain = FormatPlain(float(BuildingAttack));
		if (BuildingOnly(BuildingMode, Attack, BuildingAttack))
		{
			AddStat(Out, HOLSelectionUnitText::Replace1(HOLSelectionUnitText::StatBuildingAttack(), BuildingNumber), f"building attack {BuildingPlain}",
				HOLSelectionUnitText::Replace1(HOLSelectionUnitText::TipBuildingAttack(), BuildingNumber), f"building attack: {BuildingPlain} per hit");
		}
		if (bStrikes)
		{
			// The attack's tooltip (short, Q-unit-stats-tooltips B): its base and the technologies' bonus.
			FString AttackTip = HOLSelectionUnitText::Replace1(HOLSelectionUnitText::TipBaseAttack(), Number(Base, bLocal));
			FString AttackTipPlain = "attack: " + FormatPlain(Base);
			if (Percent != 0)
			{
				AttackTip += "\n" + HOLSelectionUnitText::Replace1(HOLSelectionUnitText::TipTechnology(), SignedPercent(Percent));
				AttackTipPlain += ", technology " + SignedPercent(Percent) + " %";
			}
			const FString Line = Percent != 0
				? HOLSelectionUnitText::Replace2(HOLSelectionUnitText::StatAttackBonus(), Number(Strike, bLocal), Number(Bonus, bLocal))
				: HOLSelectionUnitText::Replace1(HOLSelectionUnitText::StatAttack(), Number(Strike, bLocal));
			const FString PlainLine = Percent != 0 ? "attack " + FormatPlain(Strike) + " (+" + FormatPlain(Bonus) + ")" : "attack " + FormatPlain(Strike);
			AddStat(Out, Line, PlainLine, AttackTip, AttackTipPlain);
		}
		if (BuildingExtra(BuildingMode, Attack, BuildingAttack))
		{
			AddStat(Out, HOLSelectionUnitText::Replace1(HOLSelectionUnitText::StatBuildingAttackExtra(), BuildingNumber), f"building attack {BuildingPlain}",
				HOLSelectionUnitText::Replace1(HOLSelectionUnitText::TipBuildingAttackExtra(), BuildingNumber), f"building attack: {BuildingPlain} per hit");
		}
		AddStat(Out, HOLSelectionUnitText::Replace1(HOLSelectionUnitText::StatMeleeArmor(), f"{MeleeArmor}"), f"melee armor {MeleeArmor}",
			HOLSelectionUnitText::Replace1(HOLSelectionUnitText::TipMeleeArmor(), f"{MeleeArmor}"), f"melee armor: -{MeleeArmor} per hit");
		AddStat(Out, HOLSelectionUnitText::Replace1(HOLSelectionUnitText::StatRangedArmor(), f"{RangedArmor}"), f"ranged armor {RangedArmor}",
			HOLSelectionUnitText::Replace1(HOLSelectionUnitText::TipRangedArmor(), f"{RangedArmor}"), f"ranged armor: -{RangedArmor} per shot");
		AddStat(Out, HOLSelectionUnitText::Replace1(HOLSelectionUnitText::StatHeroArmor(), f"{HeroArmor}"), f"hero armor {HeroArmor}",
			HOLSelectionUnitText::Replace2(HOLSelectionUnitText::TipHeroArmor(), f"{HeroArmor}", f"{SiegeArmor}"), f"hero armor: -{HeroArmor} per hero hit, siege armor: -{SiegeArmor} per siege hit");
		const float Metres = Speed / 100.0;
		AddStat(Out, HOLSelectionUnitText::Replace1(HOLSelectionUnitText::StatSpeed(), Number(Metres, bLocal)), "speed " + FormatPlain(Metres),
			HOLSelectionUnitText::Replace1(HOLSelectionUnitText::TipSpeed(), Number(Metres, bLocal)), "speed: " + FormatPlain(Metres) + " m/s");
		return Out;
	}

	// A chip's text: the separator after every chip but the last, none in mode 2 (one line per statistic).
	FString ChipText(FString Line, int Index, int Count, int Mode)
	{
		if (Mode == 2 || Index == Count - 1)
			return Line;
		return Line + Separator();
	}

	// "a, b, c" and "a | b | c": hol.HUD.Report's "HUD stats:" and "HUD stats tip:" words.
	FString JoinWith(const TArray<FString>& Parts, FString Separator)
	{
		FString Joined = "";
		for (int Index = 0; Index < Parts.Num(); Index++)
		{
			if (Index > 0)
				Joined += Separator;
			Joined += Parts[Index];
		}
		return Joined;
	}

	// ShownHealth of one unit: ", hp 250/250", ", hp 120/250 (share)" for a client's hero before his sheet arrives; empty for a type of unknown health.
	FString HealthFragment(int Health, int MaxHealth, bool bExact)
	{
		if (MaxHealth <= 0)
			return "";
		return f", hp {Health}/{MaxHealth}" + (bExact ? "" : " (share)");
	}

	// A single unit shows its activity line when it carries (a worker) or strikes (J7b.4).
	bool ActivityShown(int CarryCapacity, int ActivityKind)
	{
		return CarryCapacity > 0 || ActivityKind == KindAttacking();
	}

	// The load is shown when its amount is known (D303) and there is some, or the unit is at its source.
	bool LoadShown(bool bLoadAmountKnown, int LoadAmount, int ActivityKind)
	{
		return bLoadAmountKnown && (LoadAmount > 0 || ActivityKind == KindGathering());
	}

	// ShownActivity of one unit: ", EHOLUnitActivity::Gathering EHOLResource::Wood 4/10" with its load, ", EHOLUnitActivity::Idle" without (the enum's own words).
	FString ActivityFragment(FString ActivityName, FString LoadResourceName, int LoadAmount, int CarryCapacity, bool bLoad)
	{
		if (bLoad)
			return f", {ActivityName} {LoadResourceName} {LoadAmount}/{CarryCapacity}";
		return f", {ActivityName}";
	}

	FText ActivityWord(int ActivityKind)
	{
		if (ActivityKind == KindGathering())
			return HOLSelectionUnitText::ActivityGathering();
		if (ActivityKind == KindBuilding())
			return HOLSelectionUnitText::ActivityBuilding();
		if (ActivityKind == KindAttacking())
			return HOLSelectionUnitText::ActivityAttacking();
		if (ActivityKind == KindMoving())
			return HOLSelectionUnitText::ActivityMoving();
		return HOLSelectionUnitText::ActivityIdle();
	}

	// The resource's name as the top bar writes it: 0 Food, 1 Wood, 2 Stone, 3 Gold (EHOLResource of the binding).
	FText ResourceWord(int Resource)
	{
		if (Resource == 1)
			return HOLTopBarText::Wood();
		if (Resource == 2)
			return HOLTopBarText::Stone();
		if (Resource == 3)
			return HOLTopBarText::Gold();
		return HOLTopBarText::Food();
	}

	// The activity line: "Récolte — Bois 4/10" with its load, "Inactif" without.
	FString ActivityText(int ActivityKind, bool bLoad, int Resource, int LoadAmount, int CarryCapacity)
	{
		if (bLoad)
			return HOLSelectionUnitText::Replace4(HOLSelectionUnitText::ActivityWithLoad(), ActivityWord(ActivityKind).ToString(), ResourceWord(Resource).ToString(), f"{LoadAmount}", f"{CarryCapacity}");
		return ActivityWord(ActivityKind).ToString();
	}

	// The workers of a group by state (the round trip counts as gathering, D307; building and fighting on their own, never idle): from the kinds of the binding.
	FHOLSelectionWorkerCounts CountWorkers(const TArray<int>& Activities)
	{
		FHOLSelectionWorkerCounts Counts;
		for (int Kind : Activities)
		{
			if (Kind == KindGathering())
				Counts.Gathering++;
			else if (Kind == KindBuilding())
				Counts.Building++;
			else if (Kind == KindAttacking())
				Counts.Fighting++;
			else if (Kind == KindMoving())
				Counts.Moving++;
			else
				Counts.Idle++;
		}
		return Counts;
	}

	bool AnyWorker(FHOLSelectionWorkerCounts Counts)
	{
		return Counts.Gathering + Counts.Building + Counts.Fighting + Counts.Moving + Counts.Idle > 0;
	}

	// ShownActivity of a group: ", gathering 2 building 1 fighting 0 moving 1 idle 3", empty with no worker.
	FString GroupFragment(FHOLSelectionWorkerCounts Counts)
	{
		if (!AnyWorker(Counts))
			return "";
		return f", gathering {Counts.Gathering} building {Counts.Building} fighting {Counts.Fighting} moving {Counts.Moving} idle {Counts.Idle}";
	}

	// The group's line, the non-empty states in the C++ order joined by " · ": "2 en récolte · 3 inactifs".
	FString GroupText(FHOLSelectionWorkerCounts Counts)
	{
		TArray<FString> Parts;
		if (Counts.Gathering > 0)
			Parts.Add(HOLSelectionUnitText::Replace1(HOLSelectionUnitText::GroupGathering(), f"{Counts.Gathering}"));
		if (Counts.Building > 0)
			Parts.Add(HOLSelectionUnitText::Replace1(HOLSelectionUnitText::GroupBuilding(), f"{Counts.Building}"));
		if (Counts.Fighting > 0)
			Parts.Add(HOLSelectionUnitText::Replace1(HOLSelectionUnitText::GroupFighting(), f"{Counts.Fighting}"));
		if (Counts.Moving > 0)
			Parts.Add(HOLSelectionUnitText::Replace1(HOLSelectionUnitText::GroupMoving(), f"{Counts.Moving}"));
		if (Counts.Idle > 0)
			Parts.Add(HOLSelectionUnitText::Replace1(Counts.Idle == 1 ? HOLSelectionUnitText::GroupIdleOne() : HOLSelectionUnitText::GroupIdleOther(), f"{Counts.Idle}"));
		return JoinWith(Parts, " · ");
	}

	// The key of the statistics drawn: they are rebuilt only when the type, the owner, the technologies' bonus or the two modes change.
	FString StatsKey(FString TypeName, int OwnerSlot, int Percent, int Mode, int BuildingMode)
	{
		return f"{TypeName}|{OwnerSlot}|{Percent}|{Mode}|{BuildingMode}";
	}
}

// The units' and the statistics' part of the selection block. Built once by the lot A's panel (Build), refreshed by it (Refresh by change, RefreshFrame every image
// for one unit, Hide when no unit is selected).
class UHOLSelectionUnitPanel : UObject
{
	UHOLScriptScreen Screen;
	UHOLSelectionPanel Owner;

	UWrapBox StatsBox;
	TArray<UCommonTextBlock> Chips;

	// What the report reads (HUD stats: / HUD stats tip:), as C++ ShownStats / ShownStatTipList; empty when the statistics are hidden.
	FString ShownStats;
	FString ShownStatsTip;
	TArray<FString> ShownStatTipList;
	bool bStatsReportChanged = false;

	// Memos of what this panel last passed to the lot A and to its chips: nothing is set again while it is the same.
	FString SentKind = "";
	FString SentHealth = "";
	FString SentActivity = "";
	// One unit selected at the last Refresh (the frame tick runs only then, as the C++ NativeTick did for NumSelectedUnits == 1).
	bool bSingleUnit = false;

	bool IsFrameLive() const
	{
		return bSingleUnit;
	}

	// The switch (HOLGameScreenScript.as RefreshSelectionBlock) calls this when the block changes path: the lot A or C wrote the shared fields since, so the
	// memos below no longer match what is shown and the next write must go through.
	void Invalidate()
	{
		SentKind = "\n";
		SentHealth = "\n";
		SentActivity = "\n";
		bSingleUnit = false;
	}
	FString StatsKey = "";

	void Build(UHOLScriptScreen InScreen, UHOLSelectionPanel InOwner)
	{
		Screen = InScreen;
		Owner = InOwner;
		StatsBox = Cast<UWrapBox>(Screen.ConstructWidget(UWrapBox, n"SelectionStats"));
		StatsBox.SetVisibility(ESlateVisibility::Collapsed);
		UVerticalBoxSlot Slot = Owner.StatsHost.AddChildToVerticalBox(StatsBox);
		Slot.SetPadding(FMargin(0.0, 2.0, 0.0, 0.0));
		for (int Index = 0; Index < 6; Index++)
		{
			UCommonTextBlock Chip = Screen.MakeText(FName(f"SelectionStat{Index}"), UHOLTextStyleHUD);
			Chip.SetVisibility(ESlateVisibility::Collapsed);
			StatsBox.AddChildToWrapBox(Chip);
			Chips.Add(Chip);
		}
	}

	// The report is read once after a change (the switch sends it to the screen).
	bool TakeStatsReportChange()
	{
		const bool bChanged = bStatsReportChanged;
		bStatsReportChanged = false;
		return bChanged;
	}

	// HideUnitStats: nothing shown, the report empty.
	void HideStats()
	{
		if (ShownStats != "" || StatsKey != "")
			bStatsReportChanged = true;
		ShownStats = "";
		ShownStatsTip = "";
		ShownStatTipList.Reset();
		StatsKey = "";
		if (StatsBox != nullptr)
			StatsBox.SetVisibility(ESlateVisibility::Collapsed);
		if (Owner != nullptr && Owner.StatsHost != nullptr)
			Owner.StatsHost.SetVisibility(ESlateVisibility::Collapsed);
	}

	// The whole panel's share, nothing selected or not units: stats, health and activity cleared (the lot A decides the rest).
	void Hide()
	{
		HideStats();
		SetHealthLine("", FText::FromString(""), false);
		SetActivityLine("", FText::FromString(""));
	}

	void SetHealthLine(FString Fragment, FText Text, bool bShown)
	{
		Owner.SetHealth(Text, bShown);
		if (Fragment != SentHealth)
		{
			SentHealth = Fragment;
			Owner.SetShownHealth(Fragment);
		}
	}

	void SetActivityLine(FString Fragment, FText Text)
	{
		if (Fragment == SentActivity)
			return;
		SentActivity = Fragment;
		Owner.SetShownActivity(Fragment);
		Owner.SetActivity(Text, Fragment != "");
	}

	// RefreshUnitStats: the six chips from the unit's type row. Skipped while the type, the owner, the bonus and the modes are the same as drawn.
	void RefreshStats(FHOLUnitSelView View)
	{
		const int Mode = HOLSelectionUnitRead::GetUnitStatsMode();
		const int BuildingMode = HOLSelectionUnitRead::GetUnitStatsBuildingMode();
		FHOLUnitStatsRow Row;
		const FName Type = View.TypeNames[0];
		if (!HOLSelectionUnitRules::StatsShown(Mode, View.bHero) || !HOLSelectionUnitRead::GetUnitStatsRow(Type, View.OwnerSlot, Screen.GetShownSlot(), Row))
		{
			HideStats();
			return;
		}
		const FString Key = HOLSelectionUnitRules::StatsKey(Type.ToString(), View.OwnerSlot, Row.BonusPercent, Mode, BuildingMode);
		if (Key == StatsKey)
			return;
		StatsKey = Key;
		FHOLSelectionStatLines Lines = HOLSelectionUnitRules::BuildStatLines(Row.Attack, Row.BuildingAttack, Row.MeleeArmor, Row.RangedArmor, Row.HeroArmor, Row.SiegeArmor, Row.Speed, Row.BonusPercent, BuildingMode, true);
		FHOLSelectionStatLines Plain = HOLSelectionUnitRules::BuildStatLines(Row.Attack, Row.BuildingAttack, Row.MeleeArmor, Row.RangedArmor, Row.HeroArmor, Row.SiegeArmor, Row.Speed, Row.BonusPercent, BuildingMode, false);
		ShownStats = HOLSelectionUnitRules::JoinWith(Plain.Plain, ", ");
		ShownStatTipList = Plain.TipPlain;
		ShownStatsTip = HOLSelectionUnitRules::JoinWith(Plain.TipPlain, " | ");
		bStatsReportChanged = true;
		for (int Index = 0; Index < Chips.Num(); Index++)
		{
			if (Index >= Lines.Lines.Num())
			{
				Chips[Index].SetVisibility(ESlateVisibility::Collapsed);
				continue;
			}
			Chips[Index].SetText(FText::FromString(HOLSelectionUnitRules::ChipText(Lines.Lines[Index], Index, Lines.Lines.Num(), Mode)));
			Chips[Index].SetToolTipText(FText::FromString(Lines.Tips[Index]));
			// Visible (not HitTestInvisible): the hover shows the statistic's own tooltip.
			Chips[Index].SetVisibility(ESlateVisibility::Visible);
		}
		StatsBox.SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		Owner.StatsHost.SetVisibility(ESlateVisibility::SelfHitTestInvisible);
	}

	// RefreshUnitHealth + RefreshUnitActivity of one unit: its health, its stats, its activity. Every image for one unit.
	void ApplySingle(FHOLUnitSelView View)
	{
		RefreshStats(View);
		const FString HealthFragment = HOLSelectionUnitRules::HealthFragment(View.Health, View.MaxHealth, View.bHealthExact);
		SetHealthLine(HealthFragment, FText::FromString(HOLTopBar::FormatRatio(View.Health, View.MaxHealth)), View.MaxHealth > 0);
		if (!HOLSelectionUnitRules::ActivityShown(View.CarryCapacity, View.ActivityKind))
		{
			SetActivityLine("", FText::FromString(""));
			return;
		}
		const bool bLoad = HOLSelectionUnitRules::LoadShown(View.bLoadAmountKnown, View.LoadAmount, View.ActivityKind);
		SetActivityLine(HOLSelectionUnitRules::ActivityFragment(View.ActivityName, View.LoadResourceName, View.LoadAmount, View.CarryCapacity, bLoad),
			FText::FromString(HOLSelectionUnitRules::ActivityText(View.ActivityKind, bLoad, View.LoadResource, View.LoadAmount, View.CarryCapacity)));
	}

	// RefreshUnitSelection, by change (and 4 Hz for a group): name, owner, ShownSelection, then the figures of one unit or the count of a group's workers.
	void Refresh()
	{
		FHOLUnitSelView View;
		bSingleUnit = false;
		if (!HOLSelectionUnitRead::GetUnitSelectionView(Screen.GetShownSlot(), View))
		{
			if (SentKind != "none")
			{
				SentKind = "none";
				Owner.SetShownKind("none");
			}
			Hide();
			return;
		}
		TArray<FString> Names;
		for (FName Each : View.TypeNames)
			Names.Add(Each.ToString());
		TArray<int> Order = HOLSelectionUnitRules::SortedTypeOrder(Names, View.TypeWorkers);
		TArray<FString> Labels;
		for (int Index : Order)
			Labels.Add(HOLSelectionUnitRules::TypeLabel(Names[Index], View.TypeCounts[Index], View.UnitCount));
		const FString Kind = HOLSelectionUnitRules::SelectionKind(Labels, View.bOwn);
		if (Kind != SentKind)
		{
			SentKind = Kind;
			Owner.SetShownKind(Kind);
		}
		if (View.UnitCount == 1)
			Owner.SetName(HOLSelectionUnitRead::GetUnitName(View.TypeNames[0], View.OwnerSlot), true);
		Owner.SetOwner(View.OwnerName, View.OwnerColour, true);
		if (View.UnitCount == 1)
		{
			bSingleUnit = true;
			ApplySingle(View);
			return;
		}
		// A group shows no health (D288) and no statistics; its workers by state.
		HideStats();
		SetHealthLine("", FText::FromString(""), false);
		FHOLSelectionWorkerCounts Counts = HOLSelectionUnitRules::CountWorkers(View.WorkerActivities);
		SetActivityLine(HOLSelectionUnitRules::GroupFragment(Counts), FText::FromString(HOLSelectionUnitRules::GroupText(Counts)));
	}

	// What runs every image for one unit (NativeTick 548-557): its health, its activity, its statistics (rebuilt only when their key changes).
	void RefreshFrame()
	{
		FHOLUnitSelView View;
		if (!HOLSelectionUnitRead::GetUnitSelectionView(Screen.GetShownSlot(), View) || View.UnitCount != 1)
			return;
		ApplySingle(View);
	}
}
