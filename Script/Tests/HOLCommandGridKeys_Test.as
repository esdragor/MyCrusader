// MIG-C-CommandGrid, lot G2a (SAURON-1119x), trimmed by L3b (HALDIR-1119cd): the command grid's keys and feedback (script rule HOLCommandGridKeys). NOT COMPILED when written.
// The sweeps against the *Native bodies (PressKey, ClickAge, GetChosenAgeAfter, CheckBuild, CheckProduction, Blocks, Describe) are gone (one source, D836). What stays:
// Test_CGKYAgreementFeedback (missing resource, population, the numbers the script assumes, the cell face: against the C++ code that stays) and Test_CGKYHardValues (the numbers in clear).
// To SEE RED (rouge a voir): in HOLCommandGridKeys.as PressKey, change `Result.ChosenAge = ChosenAge;` to `Result.ChosenAge = -1;` (Test_CGKYHardValues goes red), or in Describe
// change "queue full" to "queue is full", or in FindMissingResource swap the Food and Wood checks (Test_CGKYAgreementFeedback goes red).
// Names are prefixed CGKY: a global function of a test file is seen by no other file but its name must not clash.

FHOLResources CGKYRes(int Food, int Wood, int Stone, int Gold)
{
	FHOLResources Out;
	Out.Food = Food;
	Out.Wood = Wood;
	Out.Stone = Stone;
	Out.Gold = Gold;
	return Out;
}

// The builders' grid of the HARD values: the exact context of HOLCommandGrid_Test.as (CGMakeBuilders 9-23 + Test_CGAges 806-810): three buildable types
// at age I cells A Z E, the Barracks at age II cell Q, nothing else. NOT the table's (CGKYMakeBuilders): the real table has other buildings at age II
// (ELROND-1119x: with it, II Q and II S of the hard values failed while the agreement sweep, same context, stayed green).
FHOLGridContextView CGKYMakeHardBuilders()
{
	FHOLGridContextView Builders;
	Builders.bBuilders = true;
	Builders.bOwnUnits = true;
	Builders.BuildableTypes.Add(n"House");
	Builders.BuildableCells.Add(0);
	Builders.BuildableAges.Add(0);
	Builders.BuildableTypes.Add(n"LumberCamp");
	Builders.BuildableCells.Add(1);
	Builders.BuildableAges.Add(0);
	Builders.BuildableTypes.Add(n"MiningCamp");
	Builders.BuildableCells.Add(2);
	Builders.BuildableAges.Add(0);
	Builders.BuildableTypes.Add(n"Barracks");
	Builders.BuildableCells.Add(4);
	Builders.BuildableAges.Add(1);
	return Builders;
}

// HeroesOfLiberty.CommandGridKeys.AgreementFeedback (the script's feedback helpers against the C++ code that stays: missing resource, population, numbers, cell face)
void Test_CGKYAgreementFeedback(FUnitTest& T)
{
	int Mismatches = 0;
	FString First = "";
	TArray<int> Pool;
	Pool.Add(0);
	Pool.Add(10);
	Pool.Add(50);

	// HasMissingResource / GetMissingResource: 81 stocks x 81 costs.
	for (int S = 0; S < 81; S++)
	{
		FHOLResources Stock = CGKYRes(Pool[S % 3], Pool[(S / 3) % 3], Pool[(S / 9) % 3], Pool[(S / 27) % 3]);
		for (int C = 0; C < 81; C++)
		{
			FHOLResources Cost = CGKYRes(Pool[C % 3], Pool[(C / 3) % 3], Pool[(C / 9) % 3], Pool[(C / 27) % 3]);
			bool bScriptMissing = HOLCommandGridKeys::HasMissingResource(Stock, Cost);
			bool bNativeMissing = HOLCommandGridRead::HasMissingResource(Stock, Cost);
			bool bSame = bScriptMissing == bNativeMissing;
			if (bSame && bNativeMissing)
			{
				bSame = HOLCommandGridKeys::FindMissingResource(Stock, Cost) == int(HOLCommandGridRead::GetMissingResource(Stock, Cost));
			}
			if (!bSame)
			{
				Mismatches++;
				if (First.IsEmpty())
				{
					First = f"missing resource stock {S} cost {C}";
				}
			}
		}
	}
	for (int Taken = 0; Taken <= 12; Taken++)
	{
		for (int Cap = 8; Cap <= 11; Cap++)
		{
			for (int Place = -2; Place <= 4; Place++)
			{
				if (HOLCommandGridKeys::IsPopulationBlocked(Taken, Cap, Place) != HOLCommandGridRead::IsPopulationBlocked(Taken, Cap, Place))
				{
					Mismatches++;
					if (First.IsEmpty())
					{
						First = f"IsPopulationBlocked {Taken} / {Cap} places {Place}";
					}
				}
			}
		}
	}

	// The numbers the script assumes for the C++ enums that are not UENUMs (the library gives only some of them).
	T.AssertEquals(HOLCommandGridRead::GetPlacementRefusalBuilding(), HOLCommandGridKeys::RefusalBuilding, "the placement refusal numbers the script assumes: Building");
	T.AssertEquals(HOLCommandGridRead::GetBlockNotImplemented(), HOLCommandGridKeys::BlockNotImplemented, "the technology block numbers the script assumes: NotImplemented");
	T.AssertEquals(HOLCommandGridRead::GetBlockAlreadyResearched(), HOLCommandGridKeys::BlockAlreadyResearched, "the technology block numbers the script assumes: AlreadyResearched");
	T.AssertEquals(HOLCommandGridRead::GetBlockAlreadyQueued(), HOLCommandGridKeys::BlockAlreadyQueued, "the technology block numbers the script assumes: AlreadyQueued");
	T.AssertEquals(HOLCommandGridRead::GetBlockTierTooLow(), HOLCommandGridKeys::BlockTierTooLow, "the technology block numbers the script assumes: TierTooLow");
	T.AssertEquals(HOLCommandGridRead::GetBlockRequiresMissing(), HOLCommandGridKeys::BlockRequiresMissing, "the technology block numbers the script assumes: RequiresMissing");
	T.AssertEquals(HOLCommandGridRead::GetBlockNoBuilding(), HOLCommandGridKeys::BlockNoBuilding, "the technology block numbers the script assumes: NoBuilding");

	// The cell face.
	for (int Pass = 0; Pass < 2; Pass++)
	{
		FHOLGridCellFaceView ScriptFace = HOLCommandGridKeys::GetCellFace(Pass == 1);
		FHOLGridCellFaceView NativeFace = HOLCommandGridRead::GetCellFace(Pass == 1);
		if (ScriptFace.bIcon != NativeFace.bIcon || ScriptFace.bName != NativeFace.bName || ScriptFace.bCost != NativeFace.bCost)
		{
			Mismatches++;
			if (First.IsEmpty())
			{
				First = f"GetCellFace {Pass}";
			}
		}
	}
	T.AssertEquals(0, Mismatches, First);
}


// HeroesOfLiberty.CommandGridKeys.HardValues
void Test_CGKYHardValues(FUnitTest& T)
{
	// Feedback (copied from Script/Tests/HOLCommandGrid_Test.as 211-236).
	T.AssertFalse(HOLCommandGridKeys::HasMissingResource(CGKYRes(0, 50, 0, 0), CGKYRes(0, 50, 0, 0)), "covered: nothing missing");
	T.AssertTrue(HOLCommandGridKeys::HasMissingResource(CGKYRes(500, 49, 0, 0), CGKYRes(0, 50, 0, 0))
		&& HOLCommandGridKeys::FindMissingResource(CGKYRes(500, 49, 0, 0), CGKYRes(0, 50, 0, 0)) == int(EHOLResource::Wood), "49 wood for 50: wood missing");
	T.AssertEquals(int(EHOLResource::Food), HOLCommandGridKeys::FindMissingResource(CGKYRes(0, 0, 0, 0), CGKYRes(10, 0, 0, 10)), "food and gold missing: food first");
	T.AssertEquals(int(EHOLResource::Stone), HOLCommandGridKeys::FindMissingResource(CGKYRes(9, 9, 0, 9), CGKYRes(0, 0, 1, 0)), "stone missing");
	FHOLFeedbackView NoWood = HOLCommandGridKeys::CheckBuild(CGKYRes(200, 0, 200, 200), CGKYRes(0, 50, 0, 0));
	T.AssertTrue(NoWood.Kind == EHOLCommandFeedbackKind::NotEnoughResources && NoWood.Resource == EHOLResource::Wood, "house without wood: not enough Wood");
	T.AssertTrue(HOLCommandGridKeys::Blocks(NoWood), "not enough: blocks");
	T.AssertTrue(HOLCommandGridKeys::CheckBuild(CGKYRes(0, 50, 0, 0), CGKYRes(0, 50, 0, 0)).Kind == EHOLCommandFeedbackKind::None, "house with 50 wood: goes through");
	T.AssertTrue(HOLCommandGridKeys::Describe(NoWood) == "not enough Wood", "logged");
	FHOLResources Peasant = CGKYRes(50, 0, 0, 0);
	T.AssertTrue(HOLCommandGridKeys::CheckProduction(CGKYRes(50, 0, 0, 0), Peasant, 0, 5, 3, 10, 1).Kind == EHOLCommandFeedbackKind::None, "room, resources: goes through");
	FHOLFeedbackView Full = HOLCommandGridKeys::CheckProduction(CGKYRes(0, 0, 0, 0), Peasant, 5, 5, 10, 10, 1);
	T.AssertTrue(Full.Kind == EHOLCommandFeedbackKind::QueueFull && HOLCommandGridKeys::Blocks(Full), "queue full before the cost");
	T.AssertTrue(HOLCommandGridKeys::Describe(Full) == "queue full", "queue full logged");
	FHOLFeedbackView NoFood = HOLCommandGridKeys::CheckProduction(CGKYRes(49, 0, 0, 0), Peasant, 0, 5, 10, 10, 1);
	T.AssertTrue(NoFood.Kind == EHOLCommandFeedbackKind::NotEnoughResources && NoFood.Resource == EHOLResource::Food, "49 food for 50: not enough Food");
	FHOLFeedbackView Population = HOLCommandGridKeys::CheckProduction(CGKYRes(50, 0, 0, 0), Peasant, 0, 5, 10, 10, 1);
	T.AssertTrue(Population.Kind == EHOLCommandFeedbackKind::PopulationFull && HOLCommandGridKeys::Describe(Population) == "population full", "10 / 10: population full");
	T.AssertFalse(HOLCommandGridKeys::Blocks(Population), "population full: queued all the same (it waits)");
	T.AssertTrue(HOLCommandGridKeys::CheckProduction(CGKYRes(50, 0, 0, 0), Peasant, 0, 5, 9, 10, 1).Kind == EHOLCommandFeedbackKind::None, "9 / 10: room for one");
	T.AssertTrue(HOLCommandGridKeys::IsPopulationBlocked(9, 10, 2), "9 / 10: no room for two places");
	T.AssertFalse(HOLCommandGridKeys::IsPopulationBlocked(9, 10, 1), "9 / 10: room for one place");
	FHOLFeedbackView Ghost;
	Ghost.Kind = EHOLCommandFeedbackKind::PlacementRefused;
	Ghost.Refusal = HOLCommandGridKeys::RefusalBuilding;
	T.AssertTrue(HOLCommandGridKeys::Blocks(Ghost) && HOLCommandGridKeys::Describe(Ghost) == "placement refused (on a building)", "red ghost: blocks, logged");
	FHOLFeedbackView NoMilitary;
	NoMilitary.Kind = EHOLCommandFeedbackKind::NoBuilding;
	NoMilitary.Category = EHOLBuildingCategory::Military;
	T.AssertTrue(HOLCommandGridKeys::Describe(NoMilitary) == "no Military building" && !HOLCommandGridKeys::Blocks(NoMilitary), "no military building: logged, nothing to block");

	// The keys (copied from Script/Tests/HOLCommandGrid_Test.as 842-880): the builders' grid, only age I reached, then II.
	FHOLGridContextView Builders = CGKYMakeHardBuilders();
	FName NoName;
	FHOLGridKeyResultView Key = HOLCommandGridKeys::PressKey(Builders, -1, 0);
	T.AssertTrue(Key.Target == EHOLGridKeyTargetKind::Age && Key.ChosenAge == 0, "A: age I chosen");
	Key = HOLCommandGridKeys::PressKey(Builders, -1, 1);
	T.AssertTrue(Key.Target == EHOLGridKeyTargetKind::None && Key.ChosenAge == -1, "Z, not reached: nothing");
	Key = HOLCommandGridKeys::PressKey(Builders, -1, 4);
	T.AssertTrue(Key.Target == EHOLGridKeyTargetKind::Orders && Key.ChosenAge == -1, "Q, no age: the orders");
	Key = HOLCommandGridKeys::PressKey(Builders, -1, 11);
	T.AssertTrue(Key.Target == EHOLGridKeyTargetKind::Orders, "V, no age: the orders");
	Key = HOLCommandGridKeys::PressKey(Builders, 0, 0);
	T.AssertTrue(Key.Target == EHOLGridKeyTargetKind::Build && Key.ChosenAge == 0 && Key.TypeName == n"House", "A then A: the house, I kept");
	Key = HOLCommandGridKeys::PressKey(Builders, 0, 1);
	T.AssertTrue(Key.Target == EHOLGridKeyTargetKind::Build && Key.TypeName == n"LumberCamp", "then Z: the lumber camp");
	Key = HOLCommandGridKeys::PressKey(Builders, 0, 2);
	T.AssertTrue(Key.Target == EHOLGridKeyTargetKind::Build && Key.TypeName == n"MiningCamp", "then E: the mining camp");
	Key = HOLCommandGridKeys::PressKey(Builders, 0, 11);
	T.AssertTrue(Key.Target == EHOLGridKeyTargetKind::None && Key.ChosenAge == 0 && Key.TypeName == NoName, "I, V (empty): nothing, I kept");
	Key = HOLCommandGridKeys::PressKey(Builders, 0, 12);
	T.AssertTrue(Key.Target == EHOLGridKeyTargetKind::None && Key.ChosenAge == 0, "out of the grid: nothing, I kept");
	FHOLGridContextView Later = Builders;
	Later.ReachedAge = 1;
	Key = HOLCommandGridKeys::PressKey(Later, -1, 1);
	T.AssertTrue(Key.Target == EHOLGridKeyTargetKind::Age && Key.ChosenAge == 1, "II reached, no age, Z: II chosen");
	Key = HOLCommandGridKeys::PressKey(Later, 1, 4);
	T.AssertTrue(Key.Target == EHOLGridKeyTargetKind::Build && Key.ChosenAge == 1 && Key.TypeName == n"Barracks", "II, Q: the barracks");
	Key = HOLCommandGridKeys::PressKey(Later, 1, 5);
	T.AssertTrue(Key.Target == EHOLGridKeyTargetKind::None && Key.ChosenAge == 1, "II, S (empty): nothing");
	Key = HOLCommandGridKeys::ClickAge(Builders, -1, 0, -1);
	T.AssertTrue(Key.Target == EHOLGridKeyTargetKind::Age && Key.ChosenAge == 0, "click header I");
	Key = HOLCommandGridKeys::ClickAge(Builders, 0, 1, -1);
	T.AssertTrue(Key.Target == EHOLGridKeyTargetKind::None && Key.ChosenAge == 0, "click header II (not reached): nothing, I kept");
	Key = HOLCommandGridKeys::ClickAge(Builders, -1, 0, 1);
	T.AssertTrue(Key.Target == EHOLGridKeyTargetKind::Build && Key.ChosenAge == 0 && Key.TypeName == n"LumberCamp", "click I Z: the lumber camp, I chosen");
	T.AssertEquals(-1, HOLCommandGridKeys::GetChosenAgeAfter(0, EHOLAgeEventKind::Placed), "placed: closed");
	T.AssertEquals(0, HOLCommandGridKeys::GetChosenAgeAfter(0, EHOLAgeEventKind::PlacedKept), "placed with Shift: kept");
	T.AssertEquals(-1, HOLCommandGridKeys::GetChosenAgeAfter(0, EHOLAgeEventKind::Escape), "Escape: closed");

	// The hero's keys (copied from 773-776) and the other units' (793): A Z go to the grid, E Q to the orders.
	FHOLGridContextView Hero;
	Hero.bHero = true;
	Hero.bOwnUnits = true;
	Hero.HeroLevel = 1;
	T.AssertTrue(HOLCommandGridKeys::PressKey(Hero, -1, 0).Target == EHOLGridKeyTargetKind::Grid, "hero: A to the grid");
	T.AssertTrue(HOLCommandGridKeys::PressKey(Hero, -1, 1).Target == EHOLGridKeyTargetKind::Grid, "hero: Z to the grid");
	T.AssertTrue(HOLCommandGridKeys::PressKey(Hero, -1, 2).Target == EHOLGridKeyTargetKind::Orders, "hero: E to the orders");
	T.AssertTrue(HOLCommandGridKeys::PressKey(Hero, -1, 4).Target == EHOLGridKeyTargetKind::Orders, "hero: Q to the orders");
	FHOLGridContextView Soldiers;
	Soldiers.bOwnUnits = true;
	Soldiers.bSoldiers = true;
	T.AssertTrue(HOLCommandGridKeys::PressKey(Soldiers, -1, 0).Target == EHOLGridKeyTargetKind::Orders, "no hero: A to the orders");
	FHOLGridContextView Market;
	Market.bOwnBuilding = true;
	Market.bBuildingFinished = true;
	Market.bMarket = true;
	T.AssertTrue(HOLCommandGridKeys::PressKey(Market, -1, 0).Target == EHOLGridKeyTargetKind::Grid, "a key on a Market goes to the grid");
}

// L1b (SAURON-1119cc, plan C6-PLAN-1119 section 3): the builders' grid of a seat at tier 0, built by hand as UHOLBuildingSettings::GetBuildableTypesForSlot
// gives it (HOLBuildingTypes.cpp 72-82): the Nomad's list holds the main centre (the forum) at age II (index 1) in cell 10 (C, the C++ default of
// hol.Nomad.ForumCell) with bAlwaysOpen; the standard seat's list does not. The Market stands at II A (cell 0), not always open.
FHOLGridContextView CGKYMakeNomadBuilders(bool bNomad)
{
	FHOLGridContextView Grid;
	Grid.bBuilders = true;
	Grid.bOwnUnits = true;
	Grid.ReachedAge = 0;
	Grid.BuildableTypes.Add(n"House");
	Grid.BuildableCells.Add(0);
	Grid.BuildableAges.Add(0);
	Grid.BuildableAlwaysOpen.Add(false);
	Grid.BuildableTypes.Add(n"Market");
	Grid.BuildableCells.Add(0);
	Grid.BuildableAges.Add(1);
	Grid.BuildableAlwaysOpen.Add(false);
	if (bNomad)
	{
		Grid.BuildableTypes.Add(n"MainCentre");
		Grid.BuildableCells.Add(10);
		Grid.BuildableAges.Add(1);
		Grid.BuildableAlwaysOpen.Add(true);
	}
	return Grid;
}

// HeroesOfLiberty.CommandGridKeys.NomadAges
// Covers HOLStartingHeroTests.cpp 648-662 (FHOLStartingNomadFirstForumTest, both seats) and 717-727 (the ForumCell lambda of FHOLStartingNomadReplicatedTest).
void Test_CGNomadAges(FUnitTest& T)
{
	for (int Way = 0; Way < 2; Way++)
	{
		bool bNomad = Way == 1;
		FString Name = "standard";
		if (bNomad)
		{
			Name = "nomad";
		}
		FHOLGridContextView Grid = CGKYMakeNomadBuilders(bNomad);

		// cpp 655-656: Z opens page II at tier 0 for the Nomad only (ChosenAge 1 against INDEX_NONE = -1).
		FHOLGridKeyResultView PageTwo = HOLCommandGridKeys::PressKey(Grid, -1, 1);
		int ExpectedAge = -1;
		if (bNomad)
		{
			ExpectedAge = 1;
		}
		T.AssertEquals(ExpectedAge, PageTwo.ChosenAge, f"{Name}: Z opens the page Palier II at tier 0");
		T.AssertTrue(bNomad == (PageTwo.Target == EHOLGridKeyTargetKind::Age), f"{Name}: Z is an age key only when page II is open");

		// cpp 657-658: the forum key of page II builds the forum (Nomad only).
		FHOLGridKeyResultView KeyC = HOLCommandGridKeys::PressKey(Grid, 1, 10);
		bool bKeyCBuildsForum = KeyC.Target == EHOLGridKeyTargetKind::Build && KeyC.TypeName == n"MainCentre";
		T.AssertTrue(bNomad == (bKeyCBuildsForum), f"{Name}: the forum key of page II builds the forum");

		// cpp 659-660: II A (the Market) stays locked at tier 0, and the page stays chosen.
		FHOLGridKeyResultView KeyA = HOLCommandGridKeys::PressKey(Grid, 1, 0);
		T.AssertFalse(KeyA.Target == EHOLGridKeyTargetKind::Build, f"{Name}: II A (the Market) still locked at tier 0");
		T.AssertEquals(1, KeyA.ChosenAge, f"{Name}: II A leaves page II chosen");

		// cpp 661-662: a click on the forum cell of page II gives the same result as the key.
		FHOLGridKeyResultView Clicked = HOLCommandGridKeys::ClickAge(Grid, -1, 1, 10);
		bool bClickBuildsForum = Clicked.Target == EHOLGridKeyTargetKind::Build && Clicked.TypeName == n"MainCentre";
		T.AssertTrue(bNomad == (bClickBuildsForum), f"{Name}: a click on the forum cell of page II builds the forum");
		if (bNomad)
		{
			T.AssertEquals(KeyC.ChosenAge, Clicked.ChosenAge, "nomad: the click keeps the same age as the key");
		}

		// cpp 726: GetAgeBuilding of page II at the forum cell.
		FHOLGridCommandView Building = HOLCommandGridRules::GetAgeBuilding(Grid, 1, 10);
		bool bForumShown = Building.Command != EHOLGridCommandKind::None && Building.TypeName == n"MainCentre";
		T.AssertTrue(bNomad == (bForumShown), f"{Name}: the forum cell of page II shows the forum");
		T.AssertFalse(Building.bGreyed, f"{Name}: the forum cell is never greyed (always open or empty)");
		FHOLGridCommandView MarketCell = HOLCommandGridRules::GetAgeBuilding(Grid, 1, 0);
		T.AssertTrue(MarketCell.bGreyed, f"{Name}: the Market cell of page II is greyed at tier 0");
	}

	// The age closes after a placement, stays with Shift.
	int AfterPlaced = HOLCommandGridKeys::GetChosenAgeAfter(1, EHOLAgeEventKind::Placed);
	int AfterKept = HOLCommandGridKeys::GetChosenAgeAfter(1, EHOLAgeEventKind::PlacedKept);
	T.AssertEquals(-1, AfterPlaced, "placed: page II closes");
	T.AssertEquals(1, AfterKept, "placed with Shift: page II stays");
}

// HeroesOfLiberty.CommandGridKeys.PopulationBlocked
// Covers HOLProductionPopulationEndTests.cpp 156-157: 10 / 10 with a Peasant (1 place) = the queue says population full. The values of the
// C++ test (Used 10, Cap 10, Places 1) plus the neighbours that bracket the rule.
void Test_CGPopulationBlocked(FUnitTest& T)
{
	int Used = 10;
	int Cap = 10;
	int Places = 1;
	bool bFull = HOLCommandGridKeys::IsPopulationBlocked(Used, Cap, Places);
	T.AssertTrue(bFull, "10 / 10, one place wanted: population full");
	bool bRoomForOne = HOLCommandGridKeys::IsPopulationBlocked(Used - 1, Cap, Places);
	T.AssertFalse(bRoomForOne, "9 / 10, one place wanted: room");
	bool bNoRoomForTwo = HOLCommandGridKeys::IsPopulationBlocked(Used - 1, Cap, 2);
	T.AssertTrue(bNoRoomForTwo, "9 / 10, two places wanted: full");
	bool bZeroPlaces = HOLCommandGridKeys::IsPopulationBlocked(Used, Cap, 0);
	T.AssertFalse(bZeroPlaces, "10 / 10, no place wanted: not blocked");
	bool bOverCap = HOLCommandGridKeys::IsPopulationBlocked(Used + 1, Cap, Places);
	T.AssertTrue(bOverCap, "11 / 10 (cap lost): blocked");
}
