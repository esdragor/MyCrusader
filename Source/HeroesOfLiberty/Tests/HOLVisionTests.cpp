// Copyright Epic Games, Inc. All Rights Reserved.

#include "Buildings/HOLBuildingTypes.h"
#include "Engine/DataTable.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Map/HOLVisionRules.h"
#include "Match/HOLMatchConfig.h"
#include "Net/HOLManagerClientRules.h"
#include "Simulation/HOLSimulationSettings.h"
#include "Simulation/HOLUnitTypes.h"
#include "Misc/AutomationTest.h"
#include "Misc/ScopeExit.h"
#include "Resources/HOLSourceSubsystem.h"
#include "Resources/HOLSourceTypes.h"

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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLVisionForestTreesTest, "HeroesOfLiberty.Vision.ForestTrees", UE::HeroesOfLiberty::Tests::Vision::Flags)
bool FHOLVisionForestTreesTest::RunTest(const FString& Parameters)
{
	// F2 (D383): a cell is forest from ForestTrees standing trees on; the last ones cut, it opens again.
	using namespace UE::HeroesOfLiberty::Tests::Vision;
	FHOLVisionMap Map;
	Map.Init(MakeGrid(), 1);
	const FHOLVisionGrid& Grid = Map.GetGrid();
	const FHOLVisionTuning Tuning;
	const FIntPoint Wood(8, 10);
	const FVector2D Centre = Grid.GetCenter(Wood);
	const int32 Eye = Map.AddObserver(0, Grid.GetCenter(FIntPoint(5, 10)), 3200.);
	for (int32 Tree = 0; Tree < Tuning.ForestTrees - 1; ++Tree)
	{
		Map.AddBlocker(Centre + FVector2D(-150. + 100. * Tree, 120.));
	}
	TestEqual(TEXT("its trees counted"), Map.GetBlockers(Wood), Tuning.ForestTrees - 1);
	TestFalse(TEXT("too few trees: open"), Map.IsBlocked(Wood));
	TestEqual(TEXT("open: seen through"), Map.GetVision(0, FIntPoint(9, 10)), EHOLCellVision::Visible);

	Map.AddBlocker(Centre + FVector2D(150., -150.));
	TestTrue(TEXT("enough trees: forest"), Map.IsBlocked(Wood));
	TestEqual(TEXT("forest: behind it hidden"), Map.GetVision(0, FIntPoint(9, 10)), EHOLCellVision::Explored);

	Map.RemoveBlocker(Centre + FVector2D(150., -150.));
	TestFalse(TEXT("a tree cut below the count: open again"), Map.IsBlocked(Wood));
	TestEqual(TEXT("open again: seen through"), Map.GetVision(0, FIntPoint(9, 10)), EHOLCellVision::Visible);
	TestEqual(TEXT("the trees left"), Map.GetBlockers(Wood), Tuning.ForestTrees - 1);

	Map.AddBlocker(FVector2D(1.e6, 0.));
	Map.RemoveBlocker(FVector2D(1.e6, 0.));
	TestEqual(TEXT("out of the grid: nothing"), Map.GetBlockers(Wood), Tuning.ForestTrees - 1);
	Map.RemoveObserver(Eye);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLVisionSourceFeedTest, "HeroesOfLiberty.Vision.SourceFeed", UE::HeroesOfLiberty::Tests::Vision::Flags)
bool FHOLVisionSourceFeedTest::RunTest(const FString& Parameters)
{
	// F2 (D383): the forest is read from the source registry (the real table's trees), and opens as its trees are cut.
	using namespace UE::HeroesOfLiberty::Tests::Vision;
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, /*bInformEngineOfWorld*/ false);
	FWorldContext& Context = GEngine->CreateNewWorldContext(EWorldType::Game);
	Context.SetCurrentWorld(World);
	World->InitializeActorsForPlay(FURL());
	ON_SCOPE_EXIT
	{
		GEngine->DestroyWorldContext(World);
		World->DestroyWorld(false);
	};
	UHOLSourceSubsystem* Sources = World->GetSubsystem<UHOLSourceSubsystem>();
	if (!TestNotNull(TEXT("the source registry"), Sources))
	{
		return false;
	}
	Sources->OnWorldBeginPlay(*World);

	FHOLVisionMap Map;
	Map.Init(MakeGrid(), 1);
	const FHOLVisionGrid& Grid = Map.GetGrid();
	TArray<TPair<int32, bool>> Events;
	Sources->OnSourceStanding.AddLambda([&Map, &Events, Sources](const int32 SourceIndex, const bool bStanding)
	{
		Events.Emplace(SourceIndex, bStanding);
		if (Sources->BlocksSight(SourceIndex))
		{
			const FVector2D Position(Sources->GetSources()[SourceIndex].Location);
			bStanding ? Map.AddBlocker(Position) : Map.RemoveBlocker(Position);
		}
	});

	const FIntPoint Wood(12, 10);
	const FVector Centre(Grid.GetCenter(Wood), 0.);
	const int32 First = Sources->AddSource(HOLSourceTypeNames::Tree, Centre + FVector(-120., -120., 0.));
	const int32 Bush = Sources->AddSource(HOLSourceTypeNames::BerryBush, Centre + FVector(0., 120., 0.));
	TestTrue(TEXT("a tree cuts the sight"), Sources->BlocksSight(First));
	TestFalse(TEXT("a berry bush does not"), Sources->BlocksSight(Bush));
	TestEqual(TEXT("each source told"), Events.Num(), 2);
	TestEqual(TEXT("one tree counted, not the bush"), Map.GetBlockers(Wood), 1);
	TArray<int32> Trees = { First };
	while (Map.GetBlockers(Wood) < FHOLVisionTuning().ForestTrees && Trees.Num() < 10)
	{
		Trees.Add(Sources->AddSource(HOLSourceTypeNames::Tree, Centre + FVector(120., -150. + 40. * Trees.Num(), 0.)));
	}
	TestTrue(TEXT("a wood: forest"), Map.IsBlocked(Wood));

	const int32 Amount = Sources->GetSources()[First].Remaining;
	TestEqual(TEXT("the first tree cut down"), Sources->Take(First, Amount), Amount);
	TestTrue(TEXT("told gone"), Events.Num() > 0 && Events.Last() == TPair<int32, bool>(First, false));
	TestFalse(TEXT("its last trees cut below the count: open"), Map.IsBlocked(Wood));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLVisionBuildingSightTest, "HeroesOfLiberty.Vision.BuildingSight", UE::HeroesOfLiberty::Tests::Vision::Flags)
bool FHOLVisionBuildingSightTest::RunTest(const FString& Parameters)
{
	// F3.2 (D523): a building sees its whole sight finished, half as a site, nothing in ruin.
	TestEqual(TEXT("standing: whole"), HOLVisionRules::GetBuildingSight(6400., EHOLBuildingSight::Standing), 6400.);
	TestEqual(TEXT("a site: half"), HOLVisionRules::GetBuildingSight(6400., EHOLBuildingSight::Site), 3200.);
	TestEqual(TEXT("rubble: nothing"), HOLVisionRules::GetBuildingSight(6400., EHOLBuildingSight::Rubble), 0.);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLVisionGhostsTest, "HeroesOfLiberty.Vision.Ghosts", UE::HeroesOfLiberty::Tests::Vision::Flags)
bool FHOLVisionGhostsTest::RunTest(const FString& Parameters)
{
	// F4 (D382): a ghost keeps the last look; a destruction out of sight is learned only by going back.
	const FHOLGhost House{ HOLVisionRules::MakeGhostKey(false, 7), TEXT("House"), 2, FVector2D(1000., 0.), EHOLBuildingSight::Standing, 0 };
	const FHOLGhost Tree{ HOLVisionRules::MakeGhostKey(true, 7), TEXT("Tree"), 0xFF, FVector2D(-1000., 0.), EHOLBuildingSight::Standing, 100 };
	TestNotEqual(TEXT("a building's and a source's keys apart"), House.Key, Tree.Key);
	TMap<uint64, FHOLGhost> Ghosts;
	bool bInSight = true;
	const auto Sight = [&bInSight](const FVector2D&) { return bInSight; };

	TestTrue(TEXT("seen: remembered"), HOLVisionRules::UpdateGhosts(Ghosts, { House, Tree }, Sight));
	TestEqual(TEXT("both remembered"), Ghosts.Num(), 2);
	TestFalse(TEXT("seen again unchanged: nothing new"), HOLVisionRules::UpdateGhosts(Ghosts, { House, Tree }, Sight));

	bInSight = false;
	FHOLGhost Site = House;
	Site.State = EHOLBuildingSight::Site;
	FHOLGhost Cut = Tree;
	Cut.Remaining = 40;
	HOLVisionRules::UpdateGhosts(Ghosts, { Site, Cut }, Sight);
	TestTrue(TEXT("out of sight: the house remembered"), Ghosts.Contains(House.Key) && Ghosts.Contains(Tree.Key));
	TestEqual(TEXT("out of sight: its last look"), Ghosts.FindRef(House.Key).State, EHOLBuildingSight::Standing);
	TestEqual(TEXT("out of sight: the wood it held"), Ghosts.FindRef(Tree.Key).Remaining, 100);
	HOLVisionRules::UpdateGhosts(Ghosts, {}, Sight);
	TestEqual(TEXT("destroyed out of sight: still remembered"), Ghosts.Num(), 2);

	bInSight = true;
	TestTrue(TEXT("back in sight"), HOLVisionRules::UpdateGhosts(Ghosts, { Cut }, Sight));
	TestFalse(TEXT("gone, seen again: forgotten"), Ghosts.Contains(House.Key));
	TestEqual(TEXT("still there: its look now"), Ghosts.FindRef(Tree.Key).Remaining, 40);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLVisionBuildingDrawTest, "HeroesOfLiberty.Vision.BuildingDraw", UE::HeroesOfLiberty::Tests::Vision::Flags)
bool FHOLVisionBuildingDrawTest::RunTest(const FString& Parameters)
{
	// F7.3 (b): a building of another camp out of sight is drawn from the memory, or not at all if never seen.
	using namespace HOLVisionRules;
	TestFalse(TEXT("another camp's, in sight: as it is"), BuildingDrawsFromMemory(false, true));
	TestTrue(TEXT("another camp's, out of sight: from the memory"), BuildingDrawsFromMemory(false, false));
	TestFalse(TEXT("the viewer's own camp's, out of sight: as it is"), BuildingDrawsFromMemory(true, false));
	TestFalse(TEXT("the viewer's own camp's, in sight: as it is"), BuildingDrawsFromMemory(true, true));

	TestEqual(TEXT("in sight: live"), GetBuildingDraw(false, true, false), EHOLBuildingDraw::Live);
	TestEqual(TEXT("own camp's: live, remembered or not"), GetBuildingDraw(true, false, false), EHOLBuildingDraw::Live);
	TestEqual(TEXT("out of sight, remembered: memory"), GetBuildingDraw(false, false, true), EHOLBuildingDraw::Memory);
	TestEqual(TEXT("out of sight, never seen: hidden"), GetBuildingDraw(false, false, false), EHOLBuildingDraw::Hidden);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLVisionSourceDrawTest, "HeroesOfLiberty.Vision.SourceDraw", UE::HeroesOfLiberty::Tests::Vision::Flags)
bool FHOLVisionSourceDrawTest::RunTest(const FString& Parameters)
{
	// F7.3 (c): a tree cut out of sight stays drawn while the camp remembers it standing and its place is out of sight.
	using namespace HOLVisionRules;
	TestTrue(TEXT("remembered, out of sight: still drawn"), SourceStaysDrawn(true, false));
	TestFalse(TEXT("remembered, in sight: the cut is seen"), SourceStaysDrawn(true, true));
	TestFalse(TEXT("never seen, out of sight: nothing to show"), SourceStaysDrawn(false, false));
	TestFalse(TEXT("never remembered, in sight: gone"), SourceStaysDrawn(false, true));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLVisionGhostActorTest, "HeroesOfLiberty.Vision.GhostActor", UE::HeroesOfLiberty::Tests::Vision::Flags)
bool FHOLVisionGhostActorTest::RunTest(const FString& Parameters)
{
	// F7.3 (b), a client: the actor of a remembered building exists only out of sight and when the real one is not there.
	using namespace HOLVisionRules;
	TestTrue(TEXT("out of sight, the real one gone: an actor from the memory"), GhostActorWanted(false, false));
	TestFalse(TEXT("in sight: the server sends the real one, or it is gone"), GhostActorWanted(true, false));
	TestFalse(TEXT("out of sight but the real one still here: no double"), GhostActorWanted(false, true));
	TestFalse(TEXT("in sight and real"), GhostActorWanted(true, true));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLVisionFogModeConfigTest, "HeroesOfLiberty.Vision.FogModeConfig", UE::HeroesOfLiberty::Tests::Vision::Flags)
bool FHOLVisionFogModeConfigTest::RunTest(const FString& Parameters)
{
	// F8 (D522): the fog's mode travels with the match's configuration; hol.Fog.Mode >= 0 overrides it.
	using namespace HOLVisionRules;
	TestEqual(TEXT("console at -1: the configuration's"), ResolveFogMode(-1, EHOLFogMode::Explored), EHOLFogMode::Explored);
	TestEqual(TEXT("console at -1: the configuration's, Normal"), ResolveFogMode(-1, EHOLFogMode::Normal), EHOLFogMode::Normal);
	TestEqual(TEXT("console set: it wins"), ResolveFogMode(0, EHOLFogMode::Normal), EHOLFogMode::Disabled);
	TestEqual(TEXT("console set to Normal over a configuration Disabled"), ResolveFogMode(2, EHOLFogMode::Disabled), EHOLFogMode::Normal);
	TestEqual(TEXT("console beyond the range: clamped"), ResolveFogMode(9, EHOLFogMode::Disabled), EHOLFogMode::Normal);

	FHOLMatchConfig Config = FHOLMatchConfig::MakeDefault();
	TestEqual(TEXT("a configuration without the field: no fog (the scenarios, the tests)"), Config.FogMode, EHOLFogMode::Disabled);
	Config.FogMode = EHOLFogMode::Normal;
	FHOLMatchConfig Read;
	FString Error;
	TestTrue(TEXT("the configuration is read back"), FHOLMatchConfig::FromJson(Config.ToJson(), Read, &Error));
	TestEqual(TEXT("the fog survives the file"), Read.FogMode, EHOLFogMode::Normal);
	FHOLMatchConfig FromManager;
	TestTrue(TEXT("the manager's match.json"), FHOLMatchConfig::FromJson(TEXT("{\"version\": 3, \"map\": \"/Game/Maps/Plain.Plain\", \"fogMode\": \"Explored\"}"), FromManager, &Error));
	TestEqual(TEXT("... its fogMode by name"), FromManager.FogMode, EHOLFogMode::Explored);
	TestTrue(TEXT("a file without it"), FHOLMatchConfig::FromJson(TEXT("{\"version\": 3, \"map\": \"/Game/Maps/Plain.Plain\"}"), FromManager, &Error));
	TestEqual(TEXT("... is without fog"), FromManager.FogMode, EHOLFogMode::Disabled);

	FHOLLobby Lobby;
	TestTrue(TEXT("the lobby's state"), FHOLManagerClientRules::ParseLobby(TEXT("{\"id\": \"a\", \"name\": \"Salon\", \"state\": \"lobby\", \"slots\": [{\"index\": 0, \"state\": \"open\"}, {\"index\": 1, \"state\": \"open\"}, {\"index\": 2, \"state\": \"open\"}, {\"index\": 3, \"state\": \"open\"}, {\"index\": 4, \"state\": \"open\"}, {\"index\": 5, \"state\": \"open\"}, {\"index\": 6, \"state\": \"open\"}, {\"index\": 7, \"state\": \"open\"}], \"settings\": {\"map\": \"Plain\", \"fog\": \"Disabled\"}}"), Lobby));
	TestEqual(TEXT("the lobby's fog"), Lobby.GetFogMode(), EHOLFogMode::Disabled);
	FHOLLobby OlderManager;
	TestTrue(TEXT("an older manager's lobby"), FHOLManagerClientRules::ParseLobby(TEXT("{\"id\": \"a\", \"name\": \"Salon\", \"state\": \"lobby\", \"slots\": [{\"index\": 0, \"state\": \"open\"}, {\"index\": 1, \"state\": \"open\"}, {\"index\": 2, \"state\": \"open\"}, {\"index\": 3, \"state\": \"open\"}, {\"index\": 4, \"state\": \"open\"}, {\"index\": 5, \"state\": \"open\"}, {\"index\": 6, \"state\": \"open\"}, {\"index\": 7, \"state\": \"open\"}], \"settings\": {\"map\": \"Plain\"}}"), OlderManager));
	TestEqual(TEXT("... shows Normal"), OlderManager.GetFogMode(), EHOLFogMode::Normal);
	TestTrue(TEXT("the settings body carries the fog"), FHOLManagerClientRules::MakeSettingsBody(TEXT("Plain"), false, false, false, FString(), {}, TEXT("Explored")).Contains(TEXT("\"fog\":\"Explored\"")));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLVisionReceiversTest, "HeroesOfLiberty.Vision.Receivers", UE::HeroesOfLiberty::Tests::Vision::Flags)
bool FHOLVisionReceiversTest::RunTest(const FString& Parameters)
{
	// F6a (D518, D376): which camps' sight each connection receives.
	using namespace HOLVisionRules;
	TestEqual(TEXT("a player: its camp"), GetReceivedCamps(2, false, false), 1u << 2);
	TestEqual(TEXT("a player of a team: its camp"), GetReceivedCamps(1, false, true), 1u << 1);
	TestEqual(TEXT("a spectator: every camp"), GetReceivedCamps(INDEX_NONE, false, false), AllCamps);
	TestEqual(TEXT("defeated in a team: its team's"), GetReceivedCamps(1, true, true), 1u << 1);
	TestEqual(TEXT("defeated alone: every camp"), GetReceivedCamps(3, true, false), AllCamps);
	TestTrue(TEXT("seen by its camp: received"), Receives(GetReceivedCamps(2, false, false), (1u << 0) | (1u << 2)));
	TestFalse(TEXT("seen by others only: not received"), Receives(GetReceivedCamps(2, false, false), 1u << 0));
	TestFalse(TEXT("seen by nobody: not even a spectator"), Receives(AllCamps, 0u));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLVisionExploredRunsTest, "HeroesOfLiberty.Vision.ExploredRuns", UE::HeroesOfLiberty::Tests::Vision::Flags)
bool FHOLVisionExploredRunsTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::Vision;
	// F6e.3: what a camp explored goes to a player who comes back as run lengths, and is the same once read again.
	FHOLVisionMap Map;
	Map.Init(MakeGrid(), 2);
	const FHOLVisionGrid& Grid = Map.GetGrid();
	{
		const TArray<uint32> Nothing = Map.GetExploredRuns(0);
		TestEqual(TEXT("nothing explored: one run"), Nothing.Num(), 1);
		TestEqual(TEXT("nothing explored: all the grid"), Nothing.IsEmpty() ? 0 : static_cast<int32>(Nothing[0]), Grid.Num());
	}
	const int32 Scout = Map.AddObserver(0, Grid.GetCenter(FIntPoint(5, 5)), 800.);
	Map.MoveObserver(Scout, Grid.GetCenter(FIntPoint(12, 9)));
	Map.RemoveObserver(Scout);
	const TArray<uint32> Runs = Map.GetExploredRuns(0);
	int64 Total = 0;
	for (const uint32 Run : Runs)
	{
		Total += Run;
	}
	TestEqual(TEXT("the runs cover the grid"), Total, static_cast<int64>(Grid.Num()));
	TestTrue(TEXT("two discs: far fewer runs than cells"), !Runs.IsEmpty() && Runs.Num() < Grid.Num() / 4);

	FHOLVisionMap Other;
	Other.Init(MakeGrid(), 2);
	TestTrue(TEXT("read again"), Other.AddExploredRuns(0, Runs));
	TestEqual(TEXT("the same cells explored"), Other.CountExplored(0), Map.CountExplored(0));
	bool bSame = true;
	for (int32 Y = 0; Y < Grid.Side; ++Y)
	{
		for (int32 X = 0; X < Grid.Side; ++X)
		{
			bSame &= Other.GetVision(0, FIntPoint(X, Y)) == Map.GetVision(0, FIntPoint(X, Y));
		}
	}
	TestTrue(TEXT("cell by cell"), bSame);
	TestEqual(TEXT("the other camp untouched"), Other.CountExplored(1), 0);
	TestEqual(TEXT("explored, not in sight"), Other.CountVisible(0), 0);

	// Never takes back; a wrong total changes nothing.
	const TArray<uint32> Short = { 3, 4 };
	TestFalse(TEXT("a wrong total is refused"), Other.AddExploredRuns(0, Short));
	TestEqual(TEXT("and changes nothing"), Other.CountExplored(0), Map.CountExplored(0));
	const TArray<uint32> Blank = { static_cast<uint32>(Grid.Num()) };
	TestTrue(TEXT("an empty memory is read"), Other.AddExploredRuns(0, Blank));
	TestEqual(TEXT("and takes nothing back"), Other.CountExplored(0), Map.CountExplored(0));

	// All explored (D389): the first run of unexplored cells is 0.
	FHOLVisionMap Full;
	Full.Init(MakeGrid(), 1);
	Full.ExploreAll(0);
	const TArray<uint32> FullRuns = Full.GetExploredRuns(0);
	TestEqual(TEXT("all explored: two runs"), FullRuns.Num(), 2);
	TestEqual(TEXT("all explored: none unexplored first"), FullRuns.IsEmpty() ? 1 : static_cast<int32>(FullRuns[0]), 0);
	return true;
}

// F7 / F8: the fog's picture, the bytes the post process and the minimap read.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLVisionPictureTest, "HeroesOfLiberty.Vision.Picture", UE::HeroesOfLiberty::Tests::Vision::Flags)
bool FHOLVisionPictureTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::Vision;
	FHOLVisionMap Map;
	Map.Init(MakeGrid(), 2);
	const FHOLVisionGrid& Grid = Map.GetGrid();

	const int32 Scout = Map.AddObserver(0, Grid.GetCenter(FIntPoint(10, 10)), 800.);
	Map.MoveObserver(Scout, Grid.GetCenter(FIntPoint(14, 10)));

	TArray<uint8> Bytes;
	int32 NumVisible = -1;
	int32 NumExplored = -1;
	HOLFogPicture::Fill(Map, 0, Bytes, NumVisible, NumExplored);
	TestEqual(TEXT("one byte a cell"), Bytes.Num(), Grid.Num());
	TestEqual(TEXT("the cells in sight counted"), NumVisible, Map.CountVisible(0));
	TestEqual(TEXT("the cells ever seen counted"), NumExplored, Map.CountExplored(0));
	if (Bytes.Num() == Grid.Num())
	{
		TestEqual(TEXT("in sight: 255"), Bytes[Grid.ToIndex(FIntPoint(16, 10))], HOLFogPicture::VisibleByte);
		TestEqual(TEXT("left behind: explored"), Bytes[Grid.ToIndex(FIntPoint(8, 10))], HOLFogPicture::ExploredByte);
		TestEqual(TEXT("never seen: 0"), Bytes[Grid.ToIndex(FIntPoint(0, 0))], HOLFogPicture::UnexploredByte);
		// A row of the picture is one Y: a cell high up the grid is not read for one on its right.
		TestEqual(TEXT("(10, 17) never seen: 0"), Bytes[Grid.ToIndex(FIntPoint(10, 17))], HOLFogPicture::UnexploredByte);
	}

	TestEqual(TEXT("read in sight (a point of the cell)"), HOLFogPicture::Read(Grid, Bytes, Grid.GetCenter(FIntPoint(16, 10)) + FVector2D(120., -80.)), EHOLCellVision::Visible);
	TestEqual(TEXT("read explored"), HOLFogPicture::Read(Grid, Bytes, Grid.GetCenter(FIntPoint(8, 10))), EHOLCellVision::Explored);
	TestEqual(TEXT("read black"), HOLFogPicture::Read(Grid, Bytes, Grid.GetCenter(FIntPoint(1, 18))), EHOLCellVision::Unexplored);
	TestEqual(TEXT("read outside the grid: black"), HOLFogPicture::Read(Grid, Bytes, FVector2D(99999., 0.)), EHOLCellVision::Unexplored);
	TestEqual(TEXT("read agrees with the map"), HOLFogPicture::Read(Grid, Bytes, Grid.GetCenter(FIntPoint(12, 9))), Map.GetVision(0, FIntPoint(12, 9)));

	// The smooth read (the minimap's terrain at 480 px, soft edges): the cells' bytes interpolated bilinearly between the cells' centres, 0..1.
	if (Bytes.Num() == Grid.Num())
	{
		const float VisibleValue = Bytes[Grid.ToIndex(FIntPoint(16, 10))] / 255.f;
		const float NextValue = Bytes[Grid.ToIndex(FIntPoint(17, 10))] / 255.f;
		TestTrue(TEXT("smooth: the cells (16, 10) and (17, 10) differ (the test's premise)"), !FMath::IsNearlyEqual(VisibleValue, NextValue, 0.05f));
		TestTrue(TEXT("smooth: a cell's centre is its byte"), FMath::IsNearlyEqual(HOLFogPicture::ReadSmooth(Grid, Bytes, Grid.GetCenter(FIntPoint(16, 10))), VisibleValue, 0.001f));
		const FVector2D Between = (Grid.GetCenter(FIntPoint(16, 10)) + Grid.GetCenter(FIntPoint(17, 10))) * 0.5;
		const float Mid = HOLFogPicture::ReadSmooth(Grid, Bytes, Between);
		TestTrue(TEXT("smooth: halfway between two cells' centres is the mean of their bytes"), FMath::IsNearlyEqual(Mid, 0.5f * (VisibleValue + NextValue), 0.01f));
		const float Quarter = HOLFogPicture::ReadSmooth(Grid, Bytes, Grid.GetCenter(FIntPoint(16, 10)) * 0.75 + Grid.GetCenter(FIntPoint(17, 10)) * 0.25);
		TestTrue(TEXT("smooth: a quarter of the way is a quarter of the step"), FMath::IsNearlyEqual(Quarter, 0.75f * VisibleValue + 0.25f * NextValue, 0.01f));
		TestTrue(TEXT("smooth: outside the grid is black"), HOLFogPicture::ReadSmooth(Grid, Bytes, FVector2D(99999., 0.)) < 0.001f);
		TestTrue(TEXT("smooth: no bytes is black"), HOLFogPicture::ReadSmooth(Grid, TConstArrayView<uint8>(), FVector2D::ZeroVector) < 0.001f);
	}

	// The blur: a lone cell in sight spreads over its eight neighbours (4, 2 and 1 sixteenths), a uniform field does not change, the corner repeats itself.
	{
		TArray<uint8> Lone;
		Lone.Init(HOLFogPicture::UnexploredByte, Grid.Num());
		Lone[Grid.ToIndex(FIntPoint(10, 10))] = HOLFogPicture::VisibleByte;
		TArray<uint8> Soft;
		HOLFogPicture::Blur(Grid, Lone, Soft);
		TestEqual(TEXT("blur: one byte a cell"), Soft.Num(), Grid.Num());
		if (Soft.Num() == Grid.Num())
		{
			TestEqual(TEXT("blur: the lone cell keeps 4/16"), static_cast<int32>(Soft[Grid.ToIndex(FIntPoint(10, 10))]), 64);
			TestEqual(TEXT("blur: a side neighbour gets 2/16"), static_cast<int32>(Soft[Grid.ToIndex(FIntPoint(11, 10))]), 32);
			TestEqual(TEXT("blur: a diagonal neighbour gets 1/16"), static_cast<int32>(Soft[Grid.ToIndex(FIntPoint(9, 11))]), 16);
			TestEqual(TEXT("blur: two cells away gets nothing"), static_cast<int32>(Soft[Grid.ToIndex(FIntPoint(12, 10))]), 0);
		}
		TArray<uint8> Flat;
		Flat.Init(HOLFogPicture::ExploredByte, Grid.Num());
		TArray<uint8> FlatSoft;
		HOLFogPicture::Blur(Grid, Flat, FlatSoft);
		TestEqual(TEXT("blur: a uniform field (corner and middle) stays itself"),
			FlatSoft.Num() == Grid.Num() ? static_cast<int32>(FlatSoft[0]) * 1000 + FlatSoft[Grid.ToIndex(FIntPoint(5, 5))] : -1,
			static_cast<int32>(HOLFogPicture::ExploredByte) * 1000 + HOLFogPicture::ExploredByte);
		TArray<uint8> Nothing;
		HOLFogPicture::Blur(Grid, TConstArrayView<uint8>(), Nothing);
		TestTrue(TEXT("blur: no bytes, no bytes"), Nothing.IsEmpty());
	}

	for (const EHOLCellVision Vision : { EHOLCellVision::Unexplored, EHOLCellVision::Explored, EHOLCellVision::Visible })
	{
		TestEqual(TEXT("a byte comes back to its state"), HOLFogPicture::FromByte(HOLFogPicture::ToByte(Vision)), Vision);
	}
	TestEqual(TEXT("a filtered byte: nearly in sight"), HOLFogPicture::FromByte(230), EHOLCellVision::Visible);
	TestEqual(TEXT("a filtered byte: nearly black"), HOLFogPicture::FromByte(30), EHOLCellVision::Unexplored);

	const double Half = 0.5 * Grid.Side * Grid.CellSize;
	TestTrue(TEXT("the lower corner is (0, 0)"), HOLFogPicture::ToUV(Grid, FVector2D(-Half, -Half)).Equals(FVector2D(0., 0.), 1e-6));
	TestTrue(TEXT("the centre is (0.5, 0.5)"), HOLFogPicture::ToUV(Grid, FVector2D::ZeroVector).Equals(FVector2D(0.5, 0.5), 1e-6));
	TestTrue(TEXT("the upper corner is (1, 1)"), HOLFogPicture::ToUV(Grid, FVector2D(Half, Half)).Equals(FVector2D(1., 1.), 1e-6));

	TArray<uint8> Other;
	HOLFogPicture::Fill(Map, 1, Other, NumVisible, NumExplored);
	TestEqual(TEXT("the other camp: nothing in sight"), NumVisible, 0);
	TestEqual(TEXT("the other camp: nothing seen"), NumExplored, 0);
	TestEqual(TEXT("the other camp: black"), Other.Num() == Grid.Num() ? Other[Grid.ToIndex(FIntPoint(16, 10))] : uint8(7), HOLFogPicture::UnexploredByte);

	TArray<uint8> NoFog;
	HOLFogPicture::Fill(Map, INDEX_NONE, NoFog, NumVisible, NumExplored);
	TestEqual(TEXT("without fog: everything in sight"), NumVisible, Grid.Num());
	TestEqual(TEXT("without fog: everything seen"), NumExplored, Grid.Num());
	TestEqual(TEXT("without fog: 255 everywhere"), NoFog.Num() == Grid.Num() ? NoFog[Grid.ToIndex(FIntPoint(0, 0))] : uint8(7), HOLFogPicture::VisibleByte);
	return true;
}

#endif
