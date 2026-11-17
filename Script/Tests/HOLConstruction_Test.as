// Port of HOLConstructionTests.cpp (MIG-C-Construction, ELROND-1117n, D830/D839): same vectors, one by one, against UHOLConstructionReadLibrary
// (namespace HOLConstructionRead, wrappers of HOLConstruction in Buildings/HOLConstructionRules.h). NOT COMPILED when written: the C++ library is new.
// Ported: Grid, Placement, Age, Payment, Damage, Builders, Population, States, Probes, BuildSpots, NextSite (11 of 20).
// Not ported (types to expose): Tables, BuildQueue, WorkQueue, CampSource, CutPaths, SiteKeepOut, IdleWall, Body, FreeFootprint.
// Enums as ints: state 0 Foundation 1 UnderConstruction 2 Finished; attack 0 Melee 1 Ranged 2 Siege 3 Hero; refusal 0 None 1 OutOfMap 2 Building 3 Source 4 EnemyCentre 5 NavMesh.

const float ConTestExponent = 0.63f;
const double ConTestTile = 300.0;

// A house, 2 x 2 tiles (6 m), middle at (X, Y) m.
FHOLConstructionBox ConHouse(double X, double Y)
{
	return HOLConstructionRead::MakeFootprint(FVector2D(X * 100.0, Y * 100.0), FVector2D(600.0, 600.0));
}

bool ConNear(double A, double B, double Tol = 0.01)
{
	return Math::Abs(A - B) <= Tol;
}

bool ConNearV(FVector2D A, FVector2D B, double Tol = 0.01)
{
	return ConNear(A.X, B.X, Tol) && ConNear(A.Y, B.Y, Tol);
}

FVector2D ConCentre(FHOLConstructionBox Box)
{
	return FVector2D((Box.Min.X + Box.Max.X) * 0.5, (Box.Min.Y + Box.Max.Y) * 0.5);
}

FVector2D ConSize(FHOLConstructionBox Box)
{
	return FVector2D(Box.Max.X - Box.Min.X, Box.Max.Y - Box.Min.Y);
}

bool ConOnLine(double Value, double Grid)
{
	return Math::Abs(Math::Fmod(Math::Abs(Value) + 0.01, Grid)) <= 0.02;
}

bool ConOnGrid(FHOLConstructionBox Box, double Grid)
{
	return ConOnLine(Box.Min.X, Grid) && ConOnLine(Box.Min.Y, Grid) && ConOnLine(Box.Max.X, Grid) && ConOnLine(Box.Max.Y, Grid);
}

FHOLResources ConRes(int Food, int Wood, int Stone, int Gold)
{
	FHOLResources Out;
	Out.Food = Food;
	Out.Wood = Wood;
	Out.Stone = Stone;
	Out.Gold = Gold;
	return Out;
}

bool ConResEq(FHOLResources A, FHOLResources B)
{
	return A.Food == B.Food && A.Wood == B.Wood && A.Stone == B.Stone && A.Gold == B.Gold;
}

// One placement check; the empty 400 x 400 m map and the 40 m anti-rush radius of the C++ MakeQuery are the arguments.
int ConCheck(FHOLConstructionBox Footprint, TArray<FHOLConstructionBox> Buildings, TArray<FVector> Sources, FHOLConstructionBox Map, TArray<FVector2D> Enemies, bool bOnNavMesh)
{
	return HOLConstructionRead::CheckPlacement(Footprint, Buildings, Sources, Map, Enemies, 4000.0, bOnNavMesh);
}

FHOLConstructionBox ConMap()
{
	return HOLConstructionRead::MakeBox(FVector2D(-20000.0, -20000.0), FVector2D(20000.0, 20000.0));
}

// Grid: snapping, side by side, the ghost, the diagonal start, the yaw.
void Test_Construction_Grid(FUnitTest& T)
{
	FVector2D HouseSize(600.0, 600.0);

	FHOLConstructionBox Even = HOLConstructionRead::SnapToGrid(FVector2D(1234.0, -60.0), HouseSize, ConTestTile);
	T.AssertTrue(ConNearV(Even.Min, FVector2D(900.0, -300.0)), "2 x 2 tiles: corner on the grid");
	T.AssertTrue(ConNearV(ConSize(Even), HouseSize), "2 x 2 tiles: size kept");
	T.AssertTrue(ConNearV(ConCentre(Even), FVector2D(1200.0, 0.0)), "2 x 2 tiles: middle on a grid point");

	FHOLConstructionBox Odd = HOLConstructionRead::SnapToGrid(FVector2D(1234.0, -60.0), FVector2D(900.0, 900.0), ConTestTile);
	T.AssertTrue(ConOnGrid(Odd, ConTestTile), "3 x 3 tiles: edges on the grid lines");
	T.AssertTrue(ConNearV(ConCentre(Odd), FVector2D(1350.0, -150.0)), "3 x 3 tiles: middle in the middle of a tile");

	FHOLConstructionBox Left = HOLConstructionRead::SnapToGrid(FVector2D(0.0, 0.0), HouseSize, ConTestTile);
	FHOLConstructionBox Right = HOLConstructionRead::SnapToGrid(FVector2D(620.0, 20.0), HouseSize, ConTestTile);
	T.AssertTrue(ConNear(Right.Min.X, Left.Max.X, 0.0001), "side by side: the edges meet");
	T.AssertFalse(HOLConstructionRead::Overlaps(Left, Right), "touching is no overlap");

	FHOLConstructionBox Moved = HOLConstructionRead::SnapToGrid(FVector2D(1349.0, -149.0), HouseSize, ConTestTile);
	T.AssertTrue(ConNearV(Moved.Min, Even.Min) && ConNearV(Moved.Max, Even.Max), "still within half a tile");
	FHOLConstructionBox Next = HOLConstructionRead::SnapToGrid(FVector2D(1460.0, -60.0), HouseSize, ConTestTile);
	T.AssertTrue(ConNearV(Next.Min, FVector2D(Even.Min.X + ConTestTile, Even.Min.Y)), "one tile further");

	FVector2D ForumSize(1200.0, 1200.0);
	double Inv = 0.70710678118654752;
	FVector2D Diagonal(-1.0 * 15000.0 * Inv, 1.0 * 15000.0 * Inv);
	FHOLConstructionBox Forum = HOLConstructionRead::SnapToGrid(Diagonal, ForumSize, ConTestTile);
	T.AssertTrue(ConOnGrid(Forum, ConTestTile), "diagonal start: the centre's footprint on the grid");
	T.AssertTrue(ConNearV(ConCentre(Forum), FVector2D(-10500.0, 10500.0)), "diagonal start: the centre at (-105, 105) m");
	T.AssertTrue(Math::Abs(ConCentre(Forum).X - Diagonal.X) <= ConTestTile * 0.5 && Math::Abs(ConCentre(Forum).Y - Diagonal.Y) <= ConTestTile * 0.5, "diagonal start: moved half a tile at most each way");
	FHOLConstructionBox Beside = HOLConstructionRead::SnapToGrid(FVector2D(Forum.Max.X + ConTestTile + 40.0, ConCentre(Forum).Y + 100.0), HouseSize, ConTestTile);
	T.AssertTrue(ConNear(Beside.Min.X, Forum.Max.X, 0.0001), "a house beside the centre: the edges meet");
	T.AssertFalse(HOLConstructionRead::Overlaps(Beside, Forum), "a house beside the centre: no overlap");
	T.AssertTrue(ConNearV(ConCentre(HOLConstructionRead::SnapToGrid(FVector2D(-15000.0, 0.0), ForumSize, ConTestTile)), FVector2D(-15000.0, 0.0)), "a start on the grid already stays");

	T.AssertEquals(0.0f, HOLConstructionRead::SnapYaw(44.0f), "yaw 44 -> 0");
	T.AssertEquals(-90.0f, HOLConstructionRead::SnapYaw(-91.0f), "yaw -91 -> -90");
	T.AssertEquals(-90.0f, HOLConstructionRead::SnapYaw(300.0f), "yaw 300 -> -90");
	T.AssertEquals(180.0f, Math::Abs(HOLConstructionRead::SnapYaw(180.0f)), "yaw 180 -> 180");

	FHOLConstructionBox Free = HOLConstructionRead::SnapToGrid(FVector2D(1234.0, -60.0), HouseSize, 0.0);
	T.AssertTrue(ConNearV(ConCentre(Free), FVector2D(1234.0, -60.0)), "no grid: centred on the point");
}

// Placement: buildings, sources, map bounds, anti-rush, navmesh.
void Test_Construction_Placement(FUnitTest& T)
{
	TArray<FHOLConstructionBox> NoBuildings;
	TArray<FVector> NoSources;
	TArray<FVector2D> NoEnemies;
	FHOLConstructionBox NoMap;
	FHOLConstructionBox Map = ConMap();

	T.AssertEquals(0, ConCheck(ConHouse(0.0, 0.0), NoBuildings, NoSources, Map, NoEnemies, true), "open ground: placed");

	TArray<FHOLConstructionBox> Buildings;
	Buildings.Add(ConHouse(6.0, 0.0));
	Buildings.Add(ConHouse(-21.0, -21.0));
	T.AssertEquals(0, ConCheck(ConHouse(0.0, 0.0), Buildings, NoSources, Map, NoEnemies, true), "touching a house: placed");
	T.AssertEquals(2, ConCheck(ConHouse(3.0, 0.0), Buildings, NoSources, Map, NoEnemies, true), "overlapping a house: refused");

	TArray<FVector> Sources;
	Sources.Add(FVector(0.0, 370.0, 60.0));
	T.AssertEquals(0, ConCheck(ConHouse(0.0, 0.0), NoBuildings, Sources, Map, NoEnemies, true), "tree just clear of the edge: placed");
	Sources[0] = FVector(0.0, 330.0, 60.0);
	T.AssertEquals(3, ConCheck(ConHouse(0.0, 0.0), NoBuildings, Sources, Map, NoEnemies, true), "tree over the edge: refused");
	Sources[0] = FVector(0.0, 0.0, 300.0);
	T.AssertEquals(3, ConCheck(ConHouse(0.0, 0.0), NoBuildings, Sources, Map, NoEnemies, true), "vein under the footprint: refused");
	Sources[0] = FVector(600.0, 600.0, 300.0);
	T.AssertEquals(0, ConCheck(ConHouse(0.0, 0.0), NoBuildings, Sources, Map, NoEnemies, true), "vein off the corner (its box would touch, its disc does not): placed");

	FHOLConstructionBox AtEdge = HOLConstructionRead::MakeFootprint(FVector2D(19700.0, 0.0), FVector2D(600.0, 600.0));
	T.AssertEquals(0, ConCheck(AtEdge, NoBuildings, NoSources, Map, NoEnemies, true), "against the map's edge: placed");
	FHOLConstructionBox PastEdge = HOLConstructionRead::MakeFootprint(FVector2D(20000.0, 0.0), FVector2D(600.0, 600.0));
	T.AssertEquals(1, ConCheck(PastEdge, NoBuildings, NoSources, Map, NoEnemies, true), "past the map's edge: refused");
	T.AssertEquals(0, ConCheck(PastEdge, NoBuildings, NoSources, NoMap, NoEnemies, true), "no map bounds: no limit");

	TArray<FVector2D> Enemies;
	Enemies.Add(FVector2D(0.0, 0.0));
	FHOLConstructionBox AtRadius = HOLConstructionRead::MakeFootprint(FVector2D(4000.0 + 300.0, 0.0), FVector2D(600.0, 600.0));
	T.AssertEquals(0, ConCheck(AtRadius, NoBuildings, NoSources, NoMap, Enemies, true), "at the anti-rush radius: placed");
	FHOLConstructionBox Inside = HOLConstructionRead::MakeFootprint(FVector2D(3900.0 + 300.0, 0.0), FVector2D(600.0, 600.0));
	T.AssertEquals(4, ConCheck(Inside, NoBuildings, NoSources, NoMap, Enemies, true), "1 m inside the anti-rush radius: refused");
	T.AssertEquals(0, ConCheck(Inside, NoBuildings, NoSources, NoMap, NoEnemies, true), "the same place near one's own centre: placed");

	T.AssertEquals(5, ConCheck(Inside, NoBuildings, NoSources, NoMap, NoEnemies, false), "off the navmesh: refused");
}

// P3 (D07): a building of an age the player's tier has not reached cannot be placed.
void Test_Construction_Age(FUnitTest& T)
{
	for (int Age = 0; Age <= 3; ++Age)
	{
		for (int Tier = 0; Tier <= 3; ++Tier)
		{
			T.AssertEquals(Age <= Tier, HOLConstructionRead::IsAgeOpen(Age, Tier), "Age " + Age + " at tier " + Tier);
		}
	}
	T.AssertTrue(HOLConstructionRead::IsAgeOpen(0, 0), "Age 0 is always open");
	T.AssertFalse(HOLConstructionRead::IsAgeOpen(1, 0), "Age 1 is shut at tier 0");
	T.AssertTrue(HOLConstructionRead::IsAgeOpen(0, -1), "A negative tier is tier 0 for the age 0");
}

// Payment at placement and the D345 refund (the share not built, rounded down for each resource).
void Test_Construction_Payment(FUnitTest& T)
{
	FHOLResources Cost = ConRes(0, 50, 0, 0);
	FHOLResources Stock = ConRes(200, 120, 0, 0);
	T.AssertTrue(HOLConstructionRead::TryPay(Stock, Cost), "first house paid");
	T.AssertTrue(HOLConstructionRead::TryPay(Stock, Cost), "second house paid");
	T.AssertTrue(ConResEq(Stock, ConRes(200, 20, 0, 0)), "cost taken at placement");
	T.AssertFalse(HOLConstructionRead::TryPay(Stock, Cost), "third refused: 20 wood left");
	T.AssertTrue(ConResEq(Stock, ConRes(200, 20, 0, 0)), "nothing taken by a refusal");

	T.AssertTrue(ConResEq(HOLConstructionRead::GetRefund(Cost, 0, 1000), Cost), "cancelled before the first work: all back");
	T.AssertTrue(ConResEq(HOLConstructionRead::GetRefund(Cost, 500, 1000), ConRes(0, 25, 0, 0)), "half built: half back");
	T.AssertTrue(ConResEq(HOLConstructionRead::GetRefund(ConRes(0, 100, 0, 0), 250, 1000), ConRes(0, 75, 0, 0)), "a quarter built: three quarters back");
	T.AssertTrue(ConResEq(HOLConstructionRead::GetRefund(ConRes(1, 75, 3, 0), 500, 1000), ConRes(0, 37, 1, 0)), "rounded down, each resource");
	T.AssertTrue(ConResEq(HOLConstructionRead::GetRefund(Cost, 1, 3), ConRes(0, 33, 0, 0)), "a third built: 2/3 of 50 rounded down");
	T.AssertTrue(ConResEq(HOLConstructionRead::GetRefund(Cost, 1000, 1000), ConRes(0, 0, 0, 0)), "all built: nothing back");
	T.AssertTrue(ConResEq(HOLConstructionRead::GetRefund(Cost, 2000, 1000), ConRes(0, 0, 0, 0)), "work past the total: nothing back");
	T.AssertTrue(ConResEq(HOLConstructionRead::GetRefund(Cost, 10, 0), Cost), "no total: all back");
}

// D345.2a: the damage an attack deals a building (its ranged armour, 40) and its health with the damage taken.
void Test_Construction_Damage(FUnitTest& T)
{
	T.AssertEquals(1, HOLConstructionRead::GetBuildingDamage(5, 1, 40), "5 ranged: 1 at least");
	T.AssertEquals(1, HOLConstructionRead::GetBuildingDamage(40, 1, 40), "40 ranged: 1 at least");
	T.AssertEquals(10, HOLConstructionRead::GetBuildingDamage(50, 1, 40), "50 ranged: 10");
	T.AssertEquals(20, HOLConstructionRead::GetBuildingDamage(20, 0, 40), "20 melee: 20, no armour");
	T.AssertEquals(30, HOLConstructionRead::GetBuildingDamage(30, 2, 40), "30 siege: 30, no armour");
	T.AssertEquals(12, HOLConstructionRead::GetBuildingDamage(12, 3, 40), "12 hero: 12, no armour");

	int64 Total = 1000;
	T.AssertEquals(1, HOLConstructionRead::GetHealthWithDamage(600, 0, Total, 0), "a bare foundation: 1");
	T.AssertEquals(0, HOLConstructionRead::GetHealthWithDamage(600, 0, Total, 1), "a bare foundation hit once: 0");
	T.AssertEquals(200, HOLConstructionRead::GetHealthWithDamage(600, 500, Total, 100), "half built, 100 taken: 200");
	T.AssertEquals(500, HOLConstructionRead::GetHealthWithDamage(600, Total, Total, 100), "then finished: 500, the damage kept");
	T.AssertEquals(0, HOLConstructionRead::GetHealthWithDamage(600, 500, Total, 1000), "never below 0");

	// The same as adding the difference of GetHealth at each step (D312's rule), damage taken on the way.
	int Health = HOLConstructionRead::GetHealth(600, 0, Total);
	int Damage = 0;
	bool bSame = true;
	for (int64 Work = 0; Work < Total; Work += 37)
	{
		int64 NextWork = Work + 37;
		if (NextWork > Total)
			NextWork = Total;
		Health += HOLConstructionRead::GetHealth(600, NextWork, Total) - HOLConstructionRead::GetHealth(600, Work, Total);
		if (Work == 370)
		{
			Health -= 50;
			Damage += 50;
		}
		bSame = bSame && HOLConstructionRead::GetHealthWithDamage(600, NextWork, Total, Damage) == Health;
	}
	T.AssertTrue(bSame, "step by step: the same health");
	T.AssertEquals(550, Health, "finished with 50 taken: 550");
}

// Builders: diminishing returns, step by step, health rising to the full.
void Test_Construction_Builders(FUnitTest& T)
{
	int BuildSteps = HOLConstructionRead::GetBuildSteps(20.0f, 0.05f);
	T.AssertEquals(400, BuildSteps, "20 s = 400 steps for one builder");
	T.AssertEquals(400, HOLConstructionRead::GetStepsToFinish(BuildSteps, 1, ConTestExponent), "one builder: the table's time");

	int Three = HOLConstructionRead::GetStepsToFinish(BuildSteps, 3, ConTestExponent);
	T.AssertTrue(Math::Abs(400.0 / Three - 2.0) < 0.02, "3 builders ~2x as fast");
	int Previous = 0;
	int PreviousGain = 2147483647;
	bool bFaster = true;
	bool bLess = true;
	for (int Builders = 1; Builders <= 10; ++Builders)
	{
		int Rate = HOLConstructionRead::GetWorkPerStep(Builders, ConTestExponent);
		bFaster = bFaster && Rate > Previous;
		bLess = bLess && (Rate - Previous < PreviousGain);
		PreviousGain = Rate - Previous;
		Previous = Rate;
	}
	T.AssertTrue(bFaster, "each builder more is faster than the one before");
	T.AssertTrue(bLess, "each builder adds less than the one before");
	int PerBuilder = HOLConstructionRead::GetWorkPerBuilder();
	T.AssertTrue(HOLConstructionRead::GetWorkPerStep(10, ConTestExponent) < 5 * PerBuilder, "10 builders: far from 10x");
	T.AssertEquals(4 * PerBuilder, HOLConstructionRead::GetWorkPerStep(4, 1.0f), "exponent 1: each as useful as the first");
	T.AssertEquals(0, HOLConstructionRead::GetWorkPerStep(0, ConTestExponent), "no builder, no work");

	int64 Total = HOLConstructionRead::GetTotalWork(BuildSteps);
	int64 Work = 0;
	T.AssertEquals(1, HOLConstructionRead::GetHealth(600, Work, Total), "a bare foundation: 1 HP");
	T.AssertFalse(HOLConstructionRead::AddWork(Work, Total, 0, ConTestExponent), "no builder: no progress");
	T.AssertTrue(Work == 0, "still nothing done");
	int Steps = 0;
	int Finishes = 0;
	int Health = HOLConstructionRead::GetHealth(600, Work, Total);
	bool bHealthRising = true;
	while (Steps < 1000 && Work < Total)
	{
		int64 Before = Work;
		if (HOLConstructionRead::AddWork(Work, Total, 3, ConTestExponent))
			Finishes += 1;
		int Gain = HOLConstructionRead::GetHealth(600, Work, Total) - HOLConstructionRead::GetHealth(600, Before, Total);
		bHealthRising = bHealthRising && Gain >= 0;
		Health += Gain;
		++Steps;
	}
	T.AssertEquals(Three, Steps, "3 builders step by step: the steps foreseen");
	T.AssertEquals(1, Finishes, "finished once");
	T.AssertFalse(HOLConstructionRead::AddWork(Work, Total, 3, ConTestExponent), "a finished building takes no more work");
	T.AssertEquals(1.0f, HOLConstructionRead::GetProgress(Work, Total), "progress capped at 1");
	T.AssertTrue(bHealthRising, "health never goes down while building");
	T.AssertEquals(600, Health, "unhurt: exactly the full health at the end");
	T.AssertEquals(300, HOLConstructionRead::GetHealth(600, Total / 2, Total), "half done: half the health");
	T.AssertEquals(0.5f, HOLConstructionRead::GetProgress(Total / 2, Total), "half done: half the progress");
}

// Population: only once finished, 175 at most (D03).
void Test_Construction_Population(FUnitTest& T)
{
	int Provided = HOLConstructionRead::GetPopulationProvided(10, true);
	Provided += HOLConstructionRead::GetPopulationProvided(10, true) * 2;
	Provided += HOLConstructionRead::GetPopulationProvided(10, false);
	T.AssertEquals(30, HOLProductionRead::GetPopulationCap(Provided), "an unfinished house gives nothing");

	Provided = HOLConstructionRead::GetPopulationProvided(10, true);
	for (int House = 0; House < 20; ++House)
	{
		Provided += HOLConstructionRead::GetPopulationProvided(10, true);
	}
	T.AssertEquals(175, HOLProductionRead::GetPopulationCap(Provided), "175 at most (D03)");
}

// The construction states: foundation, under construction, finished.
void Test_Construction_States(FUnitTest& T)
{
	FHOLResources Cost = ConRes(0, 50, 0, 0);
	int Foundation = 0;
	int Under = 1;
	int Finished = 2;

	T.AssertFalse(HOLConstructionRead::BlocksNavigation(Foundation, false), "a foundation does not block the navmesh (D312)");
	T.AssertFalse(HOLConstructionRead::IsServing(Foundation), "a foundation serves nothing");
	T.AssertTrue(HOLConstructionRead::CanCancel(Foundation), "a foundation may be cancelled");
	T.AssertFalse(HOLConstructionRead::HasWorkStarted(Foundation, 0), "a foundation has no work");
	T.AssertTrue(ConResEq(HOLConstructionRead::GetCancelRefund(Cost, Foundation, 0, 1000), Cost), "a foundation cancelled: all back (D311)");

	T.AssertTrue(HOLConstructionRead::BlocksNavigation(Under, false), "under construction: blocks the navmesh (D312)");
	T.AssertFalse(HOLConstructionRead::IsServing(Under), "under construction: serves nothing yet");
	T.AssertTrue(HOLConstructionRead::HasWorkStarted(Under, 0), "under construction: work started");
	T.AssertTrue(ConResEq(HOLConstructionRead::GetCancelRefund(Cost, Under, 1000, 2000), ConRes(0, 25, 0, 0)), "under construction, half built, cancelled: half back");
	T.AssertTrue(ConResEq(HOLConstructionRead::GetCancelRefund(Cost, Under, 1600, 2000), ConRes(0, 10, 0, 0)), "under construction, 80 % built: 10 back");
	T.AssertTrue(ConResEq(HOLConstructionRead::GetCancelRefund(Cost, Under, 0, 2000), Cost), "under construction with no work yet: all back");

	T.AssertTrue(HOLConstructionRead::BlocksNavigation(Finished, false), "finished: blocks the navmesh");
	T.AssertTrue(HOLConstructionRead::IsServing(Finished), "finished: serves its owner");
	T.AssertFalse(HOLConstructionRead::CanCancel(Finished), "finished: not cancelled");
	T.AssertTrue(ConResEq(HOLConstructionRead::GetCancelRefund(Cost, Finished, 0, 1000), ConRes(0, 0, 0, 0)), "finished: nothing back");

	T.AssertEquals(0, HOLConstructionRead::GetPopulationProvided(10, HOLConstructionRead::IsServing(Foundation)), "a house's foundation gives no place");
	T.AssertEquals(10, HOLConstructionRead::GetPopulationProvided(10, HOLConstructionRead::IsServing(Finished)), "a finished house gives 10");

	// The walkable type (the farm, IA1.2-Q3): never blocks, in any state.
	T.AssertFalse(HOLConstructionRead::BlocksNavigation(Finished, true), "a walkable type (farm) never blocks, finished");
	T.AssertFalse(HOLConstructionRead::BlocksNavigation(Under, true), "a walkable type (farm) never blocks, under construction");
}

// Footprint bounds of a turned building and the navmesh probes under a placement (D311).
void Test_Construction_Probes(FUnitTest& T)
{
	FVector2D Square(1200.0, 1200.0);
	TArray<float> Yaws;
	Yaws.Add(0.0f);
	Yaws.Add(90.0f);
	Yaws.Add(-90.0f);
	Yaws.Add(180.0f);
	for (float Yaw : Yaws)
	{
		FHOLConstructionBox Box = HOLConstructionRead::GetFootprintBounds(FVector2D(-15000.0, 0.0), Square, Yaw);
		T.AssertTrue(ConNearV(Box.Min, FVector2D(-15600.0, -600.0)) && ConNearV(Box.Max, FVector2D(-14400.0, 600.0)), "a square turned by " + Yaw + ": the same box");
	}
	T.AssertTrue(ConNearV(ConSize(HOLConstructionRead::GetFootprintBounds(FVector2D(0.0, 0.0), FVector2D(600.0, 300.0), 90.0f)), FVector2D(300.0, 600.0)), "a 6 x 3 m footprint turned a quarter: 3 x 6 m");
	T.AssertTrue(ConNearV(ConSize(HOLConstructionRead::GetFootprintBounds(FVector2D(0.0, 0.0), FVector2D(600.0, 600.0), 45.0f)), FVector2D(848.53, 848.53), 0.1), "turned by 45 degrees: its bounds");

	TArray<FVector2D> Probes = HOLConstructionRead::GetNavProbes(ConHouse(0.0, 0.0), ConTestTile, 100.0);
	T.AssertEquals(8, Probes.Num(), "a house: 4 tile middles and 4 corners");
	TArray<FVector2D> Expected;
	Expected.Add(FVector2D(-150.0, -150.0));
	Expected.Add(FVector2D(-150.0, 150.0));
	Expected.Add(FVector2D(150.0, -150.0));
	Expected.Add(FVector2D(150.0, 150.0));
	Expected.Add(FVector2D(-200.0, -200.0));
	Expected.Add(FVector2D(200.0, -200.0));
	Expected.Add(FVector2D(-200.0, 200.0));
	Expected.Add(FVector2D(200.0, 200.0));
	for (FVector2D Point : Expected)
	{
		bool bFound = false;
		for (FVector2D Probe : Probes)
		{
			if (ConNearV(Probe, Point))
				bFound = true;
		}
		T.AssertTrue(bFound, "probe at (" + Point.X + ", " + Point.Y + ")");
	}
	double Nearest = 1.0e30;
	for (FVector2D Probe : Probes)
	{
		double Distance = HOLConstructionRead::GetDistance(ConHouse(6.0, 0.0), Probe);
		if (Distance < Nearest)
			Nearest = Distance;
	}
	T.AssertTrue(Nearest >= 100.0, "a neighbour touching it: its forbidden area (85 cm) far from every probe (" + Nearest + " cm)");

	TArray<FVector2D> Forum = HOLConstructionRead::GetNavProbes(HOLConstructionRead::MakeFootprint(FVector2D(0.0, 0.0), Square), ConTestTile, 100.0);
	T.AssertEquals(20, Forum.Num(), "a main centre: 16 tile middles and 4 corners");
	TArray<FVector2D> One = HOLConstructionRead::GetNavProbes(ConHouse(3.0, 3.0), 0.0, 0.0);
	T.AssertTrue(One.Num() == 1 && ConNearV(One[0], FVector2D(300.0, 300.0)), "no tile, no inset: the middle alone");
}

// The builders' spots around a footprint and "at the site".
void Test_Construction_BuildSpots(FUnitTest& T)
{
	double Radius = 60.0;
	double Offset = Radius + 25.0 + 20.0;
	double Spacing = 2.0 * Radius + 10.0;
	FHOLConstructionBox Footprint = ConHouse(0.0, 0.0);
	FVector2D From(-2000.0, 100.0);
	TArray<FVector2D> Spots = HOLConstructionRead::GetBuildSpots(Footprint, Offset, Spacing, From);
	T.AssertEquals(20, Spots.Num(), "a house: 5 spots on each of its 4 sides");
	if (Spots.Num() < 20)
		return;

	bool bAtOffset = true;
	bool bApart = true;
	for (int Index = 0; Index < Spots.Num(); ++Index)
	{
		bAtOffset = bAtOffset && ConNear(HOLConstructionRead::GetDistance(Footprint, Spots[Index]), Offset);
		for (int Other = Index + 1; Other < Spots.Num(); ++Other)
		{
			double Dx = Spots[Index].X - Spots[Other].X;
			double Dy = Spots[Index].Y - Spots[Other].Y;
			bApart = bApart && Math::Sqrt(Dx * Dx + Dy * Dy) >= Spacing - 0.01;
		}
	}
	T.AssertTrue(bAtOffset, "each spot at its offset from the footprint (a body at contact, clear of the navmesh's forbidden area)");
	T.AssertTrue(bApart, "two builders' bodies never overlap");

	T.AssertTrue(ConNearV(Spots[0], FVector2D(-300.0 - Offset, 130.0)), "the first spot on the side facing them (west), nearest them");
	for (int Index = 0; Index < 5; ++Index)
	{
		T.AssertTrue(ConNear(Spots[Index].X, -300.0 - Offset), "spot " + Index + " on the west side");
	}
	int East = 0;
	int North = 0;
	int South = 0;
	for (FVector2D Spot : Spots)
	{
		if (ConNear(Spot.X, 300.0 + Offset))
			East += 1;
		if (ConNear(Spot.Y, 300.0 + Offset))
			North += 1;
		if (ConNear(Spot.Y, -300.0 - Offset))
			South += 1;
	}
	T.AssertEquals(5, East, "5 on the east side too");
	T.AssertEquals(5, North, "5 on the north side");
	T.AssertEquals(5, South, "5 on the south side");
	T.AssertTrue(ConNear(Spots[Spots.Num() - 1].X, 300.0 + Offset), "the far side (east) last");
	TArray<FVector2D> Again = HOLConstructionRead::GetBuildSpots(Footprint, Offset, Spacing, From);
	bool bSameOrder = Again.Num() == Spots.Num();
	for (int Index = 0; bSameOrder && Index < Spots.Num(); ++Index)
	{
		bSameOrder = ConNearV(Again[Index], Spots[Index], 0.0001);
	}
	T.AssertTrue(bSameOrder, "the same order at every call");

	T.AssertTrue(HOLConstructionRead::IsAtSite(Footprint, Spots[0], Radius, 100.0), "at its spot: at the site");
	T.AssertTrue(HOLConstructionRead::IsAtSite(Footprint, FVector2D(Spots[0].X - 50.0, Spots[0].Y), Radius, 100.0), "stopped 50 cm short of its spot: still at the site");
	T.AssertFalse(HOLConstructionRead::IsAtSite(Footprint, FVector2D(-300.0 - 200.0 - Radius, 0.0), Radius, 100.0), "2 m out: not at the site");
}

// The site a builder goes on to: the gaps and the nearest site within range.
void Test_Construction_NextSite(FUnitTest& T)
{
	FHOLConstructionBox Finished = ConHouse(0.0, 0.0);
	T.AssertTrue(ConNear(HOLConstructionRead::GetGap(Finished, ConHouse(6.0, 0.0)), 0.0), "gap to a house touching it");
	T.AssertTrue(ConNear(HOLConstructionRead::GetGap(Finished, ConHouse(16.0, 0.0)), 1000.0), "gap to a house 10 m off its side");
	T.AssertTrue(ConNear(HOLConstructionRead::GetGap(Finished, ConHouse(9.0, 10.0)), 500.0), "gap to a house off its corner (3 m, 4 m)");
	T.AssertTrue(ConNear(HOLConstructionRead::GetGap(Finished, ConHouse(2.0, 2.0)), 0.0), "gap to an overlapping box");

	TArray<FHOLConstructionBox> Sites;
	Sites.Add(ConHouse(-46.0, 0.0));
	Sites.Add(ConHouse(0.0, 16.0));
	Sites.Add(ConHouse(0.0, -19.0));
	T.AssertEquals(1, HOLConstructionRead::FindNextSite(Finished, Sites, 2000.0), "10 m away chosen, not 40 m away nor 13 m");

	TArray<FHOLConstructionBox> Far;
	Far.Add(ConHouse(-46.0, 0.0));
	T.AssertEquals(-1, HOLConstructionRead::FindNextSite(Finished, Far, 2000.0), "only 40 m away: none, idle");
	TArray<FHOLConstructionBox> Exact;
	Exact.Add(ConHouse(26.0, 0.0));
	T.AssertEquals(0, HOLConstructionRead::FindNextSite(Finished, Exact, 2000.0), "exactly 20 m away: still near");
	TArray<FHOLConstructionBox> Two;
	Two.Add(ConHouse(16.0, 0.0));
	Two.Add(ConHouse(-16.0, 0.0));
	T.AssertEquals(0, HOLConstructionRead::FindNextSite(Finished, Two, 2000.0), "two as near: the first");
	TArray<FHOLConstructionBox> None;
	T.AssertEquals(-1, HOLConstructionRead::FindNextSite(Finished, None, 2000.0), "no site: none");
	T.AssertEquals(-1, HOLConstructionRead::FindNextSite(Finished, Sites, 0.0), "range 0: only one touching it");
}
