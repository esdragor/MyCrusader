// L17-02 (PLAN-1117a, D839): the cases of Source/HeroesOfLiberty/Tests/HOLSiegeTests.cpp (Siege.Tiers, Siege.Technologies,
// Siege.TechnologyHealth) that Script/Siege/HOLSiegeRules_Test.as did not carry yet, through HOLSiegeCheck (the C++ rule). The rest of
// the three tests is already ported there (Test_SiegeEngineTiers, PrototypeSet, CheckOneReason, CheckReasonOrder, FindEngine,
// Technologies, TechnologyHealth); not repeated here. Expectations: GDD 6.1 and 11.1.

// Siege.Tiers: the order of the enum is the order of the tiers, and the name an engine is found by is the name of its unit type.
void Test_SiegeTiers(FUnitTest& T)
{
	TArray<FName> Names;
	Names.Add(n"Ram");
	Names.Add(n"Mangonel");
	Names.Add(n"Trebuchet");
	Names.Add(n"Cannon");
	T.AssertEquals(HOLSiege::EngineCount, Names.Num(), "one unit type name per engine");
	for (int Index = 0; Index < Names.Num(); Index++)
	{
		int Found = -1;
		T.AssertTrue(HOLSiegeCheck::FindEngine(Names[Index], Found), f"engine {Index}: its unit type name is an engine");
		T.AssertEquals(Index, Found, f"engine {Index}: the name finds the engine of the same index");
		T.AssertEquals(HOLSiegeCheck::GetTier(int(HOLSiege::EngineAt(Index))), HOLSiegeCheck::GetTier(Found), f"engine {Index}: the same tier by index and by name");
	}
	// GDD 6.1: Ram 1, Mangonel 2, Trebuchet 3, Cannon 3 (D180); the sum is the whole table at once.
	int Sum = 0;
	for (int Index = 0; Index < HOLSiege::EngineCount; Index++)
		Sum += HOLSiegeCheck::GetTier(int(HOLSiege::EngineAt(Index)));
	T.AssertEquals(9, Sum, "the tiers of the four engines: 1 + 2 + 3 + 3");
}

// Siege.Technologies (the part of the C++ test the script can read): no technology of the siege workshop opens before the tier of the
// Ram, the first engine of that workshop (the C++ test pins the workshop's age to GetTier(Ram): "not researched before its building
// exists"). The rows' Stats and the workshop's Age / Produces are not readable from script: they stay in C++.
void Test_SiegeTechnologyTiersNotBeforeTheWorkshop(FUnitTest& T)
{
	TArray<FHOLAITechnology> Table;
	T.AssertTrue(HOLAITechnology::ReadAITechnologyTable(Table), "the real table is there");
	const int WorkshopTier = HOLSiegeCheck::GetTier(int(EHOLSiegeEngine::Ram));
	int AtTheWorkshop = 0;
	for (int Index = 0; Index < Table.Num(); Index++)
	{
		if (Table[Index].Building != n"SiegeWorkshop")
			continue;
		AtTheWorkshop++;
		T.AssertTrue(Table[Index].MinTier >= WorkshopTier, f"{Table[Index].Name}: not researched before its workshop exists (tier {Table[Index].MinTier}, workshop tier {WorkshopTier})");
	}
	T.AssertTrue(AtTheWorkshop > 0, "the siege workshop has technologies");
}

// Siege.TechnologyHealth, the rounding the C++ test does not pin (it only has 400): to nearest, once; the percent is on the health
// alone. This is the script's HealthWithTechnology; HOLCombat::GetMaxHealth itself (the C++ rule, real unit rows) has no script library
// and stays tested in C++.
void Test_SiegeHealthRounding(FUnitTest& T)
{
	T.AssertEquals(6, HOLSiege::HealthWithTechnology(5, 10), "5 + 10 % = 5.5 rounds up to 6");
	T.AssertEquals(5, HOLSiege::HealthWithTechnology(5, 8), "5 + 8 % = 5.4 rounds down to 5");
	T.AssertEquals(17, HOLSiege::HealthWithTechnology(15, 10), "15 + 10 % = 16.5 rounds up to 17");
	T.AssertEquals(0, HOLSiege::HealthWithTechnology(0, 20), "no health stays none");
	T.AssertEquals(15, HOLSiege::HealthWithTechnology(15, -50), "a negative percent is no loss");
	T.AssertTrue(Math::Abs(HOLSiege::AttackWithTechnology(45, -20) - 45.f) < 0.01f, "a negative attack percent is no loss");
	T.AssertTrue(Math::Abs(HOLSiege::AttackWithTechnology(45, 0) - 45.f) < 0.01f, "0 percent changes nothing");
}
