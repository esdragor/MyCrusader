// Copyright Epic Games, Inc. All Rights Reserved.

#include "Buildings/HOLBuilding.h"
#include "Buildings/HOLBuildingTypes.h"
#include "Engine/Engine.h"
#include "Engine/World.h"
#include "EngineUtils.h"
#include "Misc/AutomationTest.h"

#if WITH_DEV_AUTOMATION_TESTS

// Performance review (PerformanceInProgress.md, P1): FHOLBuildingIterator walks the buildings of a world from a registry, without
// visiting every actor as TActorIterator does. It must give what TActorIterator<AHOLBuilding> gave (the same buildings, in
// the order they appeared), for one world and not the others, and let a loop destroy buildings.
namespace UE::HeroesOfLiberty::Tests::BuildingRegistry
{
	constexpr EAutomationTestFlags Flags = EAutomationTestFlags::EditorContext | EAutomationTestFlags::EngineFilter;

	UWorld* MakeWorld()
	{
		UWorld* World = UWorld::CreateWorld(EWorldType::Game, /*bInformEngineOfWorld*/ false);
		FWorldContext& Context = GEngine->CreateNewWorldContext(EWorldType::Game);
		Context.SetCurrentWorld(World);
		World->InitializeActorsForPlay(FURL());
		return World;
	}

	void DestroyWorld(UWorld* World)
	{
		GEngine->DestroyWorldContext(World);
		World->DestroyWorld(/*bInformEngineOfWorld*/ false);
	}

	AHOLBuilding* Spawn(UWorld* World, const uint8 Owner)
	{
		AHOLBuilding* Building = World->SpawnActor<AHOLBuilding>();
		if (Building)
		{
			Building->Setup(HOLBuildingTypeNames::MainCentre, Owner, Owner);
		}
		return Building;
	}

	TArray<AHOLBuilding*> ByActors(UWorld* World)
	{
		TArray<AHOLBuilding*> Found;
		for (TActorIterator<AHOLBuilding> It(World); It; ++It)
		{
			Found.Add(*It);
		}
		return Found;
	}

	TArray<AHOLBuilding*> ByRegistry(const UWorld* World)
	{
		TArray<AHOLBuilding*> Found;
		for (FHOLBuildingIterator It(World); It; ++It)
		{
			Found.Add(*It);
		}
		return Found;
	}
}

IMPLEMENT_SIMPLE_AUTOMATION_TEST(FHOLBuildingRegistryTest, "HeroesOfLiberty.Buildings.Registry", UE::HeroesOfLiberty::Tests::BuildingRegistry::Flags)
bool FHOLBuildingRegistryTest::RunTest(const FString& Parameters)
{
	using namespace UE::HeroesOfLiberty::Tests::BuildingRegistry;

	UWorld* World = MakeWorld();
	UWorld* Other = MakeWorld();

	TestEqual(TEXT("no building: none found"), ByRegistry(World).Num(), 0);

	AHOLBuilding* A = Spawn(World, 0);
	AHOLBuilding* B = Spawn(World, 1);
	AHOLBuilding* C = Spawn(World, 2);
	AHOLBuilding* Alien = Spawn(Other, 3);
	if (!TestTrue(TEXT("four buildings spawned"), A && B && C && Alien))
	{
		DestroyWorld(Other);
		DestroyWorld(World);
		return false;
	}

	TestEqual(TEXT("the same buildings in the same order as TActorIterator"), ByRegistry(World), ByActors(World));
	TestEqual(TEXT("three of this world"), ByRegistry(World).Num(), 3);
	TestEqual(TEXT("the other world's alone"), ByRegistry(Other), TArray<AHOLBuilding*>{ Alien });

	// A building destroyed leaves the walk; the order of the others is kept.
	B->Destroy();
	TestEqual(TEXT("after a destroy: as TActorIterator"), ByRegistry(World), ByActors(World));
	TestEqual(TEXT("after a destroy: the two left"), ByRegistry(World), TArray<AHOLBuilding*>{ A, C });

	// A building appearing later comes last.
	AHOLBuilding* D = Spawn(World, 0);
	TestEqual(TEXT("a new one comes last"), ByRegistry(World), TArray<AHOLBuilding*>{ A, C, D });
	TestEqual(TEXT("and TActorIterator agrees"), ByRegistry(World), ByActors(World));

	// A loop that destroys what it walks (a destroyed building must not make it skip the next one).
	TArray<AHOLBuilding*> Seen;
	for (FHOLBuildingIterator It(World); It; ++It)
	{
		Seen.Add(*It);
		It->Destroy();
	}
	TestEqual(TEXT("a loop that destroys sees all three"), Seen, TArray<AHOLBuilding*>{ A, C, D });
	TestEqual(TEXT("none left"), ByRegistry(World).Num(), 0);
	TestEqual(TEXT("the other world untouched"), ByRegistry(Other), TArray<AHOLBuilding*>{ Alien });

	DestroyWorld(Other);
	DestroyWorld(World);
	return true;
}

#endif // WITH_DEV_AUTOMATION_TESTS
