// MIG-C-Gather (HALDIR-1117s): script port of the pure tests of HOLGatherTests.cpp (Load, Deposit, Choice, Massif, NearbyMassif, Spots and the first half of
// Sanctuary), run through UHOLGatherReadLibrary (Simulation/HOLGatherReadLibrary.*, namespace HOLGatherRead, envelopes only; the rules stay in C++:
// HOLGatherRules.h). Same vectors as the C++ tests. NEEDS THE C++ LOT: compile only after the build.
// Not ported (stay in HOLGatherTests.cpp): Tables (real source and unit tables; only the main centre block is in HOLBuilding_Test.as), the world and Mass
// parts (VeinSizes, LinkedSource, FarmField, Income, second half of Sanctuary: D838).
// The C++ test file is not touched: it is cut down only after the red then green of this one (not done yet).
// A resource is its number: 0 Food, 1 Wood, 2 Stone, 3 Gold. Free functions on local arrays (no script class: "Null pointer access" as argument of AssertEquals).

const double GatherStep = 0.05;

// Gathers N steps of 0.05 s; returns the units taken, adds the load lost to GatherLost, updates the load and what the source holds.
int GatherNL(FHOLGatherLoadScript& Load, int N, double Rate, int Capacity, int& Remaining, int Resource, int& LostTotal)
{
	int Taken = 0;
	int Lost = 0;
	int After = 0;
	FHOLGatherLoadScript Result = HOLGatherRead::GatherSteps(Load, Resource, Rate, GatherStep, N, Capacity, Remaining, Taken, Lost, After);
	Load = Result;
	Remaining = After;
	LostTotal += Lost;
	return Taken;
}

int GatherN(FHOLGatherLoadScript& Load, int N, double Rate, int Capacity, int& Remaining, int Resource = 1)
{
	int Ignored = 0;
	return GatherNL(Load, N, Rate, Capacity, Remaining, Resource, Ignored);
}

bool GatherNear(double A, double B, double Tolerance)
{
	return Math::Abs(A - B) <= Tolerance;
}

bool GatherAt(FVector2D P, double X, double Y, double Tolerance)
{
	return Math::Abs(P.X - X) <= Tolerance && Math::Abs(P.Y - Y) <= Tolerance;
}

double GatherDist(FVector2D A, FVector2D B)
{
	return Math::Sqrt((A.X - B.X) * (A.X - B.X) + (A.Y - B.Y) * (A.Y - B.Y));
}

bool GatherSame(TArray<int>& A, TArray<int>& B)
{
	if (A.Num() != B.Num())
		return false;
	for (int I = 0; I < A.Num(); I++)
	{
		if (A[I] != B[I])
			return false;
	}
	return true;
}

void GatherAddSource(TArray<FVector2D>& Positions, TArray<int>& Resources, double X, double Y, int Resource)
{
	Positions.Add(FVector2D(X, Y));
	Resources.Add(Resource);
}

// The sources tried, in order, when every one is refused from Start (no deposit).
int GatherSearchNone(TArray<FVector2D>& P, TArray<int>& R, double Nearby, int Start, TArray<int>& Accepted, TArray<int>& Tried)
{
	return HOLGatherRead::SearchMassif(P, R, HOLGatherRead::GetNoRoomSourceDistance(), Nearby, Start, Accepted, false, FVector2D(0.0, 0.0), FVector2D(0.0, 0.0), 0.0, Tried);
}

int GatherSearchDeposit(TArray<FVector2D>& P, TArray<int>& R, double Nearby, int Start, TArray<int>& Accepted, TArray<int>& Tried, double Cx, double Cy)
{
	return HOLGatherRead::SearchMassif(P, R, HOLGatherRead::GetNoRoomSourceDistance(), Nearby, Start, Accepted, true, FVector2D(Cx, Cy), FVector2D(750.0, 750.0), 0.0, Tried);
}

// A spot taken or not, on the claims held (owners and spots); the claims left and the staleness questions out.
bool GatherTaken(TArray<int>& Owners, TArray<FVector2D>& Spots, FVector2D Spot, double Spacing, int Owner, TArray<int>& Stale, int& Left, int& Asked)
{
	return HOLGatherRead::IsSpotTaken(Owners, Spots, Spot, Spacing, Owner, Stale, Left, Asked);
}

bool GatherTakenSimple(TArray<int>& Owners, TArray<FVector2D>& Spots, FVector2D Spot, double Spacing, int Owner)
{
	TArray<int> NoStale;
	int Left = 0;
	int Asked = 0;
	return HOLGatherRead::IsSpotTaken(Owners, Spots, Spot, Spacing, Owner, NoStale, Left, Asked);
}

// 0.7 a second: nothing whole after 1 s, one unit after 2 s; a load of 10 takes 286 steps; a full load takes nothing; a rate of 0 takes nothing; a used-up source.
void Test_Gather_Load(FUnitTest& T)
{
	FHOLGatherLoadScript Load;
	Load = HOLGatherRead::StartGathering(Load, 1);
	int Tree = 100;
	int Taken = GatherN(Load, 20, 0.7, 10, Tree);
	T.AssertEquals(0, Taken, "nothing whole after 1 s");
	T.AssertTrue(Math::Abs(Load.Progress - 0.7) <= 0.0001, "progress kept between steps");
	Taken = GatherN(Load, 20, 0.7, 10, Tree);
	T.AssertEquals(1, Taken, "one unit after 2 s");
	T.AssertEquals(99, Tree, "taken from the source");

	GatherN(Load, 245, 0.7, 10, Tree);
	T.AssertEquals(9, Load.Amount, "9 after 285 steps");
	T.AssertFalse(HOLGatherRead::IsFull(Load, 10), "not full at 9");
	GatherN(Load, 1, 0.7, 10, Tree);
	T.AssertEquals(10, Load.Amount, "10 after 286 steps");
	T.AssertTrue(HOLGatherRead::IsFull(Load, 10), "full at 10");
	T.AssertEquals(90, Tree, "10 taken from the tree");
	T.AssertTrue(Load.Progress == 0.0, "no progress kept once full");

	Taken = GatherN(Load, 100, 0.7, 10, Tree);
	T.AssertEquals(0, Taken, "a full load takes nothing");
	T.AssertEquals(90, Tree, "the tree keeps the rest");

	FHOLGatherLoadScript Soldier;
	Taken = GatherN(Soldier, 400, 0.0, 10, Tree);
	T.AssertEquals(0, Taken, "rate 0 takes nothing");

	FHOLGatherLoadScript Last;
	Last = HOLGatherRead::StartGathering(Last, 1);
	int Stump = 3;
	Taken = GatherN(Last, 400, 0.7, 10, Stump);
	T.AssertEquals(3, Taken, "the last 3 units");
	T.AssertEquals(0, Stump, "source used up");
	T.AssertEquals(3, Last.Amount, "load of 3");
	T.AssertFalse(HOLGatherRead::IsFull(Last, 10), "not full");
	T.AssertTrue(Last.Progress == 0.0, "no progress kept on an empty source");
	Taken = GatherN(Last, 100, 0.7, 10, Stump);
	T.AssertEquals(0, Taken, "an empty source gives nothing");
	T.AssertEquals(0, Stump, "never below 0");
}

// The stock rises at the deposit only; another resource keeps the load until its first whole unit (D306), then the old load is lost.
void Test_Gather_Deposit(FUnitTest& T)
{
	TArray<int> Stock;
	Stock.Add(200);
	Stock.Add(200);
	Stock.Add(200);
	Stock.Add(200);
	FHOLGatherLoadScript Load;
	Load = HOLGatherRead::StartGathering(Load, 1);
	int Tree = 100;
	GatherN(Load, 300, 0.7, 10, Tree);
	T.AssertEquals(200, Stock[1], "the stock does not rise while gathering");

	int Deposited = 0;
	TArray<int> After;
	Load = HOLGatherRead::Deposit(Load, Stock, Deposited, After);
	Stock = After;
	T.AssertEquals(10, Deposited, "10 deposited");
	T.AssertTrue(Stock[0] == 200 && Stock[1] == 210 && Stock[2] == 200 && Stock[3] == 200, "the wood rises at the deposit, alone");
	T.AssertEquals(0, Load.Amount, "load emptied");
	Load = HOLGatherRead::Deposit(Load, Stock, Deposited, After);
	Stock = After;
	T.AssertEquals(0, Deposited, "an empty load deposits nothing");
	T.AssertEquals(210, Stock[1], "wood unchanged");

	GatherN(Load, 210, 0.7, 10, Tree);
	T.AssertEquals(7, Load.Amount, "7 wood carried");
	Load = HOLGatherRead::StartGathering(Load, 1);
	T.AssertEquals(7, Load.Amount, "same resource: load kept");

	Load = HOLGatherRead::StartGathering(Load, 0);
	T.AssertEquals(7, Load.Amount, "order on berries: the 7 wood kept");
	T.AssertEquals(1, Load.Resource, "still carries wood");
	int Bush = 200;
		int Taken = GatherNL(Load, 20, 0.7, 10, Bush, 0, GatherLost);
	T.AssertEquals(0, Taken, "nothing whole after 1 s of berries");
	T.AssertEquals(7, Load.Amount, "the 7 wood still carried");
	T.AssertEquals(0, GatherLost, "nothing lost yet");
	Taken = GatherNL(Load, 20, 0.7, 10, Bush, 0, GatherLost);
	T.AssertEquals(1, Taken, "first food after 2 s");
	T.AssertEquals(7, GatherLost, "the 7 wood lost then");
	T.AssertEquals(1, Load.Amount, "now carries 1 food");
	T.AssertEquals(0, Load.Resource, "now carries food");
	GatherNL(Load, 60, 0.7, 10, Bush, 0, GatherLost);
	Load = HOLGatherRead::Deposit(Load, Stock, Deposited, After);
	Stock = After;
	T.AssertTrue(Stock[0] == 203 && Stock[1] == 210 && Stock[2] == 200 && Stock[3] == 200, "food deposited, the lost wood never");

	// A full load of another resource is not full of this one: it gathers on, the progress toward wood not counted.
	FHOLGatherLoadScript Full;
	Full.Resource = 1;
	Full.Amount = 10;
	Full.Progress = 0.5;
	Full.ProgressResource = 1;
	T.AssertTrue(HOLGatherRead::IsFullOf(Full, 1, 10), "full of wood");
	T.AssertFalse(HOLGatherRead::IsFullOf(Full, 0, 10), "not full of food");
		Taken = GatherNL(Full, 20, 0.7, 10, Bush, 0, GatherLost);
	T.AssertEquals(0, Taken, "full of wood: nothing whole after 1 s of berries");
	Taken = GatherNL(Full, 20, 0.7, 10, Bush, 0, GatherLost);
	T.AssertEquals(1, Taken, "then its first food");
	T.AssertEquals(10, GatherLost, "the 10 wood lost");

	FHOLGatherLoadScript Empty;
	Empty = HOLGatherRead::StartGathering(Empty, 3);
	T.AssertEquals(3, Empty.Resource, "empty: gold at the order");
}

// Nearest deposit measured to its footprint, distances, a turned footprint, the unloading spot, contact.
void Test_Gather_Choice(FUnitTest& T)
{
	TArray<FVector2D> Centres;
	TArray<FVector2D> Halves;
	TArray<float32> Yaws;
	Centres.Add(FVector2D(2000.0, 0.0));
	Halves.Add(FVector2D(100.0, 100.0));
	Yaws.Add(0.0);
	Centres.Add(FVector2D(-2400.0, 0.0));
	Halves.Add(FVector2D(750.0, 750.0));
	Yaws.Add(0.0);
	T.AssertEquals(1, HOLGatherRead::FindNearestDeposit(FVector2D(0.0, 0.0), Centres, Halves, Yaws), "the nearer footprint wins");
	T.AssertEquals(0, HOLGatherRead::FindNearestDeposit(FVector2D(1500.0, 0.0), Centres, Halves, Yaws), "near the small one");
	TArray<FVector2D> NoCentres;
	TArray<FVector2D> NoHalves;
	TArray<float32> NoYaws;
	T.AssertEquals(-1, HOLGatherRead::FindNearestDeposit(FVector2D(0.0, 0.0), NoCentres, NoHalves, NoYaws), "no deposit");

	FVector2D C1 = FVector2D(-2400.0, 0.0);
	FVector2D H1 = FVector2D(750.0, 750.0);
	T.AssertTrue(GatherNear(HOLGatherRead::GetDepositDistance(FVector2D(0.0, 0.0), C1, H1, 0.0), 1650.0, 0.01), "distance to a footprint");
	T.AssertTrue(GatherNear(HOLGatherRead::GetDepositDistance(FVector2D(-1350.0, 1150.0), C1, H1, 0.0), 500.0, 0.01), "distance to a corner");
	T.AssertTrue(GatherNear(HOLGatherRead::GetDepositDistance(FVector2D(-2400.0, 100.0), C1, H1, 0.0), 0.0, 0.01), "0 on the footprint");

	// A turned footprint (45 degrees): its corner points along X, 141 cm from its centre.
	T.AssertTrue(GatherNear(HOLGatherRead::GetDepositDistance(FVector2D(241.42, 0.0), FVector2D(0.0, 0.0), FVector2D(100.0, 100.0), 45.0), 100.0, 0.5), "turned: 100 cm past the corner");

	// Unloading: against the side it faces, Gap cm out, slid along it for a second try; close enough within 150 cm.
	FVector2D Spot = HOLGatherRead::GetDepositSpot(FVector2D(-2400.0, 3000.0), C1, H1, 0.0, 70.0, 0.0);
	T.AssertTrue(GatherAt(Spot, -2400.0, 820.0, 0.01), "spot against the facing side");
	FVector2D Slid = HOLGatherRead::GetDepositSpot(FVector2D(-2400.0, 3000.0), C1, H1, 0.0, 70.0, 150.0);
	T.AssertTrue(GatherAt(Slid, -2250.0, 820.0, 0.01), "spot slid along the side");
	FVector2D Stays = HOLGatherRead::GetDepositSpot(FVector2D(-1000.0, 3000.0), C1, H1, 0.0, 70.0, 150.0);
	T.AssertTrue(GatherAt(Stays, -1650.0, 820.0, 0.01), "spot stays on the side");
	T.AssertTrue(HOLGatherRead::IsAtDeposit(Spot, 60.0, C1, H1, 0.0), "unloads at its spot");
	T.AssertFalse(HOLGatherRead::IsAtDeposit(FVector2D(-2400.0, 1000.0), 60.0, C1, H1, 0.0), "too far to unload");
	T.AssertTrue(GatherNear(HOLGatherRead::GetEdgeDistance(FVector2D(0.0, 0.0), FVector2D(-2400.0, 0.0), 750.0), 1650.0, 0.001), "edge distance of a source");

	// Contact: body against body, give or take 30 cm.
	FVector2D Origin = FVector2D(0.0, 0.0);
	T.AssertTrue(HOLGatherRead::IsInContact(FVector2D(120.0, 0.0), 60.0, Origin, 60.0), "in contact at the edge");
	T.AssertTrue(HOLGatherRead::IsInContact(FVector2D(150.0, 0.0), 60.0, Origin, 60.0), "in contact 30 cm off");
	T.AssertFalse(HOLGatherRead::IsInContact(FVector2D(170.0, 0.0), 60.0, Origin, 60.0), "not in contact 50 cm off");
}

// A massif: the sources of one resource linked step by step, 8 m apart at most; a gatherer with no room looks through it from its source outward.
void Test_Gather_Massif(FUnitTest& T)
{
	TArray<FVector2D> P;
	TArray<int> R;
	for (int Tree = 0; Tree < 6; Tree++)
		GatherAddSource(P, R, 600.0 * Tree, 0.0, 1);   // a chain of trees 6 m apart; tree 3 is used up in the C++ vector, which the search ignores
	GatherAddSource(P, R, 5000.0, 0.0, 1);              // 6: 20 m beyond the chain's end
	GatherAddSource(P, R, 1200.0, 300.0, 0);            // 7: berries beside tree 2
	GatherAddSource(P, R, 0.0, 10000.0, 2);             // 8: a vein
	GatherAddSource(P, R, 6000.0, 10000.0, 2);          // 9: another vein, 60 m away
	double Link = HOLGatherRead::GetNoRoomSourceDistance();
	T.AssertEquals(1, HOLGatherRead::GetLinkCount(P, R, Link, 0.0, 0), "the chain's end: one link");
	T.AssertEquals(0, HOLGatherRead::GetLinkCount(P, R, Link, 0.0, 7), "berries beside a tree: no link to it");
	T.AssertEquals(0, HOLGatherRead::GetLinkCount(P, R, Link, 0.0, 8), "a vein alone");

	// Every source refused: the order they were tried in, from tree 2 outward.
	TArray<int> None;
	TArray<int> Tried;
	int Found = GatherSearchNone(P, R, 0.0, 2, None, Tried);
	T.AssertEquals(-1, Found, "the whole chain full: none");
	TArray<int> Expected;
	Expected.Add(1);
	Expected.Add(3);
	Expected.Add(0);
	Expected.Add(4);
	Expected.Add(5);
	T.AssertTrue(GatherSame(Tried, Expected), "nearest first, a tie to the first index, past the used-up tree 3, tree 2 itself not tried");
	T.AssertFalse(Tried.Contains(6), "the tree 20 m away never reached");
	T.AssertFalse(Tried.Contains(7), "nor the berries beside it");

	// The first one with room, nearest first; the used-up tree refused leads on to the chain beyond it.
	TArray<int> OnlyFive;
	OnlyFive.Add(5);
	Found = GatherSearchNone(P, R, 0.0, 0, OnlyFive, Tried);
	T.AssertEquals(5, Found, "tree 5, at the far end");
	TArray<int> Expected2;
	Expected2.Add(1);
	Expected2.Add(2);
	Expected2.Add(3);
	Expected2.Add(4);
	Expected2.Add(5);
	T.AssertTrue(GatherSame(Tried, Expected2), "each tree tried once, in order");
	TArray<int> AllButThree;
	AllButThree.Add(0);
	AllButThree.Add(1);
	AllButThree.Add(2);
	AllButThree.Add(4);
	AllButThree.Add(5);
	AllButThree.Add(6);
	AllButThree.Add(7);
	AllButThree.Add(8);
	AllButThree.Add(9);
	Found = GatherSearchNone(P, R, 0.0, 0, AllButThree, Tried);
	T.AssertEquals(1, Found, "the first one with room stops the search");
	T.AssertEquals(1, Tried.Num(), "nothing tried beyond it");
	TArray<int> Everything;
	Everything.Add(0);
	Everything.Add(1);
	Everything.Add(2);
	Everything.Add(3);
	Everything.Add(4);
	Everything.Add(5);
	Everything.Add(6);
	Everything.Add(7);
	Everything.Add(8);
	Everything.Add(9);
	Found = GatherSearchNone(P, R, 0.0, 6, Everything, Tried);
	T.AssertEquals(-1, Found, "from the tree 20 m away: nothing");
	Found = GatherSearchNone(P, R, 0.0, 8, Everything, Tried);
	T.AssertEquals(-1, Found, "a vein never leads to another");

	// A wood (7 x 7 trees 3 m apart) clicked at a corner tree: it fills from there outward, as far as its farthest tree.
	TArray<FVector2D> WP;
	TArray<int> WR;
	for (int Y = 0; Y < 7; Y++)
	{
		for (int X = 0; X < 7; X++)
			GatherAddSource(WP, WR, 300.0 * X, 300.0 * Y, 1);
	}
	TArray<int> Filled;
	GatherSearchNone(WP, WR, 0.0, 0, None, Filled);
	T.AssertEquals(WP.Num() - 1, Filled.Num(), "every other tree of the wood tried");
	bool bOutward = true;
	for (int I = 1; I < Filled.Num(); I++)
		bOutward = bOutward && GatherDist(WP[Filled[I]], WP[0]) >= GatherDist(WP[Filled[I - 1]], WP[0]) - 0.01;
	T.AssertTrue(bOutward, "from the tree clicked outward");
	T.AssertEquals(WP.Num() - 1, Filled[Filled.Num() - 1], "the opposite corner last");

	// Toward a deposit (after AoE IV): the edge facing it first. The wood of Plain: 8 trees along X by 5 rows 2.2 m apart, its near row at Y = 0;
	// a main centre (15 m square) whose side is 10.5 m in front.
	TArray<FVector2D> PP;
	TArray<int> PR;
	for (int Row = 0; Row < 5; Row++)
	{
		for (int Column = 0; Column < 8; Column++)
			GatherAddSource(PP, PR, (Column - 3.5) * 220.0, -220.0 * Row, 1);
	}
	for (int K = 0; K < 2; K++)
	{
		int Clicked = K == 0 ? 3 : 0;
		TArray<int> PlainTried;
		GatherSearchDeposit(PP, PR, 0.0, Clicked, None, PlainTried, 0.0, 1800.0);
		T.AssertEquals(PP.Num() - 1, PlainTried.Num(), "every other tree tried");
		int LastNear = -1;
		int FirstBack = 1000000;
		bool bAlongFromClicked = true;
		double LastAlong = 0.0;
		for (int Order = 0; Order < PlainTried.Num(); Order++)
		{
			int Row = PlainTried[Order] / 8;
			if (Row == 0)
			{
				LastNear = Order;
				double Along = Math::Abs(PP[PlainTried[Order]].X - PP[Clicked].X);
				bAlongFromClicked = bAlongFromClicked && Along >= LastAlong - 0.01;
				LastAlong = Along;
			}
			else if (Row >= 3)
				FirstBack = Math::Min(FirstBack, Order);
		}
		T.AssertTrue(bAlongFromClicked, "the near edge along from the clicked tree");
		T.AssertTrue(LastNear < FirstBack, "the whole near edge before the back of the wood");
	}
	// With no deposit, the nearest the tree clicked first, as before: the tree behind it before the end of its row.
	TArray<int> Plainly;
	GatherSearchNone(PP, PR, 0.0, 0, None, Plainly);
	T.AssertTrue(Plainly.FindIndex(8) < Plainly.FindIndex(3), "no deposit: the tree 2.2 m behind before the tree 6.6 m along");
}

// A whole massif full: a nearby grove of the same resource, ~20 m at most from one of its sources, in the same order as the massif; then rows of idle points.
void Test_Gather_NearbyMassif(FUnitTest& T)
{
	TArray<FVector2D> P;
	TArray<int> R;
	GatherAddSource(P, R, 0.0, 0.0, 1);        // 0: wood A, clicked
	GatherAddSource(P, R, 500.0, 0.0, 1);      // 1: wood A
	GatherAddSource(P, R, 1000.0, 0.0, 1);     // 2: wood A (used up in the C++ vector)
	GatherAddSource(P, R, 2500.0, 0.0, 1);     // 3: wood B, 15 m beyond the stump 2
	GatherAddSource(P, R, 3000.0, 0.0, 1);     // 4: wood B
	GatherAddSource(P, R, -2600.0, 0.0, 1);    // 5: wood C, 26 m from tree 0: out of range
	GatherAddSource(P, R, 500.0, 1200.0, 0);   // 6: berries 12 m from tree 1: another resource
	GatherAddSource(P, R, 500.0, -1900.0, 1);  // 7: wood D, 19 m from tree 1, nearer the deposit
	GatherAddSource(P, R, 0.0, 10000.0, 2);    // 8: a vein
	GatherAddSource(P, R, 6000.0, 10000.0, 2); // 9: another vein, 60 m away
	double Link = HOLGatherRead::GetNoRoomSourceDistance();
	double Near = HOLGatherRead::GetNearbyMassifDistance();
	TArray<int> Massifs;
	HOLGatherRead::GetMassifs(P, R, Link, Near, Massifs);
	T.AssertEquals(Massifs[0], Massifs[2], "one massif for wood A");
	T.AssertTrue(Massifs[3] != Massifs[0], "wood B a massif of its own");
	T.AssertEquals(2, HOLGatherRead::GetNearbyMassifCount(P, R, Link, Near, Massifs[0]), "wood A: two nearby groves (B, D)");
	T.AssertEquals(0, HOLGatherRead::GetNearbyMassifCount(P, R, Link, Near, Massifs[8]), "a vein: no nearby grove");

	// Wood A full: its own trees first, then those of B and D, never C (out of range) nor the berries (another resource).
	TArray<int> None;
	TArray<int> Tried;
	int Found = GatherSearchNone(P, R, Near, 0, None, Tried);
	T.AssertEquals(-1, Found, "everything full: none");
	TArray<int> Expected;
	Expected.Add(1);
	Expected.Add(2);
	Expected.Add(7);
	Expected.Add(3);
	Expected.Add(4);
	T.AssertTrue(GatherSame(Tried, Expected), "its massif, then the groves nearest the tree clicked");
	T.AssertFalse(Tried.Contains(5), "wood C, 26 m away: never");
	T.AssertFalse(Tried.Contains(6), "berries 12 m away: never");
	TArray<int> NotA;
	for (int I = 0; I < Massifs.Num(); I++)
	{
		if (Massifs[I] != Massifs[0])
			NotA.Add(I);
	}
	Found = GatherSearchNone(P, R, Near, 0, NotA, Tried);
	T.AssertEquals(7, Found, "wood A full: the first tree of a nearby grove");

	// Toward a deposit south of wood A: D, on the deposit's side, before B. A deposit east of wood B: B first.
	GatherSearchDeposit(P, R, Near, 0, None, Tried, 500.0, -4000.0);
	T.AssertTrue(Tried.FindIndex(7) < Tried.FindIndex(3), "toward the deposit: grove D first");
	Found = GatherSearchDeposit(P, R, Near, 0, NotA, Tried, 5000.0, 0.0);
	T.AssertEquals(4, Found, "toward a deposit beyond grove B: B first");

	// Within the range, not beyond; a vein never leads to another (60 m), nor without nearby groves (range 0).
	TArray<int> Everything;
	Everything.Add(0);
	Everything.Add(1);
	Everything.Add(2);
	Everything.Add(3);
	Everything.Add(4);
	Everything.Add(5);
	Everything.Add(6);
	Everything.Add(7);
	Everything.Add(8);
	Everything.Add(9);
	Found = GatherSearchNone(P, R, Near, 8, Everything, Tried);
	T.AssertEquals(-1, Found, "a vein never leads to another");
	Found = GatherSearchNone(P, R, 1400.0, 0, NotA, Tried);
	T.AssertEquals(-1, Found, "14 m: grove B (15 m) out of range");
	Found = GatherSearchNone(P, R, 0.0, 0, NotA, Tried);
	T.AssertEquals(-1, Found, "no range: no nearby grove");
	GatherSearchNone(P, R, 2700.0, 0, None, Tried);
	T.AssertTrue(Tried.Contains(5), "27 m: wood C in range");
	T.AssertFalse(Tried.Contains(6), "27 m: still no berries");
	// From grove B: wood A nearby, but not D (40 m from B), nor through A.
	GatherSearchNone(P, R, Near, 3, None, Tried);
	TArray<int> FromB;
	FromB.Add(4);
	FromB.Add(2);
	FromB.Add(1);
	FromB.Add(0);
	T.AssertTrue(GatherSame(Tried, FromB), "from grove B: its tree, then wood A only");

	// No grove with room: rows without limit in front of the tree (300 idle gatherers, far more than 6 rows hold), each on a point of its own, clear of the
	// spots of the tree and of the one beside it.
	FVector2D TreePos = FVector2D(5000.0, 5000.0);
	double SpotOffset = 75.0;
	double StandSpacing = 140.0;
	double SpotDistance = 60.0 + SpotOffset;
	TArray<FVector2D> SitePositions;
	TArray<float32> SiteRadii;
	SitePositions.Add(TreePos);
	SiteRadii.Add(60.0);
	SitePositions.Add(FVector2D(5220.0, 5000.0));
	SiteRadii.Add(60.0);
	TArray<int> Owners;
	TArray<FVector2D> Stood;
	TArray<FVector2D> Points;
	int FarthestRow = 0;
	for (int Idle = 1; Idle <= 300; Idle++)
	{
		bool bFound = false;
		for (int FirstRow = 0; FirstRow < 64 && !bFound; FirstRow += 6)
		{
			HOLGatherRead::GetStandPoints(TreePos, SpotDistance, StandSpacing, FVector2D(0.0, -1.0), 6, FirstRow, Points);
			for (int K = 0; K < Points.Num() && !bFound; K++)
			{
				FVector2D Point = Points[K];
				if (HOLGatherRead::IsClearOfSpots(Point, SitePositions, SiteRadii, SpotOffset, StandSpacing) && !GatherTakenSimple(Owners, Stood, Point, StandSpacing, Idle))
				{
					Owners.Add(Idle);
					Stood.Add(Point);
					FarthestRow = Math::Max(FarthestRow, FirstRow);
					bFound = true;
				}
			}
		}
	}
	T.AssertEquals(300, Stood.Num(), "300 idle gatherers, 300 points");
	T.AssertTrue(FarthestRow >= 6, "beyond the first 6 rows");
	double Closest = 1.0e30;
	for (int A = 0; A < Stood.Num(); A++)
	{
		for (int B = A + 1; B < Stood.Num(); B++)
			Closest = Math::Min(Closest, GatherDist(Stood[A], Stood[B]));
	}
	T.AssertTrue(Closest >= StandSpacing - 0.01, "no two idle gatherers closer than 140 cm");
	// The rows go on as asked: row 10 lies 11 spacings beyond the spots.
	HOLGatherRead::GetStandPoints(TreePos, SpotDistance, StandSpacing, FVector2D(0.0, -1.0), 1, 10, Points);
	T.AssertTrue(Points.Num() > 0, "row 10");
	if (Points.Num() > 0)
		T.AssertTrue(GatherNear(GatherDist(Points[0], TreePos), SpotDistance + 11.0 * (StandSpacing + 1.0), 0.01), "row 10, 11 spacings out");
}

// The spot claims (a fresh grid per look: the claims are passed in), the spots tried around a source, the points where the idle ones stand.
void Test_Gather_Spots(FUnitTest& T)
{
	double Spacing = 70.0;
	TArray<int> Owners;
	TArray<FVector2D> Spots;
	TArray<int> NoStale;
	TArray<int> Stale;
	int Left = 0;
	int Asked = 0;
	T.AssertFalse(GatherTakenSimple(Owners, Spots, FVector2D(135.0, 0.0), Spacing, 1), "nothing claimed");

	// Tree 0 at the origin: gatherer 1 on its spot facing +X.
	Owners.Add(1);
	Spots.Add(FVector2D(135.0, 0.0));
	T.AssertTrue(GatherTakenSimple(Owners, Spots, FVector2D(135.0, 50.0), Spacing, 2), "same source, 50 cm away: taken");
	T.AssertTrue(GatherTakenSimple(Owners, Spots, FVector2D(135.0, 0.0), Spacing, 2), "on the same point: taken");
	T.AssertFalse(GatherTakenSimple(Owners, Spots, FVector2D(135.0, 75.0), Spacing, 2), "same source, 75 cm away: free");
	// Tree 1, 2.2 m along X: its spot at 165 degrees is 57 cm from that gatherer; another source, taken all the same.
	double Radians = 165.0 * 3.14159265358979323846 / 180.0;
	FVector2D Neighbour = FVector2D(220.0 + 135.0 * Math::Cos(Radians), 135.0 * Math::Sin(Radians));
	T.AssertTrue(GatherTakenSimple(Owners, Spots, Neighbour, Spacing, 2), "neighbour source: taken");

	// Across the grid's cell borders, negative coordinates too.
	Owners.Add(3);
	Spots.Add(FVector2D(199.0, 900.0));
	T.AssertTrue(GatherTakenSimple(Owners, Spots, FVector2D(201.0, 900.0), Spacing, 2), "across a cell border");
	Owners.Add(4);
	Spots.Add(FVector2D(-1.0, -501.0));
	T.AssertTrue(GatherTakenSimple(Owners, Spots, FVector2D(1.0, -499.0), Spacing, 2), "across the axes");
	T.AssertTrue(GatherTakenSimple(Owners, Spots, FVector2D(-420.0, -501.0), 450.0, 2), "a spacing wider than a cell");

	// Its own old spot is no obstacle: left as it looks for a new one.
	T.AssertEquals(3, Owners.Num(), "three claims");
	bool bTaken = GatherTaken(Owners, Spots, FVector2D(135.0, 0.0), Spacing, 1, NoStale, Left, Asked);
	T.AssertFalse(bTaken, "its own spot: free");
	T.AssertEquals(2, Left, "its old claim dropped");
	Owners.RemoveAt(0);
	Spots.RemoveAt(0);

	// A stale claim (its gatherer went elsewhere) is dropped once met; a far one is not even looked at.
	Owners.Add(5);
	Spots.Add(FVector2D(2000.0, 0.0));
	Stale.Add(5);
	bTaken = GatherTaken(Owners, Spots, FVector2D(2150.0, 0.0), Spacing, 2, Stale, Left, Asked);
	T.AssertFalse(bTaken, "150 cm from a stale claim: free");
	T.AssertEquals(0, Asked, "far claims: staleness not asked");
	bTaken = GatherTaken(Owners, Spots, FVector2D(2000.0, 10.0), Spacing, 2, Stale, Left, Asked);
	T.AssertFalse(bTaken, "on a stale claim: free");
	T.AssertEquals(2, Left, "the stale claim dropped");

	// The spots tried around a source: as many as its circle holds at the spacing, the side facing the deposit first.
	FVector2D TreePos = FVector2D(5000.0, 5000.0);
	double SpotOffset = 75.0;
	double SpotDistance = 60.0 + SpotOffset;
	FVector2D Deposit = FVector2D(5000.0, 7000.0);
	TArray<FVector2D> Ring;
	HOLGatherRead::GetSourceSpots(TreePos, SpotDistance, Spacing, FVector2D(0.0, 2000.0), Ring);
	T.AssertEquals(11, Ring.Num(), "11 spots around a tree at 70 cm");
	if (Ring.Num() == 11)
	{
		T.AssertTrue(GatherAt(Ring[0], 5000.0, 5000.0 + SpotDistance, 0.01), "the first one faces the deposit");
		double ClosestSpots = 1.0e30;
		bool bNearestDepositFirst = true;
		for (int A = 0; A < Ring.Num(); A++)
		{
			bNearestDepositFirst = bNearestDepositFirst && (A == 0 || GatherDist(Ring[A], Deposit) >= GatherDist(Ring[A - 1], Deposit) - 0.01);
			for (int B = A + 1; B < Ring.Num(); B++)
				ClosestSpots = Math::Min(ClosestSpots, GatherDist(Ring[A], Ring[B]));
		}
		T.AssertTrue(ClosestSpots >= Spacing, "no two spots closer than the spacing");
		T.AssertTrue(bNearestDepositFirst, "nearest the deposit first");
	}
	HOLGatherRead::GetSourceSpots(TreePos, SpotDistance, 140.0, FVector2D(1.0, 0.0), Ring);
	T.AssertEquals(5, Ring.Num(), "5 around a tree at 140 cm (before)");
	HOLGatherRead::GetSourceSpots(TreePos, 110.0 + SpotOffset, Spacing, FVector2D(1.0, 0.0), Ring);
	T.AssertEquals(16, Ring.Num(), "16 around a bush");
	HOLGatherRead::GetSourceSpots(TreePos, 300.0 + SpotOffset, Spacing, FVector2D(1.0, 0.0), Ring);
	T.AssertEquals(33, Ring.Num(), "33 around a vein");

	// Idle gatherers in front of a full tree: rows beyond its spots, facing them, at their own spacing.
	double StandSpacing = 140.0;
	TArray<FVector2D> Points;
	HOLGatherRead::GetStandPoints(TreePos, SpotDistance, StandSpacing, FVector2D(-1.0, 0.0), 3, 0, Points);
	T.AssertTrue(Points.Num() > 6, "stand points");
	if (Points.Num() <= 6)
		return;
	T.AssertTrue(GatherAt(Points[0], 5000.0 - (SpotDistance + StandSpacing + 1.0), 5000.0, 0.01), "the first one faces the gatherer, one spacing beyond the spots");
	bool bFacing = true;
	bool bOffSpots = true;
	for (int I = 0; I < Points.Num(); I++)
	{
		bFacing = bFacing && Points[I].X <= TreePos.X + 0.01;
		bOffSpots = bOffSpots && GatherDist(Points[I], TreePos) >= SpotDistance + StandSpacing - 0.01;
	}
	T.AssertTrue(bFacing, "on the half facing it");
	T.AssertTrue(bOffSpots, "a spacing beyond the spots");
	T.AssertTrue(GatherDist(Points[0], Points[1]) >= StandSpacing - 0.01, "two points of a row a spacing apart");
	// A tree beside it: its spots are not stood on either.
	TArray<FVector2D> OneSite;
	TArray<float32> OneRadius;
	OneSite.Add(TreePos);
	OneRadius.Add(60.0);
	TArray<FVector2D> TwoSites = OneSite;
	TArray<float32> TwoRadii = OneRadius;
	TwoSites.Add(FVector2D(4700.0, 5000.0));
	TwoRadii.Add(60.0);
	T.AssertTrue(HOLGatherRead::IsClearOfSpots(Points[0], OneSite, OneRadius, SpotOffset, StandSpacing), "its own spots only: clear");
	T.AssertFalse(HOLGatherRead::IsClearOfSpots(Points[0], TwoSites, TwoRadii, SpotOffset, StandSpacing), "on the spots of the tree beside it");
	T.AssertFalse(HOLGatherRead::IsClearOfSpots(FVector2D(5000.0, 5000.0 + SpotDistance), TwoSites, TwoRadii, SpotOffset, StandSpacing), "on its own spots");

	// Ten idle gatherers, three gatherers at their spots: each takes the first point free, none closer than the spacing.
	TArray<int> FrontOwners;
	TArray<FVector2D> FrontSpots;
	for (int Gatherer = 1; Gatherer <= 3; Gatherer++)
	{
		double Angle = 3.14159265358979323846 + (Gatherer - 2.0) * 0.8;
		FrontOwners.Add(Gatherer);
		FrontSpots.Add(FVector2D(5000.0 + SpotDistance * Math::Cos(Angle), 5000.0 + SpotDistance * Math::Sin(Angle)));
	}
	TArray<FVector2D> Stood;
	for (int Idle = 10; Idle < 20; Idle++)
	{
		for (int K = 0; K < Points.Num(); K++)
		{
			FVector2D Point = Points[K];
			if (HOLGatherRead::IsClearOfSpots(Point, OneSite, OneRadius, SpotOffset, StandSpacing) && !GatherTakenSimple(FrontOwners, FrontSpots, Point, StandSpacing, Idle))
			{
				FrontOwners.Add(Idle);
				FrontSpots.Add(Point);
				Stood.Add(Point);
				break;
			}
		}
	}
	T.AssertEquals(10, Stood.Num(), "every idle gatherer has a point");
	double Closest = 1.0e30;
	for (int A = 0; A < Stood.Num(); A++)
	{
		for (int B = A + 1; B < Stood.Num(); B++)
			Closest = Math::Min(Closest, GatherDist(Stood[A], Stood[B]));
	}
	T.AssertTrue(Closest >= StandSpacing - 0.01, "idle gatherers a spacing apart");
	T.AssertFalse(GatherTakenSimple(FrontOwners, FrontSpots, FVector2D(5000.0 + SpotDistance, 5000.0), Spacing, 30), "a gatherer's spot behind the tree is still free");
}

// The farm field's bonus from the Sanctuaries (D602): the zone's edge, nothing without a Sanctuary, no stacking. (The world half of the C++ test stays in C++.)
void Test_Gather_Sanctuary(FUnitTest& T)
{
	TArray<FVector2D> Sanctuaries;
	T.AssertEquals(0, HOLGatherRead::GetSanctuaryFarmPercent(Sanctuaries, FVector2D(100.0, 0.0)), "no Sanctuary: no bonus");
	Sanctuaries.Add(FVector2D(0.0, 0.0));
	T.AssertEquals(15, HOLGatherRead::GetSanctuaryFarmPercent(Sanctuaries, FVector2D(2900.0, 0.0)), "a field 29 m away");
	T.AssertEquals(15, HOLGatherRead::GetSanctuaryFarmPercent(Sanctuaries, FVector2D(3000.0, 0.0)), "a field at 30 m");
	T.AssertEquals(0, HOLGatherRead::GetSanctuaryFarmPercent(Sanctuaries, FVector2D(3100.0, 0.0)), "a field 31 m away");
	Sanctuaries.Add(FVector2D(1000.0, 0.0));
	T.AssertEquals(15, HOLGatherRead::GetSanctuaryFarmPercent(Sanctuaries, FVector2D(500.0, 0.0)), "two Sanctuaries do not stack");
}
