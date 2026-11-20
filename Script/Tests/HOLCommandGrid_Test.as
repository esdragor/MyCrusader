// G0 (GALADRIEL-1119ba): these 15 tests now run the SCRIPT rules the game reads (HOLCommandGridRules / Keys / Groups); only the table lookups with no script twin (CheckResearch, GetCycleRankOfType, WithTableBuildables, GetProducesOf, GetBuildingResearches, IsTypeInTable, GetTypeCategory, technology and block numbers) still go through HOLCommandGridRead. The agreement sweeps against the removed *Native bodies are gone (one source, D836); the constants and tables that stay are compared in Test_CGRAgreementTables and Test_CGKYAgreementFeedback.
// MIG-C-CommandGrid (D830, D836, D839, GALADRIEL-1117m): the script port of HOLCommandGridTests.cpp, run against the read library of the command grid's
// rules that STAY in C++ (HOLCommandGrid, Player/HOLCommandGrid.h, read by the HUD): UHOLCommandGridReadLibrary (namespace HOLCommandGridRead). Same
// vectors as the C++ tests. Ported here (15 of 15, GALADRIEL-1117o): MarketGain, Market, Commands, Feedback, Cycle, Categories, Group, Subgroups,
// Distribution, Tables (both halves: builders' grid, Barracks, Archery range, Siege workshop, Chapel), UnitOrders, HeroAbilities, Ages, MouseCursor,
// Research. The C++ file stays until the rouge vu / vert is confirmed by the main session. The library (555ca10) is NOT COMPILED at the
// time of writing: if a Test_CG* fails to compile, the cause is in the .cpp or in a name used here (HOLBuildingTypeNames strings). Not read by any
// test (not exposed): the specialization view (SetSpecialization, GetOpenSpecializationTier).

FHOLGridContextView CGMakeBuilders()
{
	FHOLGridContextView Context;
	Context.bBuilders = true;
	Context.BuildableTypes.Add(n"House");
	Context.BuildableCells.Add(0);
	Context.BuildableAges.Add(0);
	Context.BuildableTypes.Add(n"LumberCamp");
	Context.BuildableCells.Add(1);
	Context.BuildableAges.Add(0);
	Context.BuildableTypes.Add(n"MiningCamp");
	Context.BuildableCells.Add(2);
	Context.BuildableAges.Add(0);
	return Context;
}

FHOLResources CGRes(int Food, int Wood, int Stone, int Gold)
{
	FHOLResources Out;
	Out.Food = Food;
	Out.Wood = Wood;
	Out.Stone = Stone;
	Out.Gold = Gold;
	return Out;
}

bool CGIntsEqual(const TArray<int>& A, const TArray<int>& B)
{
	if (A.Num() != B.Num())
	{
		return false;
	}
	for (int I = 0; I < A.Num(); I++)
	{
		if (A[I] != B[I])
		{
			return false;
		}
	}
	return true;
}

TArray<int> CGInts3(int A, int B, int C)
{
	TArray<int> Out;
	Out.Add(A);
	Out.Add(B);
	Out.Add(C);
	return Out;
}

FHOLGroupMemberView CGMember(FName TypeName, bool bAlone)
{
	FHOLGroupMemberView Out;
	Out.TypeName = TypeName;
	Out.bAlone = bAlone;
	return Out;
}

FHOLCycleBuildingView CGCycle(int Rank, int Key)
{
	FHOLCycleBuildingView Out;
	Out.Rank = Rank;
	Out.Key = Key;
	return Out;
}

FHOLQueueChoiceView CGChoice(int Queue, int Max, int Key)
{
	FHOLQueueChoiceView Out;
	Out.QueueLength = Queue;
	Out.MaxQueue = Max;
	Out.Key = Key;
	return Out;
}

FHOLGridCommandView CGCmd(EHOLGridCommandKind Kind, FName TypeName)
{
	FHOLGridCommandView Out;
	Out.Command = Kind;
	Out.TypeName = TypeName;
	return Out;
}

// HeroesOfLiberty.CommandGrid.MarketGain
void Test_CGMarketGain(FUnitTest& T)
{
	TArray<EHOLResource> Goods;
	Goods.Add(EHOLResource::Food);
	Goods.Add(EHOLResource::Wood);
	Goods.Add(EHOLResource::Stone);
	for (int I = 0; I < Goods.Num(); I++)
	{
		EHOLResource Good = Goods[I];
		FHOLTradeGainView Buy = HOLCommandGridRules::GetTradeGain(Good, true, 96);
		T.AssertEquals(96, Buy.Amount, "buy: +96 received");
		T.AssertTrue(Buy.Logo == Good, "buy: the logo is the good's");
		FHOLTradeGainView Sell = HOLCommandGridRules::GetTradeGain(Good, false, 56);
		T.AssertEquals(56, Sell.Amount, "sell: +56 received");
		T.AssertTrue(Sell.Logo == EHOLResource::Gold, "sell: the logo is gold's");
	}
	FString BuyText = HOLCommandGridRules::GetTradeGainText(HOLCommandGridRules::GetTradeGain(EHOLResource::Wood, true, 96));
	FString SellText = HOLCommandGridRules::GetTradeGainText(HOLCommandGridRules::GetTradeGain(EHOLResource::Stone, false, 84));
	T.AssertTrue(BuyText == "+96", "the purchase's text: +96");
	T.AssertTrue(SellText == "+84", "the sale's text: +84");
	T.AssertFalse(BuyText.Contains("100") || BuyText.Contains("=") || BuyText.Contains("or"), "no 100 or = X (D781)");
	T.AssertFalse(SellText.Contains("100") || SellText.Contains("=") || SellText.Contains("or"), "no 100 = X or (D781)");
	T.AssertTrue(HOLCommandGridRules::GetTradeGainText(HOLCommandGridRules::GetTradeGain(EHOLResource::Food, true, 0)) == "+0", "nothing received: +0");
}

// HeroesOfLiberty.CommandGrid.Market
void Test_CGMarket(FUnitTest& T)
{
	FHOLGridContextView Market;
	Market.bOwnBuilding = true;
	Market.bBuildingFinished = true;
	Market.bMarket = true;
	TArray<EHOLResource> Goods;
	Goods.Add(EHOLResource::Food);
	Goods.Add(EHOLResource::Wood);
	Goods.Add(EHOLResource::Stone);
	for (int I = 0; I < 3; I++)
	{
		EHOLResource Good = Goods[I];
		T.AssertEquals(I, HOLCommandGridRules::GetTradeCell(Good, true), "buy cell (A Z E)");
		T.AssertEquals(4 + I, HOLCommandGridRules::GetTradeCell(Good, false), "sell cell (Q S D)");
		FHOLGridCommandView Buy = HOLCommandGridRules::GetCommand(Market, I);
		T.AssertTrue(Buy.Command == EHOLGridCommandKind::Trade && Buy.Resource == Good && Buy.bBuy, "Market, A Z E: buys the good");
		FHOLGridCommandView Sell = HOLCommandGridRules::GetCommand(Market, 4 + I);
		T.AssertTrue(Sell.Command == EHOLGridCommandKind::Trade && Sell.Resource == Good && !Sell.bBuy, "Market, Q S D: sells the good");
	}
	T.AssertEquals(-1, HOLCommandGridRules::GetTradeCell(EHOLResource::Gold, false), "gold has no sale cell");
	T.AssertEquals(-1, HOLCommandGridRules::GetTradeCell(EHOLResource::Gold, true), "gold has no purchase cell");
	TArray<int> Empty;
	Empty.Add(3);
	Empty.Add(7);
	Empty.Add(8);
	Empty.Add(9);
	Empty.Add(10);
	Empty.Add(11);
	for (int I = 0; I < Empty.Num(); I++)
	{
		T.AssertTrue(HOLCommandGridRules::GetCommand(Market, Empty[I]).Command == EHOLGridCommandKind::None, "Market: the other cells are empty");
	}
	T.AssertTrue(HOLCommandGridKeys::PressKey(Market, -1, 0).Target == EHOLGridKeyTargetKind::Grid, "a key on a Market goes to the grid");

	FHOLGridContextView Site = Market;
	Site.bBuildingFinished = false;
	T.AssertTrue(HOLCommandGridRules::GetCommand(Site, 0).Command == EHOLGridCommandKind::None && HOLCommandGridRules::GetCommand(Site, 4).Command == EHOLGridCommandKind::None
		&& HOLCommandGridRules::GetCommand(Site, HOLCommandGridRules::GetCancelCell()).Command == EHOLGridCommandKind::CancelConstruction, "a Market site: no trade, only its cancel cell");

	FHOLGridContextView House;
	House.bOwnBuilding = true;
	House.bBuildingFinished = true;
	T.AssertTrue(HOLCommandGridRules::GetCommand(House, 0).Command == EHOLGridCommandKind::None && HOLCommandGridRules::GetCommand(House, 4).Command == EHOLGridCommandKind::None, "another building: no trade");
}

// HeroesOfLiberty.CommandGrid.Commands
void Test_CGCommands(FUnitTest& T)
{
	FHOLGridContextView Builders = CGMakeBuilders();
	FHOLGridCommandView A = HOLCommandGridRules::GetAgeBuilding(Builders, 0, 0);
	T.AssertTrue(A.Command == EHOLGridCommandKind::Build && A.TypeName == n"House" && A.Age == 0 && !A.bGreyed, "I A: build a house");
	T.AssertTrue(HOLCommandGridRules::GetAgeBuilding(Builders, 0, 1).TypeName == n"LumberCamp", "I Z: build a lumber camp");
	T.AssertTrue(HOLCommandGridRules::GetAgeBuilding(Builders, 0, 2).TypeName == n"MiningCamp", "I E: build a mining camp");
	for (int Cell = 0; Cell < HOLCommandGridRules::GetNumCells(); Cell++)
	{
		T.AssertTrue(HOLCommandGridRules::GetCommand(Builders, Cell).Command == EHOLGridCommandKind::None, "the grid of builders is empty");
		bool bNone = HOLCommandGridRules::GetAgeBuilding(Builders, 0, Cell).Command == EHOLGridCommandKind::None;
		T.AssertTrue(bNone == (Cell > 2), "panel I: a building on A Z E only");
	}
	T.AssertTrue(HOLCommandGridRules::GetAgeBuilding(Builders, -1, 0).Command == EHOLGridCommandKind::None && HOLCommandGridRules::GetAgeBuilding(Builders, 4, 0).Command == EHOLGridCommandKind::None
		&& HOLCommandGridRules::GetAgeBuilding(Builders, 0, 12).Command == EHOLGridCommandKind::None, "out of the panels: nothing");

	FHOLGridContextView Soldiers = CGMakeBuilders();
	Soldiers.bBuilders = false;
	T.AssertTrue(HOLCommandGridRules::GetCommand(Soldiers, 0).Command == EHOLGridCommandKind::None && HOLCommandGridRules::GetAgeBuilding(Soldiers, 0, 0).Command == EHOLGridCommandKind::None, "no builder: A empty");

	FHOLGridContextView Centre;
	Centre.bOwnBuilding = true;
	Centre.bBuildingFinished = true;
	Centre.Produces.Add(n"Peasant");
	FHOLGridCommandView Produce = HOLCommandGridRules::GetCommand(Centre, 0);
	T.AssertTrue(Produce.Command == EHOLGridCommandKind::Produce && Produce.TypeName == n"Peasant", "centre: A produces a Peasant");
	T.AssertTrue(HOLCommandGridRules::GetCommand(Centre, 1).Command == EHOLGridCommandKind::None, "centre: Z empty");
	T.AssertTrue(HOLCommandGridRules::GetCommand(Centre, HOLCommandGridRules::GetCancelCell()).Command == EHOLGridCommandKind::None, "centre: no cancel cell");

	FHOLGridContextView Site = Centre;
	Site.bBuildingFinished = false;
	T.AssertTrue(HOLCommandGridRules::GetCommand(Site, 0).Command == EHOLGridCommandKind::None, "site: A produces nothing");
	T.AssertTrue(HOLCommandGridRules::GetCommand(Site, HOLCommandGridRules::GetCancelCell()).Command == EHOLGridCommandKind::CancelConstruction, "site: V cancels it");
	T.AssertEquals(11, HOLCommandGridRules::GetCancelCell(), "the cancel cell is V, bottom right");

	FHOLGridCellFaceView NoIcon = HOLCommandGridKeys::GetCellFace(false);
	T.AssertTrue(NoIcon.bName && !NoIcon.bIcon && !NoIcon.bCost, "no icon yet: the name, no cost");
	FHOLGridCellFaceView Icon = HOLCommandGridKeys::GetCellFace(true);
	T.AssertTrue(Icon.bIcon && !Icon.bName && !Icon.bCost, "an icon: the icon alone, no cost");
}

// HeroesOfLiberty.CommandGrid.Feedback
void Test_CGFeedback(FUnitTest& T)
{
	T.AssertFalse(HOLCommandGridKeys::HasMissingResource(CGRes(0, 50, 0, 0), CGRes(0, 50, 0, 0)), "covered: nothing missing");
	T.AssertTrue(HOLCommandGridKeys::HasMissingResource(CGRes(500, 49, 0, 0), CGRes(0, 50, 0, 0))
		&& HOLCommandGridKeys::FindMissingResource(CGRes(500, 49, 0, 0), CGRes(0, 50, 0, 0)) == int(EHOLResource::Wood), "49 wood for 50: wood missing");
	T.AssertTrue(HOLCommandGridKeys::HasMissingResource(CGRes(0, 0, 0, 0), CGRes(10, 0, 0, 10))
		&& HOLCommandGridKeys::FindMissingResource(CGRes(0, 0, 0, 0), CGRes(10, 0, 0, 10)) == int(EHOLResource::Food), "food and gold missing: food first");
	T.AssertTrue(HOLCommandGridKeys::HasMissingResource(CGRes(9, 9, 0, 9), CGRes(0, 0, 1, 0))
		&& HOLCommandGridKeys::FindMissingResource(CGRes(9, 9, 0, 9), CGRes(0, 0, 1, 0)) == int(EHOLResource::Stone), "stone missing");

	FHOLFeedbackView NoWood = HOLCommandGridKeys::CheckBuild(CGRes(200, 0, 200, 200), CGRes(0, 50, 0, 0));
	T.AssertTrue(NoWood.Kind == EHOLCommandFeedbackKind::NotEnoughResources && NoWood.Resource == EHOLResource::Wood, "house without wood: not enough wood");
	T.AssertTrue(HOLCommandGridKeys::Blocks(NoWood), "not enough: blocks");
	T.AssertTrue(HOLCommandGridKeys::CheckBuild(CGRes(0, 50, 0, 0), CGRes(0, 50, 0, 0)).Kind == EHOLCommandFeedbackKind::None, "house with 50 wood: goes through");
	T.AssertTrue(HOLCommandGridKeys::Describe(NoWood) == "not enough Wood", "logged");

	FHOLResources Peasant = CGRes(50, 0, 0, 0);
	T.AssertTrue(HOLCommandGridKeys::CheckProduction(CGRes(50, 0, 0, 0), Peasant, 0, 5, 3, 10, 1).Kind == EHOLCommandFeedbackKind::None, "room, resources: goes through");
	FHOLFeedbackView Full = HOLCommandGridKeys::CheckProduction(CGRes(0, 0, 0, 0), Peasant, 5, 5, 10, 10, 1);
	T.AssertTrue(Full.Kind == EHOLCommandFeedbackKind::QueueFull && HOLCommandGridKeys::Blocks(Full), "queue full before the cost");
	FHOLFeedbackView NoFood = HOLCommandGridKeys::CheckProduction(CGRes(49, 0, 0, 0), Peasant, 0, 5, 10, 10, 1);
	T.AssertTrue(NoFood.Kind == EHOLCommandFeedbackKind::NotEnoughResources && NoFood.Resource == EHOLResource::Food, "the cost before the population");
	FHOLFeedbackView Population = HOLCommandGridKeys::CheckProduction(CGRes(50, 0, 0, 0), Peasant, 0, 5, 10, 10, 1);
	T.AssertTrue(Population.Kind == EHOLCommandFeedbackKind::PopulationFull, "10 / 10: population full");
	T.AssertFalse(HOLCommandGridKeys::Blocks(Population), "population full: queued all the same (it waits)");
	T.AssertTrue(HOLCommandGridKeys::CheckProduction(CGRes(50, 0, 0, 0), Peasant, 0, 5, 9, 10, 1).Kind == EHOLCommandFeedbackKind::None, "9 / 10: room for one");
	T.AssertTrue(HOLCommandGridKeys::IsPopulationBlocked(9, 10, 2), "9 / 10: no room for two places");
	T.AssertFalse(HOLCommandGridKeys::IsPopulationBlocked(9, 10, 1), "9 / 10: room for one place");

	FHOLFeedbackView Ghost;
	Ghost.Kind = EHOLCommandFeedbackKind::PlacementRefused;
	Ghost.Refusal = HOLCommandGridRead::GetPlacementRefusalBuilding();
	T.AssertTrue(HOLCommandGridKeys::Blocks(Ghost), "red ghost: blocks");
	T.AssertTrue(HOLCommandGridKeys::Describe(Ghost) == "placement refused (on a building)", "red ghost logged");
}

// HeroesOfLiberty.CommandGrid.Cycle
void Test_CGCycle(FUnitTest& T)
{
	TArray<FHOLCycleBuildingView> Buildings;
	Buildings.Add(CGCycle(1, 40));  // a house
	Buildings.Add(CGCycle(0, 90));  // the centre, its key higher
	Buildings.Add(CGCycle(2, 10));  // a lumber camp
	Buildings.Add(CGCycle(1, 20));  // another house
	T.AssertEquals(1, HOLCommandGridGroups::NextInCycle(Buildings, 0), "first press: the centre");
	T.AssertEquals(3, HOLCommandGridGroups::NextInCycle(Buildings, 90), "then the house of the lowest key");
	T.AssertEquals(0, HOLCommandGridGroups::NextInCycle(Buildings, 20), "then the other house");
	T.AssertEquals(2, HOLCommandGridGroups::NextInCycle(Buildings, 40), "then the lumber camp");
	T.AssertEquals(1, HOLCommandGridGroups::NextInCycle(Buildings, 10), "after the last one: the centre again");
	T.AssertEquals(1, HOLCommandGridGroups::NextInCycle(Buildings, 55), "a key gone (building cancelled): the first one");
	TArray<FHOLCycleBuildingView> None;
	T.AssertEquals(-1, HOLCommandGridGroups::NextInCycle(None, 0), "no building: none");
	TArray<FHOLCycleBuildingView> One;
	One.Add(CGCycle(0, 7));
	T.AssertEquals(0, HOLCommandGridGroups::NextInCycle(One, 7), "one building: always it");

	bool bOk1 = HOLCommandGridRead::IsTypeInTable(n"MainCentre") && HOLCommandGridRead::IsTypeInTable(n"House") && HOLCommandGridRead::IsTypeInTable(n"MiningCamp");
	T.AssertTrue(bOk1, "centre, house, mining camp are in the table");
	if (bOk1)
	{
		// The rank comes from the script rule (HOLCommandGridGroups::GetCycleRank) fed by two table reads: no slot + 1 copy in C++.
		int Centre = HOLCommandGridGroups::GetCycleRank(HOLCommandGridRead::IsTypeBuildable(n"MainCentre"), HOLCommandGridRead::GetBuildGridSlotOfType(n"MainCentre"));
		int House = HOLCommandGridGroups::GetCycleRank(HOLCommandGridRead::IsTypeBuildable(n"House"), HOLCommandGridRead::GetBuildGridSlotOfType(n"House"));
		int Camp = HOLCommandGridGroups::GetCycleRank(HOLCommandGridRead::IsTypeBuildable(n"MiningCamp"), HOLCommandGridRead::GetBuildGridSlotOfType(n"MiningCamp"));
		T.AssertTrue(Centre < House && House < Camp, "ranks: centre < house < mining camp");
	}
}

// HeroesOfLiberty.CommandGrid.Categories
void Test_CGCategories(FUnitTest& T)
{
	TArray<FName> Types;
	Types.Add(n"MainCentre");
	Types.Add(n"House");
	Types.Add(n"LumberCamp");
	Types.Add(n"MiningCamp");
	Types.Add(n"Mill");
	for (int I = 0; I < Types.Num(); I++)
	{
		FName TypeName = Types[I];
		bool bOk2 = HOLCommandGridRead::IsTypeInTable(TypeName);
		T.AssertTrue(bOk2, "in the table");
		if (bOk2)
		{
			T.AssertTrue(HOLCommandGridRead::GetTypeCategory(TypeName) == EHOLBuildingCategory::Economic, "economic");
			T.AssertTrue(HOLCommandGridGroups::IsCategorySelectedByCategory(HOLCommandGridRead::GetTypeCategory(TypeName), EHOLBuildingCategory::Economic), "F2 takes it");
			T.AssertFalse(HOLCommandGridGroups::IsCategorySelectedByCategory(HOLCommandGridRead::GetTypeCategory(TypeName), EHOLBuildingCategory::Military), "F1 does not");
			T.AssertFalse(HOLCommandGridGroups::IsCategorySelectedByCategory(HOLCommandGridRead::GetTypeCategory(TypeName), EHOLBuildingCategory::Technological), "F3 does not");
		}
	}
	T.AssertTrue(HOLCommandGridGroups::IsCategorySelectedByCategory(EHOLBuildingCategory::Military, EHOLBuildingCategory::Military), "a military building, site or ghost: F1 takes it");
	T.AssertFalse(HOLCommandGridGroups::IsCategorySelectedByCategory(EHOLBuildingCategory::Military, EHOLBuildingCategory::Economic), "a military building: F2 leaves it");

	TArray<int> Selected = CGInts3(7, 3, 9);
	T.AssertTrue(HOLCommandGridGroups::IsRepeatPress(CGInts3(3, 9, 7), Selected, true), "same key, same buildings: the next one");
	T.AssertFalse(HOLCommandGridGroups::IsRepeatPress(CGInts3(3, 9, 7), Selected, false), "another key: select again");
	TArray<int> OneLeft;
	OneLeft.Add(3);
	T.AssertFalse(HOLCommandGridGroups::IsRepeatPress(OneLeft, Selected, true), "one building left the selection: select again");
	T.AssertFalse(HOLCommandGridGroups::IsRepeatPress(CGInts3(3, 9, 8), Selected, true), "another selection of as many: select again");
	TArray<int> Nothing;
	T.AssertFalse(HOLCommandGridGroups::IsRepeatPress(Nothing, Nothing, true), "nothing selected: select");
	TArray<int> Four;
	Four.Add(4);
	T.AssertTrue(HOLCommandGridGroups::IsRepeatPress(Four, Four, true), "one building alone: the camera on it");

	FHOLFeedbackView NoBuilding;
	NoBuilding.Kind = EHOLCommandFeedbackKind::NoBuilding;
	NoBuilding.Category = EHOLBuildingCategory::Military;
	T.AssertTrue(HOLCommandGridKeys::Describe(NoBuilding) == "no Military building", "no military building logged");
	T.AssertFalse(HOLCommandGridKeys::Blocks(NoBuilding), "told, nothing to block");
}

// HeroesOfLiberty.CommandGrid.Group
void Test_CGGroup(FUnitTest& T)
{
	FHOLGridContextView Centres;
	Centres.bOwnBuilding = true;
	Centres.bBuildingGroup = true;
	Centres.bBuildingFinished = true;
	Centres.Produces.Add(n"Peasant");
	T.AssertTrue(HOLCommandGridRules::GetCommand(Centres, 0).Command == EHOLGridCommandKind::Produce, "a centre among others: A produces a Peasant");
	FHOLGridContextView WithSite = Centres;
	WithSite.bBuildingFinished = false;
	WithSite.Produces.Reset();
	for (int Cell = 0; Cell < HOLCommandGridRules::GetNumCells(); Cell++)
	{
		T.AssertTrue(HOLCommandGridRules::GetCommand(WithSite, Cell).Command == EHOLGridCommandKind::None, "a site among others: every cell empty");
	}
	FHOLGridContextView OneSite = WithSite;
	OneSite.bBuildingGroup = false;
	T.AssertTrue(HOLCommandGridRules::GetCommand(OneSite, HOLCommandGridRules::GetCancelCell()).Command == EHOLGridCommandKind::CancelConstruction, "a subgroup of one site: V cancels it");
}

// HeroesOfLiberty.CommandGrid.Subgroups
void Test_CGSubgroups(FUnitTest& T)
{
	FName House = n"House";
	FName Camp = n"LumberCamp";
	FName Centre = n"MainCentre";
	T.AssertTrue(HOLCommandGridGroups::IsAloneInGroup(Centre) && !HOLCommandGridGroups::IsAloneInGroup(House), "the main centre is alone in a group");
	TArray<FHOLGroupMemberView> Members;
	Members.Add(CGMember(Centre, true));  // 0
	Members.Add(CGMember(House, false));  // 1
	Members.Add(CGMember(Camp, false));   // 2
	Members.Add(CGMember(House, false));  // 3
	Members.Add(CGMember(Centre, true));  // 4: a second forum, distinct
	Members.Add(CGMember(House, false));  // 5
	TArray<FHOLSubgroupView> Subgroups = HOLCommandGridGroups::SplitSubgroups(Members);
	bool bOk3 = (5 == Subgroups.Num());
	T.AssertEquals(5, Subgroups.Num(), "All, centre, houses, camp, other centre");
	if (bOk3)
	{
		TArray<int> AllMembers;
		for (int I = 0; I < 6; I++)
		{
			AllMembers.Add(I);
		}
		T.AssertTrue(Subgroups[0].bAll && CGIntsEqual(Subgroups[0].Members, AllMembers), "first: All, every building");
		TArray<int> First;
		First.Add(0);
		T.AssertTrue(!Subgroups[1].bAll && Subgroups[1].TypeName == Centre && CGIntsEqual(Subgroups[1].Members, First), "then the first centre alone");
		T.AssertTrue(Subgroups[2].TypeName == House && CGIntsEqual(Subgroups[2].Members, CGInts3(1, 3, 5)), "then the houses, all three");
		TArray<int> CampMembers;
		CampMembers.Add(2);
		T.AssertTrue(Subgroups[3].TypeName == Camp && CGIntsEqual(Subgroups[3].Members, CampMembers), "then the camp");
		TArray<int> Second;
		Second.Add(4);
		T.AssertTrue(Subgroups[4].TypeName == Centre && CGIntsEqual(Subgroups[4].Members, Second), "the second centre, a subgroup of its own");
	}
	TArray<FHOLGroupMemberView> TwoHouses;
	TwoHouses.Add(CGMember(House, false));
	TwoHouses.Add(CGMember(House, false));
	TArray<FHOLSubgroupView> Houses = HOLCommandGridGroups::SplitSubgroups(TwoHouses);
	bool bOk4 = (2 == Houses.Num());
	T.AssertEquals(2, Houses.Num(), "houses alone: All, then the houses");
	if (bOk4)
	{
		T.AssertTrue(Houses[0].bAll && Houses[1].Members.Num() == 2, "houses alone: All then the two houses");
	}
	TArray<FHOLGroupMemberView> NoGroup;
	T.AssertEquals(0, HOLCommandGridGroups::SplitSubgroups(NoGroup).Num(), "no group: no subgroup");

	FName Technology = n"Technology";
	FName Militia = n"Militia";
	TArray<FHOLGroupMemberView> Mixed;
	Mixed.Add(CGMember(n"Mill", false));
	Mixed.Add(CGMember(Centre, true));
	Mixed.Add(CGMember(n"Barracks", false));
	T.AssertTrue(CGIntsEqual(HOLCommandGridGroups::GetCommandPriority(Mixed), CGInts3(1, 0, 2)), "priority: the centre, then the group's order");
	int Cells = HOLCommandGridRules::GetNumCells();
	TArray<FHOLGridCommandView> PerBuilding;
	for (int I = 0; I < 3 * Cells; I++)
	{
		PerBuilding.Add(FHOLGridCommandView());
	}
	PerBuilding[0] = CGCmd(EHOLGridCommandKind::Produce, Technology);          // the mill: A and Z
	PerBuilding[1] = CGCmd(EHOLGridCommandKind::Produce, Technology);
	PerBuilding[Cells + 0] = CGCmd(EHOLGridCommandKind::Produce, n"Peasant");  // the forum: A
	PerBuilding[2 * Cells + 0] = CGCmd(EHOLGridCommandKind::Produce, Militia); // the barracks: A and E
	PerBuilding[2 * Cells + 2] = CGCmd(EHOLGridCommandKind::Produce, Militia);
	TArray<FHOLGridCommandView> Merged = HOLCommandGridGroups::MergeCommands(PerBuilding, HOLCommandGridGroups::GetCommandPriority(Mixed));
	bool bOk5 = (Cells == Merged.Num());
	T.AssertEquals(Cells, Merged.Num(), "twelve cells");
	if (bOk5)
	{
		T.AssertTrue(Merged[0].TypeName == n"Peasant", "A: the forum's Peasant");
		T.AssertTrue(Merged[1].TypeName == Technology, "Z: the mill's (no other on Z)");
		T.AssertTrue(Merged[2].TypeName == Militia, "E: the barracks' (no other on E)");
		T.AssertTrue(Merged[3].Command == EHOLGridCommandKind::None, "R: empty");
	}
	TArray<FHOLGroupMemberView> NoCentre;
	NoCentre.Add(CGMember(n"Mill", false));
	NoCentre.Add(CGMember(n"Forge", false));
	NoCentre.Add(CGMember(n"Barracks", false));
	TArray<FHOLGridCommandView> MergedNoCentre = HOLCommandGridGroups::MergeCommands(PerBuilding, HOLCommandGridGroups::GetCommandPriority(NoCentre));
	T.AssertTrue(MergedNoCentre.Num() > 0 && MergedNoCentre[0].TypeName == Technology, "no centre: A to the first in the group's order");
}

// HeroesOfLiberty.CommandGrid.Distribution
void Test_CGDistribution(FUnitTest& T)
{
	TArray<FHOLQueueChoiceView> A;
	A.Add(CGChoice(3, 5, 1));
	A.Add(CGChoice(1, 5, 2));
	A.Add(CGChoice(2, 5, 3));
	T.AssertEquals(1, HOLCommandGridGroups::PickEvenly(A, 0), "the shortest queue");
	TArray<FHOLQueueChoiceView> B;
	B.Add(CGChoice(2, 5, 9));
	B.Add(CGChoice(2, 5, 4));
	T.AssertEquals(0, HOLCommandGridGroups::PickEvenly(B, 0), "equal queues, none picked yet: the first in the group's order");
	TArray<FHOLQueueChoiceView> C;
	C.Add(CGChoice(1, 1, 1));
	C.Add(CGChoice(4, 5, 2));
	T.AssertEquals(1, HOLCommandGridGroups::PickEvenly(C, 0), "a full queue is passed");
	TArray<FHOLQueueChoiceView> D;
	D.Add(CGChoice(5, 5, 1));
	D.Add(CGChoice(5, 5, 2));
	T.AssertEquals(-1, HOLCommandGridGroups::PickEvenly(D, 0), "every queue full: none");
	TArray<FHOLQueueChoiceView> E;
	E.Add(CGChoice(0, 5, 7));
	T.AssertEquals(0, HOLCommandGridGroups::PickEvenly(E, 7), "one building: it, again and again");
	TArray<FHOLQueueChoiceView> F;
	F.Add(CGChoice(1, 5, 3));
	F.Add(CGChoice(1, 5, 4));
	T.AssertEquals(0, HOLCommandGridGroups::PickEvenly(F, 99), "the last one picked gone: the first of the fewest");

	// 4 presses on 2 centres: 2 + 2, whether their queues reach this machine between the presses or not.
	for (int Pass = 0; Pass < 2; Pass++)
	{
		bool bQueuesSeen = Pass == 1;
		TArray<FHOLQueueChoiceView> Centres;
		Centres.Add(CGChoice(0, 5, 10));
		Centres.Add(CGChoice(0, 5, 20));
		int Last = 0;
		int Given0 = 0;
		int Given1 = 0;
		for (int Press = 0; Press < 4; Press++)
		{
			int Index = HOLCommandGridGroups::PickEvenly(Centres, Last);
			bool bOk6 = Index != -1;
			T.AssertTrue(bOk6, "a centre takes it");
			if (!bOk6)
			{
				break;
			}
			if (Index == 0)
			{
				Given0++;
			}
			else
			{
				Given1++;
			}
			Last = Centres[Index].Key;
			if (bQueuesSeen)
			{
				Centres[Index].QueueLength = Centres[Index].QueueLength + 1;
			}
		}
		T.AssertTrue(Given0 == 2 && Given1 == 2, "2 + 2 (queues seen or not)");
	}

	// Three barracks, one already busy: the two others first, in turn, then all three in turn.
	TArray<FHOLQueueChoiceView> Barracks;
	Barracks.Add(CGChoice(2, 5, 1));
	Barracks.Add(CGChoice(0, 5, 2));
	Barracks.Add(CGChoice(0, 5, 3));
	int Last = 0;
	TArray<int> Order;
	for (int Press = 0; Press < 7; Press++)
	{
		int Index = HOLCommandGridGroups::PickEvenly(Barracks, Last);
		Order.Add(Index);
		Last = Barracks[Index].Key;
		Barracks[Index].QueueLength = Barracks[Index].QueueLength + 1;
	}
	TArray<int> Expected;
	Expected.Add(1);
	Expected.Add(2);
	Expected.Add(1);
	Expected.Add(2);
	Expected.Add(0);
	Expected.Add(1);
	Expected.Add(2);
	T.AssertTrue(CGIntsEqual(Order, Expected), "1 2 1 2, then 0 1 2");
}

// HeroesOfLiberty.CommandGrid.Tables (first half: the builders' grid of the table, the Barracks and Archery range grids; the later grids are not ported yet)
void Test_CGTables(FUnitTest& T)
{
	FHOLGridContextView Context;
	Context.bBuilders = true;
	Context = HOLCommandGridRead::WithTableBuildables(Context);
	T.AssertEquals(13, Context.BuildableTypes.Num(), "thirteen buildable types");
	T.AssertTrue(HOLCommandGridRules::GetAgeBuilding(Context, 0, 0).TypeName == n"House", "A: house");
	T.AssertTrue(HOLCommandGridRules::GetAgeBuilding(Context, 0, 1).TypeName == n"LumberCamp", "Z: lumber camp");
	T.AssertTrue(HOLCommandGridRules::GetAgeBuilding(Context, 0, 2).TypeName == n"MiningCamp", "E: mining camp");
	T.AssertTrue(HOLCommandGridRules::GetAgeBuilding(Context, 0, 3).TypeName == n"Mill", "R: mill (D346)");
	T.AssertTrue(HOLCommandGridRules::GetAgeBuilding(Context, 0, 4).TypeName == n"Barracks", "Q: barracks");
	T.AssertTrue(HOLCommandGridRules::GetAgeBuilding(Context, 0, 4).Command == EHOLGridCommandKind::Build, "Q: a build command");
	T.AssertTrue(HOLCommandGridRules::GetAgeBuilding(Context, 0, 5).TypeName == n"ArcheryRange", "S: archery range (AR-Q2a)");
	int FarmCell = 8;
	T.AssertEquals(2, FarmCell / HOLCommandGridRules::GetGridColumns(), "W: row 3");
	T.AssertEquals(0, FarmCell % HOLCommandGridRules::GetGridColumns(), "W: column 1");
	T.AssertTrue(HOLCommandGridRules::GetAgeBuilding(Context, 0, FarmCell).TypeName == n"Farm", "W: farm (IA1.2-Q1)");
	T.AssertTrue(HOLCommandGridRules::GetAgeBuilding(Context, 0, FarmCell).Command == EHOLGridCommandKind::Build && !HOLCommandGridRules::GetAgeBuilding(Context, 0, FarmCell).bGreyed, "W: a build command, age I reached");
	T.AssertTrue(HOLCommandGridKeys::PressKey(Context, 0, FarmCell).Target == EHOLGridKeyTargetKind::Build && HOLCommandGridKeys::PressKey(Context, 0, FarmCell).TypeName == n"Farm", "W with age I chosen: builds the farm");
	FHOLGridCommandView MarketCell = HOLCommandGridRules::GetAgeBuilding(Context, 1, 0);
	T.AssertTrue(MarketCell.TypeName == n"Market" && MarketCell.Command == EHOLGridCommandKind::Build && MarketCell.Age == 1 && MarketCell.bGreyed, "II A: the Market, greyed (age II not reached)");
	FHOLGridCommandView WorkshopCell = HOLCommandGridRules::GetAgeBuilding(Context, 1, 4);
	T.AssertTrue(WorkshopCell.TypeName == n"SiegeWorkshop" && WorkshopCell.Command == EHOLGridCommandKind::Build && WorkshopCell.Age == 1 && WorkshopCell.bGreyed, "II Q: the siege workshop, greyed");
	FHOLGridCommandView ChapelCell = HOLCommandGridRules::GetAgeBuilding(Context, 1, 5);
	T.AssertTrue(ChapelCell.TypeName == n"Chapel" && ChapelCell.Command == EHOLGridCommandKind::Build && ChapelCell.Age == 1 && ChapelCell.bGreyed, "II S: the Chapel, greyed");
	FHOLGridCommandView StableCell = HOLCommandGridRules::GetAgeBuilding(Context, 1, 6);
	T.AssertTrue(StableCell.TypeName == n"Stable" && StableCell.Command == EHOLGridCommandKind::Build && StableCell.Age == 1 && StableCell.bGreyed, "II D: the Stable, greyed");
	FHOLGridCommandView ForgeCell = HOLCommandGridRules::GetAgeBuilding(Context, 1, 8);
	T.AssertTrue(ForgeCell.TypeName == n"Forge" && ForgeCell.Command == EHOLGridCommandKind::Build && ForgeCell.Age == 1 && ForgeCell.bGreyed, "II W: the Forge, greyed");
	TArray<int> EmptyII;
	EmptyII.Add(1);
	EmptyII.Add(2);
	EmptyII.Add(3);
	EmptyII.Add(7);
	EmptyII.Add(9);
	EmptyII.Add(10);
	EmptyII.Add(11);
	for (int I = 0; I < EmptyII.Num(); I++)
	{
		T.AssertTrue(HOLCommandGridRules::GetAgeBuilding(Context, 1, EmptyII[I]).Command == EHOLGridCommandKind::None, "page II: no building yet on the other cells");
	}
	TArray<int> EmptyI;
	EmptyI.Add(6);
	EmptyI.Add(7);
	EmptyI.Add(10);
	EmptyI.Add(11);
	for (int I = 0; I < EmptyI.Num(); I++)
	{
		T.AssertTrue(HOLCommandGridRules::GetAgeBuilding(Context, 0, EmptyI[I]).Command == EHOLGridCommandKind::None, "panel I: no building on the other cells");
	}
	T.AssertEquals(0, HOLCommandGridRules::GetCategoryRow(EHOLBuildingCategory::Economic), "economic: row 1");
	T.AssertEquals(1, HOLCommandGridRules::GetCategoryRow(EHOLBuildingCategory::Military), "military: row 2");
	T.AssertEquals(2, HOLCommandGridRules::GetCategoryRow(EHOLBuildingCategory::Technological), "technology: row 3");
	for (int Cell = 8; Cell < HOLCommandGridRules::GetNumCells(); Cell++)
	{
		T.AssertTrue(HOLCommandGridRules::GetCommand(Context, Cell).Command == EHOLGridCommandKind::None, "the builders' grid on the right is empty");
	}
	for (int I = 0; I < Context.BuildableTypes.Num(); I++)
	{
		T.AssertTrue(Context.BuildableTypes[I] != n"MainCentre", "the centre is not built");
	}

	// Barracks (J7a.2): A Spearman, Z Halberdier, E Man-at-arms, R Damned Warrior once finished (the table order: Spearman, Halberdier, Man-at-arms, Damned Warrior; D858); a site produces nothing, its cancel cell cancels it.
	FHOLGridContextView Barracks;
	Barracks.bOwnBuilding = true;
	Barracks.bBuildingFinished = true;
	Barracks.Produces = HOLCommandGridRead::GetProducesOf(n"Barracks");
	T.AssertTrue(HOLCommandGridRules::GetCommand(Barracks, 0).Command == EHOLGridCommandKind::Produce && HOLCommandGridRules::GetCommand(Barracks, 0).TypeName == n"Spearman", "Barracks, A: produce a Spearman");
	T.AssertTrue(HOLCommandGridRules::GetCommand(Barracks, 1).Command == EHOLGridCommandKind::Produce && HOLCommandGridRules::GetCommand(Barracks, 1).TypeName == n"Halberdier", "Barracks, Z: produce a Halberdier (D858)");
	T.AssertTrue(HOLCommandGridRules::GetCommand(Barracks, 2).Command == EHOLGridCommandKind::Produce && HOLCommandGridRules::GetCommand(Barracks, 2).TypeName == n"ManAtArms", "Barracks, E: produce a Man-at-arms");
	T.AssertTrue(HOLCommandGridRules::GetCommand(Barracks, 3).Command == EHOLGridCommandKind::Produce && HOLCommandGridRules::GetCommand(Barracks, 3).TypeName == n"DamnedWarrior", "Barracks, R: produce a Damned Warrior (L4)");
	T.AssertTrue(HOLCommandGridRules::GetCommand(Barracks, 4).Command == EHOLGridCommandKind::Produce && HOLCommandGridRules::GetCommand(Barracks, 4).TypeName == n"VirtuousKnight", "Barracks, T: produce a Virtuous Knight (D864; the Dawn's grid is then A Halberdier, Z Man-at-arms, E Virtuous Knight through FilterProduces)");
	for (int Cell = 5; Cell < HOLCommandGridRules::GetNumCells(); Cell++)
	{
		T.AssertTrue(HOLCommandGridRules::GetCommand(Barracks, Cell).Command == EHOLGridCommandKind::None, "Barracks: the other cells are empty");
	}
	Barracks.bBuildingFinished = false;
	for (int Cell = 0; Cell < HOLCommandGridRules::GetNumCells(); Cell++)
	{
		T.AssertTrue(HOLCommandGridRules::GetCommand(Barracks, Cell).Command != EHOLGridCommandKind::Produce, "Barracks site: no production");
	}
	T.AssertTrue(HOLCommandGridRules::GetCommand(Barracks, HOLCommandGridRules::GetCancelCell()).Command == EHOLGridCommandKind::CancelConstruction, "Barracks site: the cancel cell cancels it");

	// Archery range (AR-Q2a): A Archer, Z Crossbowman, E Arquebusier; no Archer left at the Barracks.
	FHOLGridContextView Range;
	Range.bOwnBuilding = true;
	Range.bBuildingFinished = true;
	Range.Produces = HOLCommandGridRead::GetProducesOf(n"ArcheryRange");
	T.AssertTrue(HOLCommandGridRules::GetCommand(Range, 0).TypeName == n"Archer" && HOLCommandGridRules::GetCommand(Range, 1).TypeName == n"Crossbowman" && HOLCommandGridRules::GetCommand(Range, 2).TypeName == n"Arquebusier", "Archery range: A Archer, Z Crossbowman, E Arquebusier");
	for (int Cell = 3; Cell < HOLCommandGridRules::GetNumCells(); Cell++)
	{
		T.AssertTrue(HOLCommandGridRules::GetCommand(Range, Cell).Command == EHOLGridCommandKind::None, "Archery range: the other cells are empty");
	}
	T.AssertFalse(HOLCommandGridRead::GetProducesOf(n"Barracks").Contains(n"Archer"), "no Archer at the Barracks any more (AR-Q2)");

	// Siege workshop (S2): A Ram, Z Mangonel once finished; nothing else.
	FHOLGridContextView Workshop;
	Workshop.bOwnBuilding = true;
	Workshop.bBuildingFinished = true;
	Workshop.Produces = HOLCommandGridRead::GetProducesOf(n"SiegeWorkshop");
	T.AssertTrue(HOLCommandGridRules::GetCommand(Workshop, 0).Command == EHOLGridCommandKind::Produce && HOLCommandGridRules::GetCommand(Workshop, 0).TypeName == n"Ram", "Siege workshop, A: produce a Ram");
	T.AssertTrue(HOLCommandGridRules::GetCommand(Workshop, 1).Command == EHOLGridCommandKind::Produce && HOLCommandGridRules::GetCommand(Workshop, 1).TypeName == n"Mangonel", "Siege workshop, Z: produce a Mangonel");
	for (int Cell = 2; Cell < HOLCommandGridRules::GetNumCells(); Cell++)
	{
		T.AssertTrue(HOLCommandGridRules::GetCommand(Workshop, Cell).Command == EHOLGridCommandKind::None,"Siege workshop: the other cells are empty");
	}

	// Chapel (Q-chapelle-moine = A): A Monk once finished; nothing else.
	FHOLGridContextView Chapel;
	Chapel.bOwnBuilding = true;
	Chapel.bBuildingFinished = true;
	Chapel.Produces = HOLCommandGridRead::GetProducesOf(n"Chapel");
	T.AssertTrue(HOLCommandGridRules::GetCommand(Chapel, 0).Command == EHOLGridCommandKind::Produce && HOLCommandGridRules::GetCommand(Chapel, 0).TypeName == n"Monk", "Chapel, A: produce a Monk");
	for (int Cell = 1; Cell < HOLCommandGridRules::GetNumCells(); Cell++)
	{
		T.AssertTrue(HOLCommandGridRules::GetCommand(Chapel, Cell).Command == EHOLGridCommandKind::None, "Chapel: the other cells are empty");
	}
}

// Helpers of the 2nd lot (UnitOrders, HeroAbilities, Ages, MouseCursor, Research).
bool CGNamesEqual(const TArray<FName>& A, const TArray<FName>& B)
{
	if (A.Num() != B.Num())
	{
		return false;
	}
	for (int I = 0; I < A.Num(); I++)
	{
		if (A[I] != B[I])
		{
			return false;
		}
	}
	return true;
}

// FHOLGridKeyResult == Key{ Target, ChosenAge } (no type name).
bool CGKeyIs(const FHOLGridKeyResultView& Result, EHOLGridKeyTargetKind Target, int ChosenAge)
{
	FName NoName;
	return Result.Target == Target && Result.ChosenAge == ChosenAge && Result.TypeName == NoName;
}

// FHOLGridKeyResult == Key{ Build, ChosenAge, TypeName }.
bool CGKeyBuild(const FHOLGridKeyResultView& Result, int ChosenAge, FName TypeName)
{
	return Result.Target == EHOLGridKeyTargetKind::Build && Result.ChosenAge == ChosenAge && Result.TypeName == TypeName;
}

// HeroesOfLiberty.CommandGrid.UnitOrders
void Test_CGUnitOrders(FUnitTest& T)
{
	// The orders' panel left of the grid: Q attack-move (soldiers among the selection), S stop, V hold (any own units).
	T.AssertEquals(4, HOLCommandGridRules::GetAttackMoveCell(), "Q is cell 4");
	T.AssertEquals(5, HOLCommandGridRules::GetStopCell(), "S is cell 5");
	T.AssertEquals(11, HOLCommandGridRules::GetHoldCell(), "V is cell 11");
	int QCell = HOLCommandGridRules::GetAttackMoveCell();
	int SCell = HOLCommandGridRules::GetStopCell();
	int VCell = HOLCommandGridRules::GetHoldCell();

	// Soldiers alone: Q S V on the left, nothing else; nothing on the right (they do not build).
	FHOLGridContextView Soldiers;
	Soldiers.bOwnUnits = true;
	Soldiers.bSoldiers = true;
	T.AssertTrue(HOLCommandGridRules::GetOrder(Soldiers, QCell).Command == EHOLGridCommandKind::AttackMove, "soldiers: Q attack-move");
	T.AssertTrue(HOLCommandGridRules::GetOrder(Soldiers, SCell).Command == EHOLGridCommandKind::Stop, "soldiers: S stop");
	T.AssertTrue(HOLCommandGridRules::GetOrder(Soldiers, VCell).Command == EHOLGridCommandKind::Hold, "soldiers: V hold");
	for (int Cell = 0; Cell < HOLCommandGridRules::GetNumCells(); Cell++)
	{
		if (Cell != QCell && Cell != SCell && Cell != VCell)
		{
			T.AssertTrue(HOLCommandGridRules::GetOrder(Soldiers, Cell).Command == EHOLGridCommandKind::None, "soldiers: the other orders are empty");
		}
		T.AssertTrue(HOLCommandGridRules::GetCommand(Soldiers, Cell).Command == EHOLGridCommandKind::None, "soldiers: the grid cell is empty");
	}
	T.AssertTrue(HOLCommandGridRules::GetOrder(Soldiers, 6).Command == EHOLGridCommandKind::None, "D is no order any more");

	// Peasants alone (builders, no soldier): no attack-move, S and V still there; the builders' grid on the right.
	FHOLGridContextView Peasants;
	Peasants.bBuilders = true;
	Peasants.bOwnUnits = true;
	Peasants = HOLCommandGridRead::WithTableBuildables(Peasants);
	T.AssertTrue(HOLCommandGridRules::IsBuildGrid(Peasants), "Peasants: the builders' grid");
	for (int Cell = 0; Cell < HOLCommandGridRules::GetNumCells(); Cell++)
	{
		T.AssertTrue(HOLCommandGridRules::GetOrder(Peasants, Cell).Command != EHOLGridCommandKind::AttackMove, "Peasants alone: no attack-move");
	}
	T.AssertTrue(HOLCommandGridRules::GetOrder(Peasants, SCell).Command == EHOLGridCommandKind::Stop, "Peasants: S stop");
	T.AssertTrue(HOLCommandGridRules::GetOrder(Peasants, VCell).Command == EHOLGridCommandKind::Hold, "Peasants: V hold");
	T.AssertTrue(HOLCommandGridRules::GetAgeBuilding(Peasants, 0, 0).TypeName == n"House", "Peasants: A of panel I the house, on the right");

	// Peasants and soldiers together: the orders, Q included; the builders' grid hidden.
	FHOLGridContextView Mixed = Peasants;
	Mixed.bSoldiers = true;
	T.AssertFalse(HOLCommandGridRules::IsBuildGrid(Mixed), "mixed: no builders' grid");
	T.AssertTrue(HOLCommandGridRules::GetOrder(Mixed, QCell).Command == EHOLGridCommandKind::AttackMove, "mixed: Q attack-move");
	for (int Cell = 0; Cell < HOLCommandGridRules::GetNumCells(); Cell++)
	{
		T.AssertTrue(HOLCommandGridRules::GetCommand(Mixed, Cell).Command == EHOLGridCommandKind::None, "mixed: the cell is empty on the right");
	}

	// An enemy selection (no own unit): no order.
	FHOLGridContextView Enemy;
	for (int Cell = 0; Cell < HOLCommandGridRules::GetNumCells(); Cell++)
	{
		T.AssertTrue(HOLCommandGridRules::GetOrder(Enemy, Cell).Command == EHOLGridCommandKind::None, "enemy selection: the order is empty");
	}

	// A building of the player's (the Barracks, finished or a site): its grid only, no orders.
	FHOLGridContextView Barracks;
	Barracks.bOwnBuilding = true;
	Barracks.Produces.Add(n"Spearman");
	Barracks.Produces.Add(n"ManAtArms");
	Barracks.bOwnUnits = true;  // never set with a building; the building wins all the same
	Barracks.bSoldiers = true;
	for (int Pass = 0; Pass < 2; Pass++)
	{
		bool bFinished = Pass == 0;
		Barracks.bBuildingFinished = bFinished;
		for (int Cell = 0; Cell < HOLCommandGridRules::GetNumCells(); Cell++)
		{
			T.AssertTrue(HOLCommandGridRules::GetOrder(Barracks, Cell).Command == EHOLGridCommandKind::None, "Barracks: no order");
		}
		EHOLGridCommandKind Expected = bFinished ? EHOLGridCommandKind::Produce : EHOLGridCommandKind::None;
		T.AssertTrue(HOLCommandGridRules::GetCommand(Barracks, 0).Command == Expected, "Barracks: A its first unit or nothing");
		T.AssertTrue(HOLCommandGridKeys::PressKey(Barracks, -1, 4).Target == EHOLGridKeyTargetKind::Grid, "Barracks: keys to the grid");
	}
}

// HeroesOfLiberty.CommandGrid.HeroAbilities
void Test_CGHeroAbilities(FUnitTest& T)
{
	// J8c.4: the hero among the units commanded, his abilities on the grid on the right, A the cry, Z the banner; the passive aura has no cell.
	int Aura = HOLCommandGridRead::GetHeroAbilityAura();
	int CloseRanks = HOLCommandGridRead::GetHeroAbilityCloseRanks();
	int Banner = HOLCommandGridRead::GetHeroAbilityBanner();
	T.AssertEquals(0, HOLCommandGridRules::GetHeroAbilityCell(CloseRanks), "A: Serrez les rangs");
	T.AssertEquals(1, HOLCommandGridRules::GetHeroAbilityCell(Banner), "Z: Banniere");
	T.AssertEquals(-1, HOLCommandGridRules::GetHeroAbilityCell(Aura), "the aura: no cell");

	// The hero alone at level 1: A the cry, ready; Z the banner, greyed (level 4); nothing else; the orders as soldiers'.
	FHOLGridContextView Hero;
	Hero.bOwnUnits = true;
	Hero.bSoldiers = true;
	Hero.bHero = true;
	Hero.HeroLevel = 1;
	FHOLGridCommandView Cry = HOLCommandGridRules::GetCommand(Hero, 0);
	T.AssertTrue(Cry.Command == EHOLGridCommandKind::HeroAbility && Cry.Ability == CloseRanks && !Cry.bGreyed && Cry.CooldownSeconds == 0, "level 1: A the cry, ready");
	FHOLGridCommandView BannerCmd = HOLCommandGridRules::GetCommand(Hero, 1);
	T.AssertTrue(BannerCmd.Command == EHOLGridCommandKind::HeroAbility && BannerCmd.Ability == Banner && BannerCmd.bGreyed, "level 1: Z the banner, greyed");
	for (int Cell = 2; Cell < HOLCommandGridRules::GetNumCells(); Cell++)
	{
		T.AssertTrue(HOLCommandGridRules::GetCommand(Hero, Cell).Command == EHOLGridCommandKind::None, "hero: the grid cell is empty");
	}
	T.AssertTrue(HOLCommandGridRules::GetOrder(Hero, HOLCommandGridRules::GetAttackMoveCell()).Command == EHOLGridCommandKind::AttackMove, "hero: Q attack-move");
	T.AssertTrue(HOLCommandGridRules::GetOrder(Hero, 0).Command == EHOLGridCommandKind::None, "hero: A no order");

	// The keys: A and Z to the grid, the others to the orders as before.
	T.AssertTrue(HOLCommandGridKeys::PressKey(Hero, -1, 0).Target == EHOLGridKeyTargetKind::Grid, "hero: A to the grid");
	T.AssertTrue(HOLCommandGridKeys::PressKey(Hero, -1, 1).Target == EHOLGridKeyTargetKind::Grid, "hero: Z to the grid");
	T.AssertTrue(HOLCommandGridKeys::PressKey(Hero, -1, 2).Target == EHOLGridKeyTargetKind::Orders, "hero: E to the orders");
	T.AssertTrue(HOLCommandGridKeys::PressKey(Hero, -1, HOLCommandGridRules::GetAttackMoveCell()).Target == EHOLGridKeyTargetKind::Orders, "hero: Q to the orders");

	// Level 4: the banner open. Cooling down: greyed, the seconds left told.
	Hero.HeroLevel = 4;
	T.AssertFalse(HOLCommandGridRules::GetCommand(Hero, 1).bGreyed, "level 4: Z ready");
	Hero.HeroCooldownSeconds.Add(0);
	Hero.HeroCooldownSeconds.Add(0);
	Hero.HeroCooldownSeconds.Add(0);
	Hero.HeroCooldownSeconds[CloseRanks] = 42;
	FHOLGridCommandView Cooling = HOLCommandGridRules::GetCommand(Hero, 0);
	T.AssertTrue(Cooling.bGreyed && Cooling.CooldownSeconds == 42, "cry cooling down: greyed, 42 s");
	T.AssertFalse(HOLCommandGridRules::GetCommand(Hero, 1).bGreyed, "the banner's cool-down is its own");

	// No hero among the units commanded (soldiers alone, another subgroup): nothing on the right, A and Z to the orders.
	FHOLGridContextView Soldiers = Hero;
	Soldiers.bHero = false;
	T.AssertTrue(HOLCommandGridRules::GetCommand(Soldiers, 0).Command == EHOLGridCommandKind::None && HOLCommandGridRules::GetCommand(Soldiers, 1).Command == EHOLGridCommandKind::None, "no hero: A empty");
	T.AssertTrue(HOLCommandGridKeys::PressKey(Soldiers, -1, 0).Target == EHOLGridKeyTargetKind::Orders, "no hero: A to the orders");

	// A building selected wins (never set together; the building's grid all the same).
	FHOLGridContextView Building = Hero;
	Building.bOwnBuilding = true;
	Building.bBuildingFinished = true;
	T.AssertTrue(HOLCommandGridRules::GetCommand(Building, 0).Command == EHOLGridCommandKind::None, "a building: no hero cell");
}

// HeroesOfLiberty.CommandGrid.Ages
void Test_CGAges(FUnitTest& T)
{
	// The 4 age panels: only age I reached for now; a building of an age not reached greyed.
	FHOLGridContextView Builders = CGMakeBuilders();
	Builders.bOwnUnits = true;
	Builders.BuildableTypes.Add(n"Barracks");
	Builders.BuildableCells.Add(4);
	Builders.BuildableAges.Add(1);
	for (int Age = 0; Age < HOLCommandGridRules::GetNumAges(); Age++)
	{
		bool bReached = HOLCommandGridRules::IsAgeReached(Builders, Age);
		T.AssertTrue(bReached == (Age == 0), "age reached: I only");
	}
	FHOLGridCommandView Locked = HOLCommandGridRules::GetAgeBuilding(Builders, 1, 4);
	T.AssertTrue(Locked.TypeName == n"Barracks" && Locked.Age == 1 && Locked.bGreyed, "II Q: the barracks, greyed");
	T.AssertTrue(HOLCommandGridRules::GetAgeBuilding(Builders, 2, 4).Command == EHOLGridCommandKind::None, "III: empty");

	// The pages' fading (D797): a page not reached faded; a Nomad's page II holding his first forum (bAlwaysOpen) shown whole at age I.
	float OpacityI = HOLCommandGridRules::GetAgePanelOpacity(Builders, 0);
	float OpacityII = HOLCommandGridRules::GetAgePanelOpacity(Builders, 1);
	T.AssertTrue(OpacityI == 1.0, "I reached: page whole");
	T.AssertTrue(OpacityII < 1.0, "II not reached: page faded");
	FHOLGridContextView Nomad = Builders;
	Nomad.BuildableTypes.Add(n"MainCentre");
	Nomad.BuildableCells.Add(10);
	Nomad.BuildableAges.Add(1);
	for (int I = 0; I < 4; I++)
	{
		Nomad.BuildableAlwaysOpen.Add(false);
	}
	Nomad.BuildableAlwaysOpen.Add(true);  // the last entry: the Nomad's first forum
	float NomadII = HOLCommandGridRules::GetAgePanelOpacity(Nomad, 1);
	float NomadIII = HOLCommandGridRules::GetAgePanelOpacity(Nomad, 2);
	T.AssertTrue(NomadII == 1.0, "Nomad: page II whole (its forum always buildable)");
	T.AssertTrue(NomadIII < 1.0, "Nomad: page III still faded");
	T.AssertFalse(HOLCommandGridRules::IsAgeReached(Nomad, 1), "Nomad: age II not reached (medallion locked)");
	T.AssertTrue(HOLCommandGridRules::GetAgeBuilding(Nomad, 1, 4).bGreyed, "Nomad: II Q barracks still greyed");

	// No age chosen: A chooses I; Z (not reached) nothing; Q S V the orders (a Peasant has no attack-move: GetOrder).
	T.AssertTrue(CGKeyIs(HOLCommandGridKeys::PressKey(Builders, -1, 0), EHOLGridKeyTargetKind::Age, 0), "A: age I chosen");
	T.AssertTrue(CGKeyIs(HOLCommandGridKeys::PressKey(Builders, -1, 1), EHOLGridKeyTargetKind::None, -1), "Z, not reached: nothing");
	T.AssertTrue(CGKeyIs(HOLCommandGridKeys::PressKey(Builders, -1, 4), EHOLGridKeyTargetKind::Orders, -1), "Q, no age: the orders");
	T.AssertTrue(HOLCommandGridKeys::PressKey(Builders, -1, 5).Target == EHOLGridKeyTargetKind::Orders
		&& HOLCommandGridRules::GetOrder(Builders, 5).Command == EHOLGridCommandKind::Stop, "S, no age: the orders (stop)");
	T.AssertTrue(HOLCommandGridKeys::PressKey(Builders, -1, 11).Target == EHOLGridKeyTargetKind::Orders, "V, no age: the orders");

	// Age I chosen: every key its building's, A then A the house's ghost, the age kept; then E the mining camp's, Z the lumber camp's, still I.
	T.AssertTrue(CGKeyBuild(HOLCommandGridKeys::PressKey(Builders, 0, 0), 0, n"House"), "A then A: the house, I kept");
	T.AssertTrue(CGKeyBuild(HOLCommandGridKeys::PressKey(Builders, 0, 2), 0, n"MiningCamp"), "then E: the mining camp, I kept");
	T.AssertTrue(CGKeyBuild(HOLCommandGridKeys::PressKey(Builders, 0, 1), 0, n"LumberCamp"), "then Z: the lumber camp, I kept");
	// An empty cell of the chosen age: nothing, never an order nor an age, the age kept.
	T.AssertTrue(CGKeyIs(HOLCommandGridKeys::PressKey(Builders, 0, 11), EHOLGridKeyTargetKind::None, 0), "I, V (empty): nothing, I kept");
	T.AssertTrue(CGKeyIs(HOLCommandGridKeys::PressKey(Builders, 0, 3), EHOLGridKeyTargetKind::None, 0), "I, R (empty): nothing, I kept");
	T.AssertTrue(CGKeyIs(HOLCommandGridKeys::PressKey(Builders, 0, 4), EHOLGridKeyTargetKind::None, 0), "I, Q (empty): nothing, I kept");
	T.AssertTrue(CGKeyIs(HOLCommandGridKeys::PressKey(Builders, 0, 12), EHOLGridKeyTargetKind::None, 0), "out of the grid: nothing, I kept");

	// Age II reached (later): Z chooses it with no age chosen, its Q the barracks; an age chosen, Z is its building's.
	FHOLGridContextView Later = Builders;
	Later.ReachedAge = 1;
	T.AssertTrue(CGKeyIs(HOLCommandGridKeys::PressKey(Later, -1, 1), EHOLGridKeyTargetKind::Age, 1), "II reached, no age, Z: II chosen");
	T.AssertTrue(CGKeyBuild(HOLCommandGridKeys::PressKey(Later, 0, 1), 0, n"LumberCamp"), "II reached, I chosen, Z: the lumber camp, I kept");
	T.AssertTrue(CGKeyIs(HOLCommandGridKeys::PressKey(Later, 1, 0), EHOLGridKeyTargetKind::None, 1), "II, A (empty): nothing, II kept");
	T.AssertTrue(CGKeyBuild(HOLCommandGridKeys::PressKey(Later, 1, 4), 1, n"Barracks"), "II, Q: the barracks");
	T.AssertTrue(CGKeyIs(HOLCommandGridKeys::PressKey(Later, 1, 5), EHOLGridKeyTargetKind::None, 1), "II, S (empty): nothing");

	// The mouse: a header chooses its age, a building its ghost (its age chosen); an age not reached nothing, the chosen kept.
	T.AssertTrue(CGKeyIs(HOLCommandGridKeys::ClickAge(Builders, -1, 0, -1), EHOLGridKeyTargetKind::Age, 0), "click header I");
	T.AssertTrue(CGKeyIs(HOLCommandGridKeys::ClickAge(Builders, 0, 1, -1), EHOLGridKeyTargetKind::None, 0), "click header II (not reached): nothing");
	T.AssertTrue(CGKeyIs(HOLCommandGridKeys::ClickAge(Builders, 0, 0, -1), EHOLGridKeyTargetKind::Age, 0), "click header I, I chosen: I");
	T.AssertTrue(CGKeyBuild(HOLCommandGridKeys::ClickAge(Builders, -1, 0, 1), 0, n"LumberCamp"), "click I Z: the lumber camp, I chosen");
	T.AssertTrue(CGKeyIs(HOLCommandGridKeys::ClickAge(Builders, -1, 1, 4), EHOLGridKeyTargetKind::None, -1), "click II Q (not reached): nothing");

	// The chosen age closed by a placement, a cancel, Escape, another selection; kept by a placement with Shift.
	T.AssertEquals(-1, HOLCommandGridKeys::GetChosenAgeAfter(0, EHOLAgeEventKind::Placed), "placed: closed");
	T.AssertEquals(0, HOLCommandGridKeys::GetChosenAgeAfter(0, EHOLAgeEventKind::PlacedKept), "placed with Shift: kept");
	T.AssertEquals(-1, HOLCommandGridKeys::GetChosenAgeAfter(0, EHOLAgeEventKind::Cancelled), "cancelled: closed");
	T.AssertEquals(-1, HOLCommandGridKeys::GetChosenAgeAfter(0, EHOLAgeEventKind::Escape), "Escape: closed");
	T.AssertEquals(-1, HOLCommandGridKeys::GetChosenAgeAfter(0, EHOLAgeEventKind::SelectionChanged), "another selection: closed");

	// Soldiers among the selection: no panels, A Z E R go to the orders (nothing there), Q to the attack-move.
	FHOLGridContextView Mixed = Builders;
	Mixed.bSoldiers = true;
	T.AssertTrue(CGKeyIs(HOLCommandGridKeys::PressKey(Mixed, -1, 0), EHOLGridKeyTargetKind::Orders, -1), "mixed, A: the orders");
	T.AssertTrue(HOLCommandGridKeys::PressKey(Mixed, -1, 4).Target == EHOLGridKeyTargetKind::Orders, "mixed, Q: the orders");
	T.AssertTrue(HOLCommandGridRules::GetOrder(Mixed, 4).Command == EHOLGridCommandKind::AttackMove, "mixed, Q: attack-move");
	T.AssertTrue(HOLCommandGridRules::GetAgeBuilding(Mixed, 0, 0).Command == EHOLGridCommandKind::None, "mixed: no panel");
	T.AssertTrue(CGKeyIs(HOLCommandGridKeys::ClickAge(Mixed, -1, 0, 0), EHOLGridKeyTargetKind::None, -1), "mixed: a click on a panel does nothing");
}

// HeroesOfLiberty.CommandGrid.MouseCursor
void Test_CGMouseCursor(FUnitTest& T)
{
	// The user's trial of A.2: the red attack cursor while Q waits for its point, the arrow again once chosen or cancelled.
	// Script rule (HOLCommandGridRules::IsAttackCursor), the one the game reads: true = the red attack cursor, false = the arrow.
	T.AssertTrue(HOLCommandGridRules::IsAttackCursor(true, false), "Q pressed, the point not chosen: the attack cursor");
	T.AssertFalse(HOLCommandGridRules::IsAttackCursor(false, false), "the point chosen or the mode cancelled: the arrow");
	T.AssertFalse(HOLCommandGridRules::IsAttackCursor(false, true), "placement mode: the arrow (the ghost shows it)");
	T.AssertFalse(HOLCommandGridRules::IsAttackCursor(true, true), "placement wins over a mode left armed");
}

// HeroesOfLiberty.CommandGrid.Research
void Test_CGResearch(FUnitTest& T)
{
	// The table's side: what each building researches, in the order of the table.
	TArray<FName> FarmTechs = HOLCommandGridRead::GetBuildingResearches(n"Farm");
	TArray<FName> FarmExpected;
	FarmExpected.Add(n"PloughI");
	FarmExpected.Add(n"PloughII");
	FarmExpected.Add(n"WheelbarrowI");
	FarmExpected.Add(n"WheelbarrowII");
	T.AssertTrue(CGNamesEqual(FarmTechs, FarmExpected), "the Farm researches the Plough and the Wheelbarrow, I then II");
	TArray<FName> CentreExpected;
	CentreExpected.Add(n"ImprovedAlarmBell");
	T.AssertTrue(CGNamesEqual(HOLCommandGridRead::GetBuildingResearches(n"MainCentre"), CentreExpected), "the main centre researches the alarm bell only (the militia is dropped, D600)");
	TArray<FName> BarracksExpected;
	BarracksExpected.Add(n"TemperedHalberds");
	T.AssertTrue(CGNamesEqual(HOLCommandGridRead::GetBuildingResearches(n"Barracks"), BarracksExpected), "the Barracks research the halberds");
	T.AssertEquals(0, HOLCommandGridRead::GetBuildingResearches(n"House").Num(), "the House researches nothing");

	// The rule's side: a finished building gives its research row from W, after its production on A; a site, a forum down, builders: none.
	FHOLGridContextView Farm;
	Farm.bOwnBuilding = true;
	Farm.bBuildingFinished = true;
	Farm.Researches = FarmTechs;
	int First = HOLCommandGridRules::GetResearchFirstCell();
	T.AssertEquals(8, First, "the research row starts on W");
	for (int Cell = 0; Cell < HOLCommandGridRules::GetNumCells(); Cell++)
	{
		FHOLGridCommandView Command = HOLCommandGridRules::GetCommand(Farm, Cell);
		int Index = Cell - First;
		if (Index >= 0 && Index < 2)
		{
			// D856: one cell per LINE, nothing researched: W the Plough (I), X the Wheelbarrow (I); not one cell per technology (I, II, I, II).
			FName Head = n"PloughI";
			if (Index == 1)
			{
				Head = n"WheelbarrowI";
			}
			T.AssertTrue(Command.Command == EHOLGridCommandKind::Research && Command.TypeName == Head, "Farm: the cell researches the first level of its line (D856)");
		}
		else
		{
			T.AssertTrue(Command.Command == EHOLGridCommandKind::None, "Farm: the cell is empty");
		}
	}
	FHOLGridContextView Centre;
	Centre.bOwnBuilding = true;
	Centre.bBuildingFinished = true;
	Centre.Produces.Add(n"Peasant");
	Centre.Researches = HOLCommandGridRead::GetBuildingResearches(n"MainCentre");
	T.AssertTrue(HOLCommandGridRules::GetCommand(Centre, 0).Command == EHOLGridCommandKind::Produce, "centre: A still produces a Peasant");
	T.AssertTrue(HOLCommandGridRules::GetCommand(Centre, 8).TypeName == n"ImprovedAlarmBell" && HOLCommandGridRules::GetCommand(Centre, 9).Command == EHOLGridCommandKind::None, "centre: W the alarm bell, X empty");
	FHOLGridContextView Site = Farm;
	Site.bBuildingFinished = false;
	T.AssertTrue(HOLCommandGridRules::GetCommand(Site, 8).Command == EHOLGridCommandKind::None, "a site researches nothing");
	T.AssertTrue(HOLCommandGridRules::GetCommand(Site, HOLCommandGridRules::GetCancelCell()).Command == EHOLGridCommandKind::CancelConstruction, "a site keeps its cancel on V");
	FHOLGridContextView Down = Centre;
	Down.bForumDown = true;
	T.AssertTrue(HOLCommandGridRules::GetCommand(Down, 8).Command == EHOLGridCommandKind::None, "a forum down researches nothing");
	T.AssertTrue(HOLCommandGridKeys::PressKey(Farm, -1, 8).Target == EHOLGridKeyTargetKind::Grid, "every key of a building is the grid's");

	// What a press tells (the order of the server's reasons, then the stock, then the queue).
	bool bPlough1 = HOLCommandGridRead::IsTechnologyInTable(n"PloughI");
	bool bPlough2 = HOLCommandGridRead::IsTechnologyInTable(n"PloughII");
	T.AssertTrue(bPlough1, "PloughI in the table");
	T.AssertTrue(bPlough2, "PloughII in the table");
	if (!bPlough1 || !bPlough2)
	{
		return;
	}
	FHOLResources Rich = CGRes(5000, 5000, 5000, 5000);
	FHOLTechFactsView Facts;
	Facts.HeroTier = 1;
	Facts.bBuildingFinished = true;
	T.AssertTrue(HOLCommandGridRead::CheckResearch(n"PloughI", Facts, Rich, 0, 5).Kind == EHOLCommandFeedbackKind::None, "tier 1, rich, room: goes through");
	Facts.HeroTier = 0;
	FHOLFeedbackView Tier = HOLCommandGridRead::CheckResearch(n"PloughI", Facts, Rich, 0, 5);
	T.AssertTrue(Tier.Kind == EHOLCommandFeedbackKind::ResearchRefused && Tier.Block == HOLCommandGridRead::GetBlockTierTooLow() && HOLCommandGridKeys::Blocks(Tier), "tier 0: refused, the tier");
	FString TierText = HOLCommandGridKeys::Describe(Tier);
	T.AssertTrue(TierText == "research refused (tier too low)", "the tier is logged");
	Facts.HeroTier = 1;
	Facts.bResearched = true;
	T.AssertEquals(HOLCommandGridRead::GetBlockAlreadyResearched(), HOLCommandGridRead::CheckResearch(n"PloughI", Facts, Rich, 0, 5).Block, "done already");
	Facts.bResearched = false;
	Facts.bQueued = true;
	T.AssertEquals(HOLCommandGridRead::GetBlockAlreadyQueued(), HOLCommandGridRead::CheckResearch(n"PloughI", Facts, Rich, 0, 5).Block, "already in a queue");
	Facts.bQueued = false;
	Facts.bRequiresDone = false;
	Facts.HeroTier = 2;
	T.AssertEquals(HOLCommandGridRead::GetBlockRequiresMissing(), HOLCommandGridRead::CheckResearch(n"PloughII", Facts, Rich, 0, 5).Block, "level II before level I");
	Facts.bRequiresDone = true;
	Facts.bBuildingFinished = false;
	T.AssertEquals(HOLCommandGridRead::GetBlockNoBuilding(), HOLCommandGridRead::CheckResearch(n"PloughI", Facts, Rich, 0, 5).Block, "no finished building");
	Facts.bBuildingFinished = true;
	int FoodCost = HOLCommandGridRead::GetTechnologyFoodCost(n"PloughI");
	FHOLFeedbackView Poor = HOLCommandGridRead::CheckResearch(n"PloughI", Facts, CGRes(FoodCost - 1, 5000, 5000, 5000), 0, 5);
	T.AssertTrue(Poor.Kind == EHOLCommandFeedbackKind::NotEnoughResources && Poor.Resource == EHOLResource::Food && HOLCommandGridKeys::Blocks(Poor), "one food short: not enough food, blocks");
	FHOLFeedbackView Full = HOLCommandGridRead::CheckResearch(n"PloughI", Facts, Rich, 5, 5);
	T.AssertTrue(Full.Kind == EHOLCommandFeedbackKind::QueueFull && HOLCommandGridKeys::Blocks(Full), "5 of 5 in the queue: queue full, blocks");
	Facts.HeroTier = 0;
	T.AssertEquals(HOLCommandGridRead::GetBlockTierTooLow(), HOLCommandGridRead::CheckResearch(n"PloughI", Facts, Rich, 5, 5).Block, "the rules before the queue");
	T.AssertEquals(HOLCommandGridRead::GetBlockTierTooLow(), HOLCommandGridRead::CheckResearch(n"PloughI", Facts, CGRes(0, 0, 0, 0), 0, 5).Block, "the rules before the stock");

	// T-Q10/T-Q11 = B (D601): the Improved Alarm Bell and the Tempered Halberds are in the grid but have no effect yet: refused "Soon",
	// whatever the tier, the building, the stock or the queue, and the press tells it.
	TArray<FName> SoonNames;
	SoonNames.Add(n"ImprovedAlarmBell");
	SoonNames.Add(n"TemperedHalberds");
	for (int I = 0; I < SoonNames.Num(); I++)
	{
		FName SoonName = SoonNames[I];
		bool bInTable = HOLCommandGridRead::IsTechnologyInTable(SoonName);
		T.AssertTrue(bInTable, "the soon technology is in the table");
		if (!bInTable)
		{
			continue;
		}
		T.AssertFalse(HOLCommandGridRead::IsTechnologyImplemented(SoonName), "not implemented in the table");
		FHOLTechFactsView SoonFacts;
		SoonFacts.HeroTier = 2;
		SoonFacts.bBuildingFinished = true;
		FHOLFeedbackView Refused = HOLCommandGridRead::CheckResearch(SoonName, SoonFacts, Rich, 0, 5);
		T.AssertTrue(Refused.Kind == EHOLCommandFeedbackKind::ResearchRefused && Refused.Block == HOLCommandGridRead::GetBlockNotImplemented() && HOLCommandGridKeys::Blocks(Refused), "refused, soon, and it blocks");
		FString SoonText = HOLCommandGridKeys::Describe(Refused);
		T.AssertTrue(SoonText == "research refused (soon)", "the log says soon");
		SoonFacts.HeroTier = 0;
		SoonFacts.bBuildingFinished = false;
		T.AssertEquals(HOLCommandGridRead::GetBlockNotImplemented(), HOLCommandGridRead::CheckResearch(SoonName, SoonFacts, CGRes(0, 0, 0, 0), 5, 5).Block, "soon even with no tier, no building, no stock");
	}
}

// GLORFINDEL-1120pe (D813, Q-garrison-ui Q1 C): the "Sortir tous" cell of a finished own building with occupants. Red until the rule gives the cell.
// To turn it red by hand: in HOLCommandGridRules::GetCommandWithOptions drop the bGarrisonOccupied branch.
void Test_GridUngarrisonAllCellOfAGarrisonedBuilding(FUnitTest& T)
{
	FHOLGridContextView Barracks;
	Barracks.bOwnBuilding = true;
	Barracks.bBuildingFinished = true;
	Barracks.Produces = HOLCommandGridRead::GetProducesOf(n"Barracks");
	int Cell = HOLCommandGridRules::GetUngarrisonAllCell();
	FHOLGridCommandView Empty = HOLCommandGridRules::GetCommand(Barracks, Cell);
	T.AssertTrue(Empty.Command == EHOLGridCommandKind::None, "nobody inside: the Sortir tous cell is empty");
	Barracks.bGarrisonOccupied = true;
	FHOLGridCommandView Full = HOLCommandGridRules::GetCommand(Barracks, Cell);
	T.AssertTrue(Full.Command == EHOLGridCommandKind::UngarrisonAll, "occupants inside: the cell sends everyone out");
	T.AssertTrue(HOLCommandGridRules::GetCommand(Barracks, 0).Command == EHOLGridCommandKind::Produce, "the production cells stay (A still produces)");
	Barracks.bBuildingFinished = false;
	FHOLGridCommandView Site = HOLCommandGridRules::GetCommand(Barracks, Cell);
	T.AssertTrue(Site.Command != EHOLGridCommandKind::UngarrisonAll, "a site holds nobody: no Sortir tous cell");
	FHOLGridContextView Units;
	Units.bOwnUnits = true;
	Units.bGarrisonOccupied = true;
	T.AssertTrue(HOLCommandGridRules::GetCommand(Units, Cell).Command != EHOLGridCommandKind::UngarrisonAll, "units selected, no building: no Sortir tous cell");
}