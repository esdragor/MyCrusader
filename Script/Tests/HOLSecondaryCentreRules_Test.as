// Secondary centres, pure rules (SC1, HOLSecondaryCentreRules.as; GDD 5.4, D384, with D22, D347, D351, D388, D397, D403, 11.1).
// A secondary centre is a real forum more, as AoE4's town centre: Peasants, deposit of every resource, garrison and fire, a population base,
// a place for the hero's return; unlocked at tier 1, no cap; no lives, never a protection from defeat, no anti-build or anti-duel zone.
// Values (cost, population base) are the rules' provisional constants, read from them (a test does not retype a value).

void Test_SecondaryCentreUnlockedAtTierOneWithoutCap(FUnitTest& T)
{
	T.AssertTrue(!HOLSecondaryCentre::CanOrder(0, 0), "tier 0: not yet (GDD 11.1, Essor)");
	T.AssertTrue(HOLSecondaryCentre::CanOrder(1, 0), "tier 1: the first one");
	T.AssertTrue(HOLSecondaryCentre::CanOrder(2, 0), "tier 2 as well");
	T.AssertTrue(HOLSecondaryCentre::CanOrder(1, 1), "a second one at tier 1");
	T.AssertTrue(HOLSecondaryCentre::CanOrder(1, 12), "no cap: the boom is a bet, punished by the rush (D384)");
	T.AssertTrue(HOLSecondaryCentre::CanOrder(3, 40), "no cap at any tier");
}

void Test_SecondaryCentreCostsMostlyStone(FUnitTest& T)
{
	HOLSecondaryCentre::FHOLCentreCost Cost = HOLSecondaryCentre::Cost();
	T.AssertTrue(Cost.Stone > 0, "it costs stone (D384, 4.x: stone builds the secondary centres)");
	T.AssertTrue(Cost.Stone >= Cost.Food && Cost.Stone >= Cost.Wood && Cost.Stone >= Cost.Gold, "stone is the largest part");
	T.AssertTrue(Cost.Food + Cost.Wood + Cost.Stone + Cost.Gold > 0, "costly, never free as the main forum");
}

void Test_SecondaryCentreStandingIsAForumMore(FUnitTest& T)
{
	HOLSecondaryCentre::FHOLCentreRole Role = HOLSecondaryCentre::RoleOf(false, true, false);
	T.AssertTrue(Role.bProducesPeasants, "it trains Peasants");
	T.AssertTrue(Role.bTakesAllResources, "it takes every resource");
	T.AssertTrue(Role.bGarrison, "it holds a garrison");
	T.AssertTrue(Role.bFires, "it fires");
	T.AssertEquals(HOLSecondaryCentre::PopulationBase, Role.Population, "it gives its population base");
	T.AssertTrue(Role.bRespawnPlace, "the hero can come back there (D397)");
	T.AssertTrue(Role.bTerritory, "it counts for the territory XP (D388)");
	T.AssertEquals(0, Role.Lives, "no lives (D384)");
	T.AssertTrue(!Role.bCountsForDefeat, "it never protects from defeat: only the main forum counts (D347)");
	T.AssertTrue(!Role.bFallsIntoRuin, "it is destroyed as any building, no ruin (D351 is the main forum's)");
	T.AssertTrue(!Role.bAntiBuildZone, "no anti-build zone around it (D22: the main centre only)");
	T.AssertTrue(!Role.bAntiDuelZone, "no anti-duel zone around it (D403)");
}

void Test_SecondaryCentreUnfinishedDoesNothing(FUnitTest& T)
{
	HOLSecondaryCentre::FHOLCentreRole Role = HOLSecondaryCentre::RoleOf(false, false, false);
	T.AssertTrue(!Role.bProducesPeasants && !Role.bTakesAllResources && !Role.bGarrison && !Role.bFires, "a foundation does not work yet");
	T.AssertEquals(0, Role.Population, "no population before it stands");
	T.AssertTrue(!Role.bRespawnPlace, "no return to a foundation");
	T.AssertTrue(!Role.bTerritory, "no territory before it stands");
}

void Test_MainForumAgainstTheSecondaryCentre(FUnitTest& T)
{
	const int MainLives = 3;
	HOLSecondaryCentre::FHOLCentreRole Main = HOLSecondaryCentre::RoleOf(true, true, false, MainLives);
	T.AssertEquals(MainLives, Main.Lives, "the main forum keeps its lives");
	T.AssertTrue(Main.bCountsForDefeat && Main.bFallsIntoRuin, "the main forum alone decides the defeat (D347)");
	T.AssertTrue(Main.bAntiBuildZone && Main.bAntiDuelZone, "its zones stay (D22, D403)");
	T.AssertTrue(!Main.bTerritory, "the territory counts the secondary centres and the relays, not the main forum (D388)");

	// The main forum down (in ruin or rebuilding, D351): no production, deposit, fire or garrison; the return, the population and the
	// zones stay.
	HOLSecondaryCentre::FHOLCentreRole Down = HOLSecondaryCentre::RoleOf(true, true, true, MainLives);
	T.AssertTrue(!Down.bProducesPeasants && !Down.bTakesAllResources && !Down.bGarrison && !Down.bFires, "down: cut");
	T.AssertTrue(Down.bRespawnPlace, "down: the hero still comes back at the ruin");
	T.AssertTrue(Down.Population > 0, "down: its population base stays");
	T.AssertTrue(Down.bAntiBuildZone && Down.bAntiDuelZone, "down: its zones stay");
}

void Test_RespawnPlacesAreTheMainForumAndOwnStandingCentres(FUnitTest& T)
{
	T.AssertTrue(HOLSecondaryCentre::IsRespawnPlace(true, true, true, false), "its own main forum");
	T.AssertTrue(HOLSecondaryCentre::IsRespawnPlace(true, true, true, true), "its own main forum in ruin (D351)");
	T.AssertTrue(HOLSecondaryCentre::IsRespawnPlace(false, true, true, false), "its own secondary centre, standing");
	T.AssertTrue(!HOLSecondaryCentre::IsRespawnPlace(false, true, false, false), "not its own foundation");
	T.AssertTrue(!HOLSecondaryCentre::IsRespawnPlace(false, false, true, false), "not another player's centre");
	T.AssertTrue(!HOLSecondaryCentre::IsRespawnPlace(true, false, true, false), "not another player's main forum");
}

void Test_SecondaryCentresHeldGiveTerritoryXP(FUnitTest& T)
{
	// Three standing secondary centres, one foundation, and the main forum: the territory XP of one minute is the rate of the three.
	TArray<HOLSecondaryCentre::FHOLCentreRole> Centres;
	Centres.Add(HOLSecondaryCentre::RoleOf(true, true, false, 2));
	Centres.Add(HOLSecondaryCentre::RoleOf(false, true, false));
	Centres.Add(HOLSecondaryCentre::RoleOf(false, true, false));
	Centres.Add(HOLSecondaryCentre::RoleOf(false, true, false));
	Centres.Add(HOLSecondaryCentre::RoleOf(false, false, false));
	const int Held = HOLSecondaryCentre::CountTerritory(Centres);
	T.AssertEquals(3, Held, "the three standing secondary centres");

	HOLRelay::FHOLTerritoryCarry Memory;
	int XP = 0;
	for (int Step = 0; Step < 60 * HOLRelay::StepsPerSecond; Step++)
		XP += HOLRelay::TerritoryXP(Memory, 0, Held);
	T.AssertEquals(3 * HOLRelay::CentreXPPerMinute, XP, "a minute of three centres gives the hero three times the centre rate");

	T.AssertEquals(3 * HOLSecondaryCentre::PopulationBase, HOLSecondaryCentre::PopulationOf(Centres) - Centres[0].Population,
		"each standing secondary centre adds its population base, the foundation nothing");
}
