// The combat figures of a unit type, read from script (IA2.2): the real chain, type table -> C++ (ReadAIUnitFigures, the row the
// combat code reads) -> script. The counter rule of IA2.3 reads these fields.

// The Crossbowman's one bonus aims at Heavy (D428), as a flat damage per hit.
void Test_CrossbowmanBonusAgainstHeavy(FUnitTest& T)
{
	FHOLAITypeFigures Figures;
	T.AssertTrue(HOLAIOrder::ReadAIUnitFigures(n"Crossbowman", Figures), "the Crossbowman is a known type");
	T.AssertTrue(Figures.bRanged, "the Crossbowman shoots");
	T.AssertTrue(Figures.Health > 0 && Figures.Damage > 0, "it has health and damage");
	T.AssertEquals(Figures.BonusCategories.Num(), Figures.BonusDamage.Num(), "the bonus arrays are parallel");
	int Found = Figures.BonusCategories.FindIndex(n"Heavy");
	T.AssertTrue(Found >= 0, "a bonus against Heavy");
	if (Found >= 0)
	{
		T.AssertTrue(Figures.BonusDamage[Found] > 0.0, "the bonus against Heavy adds damage");
	}
}

// The Man-at-arms is Heavy and sturdier than the Spearman; the Archer outranges the Spearman.
void Test_HeavyAndRangeFigures(FUnitTest& T)
{
	FHOLAITypeFigures Spear;
	FHOLAITypeFigures Heavy;
	FHOLAITypeFigures Bow;
	T.AssertTrue(HOLAIOrder::ReadAIUnitFigures(n"Spearman", Spear), "the Spearman is known");
	T.AssertTrue(HOLAIOrder::ReadAIUnitFigures(n"ManAtArms", Heavy), "the Man-at-arms is known");
	T.AssertTrue(HOLAIOrder::ReadAIUnitFigures(n"Archer", Bow), "the Archer is known");
	T.AssertTrue(Heavy.ArmorCategories.FindIndex(n"Heavy") >= 0, "the Man-at-arms is Heavy");
	T.AssertTrue(Heavy.Health > Spear.Health, "the Man-at-arms has more health than the Spearman");
	T.AssertTrue(Bow.AttackRange > Spear.AttackRange, "the Archer outranges the Spearman");
	T.AssertFalse(Spear.bRanged, "the Spearman strikes in melee");
}
