// Script port of the first "game state" test of HOLVictoryTests.cpp (D853 point 4, D854, CELEBORN-1119cf): Victory.GameState -> Test_VictoryGameState.
// Same chain as the C++ (the real AHOLGameState::InitVictory, DefeatSlot, IsSlotDefeated, GetDefeatReason, IsMatchOver, GetWinners, IsWinner; the real
// OnSlotDefeated / OnMatchOver broadcasts, counted by a UHOLVictoryEventCounter bound to them); the HOLTestWorld wrappers are thin calls to those
// methods, no rule copied. Same configuration as the C++ MakeConfig({ {Human, 0}, {AI, 0} }): slot 0 human, slot 1 AI, both team 0 (the default),
// every other slot closed (SpawnGameState(TestWorld, 1, 1) does exactly this and calls InitVictory). The C++ test stays until the main session has
// seen this one red then green.
//
// Not ported (stay in C++ in HOLVictoryTests.cpp, D854): Victory.Collapse and every Forum.* test that births units (RequestUnits +
// ServeBirthRequests, Mass) or reads components / the rubble; the Victory.Rules.* tests are already in HOLVictory_Test.as.

// HeroesOfLiberty.Victory.GameState
void Test_VictoryGameState(FUnitTest& T)
{
	UHOLTestWorldHandle TestWorld = HOLTestWorld::CreateTestWorld();
	T.AssertTrue(TestWorld != nullptr, "test world ready");
	if (TestWorld == nullptr)
		return;

	AHOLGameState State = HOLTestWorld::SpawnGameState(TestWorld, 1, 1);
	T.AssertTrue(State != nullptr, "game state spawned");
	if (State != nullptr)
	{
		UHOLVictoryEventCounter Counter = HOLTestWorld::CountVictoryEvents(State);
		T.AssertTrue(Counter != nullptr, "event counter bound");
		if (Counter != nullptr)
		{
			bool bClosedLoses = HOLTestWorld::DefeatGameStateSlot(State, 4, EHOLDefeatReason::Cheat);
			T.AssertFalse(bClosedLoses, "a closed slot cannot lose");
			bool bOverAtLaunch = HOLTestWorld::IsGameStateMatchOver(State);
			T.AssertFalse(bOverAtLaunch, "not over at the launch");
			bool bSurrenders = HOLTestWorld::DefeatGameStateSlot(State, 1, EHOLDefeatReason::Surrender);
			T.AssertTrue(bSurrenders, "slot 1 surrenders");
			bool bOneDefeated = HOLTestWorld::IsGameStateSlotDefeated(State, 1);
			T.AssertTrue(bOneDefeated, "slot 1 defeated");
			EHOLDefeatReason Reason = HOLTestWorld::GetGameStateDefeatReason(State, 1);
			T.AssertTrue(Reason == EHOLDefeatReason::Surrender, "by surrender");
			bool bZeroDefeated = HOLTestWorld::IsGameStateSlotDefeated(State, 0);
			T.AssertFalse(bZeroDefeated, "slot 0 not defeated");
			bool bOver = HOLTestWorld::IsGameStateMatchOver(State);
			T.AssertTrue(bOver, "over");
			TArray<int> Winners = HOLTestWorld::GetGameStateWinners(State);
			T.AssertEquals(1, Winners.Num(), "one winner");
			if (Winners.Num() == 1)
				T.AssertEquals(0, Winners[0], "slot 0 wins");
			bool bZeroWins = HOLTestWorld::IsGameStateWinner(State, 0);
			bool bOneWins = HOLTestWorld::IsGameStateWinner(State, 1);
			T.AssertTrue(bZeroWins && !bOneWins, "IsWinner");
			bool bLoseAfterEnd = HOLTestWorld::DefeatGameStateSlot(State, 0, EHOLDefeatReason::Cheat);
			T.AssertFalse(bLoseAfterEnd, "once over, nobody loses any more");
			bool bTwice = HOLTestWorld::DefeatGameStateSlot(State, 1, EHOLDefeatReason::Cheat);
			T.AssertFalse(bTwice, "never defeated twice");
			int Defeats = Counter.GetDefeats();
			int Ends = Counter.GetEnds();
			T.AssertEquals(1, Defeats, "one defeat told");
			T.AssertEquals(1, Ends, "one end told");
		}
	}

	HOLTestWorld::DestroyTestWorld(TestWorld);
}