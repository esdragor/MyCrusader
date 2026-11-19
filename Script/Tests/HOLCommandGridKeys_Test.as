// MIG-C-CommandGrid, lot G2a (SAURON-1119x): the AGREEMENT test of the command grid's keys and feedback. NOT COMPILED when written.
// Two copies of the rule exist (the script one, Script/Player/HOLCommandGridKeys.as = namespace HOLCommandGridKeys, and the C++ one read through
// UHOLCommandGridReadLibrary = namespace HOLCommandGridRead): for every selection, age and cell the script must give what the C++ library gives.
// Sweeps: PressKey on 4096 selections (12 flags) x chosen age -1..3 x cells -1..12, ClickAge on the builders' grids, the stock / cost pairs of
// HasMissingResource / CheckBuild / CheckProduction, every feedback of Describe (kinds, resources, refusals 0..6, categories, blocks 0..9),
// the population, the chosen age after an event. Plus hard values copied from the C++ tests of the grid:
// Script/Tests/HOLCommandGrid_Test.as lines 211-236 (feedback), 842-880 (keys, ages), 773-776 (hero keys).
// To SEE RED (rouge a voir): in HOLCommandGridKeys.as PressKey, change `Result.ChosenAge = ChosenAge;` (the line after "An age chosen") to
// `Result.ChosenAge = -1;` (the PressKey sweep and Test_CGKYHardValues go red), or in Describe change "queue full" to "queue is full" (the Describe sweep
// and the hard values go red), or in FindMissingResource swap the Food and Wood checks.
// Names are prefixed CGKY: a global function of a test file is seen by no other file but its name must not clash.

bool CGKYSameKey(const FHOLGridKeyResultView& A, const FHOLGridKeyResultView& B)
{
	return A.Target == B.Target && A.ChosenAge == B.ChosenAge && A.TypeName == B.TypeName;
}

FString CGKYDescribeKey(const FHOLGridKeyResultView& Key)
{
	return f"target {int(Key.Target)} age {Key.ChosenAge} type {Key.TypeName}";
}

FHOLResources CGKYRes(int Food, int Wood, int Stone, int Gold)
{
	FHOLResources Out;
	Out.Food = Food;
	Out.Wood = Wood;
	Out.Stone = Stone;
	Out.Gold = Gold;
	return Out;
}

// A selection from a number (same recipe as CGRMakeContext of HOLCommandGridRules_Test.as, own copy).
FHOLGridContextView CGKYMakeContext(int Mask)
{
	FHOLGridContextView Context;
	Context.bOwnBuilding = (Mask & 1) != 0;
	Context.bBuildingFinished = (Mask & 2) != 0;
	Context.bBuildingGroup = (Mask & 4) != 0;
	Context.bForumDown = (Mask & 8) != 0;
	Context.bMarket = (Mask & 16) != 0;
	Context.bPrimaryForum = (Mask & 32) != 0;
	Context.bBuilders = (Mask & 64) != 0;
	Context.bOwnUnits = (Mask & 128) != 0;
	Context.bSoldiers = (Mask & 256) != 0;
	Context.bOnlyKings = (Mask & 512) != 0;
	Context.bHero = (Mask & 1024) != 0;
	Context.bRageUnit = (Mask & 2048) != 0;
	Context.bDeployables = Mask % 3 == 0;
	Context.bDeployedAny = Mask % 5 == 0;
	Context.HeroLevel = Mask % 2 == 0 ? 1 : 4;
	Context.ReachedAge = Mask % 4;
	Context.SpecializationTier = (Mask / 7) % 4;
	Context.SpecializationPendingOption = (Mask / 5) % 3 - 1;
	Context.SpecializationRemainingMs = (Mask / 3) % 2 == 0 ? 0 : 12345;
	int Lists = Mask % 4;
	if (Lists >= 1)
	{
		Context.Produces.Add(n"Peasant");
		Context.Produces.Add(n"Spearman");
	}
	if (Lists >= 2)
	{
		Context.Produces.Add(n"ManAtArms");
		Context.Produces.Add(n"Archer");
	}
	if (Mask % 7 == 0)
	{
		Context.HeroCooldownSeconds.Add(0);
		Context.HeroCooldownSeconds.Add(30);
		Context.HeroCooldownSeconds.Add(0);
	}
	if (Mask % 2 == 1 || Context.bBuilders)
	{
		Context = HOLCommandGridRead::WithTableBuildables(Context);
		if (Mask % 9 == 0 && Context.BuildableTypes.Num() > 0)
		{
			for (int I = 0; I < Context.BuildableTypes.Num(); I++)
			{
				Context.BuildableAlwaysOpen.Add(I == Context.BuildableTypes.Num() - 1);
			}
		}
	}
	return Context;
}

// The builders' grid of the C++ tests: the table's buildable types, nothing selected but the builders, the Barracks added at age II cell Q (cpp test 806-812).
FHOLGridContextView CGKYMakeBuilders()
{
	FHOLGridContextView Builders;
	Builders.bBuilders = true;
	Builders.bOwnUnits = true;
	Builders = HOLCommandGridRead::WithTableBuildables(Builders);
	Builders.BuildableTypes.Add(n"Barracks");
	Builders.BuildableCells.Add(4);
	Builders.BuildableAges.Add(1);
	return Builders;
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

// HeroesOfLiberty.CommandGridKeys.AgreementKeys
void Test_CGKYAgreementKeys(FUnitTest& T)
{
	int Mismatches = 0;
	FString First = "";
	int NumCells = HOLCommandGridRead::GetNumCells();
	for (int Mask = 0; Mask < 4096; Mask++)
	{
		FHOLGridContextView Context = CGKYMakeContext(Mask);
		for (int Chosen = -1; Chosen <= 3; Chosen++)
		{
			for (int Cell = -1; Cell <= NumCells; Cell++)
			{
				FHOLGridKeyResultView Script = HOLCommandGridKeys::PressKey(Context, Chosen, Cell);
				FHOLGridKeyResultView Native = HOLCommandGridRead::PressKey(Context, Chosen, Cell);
				if (!CGKYSameKey(Script, Native))
				{
					Mismatches++;
					if (First.IsEmpty())
					{
						First = f"PressKey mask {Mask} chosen {Chosen} cell {Cell}: script [{CGKYDescribeKey(Script)}] C++ [{CGKYDescribeKey(Native)}]";
					}
				}
			}
		}
		// The clicks: only where a builders' grid can answer (the others give the same empty answer, swept on a few masks).
		if (!HOLCommandGridRules::IsBuildGrid(Context) && Mask % 16 != 0)
		{
			continue;
		}
		for (int Chosen = -1; Chosen <= 3; Chosen++)
		{
			for (int Age = -1; Age <= HOLCommandGridRead::GetNumAges(); Age++)
			{
				for (int Cell = -1; Cell <= NumCells; Cell++)
				{
					FHOLGridKeyResultView Script = HOLCommandGridKeys::ClickAge(Context, Chosen, Age, Cell);
					FHOLGridKeyResultView Native = HOLCommandGridRead::ClickAge(Context, Chosen, Age, Cell);
					if (!CGKYSameKey(Script, Native))
					{
						Mismatches++;
						if (First.IsEmpty())
						{
							First = f"ClickAge mask {Mask} chosen {Chosen} age {Age} cell {Cell}: script [{CGKYDescribeKey(Script)}] C++ [{CGKYDescribeKey(Native)}]";
						}
					}
				}
			}
		}
	}

	// The builders' grid of the table, every age reached, every chosen age and cell (the buildings really exist here).
	FHOLGridContextView Builders = CGKYMakeBuilders();
	T.AssertTrue(Builders.BuildableTypes.Num() > 0, "the building table has buildable types");
	for (int Reached = 0; Reached < HOLCommandGridRead::GetNumAges(); Reached++)
	{
		Builders.ReachedAge = Reached;
		for (int Chosen = -1; Chosen <= HOLCommandGridRead::GetNumAges(); Chosen++)
		{
			for (int Cell = -1; Cell <= NumCells; Cell++)
			{
				FHOLGridKeyResultView Script = HOLCommandGridKeys::PressKey(Builders, Chosen, Cell);
				FHOLGridKeyResultView Native = HOLCommandGridRead::PressKey(Builders, Chosen, Cell);
				if (!CGKYSameKey(Script, Native))
				{
					Mismatches++;
					if (First.IsEmpty())
					{
						First = f"table PressKey reached {Reached} chosen {Chosen} cell {Cell}: script [{CGKYDescribeKey(Script)}] C++ [{CGKYDescribeKey(Native)}]";
					}
				}
			}
		}
	}

	// The chosen age after every event.
	for (int Event = 0; Event < 5; Event++)
	{
		for (int Chosen = -1; Chosen < 4; Chosen++)
		{
			if (HOLCommandGridKeys::GetChosenAgeAfter(Chosen, EHOLAgeEventKind(Event)) != HOLCommandGridRead::GetChosenAgeAfter(Chosen, EHOLAgeEventKind(Event)))
			{
				Mismatches++;
				if (First.IsEmpty())
				{
					First = f"GetChosenAgeAfter event {Event} chosen {Chosen}";
				}
			}
		}
	}
	T.AssertEquals(0, Mismatches, First);
}

// HeroesOfLiberty.CommandGridKeys.AgreementFeedback
void Test_CGKYAgreementFeedback(FUnitTest& T)
{
	int Mismatches = 0;
	FString First = "";
	TArray<int> Pool;
	Pool.Add(0);
	Pool.Add(10);
	Pool.Add(50);

	// HasMissingResource / GetMissingResource / CheckBuild: 81 stocks x 81 costs.
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
			FHOLFeedbackView ScriptBuild = HOLCommandGridKeys::CheckBuild(Stock, Cost);
			FHOLFeedbackView NativeBuild = HOLCommandGridRead::CheckBuild(Stock, Cost);
			bSame = bSame && ScriptBuild.Kind == NativeBuild.Kind && ScriptBuild.Resource == NativeBuild.Resource
				&& HOLCommandGridKeys::Blocks(ScriptBuild) == HOLCommandGridRead::Blocks(NativeBuild);
			if (!bSame)
			{
				Mismatches++;
				if (First.IsEmpty())
				{
					First = f"missing / CheckBuild stock {S} cost {C}";
				}
			}
		}
	}

	// CheckProduction: stocks and costs of 0 / 50, the queue empty / one short / full, four population cases.
	TArray<int> Used;
	Used.Add(3);
	Used.Add(10);
	Used.Add(9);
	Used.Add(9);
	TArray<int> Places;
	Places.Add(1);
	Places.Add(1);
	Places.Add(2);
	Places.Add(-3);
	for (int S = 0; S < 16; S++)
	{
		FHOLResources Stock = CGKYRes((S & 1) != 0 ? 50 : 0, (S & 2) != 0 ? 50 : 0, (S & 4) != 0 ? 50 : 0, (S & 8) != 0 ? 50 : 0);
		for (int C = 0; C < 16; C++)
		{
			FHOLResources Cost = CGKYRes((C & 1) != 0 ? 50 : 0, (C & 2) != 0 ? 50 : 0, (C & 4) != 0 ? 50 : 0, (C & 8) != 0 ? 50 : 0);
			for (int Queue = 0; Queue <= 6; Queue += 3)
			{
				for (int Pop = 0; Pop < 4; Pop++)
				{
					FHOLFeedbackView Script = HOLCommandGridKeys::CheckProduction(Stock, Cost, Queue, 5, Used[Pop], 10, Places[Pop]);
					FHOLFeedbackView Native = HOLCommandGridRead::CheckProduction(Stock, Cost, Queue, 5, Used[Pop], 10, Places[Pop]);
					if (Script.Kind != Native.Kind || Script.Resource != Native.Resource
						|| HOLCommandGridKeys::Blocks(Script) != HOLCommandGridRead::Blocks(Native)
						|| HOLCommandGridKeys::Describe(Script) != HOLCommandGridRead::Describe(Native))
					{
						Mismatches++;
						if (First.IsEmpty())
						{
							First = f"CheckProduction stock {S} cost {C} queue {Queue} pop {Pop}: script {HOLCommandGridKeys::Describe(Script)} C++ {HOLCommandGridRead::Describe(Native)}";
						}
					}
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

	// Describe and Blocks: every kind, resource, refusal (0..6), category and technology block (0..9).
	for (int Kind = 0; Kind < 7; Kind++)
	{
		for (int Resource = 0; Resource < 4; Resource++)
		{
			for (int Refusal = 0; Refusal <= 6; Refusal++)
			{
				for (int Category = 0; Category < 3; Category++)
				{
					for (int Block = 0; Block <= 9; Block++)
					{
						FHOLFeedbackView Feedback;
						Feedback.Kind = EHOLCommandFeedbackKind(Kind);
						Feedback.Resource = EHOLResource(Resource);
						Feedback.Refusal = Refusal;
						Feedback.Category = EHOLBuildingCategory(Category);
						Feedback.Block = Block;
						FString ScriptText = HOLCommandGridKeys::Describe(Feedback);
						FString NativeText = HOLCommandGridRead::Describe(Feedback);
						if (ScriptText != NativeText || HOLCommandGridKeys::Blocks(Feedback) != HOLCommandGridRead::Blocks(Feedback))
						{
							Mismatches++;
							if (First.IsEmpty())
							{
								First = f"Describe kind {Kind} res {Resource} refusal {Refusal} cat {Category} block {Block}: script [{ScriptText}] C++ [{NativeText}]";
							}
						}
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

	// The cell face and the category selection.
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
	for (int TypeCategory = 0; TypeCategory < 3; TypeCategory++)
	{
		for (int Category = 0; Category < 3; Category++)
		{
			if (HOLCommandGridKeys::IsCategorySelectedByCategory(EHOLBuildingCategory(TypeCategory), EHOLBuildingCategory(Category))
				!= HOLCommandGridRead::IsCategorySelectedByCategory(EHOLBuildingCategory(TypeCategory), EHOLBuildingCategory(Category)))
			{
				Mismatches++;
				if (First.IsEmpty())
				{
					First = f"IsCategorySelectedByCategory {TypeCategory} {Category}";
				}
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
