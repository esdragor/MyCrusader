// The game state's faction-slot readers (ELROND-1120c, lot du socle des factions, morceau 1 + the faction of a slot for hol.Cheat.Honor).
//   - AHOLGameState::GetSlotFaction(Slot): the EHOLFaction index of the slot (Dawn, 0, without a configuration or out of the table);
//   - AHOLGameState::GetPowerCooldownEndMs / SetPowerCooldownEndMs (authority): the replicated end time of a slot's power cooldown, 0 = ready.
// RED to see on the first commit: the C++ is a stub (GetSlotFaction -1, the cooldown always 0). The replication itself (a client reads the server's
// value) is NOT covered here: it needs `quick_check -Parts net`.

namespace FactionSlotStateTest
{
	void Run(FUnitTest& T, AHOLGameState State)
	{
		// Each value is read into a local first (brief_commun pitfall 4).
		int Dawn = int(EHOLFaction::DawnOrder);
		int Faction0 = State.GetSlotFaction(0);
		int FactionOut = State.GetSlotFaction(99);
		int FactionNeg = State.GetSlotFaction(-1);
		T.AssertEquals(Dawn, Faction0, "a slot without a configuration is the Dawn's");
		T.AssertEquals(Dawn, FactionOut, "a slot out of the table is the Dawn's");
		T.AssertEquals(Dawn, FactionNeg, "a negative slot (spectator) is the Dawn's");

		int Fresh = State.GetPowerCooldownEndMs(0, 0);
		T.AssertEquals(0, Fresh, "a power never used is ready (0)");

		State.SetPowerCooldownEndMs(0, 0, 5000);
		State.SetPowerCooldownEndMs(0, 2, 7500);
		State.SetPowerCooldownEndMs(1, 0, 9000);
		int A = State.GetPowerCooldownEndMs(0, 0);
		int B = State.GetPowerCooldownEndMs(0, 2);
		int C = State.GetPowerCooldownEndMs(1, 0);
		int Untouched = State.GetPowerCooldownEndMs(0, 1);
		T.AssertEquals(5000, A, "slot 0 power 0 holds its end time");
		T.AssertEquals(7500, B, "slot 0 power 2 holds its own end time");
		T.AssertEquals(9000, C, "slot 1 power 0 holds its own end time (slots do not share)");
		T.AssertEquals(0, Untouched, "slot 0 power 1 is untouched");

		State.SetPowerCooldownEndMs(0, 0, 0);
		A = State.GetPowerCooldownEndMs(0, 0);
		B = State.GetPowerCooldownEndMs(0, 2);
		T.AssertEquals(0, A, "a power can be made ready again");
		T.AssertEquals(7500, B, "and its neighbour keeps its time");

		// Out of the table: no effect, no crash, reads give 0.
		State.SetPowerCooldownEndMs(99, 0, 123);
		State.SetPowerCooldownEndMs(0, 99, 123);
		State.SetPowerCooldownEndMs(-1, -1, 123);
		int OutSlot = State.GetPowerCooldownEndMs(99, 0);
		int OutPower = State.GetPowerCooldownEndMs(0, 99);
		int OutNeg = State.GetPowerCooldownEndMs(-1, -1);
		int Still = State.GetPowerCooldownEndMs(1, 0);
		T.AssertEquals(0, OutSlot, "a slot out of the table reads 0");
		T.AssertEquals(0, OutPower, "a power out of the table reads 0");
		T.AssertEquals(0, OutNeg, "negative indices read 0");
		T.AssertEquals(9000, Still, "and wrote nothing elsewhere");
	}
}

void Test_FactionSlotStateAccessors(FUnitTest& T)
{
	UHOLTestWorldHandle TestWorld = HOLTestWorld::CreateTestWorld();
	T.AssertTrue(TestWorld != nullptr, "a test world is created");
	if (TestWorld == nullptr)
		return;
	AHOLGameState State = HOLTestWorld::SpawnGameState(TestWorld, 0, 0);
	T.AssertTrue(State != nullptr, "game state");
	if (State != nullptr)
		FactionSlotStateTest::Run(T, State);
	HOLTestWorld::DestroyTestWorld(TestWorld);
}
