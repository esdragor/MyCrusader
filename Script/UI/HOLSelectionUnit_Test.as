// NOT COMPILED YET (HALDIR-1119q, needs HOLSelectionUnit.as and its C++ lot, same module). H-9 lot B: the pure rules of HOLSelectionUnitRules, in literals copied from
// the C++ screen's behaviour (HOLGameScreen.cpp RefreshUnitSelection 1086-1227, RefreshUnitStats 1573-1687, RefreshUnitHealth 1688-1712, RefreshUnitActivity
// 1385-1408) and from the words the tools read (quick_check.ps1, forge.txt, unit_stats_attack.txt: "attack 13.2 (+1.2)", "melee armor 4", ...).

TArray<FString> UnitTestNames4()
{
	TArray<FString> Names;
	Names.Add("Peasant");
	Names.Add("Archer");
	Names.Add("Lancer");
	Names.Add("Builder");
	return Names;
}

TArray<bool> UnitTestWorkers4()
{
	TArray<bool> Workers;
	Workers.Add(true);
	Workers.Add(false);
	Workers.Add(false);
	Workers.Add(true);
	return Workers;
}

void Test_SelectionUnitSortsSoldiersThenWorkersEachByName(FUnitTest& T)
{
	TArray<int> Order = HOLSelectionUnitRules::SortedTypeOrder(UnitTestNames4(), UnitTestWorkers4());
	T.AssertEquals(Order.Num(), 4, "every type is shown");
	T.AssertEquals(Order[0], 1, "Archer (soldier, A) first");
	T.AssertEquals(Order[1], 2, "Lancer (soldier, L) second");
	T.AssertEquals(Order[2], 3, "Builder (worker, B) before the Peasant");
	T.AssertEquals(Order[3], 0, "Peasant (worker, P) last");
}

void Test_SelectionUnitLabelsAndShownSelection(FUnitTest& T)
{
	T.AssertEquals(HOLSelectionUnitRules::TypeLabel("Peasant", 3, 5), "Peasant x3", "a type of a group: the key and x count");
	T.AssertEquals(HOLSelectionUnitRules::TypeLabel("King", 1, 1), "King", "a single unit: the key alone");
	TArray<FString> One;
	One.Add("Peasant x3");
	T.AssertEquals(HOLSelectionUnitRules::SelectionKind(One, true), "Peasant x3 (own)", "own group");
	TArray<FString> Two;
	Two.Add("Archer x2");
	Two.Add("Peasant x3");
	T.AssertEquals(HOLSelectionUnitRules::SelectionKind(Two, false), "Archer x2, Peasant x3 (other)", "two types of another player, joined by a comma");
	TArray<FString> Alone;
	Alone.Add("King");
	T.AssertEquals(HOLSelectionUnitRules::SelectionKind(Alone, true), "King (own)", "quick_check: selection King (own)");
}

void Test_SelectionUnitPlainNumbersHaveOneDecimalAtMost(FUnitTest& T)
{
	T.AssertEquals(HOLSelectionUnitRules::FormatPlain(12.0), "12", "whole: no decimal");
	T.AssertEquals(HOLSelectionUnitRules::FormatPlain(13.2), "13.2", "one decimal");
	T.AssertEquals(HOLSelectionUnitRules::FormatPlain(13.24), "13.2", "rounded to a tenth, down");
	T.AssertEquals(HOLSelectionUnitRules::FormatPlain(1.26), "1.3", "rounded to a tenth, up");
	T.AssertEquals(HOLSelectionUnitRules::FormatPlain(7.0), "7", "the speed 700 cm/s");
	T.AssertEquals(HOLSelectionUnitRules::FormatPlain(0.0), "0", "zero");
}

void Test_SelectionUnitAttackWithBonus(FUnitTest& T)
{
	T.AssertEquals(HOLSelectionUnitRules::FormatPlain(HOLSelectionUnitRules::AttackWithBonus(12, 10)), "13.2", "Lames affutees: 12 at +10 % is 13.2");
	T.AssertEquals(HOLSelectionUnitRules::FormatPlain(HOLSelectionUnitRules::AttackWithBonus(12, 0)), "12", "no technology: the base");
	T.AssertEquals(HOLSelectionUnitRules::FormatPlain(HOLSelectionUnitRules::AttackWithBonus(8, 25)), "10", "8 at +25 % is 10");
}

void Test_SelectionUnitStatsWordsWithBonusAreTheToolsOnes(FUnitTest& T)
{
	FHOLSelectionStatLines Lines = HOLSelectionUnitRules::BuildStatLines(12, 0, 4, 4, 2, 5, 700.0, 10, 1, false);
	T.AssertEquals(Lines.Plain.Num(), 5, "attack, three armours, speed");
	T.AssertEquals(Lines.Plain[0], "attack 13.2 (+1.2)", "forge.txt: the attack with the bonus");
	T.AssertEquals(Lines.Plain[1], "melee armor 4", "melee armour");
	T.AssertEquals(Lines.Plain[2], "ranged armor 4", "ranged armour");
	T.AssertEquals(Lines.Plain[3], "hero armor 2", "hero armour");
	T.AssertEquals(Lines.Plain[4], "speed 7", "the speed in m/s");
	T.AssertEquals(Lines.TipPlain[0], "attack: 12, technology +10 %", "the attack tip: base and bonus");
	T.AssertEquals(Lines.TipPlain[1], "melee armor: -4 per hit", "melee armour tip");
	T.AssertEquals(Lines.TipPlain[2], "ranged armor: -4 per shot", "ranged armour tip");
	T.AssertEquals(Lines.TipPlain[3], "hero armor: -2 per hero hit, siege armor: -5 per siege hit", "hero armour tip names the siege armour");
	T.AssertEquals(Lines.TipPlain[4], "speed: 7 m/s", "speed tip");
}

void Test_SelectionUnitStatsReportJoinsLikeHUDStats(FUnitTest& T)
{
	FHOLSelectionStatLines Lines = HOLSelectionUnitRules::BuildStatLines(12, 0, 4, 4, 2, 5, 700.0, 0, 1, false);
	T.AssertEquals(HOLSelectionUnitRules::JoinWith(Lines.Plain, ", "), "attack 12, melee armor 4, ranged armor 4, hero armor 2, speed 7", "HUD stats: line, no bonus");
	T.AssertEquals(Lines.TipPlain[0], "attack: 12", "no technology: no bonus in the tip");
	T.AssertEquals(HOLSelectionUnitRules::JoinWith(Lines.TipPlain, " | "), "attack: 12 | melee armor: -4 per hit | ranged armor: -4 per shot | hero armor: -2 per hero hit, siege armor: -5 per siege hit | speed: 7 m/s", "HUD stats tip: line");
}

void Test_SelectionUnitBuildingAttackFollowsTheTwoModes(FUnitTest& T)
{
	T.AssertTrue(HOLSelectionUnitRules::BuildingOnly(1, 0, 60), "the Ram: no ordinary attack, a blow against buildings, mode 1");
	T.AssertFalse(HOLSelectionUnitRules::BuildingOnly(0, 0, 60), "mode 0: never");
	T.AssertFalse(HOLSelectionUnitRules::BuildingOnly(1, 8, 20), "a unit with an ordinary attack: not alone");
	T.AssertTrue(HOLSelectionUnitRules::BuildingExtra(2, 8, 20), "Lancier 8 (batiments 20), mode 2");
	T.AssertFalse(HOLSelectionUnitRules::BuildingExtra(1, 8, 20), "mode 1: not beside an ordinary attack");
	T.AssertFalse(HOLSelectionUnitRules::BuildingExtra(2, 8, 8), "the same blow: nothing more");
	FHOLSelectionStatLines Ram = HOLSelectionUnitRules::BuildStatLines(0, 60, 4, 4, 2, 5, 700.0, 0, 1, false);
	T.AssertEquals(Ram.Plain[0], "building attack 60", "unit_stats_attack.txt: the Ram's first statistic");
	T.AssertEquals(Ram.TipPlain[0], "building attack: 60 per hit", "its tip");
	T.AssertEquals(Ram.Plain.Num(), 5, "no ordinary attack: building blow, three armours, speed");
	FHOLSelectionStatLines Lancer = HOLSelectionUnitRules::BuildStatLines(8, 20, 4, 4, 2, 5, 700.0, 0, 2, false);
	T.AssertEquals(Lancer.Plain.Num(), 6, "mode 2: the building blow beside the attack");
	T.AssertEquals(Lancer.Plain[0], "attack 8", "the attack first");
	T.AssertEquals(Lancer.Plain[1], "building attack 20", "then the blow against buildings");
	FHOLSelectionStatLines Old = HOLSelectionUnitRules::BuildStatLines(0, 60, 4, 4, 2, 5, 700.0, 0, 0, false);
	T.AssertEquals(Old.Plain[0], "melee armor 4", "mode 0: the old line, no attack at all for the Ram");
}

void Test_SelectionUnitStatsModesAndSeparator(FUnitTest& T)
{
	T.AssertTrue(HOLSelectionUnitRules::StatsShown(1, false), "mode 1, a soldier");
	T.AssertTrue(HOLSelectionUnitRules::StatsShown(2, false), "mode 2, a soldier");
	T.AssertFalse(HOLSelectionUnitRules::StatsShown(0, false), "mode 0: hidden");
	T.AssertFalse(HOLSelectionUnitRules::StatsShown(1, true), "a hero: his statistics come from his level");
	T.AssertEquals(HOLSelectionUnitRules::ChipText("Attaque 12", 0, 5, 1), "Attaque 12   ·   ", "one line: the separator after a chip");
	T.AssertEquals(HOLSelectionUnitRules::ChipText("Vitesse 7 m/s", 4, 5, 1), "Vitesse 7 m/s", "one line: none after the last");
	T.AssertEquals(HOLSelectionUnitRules::ChipText("Attaque 12", 0, 5, 2), "Attaque 12", "mode 2: one line per statistic, no separator");
	T.AssertEquals(HOLSelectionUnitRules::StatsKey("Lancer", 0, 10, 1, 1), "Lancer|0|10|1|1", "the key the stats are rebuilt on");
	T.AssertTrue(HOLSelectionUnitRules::StatsKey("Lancer", 0, 10, 1, 1) != HOLSelectionUnitRules::StatsKey("Lancer", 0, 20, 1, 1), "a new bonus rebuilds");
}

void Test_SelectionUnitHealthAndActivityFragments(FUnitTest& T)
{
	T.AssertEquals(HOLSelectionUnitRules::HealthFragment(250, 250, true), ", hp 250/250", "quick_check: selection King (own), hp 250/250");
	T.AssertEquals(HOLSelectionUnitRules::HealthFragment(120, 250, false), ", hp 120/250 (share)", "a client's hero before his sheet: (share)");
	T.AssertEquals(HOLSelectionUnitRules::HealthFragment(0, 0, true), "", "type of unknown health: nothing");
	T.AssertTrue(HOLSelectionUnitRules::ActivityShown(10, HOLSelectionUnitRules::KindIdle()), "a worker always shows its activity");
	T.AssertTrue(HOLSelectionUnitRules::ActivityShown(0, HOLSelectionUnitRules::KindAttacking()), "a soldier that strikes shows it");
	T.AssertFalse(HOLSelectionUnitRules::ActivityShown(0, HOLSelectionUnitRules::KindIdle()), "quick_check: no Attacking line for a soldier that does not strike");
	T.AssertTrue(HOLSelectionUnitRules::LoadShown(true, 4, HOLSelectionUnitRules::KindMoving()), "a known load above 0");
	T.AssertTrue(HOLSelectionUnitRules::LoadShown(true, 0, HOLSelectionUnitRules::KindGathering()), "at its source, a load of 0 is shown");
	T.AssertFalse(HOLSelectionUnitRules::LoadShown(false, 4, HOLSelectionUnitRules::KindGathering()), "a client's unknown load: the activity alone");
	T.AssertEquals(HOLSelectionUnitRules::ActivityFragment("EHOLUnitActivity::Gathering", "EHOLResource::Wood", 4, 10, true), ", EHOLUnitActivity::Gathering EHOLResource::Wood 4/10", "report words with the load");
	T.AssertEquals(HOLSelectionUnitRules::ActivityFragment("EHOLUnitActivity::Idle", "EHOLResource::Food", 0, 10, false), ", EHOLUnitActivity::Idle", "report words without it");
	T.AssertEquals(HOLSelectionUnitRules::ActivityText(HOLSelectionUnitRules::KindGathering(), true, 1, 4, 10), "Récolte — Bois 4/10", "the line with the load");
	T.AssertEquals(HOLSelectionUnitRules::ActivityText(HOLSelectionUnitRules::KindIdle(), false, 0, 0, 10), "Inactif", "the line without it");
	T.AssertEquals(HOLSelectionUnitRules::ActivityText(HOLSelectionUnitRules::KindAttacking(), false, 0, 0, 0), "Attaque", "a soldier that strikes");
}

void Test_SelectionUnitGroupWorkersCountByState(FUnitTest& T)
{
	TArray<int> Kinds;
	Kinds.Add(HOLSelectionUnitRules::KindGathering());
	Kinds.Add(HOLSelectionUnitRules::KindGathering());
	Kinds.Add(HOLSelectionUnitRules::KindBuilding());
	Kinds.Add(HOLSelectionUnitRules::KindAttacking());
	Kinds.Add(HOLSelectionUnitRules::KindMoving());
	Kinds.Add(HOLSelectionUnitRules::KindIdle());
	Kinds.Add(HOLSelectionUnitRules::KindIdle());
	FHOLSelectionWorkerCounts Counts = HOLSelectionUnitRules::CountWorkers(Kinds);
	T.AssertEquals(Counts.Gathering, 2, "two gathering (the round trip counts as gathering)");
	T.AssertEquals(Counts.Building, 1, "one building");
	T.AssertEquals(Counts.Fighting, 1, "one fighting, never idle");
	T.AssertEquals(Counts.Moving, 1, "one moving");
	T.AssertEquals(Counts.Idle, 2, "two idle");
	T.AssertEquals(HOLSelectionUnitRules::GroupFragment(Counts), ", gathering 2 building 1 fighting 1 moving 1 idle 2", "the report words");
	T.AssertEquals(HOLSelectionUnitRules::GroupText(Counts), "2 en récolte · 1 en construction · 1 au combat · 1 en marche · 2 inactifs", "the line, the C++ order");
	TArray<int> One;
	One.Add(HOLSelectionUnitRules::KindIdle());
	T.AssertEquals(HOLSelectionUnitRules::GroupText(HOLSelectionUnitRules::CountWorkers(One)), "1 inactif", "the singular");
	TArray<int> None;
	FHOLSelectionWorkerCounts Nobody = HOLSelectionUnitRules::CountWorkers(None);
	T.AssertEquals(HOLSelectionUnitRules::GroupFragment(Nobody), "", "a group of soldiers: no workers' line");
	T.AssertEquals(HOLSelectionUnitRules::GroupText(Nobody), "", "and no text");
}
