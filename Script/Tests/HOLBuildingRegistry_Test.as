// Script port of HeroesOfLiberty.Buildings.Registry (HOLBuildingRegistryTests.cpp, D853, lot 1 of PLAN-TESTS-1119, ELROND-1119r): same case, same values.
// FHOLBuildingIterator walks the buildings of a world from a registry, without visiting every actor as TActorIterator does. It must give what
// TActorIterator<AHOLBuilding> gave (the same buildings, in the order they appeared), for one world and not the others. Both walks come from
// HOLTestWorld::GetBuildingsByRegistry / GetBuildingsByActors; two worlds live at once (the second holds the "alien" building).
// NOT ported (stays in HOLBuildingRegistryTests.cpp): "a loop that destroys what it walks sees all three" (the iterator's own property: the script
// only gets a snapshot array, destroying while looping over a snapshot proves nothing about the registry walk), so the C++ test stays whole.
// What follows that loop in C++ (none left, the other world untouched) is replayed here without the loop (all destroyed one by one, then looked at).

namespace BuildingRegistryTest
{
	bool Same(const TArray<AHOLBuilding>& Left, const TArray<AHOLBuilding>& Right)
	{
		if (Left.Num() != Right.Num())
			return false;
		for (int Index = 0; Index < Left.Num(); ++Index)
		{
			if (Left[Index] != Right[Index])
				return false;
		}
		return true;
	}

	AHOLBuilding Spawn(UHOLTestWorldHandle TestWorld, int Owner)
	{
		return HOLTestWorld::SpawnBuilding(TestWorld, n"MainCentre", Owner, FVector(0.0, 0.0, 0.0), EHOLConstructionState::Finished);
	}

	TArray<AHOLBuilding> Of(AHOLBuilding First, AHOLBuilding Second, AHOLBuilding Third)
	{
		TArray<AHOLBuilding> List;
		if (First != nullptr)
			List.Add(First);
		if (Second != nullptr)
			List.Add(Second);
		if (Third != nullptr)
			List.Add(Third);
		return List;
	}

	void Run(FUnitTest& T, UHOLTestWorldHandle World, UHOLTestWorldHandle Other)
	{
		TArray<AHOLBuilding> Walk = HOLTestWorld::GetBuildingsByRegistry(World);
		T.AssertEquals(0, Walk.Num(), "no building: none found");

		AHOLBuilding A = Spawn(World, 0);
		AHOLBuilding B = Spawn(World, 1);
		AHOLBuilding C = Spawn(World, 2);
		AHOLBuilding Alien = Spawn(Other, 3);
		bool bSpawned = A != nullptr && B != nullptr && C != nullptr && Alien != nullptr;
		T.AssertTrue(bSpawned, "four buildings spawned");
		if (!bSpawned)
			return;

		T.AssertTrue(Same(HOLTestWorld::GetBuildingsByRegistry(World), HOLTestWorld::GetBuildingsByActors(World)), "the same buildings in the same order as TActorIterator");
		Walk = HOLTestWorld::GetBuildingsByRegistry(World);
		T.AssertEquals(3, Walk.Num(), "three of this world");
		T.AssertTrue(Same(HOLTestWorld::GetBuildingsByRegistry(Other), Of(Alien, nullptr, nullptr)), "the other world's alone");

		// A building destroyed leaves the walk; the order of the others is kept.
		HOLTestWorld::DestroyBuilding(B);
		T.AssertTrue(Same(HOLTestWorld::GetBuildingsByRegistry(World), HOLTestWorld::GetBuildingsByActors(World)), "after a destroy: as TActorIterator");
		T.AssertTrue(Same(HOLTestWorld::GetBuildingsByRegistry(World), Of(A, C, nullptr)), "after a destroy: the two left");

		// A building appearing later comes last.
		AHOLBuilding D = Spawn(World, 0);
		T.AssertTrue(D != nullptr, "a fourth building of the world is spawned");
		T.AssertTrue(Same(HOLTestWorld::GetBuildingsByRegistry(World), Of(A, C, D)), "a new one comes last");
		T.AssertTrue(Same(HOLTestWorld::GetBuildingsByRegistry(World), HOLTestWorld::GetBuildingsByActors(World)), "and TActorIterator agrees");

		// The end of the C++ case (after its destroying loop), without the loop: every building of the world destroyed, then none left.
		HOLTestWorld::DestroyBuilding(A);
		HOLTestWorld::DestroyBuilding(C);
		HOLTestWorld::DestroyBuilding(D);
		Walk = HOLTestWorld::GetBuildingsByRegistry(World);
		T.AssertEquals(0, Walk.Num(), "none left");
		T.AssertTrue(Same(HOLTestWorld::GetBuildingsByRegistry(Other), Of(Alien, nullptr, nullptr)), "the other world untouched");
	}
}

void Test_BuildingRegistry(FUnitTest& T)
{
	UHOLTestWorldHandle World = HOLTestWorld::CreateTestWorld();
	UHOLTestWorldHandle Other = HOLTestWorld::CreateTestWorld();
	T.AssertTrue(World != nullptr && Other != nullptr, "two test worlds are created");
	if (World != nullptr && Other != nullptr)
		BuildingRegistryTest::Run(T, World, Other);
	// An assertion that fails does not stop the test, a failed spawn returns early inside Run: the worlds are destroyed here, always (nullptr is accepted).
	HOLTestWorld::DestroyTestWorld(Other);
	HOLTestWorld::DestroyTestWorld(World);
}
