// Lot E3-02 of the migration (D830, D836, D839): the script port of HOLFactionChoiceTests.cpp (3 of its 4 tests), run against
// UHOLFactionChoiceReadLibrary (namespace HOLFactionChoiceRead, S1D-1116c), the read library of the rule that STAYS in C++ (HOLFactionChoice).
// Same vectors as the C++ tests. NOT ported: HeroesOfLiberty.Net.FactionChoice.Bodies (it calls FHOLManagerClientRules::MakeMeBody,
// MakeAISlotBody and ParseLobby, functions of the manager outside this library). The C++ file is removed at lot MIG-C++-FINAL.

// Mirror of the C++ loop "for bCheat in { false, true }" of the List test.
void CheckFactionListFor(FUnitTest& T, bool bCheat)
{
	T.AssertTrue(HOLFactionChoiceRead::IsChoosable(EHOLFaction::DawnOrder, bCheat), "the Dawn is chosen");
	T.AssertTrue(HOLFactionChoiceRead::IsChoosable(EHOLFaction::BlackLegions, bCheat), "the Legions are chosen");
	TArray<EHOLFaction> Greyed;
	Greyed.Add(EHOLFaction::DragonChildren);
	Greyed.Add(EHOLFaction::ShadowCircle);
	Greyed.Add(EHOLFaction::FireHeirs);
	for (int Index = 0; Index < Greyed.Num(); ++Index)
	{
		T.AssertTrue(HOLFactionChoiceRead::IsListed(Greyed[Index], bCheat), "Dragon, Shadow, Fire are listed...");
		T.AssertFalse(HOLFactionChoiceRead::IsChoosable(Greyed[Index], bCheat), "... but greyed, cheat or not");
	}
}

// HeroesOfLiberty.Match.FactionChoice.List: the faction list of "Nouvelle partie" and the cheat that adds the Templars.
void Test_FactionChoiceList(FUnitTest& T)
{
	CheckFactionListFor(T, false);
	CheckFactionListFor(T, true);

	T.AssertFalse(HOLFactionChoiceRead::IsListed(EHOLFaction::Templars, false), "without the cheat the Templars are not listed (not greyed: the cheat stays secret)");
	T.AssertFalse(HOLFactionChoiceRead::IsChoosable(EHOLFaction::Templars, false), "... nor chosen");
	T.AssertTrue(HOLFactionChoiceRead::IsListed(EHOLFaction::Templars, true), "with the cheat they are listed");
	T.AssertTrue(HOLFactionChoiceRead::IsChoosable(EHOLFaction::Templars, true), "... and chosen");

	TArray<EHOLFaction> Without = HOLFactionChoiceRead::GetListed(false);
	T.AssertEquals(5, Without.Num(), "five entries without the cheat");
	TArray<EHOLFaction> With = HOLFactionChoiceRead::GetListed(true);
	T.AssertEquals(6, With.Num(), "six with it");
	for (int Index = 0; Index < With.Num(); ++Index)
	{
		T.AssertEquals(Index, int(With[Index]), "the drop-down's index is the enum's value");
	}

	T.AssertTrue(HOLFactionChoiceRead::Sanitize(EHOLFaction::Templars, false) == EHOLFaction::DawnOrder, "a Templar slot without the cheat plays the Dawn");
	T.AssertTrue(HOLFactionChoiceRead::Sanitize(EHOLFaction::Templars, true) == EHOLFaction::Templars, "with the cheat it stays");
	T.AssertTrue(HOLFactionChoiceRead::Sanitize(EHOLFaction::BlackLegions, false) == EHOLFaction::BlackLegions, "the Legions are never touched");
}

// HeroesOfLiberty.Match.FactionChoice.Names: the manager's name of a faction, and the way back.
void Test_FactionChoiceNames(FUnitTest& T)
{
	T.AssertTrue(HOLFactionChoiceRead::GetName(EHOLFaction::Templars) == "Templars", "the manager's name of the Templars");
	T.AssertTrue(HOLFactionChoiceRead::GetName(EHOLFaction::DawnOrder) == "DawnOrder", "... of the Dawn");
	for (int Value = 0; Value <= int(EHOLFaction::Templars); ++Value)
	{
		EHOLFaction Faction = EHOLFaction(Value);
		EHOLFaction Back = EHOLFaction::Templars;
		T.AssertTrue(HOLFactionChoiceRead::TryFindByName(HOLFactionChoiceRead::GetName(Faction), Back), "every name reads back");
		T.AssertTrue(Back == Faction, "... to the same faction");
	}
	EHOLFaction Unknown = EHOLFaction::BlackLegions;
	T.AssertFalse(HOLFactionChoiceRead::TryFindByName("Pirates", Unknown), "an unknown name is refused");
	T.AssertFalse(HOLFactionChoiceRead::TryFindByName("", Unknown), "an empty name too");
}

// HeroesOfLiberty.Match.FactionChoice.TripleClick: the cheat flag and the triple click on the title.
void Test_FactionChoiceTripleClick(FUnitTest& T)
{
	bool bWas = HOLFactionChoiceRead::IsCheatOn();
	HOLFactionChoiceRead::SetCheatOn(false);
	T.AssertFalse(HOLFactionChoiceRead::IsCheatOn(), "off by default");
	HOLFactionChoiceRead::SetCheatOn(true);
	T.AssertTrue(HOLFactionChoiceRead::IsCheatOn(), "on");
	HOLFactionChoiceRead::SetCheatOn(bWas);

	FHOLClickCounter Clicks;
	T.AssertFalse(HOLFactionChoiceRead::Click(Clicks, 0.0), "one click");
	T.AssertFalse(HOLFactionChoiceRead::Click(Clicks, 0.3), "two clicks");
	T.AssertTrue(HOLFactionChoiceRead::Click(Clicks, 0.6), "three quick clicks are a triple");
	T.AssertFalse(HOLFactionChoiceRead::Click(Clicks, 0.7), "a fourth does not make another");

	FHOLClickCounter Slow;
	T.AssertFalse(HOLFactionChoiceRead::Click(Slow, 0.0), "slow: 1");
	T.AssertFalse(HOLFactionChoiceRead::Click(Slow, 2.1), "slow: 2 (the gap 2.1 s is over the window)");
	T.AssertFalse(HOLFactionChoiceRead::Click(Slow, 2.4), "slow: 3 (counted from the second)");
	T.AssertTrue(HOLFactionChoiceRead::Click(Slow, 2.7), "slow: three within the window after the restart");

	FHOLClickCounter Six;
	int Triples = 0;
	for (int Click = 0; Click < 6; ++Click)
	{
		if (HOLFactionChoiceRead::Click(Six, 0.1 * Click))
		{
			Triples += 1;
		}
	}
	T.AssertEquals(2, Triples, "six quick clicks are two triples");

	FHOLClickCounter Edge;
	HOLFactionChoiceRead::Click(Edge, 0.0);
	HOLFactionChoiceRead::Click(Edge, 2.0);
	T.AssertTrue(HOLFactionChoiceRead::Click(Edge, 4.0), "a gap of exactly the window still counts");
}
