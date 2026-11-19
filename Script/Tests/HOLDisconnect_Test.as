// Script port of the portable tests of HOLDisconnectTests.cpp (D853, lot 3 of PLAN-TESTS-1119, ELROND-1119v): same cases, same expected values.
//   HeroesOfLiberty.Match.DisconnectGameState  -> Test_DisconnectGameState  (whole, but GetReasonName: the reason is compared as its int, 5)
//   HeroesOfLiberty.Match.DisconnectBanner     -> Test_DisconnectBanner     (partial: FormatAbsentLine / FormatBackLine are not exposed; the
//                                                 texts they give are checked through the banner lines, as literals)
//   HeroesOfLiberty.Match.DisconnectHeroSheet  -> Test_DisconnectHeroSheet  (whole, but "the holder IsABot": only "not null and not Bob")
//   HeroesOfLiberty.Match.DisconnectPause      -> Test_DisconnectPause
//   HeroesOfLiberty.Match.DisconnectBannerClock-> Test_DisconnectBannerClock
//   HeroesOfLiberty.Match.DisconnectReportMode -> Test_DisconnectReportMode
// The C++ file stays: the main session removes the tests after it has seen these red then green and read the equivalence table.
// The configuration is the JSON of MakeNamedConfig (Alice slot 0 team 1, Bob slot 1 team 2, an AI slot 2 team 2, the rest closed; a 1v1 closes slot 2).
// C++ GiveSlotStock(Taking, Starting) without a time has no script twin: the time is given (0 for a first login, 100 for a return).

namespace DisconnectTest
{
	// EHOLSlotState: 0 Closed, 1 Human, 2 AI. EHOLDefeatReason::Disconnected = 5.
	const int SlotClosed = 0;
	const int SlotHuman = 1;
	const int SlotAI = 2;
	const int ReasonDisconnected = 5;

	FHOLResources Res(int Food, int Wood, int Stone, int Gold)
	{
		FHOLResources R;
		R.Food = Food;
		R.Wood = Wood;
		R.Stone = Stone;
		R.Gold = Gold;
		return R;
	}

	bool SameRes(FHOLResources A, FHOLResources B)
	{
		return A.Food == B.Food && A.Wood == B.Wood && A.Stone == B.Stone && A.Gold == B.Gold;
	}

	// MakeNamedConfig of the C++ file as JSON; bOneVsOne closes the AI seat of Bob's camp (the end of his window is then his defeat, D718, T5).
	FString MakeNamedJson(bool bOneVsOne)
	{
		FString Json = HOLMatchConfigRead::MakeDefaultJson();
		for (int Slot = 0; Slot < HOLMatchConfigRead::GetMaxSlots(); Slot++)
			Json = HOLMatchConfigRead::SetSlotJson(Json, Slot, SlotClosed, 0, Slot, 0, "");
		Json = HOLMatchConfigRead::SetSlotJson(Json, 0, SlotHuman, 0, 0, 1, "Alice");
		Json = HOLMatchConfigRead::SetSlotJson(Json, 1, SlotHuman, 0, 1, 2, "Bob");
		if (!bOneVsOne)
			Json = HOLMatchConfigRead::SetSlotJson(Json, 2, SlotAI, 0, 2, 2, "");
		return Json;
	}

	// One field of a tab-joined line (the absent notices: Slot, Name, EndsAt, BackAt, WindowSeconds).
	FString Field(const FString& Line, int Index)
	{
		TArray<FString> Parts;
		Line.ParseIntoArray(Parts, "\t", false);
		if (Index >= Parts.Num())
			return "<missing>";
		return Parts[Index];
	}

	// True when the slots a DefeatExpiredAbsences gave are exactly { Slot } (the C++ TArray<int32>{ 1 }).
	bool IsOnly(TArray<int> Slots, int Slot)
	{
		return Slots.Num() == 1 && Slots[0] == Slot;
	}

	// The banner as the clients read it: the one line, or "N lines".
	FString BannerAt(AHOLGameState State, double Time)
	{
		TArray<FString> Lines = HOLGameStateTest::GetAbsenceBannerLines(State, Time);
		if (Lines.Num() == 1)
			return Lines[0];
		return f"{Lines.Num()} lines";
	}

	// The line of "slot 1 ..." of hol.Disconnect.Report (DescribeDisconnections at 0, 180), or "".
	FString BobLine(AHOLGameState State)
	{
		TArray<FString> Lines = HOLGameStateTest::DescribeDisconnections(State, 0.0, 180.0);
		for (int Index = 0; Index < Lines.Num(); Index++)
		{
			if (Lines[Index].StartsWith("slot 1 "))
				return Lines[Index];
		}
		return "";
	}
}

// R7 (R-Q5, D71): a named player gone during the match keeps his slot for a window, then loses as by a surrender.
void Test_DisconnectGameState(FUnitTest& T)
{
	UHOLTestWorldHandle TestWorld = HOLTestWorld::CreateTestWorld();
	T.AssertTrue(TestWorld != nullptr, "a test world is created");
	if (TestWorld == nullptr)
		return;
	AHOLGameState State = HOLTestWorld::SpawnGameState(TestWorld, 0, 0);
	AHOLPlayerState Bob = HOLTestWorld::SpawnPlayerState(TestWorld, 1, 1, "");
	T.AssertTrue(State != nullptr, "game state spawned");
	T.AssertTrue(Bob != nullptr, "player states spawned");
	if (State != nullptr && Bob != nullptr)
	{
		// A 1v1 (the AI seat of Bob's camp closed): the end of his window is his defeat (D718, T5: with others in his camp the AI keeps the seat).
		T.AssertTrue(HOLGameStateTest::InitVictoryFromJson(State, DisconnectTest::MakeNamedJson(true)), "the 1v1 configuration is given to InitVictory");
		FHOLResources Starting = DisconnectTest::Res(200, 200, 200, 200);
		bool bReturn = HOLGameStateTest::GiveSlotStock(State, Bob, Starting, 0.0);
		T.AssertFalse(bReturn, "first login: not a return (no back to the manager)");
		T.AssertTrue(DisconnectTest::SameRes(HOLGameStateTest::GetResources(Bob), Starting), "first login: the stock of the start");
		HOLGameStateTest::SetResources(Bob, DisconnectTest::Res(0, 300, 0, 0));

		// WindowSeconds 0 = the default window (180), as the C++ default argument.
		bool bKept = HOLGameStateTest::KeepAbsentSlot(State, Bob, 10.0, 0.0);
		T.AssertTrue(bKept, "Bob leaves: slot kept");
		Bob.DestroyActor();	// as after a Logout (a live second holder of the seat would make a later KeepAbsentSlot a duplicate, T4)
		// SpawnPlayerState sets the slot at once (the C++ sets it later): spawned after the Destroy, else a live second holder of the seat (T4).
		AHOLPlayerState BobAgain = HOLTestWorld::SpawnPlayerState(TestWorld, 1, 1, "");
		T.AssertTrue(BobAgain != nullptr, "the second player state is spawned");
		T.AssertTrue(HOLGameStateTest::IsSlotAbsent(State, 1), "slot 1 absent");
		TArray<int> Early = HOLGameStateTest::DefeatExpiredAbsences(State, 189.0, 180.0);
		T.AssertEquals(0, Early.Num(), "no defeat 179 s later");
		bool bBack = HOLGameStateTest::GiveSlotStock(State, BobAgain, Starting, 100.0);
		T.AssertTrue(bBack, "back in the window: a return (M6-Q15, back to the manager)");
		T.AssertTrue(DisconnectTest::SameRes(HOLGameStateTest::GetResources(BobAgain), DisconnectTest::Res(0, 300, 0, 0)), "back in the window: his 300 wood, not the stock of the start");
		T.AssertFalse(HOLGameStateTest::IsSlotAbsent(State, 1), "slot 1 no longer absent");
		TArray<int> AfterBack = HOLGameStateTest::DefeatExpiredAbsences(State, 1000.0, 180.0);
		T.AssertEquals(0, AfterBack.Num(), "back: no defeat after the window");

		bool bLeavesAgain = HOLGameStateTest::KeepAbsentSlot(State, BobAgain, 1000.0, 0.0);
		T.AssertTrue(bLeavesAgain, "Bob leaves again");
		TArray<int> AtEnd = HOLGameStateTest::DefeatExpiredAbsences(State, 1180.0, 180.0);
		T.AssertTrue(DisconnectTest::IsOnly(AtEnd, 1), "the end of the window: slot 1 defeated");
		int Reason = HOLGameStateTest::GetDefeatReason(State, 1);
		T.AssertEquals(DisconnectTest::ReasonDisconnected, Reason, "reason Disconnected");
		T.AssertFalse(HOLGameStateTest::IsSlotAbsent(State, 1), "no longer absent once defeated");
		T.AssertTrue(HOLGameStateTest::IsMatchOver(State), "1v1: his defeat ends the match");
		bool bKeptDefeated = HOLGameStateTest::KeepAbsentSlot(State, BobAgain, 1200.0, 0.0);
		T.AssertFalse(bKeptDefeated, "a defeated slot is not kept");

		// B6b: Bob joins again after his defeat: he watches (no slot, no stock of the start on a beaten camp).
		TArray<int> NoneTaken;
		int AliceSlot = HOLGameStateTest::FindSlotOnLogin(State, DisconnectTest::MakeNamedJson(false), "Alice", NoneTaken);
		T.AssertEquals(0, AliceSlot, "Alice, not defeated: her slot");
		int BobSlot = HOLGameStateTest::FindSlotOnLogin(State, DisconnectTest::MakeNamedJson(false), "Bob", NoneTaken);
		T.AssertEquals(-1, BobSlot, "Bob after his defeat: a spectator");
	}

	// Without names (solo, a direct match): no window, the slot freed at once as before.
	AHOLGameState Solo = HOLTestWorld::SpawnGameState(TestWorld, 0, 0);
	AHOLPlayerState Player = HOLTestWorld::SpawnPlayerState(TestWorld, 0, 0, "");
	T.AssertTrue(Solo != nullptr, "solo game state spawned");
	if (Solo != nullptr && Player != nullptr)
	{
		T.AssertTrue(HOLGameStateTest::InitVictoryFromJson(Solo, HOLMatchConfigRead::MakeDefaultJson()), "the default configuration is given to InitVictory");
		bool bSoloKept = HOLGameStateTest::KeepAbsentSlot(Solo, Player, 0.0, 0.0);
		T.AssertFalse(bSoloKept, "solo: not kept");
	}
	HOLTestWorld::DestroyTestWorld(TestWorld);
}

// The banner of the absent players (R7.5): the texts, the notices, the clock behind, back for 5 s, the defeat.
// Partial: FormatAbsentLine / FormatBackLine (statics of FHOLDisconnectRules) are not exposed, their direct cases (161 s, 160.2 s, 59 s, -3 s,
// the back line has no time) stay in C++; their texts are read here through the banner lines.
void Test_DisconnectBanner(FUnitTest& T)
{
	// Neutral wording (the game does not know the players' gender), 2026-10-26.
	FString At161 = "Bob : connexion perdue — 2:41";
	FString Back = "Bob : de retour";
	T.AssertFalse(At161 == Back, "gone and back: two texts");

	UHOLTestWorldHandle TestWorld = HOLTestWorld::CreateTestWorld();
	T.AssertTrue(TestWorld != nullptr, "a test world is created");
	if (TestWorld == nullptr)
		return;
	AHOLGameState State = HOLTestWorld::SpawnGameState(TestWorld, 0, 0);
	AHOLPlayerState Bob = HOLTestWorld::SpawnPlayerState(TestWorld, 1, 1, "");
	T.AssertTrue(State != nullptr, "game state spawned");
	T.AssertTrue(Bob != nullptr, "player states spawned");
	if (State != nullptr && Bob != nullptr)
	{
		T.AssertTrue(HOLTestWorld::IsPropertyReplicated(State, n"AbsentNotices"), "the notices replicated (players and spectators see them)");
		T.AssertFalse(HOLTestWorld::IsPropertyReplicated(State, n"AbsentSlots"), "the stock kept is not (server only)");

		// A 1v1 (the AI seat of Bob's camp closed): the end of his window is his defeat (D718, T5: with others in his camp the AI keeps the seat).
		T.AssertTrue(HOLGameStateTest::InitVictoryFromJson(State, DisconnectTest::MakeNamedJson(true)), "the 1v1 configuration is given to InitVictory");
		FHOLResources Starting = DisconnectTest::Res(200, 200, 200, 200);
		HOLGameStateTest::GiveSlotStock(State, Bob, Starting, 0.0);
		T.AssertEquals(0, HOLGameStateTest::CountAbsentNotices(State), "first login: no notice");

		bool bKept = HOLGameStateTest::KeepAbsentSlot(State, Bob, 10.0, 180.0);
		T.AssertTrue(bKept, "Bob leaves at 10");
		Bob.DestroyActor();	// as after a Logout
		// Spawned after the Destroy: SpawnPlayerState sets the slot at once (a live second holder of the seat would make KeepAbsentSlot a duplicate, T4).
		AHOLPlayerState BobAgain = HOLTestWorld::SpawnPlayerState(TestWorld, 1, 1, "");
		T.AssertTrue(BobAgain != nullptr, "the second player state is spawned");
		TArray<FString> Notices = HOLGameStateTest::GetAbsentNotices(State);
		T.AssertEquals(1, Notices.Num(), "one notice");
		if (Notices.Num() == 1)
		{
			T.AssertEquals("1", DisconnectTest::Field(Notices[0], 0), "his slot");
			T.AssertEquals("Bob", DisconnectTest::Field(Notices[0], 1), "his name");
			T.AssertEquals("190.000", DisconnectTest::Field(Notices[0], 2), "the window's end at 190");
			T.AssertEquals("0.000", DisconnectTest::Field(Notices[0], 3), "not back");
		}
		// A client's clock of the server a little behind at the departure: never more than the window (3:01 seen in lobby_match -Rejoin, 2026-10-26).
		TArray<FString> At9 = HOLGameStateTest::GetAbsenceBannerLines(State, 9.6);
		T.AssertEquals(1, At9.Num(), "at 9.6: one line");
		if (At9.Num() == 1)
			T.AssertEquals("Bob : connexion perdue — 3:00", At9[0], "at 9.6 (clock behind): 3:00, not 3:01");
		TArray<FString> At29 = HOLGameStateTest::GetAbsenceBannerLines(State, 29.0);
		T.AssertEquals(1, At29.Num(), "at 29: one line");
		if (At29.Num() == 1)
			T.AssertEquals(At161, At29[0], "at 29: Bob, 2:41");

		bool bBack = HOLGameStateTest::GiveSlotStock(State, BobAgain, Starting, 100.0);
		T.AssertTrue(bBack, "back at 100");
		TArray<FString> At101 = HOLGameStateTest::GetAbsenceBannerLines(State, 101.0);
		T.AssertEquals(1, At101.Num(), "at 101: one line");
		if (At101.Num() == 1)
			T.AssertEquals(Back, At101[0], "at 101: Bob is back");
		TArray<FString> At1049 = HOLGameStateTest::GetAbsenceBannerLines(State, 104.9);
		T.AssertEquals(1, At1049.Num(), "at 104.9: still back");
		TArray<FString> At105 = HOLGameStateTest::GetAbsenceBannerLines(State, 105.0);
		T.AssertEquals(0, At105.Num(), "at 105: nothing (even before the server drops it)");
		HOLGameStateTest::DefeatExpiredAbsences(State, 105.0, 180.0);
		T.AssertEquals(0, HOLGameStateTest::CountAbsentNotices(State), "at 105: the notice dropped");

		bool bLeavesAgain = HOLGameStateTest::KeepAbsentSlot(State, BobAgain, 200.0, 180.0);
		T.AssertTrue(bLeavesAgain, "Bob leaves again at 200");
		T.AssertEquals(1, HOLGameStateTest::CountAbsentNotices(State), "his notice again");
		TArray<int> Defeated = HOLGameStateTest::DefeatExpiredAbsences(State, 380.0, 180.0);
		T.AssertTrue(DisconnectTest::IsOnly(Defeated, 1), "not back by 380: defeated");
		T.AssertEquals(0, HOLGameStateTest::CountAbsentNotices(State), "defeated: no notice left (the defeat says the rest)");
		TArray<FString> At381 = HOLGameStateTest::GetAbsenceBannerLines(State, 381.0);
		T.AssertEquals(0, At381.Num(), "defeated: no line");
	}
	HOLTestWorld::DestroyTestWorld(TestWorld);
}

// R7.10: back within the window, his hero's private sheet (XP, cooldowns, death step) is in his new player state at once,
// as his stock: the hero subsystem keeps the true sheet while he is gone, the return writes its copy (no step needed).
// Partial: the holder's "IsABot" has no script twin (here: a player state that is not null and not Bob's).
void Test_DisconnectHeroSheet(FUnitTest& T)
{
	UHOLTestWorldHandle TestWorld = HOLTestWorld::CreateTestWorld();
	T.AssertTrue(TestWorld != nullptr, "a test world is created");
	if (TestWorld == nullptr)
		return;
	AHOLGameState State = HOLTestWorld::SpawnGameState(TestWorld, 0, 0);
	AHOLPlayerState Bob = HOLTestWorld::SpawnPlayerState(TestWorld, 1, 1, "");
	T.AssertTrue(State != nullptr, "game state");
	T.AssertTrue(Bob != nullptr, "player state");
	if (State != nullptr && Bob != nullptr)
	{
		T.AssertTrue(HOLGameStateTest::InitVictoryFromJson(State, DisconnectTest::MakeNamedJson(false)), "the configuration is given to InitVictory");
		FHOLResources Starting = DisconnectTest::Res(200, 200, 200, 200);
		HOLGameStateTest::GiveSlotStock(State, Bob, Starting, 0.0);

		// His hero earns XP (hol.Hero.SetXP, the real entry): straight to his player state.
		T.AssertTrue(HOLTestWorld::SetHeroXP(TestWorld, 1, 12), "SetXP(1, 12)");
		int BobXP = HOLGameStateTest::GetPlayerHeroXP(Bob);
		T.AssertEquals(12, BobXP, "his XP in his player state");

		bool bKept = HOLGameStateTest::KeepAbsentSlot(State, Bob, 10.0, 180.0);
		T.AssertTrue(bKept, "Bob leaves at 10: slot kept");
		Bob.DestroyActor();
		// D717 (T2): the player state of slot 1 while he is gone is the AI's that holds his seat (not his).
		AHOLPlayerState Holder = HOLGameStateTest::GetPlayerStateOfSlot(State, 1);
		T.AssertTrue(Holder != nullptr && Holder != Bob, "only the AI's player state is left for slot 1 while he is gone");
		int KeptXP = HOLTestWorld::GetHeroSheetXP(TestWorld, 1);
		T.AssertEquals(12, KeptXP, "the subsystem keeps his XP while he is gone");

		// Back at 100 with a new player state (PostLogin's order: his slot, then the stock); no simulation step.
		AHOLPlayerState BobAgain = HOLTestWorld::SpawnPlayerState(TestWorld, 1, 1, "");
		T.AssertTrue(BobAgain != nullptr, "new player state");
		if (BobAgain != nullptr)
		{
			int EmptyXP = HOLGameStateTest::GetPlayerHeroXP(BobAgain);
			T.AssertEquals(0, EmptyXP, "a new player state starts empty");
			bool bBack = HOLGameStateTest::GiveSlotStock(State, BobAgain, Starting, 100.0);
			T.AssertTrue(bBack, "back at 100");
			int BackXP = HOLGameStateTest::GetPlayerHeroXP(BobAgain);
			T.AssertEquals(12, BackXP, "back: his XP in his new player state at once");
			T.AssertTrue(HOLTestWorld::HeroSheetEqualsPlayer(TestWorld, 1, BobAgain), "back: his whole private sheet is the subsystem's");
		}
	}
	HOLTestWorld::DestroyTestWorld(TestWorld);
}

// The pauses and the window's clock (the clock stands still during a pause; a departure lifts only the leaver's own pause).
void Test_DisconnectPause(FUnitTest& T)
{
	UHOLTestWorldHandle TestWorld = HOLTestWorld::CreateTestWorld();
	T.AssertTrue(TestWorld != nullptr, "a test world is created");
	if (TestWorld == nullptr)
		return;
	AHOLGameState State = HOLTestWorld::SpawnGameState(TestWorld, 0, 0);
	AHOLPlayerState Alice = HOLTestWorld::SpawnPlayerState(TestWorld, 0, 0, "");
	AHOLPlayerState Bob = HOLTestWorld::SpawnPlayerState(TestWorld, 1, 1, "");
	T.AssertTrue(State != nullptr, "game state");
	T.AssertTrue(Alice != nullptr, "Alice");
	T.AssertTrue(Bob != nullptr, "Bob");
	if (State != nullptr && Alice != nullptr && Bob != nullptr)
	{
		T.AssertTrue(HOLGameStateTest::InitVictoryFromJson(State, DisconnectTest::MakeNamedJson(true)), "the 1v1 configuration is given to InitVictory");
		FHOLResources Starting = DisconnectTest::Res(200, 200, 200, 200);
		HOLGameStateTest::GiveSlotStock(State, Alice, Starting, 0.0);
		HOLGameStateTest::GiveSlotStock(State, Bob, Starting, 0.0);

		T.AssertTrue(HOLGameStateTest::GetWindowClock(State, 50.0) == 50.0, "no pause: the window clock is the time");
		// Alice pauses at 100 and resumes at 160: the clock stood still for 60 s; Bob's departure lifts nobody else's pause.
		T.AssertTrue(HOLGameStateTest::RequestPause(State, Alice, 100.0), "Alice pauses");
		T.AssertTrue(HOLGameStateTest::GetWindowClock(State, 130.0) == 100.0, "paused: the window clock stands still");
		T.AssertFalse(HOLGameStateTest::LiftPauseOf(State, 1, 130.0), "Bob's departure lifts only his own pause");
		T.AssertTrue(HOLGameStateTest::IsMatchPaused(State), "Alice's pause stays");
		T.AssertTrue(HOLGameStateTest::RequestResume(State, Alice, 160.0), "Alice resumes");
		T.AssertTrue(HOLGameStateTest::GetWindowClock(State, 200.0) == 140.0, "resumed: 60 s lost to the window");

		// Bob pauses at 300 and leaves at 305: the match runs again for the others, the pause stays counted.
		T.AssertTrue(HOLGameStateTest::RequestPause(State, Bob, 300.0), "Bob pauses");
		int PausesAtLeaving = HOLGameStateTest::GetPausesLeft(State);
		T.AssertTrue(HOLGameStateTest::LiftPauseOf(State, 1, 305.0), "Bob leaves: his pause is lifted");
		T.AssertFalse(HOLGameStateTest::IsMatchPaused(State), "the match runs again");
		int PausesAfter = HOLGameStateTest::GetPausesLeft(State);
		T.AssertEquals(PausesAtLeaving, PausesAfter, "his pause stays counted");
		T.AssertTrue(HOLGameStateTest::GetWindowClock(State, 310.0) == 310.0 - 60.0 - 5.0, "5 s more lost to the window");
		T.AssertFalse(HOLGameStateTest::LiftPauseOf(State, 1, 306.0), "nothing to lift twice");

		// A window of 10 s from 310 (clock 245), with a pause of Alice's of 5 s inside it, ends at 325 of the time (10 s + 5 s).
		bool bAway = HOLGameStateTest::KeepAbsentSlot(State, Bob, HOLGameStateTest::GetWindowClock(State, 310.0), 10.0);
		T.AssertTrue(bAway, "Bob is away");
		Bob.DestroyActor();
		T.AssertTrue(HOLGameStateTest::RequestPause(State, Alice, 312.0), "Alice pauses");
		T.AssertTrue(HOLGameStateTest::RequestResume(State, Alice, 317.0), "Alice resumes");
		TArray<int> Before = HOLGameStateTest::DefeatExpiredAbsences(State, HOLGameStateTest::GetWindowClock(State, 324.9), 10.0);
		T.AssertEquals(0, Before.Num(), "14.9 s of time: the window is not over (5 s were paused)");
		TArray<int> After = HOLGameStateTest::DefeatExpiredAbsences(State, HOLGameStateTest::GetWindowClock(State, 325.0), 10.0);
		T.AssertTrue(DisconnectTest::IsOnly(After, 1), "15 s of time: the window is over");
	}
	HOLTestWorld::DestroyTestWorld(TestWorld);
}

// D720 (T6, cosmetic): the banner of the clients counts down on the window's clock (the time minus the pauses), as the server's window does.
// A pause of 60 s inside Bob's window of 180 s: seen at the server time 200 the banner reads 0:50 (clock 140, end 190), not 0:00; during a
// pause the figure stands still; the server's figure of the pauses is replicated (a client cannot guess it).
void Test_DisconnectBannerClock(FUnitTest& T)
{
	UHOLTestWorldHandle TestWorld = HOLTestWorld::CreateTestWorld();
	T.AssertTrue(TestWorld != nullptr, "a test world is created");
	if (TestWorld == nullptr)
		return;
	AHOLGameState State = HOLTestWorld::SpawnGameState(TestWorld, 0, 0);
	AHOLPlayerState Alice = HOLTestWorld::SpawnPlayerState(TestWorld, 0, 0, "");
	AHOLPlayerState Bob = HOLTestWorld::SpawnPlayerState(TestWorld, 1, 1, "");
	T.AssertTrue(State != nullptr, "game state");
	T.AssertTrue(Alice != nullptr, "Alice");
	T.AssertTrue(Bob != nullptr, "Bob");
	if (State != nullptr && Alice != nullptr && Bob != nullptr)
	{
		T.AssertTrue(HOLTestWorld::IsPropertyReplicated(State, n"WindowPausedSeconds"), "the seconds of the pauses are replicated (the clients' banner reads them)");
		T.AssertTrue(HOLGameStateTest::InitVictoryFromJson(State, DisconnectTest::MakeNamedJson(true)), "the 1v1 configuration is given to InitVictory");
		FHOLResources Starting = DisconnectTest::Res(200, 200, 200, 200);
		HOLGameStateTest::GiveSlotStock(State, Alice, Starting, 0.0);
		HOLGameStateTest::GiveSlotStock(State, Bob, Starting, 0.0);
		bool bKept = HOLGameStateTest::KeepAbsentSlot(State, Bob, HOLGameStateTest::GetWindowClock(State, 10.0), 180.0);
		T.AssertTrue(bKept, "Bob leaves at the clock 10 (his window ends at 190)");
		Bob.DestroyActor();

		T.AssertEquals("Bob : connexion perdue — 1:30", DisconnectTest::BannerAt(State, 100.0), "no pause, time 100: 1:30");
		T.AssertTrue(HOLGameStateTest::RequestPause(State, Alice, 100.0), "Alice pauses at 100");
		T.AssertEquals("Bob : connexion perdue — 1:30", DisconnectTest::BannerAt(State, 130.0), "paused, time 130: the figure stands still (1:30)");
		T.AssertTrue(HOLGameStateTest::RequestResume(State, Alice, 160.0), "Alice resumes at 160");
		T.AssertEquals("Bob : connexion perdue — 0:50", DisconnectTest::BannerAt(State, 200.0), "time 200, 60 s paused: 0:50 and not 0:00");
	}
	HOLTestWorld::DestroyTestWorld(TestWorld);
}

// T9: hol.Disconnect.Report on the real chain says how the AI plays the seat it keeps: "hold" in the window (the console Leave), "full brain"
// when the window ends (DefeatExpiredAbsences, Bob's camp has an AI seat), nothing once he is back. The lines are the ones the command logs.
void Test_DisconnectReportMode(FUnitTest& T)
{
	UHOLTestWorldHandle TestWorld = HOLTestWorld::CreateTestWorld();
	T.AssertTrue(TestWorld != nullptr, "a test world is created");
	if (TestWorld == nullptr)
		return;
	AHOLGameState State = HOLTestWorld::SpawnGameState(TestWorld, 0, 0);
	AHOLPlayerState Alice = HOLTestWorld::SpawnPlayerState(TestWorld, 0, 0, "");
	T.AssertTrue(State != nullptr, "game state");
	T.AssertTrue(Alice != nullptr, "Alice");
	if (State != nullptr && Alice != nullptr)
	{
		T.AssertTrue(HOLGameStateTest::InitVictoryFromJson(State, DisconnectTest::MakeNamedJson(false)), "the configuration is given to InitVictory");
		HOLGameStateTest::AddPlayerState(State, Alice);
		T.AssertFalse(DisconnectTest::BobLine(State).Contains("AI mode"), "present: no AI mode on his line");
		HOLTestWorld::RunConsoleCommand(TestWorld, "hol.Net.FakeAbsent Leave 1");
		T.AssertTrue(DisconnectTest::BobLine(State).EndsWith("AI mode hold"), "in the window: the AI plays the hold mode");
		HOLGameStateTest::DefeatExpiredAbsences(State, 1000.0, 180.0);
		T.AssertTrue(DisconnectTest::BobLine(State).EndsWith("AI mode full brain"), "window over: the AI plays the full brain");
		HOLTestWorld::RunConsoleCommand(TestWorld, "hol.Net.FakeAbsent Back 1");
		T.AssertFalse(DisconnectTest::BobLine(State).Contains("AI mode"), "back: no AI mode on his line");
	}
	HOLTestWorld::DestroyTestWorld(TestWorld);
}
