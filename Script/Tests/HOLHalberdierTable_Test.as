// The Halberdier in the type tables (D858, AubeInProgress.md A1, step 2a), on the real chain: type table (UnitTypes.csv, regenerated into
// UnitTypes.uasset by the editor script) -> C++ (ReadAIUnitFigures, the row the combat code reads) -> script. Figures of D858: 75 food + 30 wood,
// no gold, 18 s, 100 health, speed 740, attack 8 every 1.875 s, reach 130, +17 against Cavalry, +8 against HeavyInfantry, Dawn only, tier 0.
// The Spearman keeps its row (80 health, 8 + 17 against Cavalry) and now belongs to the Legions alone. The damage of a blow is
// HOLAIArmy::HitDamage, a copy of HOLCombat::GetDamage, on the real rows. RED until UnitTypes.uasset is regenerated from the CSV by the editor
// (the session that owns the editor does it; no editor, no build in the agent that wrote this test).

bool HalberdierNear(float A, float B)
{
	return Math::Abs(A - B) < 0.01f;
}

// The figures of D858, one by one, as the table gives them.
void Test_HalberdierFigures(FUnitTest& T)
{
	FHOLAITypeFigures Unit;
	T.AssertTrue(HOLAIOrder::ReadAIUnitFigures(n"Halberdier", Unit), "the Halberdier is a known type");
	T.AssertEquals(100, Unit.Health, "100 health");
	T.AssertEquals(8, Unit.Damage, "8 damage");
	T.AssertTrue(HalberdierNear(1.875f, Unit.AttackInterval), "one hit every 1.875 s");
	T.AssertFalse(Unit.bRanged, "it strikes in melee");
	T.AssertTrue(HalberdierNear(130.f, Unit.AttackRange), "reach 130 cm");
	T.AssertTrue(HalberdierNear(740.f, Unit.Speed), "740 cm/s");
	T.AssertEquals(75, Unit.CostFood, "75 food");
	T.AssertEquals(30, Unit.CostWood, "30 wood");
	T.AssertEquals(0, Unit.CostGold, "no gold");
	T.AssertTrue(HalberdierNear(18.f, Unit.BuildTime), "18 s of production");
	const int Cavalry = Unit.BonusCategories.FindIndex(n"Cavalry");
	const int Heavy = Unit.BonusCategories.FindIndex(n"HeavyInfantry");
	T.AssertTrue(Cavalry >= 0 && Heavy >= 0, "a bonus against Cavalry and one against HeavyInfantry");
	if (Cavalry >= 0 && Heavy >= 0)
	{
		T.AssertTrue(HalberdierNear(17.f, Unit.BonusDamage[Cavalry]), "+17 against Cavalry");
		T.AssertTrue(HalberdierNear(8.f, Unit.BonusDamage[Heavy]), "+8 against HeavyInfantry");
	}
}

// The reason to build one: against the Man-at-arms (HeavyInfantry, 4 melee armour) its blow beats the Spearman's, and it is as good as the
// Spearman against the Light Cavalry; it costs more than the Spearman.
void Test_HalberdierBeatsSpearmanOnHeavyInfantry(FUnitTest& T)
{
	FHOLAITypeFigures Halberd;
	FHOLAITypeFigures Spear;
	FHOLAITypeFigures Knight;
	FHOLAITypeFigures Horse;
	T.AssertTrue(HOLAIOrder::ReadAIUnitFigures(n"Halberdier", Halberd), "Halberdier known");
	T.AssertTrue(HOLAIOrder::ReadAIUnitFigures(n"Spearman", Spear), "Spearman known");
	T.AssertTrue(HOLAIOrder::ReadAIUnitFigures(n"ManAtArms", Knight), "ManAtArms known");
	T.AssertTrue(HOLAIOrder::ReadAIUnitFigures(n"LightCavalry", Horse), "LightCavalry known");
	const float HalberdOnKnight = HOLAIArmy::HitDamage(Halberd, Knight);
	const float SpearOnKnight = HOLAIArmy::HitDamage(Spear, Knight);
	const float HalberdOnHorse = HOLAIArmy::HitDamage(Halberd, Horse);
	const float SpearOnHorse = HOLAIArmy::HitDamage(Spear, Horse);
	T.AssertTrue(HalberdOnKnight > SpearOnKnight, "a Halberdier's blow on a Man-at-arms beats a Spearman's");
	T.AssertTrue(HalberdierNear(SpearOnHorse, HalberdOnHorse), "on the Light Cavalry the Halberdier is the Spearman's equal");
	T.AssertTrue(HalberdierNear(12.f, HalberdOnKnight), "a blow on a Man-at-arms: 8 + 8 - 4 = 12");
	const int HalberdCost = Halberd.CostFood + Halberd.CostWood + Halberd.CostGold;
	const int SpearCost = Spear.CostFood + Spear.CostWood + Spear.CostGold;
	T.AssertTrue(HalberdCost > SpearCost, "the Halberdier costs more than the Spearman");
}
