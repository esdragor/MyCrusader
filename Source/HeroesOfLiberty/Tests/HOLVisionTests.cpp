// Copyright Epic Games, Inc. All Rights Reserved.

#include "Buildings/HOLBuildingTypes.h"
#include "Engine/DataTable.h"
#include "Map/HOLVisionRules.h"
#include "Simulation/HOLSimulationSettings.h"
#include "Simulation/HOLUnitTypes.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

// F1 checks (FogInProgress.md, D381-D383, D521): the fog's pure rules, grid, disc, forest, counts per camp.
namespace UE::HeroesOfLiberty::Tests::Vision
{
	constexpr EAutomationTestFlags Flags = EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter;

	/** 20 x 20 cells of 4 m (-40 m to +40 m). */
	FHOLVisionGrid MakeGrid()
	{
		return FHOLVisionGrid::MakeCentered(20, 400.);
	}

	/** One cell per grid cell, forest on the columns From to To (every row). */
	TArray<bool> MakeWall(const FHOLVisionGrid& Grid, const int32 From, const int32 To)
	{
		TArray<bool> Blocked;
		Blocked.Init(false, Grid.Num());
		for (int32 Y = 0; Y < Grid.Side; ++Y)
		{
			for (int32 X = From; X <= To; ++X)
			{
				Blocked[Grid.ToIndex(FIntPoint(X, Y))] = true;
			}
		}
		return Blocked;
	}

	bool Sees(const FHOLVisionGrid& Grid, const TArray<bool>& Blocked, const FIntPoint& From, const FIntPoint& To, const double Radius)
	{
		return HOLVisionRules::GetSeenCells(Grid, Blocked, From, Radius).Contains(To);
	}

	/** The cells of both discs, once each. */
	int32 CountUnion(const TArray<FIntPoint>& A, const TArray<FIntPoint>& B)
	{
		TSet<FIntPoint> Cells(A);
		Cells.Append(B);
		return Cells.Num();
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLVisionGridTest, "HeroesOfLiberty.Vision.Grid", UE::HeroesOfLiberty::Tests::Vision::Flags)
bool FHOLVisionGridTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::Vision;
	const FHOLVisionGrid Grid = MakeGrid();
	TestEqual(TEXT("side"), Grid.Side, 20);
	TestEqual(TEXT("cells"), Grid.Num(), 400);
	TestEqual(TEXT("the origin in the cell right of the middle"), Grid.ToCell(FVector2D(0., 0.)), FIntPoint(10, 10));
	TestEqual(TEXT("the corner"), Grid.ToCell(FVector2D(-4000., -4000.)), FIntPoint(0, 0));
	TestEqual(TEXT("just in the last cell"), Grid.ToCell(FVector2D(3999., -1.)), FIntPoint(19, 9));
	TestFalse(TEXT("the far edge out of the grid"), Grid.IsInside(Grid.ToCell(FVector2D(4000., 0.))));
	TestFalse(TEXT("below the corner out of the grid"), Grid.IsInside(Grid.ToCell(FVector2D(-4001., 0.))));
	TestEqual(TEXT("centre of the first cell"), Grid.GetCenter(FIntPoint(0, 0)), FVector2D(-3800., -3800.));
	TestEqual(TEXT("a centre comes back to its cell"), Grid.ToCell(Grid.GetCenter(FIntPoint(7, 13))), FIntPoint(7, 13));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLVisionDiscTest, "HeroesOfLiberty.Vision.Disc", UE::HeroesOfLiberty::Tests::Vision::Flags)
bool FHOLVisionDiscTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::Vision;
	const FHOLVisionGrid Grid = MakeGrid();
	const TArray<FIntPoint> Disc = HOLVisionRules::GetDisc(Grid, FIntPoint(10, 10), 800.);
	TestEqual(TEXT("a sight of 2 cells: 13 cells"), Disc.Num(), 13);
	for (const FIntPoint Offset : { FIntPoint(0, 0), FIntPoint(2, 0), FIntPoint(-2, 0), FIntPoint(0, 2), FIntPoint(0, -2), FIntPoint(1, 1), FIntPoint(-1, -1) })
	{
		TestTrue(FString::Printf(TEXT("cell %d,%d in the disc"), Offset.X, Offset.Y), Disc.Contains(FIntPoint(10, 10) + Offset));
	}
	TestFalse(TEXT("the corner 2,2 out of the disc"), Disc.Contains(FIntPoint(12, 12)));
	TestFalse(TEXT("3 cells out of the disc"), Disc.Contains(FIntPoint(13, 10)));

	TestEqual(TEXT("in the grid's corner: the quarter (6 cells)"), HOLVisionRules::GetDisc(Grid, FIntPoint(0, 0), 800.).Num(), 6);
	TestEqual(TEXT("D523: a sight of 8 cells, 197 cells"), HOLVisionRules::GetDisc(FHOLVisionGrid::MakeCentered(40, 400.), FIntPoint(20, 20), 3200.).Num(), 197);
	TestEqual(TEXT("no sight: its own cell"), HOLVisionRules::GetDisc(Grid, FIntPoint(4, 4), 0.).Num(), 1);

	TArray<bool> Open;
	Open.Init(false, Grid.Num());
	TestEqual(TEXT("no forest: the seen cells are the disc"), HOLVisionRules::GetSeenCells(Grid, Open, FIntPoint(10, 10), 1600.).Num(),
		HOLVisionRules::GetDisc(Grid, FIntPoint(10, 10), 1600.).Num());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLVisionForestTest, "HeroesOfLiberty.Vision.Forest", UE::HeroesOfLiberty::Tests::Vision::Flags)
bool FHOLVisionForestTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::Vision;
	const FHOLVisionGrid Grid = MakeGrid();
	const TArray<bool> Wall = MakeWall(Grid, 8, 8);
	const FIntPoint Eye(5, 10);
	TestTrue(TEXT("before the wall: seen"), Sees(Grid, Wall, Eye, FIntPoint(7, 10), 3200.));
	TestTrue(TEXT("the edge of the forest: seen"), Sees(Grid, Wall, Eye, FIntPoint(8, 10), 3200.));
	TestTrue(TEXT("the edge seen aslant"), Sees(Grid, Wall, Eye, FIntPoint(8, 12), 3200.));
	TestFalse(TEXT("behind the wall: hidden"), Sees(Grid, Wall, Eye, FIntPoint(9, 10), 3200.));
	TestFalse(TEXT("far behind the wall: hidden"), Sees(Grid, Wall, Eye, FIntPoint(12, 10), 3200.));
	TestFalse(TEXT("behind the wall aslant: hidden"), Sees(Grid, Wall, Eye, FIntPoint(10, 12), 3200.));
	TestTrue(TEXT("on the open side: seen"), Sees(Grid, Wall, Eye, FIntPoint(1, 10), 3200.));

	// An observer in a forest of 3 columns sees out of it on both sides, then the next forest stops it as any other.
	TArray<bool> Woods = MakeWall(Grid, 8, 10);
	const TArray<bool> Second = MakeWall(Grid, 14, 15);
	for (int32 Index = 0; Index < Woods.Num(); ++Index)
	{
		Woods[Index] = Woods[Index] || Second[Index];
	}
	const FIntPoint Woodcutter(9, 10);
	TestTrue(TEXT("in the forest: its own cell seen"), HOLVisionRules::IsInLineOfSight(Grid, Woods, Woodcutter, Woodcutter));
	TestTrue(TEXT("in the forest: its neighbour seen"), Sees(Grid, Woods, Woodcutter, FIntPoint(10, 10), 3200.));
	TestTrue(TEXT("in the forest: out of it on the right"), Sees(Grid, Woods, Woodcutter, FIntPoint(12, 10), 3200.));
	TestTrue(TEXT("in the forest: out of it on the left"), Sees(Grid, Woods, Woodcutter, FIntPoint(6, 10), 3200.));
	TestTrue(TEXT("the next forest's edge: seen"), Sees(Grid, Woods, Woodcutter, FIntPoint(14, 10), 3200.));
	TestFalse(TEXT("inside the next forest: hidden"), Sees(Grid, Woods, Woodcutter, FIntPoint(15, 10), 3200.));
	TestFalse(TEXT("behind the next forest: hidden"), Sees(Grid, Woods, Woodcutter, FIntPoint(16, 10), 3200.));
	TestFalse(TEXT("out of its sight: hidden"), Sees(Grid, Woods, Woodcutter, FIntPoint(18, 10), 3200.));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLVisionHeightTest, "HeroesOfLiberty.Vision.Height", UE::HeroesOfLiberty::Tests::Vision::Flags)
bool FHOLVisionHeightTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::Vision;
	const FHOLVisionTuning Tuning;
	TestEqual(TEXT("on flat ground: its sight"), HOLVisionRules::GetSightRadius(3200., 0., Tuning), 3200.);
	TestTrue(TEXT("higher: further"), HOLVisionRules::GetSightRadius(3200., 300., Tuning) > 3200.);
	TestEqual(TEXT("moderate: capped"), HOLVisionRules::GetSightRadius(3200., 100000., Tuning), 3200. + Tuning.MaxHeightBonus);
	TestEqual(TEXT("lower: no malus"), HOLVisionRules::GetSightRadius(3200., -500., Tuning), 3200.);
	const FHOLVisionGrid Grid = FHOLVisionGrid::MakeCentered(60, 400.);
	TestTrue(TEXT("a hill made by the test: more cells seen"),
		HOLVisionRules::GetDisc(Grid, FIntPoint(30, 30), HOLVisionRules::GetSightRadius(3200., 400., Tuning)).Num() > HOLVisionRules::GetDisc(Grid, FIntPoint(30, 30), 3200.).Num());
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLVisionCountsTest, "HeroesOfLiberty.Vision.Counts", UE::HeroesOfLiberty::Tests::Vision::Flags)
bool FHOLVisionCountsTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::Vision;
	FHOLVisionMap Map;
	Map.Init(MakeGrid(), 2);
	const FHOLVisionGrid& Grid = Map.GetGrid();
	TestEqual(TEXT("at first: black"), Map.GetVision(0, FIntPoint(10, 10)), EHOLCellVision::Unexplored);

	const FVector2D Start = Grid.GetCenter(FIntPoint(10, 10));
	const int32 Scout = Map.AddObserver(0, Start, 800.);
	TestEqual(TEXT("its cell seen once"), Map.GetCount(0, FIntPoint(10, 10)), 1);
	TestEqual(TEXT("in sight"), Map.GetVision(0, FIntPoint(12, 10)), EHOLCellVision::Visible);
	TestEqual(TEXT("its disc in sight"), Map.CountVisible(0), HOLVisionRules::GetDisc(Grid, FIntPoint(10, 10), 800.).Num());
	TestEqual(TEXT("the other camp sees nothing"), Map.CountVisible(1), 0);
	TestFalse(TEXT("the other camp: hidden"), Map.IsVisible(1, Start));
	TestTrue(TEXT("its camp: seen"), Map.IsVisible(0, Start));

	TestFalse(TEXT("a step in its cell: nothing counted again"), Map.MoveObserver(Scout, Start + FVector2D(150., -150.)));
	TestTrue(TEXT("to another cell: counted again"), Map.MoveObserver(Scout, Grid.GetCenter(FIntPoint(14, 10))));
	TestEqual(TEXT("left behind: explored"), Map.GetVision(0, FIntPoint(8, 10)), EHOLCellVision::Explored);
	TestEqual(TEXT("left behind: no observer"), Map.GetCount(0, FIntPoint(8, 10)), 0);
	TestEqual(TEXT("ahead: in sight"), Map.GetVision(0, FIntPoint(16, 10)), EHOLCellVision::Visible);
	const int32 BothDiscs = CountUnion(HOLVisionRules::GetDisc(Grid, FIntPoint(10, 10), 800.), HOLVisionRules::GetDisc(Grid, FIntPoint(14, 10), 800.));
	TestEqual(TEXT("explored: both discs"), Map.CountExplored(0), BothDiscs);

	Map.RemoveObserver(Scout);
	TestEqual(TEXT("gone: nothing in sight"), Map.CountVisible(0), 0);
	TestEqual(TEXT("gone: the explored stays"), Map.CountExplored(0), BothDiscs);
	TestEqual(TEXT("gone: no observer"), Map.GetNumObservers(), 0);

	// A team: its members' sight is one (D371); the cells both see counted twice.
	const int32 A = Map.AddObserver(0, Grid.GetCenter(FIntPoint(4, 4)), 800.);
	const int32 B = Map.AddObserver(0, Grid.GetCenter(FIntPoint(6, 4)), 800.);
	TestEqual(TEXT("team: the cell between seen twice"), Map.GetCount(0, FIntPoint(5, 4)), 2);
	TestEqual(TEXT("team: the union in sight"), Map.CountVisible(0),
		CountUnion(HOLVisionRules::GetDisc(Grid, FIntPoint(4, 4), 800.), HOLVisionRules::GetDisc(Grid, FIntPoint(6, 4), 800.)));
	TestEqual(TEXT("team: the other camp still blind"), Map.CountVisible(1), 0);
	Map.RemoveObserver(A);
	TestEqual(TEXT("one gone: the other still sees"), Map.GetCount(0, FIntPoint(5, 4)), 1);
	Map.SetObserverRadius(B, 0.);
	TestEqual(TEXT("no sight: its own cell"), Map.CountVisible(0), 1);
	Map.RemoveObserver(B);
	TestEqual(TEXT("all gone: every count back to 0"), Map.CountVisible(0), 0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLVisionTreeCutTest, "HeroesOfLiberty.Vision.TreeCut", UE::HeroesOfLiberty::Tests::Vision::Flags)
bool FHOLVisionTreeCutTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::Vision;
	FHOLVisionMap Map;
	Map.Init(MakeGrid(), 1);
	const FHOLVisionGrid& Grid = Map.GetGrid();
	for (int32 Y = 0; Y < Grid.Side; ++Y)
	{
		Map.SetBlocked(FIntPoint(8, Y), true);
	}
	TestTrue(TEXT("forest"), Map.IsBlocked(FIntPoint(8, 10)));
	TestFalse(TEXT("forest again: no change"), Map.SetBlocked(FIntPoint(8, 10), true));

	const int32 Eye = Map.AddObserver(0, Grid.GetCenter(FIntPoint(5, 10)), 3200.);
	TestEqual(TEXT("the edge: in sight"), Map.GetVision(0, FIntPoint(8, 10)), EHOLCellVision::Visible);
	TestEqual(TEXT("behind the forest: black"), Map.GetVision(0, FIntPoint(9, 10)), EHOLCellVision::Unexplored);

	TestTrue(TEXT("the last tree cut"), Map.SetBlocked(FIntPoint(8, 10), false));
	TestEqual(TEXT("through the gap: in sight"), Map.GetVision(0, FIntPoint(9, 10)), EHOLCellVision::Visible);
	TestEqual(TEXT("the counts match the forest now"), Map.CountVisible(0),
		[&Grid, &Map]()
		{
			TArray<bool> Blocked;
			for (int32 Index = 0; Index < Grid.Num(); ++Index)
			{
				Blocked.Add(Map.IsBlocked(Grid.ToCell(Index)));
			}
			return HOLVisionRules::GetSeenCells(Grid, Blocked, FIntPoint(5, 10), 3200.).Num();
		}());

	TestTrue(TEXT("planted again"), Map.SetBlocked(FIntPoint(8, 10), true));
	TestEqual(TEXT("behind the forest again: explored"), Map.GetVision(0, FIntPoint(9, 10)), EHOLCellVision::Explored);
	Map.RemoveObserver(Eye);
	TestEqual(TEXT("gone: exactly what it laid taken off"), Map.CountVisible(0), 0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLVisionSightRadiusTest, "HeroesOfLiberty.Vision.SightRadius", UE::HeroesOfLiberty::Tests::Vision::Flags)
bool FHOLVisionSightRadiusTest::RunTest(const FString& Parameters)
{
	// F2 (D523): every type of the real tables sees, beyond its reach (attack or riposte) by a tile at least.
	const double Tile = UHOLBuildingSettings::GetTileSize();
	const UDataTable* UnitTypes = GetDefault<UHOLSimulationSettings>()->UnitTypesTable.LoadSynchronous();
	if (!TestNotNull(TEXT("the unit type table"), UnitTypes))
	{
		return false;
	}
	UnitTypes->ForeachRow<FHOLUnitTypeRow>(TEXT("Vision test"), [this, Tile](const FName& Name, const FHOLUnitTypeRow& Row)
	{
		const double Reach = FMath::Max(Row.AttackRange, Row.AggressionRadius);
		TestTrue(*FString::Printf(TEXT("%s sees (%.0f cm) beyond its reach (%.0f cm) by a tile"), *Name.ToString(), Row.SightRadius, Reach),
			Row.SightRadius >= Reach + Tile);
		TestEqual(*FString::Printf(TEXT("%s: the simulation reads its sight"), *Name.ToString()), Row.ToConstants(Name).SightRadius, Row.SightRadius);
	});
	const UDataTable* BuildingTypes = GetDefault<UHOLBuildingSettings>()->BuildingTypesTable.LoadSynchronous();
	if (!TestNotNull(TEXT("the building type table"), BuildingTypes))
	{
		return false;
	}
	BuildingTypes->ForeachRow<FHOLBuildingTypeRow>(TEXT("Vision test"), [this, Tile](const FName& Name, const FHOLBuildingTypeRow& Row)
	{
		const double HalfSide = 0.5 * FMath::Max(Row.Footprint.X, Row.Footprint.Y);
		TestTrue(*FString::Printf(TEXT("%s sees (%.0f cm) beyond its walls by a tile"), *Name.ToString(), Row.SightRadius), Row.SightRadius >= HalfSide + Tile);
	});
	return true;
}

#endif
