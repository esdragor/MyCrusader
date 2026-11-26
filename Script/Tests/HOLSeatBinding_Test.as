// CLAUDE-1125s (point 3, D839: a new test is written in script; ported from HOLSeatBindingTests.cpp, GLORFINDEL-1124b): the two bindings the script
// reads for a seat, UHOLHeroLibrary::GetHeroUnitId (D900: the cast's walk order names the hero by his unit id) and
// UHOLSpecializationReadLibrary::GetChosenOptionForSlot (D893: the seat's applied option, read server side). Both through the real chains: the unit
// table's hero (SpawnTestHero) and the seat's AHOLPlayerState in the game state's PlayerArray (SetPlayerSpecialization).
// The two bindings take their world from the script's world context: the scope below gives them the TEST world (FAngelscriptGameThreadScopeWorldContext).
// Not ported: the "no world: -1" cases of the C++ test (a null world context is refused by the script binding itself, not by these functions).

// HeroesOfLiberty.SeatBinding.HeroUnitId
void Test_SeatHeroUnitId(FUnitTest& T)
{
	UHOLTestWorldHandle TestWorld = HOLTestWorld::CreateTestWorld();
	T.AssertTrue(TestWorld != nullptr, "the test world");
	if (TestWorld == nullptr)
		return;
	{
		FAngelscriptGameThreadScopeWorldContext Scope(HOLTestWorld::GetWorldObject(TestWorld));
		T.AssertEquals(-1, HOLHero::GetHeroUnitId(0), "no hero born yet: -1");
		const int HeroUnit = HOLTestWorld::SpawnTestHero(TestWorld, 0, FVector(1000.0, 2000.0, 0.0));
		T.AssertTrue(HeroUnit > 0, "a hero unit was born");
		if (HeroUnit > 0)
		{
			T.AssertEquals(HeroUnit, HOLHero::GetHeroUnitId(0), "the slot's hero is named by the id the unit table gave him");
			T.AssertEquals(-1, HOLHero::GetHeroUnitId(1), "another slot has no hero: -1");
		}
		T.AssertEquals(-1, HOLHero::GetHeroUnitId(-1), "a slot out of range: -1");
	}
	HOLTestWorld::DestroyTestWorld(TestWorld);
}

// HeroesOfLiberty.SeatBinding.ChosenOptionForSlot
void Test_SeatChosenOptionForSlot(FUnitTest& T)
{
	UHOLTestWorldHandle TestWorld = HOLTestWorld::CreateTestWorld();
	T.AssertTrue(TestWorld != nullptr, "the test world");
	if (TestWorld == nullptr)
		return;
	{
		FAngelscriptGameThreadScopeWorldContext Scope(HOLTestWorld::GetWorldObject(TestWorld));
		AHOLGameState State = HOLTestWorld::SpawnGameState(TestWorld, 0, 0);
		AHOLPlayerState Seat = HOLTestWorld::SpawnPlayerState(TestWorld, 2, 0, "Seat2");
		T.AssertTrue(State != nullptr && Seat != nullptr, "the game state and the seat's player state");
		if (State != nullptr && Seat != nullptr)
		{
			FHOLSpecializationView View;
			View.Chosen.Add(-1);
			View.Chosen.Add(2);
			View.Chosen.Add(-1);
			View.Chosen.Add(0);
			T.AssertTrue(HOLTestWorld::SetPlayerSpecialization(Seat, View), "the server side writes the seat's view");
			T.AssertEquals(2, HOLSpecializationRead::GetChosenOptionForSlot(2, 1), "tier 1: the applied option, read by slot");
			T.AssertEquals(-1, HOLSpecializationRead::GetChosenOptionForSlot(2, 2), "tier 2: no option yet");
			T.AssertEquals(0, HOLSpecializationRead::GetChosenOptionForSlot(2, 3), "tier 3: option 0 is an option");
			T.AssertEquals(-1, HOLSpecializationRead::GetChosenOptionForSlot(2, 9), "a tier out of range: -1");
			T.AssertEquals(-1, HOLSpecializationRead::GetChosenOptionForSlot(5, 1), "a slot without a player state: -1");
		}
	}
	HOLTestWorld::DestroyTestWorld(TestWorld);
}
