// Copyright Epic Games, Inc. All Rights Reserved.

#include "Engine/Engine.h"
#include "Engine/Level.h"
#include "Engine/World.h"
#include "Map/HOLMapRules.h"
#include "Map/HOLMapSizeRules.h"
#include "Map/HOLVolcanoLayout.h"
#include "Misc/AutomationTest.h"
#include "Simulation/HOLVolcanoSubsystem.h"

#if WITH_DEV_AUTOMATION_TESTS

// V4b (VolcanoInProgress.md, D607, D573): the volcano sites a map lays, as data. A map's AHOLVolcanoLayout lists them (flows from the crater, an
// altar); the rules below are what every map's sites must satisfy (Map.Table calls them on the real map files): the flows are the same by a turn of
// one place (as many flows towards each camp, so that the free draw of the direction is fair), the altar as far from every base, everything inside
// the bounds. UHOLVolcanoSubsystem reads the layout when the match begins (LoadLayout).
namespace UE::HeroesOfLiberty::Tests::VolcanoLayout
{
	constexpr EAutomationTestFlags Flags = EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter;

	/** The flow from the middle towards Angle (degrees), Length long, in three points. */
	FHOLPlacedLavaFlow Flow(const double AngleDegrees, const double Length)
	{
		FHOLPlacedLavaFlow Result;
		const double Angle = FMath::DegreesToRadians(AngleDegrees);
		for (const double Fraction : { 0., 0.5, 1. })
		{
			Result.Points.Add(FVector(Length * Fraction * FMath::Cos(Angle), Length * Fraction * FMath::Sin(Angle), 0.));
		}
		return Result;
	}

	/** A site of the shape the maps give: the crater in the middle, one flow towards each of Places places (the first at FirstAngle), the altar in the middle. */
	FHOLPlacedVolcano Site(const int32 Places, const double FirstAngle = 17., const double Length = 6000.)
	{
		FHOLPlacedVolcano Result;
		for (int32 Index = 0; Index < Places; ++Index)
		{
			Result.Flows.Add(Flow(FirstAngle + 360. * Index / Places, Length));
		}
		Result.bAltar = true;
		return Result;
	}

	TArray<FVector2D> Slots(const int32 Places, const double Distance = 15000., const double FirstAngle = 17.)
	{
		TArray<FVector2D> Result;
		for (int32 Index = 0; Index < Places; ++Index)
		{
			const double Angle = FMath::DegreesToRadians(FirstAngle + 360. * Index / Places);
			Result.Add(FVector2D(Distance * FMath::Cos(Angle), Distance * FMath::Sin(Angle)));
		}
		return Result;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLVolcanoLayoutFairFlowsTest, "HeroesOfLiberty.Volcano.LayoutFlows", UE::HeroesOfLiberty::Tests::VolcanoLayout::Flags)
bool FHOLVolcanoLayoutFairFlowsTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::VolcanoLayout;
	for (const int32 Places : { 2, 3, 4, 5, 6, 7, 8 })
	{
		const double Step = 360. / Places;
		const FString Label = FString::Printf(TEXT("%d places"), Places);
		TestEqual(*(Label + TEXT(": a flow towards each camp, the same, is fair")), FHOLVolcanoLayoutRules::CountUnbalancedFlows({ Site(Places) }, Step), 0);

		// A flow shorter than the others favours the camp it aims at (the lava stops farther from it).
		FHOLPlacedVolcano Short = Site(Places);
		Short.Flows[0] = Flow(17., 4500.);
		TestTrue(*(Label + TEXT(": one flow shorter than the others is caught")), FHOLVolcanoLayoutRules::CountUnbalancedFlows({ Short }, Step) > 0);

		// A flow bent (its middle point moved by 5 m) is another flow.
		FHOLPlacedVolcano Bent = Site(Places);
		Bent.Flows[0].Points[1].Y += 500.;
		TestTrue(*(Label + TEXT(": one flow with a bent middle is caught")), FHOLVolcanoLayoutRules::CountUnbalancedFlows({ Bent }, Step) > 0);

		// A camp with two flows aimed at it and another with none.
		FHOLPlacedVolcano Doubled = Site(Places);
		Doubled.Flows[1] = Doubled.Flows[0];
		TestTrue(*(Label + TEXT(": a flow aimed twice at the same camp is caught")), FHOLVolcanoLayoutRules::CountUnbalancedFlows({ Doubled }, Step) > 0);

		// A flow without its twin: one more flow, alone.
		FHOLPlacedVolcano Lonely = Site(Places);
		Lonely.Flows.Add(Flow(17. + 0.5 * Step, 6000.));
		TestTrue(*(Label + TEXT(": a flow alone in its turn is caught")), FHOLVolcanoLayoutRules::CountUnbalancedFlows({ Lonely }, Step) > 0);

		// Inside the tolerance (1 m) a rounding of the generator is accepted.
		FHOLPlacedVolcano Rounded = Site(Places);
		Rounded.Flows[0].Points[2].X += 60.;
		TestEqual(*(Label + TEXT(": a rounding of 60 cm is accepted")), FHOLVolcanoLayoutRules::CountUnbalancedFlows({ Rounded }, Step), 0);
	}
	TestEqual(TEXT("no site: nothing unbalanced"), FHOLVolcanoLayoutRules::CountUnbalancedFlows(TConstArrayView<FHOLPlacedVolcano>(), 90.), 0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLVolcanoLayoutAltarTest, "HeroesOfLiberty.Volcano.LayoutAltar", UE::HeroesOfLiberty::Tests::VolcanoLayout::Flags)
bool FHOLVolcanoLayoutAltarTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::VolcanoLayout;
	// Two places on the X axis: the altar is as far from both on the middle line across (Y axis), not elsewhere.
	const TArray<FVector2D> Two{ FVector2D(-15000., 0.), FVector2D(15000., 0.) };
	FHOLPlacedVolcano Across = Site(2);
	Across.AltarLocation = FVector(0., 3000., 0.);
	TestEqual(TEXT("2 places: the altar on the line across is as far from both"), FHOLVolcanoLayoutRules::CountUnfairAltars({ Across }, Two), 0);
	FHOLPlacedVolcano Nearer = Site(2);
	Nearer.AltarLocation = FVector(3000., 0., 0.);
	TestEqual(TEXT("2 places: an altar 30 m nearer one base is caught"), FHOLVolcanoLayoutRules::CountUnfairAltars({ Nearer }, Two), 1);
	TestTrue(TEXT("2 places: the gap is the 60 m between the distances"), FMath::Abs(FHOLVolcanoLayoutRules::AltarSpread(Nearer, Two) - 6000.) < 1.);
	// Six places: only the middle is as far from every base.
	const TArray<FVector2D> Six = Slots(6);
	TestEqual(TEXT("6 places: the altar in the middle is as far from every base"), FHOLVolcanoLayoutRules::CountUnfairAltars({ Site(6) }, Six), 0);
	FHOLPlacedVolcano Off = Site(6);
	Off.AltarLocation = FVector(0., 3000., 0.);
	TestEqual(TEXT("6 places: an altar off the middle is caught"), FHOLVolcanoLayoutRules::CountUnfairAltars({ Off }, Six), 1);
	FHOLPlacedVolcano None = Site(6);
	None.bAltar = false;
	None.AltarLocation = FVector(0., 3000., 0.);
	TestEqual(TEXT("no altar: nothing to be unfair"), FHOLVolcanoLayoutRules::CountUnfairAltars({ None }, Six), 0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLVolcanoLayoutShapeTest, "HeroesOfLiberty.Volcano.LayoutShape", UE::HeroesOfLiberty::Tests::VolcanoLayout::Flags)
bool FHOLVolcanoLayoutShapeTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::VolcanoLayout;
	TestEqual(TEXT("a site with flows has no empty flow"), FHOLVolcanoLayoutRules::CountEmptyFlows({ Site(4) }), 0);
	FHOLPlacedVolcano Empty = Site(4);
	Empty.Flows.Reset();
	TestEqual(TEXT("a site without a flow is caught"), FHOLVolcanoLayoutRules::CountEmptyFlows({ Empty }), 1);
	FHOLPlacedVolcano OnePoint = Site(4);
	OnePoint.Flows[0].Points.SetNum(1);
	TestEqual(TEXT("a flow of one point is caught"), FHOLVolcanoLayoutRules::CountEmptyFlows({ OnePoint }), 1);
	TestEqual(TEXT("a site inside the bounds"), FHOLVolcanoLayoutRules::CountOutside({ Site(4) }, 20000.), 0);
	TestTrue(TEXT("a site whose flow leaves the bounds is caught"), FHOLVolcanoLayoutRules::CountOutside({ Site(4, 17., 26000.) }, 20000.) > 0);
	FHOLPlacedVolcano AltarOut = Site(2);
	AltarOut.AltarLocation = FVector(19500., 0., 0.);
	TestEqual(TEXT("an altar whose zone leaves the bounds is caught"), FHOLVolcanoLayoutRules::CountOutside({ AltarOut }, 20000.), 1);

	// The simulation's sites: the flows as polylines (the crater first), the altar, and no altar when the map lays none.
	FHOLPlacedVolcano Placed = Site(2);
	Placed.AltarLocation = FVector(0., 3000., 0.);
	Placed.AltarRadius = 1300.f;
	const TArray<HOLVolcano::FSite> Sites = FHOLVolcanoLayoutRules::ToSites({ Placed });
	if (TestEqual(TEXT("one site read"), Sites.Num(), 1) && TestEqual(TEXT("two flows"), Sites[0].Flows.Num(), 2))
	{
		TestEqual(TEXT("three points a flow"), Sites[0].Flows[0].Points.Num(), 3);
		TestTrue(TEXT("the crater is the first point"), Sites[0].Flows[1].Points[0].Equals(FVector::ZeroVector, 1.));
		TestTrue(TEXT("the altar is present"), Sites[0].Altar.bPresent);
		TestTrue(TEXT("the altar is where the map put it"), Sites[0].Altar.Location.Equals(FVector(0., 3000., 0.), 1.));
		TestEqual(TEXT("the altar's zone"), Sites[0].Altar.Radius, 1300.f);
	}
	Placed.bAltar = false;
	const TArray<HOLVolcano::FSite> Bare = FHOLVolcanoLayoutRules::ToSites({ Placed });
	if (TestEqual(TEXT("one site read, no altar"), Bare.Num(), 1))
	{
		TestFalse(TEXT("no altar laid"), Bare[0].Altar.bPresent);
	}
	return true;
}

// The real chain: the layout actor a map would carry, the world's volcano subsystem reading it as the match begins.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLVolcanoLayoutLoadTest, "HeroesOfLiberty.Volcano.LayoutLoad", UE::HeroesOfLiberty::Tests::VolcanoLayout::Flags)
bool FHOLVolcanoLayoutLoadTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::VolcanoLayout;
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, /*bInformEngineOfWorld*/ false);
	FWorldContext& Context = GEngine->CreateNewWorldContext(EWorldType::Game);
	Context.SetCurrentWorld(World);
	World->InitializeActorsForPlay(FURL());
	UHOLVolcanoSubsystem* Volcano = World->GetSubsystem<UHOLVolcanoSubsystem>();
	if (!TestNotNull(TEXT("the volcano subsystem"), Volcano))
	{
		GEngine->DestroyWorldContext(World);
		World->DestroyWorld(false);
		return false;
	}
	Volcano->LoadLayout();
	TestEqual(TEXT("no layout: no site"), Volcano->GetSites().Num(), 0);
	AHOLVolcanoLayout* Layout = World->SpawnActor<AHOLVolcanoLayout>();
	TestNotNull(TEXT("the layout actor"), Layout);
	if (Layout)
	{
		FHOLPlacedVolcano Placed = Site(2);
		Placed.AltarLocation = FVector(0., 3000., 0.);
		Layout->SetSites({ Placed });
		Volcano->LoadLayout();
		if (TestEqual(TEXT("one site loaded"), Volcano->GetSites().Num(), 1) && TestEqual(TEXT("two flows loaded"), Volcano->GetSites()[0].Flows.Num(), 2))
		{
			TestTrue(TEXT("the altar loaded"), Volcano->GetSites()[0].Altar.bPresent);
		}
		Volcano->LoadLayout();
		TestEqual(TEXT("loaded twice: still one (the list is rebuilt, not added to)"), Volcano->GetSites().Num(), 1);
	}
	GEngine->DestroyWorldContext(World);
	World->DestroyWorld(false);
	return true;
}

// 2026-11-04 (the user: a mesh for the altar so that it shows on the maps): the layout actor draws one placeholder altar (4 basic-shape parts: two steps, a
// pillar, a red top) for each of its sites that has an altar, and nothing for a site without one.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLVolcanoLayoutAltarMeshTest, "HeroesOfLiberty.Volcano.LayoutAltarMesh", UE::HeroesOfLiberty::Tests::VolcanoLayout::Flags)
bool FHOLVolcanoLayoutAltarMeshTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::VolcanoLayout;
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, /*bInformEngineOfWorld*/ false);
	FWorldContext& Context = GEngine->CreateNewWorldContext(EWorldType::Game);
	Context.SetCurrentWorld(World);
	World->InitializeActorsForPlay(FURL());
	World->BeginPlay();
	FHOLPlacedVolcano WithAltar = Site(2);
	WithAltar.bAltar = true;
	WithAltar.AltarLocation = FVector(0., -3000., 0.);
	FHOLPlacedVolcano WithoutAltar = Site(2);
	WithoutAltar.bAltar = false;
	AHOLVolcanoLayout* Layout = World->SpawnActorDeferred<AHOLVolcanoLayout>(AHOLVolcanoLayout::StaticClass(), FTransform::Identity);
	if (TestNotNull(TEXT("the layout actor"), Layout))
	{
		Layout->SetSites({ WithAltar, WithoutAltar });
		Layout->FinishSpawning(FTransform::Identity);
		TestEqual(TEXT("one altar drawn (4 parts), none for the site without altar"), Layout->GetAltarMeshCount(), 4);
	}
	GEngine->DestroyWorldContext(World);
	World->DestroyWorld(false);
	return true;
}

// V4b on a real file: the Steppe's small map (2 places) carries the site its recipe writes (Tools/Editor/maps/steppe.py volcano(), regenerated by
// Tools\make_maps.ps1 -Map Steppe -Sizes Small): one layout, one site, a flow towards each of the 2 places, an altar across the axis of the places.
// Map.Table checks the fairness of every map; this one checks that the recipe's site really is in the file the game loads.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLVolcanoLayoutSteppeSmallTest, "HeroesOfLiberty.Volcano.LayoutSteppeSmall", UE::HeroesOfLiberty::Tests::VolcanoLayout::Flags)
bool FHOLVolcanoLayoutSteppeSmallTest::RunTest(const FString& Parameters)
{
	const FHOLMapSizeInfo* Small = nullptr;
	for (const FHOLMapSizeInfo& Info : FHOLMapSizeRules::GetSizes())
	{
		Small = FCString::Strcmp(Info.Key, TEXT("Small")) == 0 ? &Info : Small;
	}
	if (!TestNotNull(TEXT("the Small size"), Small))
	{
		return false;
	}
	const FString File = FHOLMapSizeRules::GetMapFile(FHOLMapRules::GetAssetPath(TEXT("Steppe")), Small->Size).ToString();
	const UWorld* Map = LoadObject<UWorld>(nullptr, *File);
	if (!TestNotNull(*FString::Printf(TEXT("%s loads"), *File), Map) || !Map->PersistentLevel)
	{
		return false;
	}
	int32 Layouts = 0;
	TArray<FHOLPlacedVolcano> Sites;
	for (const AActor* Actor : Map->PersistentLevel->Actors)
	{
		if (const AHOLVolcanoLayout* Layout = Cast<AHOLVolcanoLayout>(Actor))
		{
			++Layouts;
			Sites.Append(Layout->GetSites());
		}
	}
	TestEqual(TEXT("Steppe_Small: one volcano layout (Tools\\make_maps.ps1 -Map Steppe -Sizes Small)"), Layouts, 1);
	if (TestEqual(TEXT("Steppe_Small: one site"), Sites.Num(), 1))
	{
		TestEqual(TEXT("Steppe_Small: a flow towards each of the 2 places"), Sites[0].Flows.Num(), 2);
		TestTrue(TEXT("Steppe_Small: an altar"), Sites[0].bAltar);
		for (const FHOLPlacedLavaFlow& Flow : Sites[0].Flows)
		{
			TestTrue(TEXT("Steppe_Small: the crater (the middle of the map) is the first point of a flow"), !Flow.Points.IsEmpty() && Flow.Points[0].Size2D() < 1.);
		}
	}
	return true;
}

#endif