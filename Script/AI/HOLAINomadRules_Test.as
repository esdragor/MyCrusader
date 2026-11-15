// The Nomad rule of the AI alone (N3, HOLAINomadRules.as): where and when an AI slot without a forum founds its first one (Rule 1 of
// hol.AI.NomadForumRule: the largest group of Peasants, as soon as the stock covers the price; Retours/Q-nomad-ia.md not answered). The
// stock comes from HOLNomad::StartingAmount (D402), the before-first-forum flag from FHOLNomadState (D354), the starting groups from
// HOLNomad::SplitIntoGroups (D786): the asserted places and groups are computed by the rules, never retyped. Not wired to the game yet.

TArray<int> NmdPrice()
{
	// A forum price for the test (food, wood, stone, gold); the real one is the building table's, read by the coming C++ caller.
	TArray<int> Price;
	Price.Add(0);
	Price.Add(150);
	Price.Add(400);
	Price.Add(0);
	return Price;
}

// The Nomad starting stock (D402): a standard stock plus the forum price, resource by resource.
TArray<int> NmdStartStock(const TArray<int>& Price)
{
	TArray<int> Standard;
	Standard.Add(200);
	Standard.Add(200);
	Standard.Add(100);
	Standard.Add(100);
	TArray<int> Stock;
	for (int Index = 0; Index < Price.Num(); Index++)
		Stock.Add(HOLNomad::StartingAmount(Standard[Index], Price[Index], true));
	return Stock;
}

float NmdFlat(FVector A, FVector B)
{
	return FVector(A.X - B.X, A.Y - B.Y, 0.f).Size();
}

// Five Peasants: three together in the south-west, two alone far away.
TArray<FVector> NmdThreeAndTwo()
{
	TArray<FVector> Places;
	Places.Add(FVector(10000.f, 0.f, 0.f));
	Places.Add(FVector(-5000.f, -5000.f, 0.f));
	Places.Add(FVector(-5300.f, -4800.f, 0.f));
	Places.Add(FVector(-4900.f, -5400.f, 0.f));
	Places.Add(FVector(0.f, 9000.f, 0.f));
	return Places;
}

void Test_AINomadFoundsAmongTheLargestGroup(FUnitTest& T)
{
	const TArray<FVector> Places = NmdThreeAndTwo();
	const float Radius = HOLAINomad::GroupRadiusFromCVar();
	const TArray<int> Price = NmdPrice();
	FHOLNomadState State = HOLNomad::NewState(true);
	const FHOLAINomadPlan Plan = HOLAINomad::Plan(HOLAINomad::RuleFromCVar(), HOLNomad::IsBeforeFirstForum(State), false, Places,
		NmdStartStock(Price), Price, Radius);
	T.AssertTrue(Plan.bFound, "a Nomad AI with the forum price in its starting stock founds at its first thought (D402)");
	const TArray<int> Groups = HOLAINomad::GroupsOf(Places, Radius);
	T.AssertEquals(Groups.Num(), Places.Num(), "one group per Peasant");
	if (Groups.Num() != Places.Num())
		return;
	T.AssertEquals(Plan.Group, Groups[1], "the chosen group is the one of the three Peasants together");
	const TArray<int> Sizes = HOLAINomad::GroupSizes(Groups);
	T.AssertEquals(Sizes.Num(), 3, "three groups: the three together and the two alone");
	T.AssertTrue(Plan.Group >= 0 && Plan.Group < Sizes.Num() && Sizes[Plan.Group] == 3, "the chosen group holds three Peasants");
	for (int Index = 1; Index <= 3; Index++)
		T.AssertTrue(NmdFlat(Plan.Site, Places[Index]) <= Radius, "the foundation lies among the three Peasants");
	T.AssertTrue(NmdFlat(Plan.Site, Places[0]) > Radius && NmdFlat(Plan.Site, Places[4]) > Radius, "not at a Peasant alone");
}

void Test_AINomadGroupsJoinByNeighbours(FUnitTest& T)
{
	TArray<FVector> Places;
	Places.Add(FVector(0.f, 0.f, 0.f));
	Places.Add(FVector(600.f, 0.f, 0.f));
	Places.Add(FVector(1200.f, 0.f, 0.f));
	Places.Add(FVector(5000.f, 0.f, 0.f));
	const TArray<int> Groups = HOLAINomad::GroupsOf(Places, 700.f);
	T.AssertEquals(Groups.Num(), 4, "one group per Peasant");
	if (Groups.Num() != 4)
		return;
	T.AssertEquals(Groups[0], 0, "groups are numbered from the first Peasant");
	T.AssertEquals(Groups[2], Groups[0], "12 m apart but 6 m from a common neighbour: the same group");
	T.AssertTrue(Groups[3] != Groups[0], "50 m away: another group");
}

void Test_AINomadWaitsForTheWholePrice(FUnitTest& T)
{
	const TArray<FVector> Places = NmdThreeAndTwo();
	const float Radius = HOLAINomad::GroupRadiusFromCVar();
	const TArray<int> Price = NmdPrice();
	TArray<int> Spent = NmdStartStock(Price);
	Spent[1] = Price[1] - 1;
	T.AssertTrue(!HOLAINomad::StockCovers(Spent, Price), "one wood short of the forum price: not covered");
	const FHOLAINomadPlan Short = HOLAINomad::Plan(HOLAINomad::RuleLargestGroupAtOnce, true, false, Places, Spent, Price, Radius);
	T.AssertTrue(!Short.bFound, "the wood spent elsewhere (D354): the AI waits until it has the whole price again");
	Spent[1] = Price[1];
	const FHOLAINomadPlan Again = HOLAINomad::Plan(HOLAINomad::RuleLargestGroupAtOnce, true, false, Places, Spent, Price, Radius);
	T.AssertTrue(Again.bFound, "the price gathered again: it founds");
}

void Test_AINomadFoundsOnlyOnceAndOnlyNomad(FUnitTest& T)
{
	const TArray<FVector> Places = NmdThreeAndTwo();
	const float Radius = HOLAINomad::GroupRadiusFromCVar();
	const TArray<int> Price = NmdPrice();
	const TArray<int> Stock = NmdStartStock(Price);
	const int Rule = HOLAINomad::RuleLargestGroupAtOnce;
	T.AssertTrue(!HOLAINomad::Plan(Rule, true, true, Places, Stock, Price, Radius).bFound, "a foundation underway: no second one");
	FHOLNomadState Done = HOLNomad::NewState(true);
	HOLNomad::NoteForumComplete(Done);
	T.AssertTrue(!HOLAINomad::Plan(Rule, HOLNomad::IsBeforeFirstForum(Done), false, Places, Stock, Price, Radius).bFound,
		"the first forum complete: the normal rules (D354), the Nomad rule is over");
	FHOLNomadState Standard = HOLNomad::NewState(false);
	T.AssertTrue(!HOLAINomad::Plan(Rule, HOLNomad::IsBeforeFirstForum(Standard), false, Places, Stock, Price, Radius).bFound,
		"a standard start: the main forum is there, nothing to found");
	T.AssertTrue(!HOLAINomad::Plan(HOLAINomad::RuleNone, true, false, Places, Stock, Price, Radius).bFound,
		"rule 0 (hol.AI.NomadForumRule 0): the AI never founds by itself");
}

void Test_AINomadD786StartFoundsAtTheFirstGroup(FUnitTest& T)
{
	// D786: one Peasant per starting group; the groups lie far apart (D797 scatter): every group has one Peasant, a tie.
	const TArray<int> Sizes = HOLNomad::SplitIntoGroups(5);
	TArray<FVector> Places;
	for (int Group = 0; Group < Sizes.Num(); Group++)
	{
		for (int Peasant = 0; Peasant < Sizes[Group]; Peasant++)
			Places.Add(FVector(Group * 4000.f, Peasant * 100.f, 0.f));
	}
	const float Radius = HOLAINomad::GroupRadiusFromCVar();
	const TArray<int> Price = NmdPrice();
	const FHOLAINomadPlan Plan = HOLAINomad::Plan(HOLAINomad::RuleFromCVar(), true, false, Places, NmdStartStock(Price), Price, Radius);
	const TArray<int> Groups = HOLAINomad::GroupsOf(Places, Radius);
	T.AssertEquals(HOLAINomad::GroupSizes(Groups).Num(), Sizes.Num(), "the starting groups are found again from the places alone");
	T.AssertTrue(Plan.bFound, "the stock covers the price at the start");
	if (Groups.Num() != Places.Num())
		return;
	T.AssertEquals(Plan.Group, Groups[0], "a tie between groups of one: the first group (the King's in Regicide), the same on every machine");
	T.AssertTrue(NmdFlat(Plan.Site, Places[0]) < 1.f, "the foundation at the first group's Peasant");
}
