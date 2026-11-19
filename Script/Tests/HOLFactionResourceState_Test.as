// Faction resource gauge, replicated state (lot C++ socle factions 1119, piece 1; ELROND-1119n4). D838: a script test, the state is not Mass.
// One integer per player slot in AHOLGameState::FactionResource, read and written by GetFactionResource / SetFactionResource (the library
// UHOLFactionResourceLibrary wraps the same two on the world's game state). Same shape as HOLTierCheat_Test.as: the test world's bare game state
// (SpawnGameState(.., 0, 0)), the values read into locals first (brief_commun pitfall 4).
// Not covered here: the client side (Set without effect on a client) and the library's world lookup; the net phase of quick_check covers replication.

namespace FactionResourceStateTest
{
	void Run(FUnitTest& T, AHOLGameState State)
	{
		int Fresh = State.GetFactionResource(0);
		T.AssertEquals(0, Fresh, "a fresh gauge reads 0");

		State.SetFactionResource(0, 7);
		State.SetFactionResource(1, 3);
		int Zero = State.GetFactionResource(0);
		int One = State.GetFactionResource(1);
		T.AssertEquals(7, Zero, "slot 0 reads what was set");
		T.AssertEquals(3, One, "slot 1 reads what was set, independent of slot 0");

		State.SetFactionResource(0, 12);
		Zero = State.GetFactionResource(0);
		One = State.GetFactionResource(1);
		T.AssertEquals(12, Zero, "a second Set replaces the first");
		T.AssertEquals(3, One, "and leaves the other slot alone");

		// Out of the table: reads 0, writes nothing (and touches no valid slot).
		State.SetFactionResource(99, 5);
		State.SetFactionResource(-1, 5);
		int Beyond = State.GetFactionResource(99);
		int Below = State.GetFactionResource(-1);
		T.AssertEquals(0, Beyond, "a slot beyond the table reads 0 after a Set");
		T.AssertEquals(0, Below, "a negative slot reads 0 after a Set");
		Zero = State.GetFactionResource(0);
		One = State.GetFactionResource(1);
		T.AssertEquals(12, Zero, "a Set out of range left slot 0 unchanged");
		T.AssertEquals(3, One, "a Set out of range left slot 1 unchanged");

		T.AssertTrue(HOLTestWorld::IsPropertyReplicated(State, n"FactionResource"), "the gauge is a replicated property: every client reads the same value");
	}
}

void Test_FactionResourceStateRoundTrip(FUnitTest& T)
{
	UHOLTestWorldHandle TestWorld = HOLTestWorld::CreateTestWorld();
	T.AssertTrue(TestWorld != nullptr, "a test world is created");
	if (TestWorld == nullptr)
		return;
	AHOLGameState State = HOLTestWorld::SpawnGameState(TestWorld, 0, 0);
	T.AssertTrue(State != nullptr, "game state");
	if (State != nullptr)
		FactionResourceStateTest::Run(T, State);
	HOLTestWorld::DestroyTestWorld(TestWorld);
}
