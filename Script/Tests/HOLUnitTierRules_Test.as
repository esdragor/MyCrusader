// The unit tier rule (Palier3InProgress.md G1, D711; GDD 11.1), pure script half: ONE rule says why a unit cannot be queued now (tier too low,
// no finished building, cannot pay), the siege engines' rule (HOLSiege::Check) being one case of it. The C++ twin, the CSV column MinTier and
// the call sites (production, command grid, AI) are the C++ lot of G1 and come later; here the rule and the table of GDD 11.1 are fixed and tested
// alone. The tiers are the GDD's (Peasant, Spearman, Archer 0; Man-at-arms, Crossbowman, Light cavalry, Monk, Ram 1; Mangonel 2; Trebuchet,
// Cannon, Arquebusier 3), the siege agreement is checked against HOLSiege's own function, never against typed copies.

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
	Types.Add("Trebuchet");
	Types.Add("Cannon");
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
	T.AssertEquals(3, HOLUnitTier::MinTierOf("Trebuchet"), "the Trebuchet opens at tier 3");
	T.AssertEquals(3, HOLUnitTier::MinTierOf("Cannon"), "the Cannon opens at tier 3");
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
	// The Trebuchet and the Cannon have no row in UnitTypes.csv: Check says NotListed (Test_UnitTierSiegeEnginesLikeCpp), their tier is HOLSiege's.
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
	T.AssertEquals(HOLSiegeCheck::GetTier(int(EHOLSiegeEngine::Trebuchet)), HOLUnitTier::MinTierOf("Trebuchet"), "the same tier for the Trebuchet");
	T.AssertEquals(HOLSiegeCheck::GetTier(int(EHOLSiegeEngine::Cannon)), HOLUnitTier::MinTierOf("Cannon"), "the same tier for the Cannon");
	T.AssertEquals(HOLSiege::Available, HOLUnitTier::Available, "the same number for 'nothing stops it'");
	T.AssertEquals(HOLSiege::TierTooLow, HOLUnitTier::TierTooLow, "the same number for the tier");
	T.AssertEquals(HOLSiege::NoWorkshop, HOLUnitTier::NoBuilding, "the same number for the building");
	T.AssertEquals(HOLSiege::CannotPay, HOLUnitTier::CannotPay, "the same number for the stock");
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

// The simulation's twin (C++, HOLUnitTier::Check on the column MinTier of the unit type table) gives the same reason as the script's on every
// case: each unit the table holds and the rule lists, each tier, building and stock. The tier of each type is the column's, never typed on
// the C++ side. A type the table does not hold (Trebuchet, Cannon: G7, G8) is left out until its row exists.
void Test_UnitTierScriptAndSimulationRulesAgree(FUnitTest& T)
{
	TArray<FString> Types = UnitTierTypes();
	int Cases = 0;
	int Differences = 0;
	int ColumnDifferences = 0;
	int Refused = 0;
	for (int Type = 0; Type < Types.Num(); Type++)
	{
		const int Column = HOLUnitTierRules::GetUnitMinTier(FName(Types[Type]));
		if (Column < 0)
			continue;
		if (Column != HOLUnitTier::MinTierOf(Types[Type]))
			ColumnDifferences++;
		for (int Tier = 0; Tier < 4; Tier++)
		{
			for (int Building = 0; Building < 2; Building++)
			{
				for (int Pay = 0; Pay < 2; Pay++)
				{
					const int Script = HOLUnitTier::Check(Types[Type], Tier, Building == 1, Pay == 1);
					const int Sim = HOLUnitTierRules::CheckUnit(FName(Types[Type]), Tier, Building == 1, Pay == 1);
					Cases++;
					if (Script != Sim)
						Differences++;
					if (Sim == HOLUnitTier::TierTooLow)
						Refused++;
				}
			}
		}
	}
	T.AssertEquals(10 * 4 * 2 * 2, Cases, "the ten units of the table (the Arquebusier since G3) are compared on every case");
	T.AssertEquals(0, ColumnDifferences, "the column MinTier of the table is the tiers of GDD 11.1");
	T.AssertEquals(0, Differences, "the script and the simulation agree on every case");
	T.AssertTrue(Refused > 0, "the simulation refuses a unit for its tier");
	T.AssertEquals(HOLUnitTier::TierTooLow, HOLUnitTierRules::CheckUnit(n"ManAtArms", 0, true, true), "the C++ rule refuses the Man-at-arms at tier 0 on the real column");
	T.AssertEquals(HOLUnitTier::Available, HOLUnitTierRules::CheckUnit(n"ManAtArms", 1, true, true), "and lets him out at tier 1");
	T.AssertEquals(HOLUnitTier::Available, HOLUnitTierRules::CheckUnit(n"Spearman", 0, true, true), "the Spearman is of tier 0");
}

// Every row of the unit type table (Tools\Data\UnitTypes.csv, the 16 rows), not only the units the rule lists (MIG-J-UnitTier, D830/D835):
// the C++ twin reads the column MinTier of each row, the script its own table. A row the script lists gives the very same reason on every
// tier 0 to 3, building and stock. Since D836 point 6 the script lists every row: the hero, the King and the two test dummies are of tier 0
// as the column MinTier says (they were NotListed before). A type outside the table is NotListed on both sides.
void Test_UnitTierAgreesOnEveryRowOfTheTable(FUnitTest& T)
{
	TArray<FString> Rows;
	Rows.Add("LightInfantry");
	Rows.Add("LightInfantryNanite");
	Rows.Add("Archer");
	Rows.Add("Peasant");
	Rows.Add("Zombie");
	Rows.Add("Spearman");
	Rows.Add("ManAtArms");
	Rows.Add("DamnedWarrior");
	Rows.Add("King");
	Rows.Add("Hero");
	Rows.Add("Crossbowman");
	Rows.Add("Monk");
	Rows.Add("Ram");
	Rows.Add("Mangonel");
	Rows.Add("LightCavalry");
	Rows.Add("Arquebusier");
	int RowsFound = 0;
	int Listed = 0;
	int LeftOut = 0;
	int Cases = 0;
	int Differences = 0;
	int LeftOutRefusedForTier = 0;
	for (int Row = 0; Row < Rows.Num(); Row++)
	{
		const FName Name = FName(Rows[Row]);
		if (HOLUnitTierRules::GetUnitMinTier(Name) >= 0)
			RowsFound++;
		const bool bListed = HOLUnitTier::MinTierOf(Rows[Row]) != HOLUnitTier::UnknownTier;
		if (bListed)
			Listed++;
		else
			LeftOut++;
		for (int Tier = 0; Tier <= 3; Tier++)
		{
			for (int Building = 0; Building < 2; Building++)
			{
				for (int Pay = 0; Pay < 2; Pay++)
				{
					const int Script = HOLUnitTier::Check(Rows[Row], Tier, Building == 1, Pay == 1);
					const int Sim = HOLUnitTierRules::CheckUnit(Name, Tier, Building == 1, Pay == 1);
					Cases++;
					if (bListed && Script != Sim)
						Differences++;
					if (!bListed && Sim == HOLUnitTier::TierTooLow)
						LeftOutRefusedForTier++;
				}
			}
		}
	}
	T.AssertEquals(16, RowsFound, "the 16 rows of the unit type table exist in the C++ column");
	T.AssertEquals(16 * 4 * 2 * 2, Cases, "every row on every tier 0 to 3, building and stock");
	T.AssertEquals(16, Listed, "the script lists every row of the table (D836 point 6: the hero, the King and the two test dummies at tier 0, as the column)");
	T.AssertEquals(0, LeftOut, "no row of the table is left out of the script's rule (D836 point 6)");
	T.AssertEquals(0, HOLUnitTier::MinTierOf("Hero"), "the hero is of tier 0, as its column MinTier (D836 point 6)");
	T.AssertEquals(0, HOLUnitTier::MinTierOf("King"), "the King is of tier 0, as its column MinTier");
	T.AssertEquals(0, HOLUnitTier::MinTierOf("LightInfantry"), "the LightInfantry dummy is of tier 0, as its column MinTier");
	T.AssertEquals(0, HOLUnitTier::MinTierOf("LightInfantryNanite"), "the LightInfantryNanite dummy is of tier 0, as its column MinTier");
	T.AssertEquals(HOLUnitTierRules::CheckUnit(n"Hero", 0, true, true), HOLUnitTier::Check("Hero", 0, true, true), "the hero at tier 0: the script gives the C++ reason, not NotListed");
	T.AssertEquals(0, Differences, "on every row the script lists, the script and the C++ give the same reason");
	T.AssertEquals(0, LeftOutRefusedForTier, "a row the script leaves out is never refused for its tier by the C++");
	T.AssertEquals(HOLUnitTier::NotListed, HOLUnitTierRules::CheckUnit(n"Dragon", 3, true, true), "a type outside the table: NotListed in C++");
	T.AssertEquals(HOLUnitTier::NotListed, HOLUnitTier::Check("Dragon", 3, true, true), "and in the script");
}

// MIG-CPP-UnitTier (ELROND-1116q): the Trebuchet and the Cannon have NO row in Tools\Data\UnitTypes.csv (the 16 rows hold no siege engine of G7 / G8),
// so the C++ rule, which reads the column, answers NotListed on every tier; the production never refused them for the unit tier (only TierTooLow
// refuses there) and left them to HOLSiege::Check. The script rule answers the same, so that the bridge changes nothing in the game. Their GDD tier (3)
// stays in MinTierOf (the AI and the siege agreement read it); only Check follows the column.
void Test_UnitTierSiegeEnginesLikeCpp(FUnitTest& T)
{
	TArray<FString> Engines;
	Engines.Add("Trebuchet");
	Engines.Add("Cannon");
	int Cases = 0;
	int Differences = 0;
	for (int Engine = 0; Engine < Engines.Num(); Engine++)
	{
		T.AssertEquals(-1, HOLUnitTierRules::GetUnitMinTier(FName(Engines[Engine])), Engines[Engine] + " has no row in the column MinTier (C++)");
		for (int Tier = 0; Tier <= 3; Tier++)
		{
			const int Script = HOLUnitTier::Check(Engines[Engine], Tier, true, true);
			const int Sim = HOLUnitTierRules::CheckUnit(FName(Engines[Engine]), Tier, true, true);
			Cases++;
			if (Script != Sim)
				Differences++;
			T.AssertEquals(HOLUnitTier::NotListed, Sim, Engines[Engine] + " at tier " + Tier + ": the C++ rule says not listed");
		}
	}
	T.AssertEquals(8, Cases, "the two engines on tiers 0 to 3");
	T.AssertEquals(0, Differences, "the script rule answers the C++ rule's NotListed on the 8 cases");
}