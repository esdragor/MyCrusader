// The siege engines' rules, read through HOLSiegeCheck (the C++ rule, D836 point 2; MIG-S1c-Siege), alone (SiegeInProgress.md S1, GDD 6.1, D23, D77, D180): which engine opens at which tier, which are in
// the prototype, and why an engine cannot be queued now. The expectations come from GDD 6.1 and 11.1 (Ram tier 1, Mangonel tier 2,
// Trebuchet and Cannon tier 3; Ram and Mangonel only in the prototype), and from the rules' own functions, never from typed copies.

// GDD 6.1: the tier of each engine.
void Test_SiegeEngineTiers(FUnitTest& T)
{
	T.AssertEquals(1, HOLSiegeCheck::GetTier(int(EHOLSiegeEngine::Ram)), "the Ram opens at tier 1");
	T.AssertEquals(2, HOLSiegeCheck::GetTier(int(EHOLSiegeEngine::Mangonel)), "the Mangonel opens at tier 2");
	T.AssertEquals(3, HOLSiegeCheck::GetTier(int(EHOLSiegeEngine::Trebuchet)), "the Trebuchet opens at tier 3");
	T.AssertEquals(3, HOLSiegeCheck::GetTier(int(EHOLSiegeEngine::Cannon)), "the Cannon opens at tier 3 with the powder (D180)");
}

// D77: Ram and Mangonel are the prototype's siege; the other two wait.
void Test_SiegePrototypeSet(FUnitTest& T)
{
	T.AssertTrue(HOLSiegeCheck::InPrototype(int(EHOLSiegeEngine::Ram)), "the Ram is in the prototype");
	T.AssertTrue(HOLSiegeCheck::InPrototype(int(EHOLSiegeEngine::Mangonel)), "the Mangonel is in the prototype");
	T.AssertFalse(HOLSiegeCheck::InPrototype(int(EHOLSiegeEngine::Trebuchet)), "the Trebuchet is not (D77)");
	T.AssertFalse(HOLSiegeCheck::InPrototype(int(EHOLSiegeEngine::Cannon)), "the Cannon is not (D77)");
	T.AssertEquals(2, HOLSiege::PrototypeEngines().Num(), "two engines in the prototype");
}

// The tier of the engines never goes down along the list, and a prototype engine opens before tier 3.
void Test_SiegeTierOrder(FUnitTest& T)
{
	int Last = 0;
	for (int Index = 0; Index < HOLSiege::EngineCount; Index++)
	{
		EHOLSiegeEngine Engine = HOLSiege::EngineAt(Index);
		int Tier = HOLSiegeCheck::GetTier(int(Engine));
		T.AssertTrue(Tier >= Last, f"engine {Index}: tier {Tier} does not go below {Last}");
		T.AssertTrue(Tier >= 1, f"engine {Index}: the workshop is a tier 1 building, no engine opens at tier 0");
		if (HOLSiegeCheck::InPrototype(int(Engine)))
			T.AssertTrue(Tier <= 2, f"engine {Index}: a prototype engine opens before tier 3");
		Last = Tier;
	}
}

// What a hero tier opens: nothing at tier 0, the Ram at 1, the Ram and the Mangonel from 2 (the prototype set only).
void Test_SiegeOpenedByTier(FUnitTest& T)
{
	T.AssertEquals(0, HOLSiege::OpenedEngines(0).Num(), "tier 0: no engine");
	TArray<EHOLSiegeEngine> One = HOLSiege::OpenedEngines(1);
	T.AssertEquals(1, One.Num(), "tier 1: one engine");
	T.AssertTrue(One.Num() == 1 && One[0] == EHOLSiegeEngine::Ram, "tier 1: the Ram");
	T.AssertEquals(2, HOLSiege::OpenedEngines(2).Num(), "tier 2: the Ram and the Mangonel");
	T.AssertEquals(2, HOLSiege::OpenedEngines(3).Num(), "tier 3: still the prototype's two");
}

// The reasons, one at a time: a tier too low, no finished workshop, an empty purse.
void Test_SiegeCheckOneReason(FUnitTest& T)
{
	T.AssertEquals(HOLSiege::Available, HOLSiegeCheck::Check(int(EHOLSiegeEngine::Ram), 1, true, true), "tier 1, workshop, paid: the Ram is available");
	T.AssertEquals(HOLSiege::TierTooLow, HOLSiegeCheck::Check(int(EHOLSiegeEngine::Ram), 0, true, true), "the Ram needs tier 1");
	T.AssertEquals(HOLSiege::TierTooLow, HOLSiegeCheck::Check(int(EHOLSiegeEngine::Mangonel), 1, true, true), "the Mangonel needs tier 2, tier 1 is not enough");
	T.AssertEquals(HOLSiege::Available, HOLSiegeCheck::Check(int(EHOLSiegeEngine::Mangonel), 2, true, true), "tier 2 opens the Mangonel");
	T.AssertEquals(HOLSiege::NoWorkshop, HOLSiegeCheck::Check(int(EHOLSiegeEngine::Ram), 1, false, true), "no finished siege workshop");
	T.AssertEquals(HOLSiege::CannotPay, HOLSiegeCheck::Check(int(EHOLSiegeEngine::Ram), 1, true, false), "the stock does not cover the cost");
}

// Outside the prototype first, then the order tier, workshop, cost: the first reason is the one told.
void Test_SiegeCheckReasonOrder(FUnitTest& T)
{
	T.AssertEquals(HOLSiege::NotInPrototype, HOLSiegeCheck::Check(int(EHOLSiegeEngine::Trebuchet), 3, true, true), "the Trebuchet is refused even at tier 3 (D77)");
	T.AssertEquals(HOLSiege::NotInPrototype, HOLSiegeCheck::Check(int(EHOLSiegeEngine::Cannon), 0, false, false), "outside the prototype comes first");
	T.AssertEquals(HOLSiege::TierTooLow, HOLSiegeCheck::Check(int(EHOLSiegeEngine::Mangonel), 0, false, false), "then the tier");
	T.AssertEquals(HOLSiege::NoWorkshop, HOLSiegeCheck::Check(int(EHOLSiegeEngine::Mangonel), 2, false, false), "then the workshop");
	T.AssertEquals(HOLSiege::CannotPay, HOLSiegeCheck::Check(int(EHOLSiegeEngine::Mangonel), 2, true, false), "then the cost");
}

// IsAvailable agrees with Check on every engine, every tier and every combination of the two booleans.
void Test_SiegeIsAvailableAgreesWithCheck(FUnitTest& T)
{
	for (int Index = 0; Index < HOLSiege::EngineCount; Index++)
	{
		EHOLSiegeEngine Engine = HOLSiege::EngineAt(Index);
		for (int Tier = 0; Tier <= 3; Tier++)
		{
			for (int Mask = 0; Mask < 4; Mask++)
			{
				bool bWorkshop = (Mask & 1) != 0;
				bool bPay = (Mask & 2) != 0;
				bool bAvailable = HOLSiege::IsAvailable(Engine, Tier, bWorkshop, bPay);
				T.AssertTrue(bAvailable == (HOLSiegeCheck::Check(int(Engine), Tier, bWorkshop, bPay) == HOLSiege::Available), f"engine {Index} tier {Tier} mask {Mask}: IsAvailable follows Check");
			}
		}
	}
}

// MIG-J-Siege-a (D830, migration 2026-11-16): the C++ tests of Source/HeroesOfLiberty/Tests/HOLSiegeTests.cpp ported to script. The
// tests above already carry the tiers, the prototype and the reasons of HeroesOfLiberty.Siege.Tiers; the three below carry the rest of
// Siege.Tiers, Siege.TechnologyHealth and Siege.Technologies, on the same vectors, through the REAL tables where the script can read
// them (type table -> C++ HOLAIOrder::ReadAIUnitFigures / HOLAITechnology::ReadAITechnologyTable -> script).

// Port of the end of HeroesOfLiberty.Siege.Tiers (S3): a unit type name is an engine only for the four of GDD 6.1; the words the
// production logs.
void Test_SiegeFindEngine(FUnitTest& T)
{
	int Found = int(EHOLSiegeEngine::Cannon);
	T.AssertTrue(HOLSiegeCheck::FindEngine(n"Ram", Found) && Found == int(EHOLSiegeEngine::Ram), "Ram is the Ram");
	T.AssertTrue(HOLSiegeCheck::FindEngine(n"Mangonel", Found) && Found == int(EHOLSiegeEngine::Mangonel), "Mangonel is the Mangonel");
	T.AssertTrue(HOLSiegeCheck::FindEngine(n"Trebuchet", Found) && Found == int(EHOLSiegeEngine::Trebuchet), "Trebuchet is the Trebuchet");
	T.AssertTrue(HOLSiegeCheck::FindEngine(n"Cannon", Found) && Found == int(EHOLSiegeEngine::Cannon), "Cannon is the Cannon");
	T.AssertFalse(HOLSiegeCheck::FindEngine(n"Spearman", Found), "the Spearman is no engine");
	T.AssertFalse(HOLSiegeCheck::FindEngine(NAME_None, Found), "no name is no engine");
	T.AssertTrue(HOLSiegeCheck::DescribeBlock(HOLSiege::TierTooLow) == "tier too low", "tier words");
	T.AssertTrue(HOLSiegeCheck::DescribeBlock(HOLSiege::NotInPrototype) == "not in the prototype", "prototype words");
	T.AssertTrue(HOLSiegeCheck::DescribeBlock(HOLSiege::NoWorkshop) == "no workshop", "workshop words");
	T.AssertTrue(HOLSiegeCheck::DescribeBlock(HOLSiege::CannotPay) == "cannot pay", "cost words");
	T.AssertTrue(HOLSiegeCheck::DescribeBlock(HOLSiege::Available) == "none", "nothing stops it");
}

// Port of HeroesOfLiberty.Siege.TechnologyHealth (S5): a type's maximum health with the percent its player's technologies add
// (Reinforced Ram: +10 % on the REAL Ram row's 400 -> 440), rounded to nearest, 0 changes nothing, a negative percent is no loss; the
// Mangonel's blow on the REAL Spearman row: 45, and 54 with Heavy Munitions (+20 % on the attack alone).
void Test_SiegeTechnologyHealth(FUnitTest& T)
{
	FHOLAITypeFigures Ram;
	FHOLAITypeFigures Mangonel;
	FHOLAITypeFigures Spearman;
	T.AssertTrue(HOLAIOrder::ReadAIUnitFigures(n"Ram", Ram), "Ram row");
	T.AssertTrue(HOLAIOrder::ReadAIUnitFigures(n"Mangonel", Mangonel), "Mangonel row");
	T.AssertTrue(HOLAIOrder::ReadAIUnitFigures(n"Spearman", Spearman), "Spearman row");
	T.AssertEquals(400, Ram.Health, "no technology: 400");
	T.AssertEquals(400, HOLSiege::HealthWithTechnology(Ram.Health, 0), "0 percent: 400");
	T.AssertEquals(440, HOLSiege::HealthWithTechnology(Ram.Health, 10), "+10 percent: 440");
	T.AssertEquals(480, HOLSiege::HealthWithTechnology(Ram.Health, 20), "+20 percent: 480");
	T.AssertEquals(400, HOLSiege::HealthWithTechnology(Ram.Health, -10), "a negative percent is not a loss");
	const float Blow = HOLAIArmy::HitDamage(Mangonel, Spearman);
	T.AssertTrue(Math::Abs(Blow - 45.f) < 0.01f, f"Mangonel on a Spearman: 45 (got {Blow})");
	const float Boosted = Blow - float(Mangonel.Damage) + HOLSiege::AttackWithTechnology(Mangonel.Damage, 20);
	T.AssertTrue(Math::Abs(Boosted - 54.f) < 0.01f, f"Mangonel on a Spearman with Heavy Munitions: 54 (got {Boosted})");
}

// Port of HeroesOfLiberty.Siege.Technologies on the REAL technology table (Tools/Data/Technologies.csv): the siege workshop's two
// technologies open at the tier of the engine they improve, an engine of the prototype; the statistic each changes is the one the
// engine reads (S5). Left in C++ (not readable from script): the row's Stats, the workshop's Age and Produces.
void Test_SiegeTechnologies(FUnitTest& T)
{
	TArray<FHOLAITechnology> Table;
	T.AssertTrue(HOLAITechnology::ReadAITechnologyTable(Table), "the real table is there");
	int AtTheWorkshop = 0;
	int ReinforcedRamTier = -1;
	int HeavyMunitionsTier = -1;
	for (int Index = 0; Index < Table.Num(); Index++)
	{
		if (Table[Index].Building == n"SiegeWorkshop")
			AtTheWorkshop++;
		if (Table[Index].Name == n"ReinforcedRam")
			ReinforcedRamTier = Table[Index].MinTier;
		if (Table[Index].Name == n"HeavyMunitions")
			HeavyMunitionsTier = Table[Index].MinTier;
	}
	T.AssertEquals(2, AtTheWorkshop, "two technologies at the siege workshop (GDD 11.1)");
	int RamEngine = int(EHOLSiegeEngine::Cannon);
	int MangonelEngine = int(EHOLSiegeEngine::Cannon);
	T.AssertTrue(HOLSiegeCheck::FindEngine(n"Ram", RamEngine), "Reinforced Ram improves the Ram, an engine");
	T.AssertTrue(HOLSiegeCheck::FindEngine(n"Mangonel", MangonelEngine), "Heavy Munitions improves the Mangonel, an engine");
	T.AssertEquals(HOLSiegeCheck::GetTier(int(RamEngine)), ReinforcedRamTier, "ReinforcedRam opens at the tier of the engine it improves");
	T.AssertEquals(HOLSiegeCheck::GetTier(int(MangonelEngine)), HeavyMunitionsTier, "HeavyMunitions opens at the tier of the engine it improves");
	T.AssertTrue(HOLSiegeCheck::InPrototype(int(RamEngine)), "ReinforcedRam improves an engine of the prototype");
	T.AssertTrue(HOLSiegeCheck::InPrototype(int(MangonelEngine)), "HeavyMunitions improves an engine of the prototype");
	T.AssertTrue(HOLSiegeCheck::GetHealthStat(n"Ram") == n"RamHealth", "the Ram reads RamHealth");
	T.AssertTrue(HOLSiegeCheck::GetDamageStat(n"Mangonel") == n"MangonelDamage", "the Mangonel reads MangonelDamage");
	T.AssertTrue(HOLSiegeCheck::GetHealthStat(n"Mangonel") == NAME_None, "the Mangonel's health has no technology");
	T.AssertTrue(HOLSiegeCheck::GetDamageStat(n"Ram") == NAME_None, "the Ram's damage has no technology");
	T.AssertTrue(HOLSiegeCheck::GetHealthStat(n"Spearman") == NAME_None, "a Spearman has none");
}

// MIG-J-Siege-b (D830, migration 2026-11-16): HeroesOfLiberty.Siege.Combat and HeroesOfLiberty.Siege.Table ported to script, on the
// same vectors, the rows read from the REAL type tables through C++ (HOLAIOrder::ReadAIUnitFigures / ReadAIBuildingFigures).

// Port of HeroesOfLiberty.Siege.Combat (S4, SQ1 = B, D614): the Ram takes no unit on by itself (its attack on units is 0 in the real
// table), a unit that attacks units does; the Mangonel's blow reaches the units within its area around the target and only those (the
// edge of the circle is in). The choice among candidates (HOLCombat::ChooseTarget, attack-move and hold) stays in C++ (Mass).
void Test_SiegeCombatVectors(FUnitTest& T)
{
	FHOLAITypeFigures Ram;
	FHOLAITypeFigures Mangonel;
	FHOLAITypeFigures Spearman;
	T.AssertTrue(HOLAIOrder::ReadAIUnitFigures(n"Ram", Ram), "Ram row");
	T.AssertTrue(HOLAIOrder::ReadAIUnitFigures(n"Mangonel", Mangonel), "Mangonel row");
	T.AssertTrue(HOLAIOrder::ReadAIUnitFigures(n"Spearman", Spearman), "Spearman row");
	T.AssertTrue(HOLSiege::TakesUnitsOn(Spearman.Damage), "a unit that attacks units takes the enemy on");
	T.AssertFalse(HOLSiege::TakesUnitsOn(Ram.Damage), "the Ram (no attack on units) takes nobody on");
	T.AssertTrue(HOLSiege::TakesUnitsOn(Mangonel.Damage), "the Mangonel takes units on");

	// The area: centre to centre, the circle's edge in.
	const FVector2D Centre(1000.0, 1000.0);
	T.AssertTrue(HOLSiege::IsInArea(Centre, Centre, 250.f), "on the target");
	T.AssertTrue(HOLSiege::IsInArea(Centre, Centre + FVector2D(249.0, 0.0), 250.f), "249 cm: in");
	T.AssertTrue(HOLSiege::IsInArea(Centre, Centre + FVector2D(150.0, 200.0), 250.f), "250 cm: the edge is in");
	T.AssertFalse(HOLSiege::IsInArea(Centre, Centre + FVector2D(0.0, 251.0), 250.f), "251 cm: out");
	T.AssertFalse(HOLSiege::IsInArea(Centre, Centre + FVector2D(1.0, 0.0), 0.f), "no area (0): nobody but the target itself, never another");
}

// Port of HeroesOfLiberty.Siege.Table (S2, SQ1 = B / D614, SQ2 = A / D615, SQ3 / D625): the figures of the two engines and of the
// siege workshop that the script can read, from the REAL tables. Left in C++ (not readable from script): the workshop's age, grid slot,
// footprint, health, category, products, deposit, walkability and sight; the engines' attack type, attack on buildings, area, sight,
// siege and hero armours, and the "only the Mangonel has an area" sweep over every row.
void Test_SiegeTableVectors(FUnitTest& T)
{
	FHOLAITypeFigures Workshop;
	const bool bWorkshop = HOLAIOrder::ReadAIBuildingFigures(n"SiegeWorkshop", Workshop);
	T.AssertTrue(bWorkshop, "SiegeWorkshop in the building type table");
	if (bWorkshop)
	{
		T.AssertTrue(Workshop.CostWood == 175 && Workshop.CostFood == 0 && Workshop.CostStone == 0 && Workshop.CostGold == 0, "Workshop: 175 wood, nothing else");
		T.AssertTrue(Math::Abs(Workshop.BuildTime - 25.f) < 0.01f, "Workshop: 25 s to build");
		T.AssertEquals(0, Workshop.Population, "Workshop: no population");
	}

	FHOLAITypeFigures Ram;
	const bool bRam = HOLAIOrder::ReadAIUnitFigures(n"Ram", Ram);
	T.AssertTrue(bRam, "Ram in the unit type table");
	if (bRam)
	{
		T.AssertEquals(400, Ram.Health, "Ram: 400 health");
		T.AssertTrue(Math::Abs(Ram.Speed - 220.f) < 0.01f, "Ram: 220 cm/s");
		T.AssertEquals(3, Ram.Population, "Ram: 3 places");
		T.AssertTrue(Ram.CostWood == 150 && Ram.CostGold == 75 && Ram.CostFood == 0 && Ram.CostStone == 0, "Ram: 150 wood + 75 gold");
		T.AssertTrue(Ram.ArmorCategories.Num() == 1 && Ram.ArmorCategories[0] == n"Siege", "Ram: armour category Siege alone (D345)");
		T.AssertEquals(0, Ram.Damage, "Ram: no attack on units (SQ1 = B)");
		T.AssertTrue(Ram.BuildTime > 0.f, "Ram: a production time");
		T.AssertEquals(60, Ram.RangedArmor, "Ram: ranged armour 60 (SQ3, D625)");
		T.AssertTrue(Ram.MeleeArmor >= 0 && Ram.RangedArmor >= 0, "Ram: armours not negative");
	}

	FHOLAITypeFigures Mangonel;
	const bool bMangonel = HOLAIOrder::ReadAIUnitFigures(n"Mangonel", Mangonel);
	T.AssertTrue(bMangonel, "Mangonel in the unit type table");
	if (bMangonel)
	{
		T.AssertEquals(200, Mangonel.Health, "Mangonel: 200 health");
		T.AssertTrue(Math::Abs(Mangonel.Speed - 180.f) < 0.01f, "Mangonel: 180 cm/s");
		T.AssertEquals(4, Mangonel.Population, "Mangonel: 4 places");
		T.AssertTrue(Mangonel.CostWood == 200 && Mangonel.CostGold == 150 && Mangonel.CostFood == 0 && Mangonel.CostStone == 0, "Mangonel: 200 wood + 150 gold");
		T.AssertTrue(Mangonel.ArmorCategories.Num() == 1 && Mangonel.ArmorCategories[0] == n"Siege", "Mangonel: armour category Siege alone");
		T.AssertEquals(45, Mangonel.Damage, "Mangonel: 45 damage");
		T.AssertFalse(Mangonel.bRanged, "Mangonel: a siege attack, not a ranged one");
		T.AssertTrue(Math::Abs(Mangonel.AttackInterval - 5.f) < 0.01f, "Mangonel: every 5 s");
		T.AssertTrue(Math::Abs(Mangonel.AttackRange - 3000.f) < 0.01f, "Mangonel: 3 000 cm reach");
		T.AssertTrue(Mangonel.BuildTime > 0.f, "Mangonel: a production time");
		T.AssertEquals(60, Mangonel.RangedArmor, "Mangonel: ranged armour 60 (SQ3, D625)");
		T.AssertTrue(Mangonel.MeleeArmor >= 0 && Mangonel.RangedArmor >= 0, "Mangonel: armours not negative");
	}
}
