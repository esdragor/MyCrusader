// Copyright Epic Games, Inc. All Rights Reserved.

#include "Engine/Engine.h"
#include "Engine/World.h"
#include "Map/HOLRelayLayout.h"
#include "Misc/AutomationTest.h"
#include "Simulation/HOLRelaySubsystem.h"

#if WITH_DEV_AUTOMATION_TESTS

// R2 (RelaysInProgress.md): the relays a map lays, as data. A map's AHOLRelayLayout lists them (place and zone radius, like the terrain
// zones); the rules below are what every map's relays must satisfy (the Map.Table test of a map calls them once the maps provide relays):
// symmetric by a turn of one place (TC-Q7, 8.1), no relay in a base's reach, inside the bounds. UHOLRelaySubsystem reads the layout when
// the match begins (LoadLayout).
namespace UE::HeroesOfLiberty::Tests::RelayLayout
{
	constexpr EAutomationTestFlags Flags = EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter;

	FHOLPlacedRelay Make(const double X, const double Y, const float Radius = 1200.f)
	{
		FHOLPlacedRelay Relay;
		Relay.Location = FVector(X, Y, 0.);
		Relay.Radius = Radius;
		return Relay;
	}

	/** A ring of Count relays at Distance from the origin, the first on the +X axis. */
	TArray<FHOLPlacedRelay> Ring(const int32 Count, const double Distance, const double FirstAngleDegrees = 0.)
	{
		TArray<FHOLPlacedRelay> Relays;
		for (int32 Index = 0; Index < Count; ++Index)
		{
			const double Angle = FMath::DegreesToRadians(FirstAngleDegrees + 360. * Index / Count);
			Relays.Add(Make(Distance * FMath::Cos(Angle), Distance * FMath::Sin(Angle)));
		}
		return Relays;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLRelayLayoutSymmetryTest, "HeroesOfLiberty.Relays.LayoutSymmetry", UE::HeroesOfLiberty::Tests::RelayLayout::Flags)
bool FHOLRelayLayoutSymmetryTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::RelayLayout;
	for (const int32 Places : { 2, 3, 4, 5, 6, 7, 8 })
	{
		const double Step = 360. / Places;
		TArray<FHOLPlacedRelay> Relays = Ring(Places, 9000., 17.);
		TestEqual(FString::Printf(TEXT("%d places: a ring of relays turned by one place lands on itself"), Places), FHOLRelayLayoutRules::CountUnmatchedByTurn(Relays, Step), 0);
		// The middle of the map is its own image: any number of relays there, they stay symmetric.
		Relays.Add(Make(0., 0., 800.f));
		TestEqual(FString::Printf(TEXT("%d places: a relay in the middle is its own image"), Places), FHOLRelayLayoutRules::CountUnmatchedByTurn(Relays, Step), 0);
		// One relay moved by 5 m breaks the symmetry (tolerance 1 m): the relay moved and the one it no longer meets.
		TArray<FHOLPlacedRelay> Moved = Ring(Places, 9000., 17.);
		Moved[0].Location.X += 500.;
		TestTrue(FString::Printf(TEXT("%d places: a relay moved by 5 m is caught"), Places), FHOLRelayLayoutRules::CountUnmatchedByTurn(Moved, Step) > 0);
		// A different radius is a different relay.
		TArray<FHOLPlacedRelay> Fatter = Ring(Places, 9000., 17.);
		Fatter[0].Radius = 1500.f;
		TestTrue(FString::Printf(TEXT("%d places: a relay with another radius is caught"), Places), FHOLRelayLayoutRules::CountUnmatchedByTurn(Fatter, Step) > 0);
		// A relay without its twin (one more on the ring, alone).
		TArray<FHOLPlacedRelay> Lonely = Ring(Places, 9000., 17.);
		Lonely.Add(Make(3000., 1000.));
		TestTrue(FString::Printf(TEXT("%d places: a relay alone in its turn is caught"), Places), FHOLRelayLayoutRules::CountUnmatchedByTurn(Lonely, Step) > 0);
	}
	TestEqual(TEXT("no relay: nothing unmatched"), FHOLRelayLayoutRules::CountUnmatchedByTurn(TConstArrayView<FHOLPlacedRelay>(), 90.), 0);
	// Within the tolerance (1 m) a rounding of the generator is accepted.
	TArray<FHOLPlacedRelay> Rounded = Ring(4, 9000.);
	Rounded[1].Location.X += 60.;
	TestEqual(TEXT("a rounding of 60 cm is accepted"), FHOLRelayLayoutRules::CountUnmatchedByTurn(Rounded, 90.), 0);
	return true;
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLRelayLayoutPlaceTest, "HeroesOfLiberty.Relays.LayoutPlace", UE::HeroesOfLiberty::Tests::RelayLayout::Flags)
bool FHOLRelayLayoutPlaceTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::RelayLayout;
	const TArray<FVector2D> Slots{ FVector2D(-15000., 0.), FVector2D(15000., 0.) };
	const float BaseReach = 6000.f;
	TestEqual(TEXT("relays between the bases are clear of them"),
		FHOLRelayLayoutRules::CountInBases(TArray<FHOLPlacedRelay>{ Make(0., 0.), Make(0., 9000.) }, Slots, BaseReach), 0);
	// A relay's zone must not touch the base's reach: centre at 6000 + radius is the closest allowed.
	TestEqual(TEXT("a relay whose zone enters a base's reach is caught"),
		FHOLRelayLayoutRules::CountInBases(TArray<FHOLPlacedRelay>{ Make(-9500., 0., 1200.f) }, Slots, BaseReach), 1);
	TestEqual(TEXT("a relay whose zone only touches the reach is clear"),
		FHOLRelayLayoutRules::CountInBases(TArray<FHOLPlacedRelay>{ Make(-7800., 0., 1200.f) }, Slots, BaseReach), 0);
	TestEqual(TEXT("a relay outside the bounds is caught"),
		FHOLRelayLayoutRules::CountOutside(TArray<FHOLPlacedRelay>{ Make(0., 0.), Make(19500., 0.) }, 20000.), 1);
	TestEqual(TEXT("a relay inside the bounds, zone included, is kept"),
		FHOLRelayLayoutRules::CountOutside(TArray<FHOLPlacedRelay>{ Make(18800., 0.) }, 20000.), 0);
	TestEqual(TEXT("a zero radius is caught"), FHOLRelayLayoutRules::CountBadRadius(TArray<FHOLPlacedRelay>{ Make(0., 0., 0.f), Make(10., 10.) }), 1);
	return true;
}

// The real chain: the layout actor a map would carry, the world's subsystem reading it as the match begins.
IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLRelayLayoutLoadTest, "HeroesOfLiberty.Relays.LayoutLoad", UE::HeroesOfLiberty::Tests::RelayLayout::Flags)
bool FHOLRelayLayoutLoadTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::RelayLayout;
	UWorld* World = UWorld::CreateWorld(EWorldType::Game, /*bInformEngineOfWorld*/ false);
	FWorldContext& Context = GEngine->CreateNewWorldContext(EWorldType::Game);
	Context.SetCurrentWorld(World);
	World->InitializeActorsForPlay(FURL());
	UHOLRelaySubsystem* Relays = World->GetSubsystem<UHOLRelaySubsystem>();
	if (!TestNotNull(TEXT("the relay subsystem"), Relays))
	{
		GEngine->DestroyWorldContext(World);
		World->DestroyWorld(false);
		return false;
	}
	TestEqual(TEXT("no layout: no relay"), (Relays->LoadLayout(), Relays->GetRelays().Num()), 0);
	AHOLRelayLayout* Layout = World->SpawnActor<AHOLRelayLayout>();
	TestNotNull(TEXT("the layout actor"), Layout);
	if (Layout)
	{
		Layout->SetRelays({ Make(0., 0., 800.f), Make(9000., -3000., 1300.f) });
		Relays->LoadLayout();
		TestEqual(TEXT("two relays loaded"), Relays->GetRelays().Num(), 2);
		if (Relays->GetRelays().Num() == 2)
		{
			TestEqual(TEXT("second relay: X"), Relays->GetRelays()[1].Location.X, 9000.);
			TestEqual(TEXT("second relay: Y"), Relays->GetRelays()[1].Location.Y, -3000.);
			TestEqual(TEXT("second relay: radius"), Relays->GetRelays()[1].Radius, 1300.f);
			TestEqual(TEXT("neutral at the start"), Relays->GetRelays()[0].State.Owner, static_cast<int32>(INDEX_NONE));
		}
		Relays->LoadLayout();
		TestEqual(TEXT("loaded twice: still two (the list is rebuilt, not added to)"), Relays->GetRelays().Num(), 2);
	}
	GEngine->DestroyWorldContext(World);
	World->DestroyWorld(false);
	return true;
}

#endif
