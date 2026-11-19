// MIG-C-CommandGrid, lot L3b-bis (HALDIR-1119ce, D836, D839): the SWEEPS of the 3 x 4 command grid, written as PROPERTIES in clear. NOT COMPILED when written.
// Why: lot L3b removed the script-vs-C++ agreement sweeps (one source). Three of them had no equivalent in clear: Describe on every combination,
// GetCommand on every building of the table, PressKey / ClickAge on 4096 selections. Each test below walks the same inputs as before and checks
// invariants that read as sentences (no cell outside the 3 x 4 grid, one key answers one cell, an age above the tier never opens, ...), never a
// second copy of the rule's formula. A few values come from the GDD or the tables (grid 3 x 4, orders Q S D V, trades A Z E / Q S D, the four resources).
// The rules read are the SCRIPT ones only: HOLCommandGridRules / HOLCommandGridKeys; the table lookups are the library's (HOLCommandGridRead).
// To SEE RED (rouge a voir), one falsification per test, listed in Docs/Rapports/HALDIR-1119ce.md (file, line, before -> after).
// Names are prefixed CGSW: a global function of a test file is seen by no other file but its name must not clash.

// The automation log does not show the message of an AssertTrue: a failing case is written to the game log (normal level, not Error or Warning),
// prefix "CGSW: <test>: <inputs and value read>", at most 20 lines per test (Bad / Fails count the cases of that test only).
const int CGSWMaxLogLines = 20;

void CGSWNote(int& Bad, FString& First, const FString& Test, const FString& Message)
{
	Bad++;
	if (Bad <= CGSWMaxLogLines)
	{
		Log(n"LogHOLMatch", f"CGSW: {Test}: {Message}");
	}
	if (First.IsEmpty())
	{
		First = Message;
	}
}

// An assertion inside a loop whose message the automation log hides: same line in the game log when it fails.
void CGSWCheck(FUnitTest& T, int& Fails, const FString& Test, bool bOk, const FString& Message)
{
	if (!bOk)
	{
		Fails++;
		if (Fails <= CGSWMaxLogLines)
		{
			Log(n"LogHOLMatch", f"CGSW: {Test}: {Message}");
		}
	}
	T.AssertTrue(bOk, Message);
}

// A selection from a number (the 12 flags of the grid, the other fields derived from it); the builders get the table's buildable types.
FHOLGridContextView CGSWMakeContext(int Mask)
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
	if (Mask % 3 == 1)
	{
		Context.Researches.Add(n"TechnologyOne");
		Context.Researches.Add(n"TechnologyTwo");
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

// The builders alone, nothing else selected (the GDD's builders' grid).
bool CGSWIsBuildersAlone(const FHOLGridContextView& Context)
{
	return Context.bBuilders && !Context.bSoldiers && !Context.bOwnBuilding;
}

// The index of the buildable type at this age and cell, or -1; read straight from the context's arrays.
int CGSWFindBuildable(const FHOLGridContextView& Context, FName TypeName, int Age, int Cell)
{
	int Count = Math::Min(Math::Min(Context.BuildableTypes.Num(), Context.BuildableCells.Num()), Context.BuildableAges.Num());
	for (int I = 0; I < Count; I++)
	{
		if (Context.BuildableTypes[I] == TypeName && Context.BuildableAges[I] == Age && Context.BuildableCells[I] == Cell)
		{
			return I;
		}
	}
	return -1;
}

bool CGSWIsAlwaysOpenIndex(const FHOLGridContextView& Context, int Index)
{
	return Index >= 0 && Index < Context.BuildableAlwaysOpen.Num() && Context.BuildableAlwaysOpen[Index];
}

// An age page can be chosen when the tier reached it, or when one of its buildings is always open (a Nomad's first forum, D797).
bool CGSWAgePageOpen(const FHOLGridContextView& Context, int Age)
{
	if (Age < 0 || Age >= 4)
	{
		return false;
	}
	if (Age <= Context.ReachedAge)
	{
		return true;
	}
	int Count = Math::Min(Context.BuildableAges.Num(), Context.BuildableAlwaysOpen.Num());
	for (int I = 0; I < Count; I++)
	{
		if (Context.BuildableAges[I] == Age && Context.BuildableAlwaysOpen[I])
		{
			return true;
		}
	}
	return false;
}

// The place of a key answer in words (built only when a check fails: the sweeps run about a million keys). Age -9: PressKey, no clicked age.
FString CGSWWhere(int Mask, int Chosen, int Age, int Cell)
{
	return Age == -9 ? f"selection {Mask} chosen age {Chosen} key {Cell}" : f"selection {Mask} chosen age {Chosen} click age {Age} cell {Cell}";
}

// The properties of one key answer, shared by PressKey and ClickAge: what the answer may and may not be.
void CGSWCheckKeyAnswer(const FHOLGridContextView& Context, const FHOLGridKeyResultView& Key, int ChosenAge, int Cell, int Mask, int Age, const FString& Test, int& Bad, FString& First)
{
	FName NoName;
	if (Key.ChosenAge < -1 || Key.ChosenAge >= 4)
	{
		CGSWNote(Bad, First, Test, f"{CGSWWhere(Mask, ChosenAge, Age, Cell)}: the chosen age {Key.ChosenAge} is outside -1..3");
	}
	if (Key.Target == EHOLGridKeyTargetKind::Age)
	{
		if (Key.ChosenAge < 0 || !CGSWAgePageOpen(Context, Key.ChosenAge))
		{
			CGSWNote(Bad, First, Test, f"{CGSWWhere(Mask, ChosenAge, Age, Cell)}: an age above the tier was opened (age {Key.ChosenAge}, reached {Context.ReachedAge})");
		}
		if (!CGSWIsBuildersAlone(Context))
		{
			CGSWNote(Bad, First, Test, f"{CGSWWhere(Mask, ChosenAge, Age, Cell)}: an age was chosen while the builders are not alone");
		}
	}
	if (Key.Target == EHOLGridKeyTargetKind::Build)
	{
		if (Key.TypeName == NoName)
		{
			CGSWNote(Bad, First, Test, f"{CGSWWhere(Mask, ChosenAge, Age, Cell)}: a build key without a building name");
		}
		else if (!CGSWIsBuildersAlone(Context))
		{
			CGSWNote(Bad, First, Test, f"{CGSWWhere(Mask, ChosenAge, Age, Cell)}: a build key while the builders are not alone");
		}
		else
		{
			int Index = CGSWFindBuildable(Context, Key.TypeName, Key.ChosenAge, Cell);
			if (Index < 0)
			{
				CGSWNote(Bad, First, Test, f"{CGSWWhere(Mask, ChosenAge, Age, Cell)}: {Key.TypeName} is not a building of age {Key.ChosenAge} at cell {Cell}");
			}
			else if (Key.ChosenAge > Context.ReachedAge && !CGSWIsAlwaysOpenIndex(Context, Index))
			{
				CGSWNote(Bad, First, Test, f"{CGSWWhere(Mask, ChosenAge, Age, Cell)}: {Key.TypeName} of age {Key.ChosenAge} was built above the tier {Context.ReachedAge}");
			}
		}
	}
	if (Key.Target == EHOLGridKeyTargetKind::Orders && Context.bOwnBuilding)
	{
		CGSWNote(Bad, First, Test, f"{CGSWWhere(Mask, ChosenAge, Age, Cell)}: a key of a selected building went to the orders");
	}
}

// ---------------------------------------------------------------------------------------------------------------------------------------------

// GetCommand / GetOrder on 4096 selections x cells -1..12 (was Test_CGRAgreementSelections).
void Test_CGSWCommandCells(FUnitTest& T)
{
	// The grid and its panels in clear (GDD: 3 rows x 4 columns; orders Q attack-move, S stop, D deploy, V hold; technologies W X C V; cancel on V).
	T.AssertEquals(HOLCommandGridRead::GetNumCells(), 12, "the grid has 12 cells (3 x 4)");
	T.AssertEquals(HOLCommandGridRead::GetGridColumns(), 4, "the grid has 4 columns");
	T.AssertEquals(HOLCommandGridRead::GetAttackMoveCell(), 4, "attack-move is Q (cell 4)");
	T.AssertEquals(HOLCommandGridRead::GetStopCell(), 5, "stop is S (cell 5)");
	T.AssertEquals(HOLCommandGridRead::GetDeployCell(), 6, "deploy is D (cell 6)");
	T.AssertEquals(HOLCommandGridRead::GetHoldCell(), 11, "hold is V (cell 11)");
	T.AssertEquals(HOLCommandGridRead::GetResearchFirstCell(), 8, "the technologies start at W (cell 8)");
	T.AssertEquals(HOLCommandGridRead::GetCancelCell(), 11, "the cancel of a site is V (cell 11)");

	int Bad = 0;
	FString First;
	int Selections = 0;
	for (int Mask = 0; Mask < 4096; Mask++)
	{
		FHOLGridContextView C = CGSWMakeContext(Mask);
		Selections++;
		for (int Cell = -1; Cell <= 12; Cell++)
		{
			FHOLGridCommandView Cmd = HOLCommandGridRules::GetCommand(C, Cell);
			FHOLGridCommandView Ord = HOLCommandGridRules::GetOrder(C, Cell);
			FString Where = f"selection {Mask} cell {Cell}";
			bool bInGrid = Cell >= 0 && Cell < 12;
			if (!bInGrid && (Cmd.Command != EHOLGridCommandKind::None || Ord.Command != EHOLGridCommandKind::None))
			{
				CGSWNote(Bad, First, "Test_CGSWCommandCells", f"{Where}: a command outside the 3 x 4 grid");
			}
			if (C.bOwnBuilding && C.bForumDown && Cmd.Command != EHOLGridCommandKind::None)
			{
				CGSWNote(Bad, First, "Test_CGSWCommandCells", f"{Where}: a main forum that is down gave a command");
			}
			// A building under construction produces nothing; its site is the only thing to cancel, one at a time.
			if (C.bOwnBuilding && !C.bBuildingFinished && Cmd.Command != EHOLGridCommandKind::None)
			{
				if (Cmd.Command != EHOLGridCommandKind::CancelConstruction || C.bBuildingGroup || Cell != 11)
				{
					CGSWNote(Bad, First, "Test_CGSWCommandCells", f"{Where}: a site gave {int(Cmd.Command)} (only the cancel of one site, on V, is allowed)");
				}
			}
			if (Cmd.Command == EHOLGridCommandKind::Produce)
			{
				if (!C.bOwnBuilding || !C.bBuildingFinished || C.bMarket || Cell >= C.Produces.Num() || Cmd.TypeName != C.Produces[Cell])
				{
					CGSWNote(Bad, First, "Test_CGSWCommandCells", f"{Where}: a Produce that is not the building's unit of that key ({Cmd.TypeName})");
				}
			}
			if (Cmd.Command == EHOLGridCommandKind::Research)
			{
				if (!C.bOwnBuilding || !C.bBuildingFinished || C.bMarket || Cell < 8 || Cell - 8 >= C.Researches.Num() || Cmd.TypeName != C.Researches[Cell - 8])
				{
					CGSWNote(Bad, First, "Test_CGSWCommandCells", f"{Where}: a Research that is not the technology of that key ({Cmd.TypeName})");
				}
			}
			if (Cmd.Command == EHOLGridCommandKind::Trade)
			{
				// Purchases on the top row A Z E, sales on the second row Q S D; R and F trade nothing.
				if (!C.bOwnBuilding || !C.bBuildingFinished || !C.bMarket || Cell >= 8 || Cell % 4 == 3
					|| Cmd.bBuy != (Cell < 4) || int(Cmd.Resource) != Cell % 4)
				{
					CGSWNote(Bad, First, "Test_CGSWCommandCells", f"{Where}: a Trade out of place (buy {Cmd.bBuy}, resource {int(Cmd.Resource)})");
				}
			}
			if (C.bOwnBuilding && C.bBuildingFinished && !C.bForumDown && C.bMarket && Cell >= 0 && Cell < 8 && Cell % 4 < 3 && Cmd.Command != EHOLGridCommandKind::Trade)
			{
				CGSWNote(Bad, First, "Test_CGSWCommandCells", f"{Where}: a market cell of the 2 x 3 trades gave no trade");
			}
			if (Cmd.Command == EHOLGridCommandKind::Specialize || Cmd.Command == EHOLGridCommandKind::CancelSpecialization)
			{
				bool bRunning = Cmd.Option == C.SpecializationPendingOption;
				if (!C.bOwnBuilding || !C.bPrimaryForum || C.bBuildingGroup || C.SpecializationTier <= 0 || Cmd.Tier != C.SpecializationTier
					|| Cmd.Option < 0 || Cmd.Option > 1 || Cell != HOLCommandGridRead::GetSpecializationCell(Cmd.Option)
					|| (Cmd.Command == EHOLGridCommandKind::CancelSpecialization) != bRunning
					|| Cmd.RemainingMs != (bRunning ? C.SpecializationRemainingMs : 0))
				{
					CGSWNote(Bad, First, "Test_CGSWCommandCells", f"{Where}: a specialization cell out of place (option {Cmd.Option}, tier {Cmd.Tier}, command {int(Cmd.Command)})");
				}
			}
			if (Cmd.Command == EHOLGridCommandKind::HeroAbility)
			{
				if (C.bOwnBuilding || !C.bHero || Cmd.CooldownSeconds < 0 || (Cmd.CooldownSeconds > 0 && !Cmd.bGreyed))
				{
					CGSWNote(Bad, First, "Test_CGSWCommandCells", f"{Where}: a hero ability out of place (cooldown {Cmd.CooldownSeconds}, greyed {Cmd.bGreyed})");
				}
			}
			if (Cmd.Command == EHOLGridCommandKind::NecroticRage && (C.bOwnBuilding || !C.bRageUnit))
			{
				CGSWNote(Bad, First, "Test_CGSWCommandCells", f"{Where}: the rage on a selection without a rage unit");
			}
			// Nothing but those kinds is ever a command of the grid on the right.
			if (Cmd.Command == EHOLGridCommandKind::AttackMove || Cmd.Command == EHOLGridCommandKind::Stop || Cmd.Command == EHOLGridCommandKind::Hold
				|| Cmd.Command == EHOLGridCommandKind::Deploy || Cmd.Command == EHOLGridCommandKind::Build)
			{
				CGSWNote(Bad, First, "Test_CGSWCommandCells", f"{Where}: the grid gave an order or a build command ({int(Cmd.Command)})");
			}
			// The orders' panel.
			if (Ord.Command != EHOLGridCommandKind::None)
			{
				if (!C.bOwnUnits || C.bOwnBuilding)
				{
					CGSWNote(Bad, First, "Test_CGSWCommandCells", f"{Where}: an order for a selection that is not units");
				}
				if (Ord.Command == EHOLGridCommandKind::AttackMove && (!C.bSoldiers || C.bOnlyKings || Cell != 4))
				{
					CGSWNote(Bad, First, "Test_CGSWCommandCells", f"{Where}: attack-move out of place (soldiers {C.bSoldiers}, only kings {C.bOnlyKings})");
				}
				if (Ord.Command == EHOLGridCommandKind::Stop && Cell != 5)
				{
					CGSWNote(Bad, First, "Test_CGSWCommandCells", f"{Where}: stop is not on S");
				}
				if (Ord.Command == EHOLGridCommandKind::Hold && (C.bOnlyKings || Cell != 11))
				{
					CGSWNote(Bad, First, "Test_CGSWCommandCells", f"{Where}: hold out of place (only kings {C.bOnlyKings})");
				}
				if (Ord.Command == EHOLGridCommandKind::Deploy && (!C.bDeployables || Cell != 6 || Ord.bPack != C.bDeployedAny))
				{
					CGSWNote(Bad, First, "Test_CGSWCommandCells", f"{Where}: deploy out of place (pack {Ord.bPack}, deployed {C.bDeployedAny})");
				}
				if (Ord.Command != EHOLGridCommandKind::AttackMove && Ord.Command != EHOLGridCommandKind::Stop && Ord.Command != EHOLGridCommandKind::Hold
					&& Ord.Command != EHOLGridCommandKind::Deploy)
				{
					CGSWNote(Bad, First, "Test_CGSWCommandCells", f"{Where}: the orders' panel gave {int(Ord.Command)}");
				}
			}
			if (C.bOwnUnits && !C.bOwnBuilding && Cell == 5 && Ord.Command != EHOLGridCommandKind::Stop)
			{
				CGSWNote(Bad, First, "Test_CGSWCommandCells", f"{Where}: units are selected and S does not stop (even the kings can stop)");
			}
			if (C.bOwnUnits && !C.bOwnBuilding && Cell == 4 && C.bSoldiers && !C.bOnlyKings && Ord.Command != EHOLGridCommandKind::AttackMove)
			{
				CGSWNote(Bad, First, "Test_CGSWCommandCells", f"{Where}: soldiers are selected and Q does not attack-move");
			}
		}
	}
	T.AssertTrue(Selections == 4096, "4096 selections were swept");
	T.AssertTrue(Bad == 0, f"{Bad} cells break an invariant of the grid; first: {First}");
}

// GetCommand on every building of the table, own, finished or a site, market or not (was the table loop of Test_CGRAgreementSelections / Tables).
void Test_CGSWTableBuildings(FUnitTest& T)
{
	FHOLGridContextView Builders;
	Builders.bBuilders = true;
	Builders.bOwnUnits = true;
	Builders = HOLCommandGridRead::WithTableBuildables(Builders);
	T.AssertTrue(Builders.BuildableTypes.Num() >= 8, "the table has several buildings (at least 8 are buildable)");

	FName NoName;
	int Bad = 0;
	FString First;
	int Swept = 0;
	for (int Type = 0; Type < Builders.BuildableTypes.Num(); Type++)
	{
		FName TypeName = Builders.BuildableTypes[Type];
		TArray<FName> Produces = HOLCommandGridRead::GetProducesOf(TypeName);
		TArray<FName> Researches = HOLCommandGridRead::GetBuildingResearches(TypeName);
		if (!HOLCommandGridRead::IsTypeInTable(TypeName))
		{
			CGSWNote(Bad, First, "Test_CGSWTableBuildings", f"{TypeName} is buildable but not in the table");
		}
		if (Produces.Num() > 8)
		{
			CGSWNote(Bad, First, "Test_CGSWTableBuildings", f"{TypeName} trains {Produces.Num()} units: they do not fit the two top rows (8 cells)");
		}
		if (Researches.Num() > 4)
		{
			CGSWNote(Bad, First, "Test_CGSWTableBuildings", f"{TypeName} researches {Researches.Num()} technologies: they do not fit the bottom row (W X C V)");
		}
		for (int Pass = 0; Pass < 8; Pass++)
		{
			FHOLGridContextView Building;
			Building.bOwnBuilding = true;
			Building.bBuildingFinished = (Pass & 1) == 0;
			Building.bPrimaryForum = (Pass & 2) != 0;
			Building.bMarket = TypeName == n"Market" || (Pass & 4) != 0;
			Building.Produces = Produces;
			Building.Researches = Researches;
			Building.SpecializationTier = (Pass & 2) != 0 ? 1 : 0;
			Building.SpecializationPendingOption = Pass % 3 - 1;
			Building.SpecializationRemainingMs = 4000;
			Swept++;
			int ProduceCount = 0;
			int ResearchCount = 0;
			TArray<FName> Seen;
			for (int Cell = -1; Cell <= 12; Cell++)
			{
				FHOLGridCommandView Cmd = HOLCommandGridRules::GetCommand(Building, Cell);
				FString Where = f"{TypeName} pass {Pass} cell {Cell}";
				if (Cmd.Command == EHOLGridCommandKind::None)
				{
					continue;
				}
				if (Cell < 0 || Cell >= 12)
				{
					CGSWNote(Bad, First, "Test_CGSWTableBuildings", f"{Where}: a command outside the grid");
				}
				bool bNamed = Cmd.Command == EHOLGridCommandKind::Produce || Cmd.Command == EHOLGridCommandKind::Research;
				if (bNamed && Cmd.TypeName == NoName)
				{
					CGSWNote(Bad, First, "Test_CGSWTableBuildings", f"{Where}: a command without a name");
				}
				if (Cmd.Command == EHOLGridCommandKind::Produce)
				{
					ProduceCount++;
					if (Cell >= 8)
					{
						CGSWNote(Bad, First, "Test_CGSWTableBuildings", f"{Where}: a unit on the technology row");
					}
					// A unit is not a building: IsTypeInTable (the building table) logs an Error for "Spearman". The unit must be one this building trains.
					if (!Produces.Contains(Cmd.TypeName))
					{
						CGSWNote(Bad, First, "Test_CGSWTableBuildings", f"{Where}: the unit {Cmd.TypeName} is not one the table gives to {TypeName}");
					}
				}
				if (Cmd.Command == EHOLGridCommandKind::Research)
				{
					ResearchCount++;
					if (Cell < 8)
					{
						CGSWNote(Bad, First, "Test_CGSWTableBuildings", f"{Where}: a technology above the technology row");
					}
					if (!HOLCommandGridRead::IsTechnologyInTable(Cmd.TypeName))
					{
						CGSWNote(Bad, First, "Test_CGSWTableBuildings", f"{Where}: the technology {Cmd.TypeName} is not in the table");
					}
				}
				if (bNamed)
				{
					if (Seen.Contains(Cmd.TypeName))
					{
						CGSWNote(Bad, First, "Test_CGSWTableBuildings", f"{Where}: {Cmd.TypeName} sits on two cells");
					}
					Seen.Add(Cmd.TypeName);
				}
			}
			bool bSite = !Building.bBuildingFinished || Building.bMarket;
			bool bAnswers = !bSite && Building.SpecializationTier == 0;
			if (bAnswers && ProduceCount != Produces.Num())
			{
				CGSWNote(Bad, First, "Test_CGSWTableBuildings", f"{TypeName} pass {Pass}: {ProduceCount} unit cells for {Produces.Num()} units of the table");
			}
			if (bAnswers && ResearchCount != Researches.Num())
			{
				CGSWNote(Bad, First, "Test_CGSWTableBuildings", f"{TypeName} pass {Pass}: {ResearchCount} technology cells for {Researches.Num()} technologies of the table");
			}
			if (bSite && (ProduceCount != 0 || ResearchCount != 0))
			{
				CGSWNote(Bad, First, "Test_CGSWTableBuildings", f"{TypeName} pass {Pass}: a site or a market produced or researched");
			}
		}
	}
	T.AssertTrue(Swept >= 64, "every building of the table was swept in 8 passes");
	T.AssertTrue(Bad == 0, f"{Bad} cells of the table's buildings break an invariant; first: {First}");
}

// The builders' panels with the whole table: every building sits on one age page and one cell, and no key answers two buildings.
void Test_CGSWTableCellsUnique(FUnitTest& T)
{
	FHOLGridContextView Builders;
	Builders.bBuilders = true;
	Builders.bOwnUnits = true;
	Builders.ReachedAge = 3;
	Builders = HOLCommandGridRead::WithTableBuildables(Builders);
	FName NoName;
	int Bad = 0;
	FString First;
	int Placed = 0;
	for (int Age = 0; Age < 4; Age++)
	{
		for (int Cell = 0; Cell < 12; Cell++)
		{
			FHOLGridCommandView Building = HOLCommandGridRules::GetAgeBuilding(Builders, Age, Cell);
			if (Building.Command == EHOLGridCommandKind::None)
			{
				continue;
			}
			Placed++;
			FString Where = f"age {Age} cell {Cell}";
			if (Building.Command != EHOLGridCommandKind::Build || Building.TypeName == NoName || Building.Age != Age)
			{
				CGSWNote(Bad, First, "Test_CGSWTableCellsUnique", f"{Where}: a panel cell that is not a named build command of its own age");
			}
			if (CGSWFindBuildable(Builders, Building.TypeName, Age, Cell) < 0)
			{
				CGSWNote(Bad, First, "Test_CGSWTableCellsUnique", f"{Where}: {Building.TypeName} is not a building of that age and cell in the table");
			}
			if (Building.bGreyed)
			{
				CGSWNote(Bad, First, "Test_CGSWTableCellsUnique", f"{Where}: {Building.TypeName} is greyed although every age is reached");
			}
		}
	}
	T.AssertEquals(Placed, Builders.BuildableTypes.Num(), "each building of the table is on exactly one cell of one age page (two buildings on one key would lose one)");
	T.AssertTrue(Bad == 0, f"{Bad} panel cells break an invariant; first: {First}");
}

// The age pages: greyed while the age is not reached (unless the building is always open), faded only while nothing on the page can be built.
void Test_CGSWAgePanels(FUnitTest& T)
{
	int Bad = 0;
	FString First;
	for (int Mask = 0; Mask < 4096; Mask++)
	{
		FHOLGridContextView C = CGSWMakeContext(Mask);
		for (int Age = -1; Age <= 4; Age++)
		{
			FString Where = f"selection {Mask} age {Age}";
			bool bOpen = HOLCommandGridRules::IsAgeOpen(C, Age);
			float Opacity = HOLCommandGridRules::GetAgePanelOpacity(C, Age);
			if (Age < 0 || Age >= 4)
			{
				if (bOpen || HOLCommandGridRules::IsAgeReached(C, Age) || Opacity >= 1.0f)
				{
					CGSWNote(Bad, First, "Test_CGSWAgePanels", f"{Where}: an age that does not exist is open or fully opaque");
				}
			}
			else
			{
				if (Age <= C.ReachedAge && (!bOpen || !HOLCommandGridRules::IsAgeReached(C, Age) || Opacity != 1.0f))
				{
					CGSWNote(Bad, First, "Test_CGSWAgePanels", f"{Where}: a reached age is not open or is faded");
				}
				if (bOpen && !CGSWAgePageOpen(C, Age))
				{
					CGSWNote(Bad, First, "Test_CGSWAgePanels", f"{Where}: an age above the tier is open (reached {C.ReachedAge}) without an always-open building");
				}
				if (!bOpen && Opacity >= 1.0f)
				{
					CGSWNote(Bad, First, "Test_CGSWAgePanels", f"{Where}: a locked page is fully opaque");
				}
				if (bOpen && Opacity != 1.0f)
				{
					CGSWNote(Bad, First, "Test_CGSWAgePanels", f"{Where}: an open page is faded");
				}
			}
			for (int Cell = -1; Cell <= 12; Cell++)
			{
				FHOLGridCommandView Building = HOLCommandGridRules::GetAgeBuilding(C, Age, Cell);
				if (Building.Command == EHOLGridCommandKind::None)
				{
					continue;
				}
				FString CellWhere = f"{Where} cell {Cell}";
				if (Age < 0 || Age >= 4 || Cell < 0 || Cell >= 12 || !CGSWIsBuildersAlone(C))
				{
					CGSWNote(Bad, First, "Test_CGSWAgePanels", f"{CellWhere}: a building outside the pages or for a selection that is not the builders alone");
					continue;
				}
				int Index = CGSWFindBuildable(C, Building.TypeName, Age, Cell);
				if (Index < 0)
				{
					CGSWNote(Bad, First, "Test_CGSWAgePanels", f"{CellWhere}: {Building.TypeName} is not a building of that age and cell");
					continue;
				}
				if (Age <= C.ReachedAge && Building.bGreyed)
				{
					CGSWNote(Bad, First, "Test_CGSWAgePanels", f"{CellWhere}: {Building.TypeName} is greyed on a reached age");
				}
				if (Age > C.ReachedAge && !CGSWIsAlwaysOpenIndex(C, Index) && !Building.bGreyed)
				{
					CGSWNote(Bad, First, "Test_CGSWAgePanels", f"{CellWhere}: {Building.TypeName} is not greyed above the tier");
				}
				if (Age > C.ReachedAge && CGSWIsAlwaysOpenIndex(C, Index) && Building.bGreyed)
				{
					CGSWNote(Bad, First, "Test_CGSWAgePanels", f"{CellWhere}: an always-open building is greyed");
				}
			}
		}
	}
	T.AssertTrue(Bad == 0, f"{Bad} age pages break an invariant; first: {First}");
}

// PressKey on 4096 selections x chosen age -1..3 x cells -1..12 (was Test_CGKYAgreementKeys, PressKey part).
void Test_CGSWPressKeySweep(FUnitTest& T)
{
	int Bad = 0;
	FString First;
	int Swept = 0;
	for (int Mask = 0; Mask < 4096; Mask++)
	{
		FHOLGridContextView C = CGSWMakeContext(Mask);
		for (int Chosen = -1; Chosen <= 3; Chosen++)
		{
			TArray<FName> BuiltHere;
			int AgeKeys = 0;
			for (int Cell = -1; Cell <= 12; Cell++)
			{
				FHOLGridKeyResultView Key = HOLCommandGridKeys::PressKey(C, Chosen, Cell);
				Swept++;
				bool bInGrid = Cell >= 0 && Cell < 12;
				CGSWCheckKeyAnswer(C, Key, Chosen, Cell, Mask, -9, "Test_CGSWPressKeySweep", Bad, First);
				if (!bInGrid && (Key.Target != EHOLGridKeyTargetKind::None || Key.ChosenAge != Chosen))
				{
					CGSWNote(Bad, First, "Test_CGSWPressKeySweep", f"{CGSWWhere(Mask, Chosen, -9, Cell)}: a key outside the 3 x 4 grid answered or changed the chosen age");
				}
				if (bInGrid && C.bOwnBuilding && Key.Target != EHOLGridKeyTargetKind::Grid)
				{
					CGSWNote(Bad, First, "Test_CGSWPressKeySweep", f"{CGSWWhere(Mask, Chosen, -9, Cell)}: with a building selected every key is the grid's");
				}
				if (Key.Target == EHOLGridKeyTargetKind::Age)
				{
					AgeKeys++;
					if (Chosen != -1 || Key.ChosenAge != Cell || Cell >= 4)
					{
						CGSWNote(Bad, First, "Test_CGSWPressKeySweep", f"{CGSWWhere(Mask, Chosen, -9, Cell)}: an age key must be A Z E R with no age chosen, and choose its own age (got {Key.ChosenAge})");
					}
				}
				if (Key.Target == EHOLGridKeyTargetKind::Build)
				{
					if (Chosen < 0 || Key.ChosenAge != Chosen)
					{
						CGSWNote(Bad, First, "Test_CGSWPressKeySweep", f"{CGSWWhere(Mask, Chosen, -9, Cell)}: a build key must keep the chosen age (got {Key.ChosenAge})");
					}
					if (BuiltHere.Contains(Key.TypeName))
					{
						CGSWNote(Bad, First, "Test_CGSWPressKeySweep", f"{CGSWWhere(Mask, Chosen, -9, Cell)}: {Key.TypeName} answers two keys");
					}
					BuiltHere.Add(Key.TypeName);
				}
				if (Key.Target == EHOLGridKeyTargetKind::Grid && !C.bOwnBuilding)
				{
					EHOLGridCommandKind Kind = HOLCommandGridRules::GetCommand(C, Cell).Command;
					if (Kind != EHOLGridCommandKind::HeroAbility && Kind != EHOLGridCommandKind::NecroticRage)
					{
						CGSWNote(Bad, First, "Test_CGSWPressKeySweep", f"{CGSWWhere(Mask, Chosen, -9, Cell)}: units keep a key for the grid only for the hero's abilities and the rage");
					}
				}
				if (CGSWIsBuildersAlone(C) && Chosen >= 0 && bInGrid)
				{
					// An age chosen: every key is its buildings' (an empty cell: nothing, never an order nor another age), the age kept.
					if (Key.Target != EHOLGridKeyTargetKind::None && Key.Target != EHOLGridKeyTargetKind::Build)
					{
						CGSWNote(Bad, First, "Test_CGSWPressKeySweep", f"{CGSWWhere(Mask, Chosen, -9, Cell)}: with an age chosen a key went to {int(Key.Target)} instead of a building or nothing");
					}
					if (Key.ChosenAge != Chosen)
					{
						CGSWNote(Bad, First, "Test_CGSWPressKeySweep", f"{CGSWWhere(Mask, Chosen, -9, Cell)}: a key changed the chosen age {Chosen} to {Key.ChosenAge}");
					}
				}
			}
			if (CGSWIsBuildersAlone(C) && Chosen == -1)
			{
				int OpenAges = 0;
				for (int Age = 0; Age < 4; Age++)
				{
					if (CGSWAgePageOpen(C, Age))
					{
						OpenAges++;
					}
				}
				if (AgeKeys != OpenAges)
				{
					CGSWNote(Bad, First, "Test_CGSWPressKeySweep", f"selection {Mask}: {AgeKeys} keys choose an age, {OpenAges} ages are open");
				}
			}
		}
	}
	T.AssertTrue(Swept == 4096 * 5 * 14, "4096 selections x 5 chosen ages x 14 keys were swept");
	T.AssertTrue(Bad == 0, f"{Bad} key answers break an invariant; first: {First}");
}

// ClickAge on 4096 selections x chosen age (-1, 1) x ages -1..4 x cells -1..12 (was Test_CGKYAgreementKeys, ClickAge part).
void Test_CGSWClickAgeSweep(FUnitTest& T)
{
	int Bad = 0;
	FString First;
	int Swept = 0;
	for (int Mask = 0; Mask < 4096; Mask++)
	{
		FHOLGridContextView C = CGSWMakeContext(Mask);
		for (int Pass = 0; Pass < 2; Pass++)
		{
			int Chosen = Pass == 0 ? -1 : 1;
			for (int Age = -1; Age <= 4; Age++)
			{
				for (int Cell = -1; Cell <= 12; Cell++)
				{
					FHOLGridKeyResultView Click = HOLCommandGridKeys::ClickAge(C, Chosen, Age, Cell);
					Swept++;
					CGSWCheckKeyAnswer(C, Click, Chosen, Cell, Mask, Age, "Test_CGSWClickAgeSweep", Bad, First);
					bool bAgeExists = Age >= 0 && Age < 4;
					if ((!bAgeExists || !CGSWIsBuildersAlone(C)) && (Click.Target != EHOLGridKeyTargetKind::None || Click.ChosenAge != Chosen))
					{
						CGSWNote(Bad, First, "Test_CGSWClickAgeSweep", f"{CGSWWhere(Mask, Chosen, Age, Cell)}: a click on no age, or off the builders' grid, did something");
					}
					if (Click.Target == EHOLGridKeyTargetKind::Orders || Click.Target == EHOLGridKeyTargetKind::Grid)
					{
						CGSWNote(Bad, First, "Test_CGSWClickAgeSweep", f"{CGSWWhere(Mask, Chosen, Age, Cell)}: a click on an age page went to the orders or the grid");
					}
					if (Click.Target == EHOLGridKeyTargetKind::Age && (Cell != -1 || Click.ChosenAge != Age))
					{
						CGSWNote(Bad, First, "Test_CGSWClickAgeSweep", f"{CGSWWhere(Mask, Chosen, Age, Cell)}: only the header of age {Age} chooses age {Age} (got {Click.ChosenAge})");
					}
					if (bAgeExists && CGSWIsBuildersAlone(C) && CGSWAgePageOpen(C, Age) && Cell == -1
						&& (Click.Target != EHOLGridKeyTargetKind::Age || Click.ChosenAge != Age))
					{
						CGSWNote(Bad, First, "Test_CGSWClickAgeSweep", f"{CGSWWhere(Mask, Chosen, Age, Cell)}: the header of an open age did not choose it");
					}
					if (Click.Target == EHOLGridKeyTargetKind::Build && Click.ChosenAge != Age)
					{
						CGSWNote(Bad, First, "Test_CGSWClickAgeSweep", f"{CGSWWhere(Mask, Chosen, Age, Cell)}: a click on a cell of age {Age} built under age {Click.ChosenAge}");
					}
				}
			}
		}
	}
	T.AssertTrue(Swept == 4096 * 2 * 6 * 14, "4096 selections x 2 chosen ages x 6 ages x 14 cells were swept");
	T.AssertTrue(Bad == 0, f"{Bad} clicks break an invariant; first: {First}");
}

// After a placement with Shift the age stays chosen; every other event closes it (GDD: the age page closes unless Shift keeps it).
void Test_CGSWChosenAgeAfter(FUnitTest& T)
{
	int Fails = 0;
	for (int Chosen = -1; Chosen <= 3; Chosen++)
	{
		int Kept = HOLCommandGridKeys::GetChosenAgeAfter(Chosen, EHOLAgeEventKind::PlacedKept);
		int Placed = HOLCommandGridKeys::GetChosenAgeAfter(Chosen, EHOLAgeEventKind::Placed);
		int Cancelled = HOLCommandGridKeys::GetChosenAgeAfter(Chosen, EHOLAgeEventKind::Cancelled);
		int Escaped = HOLCommandGridKeys::GetChosenAgeAfter(Chosen, EHOLAgeEventKind::Escape);
		int Changed = HOLCommandGridKeys::GetChosenAgeAfter(Chosen, EHOLAgeEventKind::SelectionChanged);
		CGSWCheck(T, Fails, "Test_CGSWChosenAgeAfter", Kept == Chosen, f"Shift placement keeps age {Chosen} (got {Kept})");
		CGSWCheck(T, Fails, "Test_CGSWChosenAgeAfter", Placed == -1, f"a placement closes age {Chosen} (got {Placed})");
		CGSWCheck(T, Fails, "Test_CGSWChosenAgeAfter", Cancelled == -1, f"a cancel closes age {Chosen} (got {Cancelled})");
		CGSWCheck(T, Fails, "Test_CGSWChosenAgeAfter", Escaped == -1, f"Escape closes age {Chosen} (got {Escaped})");
		CGSWCheck(T, Fails, "Test_CGSWChosenAgeAfter", Changed == -1, f"a new selection closes age {Chosen} (got {Changed})");
	}
}

// Describe on every combination of kind x resource x refusal x category x block (was the Describe sweep of Test_CGKYAgreementFeedback).
void Test_CGSWDescribeSweep(FUnitTest& T)
{
	int Fails = 0;
	// The four resources and the three categories, by their names (GDD: Food, Wood, Stone, Gold; economic, military, technological buildings).
	TArray<FString> ResourceNames;
	ResourceNames.Add("Food");
	ResourceNames.Add("Wood");
	ResourceNames.Add("Stone");
	ResourceNames.Add("Gold");
	TArray<FString> CategoryNames;
	CategoryNames.Add("Economic");
	CategoryNames.Add("Military");
	CategoryNames.Add("Technological");

	int Bad = 0;
	FString First;
	int Swept = 0;
	TArray<FString> KindTexts;
	TArray<FString> ResourceTexts;
	TArray<FString> RefusalTexts;
	TArray<FString> CategoryTexts;
	TArray<FString> BlockTexts;
	for (int Kind = 0; Kind <= 6; Kind++)
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
						FString Text = HOLCommandGridKeys::Describe(Feedback);
						Swept++;
						FString Where = f"kind {Kind} resource {Resource} refusal {Refusal} category {Category} block {Block}";
						if (Text.IsEmpty())
						{
							CGSWNote(Bad, First, "Test_CGSWDescribeSweep", f"{Where}: Describe gave an empty text");
						}
						if ((Kind == 0) != (Text == "none"))
						{
							CGSWNote(Bad, First, "Test_CGSWDescribeSweep", f"{Where}: only a feedback of kind None may read 'none' (got '{Text}')");
						}
						if (Kind == 1 && !Text.Contains(ResourceNames[Resource]))
						{
							CGSWNote(Bad, First, "Test_CGSWDescribeSweep", f"{Where}: 'not enough' must name the missing resource {ResourceNames[Resource]} (got '{Text}')");
						}
						if (Kind == 5 && !Text.Contains(CategoryNames[Category]))
						{
							CGSWNote(Bad, First, "Test_CGSWDescribeSweep", f"{Where}: 'no building' must name the category {CategoryNames[Category]} (got '{Text}')");
						}
						if (Kind == 4 && !Text.Contains("placement refused"))
						{
							CGSWNote(Bad, First, "Test_CGSWDescribeSweep", f"{Where}: a placement refusal must say so (got '{Text}')");
						}
						if (Kind == 6 && !Text.Contains("research refused"))
						{
							CGSWNote(Bad, First, "Test_CGSWDescribeSweep", f"{Where}: a research refusal must say so (got '{Text}')");
						}
						// One text per meaning: record the texts of the first combination of each axis.
						if (Resource == 0 || Resource > 0)
						{
							if (Kind == 1 && Refusal == 0 && Category == 0 && Block == 0)
							{
								ResourceTexts.Add(Text);
							}
							if (Kind == 4 && Resource == 0 && Category == 0 && Block == 0 && Refusal <= 5)
							{
								RefusalTexts.Add(Text);
							}
							if (Kind == 5 && Resource == 0 && Refusal == 0 && Block == 0)
							{
								CategoryTexts.Add(Text);
							}
							if (Kind == 6 && Resource == 0 && Refusal == 0 && Category == 0 && Block >= 1 && Block <= 8)
							{
								BlockTexts.Add(Text);
							}
							if (Resource == 0 && Refusal == 0 && Category == 0 && Block == 0)
							{
								KindTexts.Add(Text);
							}
						}
					}
				}
			}
		}
	}
	CGSWCheck(T, Fails, "Test_CGSWDescribeSweep", Swept == 7 * 4 * 7 * 3 * 10, "every combination of kind, resource, refusal, category, block was swept");
	CGSWCheck(T, Fails, "Test_CGSWDescribeSweep", Bad == 0, f"{Bad} feedbacks break an invariant; first: {First}");

	// Different meanings read differently (no two kinds, resources, refusals, categories or blocks share a text).
	T.AssertEquals(KindTexts.Num(), 7, "seven feedback kinds");
	for (int A = 0; A < KindTexts.Num(); A++)
	{
		for (int B = A + 1; B < KindTexts.Num(); B++)
		{
			CGSWCheck(T, Fails, "Test_CGSWDescribeSweep", KindTexts[A] != KindTexts[B], f"kinds {A} and {B} read differently ('{KindTexts[A]}')");
		}
	}
	T.AssertEquals(ResourceTexts.Num(), 4, "four resources");
	for (int A = 0; A < ResourceTexts.Num(); A++)
	{
		for (int B = A + 1; B < ResourceTexts.Num(); B++)
		{
			CGSWCheck(T, Fails, "Test_CGSWDescribeSweep", ResourceTexts[A] != ResourceTexts[B], f"resources {A} and {B} read differently ('{ResourceTexts[A]}')");
		}
	}
	T.AssertEquals(RefusalTexts.Num(), 6, "refusals 0..5");
	for (int A = 0; A < RefusalTexts.Num(); A++)
	{
		for (int B = A + 1; B < RefusalTexts.Num(); B++)
		{
			CGSWCheck(T, Fails, "Test_CGSWDescribeSweep", RefusalTexts[A] != RefusalTexts[B], f"placement refusals {A} and {B} read differently ('{RefusalTexts[A]}')");
		}
	}
	T.AssertEquals(CategoryTexts.Num(), 3, "three categories");
	for (int A = 0; A < CategoryTexts.Num(); A++)
	{
		for (int B = A + 1; B < CategoryTexts.Num(); B++)
		{
			CGSWCheck(T, Fails, "Test_CGSWDescribeSweep", CategoryTexts[A] != CategoryTexts[B], f"categories {A} and {B} read differently ('{CategoryTexts[A]}')");
		}
	}
	T.AssertEquals(BlockTexts.Num(), 8, "technology blocks 1..8");
	for (int A = 0; A < BlockTexts.Num(); A++)
	{
		for (int B = A + 1; B < BlockTexts.Num(); B++)
		{
			CGSWCheck(T, Fails, "Test_CGSWDescribeSweep", BlockTexts[A] != BlockTexts[B], f"technology blocks {A + 1} and {B + 1} read differently ('{BlockTexts[A]}')");
		}
	}

	// A few texts in clear (the words of the log).
	FHOLFeedbackView Queue;
	Queue.Kind = EHOLCommandFeedbackKind::QueueFull;
	T.AssertEquals(HOLCommandGridKeys::Describe(Queue), "queue full", "a full queue reads 'queue full'");
	FHOLFeedbackView Population;
	Population.Kind = EHOLCommandFeedbackKind::PopulationFull;
	T.AssertEquals(HOLCommandGridKeys::Describe(Population), "population full", "a full population reads 'population full'");
	FHOLFeedbackView Gold;
	Gold.Kind = EHOLCommandFeedbackKind::NotEnoughResources;
	Gold.Resource = EHOLResource::Gold;
	T.AssertEquals(HOLCommandGridKeys::Describe(Gold), "not enough Gold", "no gold reads 'not enough Gold'");
}

// What stops a command (a full population only waits, D-population) on every kind.
void Test_CGSWBlocksKinds(FUnitTest& T)
{
	int Fails = 0;
	for (int Kind = 0; Kind <= 6; Kind++)
	{
		FHOLFeedbackView Feedback;
		Feedback.Kind = EHOLCommandFeedbackKind(Kind);
		bool bBlocks = HOLCommandGridKeys::Blocks(Feedback);
		bool bMustBlock = Kind == 1 || Kind == 2 || Kind == 4 || Kind == 6;
		if (bMustBlock)
		{
			CGSWCheck(T, Fails, "Test_CGSWBlocksKinds", bBlocks, f"kind {Kind} (not enough resources, queue full, placement refused, research refused) stops the command");
		}
		if (Kind == 0 || Kind == 3)
		{
			CGSWCheck(T, Fails, "Test_CGSWBlocksKinds", !bBlocks, f"kind {Kind} (none, population full) does not stop the command: a full population only waits");
		}
	}
}
