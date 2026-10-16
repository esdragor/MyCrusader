// Copyright Epic Games, Inc. All Rights Reserved.

#include "Buildings/HOLBuildingTypes.h"
#include "Buildings/HOLConstructionRules.h"
#include "Buildings/HOLProductionRules.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

// J6a.2 checks (D314): the pure rules of construction (D311, D312) and the values of the building type table (D310, D313).
namespace UE::HeroesOfLiberty::Tests::Construction
{
	constexpr EAutomationTestFlags Flags = EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter;

	/** The diminishing returns of the settings' default (ln 2 / ln 3). */
	constexpr float Exponent = 0.63f;

	/** A tile (UHOLBuildingSettings::TileSize, D311): 3 m. */
	constexpr double Tile = 300.0;

	/** A house, 2 x 2 tiles (6 m), placed with its middle at (X, Y) m. */
	FBox2D House(const double X, const double Y)
	{
		return HOLConstruction::MakeFootprint(FVector2D(X, Y) * 100.0, FVector2D(600.0, 600.0));
	}

	/** A query on an empty 400 x 400 m map, the footprint a house at (X, Y) m. */
	FHOLPlacementQuery MakeQuery(const double X, const double Y)
	{
		FHOLPlacementQuery Query;
		Query.Footprint = House(X, Y);
		Query.MapArea = FBox2D(FVector2D(-20000.0, -20000.0), FVector2D(20000.0, 20000.0));
		Query.AntiRushRadius = 4000.0;
		return Query;
	}

	/** Both corners of the box on the grid lines. */
	bool IsOnGrid(const FBox2D& Box, const double Grid)
	{
		const auto OnLine = [Grid](const double Value) { return FMath::IsNearlyZero(FMath::Fmod(FMath::Abs(Value) + 0.01, Grid), 0.02); };
		return OnLine(Box.Min.X) && OnLine(Box.Min.Y) && OnLine(Box.Max.X) && OnLine(Box.Max.Y);
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLConstructionGridTest, "HeroesOfLiberty.Construction.Grid", UE::HeroesOfLiberty::Tests::Construction::Flags)
bool FHOLConstructionGridTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::Construction;
	const FVector2D HouseSize(600.0, 600.0);

	// Even number of tiles (a house, 2 x 2): the edges on the grid lines, the middle on a grid point.
	const FBox2D Even = HOLConstruction::SnapToGrid(FVector2D(1234.0, -60.0), HouseSize, Tile);
	TestTrue(TEXT("2 x 2 tiles: corner on the grid"), Even.Min.Equals(FVector2D(900.0, -300.0)));
	TestTrue(TEXT("2 x 2 tiles: size kept"), Even.GetSize().Equals(HouseSize));
	TestTrue(TEXT("2 x 2 tiles: middle on a grid point"), Even.GetCenter().Equals(FVector2D(1200.0, 0.0)));

	// Odd number of tiles (3 x 3, no such type yet): the middle in the middle of a tile.
	const FBox2D Odd = HOLConstruction::SnapToGrid(FVector2D(1234.0, -60.0), FVector2D(900.0, 900.0), Tile);
	TestTrue(TEXT("3 x 3 tiles: edges on the grid lines"), IsOnGrid(Odd, Tile));
	TestTrue(TEXT("3 x 3 tiles: middle in the middle of a tile"), Odd.GetCenter().Equals(FVector2D(1350.0, -150.0)));

	// Two neighbours snapped side by side touch exactly: no gap, no overlap.
	const FBox2D Left = HOLConstruction::SnapToGrid(FVector2D(0.0, 0.0), HouseSize, Tile);
	const FBox2D Right = HOLConstruction::SnapToGrid(FVector2D(620.0, 20.0), HouseSize, Tile);
	TestEqual(TEXT("side by side: the edges meet"), Right.Min.X, Left.Max.X);
	TestFalse(TEXT("touching is no overlap"), HOLConstruction::Overlaps(Left, Right));

	// The ghost moves tile by tile: still within half a tile, a whole tile further past it.
	const FBox2D Moved = HOLConstruction::SnapToGrid(FVector2D(1349.0, -149.0), HouseSize, Tile);
	TestTrue(TEXT("still within half a tile"), Moved.Min.Equals(Even.Min) && Moved.Max.Equals(Even.Max));
	const FBox2D Next = HOLConstruction::SnapToGrid(FVector2D(1460.0, -60.0), HouseSize, Tile);
	TestTrue(TEXT("one tile further"), Next.Min.Equals(Even.Min + FVector2D(Tile, 0.0)));

	// The main centre (4 x 4 tiles) of a diagonal start of Plain (150 m from the middle at 135 degrees) moved onto the
	// grid (AHOLStartSlot::GetMainCentreLocation): at most half a tile each way, and a house beside it touches it.
	const FVector2D ForumSize(1200.0, 1200.0);
	const FVector2D Diagonal = FVector2D(-1.0, 1.0) * 15000.0 * UE_DOUBLE_INV_SQRT_2;
	const FBox2D Forum = HOLConstruction::SnapToGrid(Diagonal, ForumSize, Tile);
	TestTrue(TEXT("diagonal start: the centre's footprint on the grid"), IsOnGrid(Forum, Tile));
	TestTrue(TEXT("diagonal start: the centre at (-105, 105) m"), Forum.GetCenter().Equals(FVector2D(-10500.0, 10500.0)));
	TestTrue(TEXT("diagonal start: moved half a tile at most each way"),
		FMath::Abs(Forum.GetCenter().X - Diagonal.X) <= Tile * 0.5 && FMath::Abs(Forum.GetCenter().Y - Diagonal.Y) <= Tile * 0.5);
	const FBox2D Beside = HOLConstruction::SnapToGrid(FVector2D(Forum.Max.X + Tile + 40.0, Forum.GetCenter().Y + 100.0), HouseSize, Tile);
	TestEqual(TEXT("a house beside the centre: the edges meet"), Beside.Min.X, Forum.Max.X);
	TestFalse(TEXT("a house beside the centre: no overlap"), HOLConstruction::Overlaps(Beside, Forum));
	TestTrue(TEXT("a start on the grid already stays"), HOLConstruction::SnapToGrid(FVector2D(-15000.0, 0.0), ForumSize, Tile).GetCenter().Equals(FVector2D(-15000.0, 0.0)));

	// Its yaw: the nearest quarter turn, the footprint along the grid lines.
	TestEqual(TEXT("yaw 44 -> 0"), HOLConstruction::SnapYaw(44.f), 0.f);
	TestEqual(TEXT("yaw -91 -> -90"), HOLConstruction::SnapYaw(-91.f), -90.f);
	TestEqual(TEXT("yaw 300 -> -90"), HOLConstruction::SnapYaw(300.f), -90.f);
	TestEqual(TEXT("yaw 180 -> 180"), FMath::Abs(HOLConstruction::SnapYaw(180.f)), 180.f);

	const FBox2D Free = HOLConstruction::SnapToGrid(FVector2D(1234.0, -60.0), HouseSize, 0.0);
	TestTrue(TEXT("no grid: centred on the point"), Free.GetCenter().Equals(FVector2D(1234.0, -60.0)));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLConstructionPlacementTest, "HeroesOfLiberty.Construction.Placement", UE::HeroesOfLiberty::Tests::Construction::Flags)
bool FHOLConstructionPlacementTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::Construction;
	const auto Check = [](const FHOLPlacementQuery& Query) { return int32(HOLConstruction::CheckPlacement(Query)); };

	FHOLPlacementQuery Query = MakeQuery(0.0, 0.0);
	TestEqual(TEXT("open ground: placed"), Check(Query), int32(EHOLPlacementRefusal::None));

	// Buildings: touching fine, overlapping by a tile refused.
	const TArray<FBox2D> Buildings = { House(6.0, 0.0), House(-21.0, -21.0) };
	Query.Buildings = Buildings;
	TestEqual(TEXT("touching a house: placed"), Check(Query), int32(EHOLPlacementRefusal::None));
	Query.Footprint = House(3.0, 0.0);
	TestEqual(TEXT("overlapping a house: refused"), Check(Query), int32(EHOLPlacementRefusal::Building));
	Query.Footprint = House(0.0, 0.0);

	// Sources: a tree (60 cm) beside the footprint fine, its body over the edge refused; a vein (3 m) in the middle refused.
	TArray<FHOLPlacementCircle> Sources = { { FVector2D(0.0, 370.0), 60.0 } };
	Query.Sources = Sources;
	TestEqual(TEXT("tree just clear of the edge: placed"), Check(Query), int32(EHOLPlacementRefusal::None));
	Sources[0].Centre.Y = 330.0;
	TestEqual(TEXT("tree over the edge: refused"), Check(Query), int32(EHOLPlacementRefusal::Source));
	Sources[0] = { FVector2D(0.0, 0.0), 300.0 };
	TestEqual(TEXT("vein under the footprint: refused"), Check(Query), int32(EHOLPlacementRefusal::Source));
	Sources[0] = { FVector2D(300.0 + 300.0, 300.0 + 300.0), 300.0 };
	TestEqual(TEXT("vein off the corner (its box would touch, its disc does not): placed"), Check(Query), int32(EHOLPlacementRefusal::None));
	Sources.Reset();
	Query.Sources = Sources;

	// The map's bounds: the edge on the bound fine, a tile past it refused.
	Query.Footprint = HOLConstruction::MakeFootprint(FVector2D(19700.0, 0.0), FVector2D(600.0, 600.0));
	TestEqual(TEXT("against the map's edge: placed"), Check(Query), int32(EHOLPlacementRefusal::None));
	Query.Footprint = HOLConstruction::MakeFootprint(FVector2D(20000.0, 0.0), FVector2D(600.0, 600.0));
	TestEqual(TEXT("past the map's edge: refused"), Check(Query), int32(EHOLPlacementRefusal::OutOfMap));
	Query.MapArea = FBox2D(ForceInit);
	TestEqual(TEXT("no map bounds: no limit"), Check(Query), int32(EHOLPlacementRefusal::None));

	// Anti-rush (D22): 40 m from an enemy main centre's middle to the nearest point of the footprint.
	const TArray<FVector2D> Enemies = { FVector2D(0.0, 0.0) };
	Query.EnemyMainCentres = Enemies;
	Query.Footprint = HOLConstruction::MakeFootprint(FVector2D(4000.0 + 300.0, 0.0), FVector2D(600.0, 600.0));
	TestEqual(TEXT("at the anti-rush radius: placed"), Check(Query), int32(EHOLPlacementRefusal::None));
	Query.Footprint = HOLConstruction::MakeFootprint(FVector2D(3900.0 + 300.0, 0.0), FVector2D(600.0, 600.0));
	TestEqual(TEXT("1 m inside the anti-rush radius: refused"), Check(Query), int32(EHOLPlacementRefusal::EnemyCentre));
	Query.EnemyMainCentres = TArrayView<const FVector2D>();
	TestEqual(TEXT("the same place near one's own centre: placed"), Check(Query), int32(EHOLPlacementRefusal::None));

	// The navmesh, answered by the caller.
	Query.bOnNavMesh = false;
	TestEqual(TEXT("off the navmesh: refused"), Check(Query), int32(EHOLPlacementRefusal::NavMesh));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLConstructionPaymentTest, "HeroesOfLiberty.Construction.Payment", UE::HeroesOfLiberty::Tests::Construction::Flags)
bool FHOLConstructionPaymentTest::RunTest(const FString& Parameters)
{
	const FHOLResources Cost(0, 50, 0, 0);
	FHOLResources Stock(200, 120, 0, 0);
	TestTrue(TEXT("first house paid"), HOLConstruction::TryPay(Stock, Cost));
	TestTrue(TEXT("second house paid"), HOLConstruction::TryPay(Stock, Cost));
	TestTrue(TEXT("cost taken at placement"), Stock == FHOLResources(200, 20, 0, 0));
	TestFalse(TEXT("third refused: 20 wood left"), HOLConstruction::TryPay(Stock, Cost));
	TestTrue(TEXT("nothing taken by a refusal"), Stock == FHOLResources(200, 20, 0, 0));

	TestTrue(TEXT("cancelled before the first work: all back"), HOLConstruction::GetRefund(Cost, false) == Cost);
	TestTrue(TEXT("cancelled after: half back"), HOLConstruction::GetRefund(Cost, true) == FHOLResources(0, 25, 0, 0));
	TestTrue(TEXT("half rounded down, each resource"), HOLConstruction::GetRefund(FHOLResources(1, 75, 3, 0), true) == FHOLResources(0, 37, 1, 0));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLConstructionBuildersTest, "HeroesOfLiberty.Construction.Builders", UE::HeroesOfLiberty::Tests::Construction::Flags)
bool FHOLConstructionBuildersTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::Construction;
	// A 20 s house at 20 steps per second.
	const int32 BuildSteps = HOLConstruction::GetBuildSteps(20.f, 0.05f);
	TestEqual(TEXT("20 s = 400 steps for one builder"), BuildSteps, 400);
	TestEqual(TEXT("one builder: the table's time"), HOLConstruction::GetStepsToFinish(BuildSteps, 1, Exponent), 400);

	// Diminishing returns (D312): each builder more speeds it up, less and less; 3 about twice as fast as one.
	const int32 Three = HOLConstruction::GetStepsToFinish(BuildSteps, 3, Exponent);
	TestTrue(TEXT("3 builders ~2x as fast"), FMath::Abs(400.0 / Three - 2.0) < 0.02);
	int32 Previous = 0;
	int32 PreviousGain = TNumericLimits<int32>::Max();
	for (int32 Builders = 1; Builders <= 10; ++Builders)
	{
		const int32 Rate = HOLConstruction::GetWorkPerStep(Builders, Exponent);
		TestTrue(FString::Printf(TEXT("%d builders faster than %d"), Builders, Builders - 1), Rate > Previous);
		TestTrue(FString::Printf(TEXT("the builder %d adds less than the one before"), Builders), Rate - Previous < PreviousGain);
		PreviousGain = Rate - Previous;
		Previous = Rate;
	}
	TestTrue(TEXT("10 builders: far from 10x"), HOLConstruction::GetWorkPerStep(10, Exponent) < 5 * HOLConstruction::WorkPerBuilder);
	TestEqual(TEXT("exponent 1: each as useful as the first"), HOLConstruction::GetWorkPerStep(4, 1.f), 4 * HOLConstruction::WorkPerBuilder);
	TestEqual(TEXT("no builder, no work"), HOLConstruction::GetWorkPerStep(0, Exponent), 0);

	// Step by step: nothing without builders, finished exactly once, health rising to the full.
	const int64 Total = HOLConstruction::GetTotalWork(BuildSteps);
	int64 Work = 0;
	TestEqual(TEXT("a bare foundation: 1 HP"), HOLConstruction::GetHealth(600, Work, Total), 1);
	TestFalse(TEXT("no builder: no progress"), HOLConstruction::AddWork(Work, Total, 0, Exponent));
	TestEqual(TEXT("still nothing done"), Work, int64(0));
	int32 Steps = 0;
	int32 Finishes = 0;
	int32 Health = HOLConstruction::GetHealth(600, Work, Total);
	bool bHealthRising = true;
	while (Steps < 1000 && Work < Total)
	{
		const int64 Before = Work;
		Finishes += HOLConstruction::AddWork(Work, Total, 3, Exponent) ? 1 : 0;
		const int32 Gain = HOLConstruction::GetHealth(600, Work, Total) - HOLConstruction::GetHealth(600, Before, Total);
		bHealthRising &= Gain >= 0;
		Health += Gain;
		++Steps;
	}
	TestEqual(TEXT("3 builders step by step: the steps foreseen"), Steps, Three);
	TestEqual(TEXT("finished once"), Finishes, 1);
	TestFalse(TEXT("a finished building takes no more work"), HOLConstruction::AddWork(Work, Total, 3, Exponent));
	TestEqual(TEXT("progress capped at 1"), HOLConstruction::GetProgress(Work, Total), 1.f);
	TestTrue(TEXT("health never goes down while building"), bHealthRising);
	TestEqual(TEXT("unhurt: exactly the full health at the end"), Health, 600);
	TestEqual(TEXT("half done: half the health"), HOLConstruction::GetHealth(600, Total / 2, Total), 300);
	TestEqual(TEXT("half done: half the progress"), HOLConstruction::GetProgress(Total / 2, Total), 0.5f);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLConstructionPopulationTest, "HeroesOfLiberty.Construction.Population", UE::HeroesOfLiberty::Tests::Construction::Flags)
bool FHOLConstructionPopulationTest::RunTest(const FString& Parameters)
{
	// The centre's 10, two finished houses, one still being built.
	int32 Provided = HOLConstruction::GetPopulationProvided(10, true);
	Provided += HOLConstruction::GetPopulationProvided(10, true) * 2;
	Provided += HOLConstruction::GetPopulationProvided(10, false);
	TestEqual(TEXT("an unfinished house gives nothing"), HOLProduction::GetPopulationCap(Provided), 30);

	// 20 houses and the centre: 210 places given, capped at 175 (D03).
	Provided = HOLConstruction::GetPopulationProvided(10, true);
	for (int32 House = 0; House < 20; ++House)
	{
		Provided += HOLConstruction::GetPopulationProvided(10, true);
	}
	TestEqual(TEXT("175 at most (D03)"), HOLProduction::GetPopulationCap(Provided), HOLProduction::MaxPopulation);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLConstructionTablesTest, "HeroesOfLiberty.Construction.Tables", UE::HeroesOfLiberty::Tests::Construction::Flags)
bool FHOLConstructionTablesTest::RunTest(const FString& Parameters)
{
	const UHOLBuildingSettings* Settings = GetDefault<UHOLBuildingSettings>();
	const double Grid = UHOLBuildingSettings::GetTileSize();
	TestEqual(TEXT("3 m tiles (D311)"), Grid, 300.0);
	TestTrue(TEXT("anti-rush radius set (D22)"), Settings->AntiRushRadius > 0.f);
	TestTrue(TEXT("3 builders ~2x as fast (D312)"),
		FMath::Abs(HOLConstruction::GetWorkPerStep(3, Settings->BuilderExponent) / float(HOLConstruction::WorkPerBuilder) - 2.f) < 0.05f);

	const FHOLBuildingTypeRow* MainCentre = UHOLBuildingSettings::FindType(HOLBuildingTypeNames::MainCentre);
	if (TestNotNull(TEXT("main centre"), MainCentre))
	{
		TestFalse(TEXT("the main centre is not built by Peasants"), MainCentre->IsBuildable());
		TestEqual(TEXT("the main centre: 4 x 4 tiles (12 m)"), MainCentre->Footprint, FVector2D(4.0 * Grid, 4.0 * Grid));
	}

	struct FExpected
	{
		FName Name;
		int32 Slot;
		int32 Population;
		TArray<EHOLResource> Accepts;
		int32 Wood;
	};
	const FExpected Expected[] = {
		{ HOLBuildingTypeNames::House, 0, 10, {}, 50 },
		{ HOLBuildingTypeNames::LumberCamp, 1, 0, { EHOLResource::Wood }, 100 },
		{ HOLBuildingTypeNames::MiningCamp, 2, 0, { EHOLResource::Stone, EHOLResource::Gold }, 100 },
	};
	for (const FExpected& Type : Expected)
	{
		const FHOLBuildingTypeRow* Row = UHOLBuildingSettings::FindType(Type.Name);
		const FString Name = Type.Name.ToString();
		if (!TestNotNull(Name + TEXT(" in the building type table"), Row))
		{
			continue;
		}
		TestEqual(Name + TEXT(": slot in the Peasant's grid (D313: A, Z, E)"), Row->BuildGridSlot, Type.Slot);
		TestEqual(Name + TEXT(": population given"), Row->PopulationProvided, Type.Population);
		TestTrue(Name + TEXT(": resources accepted"), Row->Accepts == Type.Accepts);
		TestTrue(Name + TEXT(": its cost, in wood only"), Row->Cost == FHOLResources(0, Type.Wood, 0, 0));
		TestTrue(Name + TEXT(": a build time"), Row->BuildTime > 0.f);
		TestEqual(Name + TEXT(": 2 x 2 tiles (6 m)"), Row->Footprint, FVector2D(2.0 * Grid, 2.0 * Grid));
		TestTrue(Name + TEXT(": smaller than the main centre"), MainCentre && Row->Footprint.X < MainCentre->Footprint.X);
	}
	return true;
}

// J6a.3 checks: what each construction state allows (D311, D312).
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLConstructionStatesTest, "HeroesOfLiberty.Construction.States", UE::HeroesOfLiberty::Tests::Construction::Flags)
bool FHOLConstructionStatesTest::RunTest(const FString& Parameters)
{
	using EState = EHOLConstructionState;
	const FHOLResources Cost(0, 50, 0, 0);

	// A bare foundation: walked over, serves nothing, cancelled with all its cost back.
	TestFalse(TEXT("a foundation does not block the navmesh (D312)"), HOLConstruction::BlocksNavigation(EState::Foundation));
	TestFalse(TEXT("a foundation serves nothing"), HOLConstruction::IsServing(EState::Foundation));
	TestTrue(TEXT("a foundation may be cancelled"), HOLConstruction::CanCancel(EState::Foundation));
	TestFalse(TEXT("a foundation has no work"), HOLConstruction::HasWorkStarted(EState::Foundation, 0));
	TestTrue(TEXT("a foundation cancelled: all back (D311)"), HOLConstruction::GetCancelRefund(Cost, EState::Foundation, 0) == Cost);

	// Under construction: blocks, serves nothing yet, cancelled with half back.
	TestTrue(TEXT("under construction: blocks the navmesh (D312)"), HOLConstruction::BlocksNavigation(EState::UnderConstruction));
	TestFalse(TEXT("under construction: serves nothing yet"), HOLConstruction::IsServing(EState::UnderConstruction));
	TestTrue(TEXT("under construction: work started"), HOLConstruction::HasWorkStarted(EState::UnderConstruction, 0));
	TestTrue(TEXT("under construction, cancelled: half back"),
		HOLConstruction::GetCancelRefund(Cost, EState::UnderConstruction, 1000) == FHOLResources(0, 25, 0, 0));
	TestTrue(TEXT("any work done halves the refund"), HOLConstruction::GetCancelRefund(Cost, EState::Foundation, 1) == FHOLResources(0, 25, 0, 0));

	// Finished: blocks, serves, never cancelled.
	TestTrue(TEXT("finished: blocks the navmesh"), HOLConstruction::BlocksNavigation(EState::Finished));
	TestTrue(TEXT("finished: serves its owner"), HOLConstruction::IsServing(EState::Finished));
	TestFalse(TEXT("finished: not cancelled"), HOLConstruction::CanCancel(EState::Finished));
	TestTrue(TEXT("finished: nothing back"), HOLConstruction::GetCancelRefund(Cost, EState::Finished, 0) == FHOLResources());

	// Population (D312): only once finished.
	TestEqual(TEXT("a house's foundation gives no place"), HOLConstruction::GetPopulationProvided(10, HOLConstruction::IsServing(EState::Foundation)), 0);
	TestEqual(TEXT("a finished house gives 10"), HOLConstruction::GetPopulationProvided(10, HOLConstruction::IsServing(EState::Finished)), 10);
	return true;
}

// J6a.3 checks: the footprint of a turned building, and where the navmesh is probed under a placement (D311).
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLConstructionProbesTest, "HeroesOfLiberty.Construction.Probes", UE::HeroesOfLiberty::Tests::Construction::Flags)
bool FHOLConstructionProbesTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::Construction;

	// Bounds of a footprint turned by its yaw: the same box on the quarter turns, swapped sides for a long one.
	const FVector2D Square(1200.0, 1200.0);
	for (const float Yaw : { 0.f, 90.f, -90.f, 180.f })
	{
		TestTrue(FString::Printf(TEXT("a square turned by %.0f: the same box"), Yaw),
			HOLConstruction::GetFootprintBounds(FVector2D(-15000.0, 0.0), Square, Yaw).Equals(FBox2D(FVector2D(-15600.0, -600.0), FVector2D(-14400.0, 600.0))));
	}
	TestTrue(TEXT("a 6 x 3 m footprint turned a quarter: 3 x 6 m"),
		HOLConstruction::GetFootprintBounds(FVector2D::ZeroVector, FVector2D(600.0, 300.0), 90.f).GetSize().Equals(FVector2D(300.0, 600.0), 0.01));
	TestTrue(TEXT("turned by 45 degrees: its bounds"),
		HOLConstruction::GetFootprintBounds(FVector2D::ZeroVector, FVector2D(600.0, 600.0), 45.f).GetSize().Equals(FVector2D(848.53, 848.53), 0.1));

	// A house (2 x 2 tiles): the middles of its four tiles, 1.5 m inside its edges, then its corners 1 m inside.
	TArray<FVector2D> Probes;
	HOLConstruction::GetNavProbes(House(0.0, 0.0), Tile, 100.0, Probes);
	TestEqual(TEXT("a house: 4 tile middles and 4 corners"), Probes.Num(), 8);
	const FVector2D Expected[] = { FVector2D(-150.0, -150.0), FVector2D(-150.0, 150.0), FVector2D(150.0, -150.0), FVector2D(150.0, 150.0),
		FVector2D(-200.0, -200.0), FVector2D(200.0, -200.0), FVector2D(-200.0, 200.0), FVector2D(200.0, 200.0) };
	for (const FVector2D& Point : Expected)
	{
		TestTrue(FString::Printf(TEXT("probe at (%.0f, %.0f)"), Point.X, Point.Y), Probes.ContainsByPredicate([&Point](const FVector2D& Probe) { return Probe.Equals(Point, 0.01); }));
	}
	// Every probe stays out of the forbidden area of a building it touches (its footprint + 25 cm + the agent's 60 cm, D310).
	const double Nearest = [&Probes]
	{
		double Best = TNumericLimits<double>::Max();
		for (const FVector2D& Probe : Probes)
		{
			Best = FMath::Min(Best, HOLConstruction::GetDistance(House(6.0, 0.0), Probe));
		}
		return Best;
	}();
	TestTrue(FString::Printf(TEXT("a neighbour touching it: its forbidden area (85 cm) far from every probe (%.0f cm)"), Nearest), Nearest >= 100.0);

	// The main centre (4 x 4 tiles): 16 tile middles and 4 corners; no tile: the middle alone.
	HOLConstruction::GetNavProbes(HOLConstruction::MakeFootprint(FVector2D::ZeroVector, Square), Tile, 100.0, Probes);
	TestEqual(TEXT("a main centre: 16 tile middles and 4 corners"), Probes.Num(), 20);
	HOLConstruction::GetNavProbes(House(3.0, 3.0), 0.0, 0.0, Probes);
	TestTrue(TEXT("no tile, no inset: the middle alone"), Probes.Num() == 1 && Probes[0].Equals(FVector2D(300.0, 300.0)));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLConstructionBuildSpotsTest, "HeroesOfLiberty.Construction.BuildSpots", UE::HeroesOfLiberty::Tests::Construction::Flags)
bool FHOLConstructionBuildSpotsTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::Construction;
	// J6a.4 (D312): a Peasant's body (60 cm), 25 cm of clearance (D310) and a gap of 20 cm; two bodies 130 cm apart.
	const double Radius = 60.0;
	const double Offset = Radius + 25.0 + 20.0;
	const double Spacing = 2.0 * Radius + 10.0;
	const FBox2D Footprint = House(0.0, 0.0);
	TArray<FVector2D> Spots;
	HOLConstruction::GetBuildSpots(Footprint, Offset, Spacing, FVector2D(-2000.0, 100.0), Spots);
	TestEqual(TEXT("a house: 5 spots on each of its 4 sides"), Spots.Num(), 20);

	// Every spot outside the footprint, at its offset from a side, no two bodies overlapping, none at a corner.
	bool bAtOffset = true;
	bool bApart = true;
	for (int32 Index = 0; Index < Spots.Num(); ++Index)
	{
		bAtOffset &= FMath::IsNearlyEqual(HOLConstruction::GetDistance(Footprint, Spots[Index]), Offset, 0.01);
		for (int32 Other = Index + 1; Other < Spots.Num(); ++Other)
		{
			bApart &= FVector2D::Distance(Spots[Index], Spots[Other]) >= Spacing - 0.01;
		}
	}
	TestTrue(TEXT("each spot at its offset from the footprint (a body at contact, clear of the navmesh's forbidden area)"), bAtOffset);
	TestTrue(TEXT("two builders' bodies never overlap"), bApart);

	// On every side, the nearest side first: the builders coming from the west fill the west side, then the others.
	TestTrue(TEXT("the first spot on the side facing them (west), nearest them"), Spots[0].Equals(FVector2D(-300.0 - Offset, 130.0), 0.01));
	for (int32 Index = 0; Index < 5; ++Index)
	{
		TestTrue(FString::Printf(TEXT("spot %d on the west side"), Index), FMath::IsNearlyEqual(Spots[Index].X, -300.0 - Offset, 0.01));
	}
	const auto CountSide = [&Spots](const TFunctionRef<bool(const FVector2D&)> OnSide)
	{
		int32 Count = 0;
		for (const FVector2D& Spot : Spots)
		{
			Count += OnSide(Spot) ? 1 : 0;
		}
		return Count;
	};
	TestEqual(TEXT("5 on the east side too"), CountSide([Offset](const FVector2D& Spot) { return FMath::IsNearlyEqual(Spot.X, 300.0 + Offset, 0.01); }), 5);
	TestEqual(TEXT("5 on the north side"), CountSide([Offset](const FVector2D& Spot) { return FMath::IsNearlyEqual(Spot.Y, 300.0 + Offset, 0.01); }), 5);
	TestEqual(TEXT("5 on the south side"), CountSide([Offset](const FVector2D& Spot) { return FMath::IsNearlyEqual(Spot.Y, -300.0 - Offset, 0.01); }), 5);
	TestTrue(TEXT("the far side (east) last"), FMath::IsNearlyEqual(Spots.Last().X, 300.0 + Offset, 0.01));
	TArray<FVector2D> Again;
	HOLConstruction::GetBuildSpots(Footprint, Offset, Spacing, FVector2D(-2000.0, 100.0), Again);
	TestTrue(TEXT("the same order at every call"), Again == Spots);

	// At the site (D312): the body within the gap of the footprint, measured from its edge.
	TestTrue(TEXT("at its spot: at the site"), HOLConstruction::IsAtSite(Footprint, Spots[0], Radius, 100.0));
	TestTrue(TEXT("stopped 50 cm short of its spot: still at the site"), HOLConstruction::IsAtSite(Footprint, Spots[0] - FVector2D(50.0, 0.0), Radius, 100.0));
	TestFalse(TEXT("2 m out: not at the site"), HOLConstruction::IsAtSite(Footprint, FVector2D(-300.0 - 200.0 - Radius, 0.0), Radius, 100.0));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLConstructionBuildQueueTest, "HeroesOfLiberty.Construction.BuildQueue", UE::HeroesOfLiberty::Tests::Construction::Flags)
bool FHOLConstructionBuildQueueTest::RunTest(const FString& Parameters)
{
	// J6a.4 (D311, D312): sites by number; Open: those not finished nor cancelled.
	TSet<int32> Closed;
	const auto IsOpen = [&Closed](const int32& Site) { return !Closed.Contains(Site); };
	TArray<int32> Queue;
	HOLConstruction::AddToBuildQueue(Queue, 1, false);
	TestTrue(TEXT("a click: its site alone"), Queue == TArray<int32>({ 1 }));
	HOLConstruction::AddToBuildQueue(Queue, 2, true);
	HOLConstruction::AddToBuildQueue(Queue, 3, true);
	TestTrue(TEXT("Shift: queued in the order placed"), Queue == TArray<int32>({ 1, 2, 3 }));
	HOLConstruction::AddToBuildQueue(Queue, 2, true);
	TestTrue(TEXT("Shift on a site already queued: no second time"), Queue == TArray<int32>({ 1, 2, 3 }));

	TestTrue(TEXT("the first one open: built first"), HOLConstruction::AdvanceBuildQueue<int32>(Queue, IsOpen) && Queue[0] == 1);
	Closed.Add(1);
	TestTrue(TEXT("finished: the next one of the queue"), HOLConstruction::AdvanceBuildQueue<int32>(Queue, IsOpen) && Queue[0] == 2);
	Closed.Add(2);
	Closed.Add(3);
	TestFalse(TEXT("the last finished, the one after cancelled: idle"), HOLConstruction::AdvanceBuildQueue<int32>(Queue, IsOpen));
	TestEqual(TEXT("nothing left"), Queue.Num(), 0);

	Queue = { 4, 5, 6 };
	Closed.Add(5);
	TestTrue(TEXT("a cancelled site in the middle waits its turn"), HOLConstruction::AdvanceBuildQueue<int32>(Queue, IsOpen) && Queue == TArray<int32>({ 4, 5, 6 }));
	Closed.Add(4);
	TestTrue(TEXT("then skipped with the finished one before it"), HOLConstruction::AdvanceBuildQueue<int32>(Queue, IsOpen) && Queue == TArray<int32>({ 6 }));
	HOLConstruction::AddToBuildQueue(Queue, 7, false);
	TestTrue(TEXT("a click without Shift replaces the queue"), Queue == TArray<int32>({ 7 }));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLConstructionWorkQueueTest, "HeroesOfLiberty.Construction.WorkQueue", UE::HeroesOfLiberty::Tests::Construction::Flags)
bool FHOLConstructionWorkQueueTest::RunTest(const FString& Parameters)
{
	// The user's trial of J6a.4: Shift + place house 1, Shift + place house 2, Shift + right click on tree 30, Shift + right
	// click on vein 90; sites by number, Open: those not finished nor cancelled.
	using FItem = HOLConstruction::TWorkItem<int32>;
	TSet<int32> Closed;
	const auto IsOpen = [&Closed](const int32& Site) { return !Closed.Contains(Site); };
	TArray<FItem> Queue;
	HOLConstruction::AddToBuildQueue(Queue, FItem::MakeBuild(1), true);
	HOLConstruction::AddToBuildQueue(Queue, FItem::MakeBuild(2), true);
	HOLConstruction::AddToBuildQueue(Queue, FItem::MakeGather(30), true);
	HOLConstruction::AddToBuildQueue(Queue, FItem::MakeGather(30), true);
	HOLConstruction::AddToBuildQueue(Queue, FItem::MakeGather(90), true);
	TestTrue(TEXT("sites and sources queued in the order given, a source twice once"),
		Queue == TArray<FItem>({ FItem::MakeBuild(1), FItem::MakeBuild(2), FItem::MakeGather(30), FItem::MakeGather(90) }));
	TestFalse(TEXT("a site is no gathering"), Queue[0].IsGather());
	TestTrue(TEXT("a source is one"), Queue[2].IsGather());
	TestFalse(TEXT("site 0 and source 0 differ"), FItem::MakeBuild(0) == FItem::MakeGather(0));

	TestTrue(TEXT("house 1 open: built first"), HOLConstruction::AdvanceWorkQueue<int32>(Queue, false, IsOpen) && Queue[0] == FItem::MakeBuild(1));
	Closed.Add(1);
	TestTrue(TEXT("house 1 finished: house 2"), HOLConstruction::AdvanceWorkQueue<int32>(Queue, false, IsOpen) && Queue[0] == FItem::MakeBuild(2));
	Closed.Add(2);
	TestTrue(TEXT("house 2 finished: the tree"), HOLConstruction::AdvanceWorkQueue<int32>(Queue, false, IsOpen) && Queue[0] == FItem::MakeGather(30));
	TestTrue(TEXT("a gathering lasts while its massif gives (head not done)"),
		HOLConstruction::AdvanceWorkQueue<int32>(Queue, false, IsOpen) && Queue[0] == FItem::MakeGather(30) && Queue.Num() == 2);
	TestTrue(TEXT("its massif used up: the vein"), HOLConstruction::AdvanceWorkQueue<int32>(Queue, true, IsOpen) && Queue[0] == FItem::MakeGather(90));
	TestFalse(TEXT("the vein used up, nothing after it: idle"), HOLConstruction::AdvanceWorkQueue<int32>(Queue, true, IsOpen));
	TestEqual(TEXT("nothing left"), Queue.Num(), 0);

	// A gathering before a site: the site finished by others meanwhile is skipped once the gathering is over.
	Queue = { FItem::MakeGather(31), FItem::MakeBuild(3), FItem::MakeBuild(4) };
	Closed.Add(3);
	TestTrue(TEXT("a site finished behind a gathering waits its turn"),
		HOLConstruction::AdvanceWorkQueue<int32>(Queue, false, IsOpen) && Queue.Num() == 3 && Queue[0] == FItem::MakeGather(31));
	TestTrue(TEXT("the gathering over: the finished site skipped, house 4"),
		HOLConstruction::AdvanceWorkQueue<int32>(Queue, true, IsOpen) && Queue.Num() == 1 && Queue[0] == FItem::MakeBuild(4));
	HOLConstruction::AddToBuildQueue(Queue, FItem::MakeGather(32), false);
	TestTrue(TEXT("a right click without Shift replaces the queue"), Queue == TArray<FItem>({ FItem::MakeGather(32) }));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLConstructionNextSiteTest, "HeroesOfLiberty.Construction.NextSite", UE::HeroesOfLiberty::Tests::Construction::Flags)
bool FHOLConstructionNextSiteTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::Construction;
	// The user's trial of J6a.4: a house finished at (0, 0) m, 6 m wide; the next site within 20 m of its footprint.
	const FBox2D Finished = House(0.0, 0.0);
	TestEqual(TEXT("gap to a house touching it"), HOLConstruction::GetGap(Finished, House(6.0, 0.0)), 0.0, 0.01);
	TestEqual(TEXT("gap to a house 10 m off its side"), HOLConstruction::GetGap(Finished, House(16.0, 0.0)), 1000.0, 0.01);
	TestEqual(TEXT("gap to a house off its corner (3 m, 4 m)"), HOLConstruction::GetGap(Finished, House(9.0, 10.0)), 500.0, 0.01);
	TestEqual(TEXT("gap to an overlapping box"), HOLConstruction::GetGap(Finished, House(2.0, 2.0)), 0.0, 0.01);

	const TArray<FBox2D> Sites = { House(-46.0, 0.0), House(0.0, 16.0), House(0.0, -19.0) };
	TestEqual(TEXT("10 m away chosen, not 40 m away nor 13 m"), HOLConstruction::FindNextSite(Finished, Sites, 2000.0), 1);
	TestEqual(TEXT("only 40 m away: none, idle"), HOLConstruction::FindNextSite(Finished, TArray<FBox2D>({ House(-46.0, 0.0) }), 2000.0), INDEX_NONE);
	TestEqual(TEXT("exactly 20 m away: still near"), HOLConstruction::FindNextSite(Finished, TArray<FBox2D>({ House(26.0, 0.0) }), 2000.0), 0);
	TestEqual(TEXT("two as near: the first"), HOLConstruction::FindNextSite(Finished, TArray<FBox2D>({ House(16.0, 0.0), House(-16.0, 0.0) }), 2000.0), 0);
	TestEqual(TEXT("no site: none"), HOLConstruction::FindNextSite(Finished, TArray<FBox2D>(), 2000.0), INDEX_NONE);
	TestEqual(TEXT("range 0: only one touching it"), HOLConstruction::FindNextSite(Finished, Sites, 0.0), INDEX_NONE);
	return true;
}

#endif
