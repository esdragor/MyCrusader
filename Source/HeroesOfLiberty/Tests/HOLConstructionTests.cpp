// Copyright Epic Games, Inc. All Rights Reserved.

#include "Buildings/HOLBuildingTypes.h"
#include "Buildings/HOLConstructionRules.h"
#include "Buildings/HOLProductionRules.h"
#include "Misc/AutomationTest.h"
#include "Resources/HOLSourceTypes.h"
#include "Simulation/HOLFormationRules.h"
#include "Simulation/HOLSimulationFragments.h"
#include "Simulation/HOLUnitTypes.h"

#if WITH_DEV_AUTOMATION_TESTS

// J6a.2 checks (D314): the pure rules of construction (D311, D312) and the values of the building type table (D310, D313).
namespace UE::HeroesOfLiberty::Tests::Construction
{
	constexpr EAutomationTestFlags Flags = EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter;

	/** A tile for the pure rules below, which take it as an argument: 3 m (the setting, UHOLBuildingSettings::TileSize, is 4 m since D344). */
	constexpr double TestTile = 300.0;

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

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLConstructionTablesTest, "HeroesOfLiberty.Construction.Tables", UE::HeroesOfLiberty::Tests::Construction::Flags)
bool FHOLConstructionTablesTest::RunTest(const FString& Parameters)
{
	const UHOLBuildingSettings* Settings = GetDefault<UHOLBuildingSettings>();
	const double Grid = UHOLBuildingSettings::GetTileSize();
	TestEqual(TEXT("4 m tiles (D311, D344)"), Grid, 400.0);
	TestTrue(TEXT("anti-rush radius set (D22)"), Settings->AntiRushRadius > 0.f);
	TestTrue(TEXT("3 builders ~2x as fast (D312)"),
		FMath::Abs(HOLConstruction::GetWorkPerStep(3, Settings->BuilderExponent) / float(HOLConstruction::WorkPerBuilder) - 2.f) < 0.05f);

	const FHOLBuildingTypeRow* MainCentre = UHOLBuildingSettings::FindType(HOLBuildingTypeNames::MainCentre);
	if (TestNotNull(TEXT("main centre"), MainCentre))
	{
		TestFalse(TEXT("the main centre is not built by Peasants"), MainCentre->IsBuildable());
		TestEqual(TEXT("the main centre: 4 x 4 tiles (16 m)"), MainCentre->Footprint, FVector2D(4.0 * Grid, 4.0 * Grid));
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
		{ HOLBuildingTypeNames::Mill, 3, 0, { EHOLResource::Food }, 100 },
	};
	for (const FExpected& Type : Expected)
	{
		const FHOLBuildingTypeRow* Row = UHOLBuildingSettings::FindType(Type.Name);
		const FString Name = Type.Name.ToString();
		if (!TestNotNull(Name + TEXT(" in the building type table"), Row))
		{
			continue;
		}
		TestEqual(Name + TEXT(": slot in the Peasant's grid (D313, age I, economic row: A, Z, E, R)"), Row->BuildGridSlot, Type.Slot);
		TestEqual(Name + TEXT(": population given"), Row->PopulationProvided, Type.Population);
		TestTrue(Name + TEXT(": resources accepted"), Row->Accepts == Type.Accepts);
		TestTrue(Name + TEXT(": its cost, in wood only"), Row->Cost == FHOLResources(0, Type.Wood, 0, 0));
		TestTrue(Name + TEXT(": a build time"), Row->BuildTime > 0.f);
		TestEqual(Name + TEXT(": 2 x 2 tiles (6 m)"), Row->Footprint, FVector2D(2.0 * Grid, 2.0 * Grid));
		TestTrue(Name + TEXT(": smaller than the main centre"), MainCentre && Row->Footprint.X < MainCentre->Footprint.X);
	}

	// J7a.1 (J7-Q2): the Barracks, built like a house; J7a.2: on R of the Peasant's grid (no military page, the pages
	// come later by age).
	const FHOLBuildingTypeRow* Barracks = UHOLBuildingSettings::FindType(HOLBuildingTypeNames::Barracks);
	if (TestNotNull(TEXT("Barracks in the building type table"), Barracks))
	{
		TestTrue(TEXT("Barracks: built by Peasants"), Barracks->IsBuildable());
		TestEqual(TEXT("Barracks: Q of the Peasant's grid (slot 4, military row, D313)"), Barracks->BuildGridSlot, 4);
		TestEqual(TEXT("Barracks: 3 x 3 tiles (12 m)"), Barracks->Footprint, FVector2D(3.0 * Grid, 3.0 * Grid));
		TestTrue(TEXT("Barracks: 150 wood"), Barracks->Cost == FHOLResources(0, 150, 0, 0));
		TestEqual(TEXT("Barracks: 20 s for one builder"), Barracks->BuildTime, 20.f);
		TestEqual(TEXT("Barracks: 1 500 health"), Barracks->MaxHealth, 1500);
		TestEqual(TEXT("Barracks: 6 m high"), Barracks->Height, 600.f);
	}
	// AR-Q2a: the Archery range, built like the Barracks, on S of the Peasant's grid (right of the Barracks), no prerequisite.
	const FHOLBuildingTypeRow* ArcheryRange = UHOLBuildingSettings::FindType(HOLBuildingTypeNames::ArcheryRange);
	if (TestNotNull(TEXT("Archery range in the building type table"), ArcheryRange))
	{
		TestTrue(TEXT("Archery range: built by Peasants"), ArcheryRange->IsBuildable());
		TestEqual(TEXT("Archery range: S of the Peasant's grid (slot 5, military row)"), ArcheryRange->BuildGridSlot, 5);
		TestEqual(TEXT("Archery range: 3 x 3 tiles (12 m)"), ArcheryRange->Footprint, FVector2D(3.0 * Grid, 3.0 * Grid));
		TestEqual(TEXT("Archery range: age I"), ArcheryRange->Age, 0);
	}

	// SC-Q1 = B (D602): the Sanctuary of the Dawn, technological, age I, built by Peasants on X of page I (the economic row's overflow, after
	// the Farm): 250 wood, 3 x 3 tiles (12 m), 30 s, 1 500 health, no production. The +15 % of food on the farms within 30 m is a second step.
	const FHOLBuildingTypeRow* Sanctuary = UHOLBuildingSettings::FindType(TEXT("Sanctuary"));
	if (TestNotNull(TEXT("Sanctuary in the building type table"), Sanctuary))
	{
		TestTrue(TEXT("Sanctuary: built by Peasants"), Sanctuary->IsBuildable());
		TestEqual(TEXT("Sanctuary: X of the Peasant's grid (slot 9, after the Farm)"), Sanctuary->BuildGridSlot, 9);
		TestEqual(TEXT("Sanctuary: age I"), Sanctuary->Age, 0);
		TestEqual(TEXT("Sanctuary: technological (it researches the Dawn's technologies; the W X C V row of the grid)"), int32(Sanctuary->Category), int32(EHOLBuildingCategory::Technological));
		TestEqual(TEXT("Sanctuary: 3 x 3 tiles (12 m)"), Sanctuary->Footprint, FVector2D(3.0 * Grid, 3.0 * Grid));
		TestTrue(TEXT("Sanctuary: 250 wood"), Sanctuary->Cost == FHOLResources(0, 250, 0, 0));
		TestEqual(TEXT("Sanctuary: 30 s for one builder"), Sanctuary->BuildTime, 30.f);
		TestEqual(TEXT("Sanctuary: 1 500 health"), Sanctuary->MaxHealth, 1500);
		TestEqual(TEXT("Sanctuary: no population"), Sanctuary->PopulationProvided, 0);
		TestTrue(TEXT("Sanctuary: produces nothing"), Sanctuary->Produces.IsEmpty());
		TestTrue(TEXT("Sanctuary: takes no resource"), Sanctuary->Accepts.IsEmpty());
		TestFalse(TEXT("Sanctuary: not walkable"), Sanctuary->Walkable);
	}

	// Q-chapelle-moine = A: the Chapel of the Dawn (GDD D211/D212, palier 1 = page II), military (it forms the Monk), built by Peasants on S of
	// page II (right of the siege workshop): 200 wood, 3 x 3 tiles (12 m), 30 s, 1 500 health; it produces the Monk and takes no resource.
	const FHOLBuildingTypeRow* Chapel = UHOLBuildingSettings::FindType(TEXT("Chapel"));
	if (TestNotNull(TEXT("Chapel in the building type table"), Chapel))
	{
		TestTrue(TEXT("Chapel: built by Peasants"), Chapel->IsBuildable());
		TestEqual(TEXT("Chapel: S of the Peasant's grid (slot 5, military row)"), Chapel->BuildGridSlot, 5);
		TestEqual(TEXT("Chapel: age II (page II)"), Chapel->Age, 1);
		TestEqual(TEXT("Chapel: military (it forms the Monk)"), int32(Chapel->Category), int32(EHOLBuildingCategory::Military));
		TestEqual(TEXT("Chapel: 3 x 3 tiles (12 m)"), Chapel->Footprint, FVector2D(3.0 * Grid, 3.0 * Grid));
		TestTrue(TEXT("Chapel: 200 wood"), Chapel->Cost == FHOLResources(0, 200, 0, 0));
		TestEqual(TEXT("Chapel: 30 s for one builder"), Chapel->BuildTime, 30.f);
		TestEqual(TEXT("Chapel: 1 500 health"), Chapel->MaxHealth, 1500);
		TestEqual(TEXT("Chapel: no population"), Chapel->PopulationProvided, 0);
		TestTrue(TEXT("Chapel: produces the Monk"), Chapel->Produces == TArray<FName>({ FName(TEXT("Monk")) }));
		TestTrue(TEXT("Chapel: takes no resource"), Chapel->Accepts.IsEmpty());
		TestFalse(TEXT("Chapel: not walkable"), Chapel->Walkable);
	}

	// M3 (GDD 5.5, 11.1, D569): the Market, economic, age II (palier 1, Essor), built by Peasants on A of page II. Its size, cost
	// and health are provisional (MarketInProgress.md, question MK-Q2): 3 x 3 tiles like the Barracks, 150 wood, 20 s.
	const FHOLBuildingTypeRow* Market = UHOLBuildingSettings::FindType(HOLBuildingTypeNames::Market);
	if (TestNotNull(TEXT("Market in the building type table"), Market))
	{
		TestTrue(TEXT("Market: built by Peasants"), Market->IsBuildable());
		TestEqual(TEXT("Market: A of the Peasant's grid (slot 0, economic row)"), Market->BuildGridSlot, 0);
		TestEqual(TEXT("Market: age II"), Market->Age, 1);
		TestEqual(TEXT("Market: economic"), int32(Market->Category), int32(EHOLBuildingCategory::Economic));
		TestEqual(TEXT("Market: 3 x 3 tiles"), Market->Footprint, FVector2D(3.0 * Grid, 3.0 * Grid));
		TestTrue(TEXT("Market: costs wood only"), Market->Cost.Food == 0 && Market->Cost.Stone == 0 && Market->Cost.Gold == 0 && Market->Cost.Wood > 0);
		TestTrue(TEXT("Market: a build time"), Market->BuildTime > 0.f);
		TestEqual(TEXT("Market: no population"), Market->PopulationProvided, 0);
		TestTrue(TEXT("Market: produces nothing"), Market->Produces.IsEmpty());
		TestTrue(TEXT("Market: takes no resource (it is not a deposit)"), Market->Accepts.IsEmpty());
		TestFalse(TEXT("Market: not walkable"), Market->Walkable);
		TestTrue(TEXT("Market: no source of its own"), Market->Source.IsNone());
		TestTrue(TEXT("Market: sees at least as far as a house"), Market->SightRadius >= 2000.f);
	}

	// IA1.2 (F4): every building so far blocks the way and has no source of its own.
	for (const FName Name : { HOLBuildingTypeNames::MainCentre, HOLBuildingTypeNames::House, HOLBuildingTypeNames::LumberCamp,
		HOLBuildingTypeNames::MiningCamp, HOLBuildingTypeNames::Mill, HOLBuildingTypeNames::Barracks,
		HOLBuildingTypeNames::ArcheryRange })
	{
		const FHOLBuildingTypeRow* Row = UHOLBuildingSettings::FindType(Name);
		if (TestNotNull(Name.ToString() + TEXT(" in the building type table"), Row))
		{
			TestFalse(Name.ToString() + TEXT(": not walkable"), Row->Walkable);
			TestTrue(Name.ToString() + TEXT(": no source of its own"), Row->Source.IsNone());
		}
	}

	// IA1.2 (F4): the Farm, 2 x 2 like the Mill, 75 wood, 15 s, on W of the Peasant's grid (IA1.2-Q1), walked over,
	// born with its FarmField (a linked source of the source type table); no deposit, no population.
	const FHOLBuildingTypeRow* Farm = UHOLBuildingSettings::FindType(HOLBuildingTypeNames::Farm);
	if (TestNotNull(TEXT("Farm in the building type table"), Farm))
	{
		TestTrue(TEXT("Farm: built by Peasants"), Farm->IsBuildable());
		TestEqual(TEXT("Farm: W of the Peasant's grid (slot 8, IA1.2-Q1)"), Farm->BuildGridSlot, 8);
		TestEqual(TEXT("Farm: age I"), Farm->Age, 0);
		TestEqual(TEXT("Farm: economic"), int32(Farm->Category), int32(EHOLBuildingCategory::Economic));
		TestEqual(TEXT("Farm: 2 x 2 tiles (8 m)"), Farm->Footprint, FVector2D(2.0 * Grid, 2.0 * Grid));
		TestTrue(TEXT("Farm: 75 wood"), Farm->Cost == FHOLResources(0, 75, 0, 0));
		TestEqual(TEXT("Farm: 7.5 s for one builder"), Farm->BuildTime, 7.5f);
		TestEqual(TEXT("Farm: no population"), Farm->PopulationProvided, 0);
		TestTrue(TEXT("Farm: takes no resource (no deposit)"), Farm->Accepts.IsEmpty());
		TestTrue(TEXT("Farm: produces nothing"), Farm->Produces.IsEmpty());
		TestTrue(TEXT("Farm: walkable"), Farm->Walkable);
		TestEqual(TEXT("Farm: its source is a FarmField"), Farm->Source, HOLSourceTypeNames::FarmField);
		const FHOLSourceTypeRow* Field = UHOLSourceSettings::FindType(Farm->Source);
		if (TestNotNull(TEXT("Farm: its source in the source type table"), Field))
		{
			TestTrue(TEXT("Farm: its source is linked"), Field->Linked);
			TestEqual(TEXT("Farm: its source gives food"), int32(Field->Resource), int32(EHOLResource::Food));
		}
	}
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLConstructionCampSourceTest, "HeroesOfLiberty.Construction.CampSource", UE::HeroesOfLiberty::Tests::Construction::Flags)
bool FHOLConstructionCampSourceTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::Construction;
	// J6b.1 (D312): a camp finished at (0, 0) m, 6 m wide; its builders gather the nearest source it accepts within 25 m of it.
	TestTrue(TEXT("a lumber camp sends its builders to gather"), HOLConstruction::SendsBuildersToGather(HOLBuildingTypeNames::LumberCamp, TArray<EHOLResource>({ EHOLResource::Wood })));
	TestTrue(TEXT("a mining camp too"), HOLConstruction::SendsBuildersToGather(HOLBuildingTypeNames::MiningCamp, TArray<EHOLResource>({ EHOLResource::Stone, EHOLResource::Gold })));
	TestTrue(TEXT("a mill too (D346, the food camp)"), HOLConstruction::SendsBuildersToGather(HOLBuildingTypeNames::Mill, TArray<EHOLResource>({ EHOLResource::Food })));
	TestFalse(TEXT("a house does not (no deposit)"), HOLConstruction::SendsBuildersToGather(HOLBuildingTypeNames::House, TArray<EHOLResource>()));
	TestFalse(TEXT("a main centre does not (the four resources)"), HOLConstruction::SendsBuildersToGather(HOLBuildingTypeNames::MainCentre,
		TArray<EHOLResource>({ EHOLResource::Food, EHOLResource::Wood, EHOLResource::Stone, EHOLResource::Gold })));

	const FBox2D Camp = House(0.0, 0.0);
	TestEqual(TEXT("gap to a tree's edge off a side (8 m from the middle, 60 cm of body)"), HOLConstruction::GetSourceGap(Camp, FVector2D(800.0, 0.0), 60.0), 440.0, 0.01);
	TestEqual(TEXT("gap to a vein off a corner (3 m, 4 m; 3 m of body)"), HOLConstruction::GetSourceGap(Camp, FVector2D(600.0, 700.0), 300.0), 200.0, 0.01);
	TestEqual(TEXT("a body over the footprint: 0"), HOLConstruction::GetSourceGap(Camp, FVector2D(350.0, 0.0), 60.0), 0.0, 0.01);

	const auto Source = [](const double X, const double Y, const double Radius, const EHOLResource Resource, const bool bStanding = true)
	{
		return FHOLCampSource{ FVector2D(X, Y) * 100.0, Radius * 100.0, Resource, bStanding };
	};
	const TArray<FHOLCampSource> Sources = {
		Source(-6.0, 0.0, 1.1, EHOLResource::Food),          // berries 1.9 m off: not wood
		Source(10.0, 0.0, 0.6, EHOLResource::Wood),          // 6.4 m
		Source(0.0, 8.0, 0.6, EHOLResource::Wood, false),    // 4.4 m, used up
		Source(0.0, -9.0, 0.6, EHOLResource::Wood),          // 5.4 m: the nearest standing tree
		Source(16.0, 16.0, 3.0, EHOLResource::Gold),         // 15.4 m
		Source(-20.0, 0.0, 3.0, EHOLResource::Stone),        // 14 m
		Source(40.0, 0.0, 0.6, EHOLResource::Wood),          // 36.4 m
	};
	const TArray<EHOLResource> Wood = { EHOLResource::Wood };
	const TArray<EHOLResource> Mining = { EHOLResource::Stone, EHOLResource::Gold };
	double Gap = -1.0;
	TestEqual(TEXT("lumber camp: the nearest standing tree, not the berries nearer, nor the stump"), HOLConstruction::FindCampSource(Camp, Wood, Sources, 2500.0, &Gap), 3);
	TestEqual(TEXT("its gap"), Gap, 540.0, 0.01);
	TestEqual(TEXT("mining camp: the nearer of stone and gold"), HOLConstruction::FindCampSource(Camp, Mining, Sources, 2500.0, &Gap), 5);
	TestEqual(TEXT("its gap (to the vein's edge)"), Gap, 1400.0, 0.01);
	TestEqual(TEXT("mill (D346): the berries, not the trees"), HOLConstruction::FindCampSource(Camp, TArray<EHOLResource>({ EHOLResource::Food }), Sources, 2500.0, &Gap), 0);
	TestEqual(TEXT("its gap (to the bush's edge)"), Gap, 190.0, 0.01);
	TestEqual(TEXT("mill with no bush left: none, never a tree"), HOLConstruction::FindCampSource(Camp, TArray<EHOLResource>({ EHOLResource::Food }),
		TArray<FHOLCampSource>({ Source(-6.0, 0.0, 1.1, EHOLResource::Food, false), Source(10.0, 0.0, 0.6, EHOLResource::Wood) }), 2500.0), INDEX_NONE);
	TestEqual(TEXT("mining camp within 10 m: none"), HOLConstruction::FindCampSource(Camp, Mining, Sources, 1000.0), INDEX_NONE);
	TestEqual(TEXT("exactly at the range: still near"), HOLConstruction::FindCampSource(Camp, Wood, Sources, 540.0), 3);
	TestEqual(TEXT("only the far tree standing: none within 25 m, they go on as after a house"),
		HOLConstruction::FindCampSource(Camp, Wood, TArray<FHOLCampSource>({ Source(40.0, 0.0, 0.6, EHOLResource::Wood) }), 2500.0), INDEX_NONE);
	TestEqual(TEXT("two as near: the first"), HOLConstruction::FindCampSource(Camp, Wood,
		TArray<FHOLCampSource>({ Source(10.0, 0.0, 0.6, EHOLResource::Wood), Source(-10.0, 0.0, 0.6, EHOLResource::Wood) }), 2500.0), 0);
	TestEqual(TEXT("a camp accepting nothing: none"), HOLConstruction::FindCampSource(Camp, TArray<EHOLResource>(), Sources, 2500.0), INDEX_NONE);
	TestEqual(TEXT("no source: none"), HOLConstruction::FindCampSource(Camp, Wood, TArray<FHOLCampSource>(), 2500.0), INDEX_NONE);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLConstructionIdleWallTest, "HeroesOfLiberty.Construction.IdleWall", UE::HeroesOfLiberty::Tests::Construction::Flags)
bool FHOLConstructionIdleWallTest::RunTest(const FString& Parameters)
{
	// EtapeDefini C.1 (trial of 2026-10-23): a Spearman standing still, jostled by builders, stayed half in a visible wall.
	// A unit standing still goes wholly beyond it, a step at a time; a walker or a worker keeps D324's shoulder.
	const FVector2D Half = HOLConstruction::GetBodySize(FVector2D(800.0, 800.0), 115.0) * 0.5;
	const double VisualGap = 115.0 - 30.0;
	const double Radius = 60.0;
	const double InsideGap = HOLConstruction::GetKeepOutInsideGap(Radius, 25.0, VisualGap);
	const double OutsideGap = Radius + 1.0;
	const double MaxStep = 10.0;
	const FBox2D Body(-Half, Half);
	TestTrue(TEXT("idle, no order, not fighting: stands still"),
		HOLConstruction::StandsClearOfVisibleWall(EHOLUnitActivity::Idle, EHOLMovePhase::None, false));
	TestFalse(TEXT("walking: shoulder"), HOLConstruction::StandsClearOfVisibleWall(EHOLUnitActivity::Moving, EHOLMovePhase::Path, false));
	TestFalse(TEXT("idle but given a path: shoulder"), HOLConstruction::StandsClearOfVisibleWall(EHOLUnitActivity::Idle, EHOLMovePhase::WaitingPath, false));
	TestFalse(TEXT("building: shoulder"), HOLConstruction::StandsClearOfVisibleWall(EHOLUnitActivity::Building, EHOLMovePhase::None, false));
	TestFalse(TEXT("gathering: shoulder"), HOLConstruction::StandsClearOfVisibleWall(EHOLUnitActivity::Gathering, EHOLMovePhase::None, false));
	TestFalse(TEXT("at a deposit: shoulder"), HOLConstruction::StandsClearOfVisibleWall(EHOLUnitActivity::GatherTrip, EHOLMovePhase::None, false));
	TestFalse(TEXT("striking: shoulder"), HOLConstruction::StandsClearOfVisibleWall(EHOLUnitActivity::Attacking, EHOLMovePhase::None, false));
	TestFalse(TEXT("idle with a target: shoulder"), HOLConstruction::StandsClearOfVisibleWall(EHOLUnitActivity::Idle, EHOLMovePhase::None, true));

	// Its middle on the wall's line: half its body in the wall.
	const FVector2D Start(Half.X + VisualGap, 40.0);
	FVector2D Walker = Start;
	TestFalse(TEXT("a walker there: as it is"), HOLConstruction::KeepOutOfVisibleWall(Half, InsideGap, OutsideGap, Radius, VisualGap, false, MaxStep, Walker));
	TestTrue(TEXT("a walker there: not moved"), Walker.Equals(Start, 0.01));

	// Standing still, holding its formation place where it stands: the place goes out with it, it never walks back in.
	FVector2D Unit = Start;
	FVector2D Place = Start;
	HOLFormation::FPlaceHold Hold;
	HOLFormation::EndWalkToPlace(Hold, Unit, Place, Radius);
	int32 Steps = 0;
	bool bWalkedBack = false;
	double LongestStep = 0.0;
	for (int32 Step = 0; Step < 100; ++Step)
	{
		const FVector2D Before = Unit;
		Steps += HOLConstruction::KeepOutOfVisibleWall(Half, InsideGap, OutsideGap, Radius, VisualGap, true, MaxStep, Unit) ? 1 : 0;
		HOLConstruction::KeepOutOfVisibleWall(Half, InsideGap, OutsideGap, Radius, VisualGap, true, MaxStep, Place);
		LongestStep = FMath::Max(LongestStep, FVector2D::Distance(Before, Unit));
		bWalkedBack |= HOLFormation::WatchPlace(Hold, Unit, Place, Radius);
	}
	const double FromBody = HOLConstruction::GetDistance(Body, Unit);
	TestTrue(FString::Printf(TEXT("wholly beyond the visible wall: %.1f cm >= %.1f"), FromBody, VisualGap + Radius), FromBody >= VisualGap + Radius);
	TestTrue(FString::Printf(TEXT("a step at a time: %d steps, %.1f cm at most"), Steps, LongestStep), Steps > 1 && LongestStep <= MaxStep + 0.01);
	TestTrue(FString::Printf(TEXT("its place out with it: %.1f cm from the body"), HOLConstruction::GetDistance(Body, Place)),
		HOLConstruction::GetDistance(Body, Place) >= VisualGap + Radius);
	TestFalse(TEXT("no walk back into the wall"), bWalkedBack);
	FVector2D Again = Unit;
	TestFalse(TEXT("out, it stays"), HOLConstruction::KeepOutOfVisibleWall(Half, InsideGap, OutsideGap, Radius, VisualGap, true, MaxStep, Again));

	// C.2: the easing out of the visible wall is told apart from a body put back (the guard's count of walkers walking in).
	FVector2D Eased = Start;
	bool bEased = false;
	TestTrue(TEXT("in the visible wall: eased out"),
		HOLConstruction::KeepOutOfVisibleWall(Half, InsideGap, OutsideGap, Radius, VisualGap, true, MaxStep, Eased, &bEased) && bEased);
	FVector2D Touching(Half.X + Radius * 0.5, 0.0);
	bool bTouchingEased = true;
	TestTrue(TEXT("its body in the building's: put back, not eased"),
		HOLConstruction::KeepOutOfVisibleWall(Half, InsideGap, OutsideGap, Radius, VisualGap, true, MaxStep, Touching, &bTouchingEased) && !bTouchingEased);
	FVector2D Inside(Half.X - 10.0, 0.0);
	bool bInsideEased = true;
	TestTrue(TEXT("its centre inside: put back, not eased"),
		HOLConstruction::KeepOutOfVisibleWall(Half, InsideGap, OutsideGap, Radius, VisualGap, true, MaxStep, Inside, &bInsideEased) && !bInsideEased);
	bool bAgainEased = true;
	TestFalse(TEXT("out: not moved, not eased"),
		HOLConstruction::KeepOutOfVisibleWall(Half, InsideGap, OutsideGap, Radius, VisualGap, true, MaxStep, Again, &bAgainEased) || bAgainEased);

	// A builder there: as it is (the shoulder; the unit's state says it does not stand still).
	FVector2D Builder = Start;
	const bool bBuilderStill = HOLConstruction::StandsClearOfVisibleWall(EHOLUnitActivity::Building, EHOLMovePhase::None, false);
	TestFalse(TEXT("a builder there: as it is"),
		HOLConstruction::KeepOutOfVisibleWall(Half, InsideGap, OutsideGap, Radius, VisualGap, bBuilderStill, MaxStep, Builder));

	// C.3 (trial of 2026-10-23): two houses side by side (tiles edge to edge, bodies 230 cm apart, narrower than both walls
	// and a body): a unit standing still in the passage cannot be out of both, it stays (D324's shoulder), nothing eased.
	HOLConstruction::FWallBody Left;
	Left.HalfExtent = Half;
	HOLConstruction::FWallBody Right = Left;
	Right.Centre = FVector2D(800.0, 0.0);
	const HOLConstruction::FWallBody Pair[] = { Left, Right };
	for (const FVector2D& InPassage : { FVector2D(400.0, 40.0), FVector2D(360.0, -100.0), FVector2D(430.0, Half.Y - 20.0) })
	{
		FVector2D Wedged = InPassage;
		int32 EasedSteps = 0;
		double Farthest = 0.0;
		for (int32 Step = 0; Step < 100; ++Step)
		{
			EasedSteps += HOLConstruction::EaseOutOfVisibleWalls(Pair, Radius, VisualGap, MaxStep, Wedged) ? 1 : 0;
			Farthest = FMath::Max(Farthest, FVector2D::Distance(InPassage, Wedged));
		}
		TestTrue(FString::Printf(TEXT("in the passage at (%.0f, %.0f): stays, %d steps eased, %.1f cm moved at most"), InPassage.X, InPassage.Y,
			EasedSteps, Farthest), EasedSteps == 0 && Farthest < 0.01);
	}
	// Near one of the two only (the far side of the left house): eased out as before, a step at a time.
	FVector2D Alone(-Half.X - VisualGap, 40.0);
	int32 AloneSteps = 0;
	for (int32 Step = 0; Step < 100; ++Step)
	{
		AloneSteps += HOLConstruction::EaseOutOfVisibleWalls(Pair, Radius, VisualGap, MaxStep, Alone) ? 1 : 0;
	}
	const double AloneFrom = HOLConstruction::GetDistance(Body, Alone);
	TestTrue(FString::Printf(TEXT("by one wall: eased out, %d steps, %.1f cm from the body"), AloneSteps, AloneFrom),
		AloneSteps > 1 && AloneFrom >= VisualGap + Radius);

	// EtapeDefini R (J loop 9's leftover): standing still in a corner of the slab (where the navmesh's edge leaves a walker
	// sent on the body's corner). Measured from the body, round at its corners, the rule left its body in the slab's
	// corner; square corners (the guard's for every unit kept out) put it wholly out, a step at a time.
	const FVector2D Slab = Half + FVector2D(VisualGap);
	const auto FromSlab = [Slab](const FVector2D& P)
	{
		const FVector2D Away = P.GetAbs() - Slab;
		return Away.GetMax() <= 0. ? Away.GetMax() : FVector2D(FMath::Max(Away.X, 0.), FMath::Max(Away.Y, 0.)).Size();
	};
	const HOLConstruction::FWallBody One[] = { Left };
	const FVector2D InCorner = Half + FVector2D(60.0, 60.0);
	FVector2D Round = InCorner;
	for (int32 Step = 0; Step < 100; ++Step)
	{
		HOLConstruction::EaseOutOfVisibleWalls(One, Radius, VisualGap, MaxStep, Round);
	}
	TestTrue(FString::Printf(TEXT("round corners: its body left in the slab's corner, %.1f cm from the slab < %.1f"), FromSlab(Round), Radius),
		FromSlab(Round) < Radius);
	FVector2D Square = InCorner;
	int32 SquareSteps = 0;
	double SquareLongest = 0.0;
	for (int32 Step = 0; Step < 100; ++Step)
	{
		const FVector2D Before = Square;
		SquareSteps += HOLConstruction::EaseOutOfVisibleWalls(One, Radius, VisualGap, MaxStep, Square, true) ? 1 : 0;
		SquareLongest = FMath::Max(SquareLongest, FVector2D::Distance(Before, Square));
	}
	TestTrue(FString::Printf(TEXT("square corners: wholly out of the slab's corner, %.1f cm >= %.1f, %d steps of %.1f cm at most"), FromSlab(Square), Radius,
		SquareSteps, SquareLongest), FromSlab(Square) >= Radius && SquareSteps > 1 && SquareLongest <= MaxStep + 0.01);
	FVector2D SquareAgain = Square;
	TestFalse(TEXT("square corners: out, it stays"), HOLConstruction::EaseOutOfVisibleWalls(One, Radius, VisualGap, MaxStep, SquareAgain, true));
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLConstructionBodyTest, "HeroesOfLiberty.Construction.Body", UE::HeroesOfLiberty::Tests::Construction::Flags)
bool FHOLConstructionBodyTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::Construction;
	// D324 (J6b.2b): the pure body rules (GetBodySize, GetBody, GetGap, DoesWalkCross, FindWays) are tested in Script\Tests\HOLConstruction_Test.as
	// (Test_Construction_Body). What stays here: the game's settings (UHOLBuildingSettings), not exposed to the script.
	const double Peasant = 60.0;

	// The game's setting, the same rule: one Peasant between two neighbours, not two.
	const double Setting = UHOLBuildingSettings::GetBodyInset();
	TestTrue(FString::Printf(TEXT("the setting (%.0f cm per side): one Peasant passes, not two"), Setting), 2.0 * Setting >= 2.0 * Peasant && 2.0 * Setting < 4.0 * Peasant);
	TestTrue(TEXT("the setting's house body"), UHOLBuildingSettings::GetBodySize(FVector2D(600.0, 600.0)).Equals(FVector2D(600.0 - 2.0 * Setting), 0.01));
	// What is seen fills at least 90 % of the tiles of any building (D324 refined), wider than the body: Peasants walk a
	// little into the shape.
	const double Seen = UHOLBuildingSettings::GetVisualInset();
	TestTrue(FString::Printf(TEXT("the visible shape (%.0f cm per side): at least 90 %% of a house's tiles"), Seen), 600.0 - 2.0 * Seen >= 0.9 * 600.0 - 0.01);
	TestTrue(TEXT("the visible shape wider than the body"), Seen <= Setting);
	TestTrue(TEXT("the setting's house shape"), UHOLBuildingSettings::GetVisualSize(FVector2D(600.0, 600.0)).Equals(FVector2D(600.0 - 2.0 * Seen), 0.01));
	return true;
}

// The user's trial of J7b, feedback 3: a ghost on a building or a source goes to the nearest free spot beside it.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLConstructionFreeFootprintTest, "HeroesOfLiberty.Construction.FreeFootprint", UE::HeroesOfLiberty::Tests::Construction::Flags)
bool FHOLConstructionFreeFootprintTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::Construction;
	const auto Anywhere = [](const FBox2D&) { return true; };
	const FVector2D Aim(100.0, 50.0);

	// The cursor on a house at (0, 0): the obstacle is that house, the ghost touches it (ring 0) on the side nearest the
	// aim (east: its middle 5 m from the aim, north 5.6 m).
	{
		const FBox2D Houses[] = { House(0.0, 0.0), House(30.0, 0.0) };
		FHOLPlacementQuery Query = MakeQuery(0.0, 0.0);
		Query.Footprint = HOLConstruction::SnapToGrid(Aim, FVector2D(600.0, 600.0), TestTile);
		Query.Buildings = Houses;
		FBox2D Obstacle;
		TestTrue(TEXT("on a house: an obstacle found"), HOLConstruction::FindObstacle(Query, Aim, TestTile, Obstacle));
		TestTrue(TEXT("on a house: the house itself, already on the grid"), Obstacle.Min.Equals(Houses[0].Min) && Obstacle.Max.Equals(Houses[0].Max));
		FBox2D Free;
		int32 Ring = -1;
		TestTrue(TEXT("beside a house: found"), HOLConstruction::FindFreeFootprint(Query, Obstacle, Aim, TestTile, 4, Anywhere, Free, &Ring));
		TestEqual(TEXT("beside a house: ring 0 (touching)"), Ring, 0);
		TestTrue(TEXT("beside a house: east, nearest the aim"), Free.GetCenter().Equals(FVector2D(600.0, 0.0)));
		TestFalse(TEXT("beside a house: touching, not overlapping"), HOLConstruction::Overlaps(Free, Houses[0]));
		TestTrue(TEXT("beside a house: on the grid, its size"), IsOnGrid(Free, TestTile) && Free.GetSize().Equals(FVector2D(600.0, 600.0)));

		// The cursor between two houses, inside the second: the one containing the aim is the obstacle.
		const FBox2D Pair[] = { House(0.0, 0.0), House(0.0, 6.0) };
		Query.Buildings = Pair;
		Query.Footprint = House(0.0, 3.0);
		TestTrue(TEXT("two houses: an obstacle found"), HOLConstruction::FindObstacle(Query, FVector2D(0.0, 400.0), TestTile, Obstacle));
		TestTrue(TEXT("two houses: the one containing the aim"), Obstacle.Min.Equals(Pair[1].Min));
	}

	// Ring 0 all taken (a frame of buildings one tile wide around the house): ring 1, east again.
	{
		const FBox2D Blocks[] = {
			FBox2D(FVector2D(-600.0, -600.0), FVector2D(600.0, -300.0)), FBox2D(FVector2D(-600.0, 300.0), FVector2D(600.0, 600.0)),
			FBox2D(FVector2D(-600.0, -300.0), FVector2D(-300.0, 300.0)), FBox2D(FVector2D(300.0, -300.0), FVector2D(600.0, 300.0)) };
		FHOLPlacementQuery Query = MakeQuery(0.0, 0.0);
		Query.Buildings = Blocks;
		FBox2D Free;
		int32 Ring = -1;
		TestTrue(TEXT("ring 0 taken: found"), HOLConstruction::FindFreeFootprint(Query, House(0.0, 0.0), Aim, TestTile, 4, Anywhere, Free, &Ring));
		TestEqual(TEXT("ring 0 taken: ring 1"), Ring, 1);
		TestTrue(TEXT("ring 0 taken: one tile east of the house"), Free.GetCenter().Equals(FVector2D(900.0, 0.0)));

		// The navmesh refuses that one: the next of ring 1 nearest the aim (north).
		const auto NotEast = [](const FBox2D& Candidate) { return Candidate.GetCenter().X < 800.0; };
		TestTrue(TEXT("off the navmesh: another found"), HOLConstruction::FindFreeFootprint(Query, House(0.0, 0.0), Aim, TestTile, 4, NotEast, Free, &Ring));
		TestTrue(TEXT("off the navmesh: still ring 1, not east"), Ring == 1 && Free.GetCenter().X < 800.0);
	}

	// A source's disc: its bounding box grown outward to the grid lines; a footprint on the grid stays as it is.
	{
		const FHOLPlacementCircle Tree[] = { { FVector2D(100.0, 100.0), 250.0 } };
		FHOLPlacementQuery Query = MakeQuery(0.0, 0.0);
		Query.Sources = Tree;
		FBox2D Obstacle;
		TestTrue(TEXT("on a tree: an obstacle found"), HOLConstruction::FindObstacle(Query, Aim, TestTile, Obstacle));
		TestTrue(TEXT("on a tree: its disc as a box on the grid"), Obstacle.Min.Equals(FVector2D(-300.0, -300.0)) && Obstacle.Max.Equals(FVector2D(600.0, 600.0)));
		const FBox2D OnGrid = HOLConstruction::SnapOutward(House(3.0, 0.0), TestTile);
		TestTrue(TEXT("a footprint on the grid: unchanged"), OnGrid.Min.Equals(House(3.0, 0.0).Min) && OnGrid.Max.Equals(House(3.0, 0.0).Max));
		FBox2D Free;
		int32 Ring = -1;
		TestTrue(TEXT("beside a tree: found"), HOLConstruction::FindFreeFootprint(Query, Obstacle, Aim, TestTile, 4, Anywhere, Free, &Ring));
		TestTrue(TEXT("beside a tree: ring 0, clear of the tree"), Ring == 0 && !HOLConstruction::Overlaps(Free, Tree[0]) && IsOnGrid(Free, TestTile));
	}

	// Nothing free within 4 rings (a building covering them all, 50 cm into ring 4): none; with 5 rings, ring 5.
	{
		const FBox2D Big[] = { FBox2D(FVector2D(-1550.0, -1550.0), FVector2D(1550.0, 1550.0)) };
		FHOLPlacementQuery Query = MakeQuery(0.0, 0.0);
		Query.Buildings = Big;
		FBox2D Free;
		int32 Ring = -1;
		TestFalse(TEXT("all taken within 4 rings: none"), HOLConstruction::FindFreeFootprint(Query, House(0.0, 0.0), Aim, TestTile, 4, Anywhere, Free, &Ring));
		TestTrue(TEXT("5 rings: ring 5"), HOLConstruction::FindFreeFootprint(Query, House(0.0, 0.0), Aim, TestTile, 5, Anywhere, Free, &Ring) && Ring == 5);
	}

	// A candidate refused by another rule is skipped: the map ends at the house's east side, the ghost goes north.
	{
		FHOLPlacementQuery Query = MakeQuery(0.0, 0.0);
		Query.MapArea.Max.X = 300.0;
		FBox2D Free;
		int32 Ring = -1;
		TestTrue(TEXT("map edge: found"), HOLConstruction::FindFreeFootprint(Query, House(0.0, 0.0), Aim, TestTile, 4, Anywhere, Free, &Ring));
		TestTrue(TEXT("map edge: ring 0, north (east is out of the map)"), Ring == 0 && Free.GetCenter().Equals(FVector2D(0.0, 600.0)));
	}
	return true;
}

#endif
