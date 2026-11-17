// MIG-C-Formation (D830, D836, D839, GALADRIEL-1117p): the script port of HOLFormationTests.cpp, run against the read library of the formation rules
// that STAY in C++ (HOLFormation, Simulation/HOLFormationRules.h, read by the simulation): UHOLFormationReadLibrary (namespace HOLFormationRead). Same
// vectors as the C++ tests, one by one. Ported here (11 of 15): Rows, Kinds, Classes, Places, Assign (empty, in shape, 40 random trials: see the
// note on the random stream), Front, Speed, Return, YieldPlace, Corridor, BreakAtMelee. NOT ported (types to expose, see the report): AssignByDepth,
// Group, Block, Settle. The C++ file stays until the rouge vu / vert is confirmed by the main session. The library is NOT COMPILED at the time
// of writing: if a Test_FM* fails to compile, the cause is in the .cpp or in a name used here.
// Note on the random stream: the C++ evaluates "FVector2D(Random.FRandRange(..), Random.FRandRange(..))" in the order the compiler picks (MSVC
// evaluates call arguments right to left), the script evaluates left to right: the random vectors of Assign may differ from the C++ ones; the
// properties asserted (a place of its own, no crossing in a row, rows filled front first) are the same.

FString FMWidthsText(const TArray<int>& Widths)
{
	FString Text = "";
	for (int I = 0; I < Widths.Num(); I++)
	{
		if (I > 0)
		{
			Text = Text + "/";
		}
		Text = Text + f"{Widths[I]}";
	}
	return Text;
}

// A formation written as the user draws it, front row first, each row left to right: "7L/3L2H2L/5H/5H".
FString FMFormationText(const TArray<int>& Widths, const TArray<int>& KindOfPlace)
{
	FString Letters = "LHIARMP";
	FString Text = "";
	int Place = 0;
	for (int Row = 0; Row < Widths.Num(); Row++)
	{
		if (Row > 0)
		{
			Text = Text + "/";
		}
		int Column = 0;
		while (Column < Widths[Row])
		{
			int Kind = KindOfPlace[Place + Column];
			int Run = 0;
			while (Column < Widths[Row] && KindOfPlace[Place + Column] == Kind)
			{
				Run++;
				Column++;
			}
			FString Letter = "?";
			if (Kind >= 0 && Kind < 7)
			{
				Letter = Letters.Mid(Kind, 1);
			}
			Text = Text + f"{Run}" + Letter;
		}
		Place += Widths[Row];
	}
	return Text;
}

// The first N counts of the kinds (the C++ vectors are brace lists of 1 to 6 values).
TArray<int> FMKinds(int N, int A, int B, int C, int D, int E, int F)
{
	TArray<int> Out;
	if (N > 0) Out.Add(A);
	if (N > 1) Out.Add(B);
	if (N > 2) Out.Add(C);
	if (N > 3) Out.Add(D);
	if (N > 4) Out.Add(E);
	if (N > 5) Out.Add(F);
	return Out;
}

int FMSum(const TArray<int>& Counts)
{
	int Total = 0;
	for (int I = 0; I < Counts.Num(); I++)
	{
		Total += Counts[I];
	}
	return Total;
}

bool FMNear(const FVector2D& A, const FVector2D& B, double Tolerance)
{
	return Math::Abs(A.X - B.X) <= Tolerance && Math::Abs(A.Y - B.Y) <= Tolerance;
}

bool FMNearly(double A, double B, double Tolerance)
{
	return Math::Abs(A - B) <= Tolerance;
}

double FMCross2D(const FVector2D& A, const FVector2D& B)
{
	return A.X * B.Y - A.Y * B.X;
}

// Whether segments A-B and C-D cross (strictly, not just touch at an end).
bool FMCross(const FVector2D& A, const FVector2D& B, const FVector2D& C, const FVector2D& D)
{
	double ABC = FMCross2D(B - A, C - A);
	double ABD = FMCross2D(B - A, D - A);
	double CDA = FMCross2D(D - C, A - C);
	double CDB = FMCross2D(D - C, B - C);
	return ABC * ABD < -1.0 && CDA * CDB < -1.0;
}

// Mode 0 peasants, 1 one kind of soldier, 2 several kinds.
void FMCheckRows(FUnitTest& T, int Mode, int Count, const FString& Expected, const FString& Name)
{
	TArray<int> Widths;
	if (Mode == 0)
	{
		HOLFormationRead::MakePeasantRows(Count, Widths);
	}
	else if (Mode == 1)
	{
		HOLFormationRead::MakeSquareRows(Count, Widths);
	}
	else
	{
		HOLFormationRead::MakeMixedRows(Count, Widths);
	}
	FString Text = FMWidthsText(Widths);
	T.AssertTrue(Text == Expected, f"{Name}: {Count} gives {Text}, expected {Expected}");
}

// HeroesOfLiberty.Formation.Rows
void Test_FMRows(FUnitTest& T)
{
	// Peasants: one row up to 5, then rows of ceil(sqrt(2 N)).
	FMCheckRows(T, 0, 0, "", "Peasants");
	FMCheckRows(T, 0, 1, "1", "Peasants");
	FMCheckRows(T, 0, 5, "5", "Peasants");
	FMCheckRows(T, 0, 6, "4/2", "Peasants");
	FMCheckRows(T, 0, 10, "5/5", "Peasants");
	FMCheckRows(T, 0, 15, "6/6/3", "Peasants");
	FMCheckRows(T, 0, 20, "7/7/6", "Peasants");
	FMCheckRows(T, 0, 50, "10/10/10/10/10", "Peasants");

	// One kind of soldier (the user's rule, 2026-10-20).
	FMCheckRows(T, 1, 0, "", "one kind");
	FMCheckRows(T, 1, 1, "1", "one kind");
	FMCheckRows(T, 1, 2, "2", "one kind");
	FMCheckRows(T, 1, 4, "3/1", "one kind");
	FMCheckRows(T, 1, 9, "3/3/3", "one kind");
	FMCheckRows(T, 1, 10, "5/5", "one kind");
	FMCheckRows(T, 1, 12, "5/5/2", "one kind");
	FMCheckRows(T, 1, 20, "5/5/5/5", "one kind");
	FMCheckRows(T, 1, 30, "7/7/7/7/2", "one kind");
	FMCheckRows(T, 1, 40, "7/7/7/7/7/5", "one kind");
	FMCheckRows(T, 1, 50, "9/9/9/9/9/5", "one kind");

	// Several kinds: odd rows, the remainder on the front rows or one more row.
	FMCheckRows(T, 2, 1, "1", "several kinds");
	FMCheckRows(T, 2, 2, "2", "several kinds");
	FMCheckRows(T, 2, 4, "3/1", "several kinds");
	FMCheckRows(T, 2, 19, "5/5/5/4", "several kinds");
	FMCheckRows(T, 2, 20, "5/5/5/5", "several kinds");
	FMCheckRows(T, 2, 24, "7/7/5/5", "several kinds");
	FMCheckRows(T, 2, 40, "9/9/8/7/7", "several kinds");
	FMCheckRows(T, 2, 60, "11/11/11/9/9/9", "several kinds");

	// The drag's width: full rows, the last one shorter, never wider than the group.
	TArray<int> Widths;
	HOLFormationRead::MakeRowsOfWidth(17, 6, Widths);
	T.AssertTrue(FMWidthsText(Widths) == "6/6/5", "17 in rows of 6");
	HOLFormationRead::MakeRowsOfWidth(3, 10, Widths);
	T.AssertTrue(FMWidthsText(Widths) == "3", "3 in rows of 10");
	HOLFormationRead::MakeRowsOfWidth(3, 0, Widths);
	T.AssertTrue(FMWidthsText(Widths) == "1/1/1", "a width of 0 is one column");
}

// HeroesOfLiberty.Formation.Kinds
void Test_FMKinds(FUnitTest& T)
{
	// The user's drawings (L Spearman, H Man-at-arms, A Archer; front row first, left to right).
	TArray<int> Widths;
	TArray<int> KindOfPlace;
	TArray<FString> Expected;
	Expected.Add("7L/3L2H2L/5H/5H");
	Expected.Add("5L/5L/5H/5H");
	Expected.Add("5L/5L/2L1H2L/4H");
	Expected.Add("7L/7H/5H/5H");
	Expected.Add("2H3L2H/7H/5H/5H");
	Expected.Add("7L/1L6H/1H3A1H/5A");
	Expected.Add("3L/1H");
	Expected.Add("11L/11L/4L3H4L/9H/9H/9H");
	Expected.Add("9L/9L/1L6H1L/7H/7H");
	for (int Scenario = 0; Scenario < 9; Scenario++)
	{
		TArray<int> Counts;
		if (Scenario == 0) Counts = FMKinds(2, 12, 12, 0, 0, 0, 0);
		if (Scenario == 1) Counts = FMKinds(2, 10, 10, 0, 0, 0, 0);
		if (Scenario == 2) Counts = FMKinds(2, 14, 5, 0, 0, 0, 0);
		if (Scenario == 3) Counts = FMKinds(2, 7, 17, 0, 0, 0, 0);
		if (Scenario == 4) Counts = FMKinds(2, 3, 21, 0, 0, 0, 0);
		if (Scenario == 5) Counts = FMKinds(4, 8, 8, 0, 8, 0, 0);
		if (Scenario == 6) Counts = FMKinds(2, 3, 1, 0, 0, 0, 0);
		if (Scenario == 7) Counts = FMKinds(2, 30, 30, 0, 0, 0, 0);
		if (Scenario == 8) Counts = FMKinds(2, 20, 20, 0, 0, 0, 0);
		HOLFormationRead::MakeMixedRows(FMSum(Counts), Widths);
		HOLFormationRead::PlaceKinds(Widths, Counts, KindOfPlace);
		FString Text = FMFormationText(Widths, KindOfPlace);
		T.AssertTrue(Text == Expected[Scenario], f"scenario {Scenario}: {Text}, expected {Expected[Scenario]}");
	}

	// A kind with no unit is skipped; one kind fills its rows (the user's rule, 2026-10-20: 12 -> 5/5/2).
	HOLFormationRead::MakeMixedRows(4, Widths);
	HOLFormationRead::PlaceKinds(Widths, FMKinds(4, 3, 0, 0, 1, 0, 0), KindOfPlace);
	T.AssertTrue(FMFormationText(Widths, KindOfPlace) == "3L/1A", "an empty kind skipped");
	HOLFormationRead::MakeSquareRows(12, Widths);
	HOLFormationRead::PlaceKinds(Widths, FMKinds(1, 12, 0, 0, 0, 0, 0), KindOfPlace);
	T.AssertTrue(FMFormationText(Widths, KindOfPlace) == "5L/5L/2L", "one kind");
}

// HeroesOfLiberty.Formation.Classes
void Test_FMClasses(FUnitTest& T)
{
	int OtherMelee = int(HOLFormationRead::GetConstant(n"OtherMeleeRowPriority"));
	int ArcherPriority = int(HOLFormationRead::GetConstant(n"ArcherRowPriority"));
	int OtherRanged = int(HOLFormationRead::GetConstant(n"OtherRangedRowPriority"));
	int SiegePriority = int(HOLFormationRead::GetConstant(n"SiegeRowPriority"));
	int PeasantPriority = int(HOLFormationRead::GetConstant(n"PeasantRowPriority"));

	T.AssertEquals(0, HOLFormationRead::GetRowPriority(n"Spearman", 0, false, false), "Spearman: 0");
	T.AssertEquals(1, HOLFormationRead::GetRowPriority(n"ManAtArms", 0, false, false), "Man-at-arms: 1");
	T.AssertEquals(OtherMelee, HOLFormationRead::GetRowPriority(n"LightInfantry", 0, false, false), "light infantry: other melee");
	T.AssertEquals(ArcherPriority, HOLFormationRead::GetRowPriority(n"Archer", 0, true, false), "Archer: a shooter");
	T.AssertEquals(OtherRanged, HOLFormationRead::GetRowPriority(n"Crossbowman", 0, true, false), "another shooter");
	T.AssertEquals(SiegePriority, HOLFormationRead::GetRowPriority(n"Mangonel", 0, true, true), "siege");
	T.AssertEquals(PeasantPriority, HOLFormationRead::GetRowPriority(n"Peasant", 10, false, false), "Peasant");
	// ERowClass order: 0 Melee, 1 Ranged, 2 Siege, 3 Worker.
	T.AssertEquals(0, HOLFormationRead::GetRowClass(OtherMelee), "other melee: Melee");
	T.AssertEquals(1, HOLFormationRead::GetRowClass(ArcherPriority), "Archer: Ranged");
	T.AssertEquals(1, HOLFormationRead::GetRowClass(OtherRanged), "other shooter: Ranged");
	T.AssertEquals(2, HOLFormationRead::GetRowClass(SiegePriority), "siege: Siege");
	T.AssertEquals(3, HOLFormationRead::GetRowClass(PeasantPriority), "Peasant: Worker");

	// The user's examples (2026-10-22): rows given by MakeGroupRows, kinds by PlaceKinds, a place per unit.
	TArray<FString> Names;
	TArray<FString> Expected;
	Names.Add("5 L + 2 H + 6 A");
	Expected.Add("5L/1H3A1H/3A");
	Names.Add("5 L + 7 A + 1 M");
	Expected.Add("5L/5A/2A/1M");
	Names.Add("1 L + 8 A");
	Expected.Add("1L/3A/3A/2A");
	Names.Add("1 L + 1 H + 1 A");
	Expected.Add("1H1L/1A");
	Names.Add("6 A + 1 M");
	Expected.Add("4A/2A/1M");
	Names.Add("5 L + 2 H + 1 M");
	Expected.Add("5L/2H/1M");
	Names.Add("3 I + 3 A");
	Expected.Add("3I/3A");
	TArray<int> Widths;
	TArray<int> KindOfPlace;
	for (int Scenario = 0; Scenario < 7; Scenario++)
	{
		TArray<int> Counts;
		if (Scenario == 0) Counts = FMKinds(4, 5, 2, 0, 6, 0, 0);
		if (Scenario == 1) Counts = FMKinds(6, 5, 0, 0, 7, 0, 1);
		if (Scenario == 2) Counts = FMKinds(4, 1, 0, 0, 8, 0, 0);
		if (Scenario == 3) Counts = FMKinds(4, 1, 1, 0, 1, 0, 0);
		if (Scenario == 4) Counts = FMKinds(6, 0, 0, 0, 6, 0, 1);
		if (Scenario == 5) Counts = FMKinds(6, 5, 2, 0, 0, 0, 1);
		if (Scenario == 6) Counts = FMKinds(4, 0, 0, 3, 3, 0, 0);
		HOLFormationRead::MakeGroupRows(Counts, false, Widths);
		FString Given = FMWidthsText(Widths);
		HOLFormationRead::PlaceKinds(Widths, Counts, KindOfPlace);
		FString Text = FMFormationText(Widths, KindOfPlace);
		T.AssertTrue(Text == Expected[Scenario], f"{Names[Scenario]} (rows given {Given}): {Text}, expected {Expected[Scenario]}");
		T.AssertEquals(FMSum(Counts), KindOfPlace.Num(), f"{Names[Scenario]}: a place per unit");
	}

	// The right-button drag's width keeps the rule: 1 L + 8 A on 4 columns.
	HOLFormationRead::MakeRowsOfWidth(9, 4, Widths);
	HOLFormationRead::PlaceKinds(Widths, FMKinds(4, 1, 0, 0, 8, 0, 0), KindOfPlace);
	T.AssertTrue(FMFormationText(Widths, KindOfPlace) == "1L/4A/4A", "drag of 4 columns: 1 L + 8 A");

	// A group in the world: 1 Spearman, 1 Man-at-arms, 1 Archer along +X, the Archer on the second row.
	TArray<FVector2D> Positions;
	Positions.Add(FVector2D(0.0, 0.0));
	Positions.Add(FVector2D(0.0, 200.0));
	Positions.Add(FVector2D(0.0, -200.0));
	TArray<int> KindOfUnit;
	KindOfUnit.Add(ArcherPriority);
	KindOfUnit.Add(0);
	KindOfUnit.Add(1);
	TArray<FVector2D> Places;
	TArray<int> PlaceOfUnit;
	HOLFormationRead::PlaceGroup(Positions, KindOfUnit, false, FVector2D(2000.0, 0.0), FVector2D(1.0, 0.0), 150.0, 180.0, 0, Places, PlaceOfUnit);
	bool bShape = Places.Num() == 3 && PlaceOfUnit.Num() == 3 && !PlaceOfUnit.Contains(-1);
	T.AssertTrue(bShape, "a place per unit in the world");
	if (bShape)
	{
		T.AssertTrue(FMNearly(Places[PlaceOfUnit[0]].X, 1820.0, 0.5), "the Archer 180 cm behind the front row");
		T.AssertTrue(FMNearly(Places[PlaceOfUnit[1]].X, 2000.0, 0.5), "the Spearman on the front row");
		T.AssertTrue(FMNearly(Places[PlaceOfUnit[2]].X, 2000.0, 0.5), "the Man-at-arms on the front row");
	}
}

// HeroesOfLiberty.Formation.Places
void Test_FMPlaces(FUnitTest& T)
{
	// Marching along +X from (1000, 0): the front row on the target, the next one 100 cm behind; the right side is +Y.
	TArray<int> Widths;
	Widths.Add(3);
	Widths.Add(2);
	TArray<FVector2D> Places;
	HOLFormationRead::MakePlaces(Widths, FVector2D(1000.0, 0.0), FVector2D(5.0, 0.0), 150.0, 100.0, Places);
	T.AssertEquals(5, Places.Num(), "a place per unit");
	if (Places.Num() != 5)
	{
		return;
	}
	T.AssertTrue(FMNear(Places[0], FVector2D(1000.0, -150.0), 0.01), "front row, left");
	T.AssertTrue(FMNear(Places[1], FVector2D(1000.0, 0.0), 0.01), "front row, middle on the target");
	T.AssertTrue(FMNear(Places[2], FVector2D(1000.0, 150.0), 0.01), "front row, right");
	T.AssertTrue(FMNear(Places[3], FVector2D(900.0, -75.0), 0.01), "even row at -0.5 spacing");
	T.AssertTrue(FMNear(Places[4], FVector2D(900.0, 75.0), 0.01), "even row at +0.5 spacing");

	// Marching along +Y: the right side is -X, the rows behind towards -Y.
	TArray<int> One;
	One.Add(2);
	HOLFormationRead::MakePlaces(One, FVector2D(0.0, 0.0), FVector2D(0.0, 1.0), 100.0, 100.0, Places);
	T.AssertTrue(Places.Num() == 2 && FMNear(Places[0], FVector2D(50.0, 0.0), 0.01), "turned: left end");
	T.AssertTrue(Places.Num() == 2 && FMNear(Places[1], FVector2D(-50.0, 0.0), 0.01), "turned: right end");
}

// HeroesOfLiberty.Formation.Assign
void Test_FMAssign(FUnitTest& T)
{
	TArray<int> PlaceOfUnit;
	TArray<FVector2D> NoPositions;
	TArray<FVector2D> NoPlaces;
	HOLFormationRead::AssignPlaces(NoPositions, NoPlaces, FVector2D(1.0, 0.0), PlaceOfUnit);
	T.AssertEquals(0, PlaceOfUnit.Num(), "no unit, nothing given");

	// A group already in the formation's shape keeps it: each unit to its own place, moved forward.
	TArray<int> Widths;
	TArray<FVector2D> Places;
	HOLFormationRead::MakeSquareRows(12, Widths);
	HOLFormationRead::MakePlaces(Widths, FVector2D(5000.0, 0.0), FVector2D(1.0, 0.0), 150.0, 150.0, Places);
	TArray<FVector2D> Positions;
	for (int I = 0; I < Places.Num(); I++)
	{
		Positions.Add(Places[I] - FVector2D(3000.0, 0.0));
	}
	HOLFormationRead::AssignPlaces(Positions, Places, FVector2D(1.0, 0.0), PlaceOfUnit);
	bool bKept = PlaceOfUnit.Num() == Places.Num();
	for (int Unit = 0; bKept && Unit < PlaceOfUnit.Num(); Unit++)
	{
		bKept = PlaceOfUnit[Unit] == Unit;
	}
	T.AssertTrue(bKept, "a group in shape keeps its shape");

	// Scattered groups, near and far, ahead and on the side: every unit gets its own place and no two paths cross.
	FRandomStream Random(7);
	for (int Trial = 0; Trial < 40; Trial++)
	{
		int Count = Random.RandRange(2, 60);
		int ExtraPlaces = 0;
		if (Trial % 4 == 0)
		{
			ExtraPlaces = Random.RandRange(1, 5);
		}
		double ForwardX = Random.RandRange(-1.0, 1.0);
		double ForwardY = Random.RandRange(-1.0, 1.0);
		FVector2D Forward(ForwardX, ForwardY);
		HOLFormationRead::MakeMixedRows(Count + ExtraPlaces, Widths);
		double TargetX = Random.RandRange(-3000.0, 3000.0);
		double TargetY = Random.RandRange(-3000.0, 3000.0);
		HOLFormationRead::MakePlaces(Widths, FVector2D(TargetX, TargetY), Forward, 150.0, 150.0, Places);
		Positions.Reset();
		double Spread = Random.RandRange(300.0, 4000.0);
		for (int Unit = 0; Unit < Count; Unit++)
		{
			double PosX = Random.RandRange(-Spread, Spread);
			double PosY = Random.RandRange(-Spread, Spread);
			Positions.Add(FVector2D(PosX, PosY));
		}
		HOLFormationRead::AssignPlaces(Positions, Places, Forward, PlaceOfUnit);

		TArray<bool> Given;
		for (int P = 0; P < Places.Num(); P++)
		{
			Given.Add(false);
		}
		bool bAllGiven = PlaceOfUnit.Num() == Count;
		for (int I = 0; bAllGiven && I < PlaceOfUnit.Num(); I++)
		{
			int Place = PlaceOfUnit[I];
			bAllGiven = Place >= 0 && Place < Places.Num() && !Given[Place];
			if (bAllGiven)
			{
				Given[Place] = true;
			}
		}
		T.AssertTrue(bAllGiven, f"trial {Trial}: a place of its own for each of {Count} units");
		if (!bAllGiven)
		{
			continue;
		}
		// By depth (J7c.2): rows filled front first (120 cm out of order at most), no two paths of one row cross.
		TArray<int> RowOfPlace;
		for (int Row = 0; Row < Widths.Num(); Row++)
		{
			for (int Column = 0; Column < Widths[Row]; Column++)
			{
				RowOfPlace.Add(Row);
			}
		}
		double Length = Math::Sqrt(Forward.X * Forward.X + Forward.Y * Forward.Y);
		FVector2D Front = FVector2D(1.0, 0.0);
		if (Length > 0.00000001)
		{
			Front = FVector2D(Forward.X / Length, Forward.Y / Length);
		}
		int Crossings = 0;
		int OutOfOrder = 0;
		for (int First = 0; First < Count; First++)
		{
			for (int Second = First + 1; Second < Count; Second++)
			{
				int RowA = RowOfPlace[PlaceOfUnit[First]];
				int RowB = RowOfPlace[PlaceOfUnit[Second]];
				if (RowA == RowB)
				{
					if (FMCross(Positions[First], Places[PlaceOfUnit[First]], Positions[Second], Places[PlaceOfUnit[Second]]))
					{
						Crossings++;
					}
				}
				else
				{
					int Ahead = Second;
					int Behind = First;
					if (RowA < RowB)
					{
						Ahead = First;
						Behind = Second;
					}
					double DepthAhead = Positions[Ahead].X * Front.X + Positions[Ahead].Y * Front.Y;
					double DepthBehind = Positions[Behind].X * Front.X + Positions[Behind].Y * Front.Y;
					if (DepthAhead < DepthBehind - 120.5)
					{
						OutOfOrder++;
					}
				}
			}
		}
		T.AssertEquals(0, Crossings, f"trial {Trial}: no paths of one row cross ({Count} units, {Places.Num()} places)");
		T.AssertEquals(0, OutOfOrder, f"trial {Trial}: the rows filled front first");
	}
}

// HeroesOfLiberty.Formation.Front
void Test_FMFront(FUnitTest& T)
{
	// The front's width from the right-button drag (J7a.4): a column at each end, one every 150 cm.
	T.AssertEquals(1, HOLFormationRead::GetFrontWidth(0.0, 150.0, 10), "drag of 0 cm: 1 column");
	T.AssertEquals(1, HOLFormationRead::GetFrontWidth(149.0, 150.0, 10), "drag of 149 cm: 1 column");
	T.AssertEquals(2, HOLFormationRead::GetFrontWidth(150.0, 150.0, 10), "drag of 150 cm: 2 columns");
	T.AssertEquals(5, HOLFormationRead::GetFrontWidth(600.0, 150.0, 10), "drag of 600 cm: 5 columns");
	T.AssertEquals(6, HOLFormationRead::GetFrontWidth(3000.0, 150.0, 6), "drag of 3000 cm, 6 units: capped at 6");
	T.AssertEquals(1, HOLFormationRead::GetFrontWidth(-50.0, 150.0, 6), "negative length: 1 column");

	// Facing (the user's 2nd trial): along the arrow, from the press to the mouse.
	T.AssertTrue(FMNear(HOLFormationRead::GetFrontFacing(FVector2D(0.0, 300.0)), FVector2D(0.0, 1.0), 0.000001), "arrow along +Y: faces +Y");
	T.AssertTrue(FMNear(HOLFormationRead::GetFrontFacing(FVector2D(-500.0, 0.0)), FVector2D(-1.0, 0.0), 0.000001), "arrow along -X: faces -X");
	double Diagonal = 1.0 / Math::Sqrt(2.0);
	T.AssertTrue(FMNear(HOLFormationRead::GetFrontFacing(FVector2D(300.0, 300.0)), FVector2D(Diagonal, Diagonal), 0.000001), "diagonal arrow: normalised");
	T.AssertTrue(FMNear(HOLFormationRead::GetFrontFacing(FVector2D(0.0, 0.0)), FVector2D(1.0, 0.0), 0.000001), "null arrow: +X");

	// The drag's width replaces the default: 12 Spearmen and 12 Men-at-arms on a 1050 cm front (8 columns): 8/8/8.
	TArray<FVector2D> Positions;
	TArray<int> KindOfUnit;
	for (int Unit = 0; Unit < 24; Unit++)
	{
		Positions.Add(FVector2D(-1000.0 - 200.0 * (Unit / 6), 200.0 * (Unit % 6)));
		if (Unit < 12)
		{
			KindOfUnit.Add(0);
		}
		else
		{
			KindOfUnit.Add(1);
		}
	}
	TArray<FVector2D> Places;
	TArray<int> PlaceOfUnit;
	int Width = HOLFormationRead::GetFrontWidth(1050.0, 150.0, Positions.Num());
	HOLFormationRead::PlaceGroup(Positions, KindOfUnit, false, FVector2D(0.0, 500.0), FVector2D(1.0, 0.0), 150.0, 180.0, Width, Places, PlaceOfUnit);
	T.AssertEquals(8, Width, "8 columns");
	T.AssertEquals(24, Places.Num(), "24 places");
	int FrontSpearmen = 0;
	bool bInRows = PlaceOfUnit.Num() == 24;
	for (int Unit = 0; bInRows && Unit < 24; Unit++)
	{
		FVector2D Place = Places[PlaceOfUnit[Unit]];
		bInRows = FMNearly(Place.X, 0.0, 0.5) || FMNearly(Place.X, -180.0, 0.5) || FMNearly(Place.X, -360.0, 0.5);
		bInRows = bInRows && Math::Abs(Place.Y - 500.0) <= 3.5 * 150.0 + 0.5;
		if (KindOfUnit[Unit] == 0 && FMNearly(Place.X, 0.0, 0.5))
		{
			FrontSpearmen++;
		}
	}
	T.AssertTrue(bInRows, "3 rows of 8, centred on the drag's middle");
	T.AssertEquals(8, FrontSpearmen, "the front row: 8 Spearmen");
}

// HeroesOfLiberty.Formation.Speed
void Test_FMSpeed(FUnitTest& T)
{
	// A group in formation walks at its slowest walker's speed (J7a.4); no group (0), its own speed.
	T.AssertTrue(FMNearly(HOLFormationRead::GetMarchSpeed(467.0, 420.0), 420.0, 0.001), "a fast unit of a slower group: the group's speed");
	T.AssertTrue(FMNearly(HOLFormationRead::GetMarchSpeed(420.0, 420.0), 420.0, 0.001), "the slowest itself: its own speed");
	T.AssertTrue(FMNearly(HOLFormationRead::GetMarchSpeed(300.0, 420.0), 300.0, 0.001), "never faster than its own");
	T.AssertTrue(FMNearly(HOLFormationRead::GetMarchSpeed(467.0, 0.0), 467.0, 0.001), "no group: its own speed");
}

// HeroesOfLiberty.Formation.Return
void Test_FMReturn(FUnitTest& T)
{
	// Back to its place after a push (J7a.4): radius 60 cm, the place at the origin.
	double Radius = 60.0;
	FVector2D Place(0.0, 0.0);
	int RestSteps = int(HOLFormationRead::GetConstant(n"ReturnRestSteps"));
	int MaxTries = int(HOLFormationRead::GetConstant(n"MaxReturnTries"));
	FHOLPlaceHoldView Hold;
	HOLFormationRead::EndWalkToPlace(Hold, FVector2D(30.0, 0.0), Place, Radius);
	T.AssertTrue(Hold.bHeld, "stopped 30 cm from its place: it holds it");

	// Standing on it, or nudged less than a radius: never a walk back.
	bool bWalked = false;
	for (int Step = 0; Step < 100; Step++)
	{
		double NudgeX = -50.0;
		if (Step % 2 == 0)
		{
			NudgeX = 50.0;
		}
		if (HOLFormationRead::WatchPlace(Hold, FVector2D(NudgeX, 0.0), Place, Radius))
		{
			bWalked = true;
		}
	}
	T.AssertFalse(bWalked, "nudged 50 cm about its place (two idle neighbours): it stays");

	// Pushed 150 cm away: while the push goes on it waits; then 5 steps unpushed, it walks back, even while it still drifts a little.
	int FirstWalk = -1;
	for (int Step = 0; Step < 60 && FirstWalk == -1; Step++)
	{
		FVector2D At = FVector2D(150.0, 1.5 * (Step - 20));
		if (Step < 20)
		{
			At = FVector2D(100.0 + 2.5 * Step, 0.0);
		}
		if (HOLFormationRead::WatchPlace(Hold, At, Place, Radius))
		{
			FirstWalk = Step;
		}
	}
	T.AssertEquals(20 + RestSteps, FirstWalk, "walks back 5 steps (0.25 s) after the push, drifting or not");
	T.AssertTrue(Hold.bReturning, "on its way back");
	T.AssertFalse(HOLFormationRead::WatchPlace(Hold, FVector2D(150.0, 0.0), Place, Radius), "no second walk while it returns");
	HOLFormationRead::EndWalkToPlace(Hold, FVector2D(10.0, 0.0), Place, Radius);
	T.AssertTrue(Hold.bHeld && !Hold.bReturning, "back on it: it holds it again");

	// A shove (10 cm in a step) is a push, even sideways.
	HOLFormationRead::WatchPlace(Hold, FVector2D(200.0, 0.0), Place, Radius);
	int Walk = -1;
	for (int Step = 0; Step < 10 && Walk == -1; Step++)
	{
		double SideY = 30.0;
		if (Step < 3)
		{
			SideY = 10.0 * (Step + 1);
		}
		if (HOLFormationRead::WatchPlace(Hold, FVector2D(200.0, SideY), Place, Radius))
		{
			Walk = Step;
		}
	}
	T.AssertEquals(2 + RestSteps, Walk, "shoved sideways 3 steps: walks back 5 steps after");

	// Walks back that end short (the place taken): tried again, MaxReturnTries in a row at most, then let go.
	int Walks = 1;
	for (int Try = 1; Try < 10 && Hold.bHeld; Try++)
	{
		HOLFormationRead::EndWalkToPlace(Hold, FVector2D(120.0, 0.0), Place, Radius);
		for (int Step = 0; Step < 40 && Hold.bHeld && !Hold.bReturning; Step++)
		{
			if (HOLFormationRead::WatchPlace(Hold, FVector2D(120.0, 0.0), Place, Radius))
			{
				Walks++;
			}
		}
	}
	T.AssertEquals(MaxTries, Walks, "ends short each time: walks back 3 times in all");
	T.AssertFalse(Hold.bHeld, "then lets its place go");
	bool bWalkedAgain = false;
	for (int Step = 0; Step < 100; Step++)
	{
		if (HOLFormationRead::WatchPlace(Hold, FVector2D(120.0, 0.0), Place, Radius))
		{
			bWalkedAgain = true;
		}
	}
	T.AssertFalse(bWalkedAgain, "then never walks back");

	// An arrival far from its place (taken, out of reach): no hold.
	HOLFormationRead::EndWalkToPlace(Hold, FVector2D(0.0, 90.0), Place, Radius);
	T.AssertFalse(Hold.bHeld, "arrived 90 cm from its place: does not hold it");
}

// HeroesOfLiberty.Formation.YieldPlace
void Test_FMYieldPlace(FUnitTest& T)
{
	// The newest order wins (MV.4, choice A): two bodies of 60 cm, new places 150 cm apart, sorted by X.
	double RadiusSum = 120.0;
	TArray<FVector2D> NewPlaces;
	NewPlaces.Add(FVector2D(-150.0, 0.0));
	NewPlaces.Add(FVector2D(0.0, 0.0));
	NewPlaces.Add(FVector2D(0.0, 150.0));
	NewPlaces.Add(FVector2D(150.0, 0.0));
	T.AssertTrue(HOLFormationRead::ShouldYieldHeldPlace(FVector2D(75.0, 30.0), NewPlaces, RadiusSum), "held place 80 cm from a new one: let go");
	T.AssertTrue(HOLFormationRead::ShouldYieldHeldPlace(FVector2D(150.0, 0.0), NewPlaces, RadiusSum), "on a new place: let go");
	T.AssertTrue(HOLFormationRead::ShouldYieldHeldPlace(FVector2D(10.0, 200.0), NewPlaces, RadiusSum), "found among places sorted by X, not only the first");
	T.AssertFalse(HOLFormationRead::ShouldYieldHeldPlace(FVector2D(270.0, 0.0), NewPlaces, RadiusSum), "two bodies away exactly: both fit, kept");
	T.AssertFalse(HOLFormationRead::ShouldYieldHeldPlace(FVector2D(-75.0, -150.0), NewPlaces, RadiusSum), "a row behind the new ones, 167 cm from the nearest: kept");
	TArray<FVector2D> NoPlaces;
	T.AssertFalse(HOLFormationRead::ShouldYieldHeldPlace(FVector2D(0.0, 0.0), NoPlaces, RadiusSum), "no new place: kept");
}

// Corridor data of the C++ lambda "Make": 50 samples, free 1000 cm each side, an obstacle over samples 20 to 29.
void FMMakeCorridor(FUnitTest& T, double Left, double Right, double MaxShift, TArray<double>& Shift, TArray<double>& Squeeze)
{
	double Step = 100.0;
	double Slope = Math::Tan(Math::DegreesToRadians(HOLFormationRead::GetConstant(n"FileSlideMaxAngle")));
	double MinY = -150.0;
	double MaxY = 150.0;
	TArray<double> FreeLeft;
	TArray<double> FreeRight;
	for (int I = 0; I < 50; I++)
	{
		FreeLeft.Add(1000.0);
		FreeRight.Add(1000.0);
	}
	for (int Sample = 20; Sample < 30; Sample++)
	{
		FreeLeft[Sample] = Left;
		FreeRight[Sample] = Right;
	}
	HOLFormationRead::MakeCorridor(FreeLeft, FreeRight, Step, MinY, MaxY, MaxShift, Slope, Shift, Squeeze);
	bool bInside = Shift.Num() == 50 && Squeeze.Num() == 50;
	bool bSlope = bInside;
	for (int Sample = 0; bInside && Sample < 50; Sample++)
	{
		bInside = bInside && Shift[Sample] + Squeeze[Sample] * MinY >= -FreeLeft[Sample] - 0.000001
			&& Shift[Sample] + Squeeze[Sample] * MaxY <= FreeRight[Sample] + 0.000001 && Math::Abs(Shift[Sample]) <= MaxShift + 0.000001;
		if (Sample > 0)
		{
			bSlope = bSlope && Math::Abs(Shift[Sample] - Shift[Sample - 1]) <= Slope * Step + 0.000001
				&& Math::Abs(Squeeze[Sample] - Squeeze[Sample - 1]) * MaxY <= Slope * Step + 0.000001;
		}
	}
	T.AssertTrue(bInside, "files inside the free interval");
	T.AssertTrue(bSlope, "slope bounded");
}

// HeroesOfLiberty.Formation.Corridor
void Test_FMCorridor(FUnitTest& T)
{
	TArray<double> Shift;
	TArray<double> Squeeze;
	// Open field: nothing changes.
	FMMakeCorridor(T, 1000.0, 1000.0, 300.0, Shift, Squeeze);
	bool bOpen = Shift.Num() == 50 && Squeeze.Num() == 50;
	for (int Sample = 0; bOpen && Sample < Shift.Num(); Sample++)
	{
		bOpen = FMNearly(Shift[Sample], 0.0, 0.0001) && FMNearly(Squeeze[Sample], 1.0, 0.0001);
	}
	T.AssertTrue(bOpen, "open field: no shift, no squeeze");
	// B: an obstacle 60 cm right of the path, room on the left: the block shifts left, 90 cm, ahead of time.
	FMMakeCorridor(T, 1000.0, 60.0, 300.0, Shift, Squeeze);
	if (Shift.Num() != 50 || Squeeze.Num() != 50)
	{
		T.AssertTrue(false, "B: 50 samples");
		return;
	}
	bool bWhole = true;
	for (int Sample = 0; Sample < Squeeze.Num(); Sample++)
	{
		bWhole = bWhole && FMNearly(Squeeze[Sample], 1.0, 0.0001);
	}
	T.AssertTrue(bWhole, "B: no squeeze");
	T.AssertTrue(FMNearly(Shift[25], -90.0, 0.000001), "B: shifted left by what fits");
	T.AssertTrue(Shift[18] < -1.0, "B: the shift starts before the obstacle");
	T.AssertTrue(FMNearly(Shift[40], 0.0, 0.0001), "B: back on the path after it");
	// A: a gap 200 cm wide for 300 cm of files: they squeeze there, ahead of time, and reopen after.
	FMMakeCorridor(T, 100.0, 100.0, 300.0, Shift, Squeeze);
	T.AssertTrue(FMNearly(Squeeze[25], 2.0 / 3.0, 0.000001), "A: squeezed in the gap");
	T.AssertTrue(Squeeze[19] < 1.0 - 0.001, "A: the squeeze starts before the gap");
	T.AssertTrue(FMNearly(Squeeze[40], 1.0, 0.0001), "A: reopened after it");
	// The shift needed (150 cm) beyond the cap (100 cm): shifted by the cap, squeezed for the rest.
	FMMakeCorridor(T, 1000.0, 0.0, 100.0, Shift, Squeeze);
	T.AssertTrue(FMNearly(Shift[25], -100.0, 0.000001), "cap: shifted by the cap");
	T.AssertTrue(FMNearly(Squeeze[25], 100.0 / 150.0, 0.000001), "cap: squeezed for the rest");
	double At = 0.0;
	double Scale = 0.0;
	HOLFormationRead::GetCorridorAt(Shift, Squeeze, 100.0, 2550.0, At, Scale);
	T.AssertTrue(FMNearly(At, -100.0, 0.000001), "read between samples");
	TArray<double> NoShift;
	TArray<double> NoSqueeze;
	HOLFormationRead::GetCorridorAt(NoShift, NoSqueeze, 100.0, 100.0, At, Scale);
	T.AssertTrue(At == 0.0 && Scale == 1.0, "no corridor: no change");
}

// HeroesOfLiberty.Formation.BreakAtMelee
void Test_FMBreakAtMelee(FUnitTest& T)
{
	// MV.4.4: the block breaks up at the melee (an enemy or a fighting friend within BlockBreakDistance), not before.
	double Break = HOLFormationRead::GetConstant(n"BlockBreakDistance");
	T.AssertFalse(HOLFormationRead::ShouldBreakBlock(true, -1.0, -1.0), "nobody around: the block holds");
	T.AssertFalse(HOLFormationRead::ShouldBreakBlock(true, Break + 1.0, -1.0), "an enemy far: the block holds");
	T.AssertFalse(HOLFormationRead::ShouldBreakBlock(true, -1.0, Break + 1.0), "a fighting friend far: the block holds");
	T.AssertTrue(HOLFormationRead::ShouldBreakBlock(true, Break - 1.0, -1.0), "an enemy close: the block breaks up");
	T.AssertTrue(HOLFormationRead::ShouldBreakBlock(true, -1.0, Break - 1.0), "a fighting friend close: the block breaks up");
	T.AssertTrue(HOLFormationRead::ShouldBreakBlock(true, 0.0, -1.0), "touching (0 cm): the block breaks up");
	// An attack-move's block never breaks up (its line holds its rows, MV.4.5); only a plain move's does.
	T.AssertFalse(HOLFormationRead::ShouldBreakBlock(false, 0.0, -1.0), "an attack-move close to an enemy: the block holds");
	T.AssertFalse(HOLFormationRead::ShouldBreakBlock(false, -1.0, 0.0), "an attack-move close to a striking friend: the block holds");
	// A friend fighting is one striking, not one with only a target or nearing it.
	T.AssertTrue(HOLFormationRead::IsFightingFriend(true, false, true), "a friend striking: fighting");
	T.AssertFalse(HOLFormationRead::IsFightingFriend(false, true, true), "a friend nearing its target: not fighting");
	T.AssertFalse(HOLFormationRead::IsFightingFriend(false, false, true), "a friend with only a target: not fighting");
	T.AssertFalse(HOLFormationRead::IsFightingFriend(false, false, false), "a friend idle: not fighting");
	// The safety net: a member holding the re-forming block for 2 s no longer holds the others.
	int Stuck = int(HOLFormationRead::GetConstant(n"BlockMemberStuckSteps"));
	T.AssertFalse(HOLFormationRead::IsMemberStuck(Stuck - 1), "holding just under 0.5 s: still in the block");
	T.AssertTrue(HOLFormationRead::IsMemberStuck(Stuck), "holding 0.5 s: it leaves the block");
	T.AssertEquals(10, Stuck, "0.5 s at 20 steps a second");
}

// ---- GALADRIEL-1117q (NOT COMPILED): the 4 remaining ports, same vectors as HOLFormationTests.cpp (AssignByDepth, Group, Block, Settle) ----

double FMDist(const FVector2D& A, const FVector2D& B)
{
	double Dx = A.X - B.X;
	double Dy = A.Y - B.Y;
	return Math::Sqrt(Dx * Dx + Dy * Dy);
}

TArray<int> FMRowOfPlace(const TArray<int>& Widths)
{
	TArray<int> Rows;
	for (int Row = 0; Row < Widths.Num(); Row++)
	{
		for (int Column = 0; Column < Widths[Row]; Column++)
		{
			Rows.Add(Row);
		}
	}
	return Rows;
}

// HeroesOfLiberty.Formation.AssignByDepth
void Test_FMAssignByDepth(FUnitTest& T)
{
	// The user trial (J7c.2): 50 Archers in rows 9 9 9 9 9 5 facing South, sent North on the spot.
	double Spacing = 150.0;
	TArray<int> Widths;
	Widths.Add(9); Widths.Add(9); Widths.Add(9); Widths.Add(9); Widths.Add(9); Widths.Add(5);
	TArray<int> RowOfPlace = FMRowOfPlace(Widths);
	FVector2D South(0.0, -1.0);
	FVector2D North(0.0, 1.0);
	TArray<FVector2D> Positions;
	HOLFormationRead::MakePlaces(Widths, FVector2D(1000.0, 2000.0), South, Spacing, Spacing, Positions);
	TArray<FVector2D> Places;
	HOLFormationRead::MakePlaces(Widths, FVector2D(1000.0, 2000.0 + 5.0 * Spacing), North, Spacing, Spacing, Places);
	TArray<int> PlaceOfUnit;
	HOLFormationRead::AssignPlaces(Positions, Places, North, PlaceOfUnit);
	TArray<bool> Given;
	Given.SetNum(50);
	int Distinct = 0;
	for (int Unit = 0; Unit < PlaceOfUnit.Num(); Unit++)
	{
		if (PlaceOfUnit[Unit] >= 0 && PlaceOfUnit[Unit] < 50 && !Given[PlaceOfUnit[Unit]])
		{
			Given[PlaceOfUnit[Unit]] = true;
			Distinct++;
		}
	}
	T.AssertEquals(50, Distinct, "half turn: a place of its own for each of 50");
	double LongestStep = 0.0;
	bool bLastRowTurnsOnly = true;
	bool bFrontRowEndsLast = true;
	int Crossings = 0;
	for (int Unit = 0; Unit < Positions.Num() && Unit < PlaceOfUnit.Num(); Unit++)
	{
		int Mine = PlaceOfUnit[Unit];
		double Step = FMDist(Positions[Unit], Places[Mine]);
		double Dy = Math::Abs(Places[Mine].Y - Positions[Unit].Y);
		if (Dy > LongestStep)
		{
			LongestStep = Dy;
		}
		if (RowOfPlace[Unit] == 5)
		{
			bLastRowTurnsOnly = bLastRowTurnsOnly && RowOfPlace[Mine] == 0 && Step < 1.0;
		}
		if (RowOfPlace[Mine] == 5)
		{
			bFrontRowEndsLast = bFrontRowEndsLast && RowOfPlace[Unit] == 0 && Step < 1.0;
		}
		for (int Other = Unit + 1; Other < Positions.Num(); Other++)
		{
			if (RowOfPlace[Mine] == RowOfPlace[PlaceOfUnit[Other]] && FMCross(Positions[Unit], Places[Mine], Positions[Other], Places[PlaceOfUnit[Other]]))
			{
				Crossings++;
			}
		}
	}
	T.AssertTrue(LongestStep <= Spacing + 0.5, "half turn: no unit moves more than one row deep");
	T.AssertTrue(bLastRowTurnsOnly, "half turn: the old last row turns on the spot in the new front row");
	T.AssertTrue(bFrontRowEndsLast, "half turn: the new last row is the old front row middle, on the spot");
	T.AssertEquals(0, Crossings, "half turn: no paths of one row cross");

	// A march straight ahead (same facing): each unit keeps its row and its column.
	TArray<FVector2D> Ahead;
	HOLFormationRead::MakePlaces(Widths, FVector2D(1000.0, -3000.0), South, Spacing, Spacing, Ahead);
	HOLFormationRead::AssignPlaces(Positions, Ahead, South, PlaceOfUnit);
	bool bKept = PlaceOfUnit.Num() == Positions.Num();
	for (int Unit = 0; bKept && Unit < PlaceOfUnit.Num(); Unit++)
	{
		bKept = PlaceOfUnit[Unit] == Unit;
	}
	T.AssertTrue(bKept, "straight ahead: each unit keeps its row and column");

	// A row with a hole: 49 units on 49 places, the last row third place missing, the old last row 5th unit left behind.
	TArray<FVector2D> Holed = Ahead;
	Holed.RemoveAt(45 + 2);
	TArray<FVector2D> Four = Positions;
	Four.RemoveAt(45 + 4);
	HOLFormationRead::AssignPlaces(Four, Holed, South, PlaceOfUnit);
	TArray<bool> HoledGiven;
	HoledGiven.SetNum(49);
	int HoledDistinct = 0;
	for (int Unit = 0; Unit < PlaceOfUnit.Num(); Unit++)
	{
		if (PlaceOfUnit[Unit] >= 0 && PlaceOfUnit[Unit] < 49 && !HoledGiven[PlaceOfUnit[Unit]])
		{
			HoledGiven[PlaceOfUnit[Unit]] = true;
			HoledDistinct++;
		}
	}
	T.AssertEquals(49, HoledDistinct, "holed row: a place of its own for each of 49");
	bool bFrontKept = PlaceOfUnit.Num() == 49;
	for (int Unit = 0; bFrontKept && Unit < 45; Unit++)
	{
		bFrontKept = PlaceOfUnit[Unit] == Unit;
	}
	T.AssertTrue(bFrontKept, "holed row: the rows of 9 keep their places");
	bool bLastInOrder = PlaceOfUnit.Num() == 49 && PlaceOfUnit[45] == 45 && PlaceOfUnit[46] == 46 && PlaceOfUnit[47] == 47 && PlaceOfUnit[48] == 48;
	T.AssertTrue(bLastInOrder, "holed row: the last row units fill its places in order");
}

// HeroesOfLiberty.Formation.Group
void Test_FMGroup(FUnitTest& T)
{
	TArray<int> Widths;
	TArray<int> Counts;
	Counts.Add(15);
	HOLFormationRead::MakeGroupRows(Counts, true, Widths);
	T.AssertEquals("6/6/3", FMWidthsText(Widths), "15 Peasants: 6/6/3");
	Counts.Empty();
	Counts.Add(0); Counts.Add(12); Counts.Add(0);
	HOLFormationRead::MakeGroupRows(Counts, false, Widths);
	T.AssertEquals("5/5/2", FMWidthsText(Widths), "12 Men-at-arms alone: 5/5/2");
	Counts.Empty();
	Counts.Add(12); Counts.Add(12);
	HOLFormationRead::MakeGroupRows(Counts, false, Widths);
	T.AssertEquals("7/7/5/5", FMWidthsText(Widths), "12 Spearmen and 12 Men-at-arms: 7/7/5/5");

	// 3 Spearmen and 3 Men-at-arms marching along +X to (5000, 0), mixed up.
	TArray<FVector2D> Positions;
	Positions.Add(FVector2D(0.0, 0.0)); Positions.Add(FVector2D(0.0, 200.0)); Positions.Add(FVector2D(200.0, 0.0));
	Positions.Add(FVector2D(200.0, 200.0)); Positions.Add(FVector2D(-200.0, 0.0)); Positions.Add(FVector2D(-200.0, 200.0));
	TArray<int> KindOfUnit;
	KindOfUnit.Add(1); KindOfUnit.Add(0); KindOfUnit.Add(0); KindOfUnit.Add(1); KindOfUnit.Add(1); KindOfUnit.Add(0);
	TArray<FVector2D> Places;
	TArray<int> PlaceOfUnit;
	int Projected = 0;
	HOLFormationRead::PlaceGroupClamped(Positions, KindOfUnit, false, FVector2D(5000.0, 0.0), FVector2D(1.0, 0.0), 150.0, 180.0, 1.0e12, Projected, Places, PlaceOfUnit);
	T.AssertEquals(Places.Num(), Projected, "one projection per place");
	T.AssertEquals(6, Places.Num(), "6 places");
	bool bOwn = PlaceOfUnit.Num() == 6;
	TArray<bool> Taken;
	Taken.SetNum(6);
	for (int Unit = 0; bOwn && Unit < 6; Unit++)
	{
		int Place = PlaceOfUnit[Unit];
		bOwn = Place >= 0 && Place < Places.Num() && Place < 6 && !Taken[Place];
		if (bOwn)
		{
			Taken[Place] = true;
			double WantedX = 4820.0;
			if (KindOfUnit[Unit] == 0)
			{
				WantedX = 5000.0;
			}
			bOwn = FMNearly(Places[Place].X, WantedX, 0.5);
		}
	}
	T.AssertTrue(bOwn, "each unit a place of its own, Spearmen in front, Men-at-arms 180 cm behind");

	// A place the navmesh does not have is brought back (X clamped to 100).
	TArray<FVector2D> FivePositions;
	TArray<int> Peasants;
	for (int I = 0; I < 5; I++)
	{
		FivePositions.Add(Positions[I]);
		Peasants.Add(4);
	}
	HOLFormationRead::PlaceGroupClamped(FivePositions, Peasants, true, FVector2D(0.0, 1000.0), FVector2D(0.0, 1.0), 150.0, 180.0, 100.0, Projected, Places, PlaceOfUnit);
	bool bClamped = Places.Num() == 5 && PlaceOfUnit.Num() == 5 && !PlaceOfUnit.Contains(-1);
	for (int I = 0; I < Places.Num(); I++)
	{
		bClamped = bClamped && Places[I].X <= 100.0 && FMNearly(Places[I].Y, 1000.0, 0.5);
	}
	T.AssertTrue(bClamped, "5 Peasants: one row across the march, the projected places");
}

// One arrival by kind: returns the crossings within a kind, asserts that each unit keeps a place of its kind.
int FMRunByKind(FUnitTest& T, const TArray<FVector2D>& Standing, const TArray<int>& Kinds, const TArray<FVector2D>& Places, const FVector2D& Facing, TArray<int>& PlaceOfUnit)
{
	HOLFormationRead::AssignPlacesByKind(Standing, Kinds, Places, Facing, PlaceOfUnit);
	int Crossings = 0;
	for (int First = 0; First < Standing.Num(); First++)
	{
		T.AssertEquals(Kinds[First], Kinds[PlaceOfUnit[First]], f"unit {First} keeps a place of its kind");
		for (int Second = First + 1; Second < Standing.Num(); Second++)
		{
			if (Kinds[First] == Kinds[Second] && FMCross(Standing[First], Places[PlaceOfUnit[First]], Standing[Second], Places[PlaceOfUnit[Second]]))
			{
				Crossings++;
			}
		}
	}
	return Crossings;
}

TArray<FVector2D> FMLocalSlots(double X0, double Y0, double X1, double Y1, double X2, double Y2, double X3, double Y3, double X4, double Y4, double X5, double Y5)
{
	TArray<FVector2D> Result;
	Result.Add(FVector2D(X0, Y0)); Result.Add(FVector2D(X1, Y1)); Result.Add(FVector2D(X2, Y2));
	Result.Add(FVector2D(X3, Y3)); Result.Add(FVector2D(X4, Y4)); Result.Add(FVector2D(X5, Y5));
	return Result;
}

// HeroesOfLiberty.Formation.Block
void Test_FMBlock(FUnitTest& T)
{
	// The block march (J7a.4): a unit slot is the block point on the path plus its offset (ahead, right) turned with the path.
	TArray<FVector2D> Straight;
	Straight.Add(FVector2D(0.0, 0.0)); Straight.Add(FVector2D(1000.0, 0.0));
	FVector2D Fallback(1.0, 0.0);
	T.AssertTrue(FMNear(HOLFormationRead::GetSlotOnPath(Straight, 500.0, FVector2D(-180.0, 150.0), Fallback), FVector2D(320.0, 150.0), 0.01), "straight: 180 cm behind, 150 to the right of the point 500 cm along");
	T.AssertTrue(FMNear(HOLFormationRead::GetPointOnLine(Straight, 3000.0), FVector2D(1000.0, 0.0), 0.01), "past the end: the end");
	T.AssertTrue(FMNear(HOLFormationRead::GetPointOnLine(Straight, -50.0), FVector2D(0.0, 0.0), 0.01), "before the start: the start");
	TArray<FVector2D> Corner;
	Corner.Add(FVector2D(0.0, 0.0)); Corner.Add(FVector2D(1000.0, 0.0)); Corner.Add(FVector2D(1000.0, 1000.0));
	T.AssertTrue(FMNearly(HOLFormationRead::GetLineLength(Corner), 2000.0, 0.01), "length of an L");
	double Window = HOLFormationRead::GetConstant(n"BlockTurnWindow");
	FVector2D AtCorner = HOLFormationRead::GetDirectionOnLine(Corner, 1000.0, Window, Fallback);
	T.AssertTrue(FMNear(AtCorner, FVector2D(1.0, 1.0).GetSafeNormal(), 0.001), "at the corner: half turned (45 degrees)");
	T.AssertTrue(FMNear(HOLFormationRead::GetSlotOnPath(Corner, 1500.0, FVector2D(0.0, 150.0), Fallback), FVector2D(850.0, 500.0), 0.01), "after the corner: along +Y, the right-hand side towards -X");
	// Two units of the block keep their gap on a straight part, wherever the block is.
	FVector2D A(0.0, -150.0);
	FVector2D B(-180.0, 150.0);
	TArray<double> Distances;
	Distances.Add(100.0); Distances.Add(400.0); Distances.Add(700.0);
	for (int I = 0; I < Distances.Num(); I++)
	{
		double Gap = FMDist(HOLFormationRead::GetSlotOnPath(Straight, Distances[I], A, Fallback), HOLFormationRead::GetSlotOnPath(Straight, Distances[I], B, Fallback));
		T.AssertTrue(FMNearly(Gap, FMDist(A, B), 0.01), f"gap kept at {Distances[I]} cm");
	}
	// Offsets back and forth between the block and the world.
	FVector2D Forward = FVector2D(3.0, 4.0).GetSafeNormal();
	FVector2D World = HOLFormationRead::BlockOffsetToWorld(B, Forward);
	T.AssertTrue(FMNear(HOLFormationRead::WorldToBlockOffset(World, Forward), B, 0.001), "offset to the world and back");
	TArray<FVector2D> Single;
	Single.Add(FVector2D(5.0, 5.0));
	T.AssertTrue(FMNear(HOLFormationRead::GetDirectionOnLine(Single, 0.0, 200.0, Fallback), Fallback, 0.001), "a single point: no direction, the fallback");

	// The arrival (the user 3rd trial): 3 soldiers (kind 0, front row at X = 0) and 3 Peasants (kind 1, at X = -180) marching along +X.
	TArray<FVector2D> Standing = FMLocalSlots(0.0, -150.0, 0.0, 0.0, 0.0, 150.0, -180.0, -150.0, -180.0, 0.0, -180.0, 150.0);
	TArray<int> Kinds;
	Kinds.Add(0); Kinds.Add(0); Kinds.Add(0); Kinds.Add(1); Kinds.Add(1); Kinds.Add(1);
	TArray<FVector2D> Ahead;
	for (int I = 0; I < Standing.Num(); I++)
	{
		Ahead.Add(FVector2D(Standing[I].X + 100.0, Standing[I].Y));
	}
	TArray<int> PlaceOfUnit;
	T.AssertEquals(0, FMRunByKind(T, Standing, Kinds, Ahead, FVector2D(1.0, 0.0), PlaceOfUnit), "same facing: no crossing");
	bool bOwnSlot = PlaceOfUnit.Num() == 6;
	for (int I = 0; bOwnSlot && I < 6; I++)
	{
		bOwnSlot = PlaceOfUnit[I] == I;
	}
	T.AssertTrue(bOwnSlot, "same facing: each to its own slot place");
	// A half turn in place (facing -X).
	TArray<FVector2D> Turned = FMLocalSlots(-180.0, 150.0, -180.0, 0.0, -180.0, -150.0, 0.0, 150.0, 0.0, 0.0, 0.0, -150.0);
	T.AssertEquals(0, FMRunByKind(T, Standing, Kinds, Turned, FVector2D(-1.0, 0.0), PlaceOfUnit), "half turn: no crossing within a kind");
	double Longest = 0.0;
	for (int Unit = 0; Unit < Standing.Num() && Unit < PlaceOfUnit.Num(); Unit++)
	{
		double Step = FMDist(Standing[Unit], Turned[PlaceOfUnit[Unit]]);
		if (Step > Longest)
		{
			Longest = Step;
		}
	}
	T.AssertTrue(Longest <= 181.0, "half turn: straight across the block, no wheel round");

	// The blended arrival (the user 4th trial).
	double Lag = HOLFormationRead::GetConstant(n"BlockLagDistance");
	double SoftHold = HOLFormationRead::GetConstant(n"BlockSoftHoldDistance");
	double BlendMargin = HOLFormationRead::GetConstant(n"BlockBlendMargin");
	T.AssertTrue(FMNearly(HOLFormationRead::GetBlockSoftPace(Lag), 1.0, 1.0e-9), "soft pace: full on the slots");
	T.AssertTrue(FMNearly(HOLFormationRead::GetBlockSoftPace(0.5 * (Lag + SoftHold)), 0.5, 1.0e-9), "soft pace: half way");
	T.AssertTrue(FMNearly(HOLFormationRead::GetBlockSoftPace(SoftHold + 50.0), 0.0, 1.0e-9), "soft pace: stopped far behind");
	T.AssertTrue(FMNearly(HOLFormationRead::GetBlockBlendDistance(400.0, 3500.0), 400.0 + BlendMargin, 1.0e-9), "blend over the block size + margin");
	T.AssertTrue(FMNearly(HOLFormationRead::GetBlockBlendDistance(400.0, 1000.0), 500.0, 1.0e-9), "blend: half the path at most");
	T.AssertTrue(FMNearly(HOLFormationRead::GetBlockBlend(800.0, 650.0), 0.0, 1.0e-9), "blend: none before");
	T.AssertTrue(FMNearly(HOLFormationRead::GetBlockBlend(325.0, 650.0), 0.5, 1.0e-9), "blend: half way");
	T.AssertTrue(FMNearly(HOLFormationRead::GetBlockBlend(0.0, 650.0), 1.0, 1.0e-9), "blend: all at the end");

	// Stragglers: five close together and one 10 m away; two clusters of three; a tight group; a wide block.
	double Join = HOLFormationRead::GetConstant(n"BlockJoinDistance");
	TArray<bool> IsStraggler;
	TArray<FVector2D> FiveAndOne = FMLocalSlots(0.0, 0.0, 150.0, 0.0, -150.0, 0.0, 0.0, 150.0, 150.0, 150.0, 1000.0, 0.0);
	T.AssertEquals(1, HOLFormationRead::FindStragglers(FiveAndOne, Join, IsStraggler), "stragglers: five and one 10 m away");
	T.AssertTrue(IsStraggler.Num() == 6 && IsStraggler[5] && !IsStraggler[0] && !IsStraggler[4], "stragglers: the one 10 m away");
	TArray<FVector2D> TwoClusters = FMLocalSlots(0.0, 0.0, 150.0, 0.0, 0.0, 150.0, 1500.0, 0.0, 1650.0, 0.0, 1500.0, 150.0);
	T.AssertEquals(0, HOLFormationRead::FindStragglers(TwoClusters, Join, IsStraggler), "stragglers: two clusters of three, none");
	bool bAnyMarked = false;
	for (int I = 0; I < IsStraggler.Num(); I++)
	{
		bAnyMarked = bAnyMarked || IsStraggler[I];
	}
	T.AssertFalse(bAnyMarked, "stragglers: two clusters of three, nobody marked");
	// The packets: largest first.
	TArray<int> PacketOfUnit;
	TArray<int> PacketSizes;
	TArray<FVector2D> TwoAt10 = FMLocalSlots(0.0, 0.0, 150.0, 0.0, 0.0, 150.0, 1000.0, 0.0, 1150.0, 0.0, 1000.0, 150.0);
	T.AssertEquals(2, HOLFormationRead::FindPackets(TwoAt10, Join, PacketOfUnit, PacketSizes), "packets: two of three 10 m apart");
	T.AssertTrue(PacketSizes.Num() == 2 && PacketSizes[0] == 3 && PacketSizes[1] == 3 && PacketOfUnit.Num() == 6 && PacketOfUnit[0] == 0 && PacketOfUnit[2] == 0 && PacketOfUnit[3] == 1 && PacketOfUnit[5] == 1, "packets: three and three, each its own");
	T.AssertEquals(2, HOLFormationRead::FindPackets(FiveAndOne, Join, PacketOfUnit, PacketSizes), "packets: five and one 10 m away");
	T.AssertTrue(PacketSizes.Num() == 2 && PacketSizes[0] == 5 && PacketSizes[1] == 1 && PacketOfUnit[5] == 1 && PacketOfUnit[0] == 0, "packets: five, then the one alone");
	TArray<FVector2D> Tight;
	for (int I = 0; I < 5; I++)
	{
		Tight.Add(FiveAndOne[I]);
	}
	T.AssertEquals(1, HOLFormationRead::FindPackets(Tight, Join, PacketOfUnit, PacketSizes), "packets: a tight group, one");
	TArray<FVector2D> Chain;
	for (int I = 0; I < 5; I++)
	{
		Chain.Add(FVector2D(400.0 * I, 0.0));
	}
	T.AssertEquals(1, HOLFormationRead::FindPackets(Chain, Join, PacketOfUnit, PacketSizes), "packets: a chain linked step by step, one");
	TArray<FVector2D> SmallFirst;
	SmallFirst.Add(FVector2D(1000.0, 0.0)); SmallFirst.Add(FVector2D(1150.0, 0.0)); SmallFirst.Add(FVector2D(0.0, 0.0)); SmallFirst.Add(FVector2D(150.0, 0.0)); SmallFirst.Add(FVector2D(0.0, 150.0));
	HOLFormationRead::FindPackets(SmallFirst, Join, PacketOfUnit, PacketSizes);
	T.AssertTrue(PacketSizes.Num() == 2 && PacketSizes[0] == 3 && PacketSizes[1] == 2 && PacketOfUnit.Num() == 5 && PacketOfUnit[0] == 1 && PacketOfUnit[1] == 1 && PacketOfUnit[2] == 0 && PacketOfUnit[3] == 0 && PacketOfUnit[4] == 0, "packets: the larger is packet 0");
	TArray<FVector2D> Apart;
	Apart.Add(FVector2D(0.0, 0.0)); Apart.Add(FVector2D(1000.0, 0.0)); Apart.Add(FVector2D(2000.0, 0.0));
	T.AssertEquals(0, HOLFormationRead::FindStragglers(Apart, Join, IsStraggler), "stragglers: everyone apart, none");
	T.AssertEquals(0, HOLFormationRead::FindStragglers(Tight, Join, IsStraggler), "stragglers: a tight group, none");
	TArray<FVector2D> WideBlock;
	for (int Row = 0; Row < 5; Row++)
	{
		for (int Column = 0; Column < 10; Column++)
		{
			WideBlock.Add(FVector2D(Row * 180.0, Column * 150.0));
		}
	}
	T.AssertEquals(0, HOLFormationRead::FindStragglers(WideBlock, Join, IsStraggler), "stragglers: a block 13.5 m wide, none");

	// A slot kept for each straggler.
	TArray<FVector2D> Slots = FMLocalSlots(0.0, -150.0, 0.0, 0.0, 0.0, 150.0, -180.0, -150.0, -180.0, 0.0, -180.0, 150.0);
	TArray<FVector2D> Local = FMLocalSlots(0.0, -150.0, 0.0, 0.0, 0.0, 150.0, -180.0, -150.0, -180.0, 0.0, -1000.0, 300.0);
	TArray<bool> LastIsStraggler;
	LastIsStraggler.Add(false); LastIsStraggler.Add(false); LastIsStraggler.Add(false); LastIsStraggler.Add(false); LastIsStraggler.Add(false); LastIsStraggler.Add(true);
	TArray<int> SlotOfUnit;
	TArray<bool> Queued;
	TArray<int> OneKind;
	TArray<int> TwoKinds;
	for (int I = 0; I < 6; I++)
	{
		OneKind.Add(0);
		TwoKinds.Add(I < 3 ? 0 : 1);
	}
	T.AssertEquals(0, HOLFormationRead::ReserveRearSlots(Slots, Local, OneKind, LastIsStraggler, SlotOfUnit, Queued), "rear slots: one kind, none queued");
	T.AssertTrue(SlotOfUnit.Num() == 6 && Queued.Num() == 6 && SlotOfUnit[5] == 5 && !Queued[5], "rear slots: the straggler in the last row on its side");
	TArray<FVector2D> LeftLocal = FMLocalSlots(0.0, -150.0, 0.0, 0.0, 0.0, 150.0, -180.0, 0.0, -180.0, 150.0, -1000.0, -400.0);
	HOLFormationRead::ReserveRearSlots(Slots, LeftLocal, OneKind, LastIsStraggler, SlotOfUnit, Queued);
	T.AssertTrue(SlotOfUnit.Num() == 6 && SlotOfUnit[5] == 3 && !Queued[5], "rear slots: from the left, the last row left slot");
	TArray<bool> UsedSlot;
	UsedSlot.SetNum(6);
	int UsedCount = 0;
	for (int I = 0; I < SlotOfUnit.Num(); I++)
	{
		if (SlotOfUnit[I] >= 0 && SlotOfUnit[I] < 6 && !UsedSlot[SlotOfUnit[I]])
		{
			UsedSlot[SlotOfUnit[I]] = true;
			UsedCount++;
		}
	}
	T.AssertEquals(6, UsedCount, "rear slots: one slot each");
	T.AssertTrue(SlotOfUnit[0] == 0 && SlotOfUnit[1] == 1 && SlotOfUnit[2] == 2, "rear slots: the body keeps its rows (no one of the front row sent back)");
	// Mixed: kind 0 in front (three), kind 1 behind (three); the straggler is of kind 0.
	TArray<bool> FirstIsStraggler;
	FirstIsStraggler.Add(true); FirstIsStraggler.Add(false); FirstIsStraggler.Add(false); FirstIsStraggler.Add(false); FirstIsStraggler.Add(false); FirstIsStraggler.Add(false);
	TArray<FVector2D> MixedLocal = FMLocalSlots(-1000.0, 0.0, 0.0, 0.0, 0.0, 150.0, -180.0, -150.0, -180.0, 0.0, -180.0, 150.0);
	T.AssertEquals(1, HOLFormationRead::ReserveRearSlots(Slots, MixedLocal, TwoKinds, FirstIsStraggler, SlotOfUnit, Queued), "rear slots: mixed, its kind in front, queued");
	T.AssertTrue(SlotOfUnit.Num() == 6 && Queued[0] && SlotOfUnit[0] <= 2 && SlotOfUnit[3] >= 3 && SlotOfUnit[4] >= 3 && SlotOfUnit[5] >= 3, "rear slots: mixed, a slot of its kind kept, the other kind on its own");
}

// HeroesOfLiberty.Formation.Settle
void Test_FMSettle(FUnitTest& T)
{
	// MV.5 (the user option A): 9 mixed soldiers sent 1 m from a stone vein edge (radius 300 cm, a body of 60 cm).
	FVector2D Centre(0.0, 400.0);
	double Clearance = 300.0 + 60.0;
	TArray<FVector2D> Positions;
	TArray<int> KindOfUnit;
	for (int Unit = 0; Unit < 9; Unit++)
	{
		Positions.Add(FVector2D(-2000.0 - 200.0 * (Unit / 3), 200.0 * (Unit % 3)));
		KindOfUnit.Add(Unit % 3);
	}
	TArray<FVector2D> Raw;
	TArray<int> PlaceOfUnit;
	int Ignored = 0;
	HOLFormationRead::PlaceGroupClamped(Positions, KindOfUnit, false, FVector2D(0.0, 0.0), FVector2D(1.0, 0.0), 150.0, 180.0, 1.0e12, Ignored, Raw, PlaceOfUnit);
	TArray<FVector2D> Places;
	int Projected = 0;
	HOLFormationRead::PlaceGroupAvoidingDisc(Positions, KindOfUnit, false, FVector2D(0.0, 0.0), FVector2D(1.0, 0.0), 150.0, 180.0, false, Centre, Clearance, Projected, Places, PlaceOfUnit);
	T.AssertEquals(Places.Num(), Projected, "one projection per place");
	int Blocked = 0;
	bool bOutside = Places.Num() == Raw.Num() && Raw.Num() == 9;
	bool bUnblockedKept = bOutside;
	bool bNear = bOutside;
	for (int Place = 0; bOutside && Place < Places.Num(); Place++)
	{
		bOutside = bOutside && FMDist(Places[Place], Centre) >= Clearance;
		if (FMDist(Raw[Place], Centre) >= Clearance)
		{
			bUnblockedKept = bUnblockedKept && FMNear(Places[Place], Raw[Place], 1.0e-6);
			continue;
		}
		Blocked++;
		bNear = bNear && FMDist(Places[Place], Raw[Place]) <= Clearance - FMDist(Raw[Place], Centre) + 2.0 * 150.0;
	}
	T.AssertTrue(Blocked >= 2, "the vein takes some places");
	T.AssertTrue(bOutside, "no place left in the vein");
	T.AssertTrue(bUnblockedKept, "the places the vein does not take do not move");
	T.AssertTrue(bNear, "each place moved goes to a near free point");
	double MinGap = 1.0e30;
	for (int A = 0; A < Places.Num(); A++)
	{
		for (int B = A + 1; B < Places.Num(); B++)
		{
			double Gap = FMDist(Places[A], Places[B]);
			if (Gap < MinGap)
			{
				MinGap = Gap;
			}
		}
	}
	T.AssertTrue(MinGap >= HOLFormationRead::GetConstant(n"SettleMinGapInSpacings") * 150.0 - 1.0e-6, "no pile: places 135 cm apart at least");
	bool bEachOwn = PlaceOfUnit.Num() == 9 && !PlaceOfUnit.Contains(-1);
	TArray<bool> Given;
	Given.SetNum(9);
	int Distinct = 0;
	for (int Unit = 0; bEachOwn && Unit < 9; Unit++)
	{
		if (PlaceOfUnit[Unit] >= 0 && PlaceOfUnit[Unit] < 9 && !Given[PlaceOfUnit[Unit]])
		{
			Given[PlaceOfUnit[Unit]] = true;
			Distinct++;
		}
	}
	T.AssertTrue(bEachOwn && Distinct == 9, "each unit a place of its own");

	// No free point at all: nothing settled, the places left for the caller fallback.
	TArray<FVector2D> Walled = Raw;
	TArray<bool> AllBlocked;
	for (int I = 0; I < Walled.Num(); I++)
	{
		AllBlocked.Add(true);
	}
	TArray<bool> Settled;
	int NumSettled = HOLFormationRead::SettleBlockedPlacesWalled(Walled, AllBlocked, FVector2D(0.0, 0.0), 150.0, Settled);
	T.AssertEquals(0, NumSettled, "walled in: nothing settled");
	bool bStay = Walled.Num() == Raw.Num();
	for (int I = 0; bStay && I < Walled.Num(); I++)
	{
		bStay = FMNear(Walled[I], Raw[I], 1.0e-9);
	}
	for (int I = 0; I < Settled.Num(); I++)
	{
		bStay = bStay && !Settled[I];
	}
	T.AssertTrue(bStay, "walled in: the places stay");
	int Fallbacks = 0;
	HOLFormationRead::PlaceGroupAvoidingDisc(Positions, KindOfUnit, false, FVector2D(0.0, 0.0), FVector2D(1.0, 0.0), 150.0, 180.0, true, Centre, Clearance, Fallbacks, Places, PlaceOfUnit);
	bool bFallback = Fallbacks == 9;
	for (int I = 0; I < Places.Num(); I++)
	{
		bFallback = bFallback && FMNear(Places[I], FVector2D(0.0, 0.0), 1.0e-9);
	}
	T.AssertTrue(bFallback, "walled in: the fallback gives every place");
}
