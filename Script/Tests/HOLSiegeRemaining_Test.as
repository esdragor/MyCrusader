// MIG2-S4 (SAURON-1118n, D839): what Script/Siege/HOLSiegeRules_Test.as could not read yet of the four C++ tests of
// Source/HeroesOfLiberty/Tests/HOLSiegeTests.cpp (Siege.Technologies, Siege.TechnologyHealth, Siege.Combat, Siege.Table), through the
// bindings added to HOLSiegeCheckLibrary (ReadUnitFigures, CountAreaRowsExcept, GetTileSize, TechnologyChangesStat, ChooseSingleTarget) and
// through HOLBuildingType::FindRow (the building row fields are BlueprintReadOnly). Expected values are the C++ test's, as literals.
// The Siege.TechnologyHealth and the rest of Siege.Combat / Siege.Table are already in HOLSiegeRules_Test.as (same values).

// Siege.Combat, the choice of the target (HOLCombat::ChooseTarget): the Ram takes no unit on by itself, on an attack-move or on hold.
// SeekMode 1 = AttackMove, 2 = Hold. One enemy at 300 cm, a reach of 50.
void Test_SiegeRemaining_ChooseTarget(FUnitTest& T)
{
	const FVector2D Enemy(300.0, 0.0);
	int Attacker = HOLSiegeCheck::ChooseSingleTarget(1, true, 50.f, Enemy);
	T.AssertEquals(0, Attacker, "a unit that attacks units takes the enemy on, on an attack-move");
	int Ram = HOLSiegeCheck::ChooseSingleTarget(1, false, 50.f, Enemy);
	T.AssertEquals(-1, Ram, "the Ram (no attack on units) takes nobody on, on an attack-move");
	int RamOnHold = HOLSiegeCheck::ChooseSingleTarget(2, false, 50.f, Enemy);
	T.AssertEquals(-1, RamOnHold, "nor on hold");
}

// Siege.Technologies (the part the script could not read): the building the technology names is a real type, not older than it, and
// the workshop produces the engine the technology improves; the statistic each technology changes is the one the engine reads.
void Test_SiegeRemaining_TechnologiesWorkshop(FUnitTest& T)
{
	FHOLBuildingTypeRow Workshop;
	const bool bWorkshop = HOLBuildingType::FindRow(n"SiegeWorkshop", Workshop);
	T.AssertTrue(bWorkshop, "the siege workshop is in the building type table");
	if (!bWorkshop)
		return;

	TArray<FHOLAITechnology> Table;
	T.AssertTrue(HOLAITechnology::ReadAITechnologyTable(Table), "the real table is there");
	int ReinforcedRamTier = -1;
	int HeavyMunitionsTier = -1;
	for (int Index = 0; Index < Table.Num(); Index++)
	{
		if (Table[Index].Name == n"ReinforcedRam")
			ReinforcedRamTier = Table[Index].MinTier;
		if (Table[Index].Name == n"HeavyMunitions")
			HeavyMunitionsTier = Table[Index].MinTier;
	}
	T.AssertTrue(ReinforcedRamTier >= Workshop.Age, f"ReinforcedRam: not researched before its building exists (tier {ReinforcedRamTier}, building age {Workshop.Age})");
	T.AssertTrue(HeavyMunitionsTier >= Workshop.Age, f"HeavyMunitions: not researched before its building exists (tier {HeavyMunitionsTier}, building age {Workshop.Age})");
	T.AssertTrue(Workshop.Produces.Contains(n"Ram"), "ReinforcedRam: the workshop produces the engine it improves (Ram)");
	T.AssertTrue(Workshop.Produces.Contains(n"Mangonel"), "HeavyMunitions: the workshop produces the engine it improves (Mangonel)");

	// S5: the statistic each technology changes is the one the engine reads (not a name typed twice).
	const FName RamHealthStat = HOLSiegeCheck::GetHealthStat(n"Ram");
	const FName MangonelDamageStat = HOLSiegeCheck::GetDamageStat(n"Mangonel");
	T.AssertTrue(HOLSiegeCheck::TechnologyChangesStat(n"ReinforcedRam", RamHealthStat), "Reinforced Ram changes what the Ram reads");
	T.AssertTrue(HOLSiegeCheck::TechnologyChangesStat(n"HeavyMunitions", MangonelDamageStat), "Heavy Munitions changes what the Mangonel reads");
}

// Siege.Table, the siege workshop's row (GDD 6.1, 11.1; D615): age, category, grid slot, footprint, health, products, deposit, walk, sight.
void Test_SiegeRemaining_TableWorkshop(FUnitTest& T)
{
	FHOLBuildingTypeRow Workshop;
	const bool bWorkshop = HOLBuildingType::FindRow(n"SiegeWorkshop", Workshop);
	T.AssertTrue(bWorkshop, "SiegeWorkshop in the building type table");
	if (!bWorkshop)
		return;
	const double Grid = HOLSiegeCheck::GetTileSize();
	T.AssertTrue(Workshop.BuildGridSlot != -1, "Workshop: built by Peasants");
	T.AssertEquals(int(EHOLBuildingCategory::Military), int(Workshop.Category), "Workshop: military");
	T.AssertEquals(1, Workshop.Age, "Workshop: age II (index 1), the tier of the Ram (GDD 6.1, 11.1)");
	T.AssertEquals(4, Workshop.BuildGridSlot, "Workshop: Q of page II (slot 4, military row)");
	T.AssertTrue(Math::Abs(Workshop.Footprint.X - 3.0 * Grid) < 0.01 && Math::Abs(Workshop.Footprint.Y - 3.0 * Grid) < 0.01, "Workshop: 3 x 3 tiles");
	T.AssertEquals(1500, Workshop.MaxHealth, "Workshop: 1 500 health");
	T.AssertEquals(2, Workshop.Produces.Num(), "Workshop: produces two engines");
	if (Workshop.Produces.Num() == 2)
	{
		T.AssertTrue(Workshop.Produces[0] == n"Ram" && Workshop.Produces[1] == n"Mangonel", "Workshop: produces the Ram then the Mangonel");
	}
	T.AssertEquals(0, Workshop.PopulationProvided, "Workshop: no population");
	T.AssertEquals(0, Workshop.Accepts.Num(), "Workshop: not a deposit");
	T.AssertFalse(Workshop.Walkable, "Workshop: not walkable");
	T.AssertTrue(Workshop.SightRadius >= 2000.f, "Workshop: sees at least as far as a house");
	// The workshop's engines are all in the unit table, each with the Siege armour category.
	for (int Index = 0; Index < Workshop.Produces.Num(); Index++)
	{
		FHOLSiegeUnitFigures Engine;
		T.AssertTrue(HOLSiegeCheck::ReadUnitFigures(Workshop.Produces[Index], Engine), f"Workshop: {Workshop.Produces[Index]} in the unit type table");
		T.AssertTrue(Engine.bHasSiegeCategory, f"Workshop: {Workshop.Produces[Index]} is a siege unit");
	}
}

// Siege.Table, the Ram's row (SQ1 = B, D614, D625): hits buildings only, siege attack, 60 every 2.5 s, no area; sees a tile beyond its reach.
void Test_SiegeRemaining_TableRam(FUnitTest& T)
{
	FHOLSiegeUnitFigures Ram;
	const bool bRam = HOLSiegeCheck::ReadUnitFigures(n"Ram", Ram);
	T.AssertTrue(bRam, "Ram in the unit type table");
	if (!bRam)
		return;
	T.AssertTrue(Ram.bBuildingSiegeAttack, "Ram: siege attack on buildings");
	T.AssertEquals(60, Ram.BuildingAttack, "Ram: 60 damage on a building");
	T.AssertTrue(Math::Abs(Ram.BuildingAttackInterval - 2.5) < 0.01, "Ram: every 2.5 s");
	T.AssertTrue(Math::Abs(Ram.AreaRadius) < 0.01, "Ram: no area");
	T.AssertTrue(Ram.MeleeArmor >= 0 && Ram.RangedArmor >= 0 && Ram.SiegeArmor >= 0 && Ram.HeroArmor >= 0, "Ram: armours not negative");
	T.AssertTrue(Ram.SightRadius >= Ram.BuildingAttackRange + 400.0, "Ram: sees at least a tile beyond its reach");
}

// Siege.Table, the Mangonel's row: siege attack, 45 every 5 s at 3 000 cm in an area of 250 cm on units, one blow on a building.
void Test_SiegeRemaining_TableMangonel(FUnitTest& T)
{
	FHOLSiegeUnitFigures Mangonel;
	const bool bMangonel = HOLSiegeCheck::ReadUnitFigures(n"Mangonel", Mangonel);
	T.AssertTrue(bMangonel, "Mangonel in the unit type table");
	if (!bMangonel)
		return;
	T.AssertTrue(Mangonel.bSiegeAttack, "Mangonel: siege attack");
	T.AssertTrue(Math::Abs(Mangonel.AreaRadius - 250.0) < 0.01, "Mangonel: area of 250 cm on units");
	T.AssertTrue(Mangonel.bBuildingSiegeAttack, "Mangonel: siege attack on buildings");
	T.AssertEquals(45, Mangonel.BuildingAttack, "Mangonel: 45 damage on a building, one blow");
	T.AssertTrue(Math::Abs(Mangonel.BuildingAttackInterval - 5.0) < 0.01, "Mangonel: a building every 5 s");
	T.AssertTrue(Math::Abs(Mangonel.BuildingAttackRange - 3000.0) < 0.01, "Mangonel: 3 000 cm reach on a building");
	T.AssertTrue(Mangonel.MeleeArmor >= 0 && Mangonel.RangedArmor >= 0 && Mangonel.SiegeArmor >= 0 && Mangonel.HeroArmor >= 0, "Mangonel: armours not negative");
	T.AssertTrue(Mangonel.SightRadius >= Mangonel.AttackRange + 400.0, "Mangonel: sees at least a tile beyond its reach");
	// Only the Mangonel has an area; no other unit type does (the column is new).
	T.AssertEquals(0, HOLSiegeCheck::CountAreaRowsExcept(n"Mangonel"), "no other unit type has area damage");
}
