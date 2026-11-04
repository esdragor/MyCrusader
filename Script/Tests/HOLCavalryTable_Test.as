// The Light Cavalry and the Stable in the type tables (C1, D672; CavalryInProgress.md, stats validated as a block: CV-Q3), on the real chain: type
// table -> C++ (ReadAIUnitFigures, the row the combat code reads) -> script. Every figure is the one of UnitStats.md. The matchups use
// HOLAIArmy::HitDamage, a copy of HOLCombat::GetDamage, on the real rows of the Spearman, the Archer, the Crossbowman and the Mangonel.
// NOT COMPILED, NOT RUN (written 2026-11-04 without a build, and without the regenerated UnitTypes.uasset / BuildingTypes.uasset, which the
// editor script makes from the CSV): red until the assets are regenerated. The charge (the second attack) has no column yet: C2 (C++).

bool CavalryNear(float A, float B)
{
	return Math::Abs(A - B) < 0.01f;
}

// The proposal of CavalryInProgress.md, figure by figure.
void Test_LightCavalryFigures(FUnitTest& T)
{
	FHOLAITypeFigures Unit;
	T.AssertTrue(HOLAIOrder::ReadAIUnitFigures(n"LightCavalry", Unit), "the Light Cavalry is a known type");
	T.AssertEquals(120, Unit.Health, "120 health");
	T.AssertEquals(9, Unit.Damage, "9 damage");
	T.AssertTrue(CavalryNear(1.5f, Unit.AttackInterval), "one hit every 1.5 s");
	T.AssertFalse(Unit.bRanged, "it strikes in melee");
	T.AssertTrue(CavalryNear(70.f, Unit.AttackRange), "melee reach 70 cm (0.17 tile, as the Man-at-arms)");
	T.AssertEquals(1, Unit.MeleeArmor, "1 melee armour");
	T.AssertEquals(2, Unit.RangedArmor, "2 ranged armour");
	T.AssertTrue(CavalryNear(875.f, Unit.Speed), "875 cm/s");
	T.AssertEquals(80, Unit.CostFood, "80 food");
	T.AssertEquals(40, Unit.CostGold, "40 gold");
	T.AssertEquals(0, Unit.CostWood, "no wood");
	T.AssertTrue(CavalryNear(20.f, Unit.BuildTime), "20 s of production");
	T.AssertEquals(1, Unit.Population, "one place");
	T.AssertTrue(Unit.ArmorCategories.FindIndex(n"Cavalry") >= 0, "its armour category is Cavalry");
	const int Ranged = Unit.BonusCategories.FindIndex(n"Ranged");
	const int Siege = Unit.BonusCategories.FindIndex(n"Siege");
	T.AssertTrue(Ranged >= 0 && Siege >= 0, "a bonus against Ranged (Distance) and one against Siege");
	if (Ranged >= 0 && Siege >= 0)
	{
		T.AssertTrue(CavalryNear(10.f, Unit.BonusDamage[Ranged]), "+10 against Ranged");
		T.AssertTrue(CavalryNear(20.f, Unit.BonusDamage[Siege]), "+20 against Siege");
	}
}

// The counters of the design (GDD 7.1): strong against the shooters and the siege, weak against the Spearman, on the real rows.
void Test_LightCavalryMatchups(FUnitTest& T)
{
	FHOLAITypeFigures Cav;
	FHOLAITypeFigures Spear;
	FHOLAITypeFigures Bow;
	FHOLAITypeFigures Bolt;
	FHOLAITypeFigures Engine;
	T.AssertTrue(HOLAIOrder::ReadAIUnitFigures(n"LightCavalry", Cav), "Light Cavalry known");
	T.AssertTrue(HOLAIOrder::ReadAIUnitFigures(n"Spearman", Spear), "Spearman known");
	T.AssertTrue(HOLAIOrder::ReadAIUnitFigures(n"Archer", Bow), "Archer known");
	T.AssertTrue(HOLAIOrder::ReadAIUnitFigures(n"Crossbowman", Bolt), "Crossbowman known");
	T.AssertTrue(HOLAIOrder::ReadAIUnitFigures(n"Mangonel", Engine), "Mangonel known");
	T.AssertTrue(CavalryNear(19.f, HOLAIArmy::HitDamage(Cav, Bow)), "a blow on an Archer: 9 + 10 = 19");
	T.AssertTrue(CavalryNear(19.f, HOLAIArmy::HitDamage(Cav, Bolt)), "a blow on a Crossbowman: 9 + 10 = 19");
	T.AssertTrue(CavalryNear(29.f, HOLAIArmy::HitDamage(Cav, Engine)), "a blow on a Mangonel: 9 + 20 = 29 (no melee armour)");
	T.AssertTrue(CavalryNear(9.f, HOLAIArmy::HitDamage(Cav, Spear)), "a blow on a Spearman: 9, no bonus, no armour");
	T.AssertTrue(CavalryNear(24.f, HOLAIArmy::HitDamage(Spear, Cav)), "a Spearman's blow on it: 8 + 17 - 1 = 24 (5 blows kill it)");
	T.AssertTrue(CavalryNear(3.f, HOLAIArmy::HitDamage(Bow, Cav)), "an arrow on it: 5 - 2 = 3");
	// One on one, the rule's own measure (HOLAIArmy::Matchup, above 1 the first side wins).
	const FHOLAIArmyData Data = HOLAIArmy::DefaultData();
	T.AssertTrue(HOLAIArmy::Matchup(Data, Cav, Bow) > 1.f, "it beats an Archer");
	T.AssertTrue(HOLAIArmy::Matchup(Data, Cav, Spear) < 1.f, "it loses to a Spearman");
}

// The cavalry is a type the AI knows (its value and its matchups count in the power ratio), once the table has it.
void Test_LightCavalryIsKnownToTheArmyRules(FUnitTest& T)
{
	FHOLAIArmyData Data = HOLAIArmy::DefaultData();
	HOLAIArmy::LoadFigures(Data);
	T.AssertTrue(Data.Types.FindIndex(n"LightCavalry") >= 0, "LightCavalry is among the types of the army rules");
	T.AssertTrue(HOLAIArmy::TypeIndex(Data, n"LightCavalry") >= 0, "its figures were read from the table (health above 0)");
}
