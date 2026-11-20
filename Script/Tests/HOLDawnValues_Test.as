// The starting values of the Dawn's Honor (D859, step 1a; HOLDawnValues.as): every assertion is an EFFECT computed by the real
// chain (HOLFactionResource::Make / Gain / Lose, HOLGroundAbility zone and cool-down rules), never a constant copied back.

// A gauge made from the sources starts at 0 of 100; accept (+10), win (+30), refuse (-5) leave 35; a gain of 1000 on a fresh
// gauge fills it and the surplus lost is 900 (the cap is read from the source); kills and heals add the points of the table.
void Test_HonorStartingValues(FUnitTest& T)
{
	FHOLFactionResource Honor = HOLFactionResource::Make(HOLDawnValues::HonorStart(), 0, HOLDawnValues::HonorCap());
	T.AssertEquals("Honor 0/100", HOLFactionResource::Describe("Honor", Honor), "fresh gauge: starts at 0 under a cap of 100");

	HOLFactionResource::Gain(Honor, HOLDawnValues::HonorDuelAccept());
	HOLFactionResource::Gain(Honor, HOLDawnValues::HonorDuelWin());
	HOLFactionResource::Lose(Honor, HOLDawnValues::HonorDuelRefuse());
	T.AssertEquals(35, Honor.Value, "accept +10, win +30, refuse -5 leave 35");

	FHOLFactionResource Fresh = HOLFactionResource::Make(HOLDawnValues::HonorStart(), 0, HOLDawnValues::HonorCap());
	const int Surplus = HOLFactionResource::Gain(Fresh, 1000);
	T.AssertEquals(900, Surplus, "a gain of 1000 on an empty gauge loses 900 above the cap of 100");
	T.AssertEquals(100, Fresh.Value, "the gauge is full");

	// Combat: three kills near a building, 230 hit points healed.
	FHOLFactionResource Combat = HOLFactionResource::Make(HOLDawnValues::HonorStart(), 0, HOLDawnValues::HonorCap());
	HOLFactionResource::Gain(Combat, 3 * HOLDawnValues::HonorKillGain());
	const int HealPerPoint = HOLDawnValues::HonorHealPerPoint();
	const int HealPoints = HealPerPoint > 0 ? 230 / HealPerPoint : 0;
	HOLFactionResource::Gain(Combat, HealPoints);
	T.AssertEquals(7, Combat.Value, "3 kills (+3) and 230 hit points healed (+4) give 7");

	// The kill radius is 12 m: a kill at 11.99 m of the building counts, one at 12.01 m does not.
	const int KillRadius = HOLDawnValues::HonorKillRadiusCm();
	T.AssertTrue(HOLGroundAbility::IsInZone(0, 0, KillRadius, 1199, 0), "a kill at 11.99 m is near the building");
	T.AssertFalse(HOLGroundAbility::IsInZone(0, 0, KillRadius, 1201, 0), "a kill at 12.01 m is not");
}

// The Knight's challenge (provisional values): 8 m zone around the Knight, 8 s long, usable again 45 s after the cast.
void Test_KnightChallengeValues(FUnitTest& T)
{
	const int Radius = HOLDawnValues::RadiusCm();
	T.AssertTrue(HOLGroundAbility::IsInZone(0, 0, Radius, 799, 0), "a target at 7.99 m is challenged");
	T.AssertFalse(HOLGroundAbility::IsInZone(0, 0, Radius, 801, 0), "a target at 8.01 m is not");

	const int Duration = HOLDawnValues::KnightChallengeDurationMs();
	T.AssertTrue(HOLGroundAbility::ZoneActive(1000, Duration, 1000 + 7999), "the challenge lasts until 8 s");
	T.AssertFalse(HOLGroundAbility::ZoneActive(1000, Duration, 1000 + 8001), "and is over after 8 s");

	const int CooldownEnd = HOLGroundAbility::CooldownEndMs(1000, HOLDawnValues::CooldownMs());
	T.AssertEquals(46000, CooldownEnd, "cast at 1 s: usable again at 46 s");
}
