// Script port of the four tests of HOLDisconnectDisplayTests.cpp (D853, lot 2 of PLAN-TESTS-1119, ELROND-1119t): same cases, same expected values.
//   HeroesOfLiberty.Match.DisconnectTable        -> Test_DisconnectTable
//   HeroesOfLiberty.Match.DisconnectMessage      -> Test_DisconnectMessage
//   HeroesOfLiberty.Match.DisconnectLeftIsDefeat -> Test_DisconnectLeftIsDefeat
//   HeroesOfLiberty.Match.DisconnectReport       -> Test_DisconnectReport
// The configuration is built as JSON (HOLMatchConfigRead), the rules are read through HOLGameStateTest (UHOLGameStateTestLibrary): the table rows
// as one tab-joined line a row (Name, State, bAway, bAIHeld, Shown), the seat report, the game state and the recorded forum events as ints.
// The C++ file stays until this one has been seen red then green (D853: the main session removes it).

namespace DisconnectDisplayTest
{
	// EHOLSlotState: 0 Closed, 1 Human, 2 AI. EHOLDefeatReason: 1 Surrender, 5 Disconnected. FHOLForumEventValue.Kind 3 = Defeated, Reason 1 = Disconnected.
	const int SlotClosed = 0;
	const int SlotHuman = 1;
	const int SlotAI = 2;

	// MakeFourHumans of the C++ file: Alice, Bob, Cleo, Dan, colours 0 to 3, the slots from 4 closed. The teams and states of a slot can be changed after.
	FString MakeFourHumans()
	{
		FString Json = HOLMatchConfigRead::MakeDefaultJson();
		TArray<FString> Names;
		Names.Add("Alice");
		Names.Add("Bob");
		Names.Add("Cleo");
		Names.Add("Dan");
		for (int Slot = 0; Slot < 4; Slot++)
			Json = HOLMatchConfigRead::SetSlotJson(Json, Slot, SlotHuman, 0, Slot, 0, Names[Slot]);
		for (int Slot = 4; Slot < HOLMatchConfigRead::GetMaxSlots(); Slot++)
			Json = HOLMatchConfigRead::SetSlotJson(Json, Slot, SlotClosed, 0, Slot, 0, "");
		return Json;
	}

	// The scenario the forum messages read: the four seats' states, teams and names (HOLForumMessagesRead::GetMessage).
	FHOLForumScenario MakeScenario(int State2, int State3, int Team0, int Team1, int Team2)
	{
		FHOLForumScenario Scenario;
		Scenario.States.Add(SlotHuman);
		Scenario.States.Add(SlotHuman);
		Scenario.States.Add(State2);
		Scenario.States.Add(State3);
		Scenario.Teams.Add(Team0);
		Scenario.Teams.Add(Team1);
		Scenario.Teams.Add(Team2);
		Scenario.Teams.Add(0);
		Scenario.Names.Add("Alice");
		Scenario.Names.Add("Bob");
		Scenario.Names.Add("Cleo");
		Scenario.Names.Add("Dan");
		return Scenario;
	}

	FHOLForumEventValue MakeDefeatedEvent(int Slot, int Reason)
	{
		FHOLForumEventValue Event;
		Event.Kind = 3;
		Event.Slot = Slot;
		Event.OtherSlot = -1;
		Event.Lives = 0;
		Event.Reason = Reason;
		return Event;
	}

	FHOLForumMessageValue Message(FHOLForumEventValue Event, FHOLForumScenario Scenario, int Viewer)
	{
		return HOLForumMessagesRead::GetMessage(Event, Scenario, Viewer, false);
	}

	// One field of a row line (Name, State, bAway, bAIHeld, Shown).
	FString Field(const FString& Line, int Index)
	{
		TArray<FString> Parts;
		Line.ParseIntoArray(Parts, "\t", false);
		if (Index >= Parts.Num())
			return "<missing>";
		return Parts[Index];
	}
}

// T8 (D722): the players' table keeps the name and adds a pastille: "Connexion perdue" while the window runs, "IA" once the AI keeps the seat.
void Test_DisconnectTable(FUnitTest& T)
{
	// Bob (1) away, Cleo (2) held by the AI, Dan (3) defeated while away (no pastille), Alice (0) present: defeated mask 1<<3, local slot 0,
	// absent mask (1<<1)|(1<<3), AI-held mask 1<<2.
	TArray<FString> Rows = HOLGameStateTest::GetPlayersTableRows(DisconnectDisplayTest::MakeFourHumans(), 1 << 3, 0, (1 << 1) | (1 << 3), 1 << 2);
	T.AssertEquals(4, Rows.Num(), "one row per seat");
	if (Rows.Num() != 4)
		return;
	// Fields: 0 Name, 1 State, 2 bAway, 3 bAIHeld, 4 Shown.
	T.AssertEquals("0", DisconnectDisplayTest::Field(Rows[0], 2), "present: no pastille (not away)");
	T.AssertEquals("0", DisconnectDisplayTest::Field(Rows[0], 3), "present: no pastille (not AI-held)");
	T.AssertEquals("1", DisconnectDisplayTest::Field(Rows[1], 2), "away: pastille");
	T.AssertEquals("0", DisconnectDisplayTest::Field(Rows[1], 3), "away: not AI-held");
	T.AssertEquals("Bob", DisconnectDisplayTest::Field(Rows[1], 0), "away: the name stays");
	T.AssertEquals("Connexion perdue", DisconnectDisplayTest::Field(Rows[1], 1), "away: the state says so");
	T.AssertTrue(DisconnectDisplayTest::Field(Rows[1], 4).Contains(" away"), "away: in the report");
	T.AssertEquals("1", DisconnectDisplayTest::Field(Rows[2], 3), "AI holds: pastille");
	T.AssertEquals("0", DisconnectDisplayTest::Field(Rows[2], 2), "AI holds: not away");
	T.AssertEquals("IA", DisconnectDisplayTest::Field(Rows[2], 1), "AI holds: the state says IA");
	T.AssertTrue(DisconnectDisplayTest::Field(Rows[2], 4).Contains(" ai-held"), "AI holds: in the report");
	T.AssertEquals("0", DisconnectDisplayTest::Field(Rows[3], 2), "defeated: no pastille (not away)");
	T.AssertEquals("0", DisconnectDisplayTest::Field(Rows[3], 3), "defeated: no pastille (not AI-held)");
	T.AssertEquals("Vaincu", DisconnectDisplayTest::Field(Rows[3], 1), "defeated: Vaincu");
}

// T8 (D722): the defeat of a player who left and did not come back has its own text, not "vaincu".
void Test_DisconnectMessage(FUnitTest& T)
{
	// Answer B (Q-disconnect-left): one kind, Defeated, and a Reason that picks the text (Reason 1 = Disconnected).
	FHOLForumScenario Scenario = DisconnectDisplayTest::MakeScenario(DisconnectDisplayTest::SlotHuman, DisconnectDisplayTest::SlotHuman, 0, 0, 0);
	FHOLForumEventValue Left = DisconnectDisplayTest::MakeDefeatedEvent(1, 1);
	FHOLForumMessageValue SeenByAnother = DisconnectDisplayTest::Message(Left, Scenario, 0);
	FHOLForumMessageValue SeenByHimself = DisconnectDisplayTest::Message(Left, Scenario, 1);
	T.AssertEquals("Bob a quitté la partie", SeenByAnother.Source, "seen by another");
	T.AssertEquals("Vous avez quitté la partie", SeenByHimself.Source, "seen by himself");
	T.AssertFalse(SeenByAnother.Source.Contains("vaincu"), "not the plain defeat");
	T.AssertFalse(SeenByHimself.bAlert, "his own leaving is no alert for himself");
	FHOLForumEventValue Beaten = DisconnectDisplayTest::MakeDefeatedEvent(1, 0);
	FHOLForumMessageValue Plain = DisconnectDisplayTest::Message(Beaten, Scenario, 0);
	T.AssertEquals("Bob est vaincu", Plain.Source, "no reason: the plain defeat");
}

// Answer B on the real chain: DefeatSlot(Disconnected) sends ONE Defeated event carrying the reason, the text written for the viewers says
// "a quitté la partie", and the statistics count it as the seat's one defeat (D508: one line per seat).
void Test_DisconnectLeftIsDefeat(FUnitTest& T)
{
	UHOLTestWorldHandle TestWorld = HOLTestWorld::CreateTestWorld();
	T.AssertTrue(TestWorld != nullptr, "a test world is created");
	if (TestWorld == nullptr)
		return;
	AHOLGameState State = HOLTestWorld::SpawnGameState(TestWorld, 0, 0);
	T.AssertTrue(State != nullptr, "game state");
	if (State != nullptr)
	{
		// Alice team 1, Bob team 2, Cleo AI team 2, Dan closed.
		FString Json = DisconnectDisplayTest::MakeFourHumans();
		Json = HOLMatchConfigRead::SetSlotJson(Json, 0, DisconnectDisplayTest::SlotHuman, 0, 0, 1, "Alice");
		Json = HOLMatchConfigRead::SetSlotJson(Json, 1, DisconnectDisplayTest::SlotHuman, 0, 1, 2, "Bob");
		Json = HOLMatchConfigRead::SetSlotJson(Json, 2, DisconnectDisplayTest::SlotAI, 0, 2, 2, "Cleo");
		Json = HOLMatchConfigRead::SetSlotJson(Json, 3, DisconnectDisplayTest::SlotClosed, 0, 3, 0, "Dan");
		T.AssertTrue(HOLGameStateTest::InitVictoryFromJson(State, Json), "the match configuration is read and given to InitVictory");
		UHOLForumEventRecorder Events = HOLGameStateTest::RecordForumEvents(State);
		T.AssertTrue(Events != nullptr, "the forum events are recorded");
		FHOLForumScenario Scenario = DisconnectDisplayTest::MakeScenario(DisconnectDisplayTest::SlotAI, DisconnectDisplayTest::SlotClosed, 1, 2, 2);

		bool bBobDefeated = HOLGameStateTest::DefeatSlot(State, 1, 5);
		T.AssertTrue(bBobDefeated, "Bob's seat is defeated (Disconnected)");
		int Count = Events.GetCount();
		T.AssertEquals(1, Count, "one event");
		if (Count == 1)
		{
			int Kind = Events.GetKind(0);
			int Reason = Events.GetReason(0);
			T.AssertEquals(3, Kind, "the kind is Defeated");
			T.AssertEquals(1, Reason, "it carries the reason");
			FHOLForumEventValue First = DisconnectDisplayTest::MakeDefeatedEvent(Events.GetSlot(0), Reason);
			FHOLForumMessageValue Read = DisconnectDisplayTest::Message(First, Scenario, 0);
			T.AssertEquals("Bob a quitté la partie", Read.Source, "the text Alice reads");
		}
		int Defeats = HOLGameStateTest::CountStatsDefeats(State, 1);
		T.AssertEquals(1, Defeats, "the timeline counts one defeat for Bob's seat");

		// A surrender stays "vaincu" and counts the same way.
		bool bAliceDefeated = HOLGameStateTest::DefeatSlot(State, 0, 1);
		T.AssertTrue(bAliceDefeated, "Alice gives up");
		int CountAfter = Events.GetCount();
		T.AssertEquals(2, CountAfter, "a second event");
		if (CountAfter == 2)
		{
			int Kind2 = Events.GetKind(1);
			int Reason2 = Events.GetReason(1);
			T.AssertEquals(3, Kind2, "... a Defeated event");
			T.AssertEquals(0, Reason2, "... the plain defeat, no reason");
		}
	}
	// No assertion stops the test (they record and go on), so the world is always destroyed here.
	HOLTestWorld::DestroyTestWorld(TestWorld);
}

// T9: the lines of hol.Disconnect.Report. SeatState: 0 Present, 1 AbsentAIHolds, 2 AIHolds, 3 Defeated; AIMode: 0 None, 1 Hold, 2 Full.
void Test_DisconnectReport(FUnitTest& T)
{
	FString Line = HOLGameStateTest::FormatSeatReport(0, "Alice", 0, 0.0, 0);
	T.AssertEquals("slot 0 (Alice) present", Line, "present");
	Line = HOLGameStateTest::FormatSeatReport(1, "Bob", 1, 161.2, 0);
	T.AssertEquals("slot 1 (Bob) away, AI holds, 162 s left", Line, "away, AI holds");
	Line = HOLGameStateTest::FormatSeatReport(2, "Cleo", 2, 0.0, 0);
	T.AssertEquals("slot 2 (Cleo) away, window over, the AI keeps the seat", Line, "window over");
	Line = HOLGameStateTest::FormatSeatReport(3, "Dan", 3, 0.0, 0);
	T.AssertEquals("slot 3 (Dan) defeated", Line, "defeated");

	// The AI's mode ends the line of a seat it keeps (the report said who holds the seat, not how the AI plays it).
	Line = HOLGameStateTest::FormatSeatReport(1, "Bob", 1, 161.2, 1);
	T.AssertEquals("slot 1 (Bob) away, AI holds, 162 s left, AI mode hold", Line, "away, hold mode");
	Line = HOLGameStateTest::FormatSeatReport(2, "Cleo", 2, 0.0, 2);
	T.AssertEquals("slot 2 (Cleo) away, window over, the AI keeps the seat, AI mode full brain", Line, "window over, full brain");
	Line = HOLGameStateTest::FormatSeatReport(0, "Alice", 0, 0.0, 0);
	T.AssertEquals("slot 0 (Alice) present", Line, "present: no mode");
}
