// Port of HOLConstructionTests.cpp (MIG-C-Construction, ELROND-1117n, D830/D839): same vectors, one by one, against UHOLConstructionReadLibrary
// (namespace HOLConstructionRead, wrappers of HOLConstruction in Buildings/HOLConstructionRules.h). NOT COMPILED when written: the C++ library is new.
// Ported: Grid, Placement, Age, Payment, Damage, Builders, Population, States, Probes, BuildSpots, NextSite, CutPaths, SiteKeepOut, Body (14 of 20; Body without its UHOLBuildingSettings part).
// Tables ported by CELEBORN-1118m (Test_Construction_Tables, the row fields being exposed); not ported (types to expose): CampSource FindCampSource (FHOLCampSource; its gap and SendsBuildersToGather are in Test_Construction_CampGap), IdleWall (FWallBody, EHOLUnitActivity), FreeFootprint (TFunctionRef). BuildQueue, WorkQueue: ELROND-1117o (int queues).
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

FVector2D ConPoint(double X, double Y)
{
	return FVector2D(X * 100.0, Y * 100.0);
}

TArray<FVector2D> ConWalk2(FVector2D A, FVector2D B)
{
	TArray<FVector2D> Out;
	Out.Add(A);
	Out.Add(B);
	return Out;
}

// J6b.2 (D314): the ways cut by a new footprint (segment distance, walk crossing, a slot pushed out).
void Test_Construction_CutPaths(FUnitTest& T)
{
	FHOLConstructionBox Site = ConHouse(0.0, 0.0);
	double Margin = 85.0;
	T.AssertTrue(ConNear(HOLConstructionRead::GetSegmentDistance(Site, ConPoint(-10.0, 0.0), ConPoint(10.0, 0.0)), 0.0), "a segment through it: 0");
	T.AssertTrue(ConNear(HOLConstructionRead::GetSegmentDistance(Site, ConPoint(-10.0, 1.0), ConPoint(0.0, 1.0)), 0.0), "a segment ending in it: 0");
	T.AssertTrue(ConNear(HOLConstructionRead::GetSegmentDistance(Site, ConPoint(-10.0, 4.0), ConPoint(10.0, 4.0)), 100.0), "a segment beside a side, 1 m off");
	T.AssertTrue(ConNear(HOLConstructionRead::GetSegmentDistance(Site, ConPoint(0.0, 7.5), ConPoint(7.5, 0.0)), 150.0 / Math::Sqrt(2.0)), "a diagonal passing its corner: to the corner (1.06 m off)");
	T.AssertFalse(HOLConstructionRead::DoesWalkCross(Site, Margin, ConWalk2(ConPoint(0.0, 7.5), ConPoint(7.5, 0.0))), "so a walk past the corner, 1.06 m off, is on its way");
	T.AssertTrue(ConNear(HOLConstructionRead::GetSegmentDistance(Site, ConPoint(-10.0, 0.0), ConPoint(-5.0, 0.0)), 200.0), "a segment ending short of it, in line");
	T.AssertTrue(ConNear(HOLConstructionRead::GetSegmentDistance(Site, ConPoint(0.0, 5.0), ConPoint(0.0, 5.0)), 200.0), "a point (a segment of no length)");
	T.AssertTrue(ConNear(HOLConstructionRead::GetSegmentDistance(Site, ConPoint(-10.0, 3.0), ConPoint(10.0, 3.0)), 0.0), "a segment along a side, on it: 0");

	T.AssertTrue(HOLConstructionRead::DoesWalkCross(Site, Margin, ConWalk2(ConPoint(-10.0, 0.0), ConPoint(10.0, 0.0))), "a walk straight through it: cut");
	T.AssertTrue(HOLConstructionRead::DoesWalkCross(Site, Margin, ConWalk2(ConPoint(-10.0, 3.8), ConPoint(10.0, 3.8))), "a walk 80 cm off its side: cut (a body would brush it)");
	T.AssertFalse(HOLConstructionRead::DoesWalkCross(Site, Margin, ConWalk2(ConPoint(-10.0, 4.0), ConPoint(10.0, 4.0))), "a walk 1 m off its side: on its way");
	TArray<FVector2D> Round;
	Round.Add(ConPoint(-10.0, 0.0));
	Round.Add(ConPoint(-4.0, 4.0));
	Round.Add(ConPoint(4.0, 4.0));
	Round.Add(ConPoint(10.0, 0.0));
	T.AssertFalse(HOLConstructionRead::DoesWalkCross(Site, Margin, Round), "a walk round it (a detour of the navmesh): on its way");
	TArray<FVector2D> Second;
	Second.Add(ConPoint(-10.0, 10.0));
	Second.Add(ConPoint(-1.0, 5.0));
	Second.Add(ConPoint(1.0, -5.0));
	Second.Add(ConPoint(10.0, -10.0));
	T.AssertTrue(HOLConstructionRead::DoesWalkCross(Site, Margin, Second), "a walk whose second leg crosses it: cut");
	T.AssertTrue(HOLConstructionRead::DoesWalkCross(Site, Margin, ConWalk2(ConPoint(-10.0, 0.0), ConPoint(0.0, 0.0))), "a walk ending in it (its slot now under the house): cut");
	T.AssertFalse(HOLConstructionRead::DoesWalkCross(Site, Margin, ConWalk2(ConPoint(-10.0, 0.0), ConPoint(-4.0, 0.0))), "a walk ending short of it: on its way");
	T.AssertTrue(HOLConstructionRead::DoesWalkCross(Site, Margin + 200.0, ConWalk2(ConPoint(-10.0, 4.5), ConPoint(10.0, 4.5))), "a wider margin (2 m): cut 1.5 m off");
	TArray<FVector2D> Standing;
	Standing.Add(ConPoint(3.5, 0.0));
	T.AssertTrue(HOLConstructionRead::DoesWalkCross(Site, Margin, Standing), "a body standing against it");
	TArray<FVector2D> NoPoint;
	T.AssertFalse(HOLConstructionRead::DoesWalkCross(Site, Margin, NoPoint), "no point: nothing");

	double Out = 105.0;
	T.AssertTrue(ConNearV(HOLConstructionRead::PushOutOfFootprint(Site, Out, ConPoint(1.0, 0.5)), FVector2D(405.0, 202.5)), "a slot east of the middle: out by the east side, at its height");
	T.AssertTrue(ConNearV(HOLConstructionRead::PushOutOfFootprint(Site, Out, ConPoint(2.0, 2.0)), FVector2D(405.0, 405.0)), "a slot by a corner: out along its diagonal");
	T.AssertFalse(ConNearV(HOLConstructionRead::PushOutOfFootprint(Site, Out, ConPoint(1.0, 0.5)), HOLConstructionRead::PushOutOfFootprint(Site, Out, ConPoint(1.0, -0.5)), 1.0), "two slots of a group: two places, not one");
	T.AssertTrue(ConNearV(HOLConstructionRead::PushOutOfFootprint(Site, Out, ConPoint(0.0, 0.0)), FVector2D(405.0, 0.0)), "the middle itself: toward +X");
	T.AssertTrue(ConNearV(HOLConstructionRead::PushOutOfFootprint(Site, Out, ConPoint(0.0, 3.5)), FVector2D(0.0, 405.0)), "a slot within the margin, outside: pushed to it");
	T.AssertTrue(ConNearV(HOLConstructionRead::PushOutOfFootprint(Site, Out, ConPoint(0.0, 5.0)), ConPoint(0.0, 5.0)), "a slot farther: as it is");
}

// D310: a site started under a unit puts it out of the body, beyond the navmesh's margin and the visible wall.
void Test_Construction_SiteKeepOut(FUnitTest& T)
{
	FVector2D Tiles(800.0, 800.0);
	double BodyInset = 105.0;
	double VisualInset = 30.0;
	double Clearance = 25.0;
	FVector2D BodySize = HOLConstructionRead::GetBodySize(Tiles, BodyInset);
	FVector2D VisualSize = HOLConstructionRead::GetBodySize(Tiles, VisualInset);
	FVector2D Half(BodySize.X * 0.5, BodySize.Y * 0.5);
	FHOLConstructionBox Body = HOLConstructionRead::MakeBox(FVector2D(-Half.X, -Half.Y), Half);
	FHOLConstructionBox Visual = HOLConstructionRead::MakeBox(FVector2D(-VisualSize.X * 0.5, -VisualSize.Y * 0.5), FVector2D(VisualSize.X * 0.5, VisualSize.Y * 0.5));
	TArray<double> Radii;
	Radii.Add(60.0);
	Radii.Add(52.0);
	Radii.Add(90.0);
	TArray<FVector2D> Starts;
	Starts.Add(FVector2D(0.0, 0.0));
	Starts.Add(FVector2D(280.0, 50.0));
	Starts.Add(FVector2D(-100.0, -290.0));
	Starts.Add(FVector2D(290.0, 290.0));
	for (double Radius : Radii)
	{
		double InsideGap = HOLConstructionRead::GetKeepOutInsideGap(Radius, Clearance, BodyInset - VisualInset);
		double OutsideGap = Radius + 1.0;
		for (FVector2D Start : Starts)
		{
			FVector2D Local = Start;
			bool bMoved = HOLConstructionRead::KeepOutOfBody(Half, InsideGap, OutsideGap, Local);
			FString Where = "radius " + Radius + " cm, from (" + Start.X + ", " + Start.Y + ") to (" + Local.X + ", " + Local.Y + ")";
			T.AssertTrue(bMoved, "put out (" + Where + ")");
			double FromBody = HOLConstructionRead::GetDistance(Body, Local);
			T.AssertTrue(FromBody >= Clearance + Radius, "beyond the navmesh's margin: " + FromBody + " >= " + (Clearance + Radius) + " (" + Where + ")");
			double FromWall = HOLConstructionRead::GetDistance(Visual, Local);
			T.AssertTrue(FromWall >= Radius, "not in the visible walls: " + FromWall + " >= " + Radius + " (" + Where + ")");
			FVector2D Again = Local;
			T.AssertFalse(HOLConstructionRead::KeepOutOfBody(Half, InsideGap, OutsideGap, Again), "left as it is at the next step (" + Where + ")");
		}
		FVector2D Brushing(Half.X + 40.0, 0.0);
		T.AssertTrue(HOLConstructionRead::KeepOutOfBody(Half, InsideGap, OutsideGap, Brushing), "a centre outside, nearer than its radius: moved");
		T.AssertTrue(ConNear(Brushing.X, Half.X + OutsideGap), "to its radius and 1 cm, no farther");
		FVector2D Clear(Half.X + Radius + 11.0, 0.0);
		T.AssertFalse(HOLConstructionRead::KeepOutOfBody(Half, InsideGap, OutsideGap, Clear), "a centre outside, its body clear: as it is");
	}
}

// D324: a building's body inset in its tiles, the ways between neighbours (the game's settings part of the C++ test is not ported).
void Test_Construction_Body(FUnitTest& T)
{
	double Inset = 105.0;
	T.AssertTrue(ConNearV(HOLConstructionRead::GetBodySize(FVector2D(600.0, 600.0), Inset), FVector2D(390.0, 390.0)), "a house's body: 3.9 m");
	T.AssertTrue(ConNearV(HOLConstructionRead::GetBodySize(FVector2D(1200.0, 1200.0), Inset), FVector2D(990.0, 990.0)), "a main centre's body: 9.9 m");
	T.AssertTrue(ConNearV(HOLConstructionRead::GetBodySize(FVector2D(900.0, 600.0), Inset), FVector2D(690.0, 390.0)), "each side on its own (3 x 2 tiles)");
	T.AssertTrue(ConNearV(HOLConstructionRead::GetBodySize(FVector2D(600.0, 600.0), 0.0), FVector2D(600.0, 600.0)), "no inset: the tiles");
	double MinSide = HOLConstructionRead::GetMinBodySide();
	T.AssertTrue(ConNearV(HOLConstructionRead::GetBodySize(FVector2D(150.0, 150.0), Inset), FVector2D(MinSide, MinSide)), "a tiny footprint keeps a body of MinBodySide");
	T.AssertTrue(ConNearV(HOLConstructionRead::GetBodySize(FVector2D(50.0, 50.0), Inset), FVector2D(50.0, 50.0)), "never larger than its tiles");
	FHOLConstructionBox Tiles = HOLConstructionRead::SnapToGrid(FVector2D(100.0, 100.0), FVector2D(600.0, 600.0), ConTestTile);
	FHOLConstructionBox Body = HOLConstructionRead::GetBody(Tiles, Inset);
	T.AssertTrue(ConNearV(Body.Min, FVector2D(Tiles.Min.X + Inset, Tiles.Min.Y + Inset)) && ConNearV(Body.Max, FVector2D(Tiles.Max.X - Inset, Tiles.Max.Y - Inset)), "the body box: on the tiles' middle, inset on every side");

	FHOLConstructionBox West = HOLConstructionRead::SnapToGrid(FVector2D(0.0, 0.0), FVector2D(600.0, 600.0), ConTestTile);
	FHOLConstructionBox East = HOLConstructionRead::SnapToGrid(FVector2D(600.0, 0.0), FVector2D(600.0, 600.0), ConTestTile);
	T.AssertTrue(ConNear(HOLConstructionRead::GetGap(West, East), 0.0), "their tiles touch");
	T.AssertFalse(HOLConstructionRead::Overlaps(West, East), "so they may stand side by side");
	double Gap = HOLConstructionRead::GetGap(HOLConstructionRead::GetBody(West, Inset), HOLConstructionRead::GetBody(East, Inset));
	T.AssertTrue(ConNear(Gap, 2.0 * Inset), "their bodies: 2.1 m apart");
	double Peasant = 60.0;
	T.AssertTrue(Gap >= 2.0 * Peasant, "one Peasant passes between them");
	T.AssertTrue(Gap < 4.0 * Peasant, "not two abreast");
	double Middle = (West.Max.X + East.Min.X) * 0.5;
	TArray<FVector2D> Through = ConWalk2(FVector2D(Middle, -1000.0), FVector2D(Middle, 1000.0));
	T.AssertFalse(HOLConstructionRead::DoesWalkCross(HOLConstructionRead::GetBody(West, Inset), 85.0, Through), "a walk through the way: not cut by the west body");
	T.AssertFalse(HOLConstructionRead::DoesWalkCross(HOLConstructionRead::GetBody(East, Inset), 85.0, Through), "nor by the east one");
	T.AssertTrue(HOLConstructionRead::DoesWalkCross(West, 85.0, Through), "but by their tiles");

	FHOLConstructionBox North = HOLConstructionRead::SnapToGrid(FVector2D(0.0, 600.0), FVector2D(600.0, 600.0), ConTestTile);
	FHOLConstructionBox Far = HOLConstructionRead::SnapToGrid(FVector2D(2400.0, 0.0), FVector2D(600.0, 600.0), ConTestTile);
	FHOLConstructionBox Diagonal = HOLConstructionRead::SnapToGrid(FVector2D(600.0, 600.0), FVector2D(600.0, 600.0), ConTestTile);
	TArray<FHOLConstructionBox> Bodies;
	Bodies.Add(HOLConstructionRead::GetBody(West, Inset));
	Bodies.Add(HOLConstructionRead::GetBody(East, Inset));
	Bodies.Add(HOLConstructionRead::GetBody(North, Inset));
	Bodies.Add(HOLConstructionRead::GetBody(Far, Inset));
	TArray<FHOLConstructionBox> Ways = HOLConstructionRead::FindWays(Bodies, 2.0 * Inset + HOLConstructionRead::GetTouchTolerance());
	T.AssertEquals(2, Ways.Num(), "two ways: west-east (along Y), west-north (along X); none to the far house");
	if (Ways.Num() == 2)
	{
		T.AssertTrue(ConNearV(Ways[0].Min, FVector2D(Middle - Inset, -300.0 + Inset)) && ConNearV(Ways[0].Max, FVector2D(Middle + Inset, 300.0 - Inset)), "west-east: 2.1 m wide, along their facing sides");
		T.AssertTrue(ConNear(ConSize(Ways[1]).Y, 2.0 * Inset) && ConNear(ConCentre(Ways[1]).Y, 300.0), "west-north: across Y");
	}
	TArray<FHOLConstructionBox> Pair;
	Pair.Add(HOLConstructionRead::GetBody(West, Inset));
	Pair.Add(HOLConstructionRead::GetBody(Diagonal, Inset));
	T.AssertEquals(0, HOLConstructionRead::FindWays(Pair, 2.0 * Inset + 1.0).Num(), "corner to corner: no way");
}

// ---- BuildQueue, WorkQueue, CampSource (gap and which types send builders to gather), ELROND-1117o. NOT COMPILED when written. ----
// Queues are TArray<int32> (sites by number); a work item is an int made by MakeWorkBuild / MakeWorkGather / MakeWorkDeposit.

TArray<int32> ConInts(int32 A = -1, int32 B = -1, int32 C = -1, int32 D = -1)
{
	TArray<int32> Out;
	if (A >= 0) Out.Add(A);
	if (B >= 0) Out.Add(B);
	if (C >= 0) Out.Add(C);
	if (D >= 0) Out.Add(D);
	return Out;
}

bool ConSameInts(TArray<int32> A, TArray<int32> B)
{
	if (A.Num() != B.Num())
		return false;
	for (int32 I = 0; I < A.Num(); I++)
	{
		if (A[I] != B[I])
			return false;
	}
	return true;
}

void Test_Construction_BuildQueue(FUnitTest& T)
{
	// J6a.4 (D311, D312): sites by number; Closed: those finished or cancelled.
	TArray<int32> Closed;
	TArray<int32> Queue;
	HOLConstructionRead::AddToBuildQueue(Queue, 1, false);
	T.AssertTrue(ConSameInts(Queue, ConInts(1)), "a click: its site alone");
	HOLConstructionRead::AddToBuildQueue(Queue, 2, true);
	HOLConstructionRead::AddToBuildQueue(Queue, 3, true);
	T.AssertTrue(ConSameInts(Queue, ConInts(1, 2, 3)), "Shift: queued in the order placed");
	HOLConstructionRead::AddToBuildQueue(Queue, 2, true);
	T.AssertTrue(ConSameInts(Queue, ConInts(1, 2, 3)), "Shift on a site already queued: no second time");

	bool bLeft = HOLConstructionRead::AdvanceBuildQueue(Queue, Closed);
	T.AssertTrue(bLeft && Queue[0] == 1, "the first one open: built first");
	Closed.Add(1);
	bLeft = HOLConstructionRead::AdvanceBuildQueue(Queue, Closed);
	T.AssertTrue(bLeft && Queue[0] == 2, "finished: the next one of the queue");
	Closed.Add(2);
	Closed.Add(3);
	bLeft = HOLConstructionRead::AdvanceBuildQueue(Queue, Closed);
	T.AssertFalse(bLeft, "the last finished, the one after cancelled: idle");
	T.AssertEquals(0, Queue.Num(), "nothing left");

	Queue = ConInts(4, 5, 6);
	Closed.Add(5);
	bLeft = HOLConstructionRead::AdvanceBuildQueue(Queue, Closed);
	T.AssertTrue(bLeft && ConSameInts(Queue, ConInts(4, 5, 6)), "a cancelled site in the middle waits its turn");
	Closed.Add(4);
	bLeft = HOLConstructionRead::AdvanceBuildQueue(Queue, Closed);
	T.AssertTrue(bLeft && ConSameInts(Queue, ConInts(6)), "then skipped with the finished one before it");
	HOLConstructionRead::AddToBuildQueue(Queue, 7, false);
	T.AssertTrue(ConSameInts(Queue, ConInts(7)), "a click without Shift replaces the queue");
}

void Test_Construction_WorkQueue(FUnitTest& T)
{
	// The user's trial of J6a.4: Shift + place house 1, Shift + place house 2, Shift + right click on tree 30, Shift + right click on vein 90.
	TArray<int32> Closed;
	TArray<int32> Queue;
	int32 B1 = HOLConstructionRead::MakeWorkBuild(1);
	int32 B2 = HOLConstructionRead::MakeWorkBuild(2);
	int32 G30 = HOLConstructionRead::MakeWorkGather(30);
	int32 G90 = HOLConstructionRead::MakeWorkGather(90);
	HOLConstructionRead::AddToBuildQueue(Queue, B1, true);
	HOLConstructionRead::AddToBuildQueue(Queue, B2, true);
	HOLConstructionRead::AddToBuildQueue(Queue, G30, true);
	HOLConstructionRead::AddToBuildQueue(Queue, G30, true);
	HOLConstructionRead::AddToBuildQueue(Queue, G90, true);
	T.AssertTrue(ConSameInts(Queue, ConInts(B1, B2, G30, G90)), "sites and sources queued in the order given, a source twice once");
	T.AssertEquals(0, HOLConstructionRead::GetWorkItemKind(Queue[0]), "a site is no gathering");
	T.AssertEquals(1, HOLConstructionRead::GetWorkItemKind(Queue[2]), "a source is one");
	T.AssertTrue(HOLConstructionRead::MakeWorkBuild(0) != HOLConstructionRead::MakeWorkGather(0), "site 0 and source 0 differ");

	bool bLeft = HOLConstructionRead::AdvanceWorkQueue(Queue, false, Closed);
	T.AssertTrue(bLeft && Queue[0] == B1, "house 1 open: built first");
	Closed.Add(1);
	bLeft = HOLConstructionRead::AdvanceWorkQueue(Queue, false, Closed);
	T.AssertTrue(bLeft && Queue[0] == B2, "house 1 finished: house 2");
	Closed.Add(2);
	bLeft = HOLConstructionRead::AdvanceWorkQueue(Queue, false, Closed);
	T.AssertTrue(bLeft && Queue[0] == G30, "house 2 finished: the tree");
	bLeft = HOLConstructionRead::AdvanceWorkQueue(Queue, false, Closed);
	T.AssertTrue(bLeft && Queue[0] == G30 && Queue.Num() == 2, "a gathering lasts while its massif gives (head not done)");
	bLeft = HOLConstructionRead::AdvanceWorkQueue(Queue, true, Closed);
	T.AssertTrue(bLeft && Queue[0] == G90, "its massif used up: the vein");
	bLeft = HOLConstructionRead::AdvanceWorkQueue(Queue, true, Closed);
	T.AssertFalse(bLeft, "the vein used up, nothing after it: idle");
	T.AssertEquals(0, Queue.Num(), "nothing left");

	// A gathering before a site: the site finished by others meanwhile is skipped once the gathering is over.
	Queue = ConInts(HOLConstructionRead::MakeWorkGather(31), HOLConstructionRead::MakeWorkBuild(3), HOLConstructionRead::MakeWorkBuild(4));
	Closed.Add(3);
	bLeft = HOLConstructionRead::AdvanceWorkQueue(Queue, false, Closed);
	T.AssertTrue(bLeft && Queue.Num() == 3 && Queue[0] == HOLConstructionRead::MakeWorkGather(31), "a site finished behind a gathering waits its turn");
	bLeft = HOLConstructionRead::AdvanceWorkQueue(Queue, true, Closed);
	T.AssertTrue(bLeft && Queue.Num() == 1 && Queue[0] == HOLConstructionRead::MakeWorkBuild(4), "the gathering over: the finished site skipped, house 4");
	HOLConstructionRead::AddToBuildQueue(Queue, HOLConstructionRead::MakeWorkGather(32), false);
	T.AssertTrue(ConSameInts(Queue, ConInts(HOLConstructionRead::MakeWorkGather(32))), "a right click without Shift replaces the queue");

	// D485: Shift + right click on the finished mill 5 queues an unload there; a finished building is no open site, yet the deposit waits its turn.
	Closed.Add(5);
	int32 D5 = HOLConstructionRead::MakeWorkDeposit(5);
	Queue = ConInts(HOLConstructionRead::MakeWorkBuild(6));
	HOLConstructionRead::AddToBuildQueue(Queue, D5, true);
	HOLConstructionRead::AddToBuildQueue(Queue, D5, true);
	T.AssertTrue(ConSameInts(Queue, ConInts(HOLConstructionRead::MakeWorkBuild(6), D5)), "a deposit queued after a site, twice once");
	T.AssertEquals(2, HOLConstructionRead::GetWorkItemKind(Queue[1]), "a deposit is neither a site to build nor a gathering");
	T.AssertEquals(0, HOLConstructionRead::GetWorkItemKind(Queue[0]), "a site is no deposit");
	T.AssertTrue(HOLConstructionRead::MakeWorkBuild(5) != D5, "site 5 and deposit 5 differ");
	Closed.Add(6);
	bLeft = HOLConstructionRead::AdvanceWorkQueue(Queue, false, Closed);
	T.AssertTrue(bLeft && Queue.Num() == 1 && Queue[0] == D5, "house 6 finished: the unload at the finished mill, not dropped");
	bLeft = HOLConstructionRead::AdvanceWorkQueue(Queue, false, Closed);
	T.AssertTrue(bLeft && Queue[0] == D5, "not unloaded yet: it stays");
	bLeft = HOLConstructionRead::AdvanceWorkQueue(Queue, true, Closed);
	T.AssertFalse(bLeft, "unloaded, nothing after it: idle");
}

// CampSource, the part that needs no FHOLCampSource: which types send their builders to gather, and the gap to a source. FindCampSource stays in C++.
void Test_Construction_CampGap(FUnitTest& T)
{
	TArray<int32> WoodOnly;
	WoodOnly.Add(1);
	TArray<int32> Mining;
	Mining.Add(2);
	Mining.Add(3);
	TArray<int32> FoodOnly;
	FoodOnly.Add(0);
	TArray<int32> Nothing;
	TArray<int32> All = ConInts(0, 1, 2, 3);
	T.AssertTrue(HOLConstructionRead::SendsBuildersToGather(n"LumberCamp", WoodOnly), "a lumber camp sends its builders to gather");
	T.AssertTrue(HOLConstructionRead::SendsBuildersToGather(n"MiningCamp", Mining), "a mining camp too");
	T.AssertTrue(HOLConstructionRead::SendsBuildersToGather(n"Mill", FoodOnly), "a mill too (D346, the food camp)");
	T.AssertFalse(HOLConstructionRead::SendsBuildersToGather(n"House", Nothing), "a house does not (no deposit)");
	T.AssertFalse(HOLConstructionRead::SendsBuildersToGather(n"MainCentre", All), "a main centre does not (the four resources)");

	FHOLConstructionBox Camp = ConHouse(0.0, 0.0);
	T.AssertTrue(ConNear(HOLConstructionRead::GetSourceGap(Camp, FVector2D(800.0, 0.0), 60.0), 440.0), "gap to a tree's edge off a side (8 m from the middle, 60 cm of body)");
	T.AssertTrue(ConNear(HOLConstructionRead::GetSourceGap(Camp, FVector2D(600.0, 700.0), 300.0), 200.0), "gap to a vein off a corner (3 m, 4 m; 3 m of body)");
	T.AssertTrue(ConNear(HOLConstructionRead::GetSourceGap(Camp, FVector2D(350.0, 0.0), 60.0), 0.0), "a body over the footprint: 0");
}

// ELROND-1117p: Tables, Body (UHOLBuildingSettings part), CampSource (FindCampSource), IdleWall, FreeFootprint, same vectors as the C++ tests (which stay).
// NOT COMPILED when written. Enums as ints: category 0 Economic 1 Military 2 Technological; resource 0 Food 1 Wood 2 Stone 3 Gold;
// activity 0 Idle 1 Moving 2 Gathering 3 GatherTrip 4 Building 5 Attacking 6 Dying; phase 0 None 1 WaitingPath 2 Path 3 FinalLeg 4 FormUp 5 Block.

TArray<int32> ConOneInt(int A)
{
	TArray<int32> Out;
	Out.Add(A);
	return Out;
}

// Body, the game's settings (UHOLBuildingSettings), the part the C++ Body test keeps.
void Test_Construction_BodySettings(FUnitTest& T)
{
	double Peasant = 60.0;
	double Setting = HOLConstructionRead::GetSettingBodyInset();
	T.AssertTrue(2.0 * Setting >= 2.0 * Peasant && 2.0 * Setting < 4.0 * Peasant, "the setting: one Peasant passes, not two");
	T.AssertTrue(ConNearV(HOLConstructionRead::GetSettingBodySize(FVector2D(600.0, 600.0)), FVector2D(600.0 - 2.0 * Setting, 600.0 - 2.0 * Setting)), "the setting's house body");
	double Seen = HOLConstructionRead::GetSettingVisualInset();
	T.AssertTrue(600.0 - 2.0 * Seen >= 0.9 * 600.0 - 0.01, "the visible shape: at least 90 % of a house's tiles");
	T.AssertTrue(Seen <= Setting, "the visible shape wider than the body");
	T.AssertTrue(ConNearV(HOLConstructionRead::GetSettingVisualSize(FVector2D(600.0, 600.0)), FVector2D(600.0 - 2.0 * Seen, 600.0 - 2.0 * Seen)), "the setting's house shape");
}

FHOLConstructionCampSource ConSource(double X, double Y, double Radius, int Resource, bool bStanding = true)
{
	FHOLConstructionCampSource Out;
	Out.Centre = FVector2D(X * 100.0, Y * 100.0);
	Out.Radius = Radius * 100.0;
	Out.Resource = Resource;
	Out.bStanding = bStanding;
	return Out;
}

void Test_Construction_CampSource(FUnitTest& T)
{
	FHOLConstructionBox Camp = ConHouse(0.0, 0.0);
	TArray<FHOLConstructionCampSource> Sources;
	Sources.Add(ConSource(-6.0, 0.0, 1.1, 0));          // berries 1.9 m off: not wood
	Sources.Add(ConSource(10.0, 0.0, 0.6, 1));          // 6.4 m
	Sources.Add(ConSource(0.0, 8.0, 0.6, 1, false));    // 4.4 m, used up
	Sources.Add(ConSource(0.0, -9.0, 0.6, 1));          // 5.4 m: the nearest standing tree
	Sources.Add(ConSource(16.0, 16.0, 3.0, 3));         // 15.4 m
	Sources.Add(ConSource(-20.0, 0.0, 3.0, 2));         // 14 m
	Sources.Add(ConSource(40.0, 0.0, 0.6, 1));          // 36.4 m
	TArray<int32> Wood = ConOneInt(1);
	TArray<int32> Mining;
	Mining.Add(2);
	Mining.Add(3);
	TArray<int32> Food = ConOneInt(0);
	double Gap = -1.0;
	int Found = HOLConstructionRead::FindCampSource(Camp, Wood, Sources, 2500.0, Gap);
	T.AssertEquals(Found, 3, "lumber camp: the nearest standing tree, not the berries nearer, nor the stump");
	T.AssertTrue(ConNear(Gap, 540.0), "its gap");
	Found = HOLConstructionRead::FindCampSource(Camp, Mining, Sources, 2500.0, Gap);
	T.AssertEquals(Found, 5, "mining camp: the nearer of stone and gold");
	T.AssertTrue(ConNear(Gap, 1400.0), "its gap (to the vein's edge)");
	Found = HOLConstructionRead::FindCampSource(Camp, Food, Sources, 2500.0, Gap);
	T.AssertEquals(Found, 0, "mill (D346): the berries, not the trees");
	T.AssertTrue(ConNear(Gap, 190.0), "its gap (to the bush's edge)");

	TArray<FHOLConstructionCampSource> NoBush;
	NoBush.Add(ConSource(-6.0, 0.0, 1.1, 0, false));
	NoBush.Add(ConSource(10.0, 0.0, 0.6, 1));
	Found = HOLConstructionRead::FindCampSource(Camp, Food, NoBush, 2500.0, Gap);
	T.AssertEquals(Found, -1, "mill with no bush left: none, never a tree");
	Found = HOLConstructionRead::FindCampSource(Camp, Mining, Sources, 1000.0, Gap);
	T.AssertEquals(Found, -1, "mining camp within 10 m: none");
	Found = HOLConstructionRead::FindCampSource(Camp, Wood, Sources, 540.0, Gap);
	T.AssertEquals(Found, 3, "exactly at the range: still near");
	TArray<FHOLConstructionCampSource> FarTree;
	FarTree.Add(ConSource(40.0, 0.0, 0.6, 1));
	Found = HOLConstructionRead::FindCampSource(Camp, Wood, FarTree, 2500.0, Gap);
	T.AssertEquals(Found, -1, "only the far tree standing: none within 25 m");
	TArray<FHOLConstructionCampSource> TwoAsNear;
	TwoAsNear.Add(ConSource(10.0, 0.0, 0.6, 1));
	TwoAsNear.Add(ConSource(-10.0, 0.0, 0.6, 1));
	Found = HOLConstructionRead::FindCampSource(Camp, Wood, TwoAsNear, 2500.0, Gap);
	T.AssertEquals(Found, 0, "two as near: the first");
	TArray<int32> Nothing;
	Found = HOLConstructionRead::FindCampSource(Camp, Nothing, Sources, 2500.0, Gap);
	T.AssertEquals(Found, -1, "a camp accepting nothing: none");
	TArray<FHOLConstructionCampSource> NoSource;
	Found = HOLConstructionRead::FindCampSource(Camp, Wood, NoSource, 2500.0, Gap);
	T.AssertEquals(Found, -1, "no source: none");
}

double ConFromSlab(FVector2D P, FVector2D Slab)
{
	double AwayX = Math::Abs(P.X) - Slab.X;
	double AwayY = Math::Abs(P.Y) - Slab.Y;
	double Most = Math::Max(AwayX, AwayY);
	if (Most <= 0.0)
		return Most;
	double Ax = Math::Max(AwayX, 0.0);
	double Ay = Math::Max(AwayY, 0.0);
	return Math::Sqrt(Ax * Ax + Ay * Ay);
}

void Test_Construction_IdleWall(FUnitTest& T)
{
	FVector2D BodySize = HOLConstructionRead::GetBodySize(FVector2D(800.0, 800.0), 115.0);
	FVector2D Half = FVector2D(BodySize.X * 0.5, BodySize.Y * 0.5);
	double VisualGap = 115.0 - 30.0;
	double Radius = 60.0;
	double InsideGap = HOLConstructionRead::GetKeepOutInsideGap(Radius, 25.0, VisualGap);
	double OutsideGap = Radius + 1.0;
	double MaxStep = 10.0;
	FHOLConstructionBox Body = HOLConstructionRead::MakeBox(FVector2D(-Half.X, -Half.Y), Half);
	T.AssertTrue(HOLConstructionRead::StandsClearOfVisibleWall(0, 0, false), "idle, no order, not fighting: stands still");
	T.AssertFalse(HOLConstructionRead::StandsClearOfVisibleWall(1, 2, false), "walking: shoulder");
	T.AssertFalse(HOLConstructionRead::StandsClearOfVisibleWall(0, 1, false), "idle but given a path: shoulder");
	T.AssertFalse(HOLConstructionRead::StandsClearOfVisibleWall(4, 0, false), "building: shoulder");
	T.AssertFalse(HOLConstructionRead::StandsClearOfVisibleWall(2, 0, false), "gathering: shoulder");
	T.AssertFalse(HOLConstructionRead::StandsClearOfVisibleWall(3, 0, false), "at a deposit: shoulder");
	T.AssertFalse(HOLConstructionRead::StandsClearOfVisibleWall(5, 0, false), "striking: shoulder");
	T.AssertFalse(HOLConstructionRead::StandsClearOfVisibleWall(0, 0, true), "idle with a target: shoulder");

	// Its middle on the wall's line: half its body in the wall.
	FVector2D Start = FVector2D(Half.X + VisualGap, 40.0);
	FVector2D Walker = Start;
	bool bEasedWalker = false;
	bool bWalkerMoved = HOLConstructionRead::KeepOutOfVisibleWall(Half, InsideGap, OutsideGap, Radius, VisualGap, false, MaxStep, Walker, bEasedWalker);
	T.AssertFalse(bWalkerMoved, "a walker there: as it is");
	T.AssertTrue(ConNearV(Walker, Start), "a walker there: not moved");

	// Standing still, holding its formation place where it stands: the place goes out with it, it never walks back in.
	FVector2D Unit = Start;
	FVector2D Place = Start;
	FHOLPlaceHoldView Hold;
	HOLFormationRead::EndWalkToPlace(Hold, Unit, Place, Radius);
	int Steps = 0;
	bool bWalkedBack = false;
	double LongestStep = 0.0;
	bool bEasedStep = false;
	for (int Step = 0; Step < 100; ++Step)
	{
		FVector2D Before = Unit;
		if (HOLConstructionRead::KeepOutOfVisibleWall(Half, InsideGap, OutsideGap, Radius, VisualGap, true, MaxStep, Unit, bEasedStep))
			Steps += 1;
		HOLConstructionRead::KeepOutOfVisibleWall(Half, InsideGap, OutsideGap, Radius, VisualGap, true, MaxStep, Place, bEasedStep);
		double Moved = Math::Sqrt((Unit.X - Before.X) * (Unit.X - Before.X) + (Unit.Y - Before.Y) * (Unit.Y - Before.Y));
		LongestStep = Math::Max(LongestStep, Moved);
		if (HOLFormationRead::WatchPlace(Hold, Unit, Place, Radius))
			bWalkedBack = true;
	}
	double FromBody = HOLConstructionRead::GetDistance(Body, Unit);
	T.AssertTrue(FromBody >= VisualGap + Radius, "wholly beyond the visible wall");
	T.AssertTrue(Steps > 1 && LongestStep <= MaxStep + 0.01, "a step at a time");
	T.AssertTrue(HOLConstructionRead::GetDistance(Body, Place) >= VisualGap + Radius, "its place out with it");
	T.AssertFalse(bWalkedBack, "no walk back into the wall");
	FVector2D Again = Unit;
	T.AssertFalse(HOLConstructionRead::KeepOutOfVisibleWall(Half, InsideGap, OutsideGap, Radius, VisualGap, true, MaxStep, Again, bEasedStep), "out, it stays");

	// C.2: the easing out of the visible wall is told apart from a body put back.
	FVector2D Eased = Start;
	bool bEased = false;
	bool bMoved = HOLConstructionRead::KeepOutOfVisibleWall(Half, InsideGap, OutsideGap, Radius, VisualGap, true, MaxStep, Eased, bEased);
	T.AssertTrue(bMoved && bEased, "in the visible wall: eased out");
	FVector2D Touching = FVector2D(Half.X + Radius * 0.5, 0.0);
	bool bTouchingEased = true;
	bMoved = HOLConstructionRead::KeepOutOfVisibleWall(Half, InsideGap, OutsideGap, Radius, VisualGap, true, MaxStep, Touching, bTouchingEased);
	T.AssertTrue(bMoved && !bTouchingEased, "its body in the building's: put back, not eased");
	FVector2D Inside = FVector2D(Half.X - 10.0, 0.0);
	bool bInsideEased = true;
	bMoved = HOLConstructionRead::KeepOutOfVisibleWall(Half, InsideGap, OutsideGap, Radius, VisualGap, true, MaxStep, Inside, bInsideEased);
	T.AssertTrue(bMoved && !bInsideEased, "its centre inside: put back, not eased");
	bool bAgainEased = true;
	bMoved = HOLConstructionRead::KeepOutOfVisibleWall(Half, InsideGap, OutsideGap, Radius, VisualGap, true, MaxStep, Again, bAgainEased);
	T.AssertFalse(bMoved || bAgainEased, "out: not moved, not eased");

	// A builder there: as it is (the unit's state says it does not stand still).
	FVector2D Builder = Start;
	bool bBuilderStill = HOLConstructionRead::StandsClearOfVisibleWall(4, 0, false);
	bool bBuilderEased = false;
	T.AssertFalse(HOLConstructionRead::KeepOutOfVisibleWall(Half, InsideGap, OutsideGap, Radius, VisualGap, bBuilderStill, MaxStep, Builder, bBuilderEased), "a builder there: as it is");

	// C.3: two houses side by side: a unit standing still in the passage stays (D324's shoulder), nothing eased.
	FHOLConstructionWallBody Left;
	Left.HalfExtent = Half;
	FHOLConstructionWallBody Right = Left;
	Right.Centre = FVector2D(800.0, 0.0);
	TArray<FHOLConstructionWallBody> Pair;
	Pair.Add(Left);
	Pair.Add(Right);
	TArray<FVector2D> Passage;
	Passage.Add(FVector2D(400.0, 40.0));
	Passage.Add(FVector2D(360.0, -100.0));
	Passage.Add(FVector2D(430.0, Half.Y - 20.0));
	for (int I = 0; I < Passage.Num(); ++I)
	{
		FVector2D InPassage = Passage[I];
		FVector2D Wedged = InPassage;
		int EasedSteps = 0;
		double Farthest = 0.0;
		for (int Step = 0; Step < 100; ++Step)
		{
			if (HOLConstructionRead::EaseOutOfVisibleWalls(Pair, Radius, VisualGap, MaxStep, Wedged, false))
				EasedSteps += 1;
			double Away = Math::Sqrt((Wedged.X - InPassage.X) * (Wedged.X - InPassage.X) + (Wedged.Y - InPassage.Y) * (Wedged.Y - InPassage.Y));
			Farthest = Math::Max(Farthest, Away);
		}
		T.AssertTrue(EasedSteps == 0 && Farthest < 0.01, "in the passage: stays, nothing eased");
	}
	// Near one of the two only (the far side of the left house): eased out as before, a step at a time.
	FVector2D Alone = FVector2D(-Half.X - VisualGap, 40.0);
	int AloneSteps = 0;
	for (int Step = 0; Step < 100; ++Step)
	{
		if (HOLConstructionRead::EaseOutOfVisibleWalls(Pair, Radius, VisualGap, MaxStep, Alone, false))
			AloneSteps += 1;
	}
	T.AssertTrue(AloneSteps > 1 && HOLConstructionRead::GetDistance(Body, Alone) >= VisualGap + Radius, "by one wall: eased out");

	// EtapeDefini R: a corner of the slab: round corners leave the body in it, square corners put it wholly out.
	FVector2D Slab = FVector2D(Half.X + VisualGap, Half.Y + VisualGap);
	TArray<FHOLConstructionWallBody> One;
	One.Add(Left);
	FVector2D InCorner = FVector2D(Half.X + 60.0, Half.Y + 60.0);
	FVector2D Round = InCorner;
	for (int Step = 0; Step < 100; ++Step)
		HOLConstructionRead::EaseOutOfVisibleWalls(One, Radius, VisualGap, MaxStep, Round, false);
	T.AssertTrue(ConFromSlab(Round, Slab) < Radius, "round corners: its body left in the slab's corner");
	FVector2D Square = InCorner;
	int SquareSteps = 0;
	double SquareLongest = 0.0;
	for (int Step = 0; Step < 100; ++Step)
	{
		FVector2D Before = Square;
		if (HOLConstructionRead::EaseOutOfVisibleWalls(One, Radius, VisualGap, MaxStep, Square, true))
			SquareSteps += 1;
		double Moved = Math::Sqrt((Square.X - Before.X) * (Square.X - Before.X) + (Square.Y - Before.Y) * (Square.Y - Before.Y));
		SquareLongest = Math::Max(SquareLongest, Moved);
	}
	T.AssertTrue(ConFromSlab(Square, Slab) >= Radius && SquareSteps > 1 && SquareLongest <= MaxStep + 0.01, "square corners: wholly out of the slab's corner, a step at a time");
	FVector2D SquareAgain = Square;
	T.AssertFalse(HOLConstructionRead::EaseOutOfVisibleWalls(One, Radius, VisualGap, MaxStep, SquareAgain, true), "square corners: out, it stays");
}

bool ConOnGridLine(double Value, double Grid)
{
	double V = Math::Abs(Value) + 0.01;
	double Rem = V - double(Math::FloorToInt(V / Grid)) * Grid;
	return Rem < 0.02;
}

void Test_Construction_FreeFootprint(FUnitTest& T)
{
	double Tile = ConTestTile;
	FVector2D Aim = FVector2D(100.0, 50.0);
	TArray<FVector> NoSources;
	TArray<FHOLConstructionBox> NoBuildings;
	TArray<FHOLConstructionBox> Anywhere;

	// The cursor on a house at (0, 0): the obstacle is that house, the ghost touches it (ring 0) on the side nearest the aim.
	{
		TArray<FHOLConstructionBox> Houses;
		Houses.Add(ConHouse(0.0, 0.0));
		Houses.Add(ConHouse(30.0, 0.0));
		FHOLConstructionBox Footprint = HOLConstructionRead::SnapToGrid(Aim, FVector2D(600.0, 600.0), Tile);
		FHOLConstructionBox Obstacle;
		T.AssertTrue(HOLConstructionRead::FindObstacle(Footprint, Houses, NoSources, ConMap(), 4000.0, Aim, Tile, Obstacle), "on a house: an obstacle found");
		T.AssertTrue(ConNearV(Obstacle.Min, Houses[0].Min) && ConNearV(Obstacle.Max, Houses[0].Max), "on a house: the house itself, already on the grid");
		FHOLConstructionBox Free;
		int Ring = -1;
		T.AssertTrue(HOLConstructionRead::FindFreeFootprint(Footprint, Houses, NoSources, ConMap(), 4000.0, Obstacle, Aim, Tile, 4, Anywhere, Free, Ring), "beside a house: found");
		T.AssertEquals(Ring, 0, "beside a house: ring 0 (touching)");
		T.AssertTrue(ConNearV(ConCentre(Free), FVector2D(600.0, 0.0)), "beside a house: east, nearest the aim");
		T.AssertFalse(HOLConstructionRead::Overlaps(Free, Houses[0]), "beside a house: touching, not overlapping");
		T.AssertTrue(ConOnGrid(Free, Tile) && ConNearV(FVector2D(Free.Max.X - Free.Min.X, Free.Max.Y - Free.Min.Y), FVector2D(600.0, 600.0)), "beside a house: on the grid, its size");

		// The cursor between two houses, inside the second: the one containing the aim is the obstacle.
		TArray<FHOLConstructionBox> Pair;
		Pair.Add(ConHouse(0.0, 0.0));
		Pair.Add(ConHouse(0.0, 6.0));
		T.AssertTrue(HOLConstructionRead::FindObstacle(ConHouse(0.0, 3.0), Pair, NoSources, ConMap(), 4000.0, FVector2D(0.0, 400.0), Tile, Obstacle), "two houses: an obstacle found");
		T.AssertTrue(ConNearV(Obstacle.Min, Pair[1].Min), "two houses: the one containing the aim");
	}

	// Ring 0 all taken (a frame of buildings one tile wide around the house): ring 1, east again.
	{
		TArray<FHOLConstructionBox> Blocks;
		Blocks.Add(HOLConstructionRead::MakeBox(FVector2D(-600.0, -600.0), FVector2D(600.0, -300.0)));
		Blocks.Add(HOLConstructionRead::MakeBox(FVector2D(-600.0, 300.0), FVector2D(600.0, 600.0)));
		Blocks.Add(HOLConstructionRead::MakeBox(FVector2D(-600.0, -300.0), FVector2D(-300.0, 300.0)));
		Blocks.Add(HOLConstructionRead::MakeBox(FVector2D(300.0, -300.0), FVector2D(600.0, 300.0)));
		FHOLConstructionBox Free;
		int Ring = -1;
		T.AssertTrue(HOLConstructionRead::FindFreeFootprint(ConHouse(0.0, 0.0), Blocks, NoSources, ConMap(), 4000.0, ConHouse(0.0, 0.0), Aim, Tile, 4, Anywhere, Free, Ring), "ring 0 taken: found");
		T.AssertEquals(Ring, 1, "ring 0 taken: ring 1");
		T.AssertTrue(ConNearV(ConCentre(Free), FVector2D(900.0, 0.0)), "ring 0 taken: one tile east of the house");

		// The navmesh refuses that one (every middle at X >= 800): the next of ring 1 nearest the aim (north).
		TArray<FHOLConstructionBox> East;
		East.Add(HOLConstructionRead::MakeBox(FVector2D(800.0, -1000000.0), FVector2D(1000000.0, 1000000.0)));
		T.AssertTrue(HOLConstructionRead::FindFreeFootprint(ConHouse(0.0, 0.0), Blocks, NoSources, ConMap(), 4000.0, ConHouse(0.0, 0.0), Aim, Tile, 4, East, Free, Ring), "off the navmesh: another found");
		T.AssertTrue(Ring == 1 && ConCentre(Free).X < 800.0, "off the navmesh: still ring 1, not east");
	}

	// A source's disc: its bounding box grown outward to the grid lines; a footprint on the grid stays as it is.
	{
		TArray<FVector> Tree;
		Tree.Add(FVector(100.0, 100.0, 250.0));
		FHOLConstructionBox Obstacle;
		T.AssertTrue(HOLConstructionRead::FindObstacle(ConHouse(0.0, 0.0), NoBuildings, Tree, ConMap(), 4000.0, Aim, Tile, Obstacle), "on a tree: an obstacle found");
		T.AssertTrue(ConNearV(Obstacle.Min, FVector2D(-300.0, -300.0)) && ConNearV(Obstacle.Max, FVector2D(600.0, 600.0)), "on a tree: its disc as a box on the grid");
		FHOLConstructionBox OnGrid = HOLConstructionRead::SnapOutward(ConHouse(3.0, 0.0), Tile);
		T.AssertTrue(ConNearV(OnGrid.Min, ConHouse(3.0, 0.0).Min) && ConNearV(OnGrid.Max, ConHouse(3.0, 0.0).Max), "a footprint on the grid: unchanged");
		FHOLConstructionBox Free;
		int Ring = -1;
		T.AssertTrue(HOLConstructionRead::FindFreeFootprint(ConHouse(0.0, 0.0), NoBuildings, Tree, ConMap(), 4000.0, Obstacle, Aim, Tile, 4, Anywhere, Free, Ring), "beside a tree: found");
		T.AssertTrue(Ring == 0 && ConOnGrid(Free, Tile), "beside a tree: ring 0, on the grid");
		FVector2D Disc = FVector2D(Free.Min.X, Free.Min.Y);
		T.AssertTrue(Free.Min.X >= 100.0 + 250.0 - 0.01 || Free.Max.X <= 100.0 - 250.0 + 0.01 || Free.Min.Y >= 100.0 + 250.0 - 0.01 || Free.Max.Y <= 100.0 - 250.0 + 0.01, "beside a tree: clear of the tree");
	}

	// Nothing free within 4 rings (a building covering them all, 50 cm into ring 4): none; with 5 rings, ring 5.
	{
		TArray<FHOLConstructionBox> Big;
		Big.Add(HOLConstructionRead::MakeBox(FVector2D(-1550.0, -1550.0), FVector2D(1550.0, 1550.0)));
		FHOLConstructionBox Free;
		int Ring = -1;
		T.AssertFalse(HOLConstructionRead::FindFreeFootprint(ConHouse(0.0, 0.0), Big, NoSources, ConMap(), 4000.0, ConHouse(0.0, 0.0), Aim, Tile, 4, Anywhere, Free, Ring), "all taken within 4 rings: none");
		bool bFive = HOLConstructionRead::FindFreeFootprint(ConHouse(0.0, 0.0), Big, NoSources, ConMap(), 4000.0, ConHouse(0.0, 0.0), Aim, Tile, 5, Anywhere, Free, Ring);
		T.AssertTrue(bFive && Ring == 5, "5 rings: ring 5");
	}

	// A candidate refused by another rule is skipped: the map ends at the house's east side, the ghost goes north.
	{
		FHOLConstructionBox Map = HOLConstructionRead::MakeBox(FVector2D(-20000.0, -20000.0), FVector2D(300.0, 20000.0));
		FHOLConstructionBox Free;
		int Ring = -1;
		T.AssertTrue(HOLConstructionRead::FindFreeFootprint(ConHouse(0.0, 0.0), NoBuildings, NoSources, Map, 4000.0, ConHouse(0.0, 0.0), Aim, Tile, 4, Anywhere, Free, Ring), "map edge: found");
		T.AssertTrue(Ring == 0 && ConNearV(ConCentre(Free), FVector2D(0.0, 600.0)), "map edge: ring 0, north (east is out of the map)");
	}
}

// Tables (CELEBORN-1118m, port of HeroesOfLiberty.Construction.Tables, D314): the values of the building type table (D310, D313) and of the settings,
// read through HOLConstructionRead::FindBuildingType (the row fields are BlueprintReadOnly since the lot of 2026-11-17). Resources as ints:
// 0 Food, 1 Wood, 2 Stone, 3 Gold; category 0 Economic, 1 Military, 2 Technological. The 4 m tile is read from the settings (GetSettingTileSize).

// One of the four economic types of the C++ table (House, LumberCamp, MiningCamp, Mill): slot, population, resources accepted, wood-only cost, 2 x 2 tiles.
void ConCheckEconomicType(FUnitTest& T, FName Name, int Slot, int Population, TArray<int> Accepts, int Wood, double Grid, double CentreWidth)
{
	FHOLBuildingTypeRow Row;
	bool bFound = HOLConstructionRead::FindBuildingType(Name, Row);
	FString Label = Name.ToString();
	T.AssertTrue(bFound, Label + " in the building type table");
	if (!bFound)
	{
		return;
	}
	T.AssertEquals(Slot, Row.BuildGridSlot, Label + ": slot in the Peasant's grid (D313, age I, economic row: A, Z, E, R)");
	T.AssertEquals(Population, Row.PopulationProvided, Label + ": population given");
	T.AssertTrue(ConSameInts(HOLConstructionRead::GetBuildingTypeAccepts(Row), Accepts), Label + ": resources accepted");
	T.AssertTrue(ConResEq(Row.Cost, ConRes(0, Wood, 0, 0)), Label + ": its cost, in wood only");
	T.AssertTrue(Row.BuildTime > 0.0f, Label + ": a build time");
	T.AssertTrue(ConNearV(Row.Footprint, FVector2D(2.0 * Grid, 2.0 * Grid)), Label + ": 2 x 2 tiles (6 m)");
	T.AssertTrue(Row.Footprint.X < CentreWidth, Label + ": smaller than the main centre");
}

void Test_Construction_Tables(FUnitTest& T)
{
	double Grid = HOLConstructionRead::GetSettingTileSize();
	T.AssertTrue(ConNear(Grid, 400.0), "4 m tiles (D311, D344)");
	T.AssertTrue(HOLConstructionRead::GetSettingAntiRushRadius() > 0.0f, "anti-rush radius set (D22)");
	double Ratio = double(HOLConstructionRead::GetWorkPerStep(3, HOLConstructionRead::GetSettingBuilderExponent())) / double(HOLConstructionRead::GetWorkPerBuilder());
	T.AssertTrue(Math::Abs(Ratio - 2.0) < 0.05, "3 builders ~2x as fast (D312)");

	FHOLBuildingTypeRow MainCentre;
	bool bCentre = HOLConstructionRead::FindBuildingType(n"MainCentre", MainCentre);
	T.AssertTrue(bCentre, "main centre");
	if (!bCentre)
	{
		return;
	}
	T.AssertFalse(HOLConstructionRead::IsBuildingTypeBuildable(MainCentre), "the main centre is not built by Peasants");
	T.AssertTrue(ConNearV(MainCentre.Footprint, FVector2D(4.0 * Grid, 4.0 * Grid)), "the main centre: 4 x 4 tiles (16 m)");

	ConCheckEconomicType(T, n"House", 0, 10, ConInts(), 50, Grid, MainCentre.Footprint.X);
	ConCheckEconomicType(T, n"LumberCamp", 1, 0, ConInts(1), 100, Grid, MainCentre.Footprint.X);
	ConCheckEconomicType(T, n"MiningCamp", 2, 0, ConInts(2, 3), 100, Grid, MainCentre.Footprint.X);
	ConCheckEconomicType(T, n"Mill", 3, 0, ConInts(0), 100, Grid, MainCentre.Footprint.X);

	// J7a.1 (J7-Q2): the Barracks, built like a house; J7a.2: on R of the Peasant's grid (military row).
	FHOLBuildingTypeRow Barracks;
	bool bBarracks = HOLConstructionRead::FindBuildingType(n"Barracks", Barracks);
	T.AssertTrue(bBarracks, "Barracks in the building type table");
	if (bBarracks)
	{
		T.AssertTrue(HOLConstructionRead::IsBuildingTypeBuildable(Barracks), "Barracks: built by Peasants");
		T.AssertEquals(4, Barracks.BuildGridSlot, "Barracks: Q of the Peasant's grid (slot 4, military row, D313)");
		T.AssertTrue(ConNearV(Barracks.Footprint, FVector2D(3.0 * Grid, 3.0 * Grid)), "Barracks: 3 x 3 tiles (12 m)");
		T.AssertTrue(ConResEq(Barracks.Cost, ConRes(0, 150, 0, 0)), "Barracks: 150 wood");
		T.AssertTrue(ConNear(Barracks.BuildTime, 20.0), "Barracks: 20 s for one builder");
		T.AssertEquals(1500, Barracks.MaxHealth, "Barracks: 1 500 health");
		T.AssertTrue(ConNear(Barracks.Height, 600.0), "Barracks: 6 m high");
	}

	// AR-Q2a: the Archery range, built like the Barracks, on S of the Peasant's grid, no prerequisite.
	FHOLBuildingTypeRow Archery;
	bool bArchery = HOLConstructionRead::FindBuildingType(n"ArcheryRange", Archery);
	T.AssertTrue(bArchery, "Archery range in the building type table");
	if (bArchery)
	{
		T.AssertTrue(HOLConstructionRead::IsBuildingTypeBuildable(Archery), "Archery range: built by Peasants");
		T.AssertEquals(5, Archery.BuildGridSlot, "Archery range: S of the Peasant's grid (slot 5, military row)");
		T.AssertTrue(ConNearV(Archery.Footprint, FVector2D(3.0 * Grid, 3.0 * Grid)), "Archery range: 3 x 3 tiles (12 m)");
		T.AssertEquals(0, Archery.Age, "Archery range: age I");
	}

	// SC-Q1 = B (D602): the Sanctuary of the Dawn, technological, age I, X of page I, 250 wood, 3 x 3 tiles, 30 s, 1 500 health, no production.
	FHOLBuildingTypeRow Sanctuary;
	bool bSanctuary = HOLConstructionRead::FindBuildingType(n"Sanctuary", Sanctuary);
	T.AssertTrue(bSanctuary, "Sanctuary in the building type table");
	if (bSanctuary)
	{
		T.AssertTrue(HOLConstructionRead::IsBuildingTypeBuildable(Sanctuary), "Sanctuary: built by Peasants");
		T.AssertEquals(9, Sanctuary.BuildGridSlot, "Sanctuary: X of the Peasant's grid (slot 9, after the Farm)");
		T.AssertEquals(0, Sanctuary.Age, "Sanctuary: age I");
		T.AssertEquals(2, HOLConstructionRead::GetBuildingTypeCategory(Sanctuary), "Sanctuary: technological");
		T.AssertTrue(ConNearV(Sanctuary.Footprint, FVector2D(3.0 * Grid, 3.0 * Grid)), "Sanctuary: 3 x 3 tiles (12 m)");
		T.AssertTrue(ConResEq(Sanctuary.Cost, ConRes(0, 250, 0, 0)), "Sanctuary: 250 wood");
		T.AssertTrue(ConNear(Sanctuary.BuildTime, 30.0), "Sanctuary: 30 s for one builder");
		T.AssertEquals(1500, Sanctuary.MaxHealth, "Sanctuary: 1 500 health");
		T.AssertEquals(0, Sanctuary.PopulationProvided, "Sanctuary: no population");
		T.AssertEquals(0, Sanctuary.Produces.Num(), "Sanctuary: produces nothing");
		T.AssertEquals(0, HOLConstructionRead::GetBuildingTypeAccepts(Sanctuary).Num(), "Sanctuary: takes no resource");
		T.AssertFalse(Sanctuary.Walkable, "Sanctuary: not walkable");
	}

	// Q-chapelle-moine = A: the Chapel of the Dawn (age II, page II), military, S of page II, 200 wood, 3 x 3 tiles, 30 s, 1 500 health; it produces the Monk.
	FHOLBuildingTypeRow Chapel;
	bool bChapel = HOLConstructionRead::FindBuildingType(n"Chapel", Chapel);
	T.AssertTrue(bChapel, "Chapel in the building type table");
	if (bChapel)
	{
		T.AssertTrue(HOLConstructionRead::IsBuildingTypeBuildable(Chapel), "Chapel: built by Peasants");
		T.AssertEquals(5, Chapel.BuildGridSlot, "Chapel: S of the Peasant's grid (slot 5, military row)");
		T.AssertEquals(1, Chapel.Age, "Chapel: age II (page II)");
		T.AssertEquals(1, HOLConstructionRead::GetBuildingTypeCategory(Chapel), "Chapel: military (it forms the Monk)");
		T.AssertTrue(ConNearV(Chapel.Footprint, FVector2D(3.0 * Grid, 3.0 * Grid)), "Chapel: 3 x 3 tiles (12 m)");
		T.AssertTrue(ConResEq(Chapel.Cost, ConRes(0, 200, 0, 0)), "Chapel: 200 wood");
		T.AssertTrue(ConNear(Chapel.BuildTime, 30.0), "Chapel: 30 s for one builder");
		T.AssertEquals(1500, Chapel.MaxHealth, "Chapel: 1 500 health");
		T.AssertEquals(0, Chapel.PopulationProvided, "Chapel: no population");
		T.AssertTrue(Chapel.Produces.Num() == 1 && Chapel.Produces[0] == n"Monk", "Chapel: produces the Monk");
		T.AssertEquals(0, HOLConstructionRead::GetBuildingTypeAccepts(Chapel).Num(), "Chapel: takes no resource");
		T.AssertFalse(Chapel.Walkable, "Chapel: not walkable");
	}

	// M3 (GDD 5.5, 11.1, D569): the Market, economic, age II, A of page II; size, cost and health provisional: 3 x 3 tiles, wood only, a build time.
	FHOLBuildingTypeRow Market;
	bool bMarket = HOLConstructionRead::FindBuildingType(n"Market", Market);
	T.AssertTrue(bMarket, "Market in the building type table");
	if (bMarket)
	{
		T.AssertTrue(HOLConstructionRead::IsBuildingTypeBuildable(Market), "Market: built by Peasants");
		T.AssertEquals(0, Market.BuildGridSlot, "Market: A of the Peasant's grid (slot 0, economic row)");
		T.AssertEquals(1, Market.Age, "Market: age II");
		T.AssertEquals(0, HOLConstructionRead::GetBuildingTypeCategory(Market), "Market: economic");
		T.AssertTrue(ConNearV(Market.Footprint, FVector2D(3.0 * Grid, 3.0 * Grid)), "Market: 3 x 3 tiles");
		T.AssertTrue(Market.Cost.Food == 0 && Market.Cost.Stone == 0 && Market.Cost.Gold == 0 && Market.Cost.Wood > 0, "Market: costs wood only");
		T.AssertTrue(Market.BuildTime > 0.0f, "Market: a build time");
		T.AssertEquals(0, Market.PopulationProvided, "Market: no population");
		T.AssertEquals(0, Market.Produces.Num(), "Market: produces nothing");
		T.AssertEquals(0, HOLConstructionRead::GetBuildingTypeAccepts(Market).Num(), "Market: takes no resource (it is not a deposit)");
		T.AssertFalse(Market.Walkable, "Market: not walkable");
		T.AssertTrue(Market.Source == n"None", "Market: no source of its own");
		T.AssertTrue(Market.SightRadius >= 2000.0f, "Market: sees at least as far as a house");
	}

	// IA1.2 (F4): every building so far blocks the way and has no source of its own.
	TArray<FName> Blocking;
	Blocking.Add(n"MainCentre");
	Blocking.Add(n"House");
	Blocking.Add(n"LumberCamp");
	Blocking.Add(n"MiningCamp");
	Blocking.Add(n"Mill");
	Blocking.Add(n"Barracks");
	Blocking.Add(n"ArcheryRange");
	for (int Index = 0; Index < Blocking.Num(); ++Index)
	{
		FHOLBuildingTypeRow Row;
		FString Label = Blocking[Index].ToString();
		bool bFoundRow = HOLConstructionRead::FindBuildingType(Blocking[Index], Row);
		T.AssertTrue(bFoundRow, Label + " in the building type table");
		if (bFoundRow)
		{
			T.AssertFalse(Row.Walkable, Label + ": not walkable");
			T.AssertTrue(Row.Source == n"None", Label + ": no source of its own");
		}
	}

	// IA1.2 (F4): the Farm, 2 x 2 like the Mill, 75 wood, 7.5 s, W of the Peasant's grid (slot 8), walked over, born with its FarmField (a linked food source).
	FHOLBuildingTypeRow Farm;
	bool bFarm = HOLConstructionRead::FindBuildingType(n"Farm", Farm);
	T.AssertTrue(bFarm, "Farm in the building type table");
	if (bFarm)
	{
		T.AssertTrue(HOLConstructionRead::IsBuildingTypeBuildable(Farm), "Farm: built by Peasants");
		T.AssertEquals(8, Farm.BuildGridSlot, "Farm: W of the Peasant's grid (slot 8, IA1.2-Q1)");
		T.AssertEquals(0, Farm.Age, "Farm: age I");
		T.AssertEquals(0, HOLConstructionRead::GetBuildingTypeCategory(Farm), "Farm: economic");
		T.AssertTrue(ConNearV(Farm.Footprint, FVector2D(2.0 * Grid, 2.0 * Grid)), "Farm: 2 x 2 tiles (8 m)");
		T.AssertTrue(ConResEq(Farm.Cost, ConRes(0, 75, 0, 0)), "Farm: 75 wood");
		T.AssertTrue(ConNear(Farm.BuildTime, 7.5), "Farm: 7.5 s for one builder");
		T.AssertEquals(0, Farm.PopulationProvided, "Farm: no population");
		T.AssertEquals(0, HOLConstructionRead::GetBuildingTypeAccepts(Farm).Num(), "Farm: takes no resource (no deposit)");
		T.AssertEquals(0, Farm.Produces.Num(), "Farm: produces nothing");
		T.AssertTrue(Farm.Walkable, "Farm: walkable");
		T.AssertTrue(Farm.Source == n"FarmField", "Farm: its source is a FarmField");
		bool bLinked = false;
		int FieldResource = -1;
		bool bField = HOLConstructionRead::FindSourceTypeInfo(Farm.Source, bLinked, FieldResource);
		T.AssertTrue(bField, "Farm: its source in the source type table");
		if (bField)
		{
			T.AssertTrue(bLinked, "Farm: its source is linked");
			T.AssertEquals(0, FieldResource, "Farm: its source gives food");
		}
	}
}


// K1a (MIG-CPP step 5, GALADRIEL-1119v): the order of the gates of UHOLConstructionSubsystem::Place and ::Cancel, case by case, on the script board
// (Script/Buildings/HOLConstructionBoard.as, namespace HOLConstructionBoard). The board calls the rules already exposed (IsAgeOpen, TryPay, CanCancel,
// GetCancelRefund); only the ORDER and the verdict are new. Place gates, in the C++ order (Place 426-462): buildable, faction, age, placement, payment.
// RED until HOLConstructionBoard.as exists. To SEE the red: in HOLConstructionBoard.as, swap the faction and age gates of PlaceVerdict (the double refusal
// case then gives AgeNotReached) or turn the payment test of TryPay into a strict one (50 wood for 50 then gives NotEnoughResources).
int ConPlace(bool bBuildable, bool bFaction, int Age, int Tier, int Refusal, FHOLResources Stock, FHOLResources Cost)
{
	return int(HOLConstructionBoard::PlaceVerdict(bBuildable, bFaction, Age, Tier, Refusal, Stock, Cost));
}

void Test_Construction_PlaceVerdict(FUnitTest& T)
{
	FHOLResources Cost = ConRes(0, 50, 0, 0);
	FHOLResources Enough = ConRes(0, 50, 0, 0);
	int Ok = int(EHOLPlaceVerdict::Ok);

	T.AssertEquals(Ok, ConPlace(true, true, 1, 1, 0, Enough, Cost), "all gates open, exactly the cost in stock: placed (50 for 50)");
	T.AssertEquals(Ok, ConPlace(true, true, 2, 2, 0, ConRes(0, 0, 0, 0), ConRes(0, 0, 0, 0)), "a free building with an empty stock: placed, age = tier");
	T.AssertEquals(int(EHOLPlaceVerdict::NotEnoughResources), ConPlace(true, true, 1, 1, 0, ConRes(0, 49, 0, 0), Cost), "49 wood for 50: not enough");
	T.AssertEquals(int(EHOLPlaceVerdict::NotEnoughResources), ConPlace(true, true, 1, 1, 0, ConRes(0, 50, 0, 0), ConRes(5, 50, 0, 0)), "wood enough but food short: not enough");
	T.AssertEquals(int(EHOLPlaceVerdict::AgeNotReached), ConPlace(true, true, 3, 2, 0, Enough, Cost), "age 3 with tier 2: age not reached");

	// The order of the gates: each case fails the gate named AND every gate after it.
	T.AssertEquals(int(EHOLPlaceVerdict::NotBuildable), ConPlace(false, false, 3, 1, 3, ConRes(0, 0, 0, 0), Cost), "not buildable first, whatever the rest");
	T.AssertEquals(int(EHOLPlaceVerdict::NotOfFaction), ConPlace(true, false, 3, 1, 3, ConRes(0, 0, 0, 0), Cost), "faction before age: both shut gives the faction");
	T.AssertEquals(int(EHOLPlaceVerdict::AgeNotReached), ConPlace(true, true, 3, 1, 3, ConRes(0, 0, 0, 0), Cost), "age before the placement refusal");
	T.AssertEquals(int(EHOLPlaceVerdict::PlacementRefused), ConPlace(true, true, 1, 1, 3, ConRes(0, 0, 0, 0), Cost), "placement refusal before the payment: refused AND poor gives the refusal");
	T.AssertEquals(int(EHOLPlaceVerdict::PlacementRefused), ConPlace(true, true, 1, 1, 5, Enough, Cost), "a navmesh refusal (5) with the stock: refused, nothing paid");

	// The stock after a placement: the cost taken, nothing taken by a refusal (the C++ writes it only once the foundation is born).
	T.AssertTrue(ConResEq(HOLConstructionBoard::StockAfterPlace(Enough, Cost), ConRes(0, 0, 0, 0)), "50 for 50: 0 wood left");
	T.AssertTrue(ConResEq(HOLConstructionBoard::StockAfterPlace(ConRes(200, 120, 7, 3), Cost), ConRes(200, 70, 7, 3)), "only the wood is taken");
	T.AssertTrue(ConResEq(HOLConstructionBoard::StockAfterPlace(ConRes(0, 49, 0, 0), Cost), ConRes(0, 49, 0, 0)), "a refusal takes nothing");
}

void Test_Construction_CancelVerdict(FUnitTest& T)
{
	FHOLResources Cost = ConRes(0, 50, 0, 0);
	int Allowed = int(EHOLCancelVerdict::Allowed);
	int Ignored = int(EHOLCancelVerdict::Ignored);

	T.AssertEquals(Allowed, int(HOLConstructionBoard::CancelVerdict(true, 1, 1, 0)), "the owner cancels its foundation");
	T.AssertEquals(Allowed, int(HOLConstructionBoard::CancelVerdict(true, 1, 1, 1)), "the owner cancels a building under construction");
	T.AssertEquals(Ignored, int(HOLConstructionBoard::CancelVerdict(true, 1, 1, 2)), "a finished building is not cancelled");
	T.AssertEquals(Ignored, int(HOLConstructionBoard::CancelVerdict(true, 0, 1, 0)), "another player's foundation is not cancelled");
	T.AssertEquals(Ignored, int(HOLConstructionBoard::CancelVerdict(false, 1, 1, 0)), "no building (a client named a dead or unknown one): ignored");

	T.AssertTrue(ConResEq(HOLConstructionBoard::CancelRefund(true, Cost, 0, 0, 1000), Cost), "a foundation: all back (D311)");
	T.AssertTrue(ConResEq(HOLConstructionBoard::CancelRefund(true, Cost, 1, 500, 1000), ConRes(0, 25, 0, 0)), "half built: half back");
	T.AssertTrue(ConResEq(HOLConstructionBoard::CancelRefund(true, Cost, 1, 1, 1000), ConRes(0, 49, 0, 0)), "D345: after a first work 49 of 50 wood come back, not all");
	T.AssertTrue(ConResEq(HOLConstructionBoard::CancelRefund(true, Cost, 2, 0, 1000), ConRes(0, 0, 0, 0)), "finished: nothing back");
	T.AssertTrue(ConResEq(HOLConstructionBoard::CancelRefund(false, Cost, 0, 0, 1000), ConRes(0, 0, 0, 0)), "no type row: nothing back (the C++ FHOLResources())");
}

// K1b: the refund of a production queue and the destruction of a building (HOLConstructionBoard::QueueRefundVerdict, QueueRefund, DestroyVerdict, RubbleVerdict;
// the site's share is CancelRefund, tested above). Costs are given in queue order, one per item.
void Test_Construction_RefundDestroyVerdict(FUnitTest& T)
{
	int RefundQueue = int(EHOLQueueRefundVerdict::RefundQueue);
	int NothingToRefund = int(EHOLQueueRefundVerdict::NothingToRefund);
	T.AssertEquals(RefundQueue, int(HOLConstructionBoard::QueueRefundVerdict(true, true, 3)), "a server building with a queue: refunded");
	T.AssertEquals(NothingToRefund, int(HOLConstructionBoard::QueueRefundVerdict(true, true, 0)), "an empty queue: nothing to refund");
	T.AssertEquals(NothingToRefund, int(HOLConstructionBoard::QueueRefundVerdict(true, false, 3)), "a client (no authority): nothing");
	T.AssertEquals(NothingToRefund, int(HOLConstructionBoard::QueueRefundVerdict(false, true, 3)), "no world: nothing");

	TArray<FHOLResources> Costs;
	T.AssertTrue(ConResEq(HOLConstructionBoard::QueueRefund(Costs), ConRes(0, 0, 0, 0)), "no item: nothing back");
	Costs.Add(ConRes(50, 0, 0, 0));
	Costs.Add(ConRes(50, 0, 0, 20));
	Costs.Add(ConRes(0, 10, 0, 0));
	T.AssertTrue(ConResEq(HOLConstructionBoard::QueueRefund(Costs), ConRes(100, 10, 0, 20)), "every item back in full, the head too: the sum of the costs");

	int DestroyIt = int(EHOLDestroyVerdict::DestroyIt);
	int LeaveIt = int(EHOLDestroyVerdict::LeaveIt);
	T.AssertEquals(DestroyIt, int(HOLConstructionBoard::DestroyVerdict(true, true, true)), "a valid server building is destroyed");
	T.AssertEquals(LeaveIt, int(HOLConstructionBoard::DestroyVerdict(false, true, true)), "no world: left");
	T.AssertEquals(LeaveIt, int(HOLConstructionBoard::DestroyVerdict(true, false, true)), "a client: left");
	T.AssertEquals(LeaveIt, int(HOLConstructionBoard::DestroyVerdict(true, true, false)), "an invalid actor: left");

	int NoRubble = int(EHOLRubbleVerdict::NoRubble);
	int LeaveRubble = int(EHOLRubbleVerdict::LeaveRubble);
	T.AssertEquals(LeaveRubble, int(HOLConstructionBoard::RubbleVerdict(false, true)), "a finished or rising building leaves rubble (D345.2a)");
	T.AssertEquals(NoRubble, int(HOLConstructionBoard::RubbleVerdict(true, true)), "a bare foundation: none, the others never saw it");
	T.AssertEquals(NoRubble, int(HOLConstructionBoard::RubbleVerdict(false, false)), "no type row: none");

	FHOLResources Cost = ConRes(0, 50, 0, 0);
	T.AssertTrue(ConResEq(HOLConstructionBoard::CancelRefund(true, Cost, 0, 0, 1000), Cost), "destroyed foundation: the site's share is all of it");
	T.AssertTrue(ConResEq(HOLConstructionBoard::CancelRefund(true, Cost, 2, 0, 1000), ConRes(0, 0, 0, 0)), "destroyed finished building: no site share");
}

// MainCentre: 150 s for one builder (D816 Q-nomad-forum-duree = B; HALDIR-1121a, port of the C++ test planned on the Barracks 20 s model of Test_Construction_Builders).
// The real chain, no value typed by hand: the table row's BuildTime -> GetBuildSteps -> AddWork one step at a time with one builder, then with 4.
// Effect asserted: the forum foundation stays in work (not finished, health below the full) until step N, finishes exactly at step N (N = 150 s / 0.05 s);
// four builders finish in N / 4^0.63 steps (the table's diminishing returns).
void Test_Construction_MainCentreOneBuilder(FUnitTest& T)
{
	FHOLBuildingTypeRow Forum;
	bool bForum = HOLConstructionRead::FindBuildingType(n"MainCentre", Forum);
	T.AssertTrue(bForum, "MainCentre in the building type table");
	if (!bForum)
	{
		return;
	}
	int BuildSteps = HOLConstructionRead::GetBuildSteps(Forum.BuildTime, 0.05f);
	T.AssertEquals(3000, BuildSteps, "MainCentre: 150 s = 3000 steps of 0.05 s for one builder (the table's BuildTime, not 0)");
	T.AssertEquals(BuildSteps, HOLConstructionRead::GetStepsToFinish(BuildSteps, 1, ConTestExponent), "one builder: the table's time");

	int64 Total = HOLConstructionRead::GetTotalWork(BuildSteps);
	int64 Work = 0;
	int Steps = 0;
	int Finishes = 0;
	bool bFinishedEarly = false;
	while (Steps < BuildSteps + 10 && Work < Total)
	{
		if (HOLConstructionRead::AddWork(Work, Total, 1, ConTestExponent))
			Finishes += 1;
		++Steps;
		if (Steps == BuildSteps - 1 && (Work >= Total || HOLConstructionRead::GetHealth(Forum.MaxHealth, Work, Total) >= Forum.MaxHealth))
			bFinishedEarly = true;
	}
	T.AssertFalse(bFinishedEarly, "one builder: the foundation is still in work at step N - 1 (health below the full)");
	T.AssertEquals(BuildSteps, Steps, "one builder step by step: finished at step N (150 s), not before");
	T.AssertEquals(1, Finishes, "finished once");
	T.AssertEquals(Forum.MaxHealth, HOLConstructionRead::GetHealth(Forum.MaxHealth, Work, Total), "finished: the full health");

	int Four = HOLConstructionRead::GetStepsToFinish(BuildSteps, 4, ConTestExponent);
	double Expected = double(BuildSteps) / Math::Pow(4.0, double(ConTestExponent));
	T.AssertTrue(Math::Abs(double(Four) - Expected) < Expected * 0.02, "4 builders: N / 4^0.63 steps (about 1 / 2.4 of 150 s)");
	T.AssertTrue(Four < BuildSteps && Four > BuildSteps / 4, "4 builders: faster than one, slower than 4x");
}

// D898 G4: a wall piece (the 400 x 400 row footprint, SubDivision 4) snaps on the 1 m sub-grid and is 1 m wide; a 600 x 600 house at SubDivision 1 stays on the 4 m grid.
void Test_Construction_WallPieceSubGrid(FUnitTest& T)
{
	FHOLConstructionBox Piece = HOLConstructionRead::SnapWallPiece(FVector2D(130.0, 70.0), FVector2D(400.0, 400.0), 400.0, 4);
	T.AssertTrue(ConNearV(Piece.Min, FVector2D(100.0, 0.0)), "wall piece at (130, 70): lower corner on the 1 m grid, (100, 0)");
	T.AssertTrue(ConNearV(ConSize(Piece), FVector2D(100.0, 100.0)), "wall piece: 1 m x 1 m");
	FHOLConstructionBox House = HOLConstructionRead::SnapWallPiece(FVector2D(1234.0, -60.0), FVector2D(600.0, 600.0), 400.0, 1);
	T.AssertTrue(ConNearV(House.Min, FVector2D(800.0, -400.0)), "SubDivision 1: the 4 m grid of today");
	T.AssertTrue(ConNearV(ConSize(House), FVector2D(600.0, 600.0)), "SubDivision 1: size kept");
}