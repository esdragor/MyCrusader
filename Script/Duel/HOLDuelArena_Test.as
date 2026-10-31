// The duel's arena rules, alone (DuelInProgress.md D5): every expectation is read from the settings or from the arena made.

// The preset gives the arena a radius, short (a few metres), and keeps it under the aura's reach so that a duel stays
// on the battle around it (10.2: the battle goes on around).
void Test_DuelArenaPresetRadius(FUnitTest& T)
{
	FHOLDuelSettings S = HOLDuel::Preset();
	T.AssertTrue(S.ArenaRadiusCm > 0, "the preset has an arena radius");
	T.AssertTrue(S.ArenaRadiusCm <= 1200, "a short radius (under the hero's 12 m aura)");
}

// The circle is centred between the two heroes (whole centimetres, truncated toward zero) and has the settings' radius.
void Test_DuelArenaCentredBetweenTheHeroes(FUnitTest& T)
{
	FHOLDuelSettings S = HOLDuel::Preset();
	FHOLDuelArena Arena = HOLDuel::MakeArena(S, 1000, 2000, 1200, 2200);
	T.AssertEquals(1100, Arena.CenterX, "centre x is the middle");
	T.AssertEquals(2100, Arena.CenterY, "centre y is the middle");
	T.AssertEquals(S.ArenaRadiusCm, Arena.RadiusCm, "the settings' radius");
	FHOLDuelArena Odd = HOLDuel::MakeArena(S, -301, 101, 100, -50);
	T.AssertEquals(-100, Odd.CenterX, "negative coordinates truncate toward zero: (-301 + 100) / 2");
	T.AssertEquals(25, Odd.CenterY, "(101 - 50) / 2");
}

// Both duellists stand inside the circle made from their positions; the edge counts; one centimetre further does not.
void Test_DuelArenaHoldsTheDuellists(FUnitTest& T)
{
	FHOLDuelSettings S = HOLDuel::Preset();
	FHOLDuelArena Arena = HOLDuel::MakeArena(S, 0, 0, 200, 0);
	T.AssertTrue(HOLDuel::InArena(Arena, 0, 0), "the first duellist");
	T.AssertTrue(HOLDuel::InArena(Arena, 200, 0), "the second duellist");
	T.AssertTrue(HOLDuel::InArena(Arena, Arena.CenterX + Arena.RadiusCm, Arena.CenterY), "the edge counts");
	T.AssertFalse(HOLDuel::InArena(Arena, Arena.CenterX + Arena.RadiusCm + 1, Arena.CenterY), "one centimetre further is outside");
	T.AssertFalse(HOLDuel::InArena(Arena, Arena.CenterX, Arena.CenterY - Arena.RadiusCm - 1), "outside on the other axis");
}

// "Short distance": the two heroes fit together in one arena, no further than twice the radius apart.
void Test_DuelArenaShortDistance(FUnitTest& T)
{
	FHOLDuelSettings S = HOLDuel::Preset();
	int Diameter = S.ArenaRadiusCm * 2;
	T.AssertTrue(HOLDuel::FitsInArena(S, 0, 0, Diameter, 0), "a diameter apart fits");
	T.AssertFalse(HOLDuel::FitsInArena(S, 0, 0, Diameter + 1, 0), "one centimetre more does not");
	T.AssertFalse(HOLDuel::FitsInArena(S, -Diameter, 0, 1, 0), "nor on the other side of the origin");
	T.AssertTrue(HOLDuel::FitsInArena(S, 500, 500, 500, 500), "the same place fits");
}
