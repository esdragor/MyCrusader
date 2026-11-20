// hol.Cheat.Honor <value> (CELEBORN-1120b, lot du socle des factions, morceau 2; model: hol.Cheat.Tier and HOLTierCheat_Test.as). All script, no C++: the cheat guard is
// HOLRpcBridge::CheatRefused (HOLRpcPlayer.as), the write is AHOLGameState::SetFactionResource. Three pieces, each tested here or in HOLControllerCommandsC2_Test.as:
//   - HOLRpc::CheatHonorRefusal (pure refusals of the RPC: 1 cheats off, 2 no slot, 3 the faction has no gauge),
//   - HOLFactionResource::CheatTarget (the value held between the gauge's floor and cap) and HOLHonorCheat::Apply (the effect on a real game state),
//   - the console plan (HOLControllerCommandsC2.as), tested in HOLControllerCommandsC2_Test.as (Test_ControllerCommandsC2CheatHonor).
// Cap 100 and floor 0 are D859 / D688 (tunable after a try, D345). RED to see on the first commit: the three rules are stubs.

void Test_HonorCheatRefusal(FUnitTest& T)
{
	T.AssertEquals(0, HOLRpc::CheatHonorRefusal(true, 1, true), "cheats on, a seated Dawn player: passed on");
	T.AssertEquals(1, HOLRpc::CheatHonorRefusal(false, 1, true), "cheats off: refused first");
	T.AssertEquals(2, HOLRpc::CheatHonorRefusal(true, -1, true), "a spectator is refused");
	T.AssertEquals(3, HOLRpc::CheatHonorRefusal(true, 1, false), "a faction without a gauge is refused");
	T.AssertEquals(1, HOLRpc::CheatHonorRefusal(false, -1, false), "cheats are checked before everything else");
}

void Test_HonorCheatTargetIsBounded(FUnitTest& T)
{
	const FHOLFactionResource Gauge = HOLFactionResource::Make(30, 0, 100);
	int Inside = HOLFactionResource::CheatTarget(42, Gauge);
	int Above = HOLFactionResource::CheatTarget(250, Gauge);
	int Below = HOLFactionResource::CheatTarget(-5, Gauge);
	int AtCap = HOLFactionResource::CheatTarget(100, Gauge);
	T.AssertEquals(42, Inside, "a value inside the bounds is kept");
	T.AssertEquals(100, Above, "a value above the cap is held to the cap");
	T.AssertEquals(0, Below, "a value below the floor is held to the floor (no debt)");
	T.AssertEquals(100, AtCap, "the cap itself is allowed");
}

namespace HonorCheatTest
{
	void Run(FUnitTest& T, AHOLGameState State)
	{
		int Cap = HOLHonorCheat::HonorCap();
		T.AssertEquals(100, Cap, "the Dawn's Honor cap is 100 (D859)");

		// Each value is read into a local first (brief_commun pitfall 4).
		int Stored = HOLHonorCheat::Apply(State, 0, EHOLFaction::DawnOrder, 60);
		int Read = State.GetFactionResource(0);
		T.AssertEquals(60, Stored, "the Dawn's gauge takes 60 and says so");
		T.AssertEquals(60, Read, "the game state holds 60 for slot 0");
		int Other = State.GetFactionResource(1);
		T.AssertEquals(0, Other, "the other slots are untouched");

		Stored = HOLHonorCheat::Apply(State, 0, EHOLFaction::DawnOrder, 250);
		Read = State.GetFactionResource(0);
		T.AssertEquals(100, Read, "250 is held to the cap");
		Stored = HOLHonorCheat::Apply(State, 0, EHOLFaction::DawnOrder, -5);
		Read = State.GetFactionResource(0);
		T.AssertEquals(0, Read, "-5 is held to the floor");

		HOLHonorCheat::Apply(State, 0, EHOLFaction::DawnOrder, 40);
		Stored = HOLHonorCheat::Apply(State, 1, EHOLFaction::BlackLegions, 50);
		Read = State.GetFactionResource(1);
		int Dawn = State.GetFactionResource(0);
		T.AssertEquals(-1, Stored, "a faction without a gauge stores nothing and says so");
		T.AssertEquals(0, Read, "the Legions' slot stays at 0");
		T.AssertEquals(40, Dawn, "and the Dawn's slot is untouched by the refused call");
		Stored = HOLHonorCheat::Apply(State, 1, EHOLFaction::Templars, 50);
		Read = State.GetFactionResource(1);
		T.AssertEquals(0, Read, "the Templars have no gauge either");

		Stored = HOLHonorCheat::Apply(State, -1, EHOLFaction::DawnOrder, 50);
		T.AssertEquals(-1, Stored, "a negative slot (spectator) stores nothing");
	}
}

// CELEBORN-1120d: the server side of ServerCheatHonor on a real AHOLGameState: the faction is read from the state (GetSlotFaction, Dawn by default), the refusals leave the gauge alone.
namespace HonorCheatServerTest
{
	void Run(FUnitTest& T, AHOLGameState State)
	{
		int Done = HOLHonorCheat::ServerApply(State, true, 0, 70);
		int Read = State.GetFactionResource(0);
		T.AssertEquals(70, Done, "a seated Dawn player with cheats on: 70 stored");
		T.AssertEquals(70, Read, "the game state holds 70 for the sender slot");
		int Off = HOLHonorCheat::ServerApply(State, false, 0, 10);
		Read = State.GetFactionResource(0);
		T.AssertEquals(-1, Off, "cheats off: refused 1");
		T.AssertEquals(70, Read, "cheats off: the gauge is untouched");
		int Spectator = HOLHonorCheat::ServerApply(State, true, -1, 10);
		T.AssertEquals(-2, Spectator, "a spectator: refused 2");
		int Capped = HOLHonorCheat::ServerApply(State, true, 0, 900);
		Read = State.GetFactionResource(0);
		T.AssertEquals(100, Capped, "900 is held to the cap by the server");
		T.AssertEquals(100, Read, "and the state holds the cap");
		int NoState = HOLHonorCheat::ServerApply(nullptr, true, 0, 5);
		T.AssertEquals(-4, NoState, "no game state: nothing written, -4");
	}
}

void Test_HonorCheatServerApply(FUnitTest& T)
{
	UHOLTestWorldHandle TestWorld = HOLTestWorld::CreateTestWorld();
	T.AssertTrue(TestWorld != nullptr, "a test world is created");
	if (TestWorld == nullptr)
		return;
	AHOLGameState State = HOLTestWorld::SpawnGameState(TestWorld, 0, 0);
	T.AssertTrue(State != nullptr, "game state");
	if (State != nullptr)
		HonorCheatServerTest::Run(T, State);
	HOLTestWorld::DestroyTestWorld(TestWorld);
}

void Test_HonorCheatEffect(FUnitTest& T)
{
	UHOLTestWorldHandle TestWorld = HOLTestWorld::CreateTestWorld();
	T.AssertTrue(TestWorld != nullptr, "a test world is created");
	if (TestWorld == nullptr)
		return;
	AHOLGameState State = HOLTestWorld::SpawnGameState(TestWorld, 0, 0);
	T.AssertTrue(State != nullptr, "game state");
	if (State != nullptr)
		HonorCheatTest::Run(T, State);
	HOLTestWorld::DestroyTestWorld(TestWorld);
}
