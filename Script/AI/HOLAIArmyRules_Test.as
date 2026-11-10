// The army rules alone (IA2.3, HOLAIArmy.as): damage per second, matchups, values, the power ratio and the unit to queue next, on
// hand-made figures (the numbers of UnitTypes.csv typed in, so that a table change does not move these tests) and, for the last
// ones, on the real table through HOLAIArmy::LoadFigures (ReadAIUnitFigures: the row the combat code reads).

bool RulesNear(float A, float B, float Tolerance)
{
	return Math::Abs(A - B) < Tolerance;
}

FHOLAITypeFigures RulesFigures(int Health, int Damage, float Interval, float Range, int MeleeArmor, int RangedArmor, bool bRanged, float Speed, int Food, int Wood, int Gold)
{
	FHOLAITypeFigures Figures;
	Figures.Health = Health;
	Figures.Damage = Damage;
	Figures.AttackInterval = Interval;
	Figures.AttackRange = Range;
	Figures.MeleeArmor = MeleeArmor;
	Figures.RangedArmor = RangedArmor;
	Figures.bRanged = bRanged;
	Figures.Speed = Speed;
	Figures.CostFood = Food;
	Figures.CostWood = Wood;
	Figures.CostGold = Gold;
	return Figures;
}

FHOLAITypeFigures RulesSpearman()
{
	FHOLAITypeFigures Figures = RulesFigures(80, 8, 1.875f, 130.f, 0, 0, false, 777.f, 60, 20, 0);
	Figures.ArmorCategories.Add(n"LightInfantry");
	Figures.BonusCategories.Add(n"Cavalry");
	Figures.BonusDamage.Add(17.f);
	return Figures;
}

FHOLAITypeFigures RulesManAtArms()
{
	FHOLAITypeFigures Figures = RulesFigures(155, 12, 1.375f, 70.f, 4, 4, false, 697.f, 100, 0, 20);
	Figures.ArmorCategories.Add(n"HeavyInfantry");
	Figures.ArmorCategories.Add(n"Heavy");
	return Figures;
}

FHOLAITypeFigures RulesArcher()
{
	FHOLAITypeFigures Figures = RulesFigures(70, 5, 1.625f, 2000.f, 0, 0, true, 779.f, 30, 50, 0);
	Figures.ArmorCategories.Add(n"Ranged");
	Figures.BonusCategories.Add(n"LightInfantry");
	Figures.BonusDamage.Add(5.f);
	Figures.BonusCategories.Add(n"GunpowderInfantry");
	Figures.BonusDamage.Add(5.f);
	return Figures;
}

FHOLAITypeFigures RulesCrossbowman()
{
	FHOLAITypeFigures Figures = RulesFigures(80, 12, 2.125f, 2000.f, 0, 0, true, 720.f, 80, 0, 40);
	Figures.ArmorCategories.Add(n"Ranged");
	Figures.BonusCategories.Add(n"Heavy");
	Figures.BonusDamage.Add(10.f);
	return Figures;
}

// The default data with the hand-made figures.
FHOLAIArmyData RulesHandData()
{
	FHOLAIArmyData Data = HOLAIArmy::DefaultData();
	Data.Types.Empty();
	Data.Figures.Empty();
	Data.Types.Add(n"Spearman");
	Data.Figures.Add(RulesSpearman());
	Data.Types.Add(n"ManAtArms");
	Data.Figures.Add(RulesManAtArms());
	Data.Types.Add(n"Archer");
	Data.Figures.Add(RulesArcher());
	Data.Types.Add(n"Crossbowman");
	Data.Figures.Add(RulesCrossbowman());
	return Data;
}

void RulesAddUnits(TArray<FHOLAIUnit>& List, FName Type, int Count, int Health, int Slot)
{
	for (int Index = 0; Index < Count; Index++)
	{
		FHOLAIUnit Unit;
		Unit.Id = 100 + List.Num();
		Unit.OwnerSlot = Slot;
		Unit.Type = Type;
		Unit.Health = Health;
		Unit.bIdle = true;
		List.Add(Unit);
	}
}

// ---------------------------------------------------------------------------------------------------------------------------
// Damage per second: flat bonus by category, then the armour of the hit's kind, never under 1 (HOLCombat::GetDamage), over the interval.

void Test_ArmyDamagePerSecond(FUnitTest& T)
{
	const FHOLAITypeFigures Spear = RulesSpearman();
	const FHOLAITypeFigures Heavy = RulesManAtArms();
	const FHOLAITypeFigures Bow = RulesArcher();
	const FHOLAITypeFigures Bolt = RulesCrossbowman();
	T.AssertTrue(RulesNear(HOLAIArmy::DamagePerSecond(Spear, Heavy), 4.f / 1.875f, 0.01f), "Spearman on Man-at-arms: 8 - 4 melee armour");
	T.AssertTrue(RulesNear(HOLAIArmy::DamagePerSecond(Heavy, Spear), 12.f / 1.375f, 0.01f), "Man-at-arms on Spearman: no armour");
	T.AssertTrue(RulesNear(HOLAIArmy::DamagePerSecond(Bow, Spear), 10.f / 1.625f, 0.01f), "Archer on Spearman: 5 + 5 light infantry");
	T.AssertTrue(RulesNear(HOLAIArmy::DamagePerSecond(Bow, Heavy), 1.f / 1.625f, 0.01f), "Archer on Man-at-arms: 5 - 4 ranged armour");
	T.AssertTrue(RulesNear(HOLAIArmy::DamagePerSecond(Bolt, Heavy), 18.f / 2.125f, 0.01f), "Crossbowman on Man-at-arms: 12 + 10 - 4");
	T.AssertTrue(RulesNear(HOLAIArmy::DamagePerSecond(Bolt, Bow), 12.f / 2.125f, 0.01f), "Crossbowman on Archer: no bonus");
}

// A hit never goes under 1, whatever the armour; no interval, no damage.
void Test_ArmyDamageFloorAndNoInterval(FUnitTest& T)
{
	FHOLAITypeFigures Weak = RulesFigures(50, 3, 2.f, 100.f, 0, 0, false, 800.f, 50, 0, 0);
	FHOLAITypeFigures Plated = RulesFigures(100, 10, 1.f, 100.f, 20, 20, false, 600.f, 100, 0, 0);
	T.AssertTrue(RulesNear(HOLAIArmy::DamagePerSecond(Weak, Plated), 0.5f, 0.001f), "the hit is at least 1: 1 over 2 s");
	Weak.AttackInterval = 0.f;
	T.AssertTrue(RulesNear(HOLAIArmy::DamagePerSecond(Weak, Plated), 0.f, 0.001f), "no interval, no damage per second");
}

// ---------------------------------------------------------------------------------------------------------------------------
// Matchups: the GDD table (Crossbowman against the Heavy, Archer against light infantry, Man-at-arms tanky).

void Test_ArmyMatchupsFollowTheGdd(FUnitTest& T)
{
	const FHOLAIArmyData Data = RulesHandData();
	const FHOLAITypeFigures Spear = RulesSpearman();
	const FHOLAITypeFigures Heavy = RulesManAtArms();
	const FHOLAITypeFigures Bow = RulesArcher();
	const FHOLAITypeFigures Bolt = RulesCrossbowman();
	T.AssertTrue(HOLAIArmy::Matchup(Data, Bolt, Heavy) > 1.f, "the Crossbowman beats the Man-at-arms");
	T.AssertTrue(HOLAIArmy::Matchup(Data, Heavy, Bow) > 1.f, "the Man-at-arms beats the Archer (armour)");
	T.AssertTrue(HOLAIArmy::Matchup(Data, Heavy, Spear) > 1.f, "the Man-at-arms beats the Spearman");
	T.AssertTrue(HOLAIArmy::Matchup(Data, Bow, Spear) > 1.f, "the Archer beats the Spearman (light infantry)");
	T.AssertTrue(HOLAIArmy::Matchup(Data, Heavy, Bolt) < 1.f, "the Man-at-arms loses to the Crossbowman");
	T.AssertTrue(HOLAIArmy::Matchup(Data, Spear, Bow) < 1.f, "the Spearman loses to the Archer");
}

// The matchup of a type against itself is 1, and the two sides of a pair are inverse of each other.
void Test_ArmyMatchupIsSymmetric(FUnitTest& T)
{
	const FHOLAIArmyData Data = RulesHandData();
	for (int First = 0; First < Data.Figures.Num(); First++)
	{
		T.AssertTrue(RulesNear(HOLAIArmy::Matchup(Data, Data.Figures[First], Data.Figures[First]), 1.f, 0.001f), "a type against itself: 1");
		for (int Second = 0; Second < Data.Figures.Num(); Second++)
		{
			const float Both = HOLAIArmy::Matchup(Data, Data.Figures[First], Data.Figures[Second]) * HOLAIArmy::Matchup(Data, Data.Figures[Second], Data.Figures[First]);
			T.AssertTrue(RulesNear(Both, 1.f, 0.01f), "the two sides of a pair multiply to 1");
		}
	}
}

// The value of a unit is its cost with the data's weights (gold dearer).
void Test_ArmyUnitValue(FUnitTest& T)
{
	FHOLAIArmyData Data = RulesHandData();
	T.AssertTrue(RulesNear(HOLAIArmy::UnitValue(Data, RulesSpearman()), 80.f, 0.01f), "Spearman: 60 food + 20 wood");
	T.AssertTrue(RulesNear(HOLAIArmy::UnitValue(Data, RulesManAtArms()), 130.f, 0.01f), "Man-at-arms: 100 food + 20 gold x 1.5");
	T.AssertTrue(RulesNear(HOLAIArmy::UnitValue(Data, RulesCrossbowman()), 140.f, 0.01f), "Crossbowman: 80 food + 40 gold x 1.5");
	Data.GoldWeight = 2.f;
	T.AssertTrue(RulesNear(HOLAIArmy::UnitValue(Data, RulesCrossbowman()), 160.f, 0.01f), "the weight is the data's");
}

// ---------------------------------------------------------------------------------------------------------------------------
// Values and power ratio.

void Test_ArmyValueCountsHealthFraction(FUnitTest& T)
{
	const FHOLAIArmyData Data = RulesHandData();
	FHOLAISummary Summary;
	RulesAddUnits(Summary.Army, n"Spearman", 2, 80, 0);
	RulesAddUnits(Summary.Army, n"Archer", 1, 35, 0);
	T.AssertTrue(RulesNear(HOLAIArmy::ArmyValue(Data, Summary), 2.f * 80.f + 0.5f * 80.f, 0.01f), "two whole Spearmen and a half Archer: 160 + 40");
	FHOLAISummary Empty;
	T.AssertTrue(RulesNear(HOLAIArmy::ArmyValue(Data, Empty), 0.f, 0.001f), "no army, no value");
}

// The enemy's Peasants and the types the table does not know are worth nothing to the army comparison.
void Test_ArmyEnemyValueSkipsPeasantsAndUnknown(FUnitTest& T)
{
	const FHOLAIArmyData Data = RulesHandData();
	FHOLAISummary Summary;
	RulesAddUnits(Summary.Army, n"Spearman", 3, 80, 0);
	RulesAddUnits(Summary.Enemies, n"Peasant", 10, 50, 1);
	RulesAddUnits(Summary.Enemies, n"Dragon", 2, 500, 1);
	T.AssertTrue(RulesNear(HOLAIArmy::EnemyValue(Data, Summary), 0.f, 0.001f), "Peasants and an unknown type are no army");
	T.AssertTrue(HOLAIArmy::PowerRatio(Data, Summary) >= Data.LargeRatio, "an enemy without an army: a large ratio");
}

// The same enemy counts for less against the army that counters it: 10 Men-at-arms against Archers more than against Crossbowmen.
void Test_ArmyEnemyValueScaledByOurCounters(FUnitTest& T)
{
	const FHOLAIArmyData Data = RulesHandData();
	FHOLAISummary VersusBows;
	RulesAddUnits(VersusBows.Army, n"Archer", 10, 70, 0);
	RulesAddUnits(VersusBows.Enemies, n"ManAtArms", 10, 155, 1);
	FHOLAISummary VersusBolts;
	RulesAddUnits(VersusBolts.Army, n"Crossbowman", 10, 80, 0);
	RulesAddUnits(VersusBolts.Enemies, n"ManAtArms", 10, 155, 1);
	const float AgainstBows = HOLAIArmy::EnemyValue(Data, VersusBows);
	const float AgainstBolts = HOLAIArmy::EnemyValue(Data, VersusBolts);
	T.AssertTrue(AgainstBows > AgainstBolts, "10 Men-at-arms count for more against Archers than against Crossbowmen");
	T.AssertTrue(AgainstBolts > 0.f, "they still count");
}

void Test_ArmyPowerRatio(FUnitTest& T)
{
	const FHOLAIArmyData Data = RulesHandData();
	FHOLAISummary Strong;
	RulesAddUnits(Strong.Army, n"Archer", 10, 70, 0);
	RulesAddUnits(Strong.Enemies, n"Spearman", 2, 80, 1);
	T.AssertTrue(HOLAIArmy::PowerRatio(Data, Strong) > 1.f, "10 Archers against 2 Spearmen: ahead");
	FHOLAISummary Weak;
	RulesAddUnits(Weak.Army, n"Spearman", 2, 80, 0);
	RulesAddUnits(Weak.Enemies, n"Archer", 10, 70, 1);
	T.AssertTrue(HOLAIArmy::PowerRatio(Data, Weak) < 1.f, "2 Spearmen against 10 Archers: behind");
	FHOLAISummary NoArmy;
	RulesAddUnits(NoArmy.Enemies, n"Spearman", 2, 80, 1);
	T.AssertTrue(RulesNear(HOLAIArmy::PowerRatio(Data, NoArmy), 0.f, 0.001f), "no army against an army: 0");
	FHOLAISummary Alone;
	T.AssertTrue(HOLAIArmy::PowerRatio(Data, Alone) >= Data.LargeRatio, "nobody on either side: a large ratio");
}

// ---------------------------------------------------------------------------------------------------------------------------
// The unit to queue next.

void Test_ArmyCompositionCountersHeavyWithCrossbowmen(FUnitTest& T)
{
	const FHOLAIArmyData Data = RulesHandData();
	FHOLAISummary Summary;
	Summary.Tier = 3; // G1 (D711): every unit is open at tier 3
	RulesAddUnits(Summary.Enemies, n"ManAtArms", 5, 155, 1);
	const FHOLAIComposition Choice = HOLAIArmy::ChooseComposition(Summary, Data);
	T.AssertTrue(Choice.bValid, "a choice");
	T.AssertTrue(Choice.bCounter, "decided by the counter rule");
	T.AssertTrue(Choice.Type == n"Crossbowman", "5 Men-at-arms: Crossbowmen");
	T.AssertTrue(Choice.Building == n"ArcheryRange", "made at the Archery range");
}

void Test_ArmyCompositionCountersSpearmenWithArchers(FUnitTest& T)
{
	const FHOLAIArmyData Data = RulesHandData();
	FHOLAISummary Summary;
	RulesAddUnits(Summary.Enemies, n"Spearman", 6, 80, 1);
	const FHOLAIComposition Choice = HOLAIArmy::ChooseComposition(Summary, Data);
	T.AssertTrue(Choice.bCounter, "decided by the counter rule");
	T.AssertTrue(Choice.Type == n"Archer", "6 Spearmen: Archers");
	T.AssertTrue(Choice.Building == n"ArcheryRange", "made at the Archery range");
}

// GDD: the Man-at-arms wins by attrition against Spearmen and Archers.
void Test_ArmyCompositionAnswersArchersWithManAtArms(FUnitTest& T)
{
	const FHOLAIArmyData Data = RulesHandData();
	FHOLAISummary Summary;
	Summary.Tier = 3; // G1 (D711): every unit is open at tier 3
	RulesAddUnits(Summary.Enemies, n"Archer", 6, 70, 1);
	const FHOLAIComposition Choice = HOLAIArmy::ChooseComposition(Summary, Data);
	T.AssertTrue(Choice.bCounter, "decided by the counter rule");
	T.AssertTrue(Choice.Type == n"ManAtArms", "6 Archers: Men-at-arms");
	T.AssertTrue(Choice.Building == n"Barracks", "made at the Barracks");
}

// The Peasants of the enemy are no army to counter: with only them, the fixed mix decides.
void Test_ArmyCompositionIgnoresEnemyPeasants(FUnitTest& T)
{
	const FHOLAIArmyData Data = RulesHandData();
	FHOLAISummary Summary;
	RulesAddUnits(Summary.Enemies, n"Peasant", 12, 50, 1);
	const FHOLAIComposition Choice = HOLAIArmy::ChooseComposition(Summary, Data);
	T.AssertTrue(Choice.bValid, "a choice");
	T.AssertFalse(Choice.bCounter, "the fixed mix");
	T.AssertTrue(Choice.Type == n"Spearman", "nothing yet: the first of the mix");
}

// No enemy: the type the most below its share of the mix (Spearman 2 : ManAtArms 1 : Archer 2 : Crossbowman 1).
void Test_ArmyCompositionFixedMix(FUnitTest& T)
{
	const FHOLAIArmyData Data = RulesHandData();
	FHOLAISummary Summary;
	Summary.Tier = 3; // G1 (D711): every unit is open at tier 3
	FHOLAIComposition Choice = HOLAIArmy::ChooseComposition(Summary, Data);
	T.AssertTrue(Choice.bValid, "a choice with nothing at all");
	T.AssertFalse(Choice.bCounter, "no enemy: the fixed mix");
	T.AssertTrue(Choice.Type == n"Spearman", "empty army: Spearman (first of the mix)");
	T.AssertTrue(Choice.Building == n"Barracks", "made at the Barracks");

	RulesAddUnits(Summary.Army, n"Spearman", 2, 80, 0);
	RulesAddUnits(Summary.Army, n"Archer", 2, 70, 0);
	Choice = HOLAIArmy::ChooseComposition(Summary, Data);
	T.AssertTrue(Choice.Type == n"ManAtArms" || Choice.Type == n"Crossbowman", "2 Spearmen and 2 Archers: one of the two types still lacking");

	// The units in a queue count too: with a Man-at-arms waiting, the Crossbowman is the one lacking ...
	FHOLAIBuilding Barracks;
	Barracks.TypeName = n"Barracks";
	Barracks.State = EHOLConstructionState::Finished;
	Barracks.Queue.Add(n"ManAtArms");
	Summary.Buildings.Add(Barracks);
	Choice = HOLAIArmy::ChooseComposition(Summary, Data);
	T.AssertFalse(Choice.bCounter, "still the fixed mix");
	T.AssertTrue(Choice.Type == n"Crossbowman", "2 Spearmen, 2 Archers, a Man-at-arms waiting: the Crossbowman");
	T.AssertTrue(Choice.Building == n"ArcheryRange", "made at the Archery range");

	// ... and once every type is at its share, the first of the mix again.
	FHOLAIBuilding Range;
	Range.TypeName = n"ArcheryRange";
	Range.State = EHOLConstructionState::Finished;
	Range.Queue.Add(n"Crossbowman");
	Summary.Buildings.Add(Range);
	// G14 (Q-ai-arquebusier A): the mix counts 7 shares at tier 3 (2 + 1 + 2 + 1 + the Arquebusier 1), so with the Crossbowman waiting the Arquebusier is the one lacking ...
	Choice = HOLAIArmy::ChooseComposition(Summary, Data);
	T.AssertTrue(Choice.Type == n"Arquebusier", "2 Spearmen, 2 Archers, a Man-at-arms and a Crossbowman waiting: the Arquebusier (7th share)");
	T.AssertTrue(Choice.Building == n"ArcheryRange", "the Arquebusier is made at the Archery range");

	// ... and once it waits too, every one of the 7 shares is filled: the first of the mix again.
	Summary.Buildings[1].Queue.Add(n"Arquebusier");
	Choice = HOLAIArmy::ChooseComposition(Summary, Data);
	T.AssertTrue(Choice.Type == n"Spearman", "every type at its share: the first of the mix");
}

// ---------------------------------------------------------------------------------------------------------------------------
// The real table: the figures the combat code reads, through HOLAIArmy::LoadFigures.

void Test_ArmyRealTableFigures(FUnitTest& T)
{
	FHOLAIArmyData Data = HOLAIArmy::DefaultData();
	HOLAIArmy::LoadFigures(Data);
	T.AssertEquals(Data.Types.Num(), Data.Figures.Num(), "one set of figures per type");
	T.AssertTrue(Data.Types.FindIndex(n"Crossbowman") >= 0, "the Crossbowman is in the data");
	const int Heavy = Data.Types.FindIndex(n"ManAtArms");
	const int Bolt = Data.Types.FindIndex(n"Crossbowman");
	const int Bow = Data.Types.FindIndex(n"Archer");
	const int Spear = Data.Types.FindIndex(n"Spearman");
	T.AssertTrue(Heavy >= 0 && Bolt >= 0 && Bow >= 0 && Spear >= 0, "the four types are known");
	if (Heavy < 0 || Bolt < 0 || Bow < 0 || Spear < 0)
	{
		return;
	}
	T.AssertTrue(Data.Figures[Heavy].Health > 0, "the figures were read from the table");
	T.AssertTrue(RulesNear(HOLAIArmy::DamagePerSecond(Data.Figures[Bolt], Data.Figures[Heavy]), HOLAIArmy::DamagePerSecond(RulesCrossbowman(), RulesManAtArms()), 0.05f), "the table gives the typed-in Crossbowman on Man-at-arms");
	T.AssertTrue(HOLAIArmy::Matchup(Data, Data.Figures[Bolt], Data.Figures[Heavy]) > 1.f, "table: the Crossbowman beats the Man-at-arms");
	T.AssertTrue(HOLAIArmy::Matchup(Data, Data.Figures[Heavy], Data.Figures[Bow]) > 1.f, "table: the Man-at-arms beats the Archer");
	T.AssertTrue(HOLAIArmy::Matchup(Data, Data.Figures[Bow], Data.Figures[Spear]) > 1.f, "table: the Archer beats the Spearman");
	T.AssertTrue(HOLAIArmy::UnitValue(Data, Data.Figures[Heavy]) > HOLAIArmy::UnitValue(Data, Data.Figures[Spear]), "table: the Man-at-arms costs more than the Spearman");

	FHOLAISummary Summary;
	Summary.Tier = 3; // G1 (D711): every unit is open at tier 3
	RulesAddUnits(Summary.Enemies, n"ManAtArms", 5, Data.Figures[Heavy].Health, 1);
	const FHOLAIComposition Choice = HOLAIArmy::ChooseComposition(Summary, Data);
	T.AssertTrue(Choice.Type == n"Crossbowman", "table: 5 Men-at-arms, Crossbowmen");
	T.AssertTrue(Choice.Building == n"ArcheryRange", "table: from the Archery range");

	FHOLAISummary Ahead;
	RulesAddUnits(Ahead.Army, n"Archer", 10, Data.Figures[Bow].Health, 0);
	RulesAddUnits(Ahead.Enemies, n"Spearman", 2, Data.Figures[Spear].Health, 1);
	T.AssertTrue(HOLAIArmy::PowerRatio(Data, Ahead) > 1.f, "table: 10 Archers against 2 Spearmen, ahead");
}
