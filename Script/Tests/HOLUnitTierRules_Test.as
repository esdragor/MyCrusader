// The unit tier rule (Palier3InProgress.md G1, D711; GDD 11.1), pure script half: ONE rule says why a unit cannot be queued now (tier too low,
// no finished building, cannot pay), the siege engines' rule (HOLSiege::Check) being one case of it. The C++ twin, the CSV column MinTier and
// the call sites (production, command grid, AI) are the C++ lot of G1 and come later; here the rule and the table of GDD 11.1 are fixed and tested
// alone. The tiers are the column MinTier of Tools\Data\UnitTypes.csv, read by HOLUnitTierRules::GetUnitMinTier (Peasant, Spearman, Archer 0;
// Man-at-arms, Crossbowman, Light cavalry, Monk, Ram 1; Mangonel 2; Arquebusier 3; the Trebuchet and the Cannon have no row: NotListed), the
// siege agreement is checked against HOLSiege's own function, never against typed copies. There is no C++ twin of the rule any more (the
// agreement tests are gone); Test_UnitTierColumnOfEveryRow fixes the 16 rows of the column in literals.

TArray<FString> UnitTierTypes()
{
	TArray<FString> Types;
	Types.Add("Peasant");
	Types.Add("Spearman");
	Types.Add("Archer");
	Types.Add("ManAtArms");
	Types.Add("Crossbowman");
	Types.Add("LightCavalry");
	Types.Add("Monk");
	Types.Add("Ram");
	Types.Add("Mangonel");
	Types.Add("Arquebusier");
	return Types;
}

// GDD 11.1: the tier each unit opens at.
void Test_UnitTierTable(FUnitTest& T)
{
	T.AssertEquals(0, HOLUnitTier::MinTierOf("Peasant"), "the Peasant is of tier 0");
	T.AssertEquals(0, HOLUnitTier::MinTierOf("Spearman"), "the Spearman is of tier 0");
	T.AssertEquals(0, HOLUnitTier::MinTierOf("Archer"), "the Archer is of tier 0");
	T.AssertEquals(1, HOLUnitTier::MinTierOf("ManAtArms"), "the Man-at-arms opens at tier 1");
	T.AssertEquals(1, HOLUnitTier::MinTierOf("Crossbowman"), "the Crossbowman opens at tier 1");
	T.AssertEquals(1, HOLUnitTier::MinTierOf("LightCavalry"), "the Light cavalry opens at tier 1");
	T.AssertEquals(1, HOLUnitTier::MinTierOf("Monk"), "the Monk opens at tier 1");
	T.AssertEquals(1, HOLUnitTier::MinTierOf("Ram"), "the Ram opens at tier 1");
	T.AssertEquals(2, HOLUnitTier::MinTierOf("Mangonel"), "the Mangonel opens at tier 2");
	T.AssertEquals(HOLUnitTier::UnknownTier, HOLUnitTier::MinTierOf("Trebuchet"), "the Trebuchet has no row in the column MinTier");
	T.AssertEquals(HOLUnitTier::UnknownTier, HOLUnitTier::MinTierOf("Cannon"), "the Cannon has no row in the column MinTier");
	T.AssertEquals(3, HOLUnitTier::MinTierOf("Arquebusier"), "the Arquebusier opens at tier 3");
	T.AssertEquals(0, HOLUnitTier::MinTierOf("Zombie"), "the Zombie (Legions) is of tier 0, as its column MinTier");
	T.AssertEquals(1, HOLUnitTier::MinTierOf("DamnedWarrior"), "the Damned Warrior (Legions) opens at tier 1, as its column MinTier");
	T.AssertEquals(HOLUnitTier::UnknownTier, HOLUnitTier::MinTierOf("Dragon"), "a type outside the table has no tier");
}

// At tier 0 the Spearman, the Archer and the Peasant come out and the three tier 1 soldiers are refused for their tier; at tier 1 they come out.
void Test_UnitTierOpensSoldiers(FUnitTest& T)
{
	TArray<FString> Open0;
	Open0.Add("Peasant");
	Open0.Add("Spearman");
	Open0.Add("Archer");
	for (int Index = 0; Index < Open0.Num(); Index++)
	{
		T.AssertEquals(HOLUnitTier::Available, HOLUnitTier::Check(Open0[Index], 0, true, true), Open0[Index] + " comes out at tier 0");
	}
	TArray<FString> Closed0;
	Closed0.Add("ManAtArms");
	Closed0.Add("Crossbowman");
	Closed0.Add("LightCavalry");
	for (int Index = 0; Index < Closed0.Num(); Index++)
	{
		T.AssertEquals(HOLUnitTier::TierTooLow, HOLUnitTier::Check(Closed0[Index], 0, true, true), Closed0[Index] + " is refused at tier 0: tier");
		T.AssertEquals(HOLUnitTier::Available, HOLUnitTier::Check(Closed0[Index], 1, true, true), Closed0[Index] + " comes out at tier 1");
	}
	T.AssertEquals(HOLUnitTier::TierTooLow, HOLUnitTier::Check("Mangonel", 1, true, true), "the Mangonel stays closed at tier 1");
	T.AssertEquals(HOLUnitTier::Available, HOLUnitTier::Check("Mangonel", 2, true, true), "the Mangonel opens at tier 2");
	T.AssertEquals(HOLUnitTier::TierTooLow, HOLUnitTier::Check("Arquebusier", 2, true, true), "the Arquebusier stays closed at tier 2");
	T.AssertEquals(HOLUnitTier::Available, HOLUnitTier::Check("Arquebusier", 3, true, true), "the Arquebusier opens at tier 3");
	T.AssertTrue(HOLUnitTier::IsAvailable("Arquebusier", 3, true, true), "available at tier 3");
	T.AssertFalse(HOLUnitTier::IsAvailable("Arquebusier", 2, true, true), "not at tier 2");
	// The Trebuchet and the Cannon have no row in UnitTypes.csv: Check says NotListed (Test_UnitTierSiegeEnginesNotListed), their tier is HOLSiege's.
	T.AssertFalse(HOLUnitTier::IsAvailable("Trebuchet", 3, true, true), "the Trebuchet has no unit row, the siege rule decides");
}

// The reasons come in one order, the first is the one given to the player: tier, then building, then the stock; a type outside the table first.
void Test_UnitTierReasonOrder(FUnitTest& T)
{
	T.AssertEquals(HOLUnitTier::NotListed, HOLUnitTier::Check("Dragon", 3, true, true), "a type outside the table is refused first");
	T.AssertEquals(HOLUnitTier::TierTooLow, HOLUnitTier::Check("Arquebusier", 0, false, false), "tier before building and stock");
	T.AssertEquals(HOLUnitTier::NoBuilding, HOLUnitTier::Check("Arquebusier", 3, false, false), "building before stock");
	T.AssertEquals(HOLUnitTier::CannotPay, HOLUnitTier::Check("Arquebusier", 3, true, false), "the stock last");
	T.AssertEquals(HOLUnitTier::Available, HOLUnitTier::Check("Arquebusier", 3, true, true), "nothing stops it");
	T.AssertEquals(HOLUnitTier::NoBuilding, HOLUnitTier::Check("Peasant", 0, false, true), "a tier 0 unit still wants its building");
}

// The siege engines' rule is one case of the unit rule: for the Ram and the Mangonel, over every tier and every building / stock state, the
// unit rule gives the very reason HOLSiege::Check gives (same reason numbers), so that the tests of the siege stay green without a number changed.
void Test_UnitTierAgreesWithTheSiegeRule(FUnitTest& T)
{
	int Cases = 0;
	int Differences = 0;
	for (int Tier = 0; Tier <= 3; Tier++)
	{
		for (int Building = 0; Building < 2; Building++)
		{
			for (int Pay = 0; Pay < 2; Pay++)
			{
				const int RamSiege = HOLSiegeCheck::Check(int(EHOLSiegeEngine::Ram), Tier, Building == 1, Pay == 1);
				const int RamUnit = HOLUnitTier::Check("Ram", Tier, Building == 1, Pay == 1);
				const int MangonelSiege = HOLSiegeCheck::Check(int(EHOLSiegeEngine::Mangonel), Tier, Building == 1, Pay == 1);
				const int MangonelUnit = HOLUnitTier::Check("Mangonel", Tier, Building == 1, Pay == 1);
				Cases += 2;
				if (RamSiege != RamUnit)
					Differences++;
				if (MangonelSiege != MangonelUnit)
					Differences++;
			}
		}
	}
	T.AssertEquals(32, Cases, "every case compared");
	T.AssertEquals(0, Differences, "the unit rule and the siege rule give the same reason on every case");
	T.AssertEquals(HOLSiegeCheck::GetTier(int(EHOLSiegeEngine::Ram)), HOLUnitTier::MinTierOf("Ram"), "the same tier for the Ram");
	T.AssertEquals(HOLSiegeCheck::GetTier(int(EHOLSiegeEngine::Mangonel)), HOLUnitTier::MinTierOf("Mangonel"), "the same tier for the Mangonel");
	T.AssertEquals(int(EHOLSiegeBlock::Available), HOLUnitTier::Available, "the same number for 'nothing stops it'");
	T.AssertEquals(int(EHOLSiegeBlock::TierTooLow), HOLUnitTier::TierTooLow, "the same number for the tier");
	T.AssertEquals(int(EHOLSiegeBlock::NoWorkshop), HOLUnitTier::NoBuilding, "the same number for the building");
	T.AssertEquals(int(EHOLSiegeBlock::CannotPay), HOLUnitTier::CannotPay, "the same number for the stock");
}

// Every tier of the table is between 0 and 3 and a unit open at a tier is open at every higher tier (never closes when the tier rises).
void Test_UnitTierMonotone(FUnitTest& T)
{
	TArray<FString> Types = UnitTierTypes();
	int Violations = 0;
	for (int Index = 0; Index < Types.Num(); Index++)
	{
		const int MinTier = HOLUnitTier::MinTierOf(Types[Index]);
		if (MinTier < 0 || MinTier > 3)
			Violations++;
		bool bWasOpen = false;
		for (int Tier = 0; Tier <= 3; Tier++)
		{
			const bool bOpen = HOLUnitTier::IsAvailable(Types[Index], Tier, true, true);
			if (bWasOpen && !bOpen)
				Violations++;
			bWasOpen = bOpen;
		}
	}
	T.AssertEquals(0, Violations, "tiers in range, and never closing when the tier rises");
}

// The column MinTier of every row of Tools\Data\UnitTypes.csv (the 16 rows), in literals, as the script rule reads it (MinTierOf asks
// HOLUnitTierRules::GetUnitMinTier, the column itself; no typed copy of the table). To see this test red, change one literal below (for example
// Mangonel 2 -> 3) or one MinTier cell of UnitTypes.csv. A type outside the table has no tier.
void Test_UnitTierColumnOfEveryRow(FUnitTest& T)
{
	TArray<FString> Rows;
	TArray<int> Expected;
	Rows.Add("LightInfantry");
	Expected.Add(0);
	Rows.Add("LightInfantryNanite");
	Expected.Add(0);
	Rows.Add("Archer");
	Expected.Add(0);
	Rows.Add("Peasant");
	Expected.Add(0);
	Rows.Add("Zombie");
	Expected.Add(0);
	Rows.Add("Spearman");
	Expected.Add(0);
	Rows.Add("ManAtArms");
	Expected.Add(1);
	Rows.Add("DamnedWarrior");
	Expected.Add(1);
	Rows.Add("King");
	Expected.Add(0);
	Rows.Add("Hero");
	Expected.Add(0);
	Rows.Add("Crossbowman");
	Expected.Add(1);
	Rows.Add("Monk");
	Expected.Add(1);
	Rows.Add("Ram");
	Expected.Add(1);
	Rows.Add("Mangonel");
	Expected.Add(2);
	Rows.Add("LightCavalry");
	Expected.Add(1);
	Rows.Add("Arquebusier");
	Expected.Add(3);
	T.AssertEquals(16, Rows.Num(), "the 16 rows of the unit type table");
	T.AssertEquals(Rows.Num(), Expected.Num(), "one expected tier per row");
	for (int Row = 0; Row < Rows.Num(); Row++)
	{
		const int Tier = HOLUnitTier::MinTierOf(Rows[Row]);
		T.AssertEquals(Expected[Row], Tier, Rows[Row] + ": the column MinTier");
		T.AssertEquals(Expected[Row], HOLUnitTierRules::GetUnitMinTier(FName(Rows[Row])), Rows[Row] + ": the library reads the same column");
		// Every row of the table is listed: at its own tier, with the building and the stock, the unit comes out (the hero at tier 0 too).
		T.AssertEquals(HOLUnitTier::Available, HOLUnitTier::Check(Rows[Row], Expected[Row], true, true), Rows[Row] + " comes out at its own tier");
	}
	T.AssertEquals(HOLUnitTier::UnknownTier, HOLUnitTier::MinTierOf("Dragon"), "a type outside the table has no tier");
	T.AssertEquals(HOLUnitTier::NotListed, HOLUnitTier::Check("Dragon", 3, true, true), "a type outside the table is NotListed");
}

// The Trebuchet and the Cannon have NO row in Tools\Data\UnitTypes.csv (G7, G8): the column gives no tier, so the rule answers NotListed on every
// tier (the production leaves them to HOLSiege::Check). Literals only, no twin to compare with.
void Test_UnitTierSiegeEnginesNotListed(FUnitTest& T)
{
	TArray<FString> Engines;
	Engines.Add("Trebuchet");
	Engines.Add("Cannon");
	for (int Engine = 0; Engine < Engines.Num(); Engine++)
	{
		T.AssertEquals(-1, HOLUnitTierRules::GetUnitMinTier(FName(Engines[Engine])), Engines[Engine] + " has no row in the column MinTier");
		for (int Tier = 0; Tier <= 3; Tier++)
		{
			T.AssertEquals(HOLUnitTier::NotListed, HOLUnitTier::Check(Engines[Engine], Tier, true, true), Engines[Engine] + " at tier " + Tier + ": not listed");
		}
	}
}
