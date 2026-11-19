// The generic faction resource gauge (HOLFactionResourceRules.as). The bounds are fixture values (floor 0, cap 100), never a
// value of the game: the Honor's cap and start are not decided (D345).

// Gain: adds, stops at the cap and says the surplus lost; a null or negative gain changes nothing.
void Test_FactionResourceGainCapsAndLosesSurplus(FUnitTest& T)
{
	FHOLFactionResource Gauge = HOLFactionResource::Make(30, 0, 100);
	T.AssertEquals(30, Gauge.Value, "starts at 30");
	const int LostA = HOLFactionResource::Gain(Gauge, 50);
	T.AssertEquals(80, Gauge.Value, "30 + 50");
	T.AssertEquals(0, LostA, "nothing lost below the cap");
	const int LostB = HOLFactionResource::Gain(Gauge, 50);
	T.AssertEquals(100, Gauge.Value, "capped at 100");
	T.AssertEquals(30, LostB, "50 - 20 room = 30 lost");
	const int LostC = HOLFactionResource::Gain(Gauge, 10);
	T.AssertEquals(100, Gauge.Value, "still 100");
	T.AssertEquals(10, LostC, "all of it lost at the cap");
	const int LostD = HOLFactionResource::Gain(Gauge, -5);
	T.AssertEquals(100, Gauge.Value, "a negative gain changes nothing");
	T.AssertEquals(0, LostD, "and loses nothing");
	const FString Line = HOLFactionResource::Describe("res", Gauge);
	T.AssertEquals("res 100/100", Line, "described");
}

// TrySpend: accepted when the stock allows it (down to the floor exactly), refused when short and then the stock is intact.
void Test_FactionResourceSpendRefusedWhenShort(FUnitTest& T)
{
	FHOLFactionResource Gauge = HOLFactionResource::Make(40, 0, 100);
	const bool bShort = HOLFactionResource::TrySpend(Gauge, 41);
	T.AssertFalse(bShort, "41 of 40 refused");
	T.AssertEquals(40, Gauge.Value, "stock intact after the refusal");
	const bool bOk = HOLFactionResource::TrySpend(Gauge, 15);
	T.AssertTrue(bOk, "15 of 40 accepted");
	T.AssertEquals(25, Gauge.Value, "25 left");
	const bool bAll = HOLFactionResource::TrySpend(Gauge, 25);
	T.AssertTrue(bAll, "the whole stock accepted");
	T.AssertEquals(0, Gauge.Value, "down to the floor");
	const bool bEmpty = HOLFactionResource::TrySpend(Gauge, 1);
	T.AssertFalse(bEmpty, "1 of 0 refused");
	T.AssertEquals(0, Gauge.Value, "still 0");
	// A floor above 0: the stock is what lies above it.
	FHOLFactionResource Raised = HOLFactionResource::Make(12, 10, 50);
	const bool bBelowFloor = HOLFactionResource::TrySpend(Raised, 3);
	T.AssertFalse(bBelowFloor, "spending below the floor refused");
	T.AssertEquals(12, Raised.Value, "stock intact above the raised floor");
}

// Lose: removes, stops at the floor and says what was really removed.
void Test_FactionResourceLoseStopsAtFloor(FUnitTest& T)
{
	FHOLFactionResource Gauge = HOLFactionResource::Make(30, 0, 100);
	const int TakenA = HOLFactionResource::Lose(Gauge, 20);
	T.AssertEquals(10, Gauge.Value, "30 - 20");
	T.AssertEquals(20, TakenA, "20 removed");
	const int TakenB = HOLFactionResource::Lose(Gauge, 20);
	T.AssertEquals(0, Gauge.Value, "stops at the floor");
	T.AssertEquals(10, TakenB, "only 10 really removed");
	const int TakenC = HOLFactionResource::Lose(Gauge, -4);
	T.AssertEquals(0, Gauge.Value, "a negative loss changes nothing");
	T.AssertEquals(0, TakenC, "and removes nothing");
	FHOLFactionResource Raised = HOLFactionResource::Make(15, 10, 50);
	const int TakenD = HOLFactionResource::Lose(Raised, 30);
	T.AssertEquals(10, Raised.Value, "stops at a floor of 10");
	T.AssertEquals(5, TakenD, "5 removed");
}

// HasResource (EHOLFaction): the Dawn alone has a gauge today; every other value of the enumeration has none (the Legions' corpses
// have their own model, HOLCorpse). Walks the whole enumeration so a faction added later without a rule reads as having none.
void Test_FactionResourceOnlyDawnHasOne(FUnitTest& T)
{
	const bool bDawn = HOLFactionResource::HasResource(EHOLFaction::DawnOrder);
	const bool bLegions = HOLFactionResource::HasResource(EHOLFaction::BlackLegions);
	T.AssertTrue(bDawn, "the Dawn has one");
	T.AssertFalse(bLegions, "the Legions have none");
	for (int Value = 0; Value <= int(EHOLFaction::Templars); ++Value)
	{
		const EHOLFaction Faction = EHOLFaction(Value);
		if (Faction == EHOLFaction::DawnOrder)
			continue;
		const bool bHas = HOLFactionResource::HasResource(Faction);
		T.AssertFalse(bHas, f"faction {Value} (not the Dawn) has none");
	}
}

// The Templars (and the other factions without a rule of their own) have no gauge: only the Dawn's Honor is a resource of this model.
void Test_FactionResourceTemplarsHaveNone(FUnitTest& T)
{
	const bool bTemplars = HOLFactionResource::HasResource(EHOLFaction::Templars);
	const bool bDragon = HOLFactionResource::HasResource(EHOLFaction::DragonChildren);
	T.AssertFalse(bTemplars, "the Templars have none");
	T.AssertFalse(bDragon, "the Dragon's Children have none");
}