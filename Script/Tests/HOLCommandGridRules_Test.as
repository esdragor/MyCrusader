// MIG-C-CommandGrid, lot G1 (SAURON-1119t): the AGREEMENT test of the command grid's content. NOT COMPILED when written.
// Two copies of the rule exist (the script one, Script/Player/HOLCommandGridRules.as = namespace HOLCommandGridRules, and the C++ one read through
// UHOLCommandGridReadLibrary = namespace HOLCommandGridRead): for every selection and every cell the script rule must give what the C++ library gives.
// Sweeps: 4096 selections (12 flags) x 12 cells (+ out-of-grid cells), the builders' grid on every age and cell, every type of the building table
// (its Produces, its Researches, the Market), the trades, the ages, the cursor. Plus a few hard values (the numbers do not come from the C++ copy).
// To SEE RED (rouge a voir): in HOLCommandGridRules.as GetCommand, change `Result.bBuy = Cell < GridColumns;` to `Cell > GridColumns` (the sweep and
// Test_CGRHardValues both go red), or HeroAbilityBanner = 2 to 3, or the rage cell 2 to 3.
// Script tests of the same grid on the C++ library alone: Script/Tests/HOLCommandGrid_Test.as (untouched).

bool CGRSame(const FHOLGridCommandView& A, const FHOLGridCommandView& B)
{
	return A.Command == B.Command && A.TypeName == B.TypeName && A.Age == B.Age && A.bGreyed == B.bGreyed && A.Resource == B.Resource
		&& A.bBuy == B.bBuy && A.Ability == B.Ability && A.CooldownSeconds == B.CooldownSeconds && A.bPack == B.bPack && A.Tier == B.Tier
		&& A.Option == B.Option && A.RemainingMs == B.RemainingMs;
}

FString CGRDescribe(const FHOLGridCommandView& Command)
{
	return f"cmd {int(Command.Command)} type {Command.TypeName} age {Command.Age} greyed {Command.bGreyed} res {int(Command.Resource)} buy {Command.bBuy} ability {Command.Ability} cd {Command.CooldownSeconds} pack {Command.bPack} tier {Command.Tier} option {Command.Option} ms {Command.RemainingMs}";
}

// A selection from a number: 12 flags from its bits, the other facts from its residues, so that every combination meets every kind of list.
FHOLGridContextView CGRMakeContext(int Mask)
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
		Context.Produces.Add(n"Knight");
		Context.Produces.Add(n"Scout");
		Context.Produces.Add(n"Monk");
		Context.Produces.Add(n"Catapult");
	}
	if ((Mask / 4) % 3 >= 1)
	{
		Context.Researches.Add(n"PloughI");
		Context.Researches.Add(n"PloughII");
	}
	if ((Mask / 4) % 3 == 2)
	{
		Context.Researches.Add(n"WheelbarrowI");
		Context.Researches.Add(n"WheelbarrowII");
		Context.Researches.Add(n"ImprovedAlarmBell");
	}
	if (Mask % 7 == 0)
	{
		Context.HeroCooldownSeconds.Add(0);
		Context.HeroCooldownSeconds.Add(30);
		Context.HeroCooldownSeconds.Add(0);
	}
	if (Mask % 11 == 0)
	{
		Context.HeroCooldownSeconds.Add(0);
		Context.HeroCooldownSeconds.Add(0);
		Context.HeroCooldownSeconds.Add(5);
	}
	if (Mask % 13 == 0)
	{
		Context.HeroCooldownSeconds.Add(-4);
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

// HeroesOfLiberty.CommandGridRules.AgreementSelections
void Test_CGRAgreementSelections(FUnitTest& T)
{
	int Mismatches = 0;
	FString First = "";
	int NumCells = HOLCommandGridRead::GetNumCells();
	T.AssertEquals(NumCells, HOLCommandGridRules::GetNumCells(), "the grid has the same number of cells");
	for (int Mask = 0; Mask < 4096; Mask++)
	{
		FHOLGridContextView Context = CGRMakeContext(Mask);
		for (int Cell = -1; Cell <= NumCells; Cell++)
		{
			FHOLGridCommandView Script = HOLCommandGridRules::GetCommand(Context, Cell);
			FHOLGridCommandView Native = HOLCommandGridRead::GetCommand(Context, Cell);
			if (!CGRSame(Script, Native))
			{
				Mismatches++;
				if (First.IsEmpty())
				{
					First = f"GetCommand mask {Mask} cell {Cell}: script [{CGRDescribe(Script)}] C++ [{CGRDescribe(Native)}]";
				}
			}
			FHOLGridCommandView ScriptOrder = HOLCommandGridRules::GetOrder(Context, Cell);
			FHOLGridCommandView NativeOrder = HOLCommandGridRead::GetOrder(Context, Cell);
			if (!CGRSame(ScriptOrder, NativeOrder))
			{
				Mismatches++;
				if (First.IsEmpty())
				{
					First = f"GetOrder mask {Mask} cell {Cell}: script [{CGRDescribe(ScriptOrder)}] C++ [{CGRDescribe(NativeOrder)}]";
				}
			}
			FName ScriptReport = HOLCommandGridRules::GetReportName(Script);
			FName NativeReport = HOLCommandGridRead::GetReportName(Native);
			if (ScriptReport != NativeReport)
			{
				Mismatches++;
				if (First.IsEmpty())
				{
					First = f"GetReportName mask {Mask} cell {Cell}: script {ScriptReport} C++ {NativeReport}";
				}
			}
		}
		if (HOLCommandGridRules::IsBuildGrid(Context) != HOLCommandGridRead::IsBuildGrid(Context))
		{
			Mismatches++;
			if (First.IsEmpty())
			{
				First = f"IsBuildGrid mask {Mask}";
			}
		}
		if (Mask % 4 != 0 && !HOLCommandGridRules::IsBuildGrid(Context))
		{
			continue;
		}
		for (int Age = -1; Age <= HOLCommandGridRead::GetNumAges(); Age++)
		{
			if (HOLCommandGridRules::IsAgeReached(Context, Age) != HOLCommandGridRead::IsAgeReached(Context, Age)
				|| HOLCommandGridRules::IsAgeOpen(Context, Age) != HOLCommandGridRead::IsAgeOpen(Context, Age)
				|| !Math::IsNearlyEqual(HOLCommandGridRules::GetAgePanelOpacity(Context, Age), HOLCommandGridRead::GetAgePanelOpacity(Context, Age), 0.0001))
			{
				Mismatches++;
				if (First.IsEmpty())
				{
					First = f"age state mask {Mask} age {Age}";
				}
			}
			for (int Cell = -1; Cell <= NumCells; Cell++)
			{
				FHOLGridCommandView Script = HOLCommandGridRules::GetAgeBuilding(Context, Age, Cell);
				FHOLGridCommandView Native = HOLCommandGridRead::GetAgeBuilding(Context, Age, Cell);
				if (!CGRSame(Script, Native))
				{
					Mismatches++;
					if (First.IsEmpty())
					{
						First = f"GetAgeBuilding mask {Mask} age {Age} cell {Cell}: script [{CGRDescribe(Script)}] C++ [{CGRDescribe(Native)}]";
					}
				}
			}
		}
	}
	T.AssertEquals(0, Mismatches, First);
}

// HeroesOfLiberty.CommandGridRules.AgreementTables
void Test_CGRAgreementTables(FUnitTest& T)
{
	int Mismatches = 0;
	FString First = "";
	int NumCells = HOLCommandGridRead::GetNumCells();

	// The builders' grid of the building table: every age reached, every age, every cell.
	FHOLGridContextView Builders;
	Builders.bBuilders = true;
	Builders.bOwnUnits = true;
	Builders = HOLCommandGridRead::WithTableBuildables(Builders);
	T.AssertTrue(Builders.BuildableTypes.Num() > 0, "the building table has buildable types");
	for (int Reached = 0; Reached < HOLCommandGridRead::GetNumAges(); Reached++)
	{
		Builders.ReachedAge = Reached;
		for (int Age = -1; Age <= HOLCommandGridRead::GetNumAges(); Age++)
		{
			for (int Cell = -1; Cell <= NumCells; Cell++)
			{
				FHOLGridCommandView Script = HOLCommandGridRules::GetAgeBuilding(Builders, Age, Cell);
				FHOLGridCommandView Native = HOLCommandGridRead::GetAgeBuilding(Builders, Age, Cell);
				if (!CGRSame(Script, Native))
				{
					Mismatches++;
					if (First.IsEmpty())
					{
						First = f"table GetAgeBuilding reached {Reached} age {Age} cell {Cell}: script [{CGRDescribe(Script)}] C++ [{CGRDescribe(Native)}]";
					}
				}
			}
		}
	}

	// Every building type of the table, own and finished or a site, as a Market or not: its units (Produces) and its technologies (Researches).
	int Selections = 0;
	for (int Type = 0; Type < Builders.BuildableTypes.Num(); Type++)
	{
		FName TypeName = Builders.BuildableTypes[Type];
		for (int Pass = 0; Pass < 8; Pass++)
		{
			FHOLGridContextView Building;
			Building.bOwnBuilding = true;
			Building.bBuildingFinished = (Pass & 1) == 0;
			Building.bPrimaryForum = (Pass & 2) != 0;
			Building.bMarket = TypeName == n"Market" || (Pass & 4) != 0;
			Building.Produces = HOLCommandGridRead::GetProducesOf(TypeName);
			Building.Researches = HOLCommandGridRead::GetBuildingResearches(TypeName);
			Building.SpecializationTier = (Pass & 2) != 0 ? 1 : 0;
			Building.SpecializationPendingOption = Pass % 3 - 1;
			Building.SpecializationRemainingMs = 4000;
			Selections++;
			for (int Cell = -1; Cell <= NumCells; Cell++)
			{
				FHOLGridCommandView Script = HOLCommandGridRules::GetCommand(Building, Cell);
				FHOLGridCommandView Native = HOLCommandGridRead::GetCommand(Building, Cell);
				if (!CGRSame(Script, Native))
				{
					Mismatches++;
					if (First.IsEmpty())
					{
						First = f"table building {TypeName} pass {Pass} cell {Cell}: script [{CGRDescribe(Script)}] C++ [{CGRDescribe(Native)}]";
					}
				}
			}
		}
	}
	T.AssertTrue(Selections >= 8, "at least one building of the table was swept");

	// The trades: every good, buying and selling, the cell and the gain (text and logo), the amounts around zero.
	for (int Good = 0; Good < 4; Good++)
	{
		EHOLResource Resource = EHOLResource(Good);
		for (int Pass = 0; Pass < 2; Pass++)
		{
			bool bBuy = Pass == 0;
			if (HOLCommandGridRules::GetTradeCell(Resource, bBuy) != HOLCommandGridRead::GetTradeCell(Resource, bBuy))
			{
				Mismatches++;
				if (First.IsEmpty())
				{
					First = f"GetTradeCell good {Good} buy {bBuy}";
				}
			}
			for (int Received = -3; Received <= 250; Received += 11)
			{
				FHOLTradeGainView ScriptGain = HOLCommandGridRules::GetTradeGain(Resource, bBuy, Received);
				FHOLTradeGainView NativeGain = HOLCommandGridRead::GetTradeGain(Resource, bBuy, Received);
				if (ScriptGain.Amount != NativeGain.Amount || ScriptGain.Logo != NativeGain.Logo
					|| HOLCommandGridRules::GetTradeGainText(ScriptGain) != HOLCommandGridRead::GetTradeGainText(NativeGain))
				{
					Mismatches++;
					if (First.IsEmpty())
					{
						First = f"GetTradeGain good {Good} buy {bBuy} received {Received}";
					}
				}
			}
		}
	}

	// The categories' rows, the hero cells, the rage cell, the cursor, the constants.
	if (HOLCommandGridRules::GetCategoryRow(EHOLBuildingCategory::Economic) != HOLCommandGridRead::GetCategoryRow(EHOLBuildingCategory::Economic)
		|| HOLCommandGridRules::GetCategoryRow(EHOLBuildingCategory::Military) != HOLCommandGridRead::GetCategoryRow(EHOLBuildingCategory::Military)
		|| HOLCommandGridRules::GetCategoryRow(EHOLBuildingCategory::Technological) != HOLCommandGridRead::GetCategoryRow(EHOLBuildingCategory::Technological))
	{
		Mismatches++;
		if (First.IsEmpty())
		{
			First = "GetCategoryRow";
		}
	}
	for (int Ability = -1; Ability <= 3; Ability++)
	{
		if (HOLCommandGridRules::GetHeroAbilityCell(Ability) != HOLCommandGridRead::GetHeroAbilityCell(Ability))
		{
			Mismatches++;
			if (First.IsEmpty())
			{
				First = f"GetHeroAbilityCell {Ability}";
			}
		}
	}
	T.AssertEquals(1, HOLCommandGridRead::GetHeroAbilityCloseRanks(), "the ability numbers the script rule assumes: the cry is 1");
	T.AssertEquals(2, HOLCommandGridRead::GetHeroAbilityBanner(), "the ability numbers the script rule assumes: the banner is 2");
	if (HOLCommandGridRules::GetRageCell() != HOLCommandGridRead::GetRageCell())
	{
		Mismatches++;
		if (First.IsEmpty())
		{
			First = "GetRageCell";
		}
	}
	for (int Pass = 0; Pass < 4; Pass++)
	{
		bool bArmed = (Pass & 1) != 0;
		bool bPlacing = (Pass & 2) != 0;
		bool bScriptAttack = HOLCommandGridRules::IsAttackCursor(bArmed, bPlacing);
		bool bNativeAttack = HOLCommandGridRead::GetMouseCursor(bArmed, bPlacing) == HOLCommandGridRead::GetCursorCrosshairs();
		if (bScriptAttack != bNativeAttack)
		{
			Mismatches++;
			if (First.IsEmpty())
			{
				First = f"cursor armed {bArmed} placing {bPlacing}";
			}
		}
	}
	if (HOLCommandGridRules::GetGridColumns() != HOLCommandGridRead::GetGridColumns() || HOLCommandGridRules::GetCancelCell() != HOLCommandGridRead::GetCancelCell()
		|| HOLCommandGridRules::GetResearchFirstCell() != HOLCommandGridRead::GetResearchFirstCell() || HOLCommandGridRules::GetNumAges() != HOLCommandGridRead::GetNumAges()
		|| HOLCommandGridRules::GetAttackMoveCell() != HOLCommandGridRead::GetAttackMoveCell() || HOLCommandGridRules::GetStopCell() != HOLCommandGridRead::GetStopCell()
		|| HOLCommandGridRules::GetHoldCell() != HOLCommandGridRead::GetHoldCell() || HOLCommandGridRules::GetDeployCell() != HOLCommandGridRead::GetDeployCell())
	{
		Mismatches++;
		if (First.IsEmpty())
		{
			First = "a constant of the grid differs";
		}
	}
	T.AssertEquals(0, Mismatches, First);
}

// HeroesOfLiberty.CommandGridRules.HardValues
void Test_CGRHardValues(FUnitTest& T)
{
	// The Market: purchases A Z E on the top row, sales Q S D on the second, nothing on the 4th column nor on the third row.
	FHOLGridContextView Market;
	Market.bOwnBuilding = true;
	Market.bBuildingFinished = true;
	Market.bMarket = true;
	FHOLGridCommandView Buy = HOLCommandGridRules::GetCommand(Market, 0);
	T.AssertTrue(Buy.Command == EHOLGridCommandKind::Trade && Buy.Resource == EHOLResource::Food && Buy.bBuy, "Market A: buy Food");
	FHOLGridCommandView Sell = HOLCommandGridRules::GetCommand(Market, 5);
	T.AssertTrue(Sell.Command == EHOLGridCommandKind::Trade && Sell.Resource == EHOLResource::Wood && !Sell.bBuy, "Market S: sell Wood");
	T.AssertTrue(HOLCommandGridRules::GetCommand(Market, 3).Command == EHOLGridCommandKind::None, "Market R: empty (a 4th good does not exist)");
	T.AssertTrue(HOLCommandGridRules::GetCommand(Market, 8).Command == EHOLGridCommandKind::None, "Market W: empty (third row)");
	T.AssertEquals(2, HOLCommandGridRules::GetTradeCell(EHOLResource::Stone, true), "buy Stone: cell E");
	T.AssertEquals(6, HOLCommandGridRules::GetTradeCell(EHOLResource::Stone, false), "sell Stone: cell D");
	T.AssertEquals(-1, HOLCommandGridRules::GetTradeCell(EHOLResource::Gold, true), "Gold is not a traded good");
	FHOLTradeGainView Gain = HOLCommandGridRules::GetTradeGain(EHOLResource::Stone, false, -7);
	T.AssertTrue(Gain.Amount == 0 && Gain.Logo == EHOLResource::Gold, "selling: gold logo, a negative amount counts 0");
	T.AssertTrue(HOLCommandGridRules::GetTradeGainText(HOLCommandGridRules::GetTradeGain(EHOLResource::Wood, true, 12)) == "+12", "gain text +12");

	// A site: only its cancel cell (V, cell 11), and not for a group; a forum down: nothing.
	FHOLGridContextView Site;
	Site.bOwnBuilding = true;
	T.AssertTrue(HOLCommandGridRules::GetCommand(Site, 11).Command == EHOLGridCommandKind::CancelConstruction, "site: V cancels the construction");
	Site.bBuildingGroup = true;
	T.AssertTrue(HOLCommandGridRules::GetCommand(Site, 11).Command == EHOLGridCommandKind::None, "a group of sites: no cancel");

	// The hero: level 1, the cry ready on A, the banner greyed on Z; cooling down: greyed with its seconds.
	FHOLGridContextView Hero;
	Hero.bHero = true;
	Hero.HeroLevel = 1;
	FHOLGridCommandView Cry = HOLCommandGridRules::GetCommand(Hero, 0);
	T.AssertTrue(Cry.Command == EHOLGridCommandKind::HeroAbility && Cry.Ability == 1 && !Cry.bGreyed && Cry.CooldownSeconds == 0, "hero level 1: A the cry, ready");
	T.AssertTrue(HOLCommandGridRules::GetCommand(Hero, 1).bGreyed, "hero level 1: Z the banner, greyed");
	Hero.HeroLevel = 4;
	Hero.HeroCooldownSeconds.Add(0);
	Hero.HeroCooldownSeconds.Add(42);
	Hero.HeroCooldownSeconds.Add(0);
	FHOLGridCommandView Cooling = HOLCommandGridRules::GetCommand(Hero, 0);
	T.AssertTrue(Cooling.bGreyed && Cooling.CooldownSeconds == 42, "hero: the cry cooling down, greyed, 42 s");
	T.AssertFalse(HOLCommandGridRules::GetCommand(Hero, 1).bGreyed, "hero level 4: the banner ready, its cool-down is its own");

	// The rage on E; the orders Q S V D.
	FHOLGridContextView Rage;
	Rage.bRageUnit = true;
	T.AssertTrue(HOLCommandGridRules::GetCommand(Rage, 2).Command == EHOLGridCommandKind::NecroticRage, "rage unit: E the rage");
	FHOLGridContextView Soldiers;
	Soldiers.bOwnUnits = true;
	Soldiers.bSoldiers = true;
	Soldiers.bDeployables = true;
	Soldiers.bDeployedAny = true;
	T.AssertTrue(HOLCommandGridRules::GetOrder(Soldiers, 4).Command == EHOLGridCommandKind::AttackMove, "soldiers: Q attack-move");
	T.AssertTrue(HOLCommandGridRules::GetOrder(Soldiers, 5).Command == EHOLGridCommandKind::Stop, "soldiers: S stop");
	T.AssertTrue(HOLCommandGridRules::GetOrder(Soldiers, 11).Command == EHOLGridCommandKind::Hold, "soldiers: V hold");
	FHOLGridCommandView Deploy = HOLCommandGridRules::GetOrder(Soldiers, 6);
	T.AssertTrue(Deploy.Command == EHOLGridCommandKind::Deploy && Deploy.bPack, "soldiers with deployed machines: D packs");
	Soldiers.bOnlyKings = true;
	T.AssertTrue(HOLCommandGridRules::GetOrder(Soldiers, 4).Command == EHOLGridCommandKind::None && HOLCommandGridRules::GetOrder(Soldiers, 11).Command == EHOLGridCommandKind::None, "kings alone: no attack-move, no hold");
	T.AssertTrue(HOLCommandGridRules::GetOrder(Soldiers, 5).Command == EHOLGridCommandKind::Stop, "kings alone: stop stays");

	// The specialization cells (default cvar 6 and 7): the running option cancels with its time left, the other starts; the name for the report.
	FHOLGridContextView Forum;
	Forum.bOwnBuilding = true;
	Forum.bBuildingFinished = true;
	Forum.bPrimaryForum = true;
	Forum.SpecializationTier = 2;
	Forum.SpecializationPendingOption = 1;
	Forum.SpecializationRemainingMs = 5000;
	FHOLGridCommandView OptionA = HOLCommandGridRules::GetCommand(Forum, HOLCommandGridRead::GetSpecializationCell(0));
	FHOLGridCommandView OptionB = HOLCommandGridRules::GetCommand(Forum, HOLCommandGridRead::GetSpecializationCell(1));
	T.AssertTrue(OptionA.Command == EHOLGridCommandKind::Specialize && OptionA.Tier == 2 && OptionA.Option == 0 && OptionA.RemainingMs == 0, "forum: option A starts");
	T.AssertTrue(OptionB.Command == EHOLGridCommandKind::CancelSpecialization && OptionB.Option == 1 && OptionB.RemainingMs == 5000, "forum: option B (running) cancels, 5000 ms left");
	T.AssertTrue(HOLCommandGridRules::GetReportName(OptionA) == n"SpecializeT2A", "report name SpecializeT2A");
	T.AssertTrue(HOLCommandGridRules::GetReportName(OptionB) == n"CancelSpecializationT2B", "report name CancelSpecializationT2B");

	// The cursor and the ages.
	T.AssertTrue(HOLCommandGridRules::IsAttackCursor(true, false), "Q armed: the attack cursor");
	T.AssertFalse(HOLCommandGridRules::IsAttackCursor(true, true), "placement wins over an armed Q");
	FHOLGridContextView Builders;
	Builders.bBuilders = true;
	T.AssertFalse(HOLCommandGridRules::IsAgeReached(Builders, 1), "age II not reached at the start");
	T.AssertTrue(Math::IsNearlyEqual(HOLCommandGridRules::GetAgePanelOpacity(Builders, 1), 0.55, 0.0001), "a page not open: faded to 0.55");
	T.AssertTrue(Math::IsNearlyEqual(HOLCommandGridRules::GetAgePanelOpacity(Builders, 0), 1.0, 0.0001), "age I: the page whole");
}

// MIG-C-CommandGrid, lot L1a (GALADRIEL-1119cc): the grid extracts of the C++ group-A tests, as script tests on the SCRIPT rule alone (no C++ copy read, no *Answer
// called): the same inputs and the same expected values as HOLRegicideTests.cpp:288-299, HOLVictoryTests.cpp:1197-1204, HOLDamnedWarriorTests.cpp:230-235.
// NOT COMPILED when written. To SEE RED: see the FALSIFICATION of GALADRIEL-1119cc.md (one wrong expected value per test).

// HeroesOfLiberty.CommandGridRules.KingsOnly (HOLRegicideTests.cpp:288-299)
void Test_CGKingsOnly(FUnitTest& T)
{
	FHOLGridContextView Kings;
	Kings.bOwnUnits = true;
	Kings.bSoldiers = true;
	Kings.bOnlyKings = true;
	int AttackMoveCell = HOLCommandGridRules::GetAttackMoveCell();
	int StopCell = HOLCommandGridRules::GetStopCell();
	int HoldCell = HOLCommandGridRules::GetHoldCell();
	FHOLGridCommandView KingsAttackMove = HOLCommandGridRules::GetOrder(Kings, AttackMoveCell);
	FHOLGridCommandView KingsStop = HOLCommandGridRules::GetOrder(Kings, StopCell);
	FHOLGridCommandView KingsHold = HOLCommandGridRules::GetOrder(Kings, HoldCell);
	bool KingsBuildGrid = HOLCommandGridRules::IsBuildGrid(Kings);
	T.AssertTrue(KingsAttackMove.Command == EHOLGridCommandKind::None, "Kings alone: no attack-move");
	T.AssertTrue(KingsStop.Command == EHOLGridCommandKind::Stop, "Kings alone: stop");
	T.AssertTrue(KingsHold.Command == EHOLGridCommandKind::None, "Kings alone: no hold (it would take on enemies in reach)");
	T.AssertFalse(KingsBuildGrid, "Kings alone: no builders' grid");

	FHOLGridContextView Mixed = Kings;
	Mixed.bOnlyKings = false;
	FHOLGridCommandView MixedAttackMove = HOLCommandGridRules::GetOrder(Mixed, AttackMoveCell);
	FHOLGridCommandView MixedHold = HOLCommandGridRules::GetOrder(Mixed, HoldCell);
	T.AssertTrue(MixedAttackMove.Command == EHOLGridCommandKind::AttackMove, "A King with soldiers: the attack-move stays");
	T.AssertTrue(MixedHold.Command == EHOLGridCommandKind::Hold, "A King with soldiers: the hold stays");
}

// HeroesOfLiberty.CommandGridRules.ForumDown (HOLVictoryTests.cpp:1197-1204)
void Test_CGForumDown(FUnitTest& T)
{
	FHOLGridContextView Grid;
	Grid.bOwnBuilding = true;
	Grid.bBuildingFinished = true;
	Grid.Produces.Add(n"Peasant");
	FHOLGridCommandView Standing = HOLCommandGridRules::GetCommand(Grid, 0);
	T.AssertTrue(Standing.Command == EHOLGridCommandKind::Produce, "a forum standing produces");
	Grid.bForumDown = true;
	int CancelCell = HOLCommandGridRules::GetCancelCell();
	FHOLGridCommandView DownFirst = HOLCommandGridRules::GetCommand(Grid, 0);
	FHOLGridCommandView DownCancel = HOLCommandGridRules::GetCommand(Grid, CancelCell);
	T.AssertTrue(DownFirst.Command == EHOLGridCommandKind::None, "a forum down: no production");
	T.AssertTrue(DownCancel.Command == EHOLGridCommandKind::None, "a forum down: nothing to cancel");
}

// HeroesOfLiberty.CommandGridRules.RageButton (HOLDamnedWarriorTests.cpp:230-235)
void Test_CGRageButton(FUnitTest& T)
{
	FHOLGridContextView Selection;
	Selection.bOwnUnits = true;
	Selection.bRageUnit = true;
	int RageCell = HOLCommandGridRules::GetRageCell();
	FHOLGridCommandView WithRage = HOLCommandGridRules::GetCommand(Selection, RageCell);
	T.AssertTrue(WithRage.Command == EHOLGridCommandKind::NecroticRage, "the grid shows the rage button for a selection with a Damned Warrior");
	Selection.bRageUnit = false;
	FHOLGridCommandView WithoutRage = HOLCommandGridRules::GetCommand(Selection, RageCell);
	T.AssertTrue(WithoutRage.Command != EHOLGridCommandKind::NecroticRage, "... and none for another selection");
}
