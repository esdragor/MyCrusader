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

#endif
