// Script port of the component-free part of HOLBuildingTests.cpp (D853, CELEBORN-1119cc): Foundation, Selection, Damage.
//   Buildings.Foundation -> Test_BuildingFoundation; Buildings.Selection -> Test_BuildingSelection; Buildings.Damage -> Test_BuildingDamage.
// Same chain as the C++ (a spawned AHOLBuilding of the test world, the real AHOLBuilding::SetConstructionState / SetConstructionWork / GetHealth,
// UHOLConstructionSubsystem::DamageBuilding, the selection channel's line trace); the HOLTestWorld wrappers are thin calls to those methods, no rule
// copied. The C++ Selection test is removed (all ported, red seen then green on integ-19, CELEBORN-1119ch); Foundation and Damage stay in HOLBuildingTests.cpp for their unported assertions (listed below). HOLBuilding_Test.as (the table tests) is a different file.
//
// Not ported, and why:
//   Foundation:
//   - the player's OWN foundation (SetOwner(local APlayerController): IsSeenHere true, the 13 pieces of a ghost, the unseen click box, the bounds
//     height / footprint, the piece colours, the selection ray hitting the ghost, the 4 / 13 / 4 pieces of site, half-built and finished states, the
//     heights of the bounds): a local APlayerController and the components' Bounds / materials (UStaticMeshComponent, UBoxComponent, material colour)
//     are not exposed to the script; the C++ test stays for them.
//   - "its forbidden area counts for the navmesh" (UBoxComponent::CanEverAffectNavigation): component, not exposed.
//   Damage:
//   - "its footprint blocks the navmesh / no longer blocks" (UBoxComponent::CanEverAffectNavigation): component, not exposed.
//   - the rubble (TActorIterator<AHOLRubble>: its type name, 10 s life span, no click answered): AHOLRubble is not exposed and has no wrapper yet.
//   Selection: all ported, C++ test removed.

namespace BuildingWorldTest
{
	// The C++ spawns with World->SpawnActor<AHOLBuilding>() at the origin then Setup(Type, Owner, Colour, State); SpawnBuilding does Setup(Type, Owner,
	// Owner, State) at the given location (the colour is the owner's; no assertion here reads it).
	AHOLBuilding Make(UHOLTestWorldHandle TestWorld, FName TypeName, int Owner, EHOLConstructionState State)
	{
		AHOLBuilding Building = HOLTestWorld::SpawnBuilding(TestWorld, TypeName, Owner, FVector(0.0, 0.0, 0.0), State);
		HOLTestWorld::ApplyBuildingAppearance(Building);
		return Building;
	}
}

// Foundation, the part without the owner's controller: another player's foundation shows nothing and answers no click (J6a.4), and the building's
// states go Foundation -> a site -> Finished as the game's own accessors read them.
void Test_BuildingFoundation(FUnitTest& T)
{
	UHOLTestWorldHandle TestWorld = HOLTestWorld::CreateTestWorld();
	T.AssertTrue(TestWorld != nullptr, "test world ready");
	if (TestWorld == nullptr)
		return;

	AHOLBuilding Other = BuildingWorldTest::Make(TestWorld, n"House", 1, EHOLConstructionState::Foundation);
	T.AssertTrue(Other != nullptr, "another player's building spawned");
	if (Other != nullptr)
	{
		// No local owner in this world: the C++ asserts exactly this.
		bool bSeen = HOLTestWorld::IsBuildingSeenHere(Other);
		T.AssertFalse(bSeen, "another player's foundation: not seen here");
		int Pieces = HOLTestWorld::CountBuildingMeshPieces(Other);
		T.AssertEquals(0, Pieces, "another player's foundation: no piece");
		AActor Hit = HOLTestWorld::TraceSelectionRayDown(TestWorld, 100.0, 100.0);
		T.AssertTrue(Hit == nullptr, "another player's foundation: nothing to click");

		// The state accessors (the C++ asserts them on the owner's foundation: "a foundation"; they do not read the owner).
		bool bFoundation = HOLTestWorld::IsBuildingFoundation(Other);
		bool bFinished = HOLTestWorld::IsBuildingFinished(Other);
		bool bStarted = HOLTestWorld::HasBuildingWorkStarted(Other);
		T.AssertTrue(bFoundation && !bFinished && !bStarted, "a foundation: IsFoundation, not finished, no work started");

		// Its first work (D312): a site, no longer a foundation.
		HOLTestWorld::SetBuildingConstructionState(Other, EHOLConstructionState::UnderConstruction);
		HOLTestWorld::ApplyBuildingAppearance(Other);
		T.AssertFalse(HOLTestWorld::IsBuildingFoundation(Other), "a site: no longer a foundation");
		T.AssertFalse(HOLTestWorld::IsBuildingFinished(Other), "a site: not finished");
		// Half its work: the work is read back as set (the game's own total).
		int64 Total = HOLTestWorld::GetBuildingTotalWork(Other);
		T.AssertTrue(Total > 0, "a house has work to do");
		int64 Half = Total / 2;
		HOLTestWorld::SetBuildingConstructionWork(Other, Half);
		int64 Work = HOLTestWorld::GetBuildingConstructionWork(Other);
		T.AssertTrue(Half == Work, "half its work: the work set is the work read");

		// Finished.
		HOLTestWorld::SetBuildingConstructionState(Other, EHOLConstructionState::Finished);
		HOLTestWorld::ApplyBuildingAppearance(Other);
		bool bDone = HOLTestWorld::IsBuildingFinished(Other);
		T.AssertTrue(bDone, "finished");
	}

	HOLTestWorld::DestroyTestWorld(TestWorld);
}

// Selection (D262): the selection ray, from the height of the camera, hits the building and nothing beside it.
void Test_BuildingSelection(FUnitTest& T)
{
	UHOLTestWorldHandle TestWorld = HOLTestWorld::CreateTestWorld();
	T.AssertTrue(TestWorld != nullptr, "test world ready");
	if (TestWorld == nullptr)
		return;

	AHOLBuilding Building = BuildingWorldTest::Make(TestWorld, n"MainCentre", 1, EHOLConstructionState::Finished);
	T.AssertTrue(Building != nullptr, "building spawned");
	if (Building != nullptr)
	{
		AActor Hit = HOLTestWorld::TraceSelectionRayDown(TestWorld, 0.0, 0.0);
		AHOLBuilding HitBuilding = Cast<AHOLBuilding>(Hit);
		T.AssertTrue(HitBuilding == Building, "the selection ray hits the building");
		AActor Beside = HOLTestWorld::TraceSelectionRayDown(TestWorld, 2000.0, 0.0);
		T.AssertTrue(Beside == nullptr, "nothing beside it");
	}

	HOLTestWorld::DestroyTestWorld(TestWorld);
}

// D345.2a: a building's health (its work's share, the damage kept), its armour against arrows, its destruction. The expected healths come from the
// type row's MaxHealth (the table) and the game's own GetHealth / DamageBuilding; 10 = 50 ranged minus the 40 armour, as the C++ writes it.
void Test_BuildingDamage(FUnitTest& T)
{
	UHOLTestWorldHandle TestWorld = HOLTestWorld::CreateTestWorld();
	T.AssertTrue(TestWorld != nullptr, "test world ready");
	if (TestWorld == nullptr)
		return;

	int MaxHealth = HOLTestWorld::GetBuildingTypeMaxHealth(n"House");
	T.AssertTrue(MaxHealth > 0, "house in the table");
	AHOLBuilding Site = BuildingWorldTest::Make(TestWorld, n"House", 0, EHOLConstructionState::Foundation);
	T.AssertTrue(Site != nullptr, "site spawned");
	if (MaxHealth > 0 && Site != nullptr)
	{
		int Armor = HOLTestWorld::GetBuildingTypeRangedArmor(n"House");
		T.AssertEquals(40, Armor, "every building: 40 ranged armour (D345)");

		int Health = HOLTestWorld::GetBuildingHealth(Site);
		T.AssertEquals(1, Health, "a bare foundation: 1 health");
		HOLTestWorld::SetBuildingConstructionState(Site, EHOLConstructionState::UnderConstruction);
		int64 Half = HOLTestWorld::GetBuildingTotalWork(Site) / 2;
		HOLTestWorld::SetBuildingConstructionWork(Site, Half);
		Health = HOLTestWorld::GetBuildingHealth(Site);
		T.AssertEquals(MaxHealth / 2, Health, "half built: half the health");

		int Dealt = HOLTestWorld::DamageBuildingByAttack(Site, 50, true);
		T.AssertEquals(10, Dealt, "50 ranged: 10 dealt");
		bool bHurt = HOLTestWorld::IsBuildingHurt(Site);
		T.AssertTrue(bHurt, "hurt");
		HOLTestWorld::SetBuildingConstructionWork(Site, HOLTestWorld::GetBuildingTotalWork(Site));
		HOLTestWorld::SetBuildingConstructionState(Site, EHOLConstructionState::Finished);
		Health = HOLTestWorld::GetBuildingHealth(Site);
		T.AssertEquals(MaxHealth - 10, Health, "finished: the damage kept");

		int Killing = HOLTestWorld::DamageBuildingByAttack(Site, 100000, false);
		T.AssertEquals(MaxHealth - 10, Killing, "a blow of all its health: all of it dealt");
		bool bAlive = HOLTestWorld::IsBuildingAlive(Site);
		T.AssertFalse(bAlive, "destroyed");
	}

	HOLTestWorld::DestroyTestWorld(TestWorld);
}
