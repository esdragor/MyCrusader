// Script port, part 2, of HOLDisconnectTests.cpp (D853, ELROND-1119z): the tests the first file (HOLDisconnect_Test.as) did not hold.
//   DisconnectRules -> Test_DisconnectRules; DisconnectWindowEnd -> Test_DisconnectWindowEnd; DisconnectGains -> Test_DisconnectGains;
//   DisconnectTechnologies -> Test_DisconnectTechnologies; DisconnectHeld -> Test_DisconnectHeld; DisconnectHeldStock -> Test_DisconnectHeldStock;
//   DisconnectLateReturn -> Test_DisconnectLateReturn; DisconnectReplace -> Test_DisconnectReplace; DisconnectFakeBack -> Test_DisconnectFakeBack.
// Same values as the C++; the differences are said at each test. The C++ tests stay until the main session has seen these red then green (it then removes them).
// SpawnPlayerStateNoSlot + SetPlayerMatchSlot is the C++ order (SetMatchSlot later than the spawn), so no player state is spawned "after the Destroy".

namespace DisconnectTest2
{
	// EHOLResource: Food 0, Wood 1, Stone 2, Gold 3. FHOLDisconnectRules::ESeatState: Present 0, AbsentAIHolds 1, AIHolds 2, Defeated 3.
	// EWindowEnd: Defeat 0, AIHolds 1.
	const int Wood = 1;
	const int Food = 0;
	const int Defeat = 0;
	const int AIHolds = 1;

	// Seats = the team of each human slot (0 = no team); a negative one is an AI of the team -value (the C++ Make lambda).
	FString MakeTeamsJson(TArray<int> Seats)
	{
		FString Json = HOLMatchConfigRead::MakeDefaultJson();
		for (int Slot = 0; Slot < HOLMatchConfigRead::GetMaxSlots(); Slot++)
			Json = HOLMatchConfigRead::SetSlotJson(Json, Slot, 0, 0, Slot, 0, "");
		for (int Index = 0; Index < Seats.Num(); Index++)
		{
			int Team = Seats[Index] < 0 ? -Seats[Index] : Seats[Index];
			if (Seats[Index] < 0)
				Json = HOLMatchConfigRead::SetSlotJson(Json, Index, 2, 0, Index, Team, "");
			else
				Json = HOLMatchConfigRead::SetSlotJson(Json, Index, 1, 0, Index, Team, f"P{Index}");
		}
		return Json;
	}

	int End(TArray<int> Seats, int Slot)
	{
		return HOLGameStateTest::RulesGetWindowEnd(MakeTeamsJson(Seats), Slot);
	}

	// The same in a match mode (0 Standard, 1 Diplomacy): the JSON setters do not carry the mode, the envelope sets it after the read.
	int EndInMode(TArray<int> Seats, int Slot, int Mode)
	{
		return HOLGameStateTest::RulesGetWindowEndInMode(MakeTeamsJson(Seats), Slot, Mode);
	}

	// The 1v1 named configuration (Alice 0 team 1, Bob 1 team 2) when bOneVsOne, else with the AI seat 2 of Bob's camp.
	FString NamedJson(bool bOneVsOne)
	{
		return DisconnectTest::MakeNamedJson(bOneVsOne);
	}

	bool IsExactly(TArray<int> Slots, TArray<int> Expected)
	{
		if (Slots.Num() != Expected.Num())
			return false;
		for (int Index = 0; Index < Slots.Num(); Index++)
		{
			if (Slots[Index] != Expected[Index])
				return false;
		}
		return true;
	}

	TArray<int> Ints1(int A)
	{
		TArray<int> Result;
		Result.Add(A);
		return Result;
	}

	TArray<int> Ints2(int A, int B)
	{
		TArray<int> Result;
		Result.Add(A);
		Result.Add(B);
		return Result;
	}

	// A player state of a slot with no slot yet at the spawn, then the slot (the order the C++ tests follow).
	AHOLPlayerState Seat(UHOLTestWorldHandle TestWorld, AHOLGameState State, int Slot, bool bAdd)
	{
		AHOLPlayerState Player = HOLTestWorld::SpawnPlayerStateNoSlot(TestWorld, "");
		if (Player != nullptr)
		{
			HOLGameStateTest::SetPlayerMatchSlot(Player, Slot, Slot);
			if (bAdd)
				HOLGameStateTest::AddPlayerState(State, Player);
		}
		return Player;
	}

	// Is this very player state in the game state's PlayerArray (what the C++ test reads: "Each == Bob")? Not a count by seat:
	// when a seat is kept, the game spawns the AI's player state for it (AHOLGameState::SpawnHeldBot, D717 T2), which has the same PlayerIndex.
	bool IsListed(AHOLGameState State, AHOLPlayerState Player)
	{
		for (int Index = 0; Index < State.PlayerArray.Num(); Index++)
		{
			// The array's elements are TObjectPtr<APlayerState>: Get() gives the handle.
			APlayerState Each = State.PlayerArray[Index].Get();
			if (Each == Player)
				return true;
		}
		return false;
	}
}

// D718 (T1): the end of the window by the camps of the launch. (Whole: the two Diplomacy assertions go through RulesGetWindowEndInMode, the JSON setters not carrying the mode.)
void Test_DisconnectWindowEnd(FUnitTest& T)
{
	TArray<int> S;
	T.AssertTrue(DisconnectTest2::End(DisconnectTest2::Ints2(1, 2), 0) == DisconnectTest2::Defeat && DisconnectTest2::End(DisconnectTest2::Ints2(1, 2), 1) == DisconnectTest2::Defeat, "1v1: defeat");
	T.AssertTrue(DisconnectTest2::End(DisconnectTest2::Ints2(0, 0), 0) == DisconnectTest2::Defeat, "1v1 free for all (no teams): defeat");
	TArray<int> TwoVsTwo = DisconnectTest2::Ints2(1, 1);
	TwoVsTwo.Add(2);
	TwoVsTwo.Add(2);
	T.AssertTrue(DisconnectTest2::End(TwoVsTwo, 0) == DisconnectTest2::AIHolds && DisconnectTest2::End(TwoVsTwo, 3) == DisconnectTest2::AIHolds, "2v2: the AI keeps the seat");
	TArray<int> Ffa3 = DisconnectTest2::Ints2(0, 0);
	Ffa3.Add(0);
	T.AssertTrue(DisconnectTest2::End(Ffa3, 1) == DisconnectTest2::AIHolds, "free for all of 3: the AI keeps the seat");
	TArray<int> Ffa8;
	for (int Index = 0; Index < 8; Index++)
		Ffa8.Add(0);
	T.AssertTrue(DisconnectTest2::End(Ffa8, 7) == DisconnectTest2::AIHolds, "free for all of 8: the AI keeps the seat");
	TArray<int> OneVsTwo = DisconnectTest2::Ints2(1, 2);
	OneVsTwo.Add(2);
	T.AssertTrue(DisconnectTest2::End(OneVsTwo, 0) == DisconnectTest2::Defeat, "1 against 2: the one alone loses");
	T.AssertTrue(DisconnectTest2::End(OneVsTwo, 1) == DisconnectTest2::AIHolds, "1 against 2: one of the two: the AI keeps the seat");
	TArray<int> OneVsThree = DisconnectTest2::Ints2(1, 2);
	OneVsThree.Add(2);
	OneVsThree.Add(2);
	TArray<int> ThreeCamps = DisconnectTest2::Ints2(1, 2);
	ThreeCamps.Add(3);
	T.AssertTrue(DisconnectTest2::End(OneVsThree, 2) == DisconnectTest2::AIHolds && DisconnectTest2::End(ThreeCamps, 0) == DisconnectTest2::AIHolds, "1 against 3 and 1 against 1 against 1: three camps or the camp of two: the AI keeps");
	// A human with an AI seat in his camp (Bob and an AI against Alice).
	TArray<int> WithAI = DisconnectTest2::Ints2(1, 2);
	WithAI.Add(-2);
	T.AssertTrue(DisconnectTest2::End(WithAI, 1) == DisconnectTest2::AIHolds, "a human with an AI of his camp against one: the AI keeps");
	T.AssertTrue(DisconnectTest2::End(WithAI, 0) == DisconnectTest2::Defeat, "... the one alone against that camp of two: defeat");
	T.AssertTrue(DisconnectTest2::End(DisconnectTest2::Ints2(1, -2), 0) == DisconnectTest2::Defeat, "a human against an AI alone (two camps): defeat");
	T.AssertTrue(DisconnectTest2::End(DisconnectTest2::Ints2(1, 2), 5) == DisconnectTest2::Defeat && DisconnectTest2::End(DisconnectTest2::Ints2(1, 2), -1) == DisconnectTest2::Defeat, "a closed or unknown slot holds nothing: defeat");

	// Diplomacy (mode 1): the teams are ignored, every human is a camp.
	T.AssertEquals(DisconnectTest2::AIHolds, DisconnectTest2::EndInMode(TwoVsTwo, 0, 1), "Diplomacy, 2v2 teams ignored: four camps, the AI keeps");
	T.AssertEquals(DisconnectTest2::Defeat, DisconnectTest2::EndInMode(DisconnectTest2::Ints2(1, 1), 0, 1), "Diplomacy, two players: two camps alone, defeat");

	// The seat's states: Present 0, AbsentAIHolds 1, AIHolds 2, Defeated 3.
	T.AssertTrue(HOLGameStateTest::RulesMayReturn(0, false), "present: his name takes the seat again (a second connection)");
	T.AssertTrue(HOLGameStateTest::RulesMayReturn(1, false), "away, the AI holds it (window): he may return");
	T.AssertTrue(HOLGameStateTest::RulesMayReturn(2, false), "the AI holds it after the window: he may return while the match lasts");
	T.AssertFalse(HOLGameStateTest::RulesMayReturn(2, true), "... not when the match is over");
	T.AssertFalse(HOLGameStateTest::RulesMayReturn(3, false), "defeated: he returns as a spectator only");
	T.AssertFalse(HOLGameStateTest::RulesKeepsSlot(DisconnectTest2::MakeTeamsJson(DisconnectTest2::Ints2(1, 2)), 0, false, false, true), "a seat defeated is not kept (KeepsSlot)");
}

// The pure rules of FHOLDisconnectRules (KeepsSlot, IsWindowOver, GetSecondsLeft, FindAbsent, GetExpired, GetStockOnTaking).
// Whole: DefaultWindowSeconds == 180 is read through RulesGetDefaultWindowSeconds (envelope added by GALADRIEL-1119co).
void Test_DisconnectRules(FUnitTest& T)
{
	FString Named = DisconnectTest::MakeNamedJson(false);
	T.AssertTrue(HOLGameStateTest::RulesKeepsSlot(Named, 1, false, false, false), "Bob leaves during the match: his slot kept");
	T.AssertFalse(HOLGameStateTest::RulesKeepsSlot(Named, 1, true, false, false), "still waiting for the players: not kept");
	T.AssertFalse(HOLGameStateTest::RulesKeepsSlot(Named, 1, false, true, false), "the match over: not kept");
	T.AssertFalse(HOLGameStateTest::RulesKeepsSlot(Named, 1, false, false, true), "his slot defeated: not kept");
	T.AssertFalse(HOLGameStateTest::RulesKeepsSlot(Named, 2, false, false, false), "an AI slot: not kept");
	T.AssertFalse(HOLGameStateTest::RulesKeepsSlot(Named, 4, false, false, false), "a closed slot: not kept");
	T.AssertFalse(HOLGameStateTest::RulesKeepsSlot(Named, -1, false, false, false), "no slot (spectator): not kept");
	T.AssertFalse(HOLGameStateTest::RulesKeepsSlot(HOLMatchConfigRead::MakeDefaultJson(), 0, false, false, false), "a configuration without names (solo, direct): not kept");

	T.AssertFalse(HOLGameStateTest::RulesIsWindowOver(10.0, 189.9, 180.0), "left at 10, now 189.9: still in the window");
	T.AssertTrue(HOLGameStateTest::RulesIsWindowOver(10.0, 190.0, 180.0), "left at 10, now 190: over");
	double Left = HOLGameStateTest::RulesGetSecondsLeft(10.0, 31.0, 180.0);
	T.AssertTrue(Left == 159.0, "seconds left");
	double None = HOLGameStateTest::RulesGetSecondsLeft(10.0, 500.0, 180.0);
	T.AssertTrue(None == 0.0, "no seconds left after the end");
	T.AssertEquals(180, int(HOLGameStateTest::RulesGetDefaultWindowSeconds()), "the window by default: 3 min");

	// Absent: Dan (slot 3, left at 0), Bob (slot 1, left at 50, 300 wood).
	TArray<int> Slots = DisconnectTest2::Ints2(3, 1);
	TArray<double> LeftAts;
	LeftAts.Add(0.0);
	LeftAts.Add(50.0);
	int FoundBob = HOLGameStateTest::RulesFindAbsent(Slots, 1);
	T.AssertEquals(1, FoundBob, "Bob's slot found");
	int FoundAlice = HOLGameStateTest::RulesFindAbsent(Slots, 0);
	T.AssertEquals(-1, FoundAlice, "Alice not absent");
	TArray<int> At200 = HOLGameStateTest::RulesGetExpired(Slots, LeftAts, 200.0, 180.0);
	T.AssertTrue(DisconnectTest2::IsExactly(At200, DisconnectTest2::Ints1(3)), "at 200: Dan's window over, not Bob's");
	TArray<int> At230 = HOLGameStateTest::RulesGetExpired(Slots, LeftAts, 230.0, 180.0);
	T.AssertTrue(DisconnectTest2::IsExactly(At230, DisconnectTest2::Ints2(3, 1)), "at 230: both, in the order of the departures");

	FHOLResources Starting = DisconnectTest::Res(200, 200, 200, 200);
	FHOLResources Kept = DisconnectTest::Res(0, 300, 0, 0);
	T.AssertTrue(DisconnectTest::SameRes(HOLGameStateTest::RulesGetStockOnTaking(true, Kept, Starting), Kept), "back: the stock kept");
	T.AssertTrue(DisconnectTest::SameRes(HOLGameStateTest::RulesGetStockOnTaking(false, Kept, Starting), Starting), "first login: the stock of the start");
}

// M6-Q16 (R7): what Bob gains while away is kept for him (a Peasant's deposit, a foundation's refund, a queued Peasant's cost).
// Whole (GALADRIEL-1119co: Bob is in the PlayerArray, the player state gone is counted, the load left is read). Difference: the Peasant's cost is read through GetProductionCostResources(.., slot 1) and the deposit through DepositLoad (the loaded amount is the 10).
void Test_DisconnectGains(FUnitTest& T)
{
	UHOLTestWorldHandle TestWorld = HOLTestWorld::CreateTestWorld();
	T.AssertTrue(TestWorld != nullptr, "a test world is created");
	if (TestWorld == nullptr)
		return;
	AHOLGameState State = HOLTestWorld::SpawnGameState(TestWorld, 0, 0);
	AHOLPlayerState Bob = DisconnectTest2::Seat(TestWorld, State, 1, true);
	AHOLBuilding Foundation = HOLTestWorld::SpawnBuilding(TestWorld, n"House", 1, FVector(3000.0, 0.0, 0.0), EHOLConstructionState::Foundation);
	AHOLBuilding Centre = HOLTestWorld::SpawnBuilding(TestWorld, n"MainCentre", 1, FVector(0.0, 0.0, 0.0), EHOLConstructionState::Finished);
	T.AssertTrue(State != nullptr && Bob != nullptr && Foundation != nullptr && Centre != nullptr, "game state, player state, buildings");
	if (State != nullptr && Bob != nullptr && Foundation != nullptr && Centre != nullptr)
	{
		HOLTestWorld::InitProductionSubsystem(TestWorld);
		T.AssertTrue(HOLGameStateTest::InitVictoryFromJson(State, DisconnectTest::MakeNamedJson(false)), "the configuration is given to InitVictory");
		FHOLResources Starting = DisconnectTest::Res(500, 300, 100, 100);
		HOLGameStateTest::GiveSlotStock(State, Bob, Starting, 0.0);

		FHOLResources PeasantCost = HOLTestWorld::GetProductionCostResources(TestWorld, n"Peasant", 1);
		T.AssertTrue(HOLTestWorld::EnqueueProduction(TestWorld, Centre, n"Peasant", 1, Bob, Starting, 5), "a Peasant queued");
		FHOLResources AtDeparture = HOLGameStateTest::GetResources(Bob);
		FHOLResources FoundationRefund = HOLTestWorld::GetBuildingCancelRefund(Foundation);
		T.AssertTrue(FoundationRefund.Wood > 0, "a house's foundation gives something back");
		T.AssertTrue(PeasantCost.Food > 0, "a Peasant costs something");

		T.AssertEquals(1, HOLTestWorld::CountPlayerStatesOfSlot(TestWorld, 1), "Bob has his player state in the game state before he leaves");
		T.AssertTrue(DisconnectTest2::IsListed(State, Bob), "Bob is listed in the game state before he leaves");
		T.AssertTrue(HOLGameStateTest::KeepAbsentSlot(State, Bob, 10.0, 0.0), "Bob leaves: slot kept");
		Bob.DestroyActor();
		T.AssertFalse(DisconnectTest2::IsListed(State, Bob), "his player state gone, as after a Logout");
		T.AssertEquals(1, HOLTestWorld::CountPlayerStatesOfSlot(TestWorld, 1), "one player state left for his seat: the AI's, which holds it (D717 T2)");

		// A Peasant of his unloads 10 wood at his centre (the gathering's deposit).
		int Remaining = -1;
		int Deposited = HOLTestWorld::DepositLoadWithRemaining(TestWorld, 1, DisconnectTest2::Wood, 10, Remaining);
		T.AssertEquals(10, Deposited, "his Peasant unloads 10 wood");
		T.AssertEquals(0, Remaining, "its load emptied");

		// The enemy destroys his foundation and his centre (the combat's path, UHOLConstructionSubsystem::DestroyBuilding): the cost of the foundation and the queue back.
		HOLTestWorld::DestroyBuildingWithRefund(Foundation);
		HOLTestWorld::DestroyBuildingWithRefund(Centre);

		AHOLPlayerState BobAgain = DisconnectTest2::Seat(TestWorld, State, 1, false);
		HOLGameStateTest::GiveSlotStock(State, BobAgain, Starting, 100.0);
		FHOLResources Expected = AtDeparture;
		Expected.Wood += 10;
		Expected.Food += FoundationRefund.Food + PeasantCost.Food;
		Expected.Wood += FoundationRefund.Wood + PeasantCost.Wood;
		Expected.Stone += FoundationRefund.Stone + PeasantCost.Stone;
		Expected.Gold += FoundationRefund.Gold + PeasantCost.Gold;
		FHOLResources Back = HOLGameStateTest::GetResources(BobAgain);
		T.AssertTrue(DisconnectTest::SameRes(Back, Expected), f"back: his stock plus the 10 wood, the foundation's cost and the queue's (back {Back.Food}/{Back.Wood}/{Back.Stone}/{Back.Gold}, expected {Expected.Food}/{Expected.Wood}/{Expected.Stone}/{Expected.Gold})");
	}
	HOLTestWorld::DestroyTestWorld(TestWorld);
}

// D719 (T0): the technologies are kept for the absent as the stock. Differences: the journal is captured (BeginLogCapture; the two "researched PloughI" count
// assertions are ported, over 3000 steps instead of 8), and the AI's reader is read twice: GetAITechnologiesDone (ReadAITechnologiesDone) and the technologies of the AI's player state of his seat.
void Test_DisconnectTechnologies(FUnitTest& T)
{
	T.AssertTrue(HOLGameStateTest::IsTechnologyInTable(n"PloughII"), "the technology table is loaded (Plough II)");
	UHOLTestWorldHandle TestWorld = HOLTestWorld::CreateTestWorld();
	T.AssertTrue(TestWorld != nullptr, "a test world is created");
	if (TestWorld == nullptr)
		return;
	// No hol.Prod.BuildTime here: a console set (SetByConsole) would stay and the C++ test's SetByCode would then be ignored; more steps instead.
	AHOLGameState State = HOLTestWorld::SpawnGameState(TestWorld, 0, 0);
	AHOLPlayerState Alice = DisconnectTest2::Seat(TestWorld, State, 0, true);
	AHOLPlayerState Bob = DisconnectTest2::Seat(TestWorld, State, 1, true);
	AHOLBuilding BobFarm = HOLTestWorld::SpawnBuilding(TestWorld, n"Farm", 1, FVector(0.0, 0.0, 0.0), EHOLConstructionState::Finished);
	AHOLBuilding AliceFarm = HOLTestWorld::SpawnBuilding(TestWorld, n"Farm", 0, FVector(5000.0, 0.0, 0.0), EHOLConstructionState::Finished);
	AHOLBuilding NobodyFarm = HOLTestWorld::SpawnBuilding(TestWorld, n"Farm", 3, FVector(-5000.0, 0.0, 0.0), EHOLConstructionState::Finished);
	T.AssertTrue(State != nullptr && Alice != nullptr && Bob != nullptr && BobFarm != nullptr && AliceFarm != nullptr && NobodyFarm != nullptr, "game state, player states, farms");
	if (State != nullptr && Alice != nullptr && Bob != nullptr && BobFarm != nullptr && AliceFarm != nullptr && NobodyFarm != nullptr)
	{
		T.AssertTrue(HOLGameStateTest::InitVictoryFromJson(State, DisconnectTest::MakeNamedJson(false)), "the configuration is given to InitVictory");
		FHOLResources Starting = DisconnectTest::Res(2000, 2000, 500, 500);
		HOLGameStateTest::GiveSlotStock(State, Alice, Starting, 0.0);
		HOLGameStateTest::GiveSlotStock(State, Bob, Starting, 0.0);
		HOLTestWorld::InitProductionSubsystem(TestWorld);

		T.AssertTrue(HOLGameStateTest::AddTechnology(Bob, n"PloughI"), "Bob has the Plough I");
		T.AssertTrue(HOLTestWorld::EnqueueProduction(TestWorld, BobFarm, n"PloughII", 1, Bob, Starting, 5), "Bob queues the Plough II");
		T.AssertTrue(HOLTestWorld::EnqueueProduction(TestWorld, AliceFarm, n"PloughI", 0, Alice, Starting, 5), "Alice queues the Plough I");
		T.AssertTrue(HOLTestWorld::EnqueueProduction(TestWorld, NobodyFarm, n"PloughI", 3, nullptr, Starting, 5), "slot 3 queues the Plough I");

		T.AssertTrue(HOLGameStateTest::KeepAbsentSlot(State, Bob, 10.0, 180.0), "Bob leaves: slot kept");
		Bob.DestroyActor();
		AHOLPlayerState HolderAway = HOLTestWorld::FindHeldBot(TestWorld, 1);
		T.AssertTrue(HolderAway != nullptr && HOLGameStateTest::HasTechnology(HolderAway, n"PloughI"), "away: his Plough I is still his (read on the AI's player state of his seat)");
		TArray<FName> DoneAway = HOLTestWorld::GetAITechnologiesDone(TestWorld, 1);
		T.AssertTrue(DoneAway.Contains(n"PloughI"), "away: his Plough I is still read as done (the AI's reader)");

		HOLTestWorld::BeginLogCapture();
		HOLTestWorld::AdvanceSteps(TestWorld, 3000);
		int AliceLogged = HOLTestWorld::CountCapturedLogLines("researched PloughI", "slot 0");
		int NobodyLogged = HOLTestWorld::CountCapturedLogLines("researched PloughI", "slot 3");
		HOLTestWorld::EndLogCapture();

		T.AssertTrue(HOLGameStateTest::HasTechnology(Alice, n"PloughI"), "Alice's research landed in her player state, as ever");
		AHOLPlayerState HolderLater = HOLTestWorld::FindHeldBot(TestWorld, 1);
		T.AssertTrue(HolderLater != nullptr && HOLGameStateTest::HasTechnology(HolderLater, n"PloughII"), "away: the research ended while he was gone is his");
		TArray<FName> DoneLater = HOLTestWorld::GetAITechnologiesDone(TestWorld, 1);
		T.AssertTrue(DoneLater.Contains(n"PloughII"), "away: the research ended while he was gone is read as done (the AI's reader)");

		T.AssertEquals(1, AliceLogged, "Alice's research logged once");
		T.AssertEquals(0, NobodyLogged, "slot 3's research, going nowhere, is not logged as researched");

		AHOLPlayerState BobAgain = DisconnectTest2::Seat(TestWorld, State, 1, true);
		T.AssertTrue(BobAgain != nullptr, "new player state");
		if (BobAgain != nullptr)
		{
			T.AssertFalse(HOLGameStateTest::HasTechnology(BobAgain, n"PloughI"), "a new player state starts without technologies");
			T.AssertTrue(HOLGameStateTest::GiveSlotStock(State, BobAgain, Starting, 100.0), "back at 100");
			T.AssertTrue(HOLGameStateTest::HasTechnology(BobAgain, n"PloughI"), "back: his Plough I");
			T.AssertTrue(HOLGameStateTest::HasTechnology(BobAgain, n"PloughII"), "back: his Plough II, ended while he was away");
			T.AssertTrue(HOLGameStateTest::GetGatherBonusPercent(BobAgain, DisconnectTest2::Food) > 0, "back: their effect (food gathering) counts at once");
			T.AssertFalse(HOLGameStateTest::GetTechnologyCount(BobAgain) > 2, "back: Alice's technologies are not his");
		}
		T.AssertEquals(1, HOLGameStateTest::GetTechnologyCount(Alice), "Alice never left: her technologies are hers");
	}
	HOLTestWorld::DestroyTestWorld(TestWorld);
}

// D717 (T2): the AI holds the seat of the player who left, from the Logout to his return.
void Test_DisconnectHeld(FUnitTest& T)
{
	UHOLTestWorldHandle TestWorld = HOLTestWorld::CreateTestWorld();
	T.AssertTrue(TestWorld != nullptr, "a test world is created");
	if (TestWorld == nullptr)
		return;
	AHOLGameState State = HOLTestWorld::SpawnGameState(TestWorld, 0, 0);
	UHOLAIBrainSubsystem Brain = HOLTestWorld::GetAIBrain(TestWorld);
	AHOLPlayerState Alice = DisconnectTest2::Seat(TestWorld, State, 0, true);
	AHOLPlayerState Bob = DisconnectTest2::Seat(TestWorld, State, 1, true);
	T.AssertTrue(State != nullptr && Brain != nullptr && Alice != nullptr && Bob != nullptr, "game state, the script brain, player states");
	if (State != nullptr && Brain != nullptr && Alice != nullptr && Bob != nullptr)
	{
		T.AssertTrue(HOLGameStateTest::InitVictoryFromJson(State, DisconnectTest::MakeNamedJson(false)), "the configuration is given to InitVictory");
		FHOLResources Starting = DisconnectTest::Res(2000, 2000, 500, 500);
		HOLGameStateTest::GiveSlotStock(State, Alice, Starting, 0.0);
		HOLGameStateTest::GiveSlotStock(State, Bob, Starting, 0.0);
		const int Normal = 1;

		T.AssertFalse(Brain.IsHeld(1), "present: Bob's seat is not held");
		T.AssertEquals(-1, HOLTestWorld::GetSlotLevel(TestWorld, 1), "present: a human seat has no level");
		T.AssertFalse(Brain.GetThinkingSlots().Contains(1), "present: nobody thinks for him");

		T.AssertTrue(HOLGameStateTest::KeepAbsentSlot(State, Bob, 10.0, 180.0), "Bob leaves: slot kept");
		Bob.DestroyActor();
		T.AssertTrue(Brain.IsHeld(1), "away: the AI holds his seat");
		T.AssertEquals(Normal, HOLTestWorld::GetSlotLevel(TestWorld, 1), "away: a held seat is read as Normal");
		T.AssertFalse(Brain.IsHeld(0), "away: Alice's seat is not held");
		T.AssertTrue(Brain.GetThinkingSlots().Contains(1), "away: the brain thinks for his seat");
		T.AssertFalse(Brain.GetThinkingSlots().Contains(0), "away: and not for Alice's");
		Brain.ThinkNow(1);

		AHOLPlayerState BobAgain = DisconnectTest2::Seat(TestWorld, State, 1, true);
		T.AssertTrue(BobAgain != nullptr, "new player state");
		if (BobAgain != nullptr)
		{
			T.AssertTrue(HOLGameStateTest::GiveSlotStock(State, BobAgain, Starting, 100.0), "back at 100");
			T.AssertFalse(Brain.IsHeld(1), "back: the AI lets the seat go");
			T.AssertFalse(Brain.GetThinkingSlots().Contains(1), "back: the brain no longer thinks for it");
			T.AssertEquals(-1, HOLTestWorld::GetSlotLevel(TestWorld, 1), "back: a human seat has no level again");
		}
	}
	HOLTestWorld::DestroyTestWorld(TestWorld);
}

// D717 (T2): the AI that holds the seat plays with HIS stock (the real chain, KeepAbsentSlot then the holder's player state).
void Test_DisconnectHeldStock(FUnitTest& T)
{
	UHOLTestWorldHandle TestWorld = HOLTestWorld::CreateTestWorld();
	T.AssertTrue(TestWorld != nullptr, "a test world is created");
	if (TestWorld == nullptr)
		return;
	AHOLGameState State = HOLTestWorld::SpawnGameState(TestWorld, 0, 0);
	AHOLPlayerState Alice = DisconnectTest2::Seat(TestWorld, State, 0, true);
	AHOLPlayerState Bob = DisconnectTest2::Seat(TestWorld, State, 1, true);
	AHOLBuilding BobCentre = HOLTestWorld::SpawnBuilding(TestWorld, n"MainCentre", 1, FVector(0.0, 0.0, 0.0), EHOLConstructionState::Finished);
	AHOLBuilding AliceCentre = HOLTestWorld::SpawnBuilding(TestWorld, n"MainCentre", 0, FVector(-20000.0, 0.0, 0.0), EHOLConstructionState::Finished);
	T.AssertTrue(State != nullptr && Alice != nullptr && Bob != nullptr && BobCentre != nullptr && AliceCentre != nullptr, "game state, player states, centres");
	if (State != nullptr && Alice != nullptr && Bob != nullptr && BobCentre != nullptr && AliceCentre != nullptr)
	{
		T.AssertTrue(HOLGameStateTest::InitVictoryFromJson(State, DisconnectTest::MakeNamedJson(false)), "the configuration is given to InitVictory");
		FHOLResources Starting = DisconnectTest::Res(2000, 2000, 500, 500);
		HOLGameStateTest::GiveSlotStock(State, Alice, Starting, 0.0);
		HOLGameStateTest::GiveSlotStock(State, Bob, Starting, 0.0);
		HOLTestWorld::InitProductionSubsystem(TestWorld);
		HOLGameStateTest::AddTechnology(Bob, n"PloughI");

		T.AssertTrue(HOLGameStateTest::KeepAbsentSlot(State, Bob, 10.0, 180.0), "Bob leaves: slot kept");
		Bob.DestroyActor();

		AHOLPlayerState Holder = HOLTestWorld::FindHeldBot(TestWorld, 1);
		T.AssertTrue(Holder != nullptr, "away: a player state of the AI holds the seat");
		if (Holder != nullptr)
		{
			T.AssertTrue(HOLGameStateTest::IsABot(Holder), "it is an AI's");
			T.AssertEquals("Bob", Holder.GetPlayerName(), "it keeps his name");
			T.AssertTrue(DisconnectTest::SameRes(HOLGameStateTest::GetResources(Holder), Starting), "it has his stock");
			T.AssertTrue(HOLGameStateTest::HasTechnology(Holder, n"PloughI"), "it has his Plough I");
		}
		T.AssertTrue(HOLTestWorld::FindHeldBot(TestWorld, 0) == nullptr, "Alice's seat has none");
		TArray<int> InUse = HOLGameStateTest::GetSlotsInUse(State, nullptr);
		T.AssertFalse(InUse.Contains(1), "away: PostLogin does not see his seat as taken");
		T.AssertTrue(InUse.Contains(0), "... Alice's is");

		// What the brain orders for the seat is paid on his stock (the real order path, as the script's).
		T.AssertTrue(HOLTestWorld::HasSlotStock(TestWorld, 1), "away: his stock is read");
		FHOLResources Before = HOLTestWorld::GetSlotStock(TestWorld, 1);
		// The AI's order path (UHOLAIOrderLibrary::IssueAIProduceOrder, the test world as its context), paid on the holder's stock.
		T.AssertTrue(HOLTestWorld::IssueAIProduceOrder(TestWorld, 1, FVector(0.0, 0.0, 0.0), n"Peasant"), "the AI orders a Peasant at his centre");
		HOLTestWorld::AdvanceSteps(TestWorld, 1);
		T.AssertEquals(1, HOLTestWorld::GetProductionQueueCount(BobCentre), "next step: the Peasant is in his centre's queue");
		FHOLResources After = HOLTestWorld::GetSlotStock(TestWorld, 1);
		T.AssertTrue(After.Food < Before.Food, "paid: his food went down");
		// A deposit (what a Peasant unloads) goes where the orders pay.
		T.AssertTrue(HOLTestWorld::AddSlotStock(TestWorld, 1, DisconnectTest::Res(0, 100, 0, 0)), "a deposit is added to his stock");
		FHOLResources Stock = HOLTestWorld::GetSlotStock(TestWorld, 1);
		T.AssertEquals(After.Wood + 100, Stock.Wood, "... and the orders pay on it (wood +100)");
		T.AssertTrue(DisconnectTest::SameRes(Stock, DisconnectTest::Res(After.Food, After.Wood + 100, After.Stone, After.Gold)), "... read back");
		T.AssertTrue(DisconnectTest::SameRes(HOLGameStateTest::GetResources(Alice), Starting), "Alice's stock is untouched");

		// Back with a new player state: what the AI left, the AI gone.
		AHOLPlayerState BobAgain = DisconnectTest2::Seat(TestWorld, State, 1, true);
		T.AssertTrue(BobAgain != nullptr, "new player state");
		if (BobAgain != nullptr)
		{
			T.AssertTrue(HOLGameStateTest::GiveSlotStock(State, BobAgain, Starting, 100.0), "back at 100");
			T.AssertTrue(DisconnectTest::SameRes(HOLGameStateTest::GetResources(BobAgain), Stock), "back: the stock as the AI left it (spent, deposited)");
			T.AssertTrue(HOLGameStateTest::HasTechnology(BobAgain, n"PloughI"), "back: his Plough I");
			T.AssertTrue(HOLTestWorld::FindHeldBot(TestWorld, 1) == nullptr, "back: no player state of the AI any more");
			T.AssertTrue(HOLTestWorld::FindLiveHolder(TestWorld, 1, nullptr) == BobAgain, "back: one player state for his seat");
			T.AssertEquals(1, HOLTestWorld::CountPlayerStatesOfSlot(TestWorld, 1), "back: one player state for his seat (counted over the PlayerArray)");
			T.AssertEquals(1, HOLTestWorld::GetProductionQueueCount(BobCentre), "back: his order stays in the centre's queue");
		}
	}
	HOLTestWorld::DestroyTestWorld(TestWorld);
}

// D717 (T4): a name that comes back while its OLD connection is still alive takes his seat back.
// Difference: none known (the order is the AI's, HOLTestWorld::IssueAIProduceOrder, then one step applies it, as the C++).
// D717 (T4) again: the old player state is a spectator without a seat once the new one has it.
void Test_DisconnectReplace(FUnitTest& T)
{
	UHOLTestWorldHandle TestWorld = HOLTestWorld::CreateTestWorld();
	T.AssertTrue(TestWorld != nullptr, "a test world is created");
	if (TestWorld == nullptr)
		return;
	AHOLGameState State = HOLTestWorld::SpawnGameState(TestWorld, 0, 0);
	AHOLPlayerState Alice = DisconnectTest2::Seat(TestWorld, State, 0, true);
	AHOLPlayerState BobOld = DisconnectTest2::Seat(TestWorld, State, 1, true);
	AHOLPlayerState BobNew = HOLTestWorld::SpawnPlayerStateNoSlot(TestWorld, "");
	AHOLBuilding BobCentre = HOLTestWorld::SpawnBuilding(TestWorld, n"MainCentre", 1, FVector(0.0, 0.0, 0.0), EHOLConstructionState::Finished);
	AHOLBuilding AliceCentre = HOLTestWorld::SpawnBuilding(TestWorld, n"MainCentre", 0, FVector(-20000.0, 0.0, 0.0), EHOLConstructionState::Finished);
	T.AssertTrue(State != nullptr && Alice != nullptr && BobOld != nullptr && BobNew != nullptr && BobCentre != nullptr && AliceCentre != nullptr, "game state, player states, centres");
	if (State != nullptr && Alice != nullptr && BobOld != nullptr && BobNew != nullptr && BobCentre != nullptr && AliceCentre != nullptr)
	{
		FString Config = DisconnectTest::MakeNamedJson(false);
		T.AssertTrue(HOLGameStateTest::InitVictoryFromJson(State, Config), "the configuration is given to InitVictory");
		FHOLResources Starting = DisconnectTest::Res(2000, 2000, 500, 500);
		HOLGameStateTest::GiveSlotStock(State, Alice, Starting, 0.0);
		HOLGameStateTest::GiveSlotStock(State, BobOld, Starting, 0.0);
		HOLTestWorld::InitProductionSubsystem(TestWorld);
		HOLGameStateTest::AddTechnology(BobOld, n"PloughI");
		T.AssertTrue(HOLTestWorld::IssueAIProduceOrder(TestWorld, 1, FVector(0.0, 0.0, 0.0), n"Peasant"), "Bob orders a Peasant");
		HOLTestWorld::AdvanceSteps(TestWorld, 1);
		T.AssertEquals(1, HOLTestWorld::GetProductionQueueCount(BobCentre), "the Peasant is in his centre's queue");
		FHOLResources BobStock = HOLGameStateTest::GetResources(BobOld);
		T.AssertFalse(DisconnectTest::SameRes(BobStock, Starting), "(his stock is no longer the starting one)");

		// The new connection of his name, no slot yet (PostLogin's state), while the old one is still in the player array.
		HOLGameStateTest::AddPlayerState(State, BobNew);
		int Slot = HOLGameStateTest::FindSlotForLogin(State, Config, "Bob", BobNew);
		T.AssertEquals(1, Slot, "his name finds his seat, not a spectator");
		AHOLPlayerState Replaced = HOLGameStateTest::FindReplacedForLogin(State, Config, "Bob", BobNew);
		T.AssertTrue(Replaced == BobOld, "the old connection's player state is the one to replace");
		int ZedSlot = HOLGameStateTest::FindSlotForLogin(State, Config, "Zed", BobNew);
		T.AssertEquals(-1, ZedSlot, "an unknown name watches");
		T.AssertTrue(HOLGameStateTest::FindReplacedForLogin(State, Config, "Zed", BobNew) == nullptr, "... and replaces nobody");

		// PostLogin's order: the old one handed over, then the slot and the stock to the new one.
		if (Slot == 1 && Replaced == BobOld)
		{
			HOLGameStateTest::RetireHolder(State, BobOld, 100.0, 180.0);
			HOLGameStateTest::SetPlayerMatchSlot(BobNew, Slot, 1);
			T.AssertTrue(HOLGameStateTest::GiveSlotStock(State, BobNew, Starting, 100.0), "he takes his seat back");
			T.AssertTrue(DisconnectTest::SameRes(HOLGameStateTest::GetResources(BobNew), BobStock), "his stock is the one of his old connection, not the starting one");
			T.AssertTrue(HOLGameStateTest::HasTechnology(BobNew, n"PloughI"), "his Plough I");
			T.AssertEquals(1, HOLTestWorld::GetProductionQueueCount(BobCentre), "his order stays in the centre's queue");
			T.AssertTrue(HOLGameStateTest::IsOnlyASpectator(BobOld), "the old player state is a spectator now");
			T.AssertTrue(HOLTestWorld::GetPlayerStateSlot(BobOld) != 1, "... without a seat");
			T.AssertTrue(HOLTestWorld::FindLiveHolder(TestWorld, 1, nullptr) == BobNew, "the new player state alone holds the seat");
			T.AssertTrue(HOLTestWorld::FindHeldBot(TestWorld, 1) == nullptr, "no player state of the AI is left on the seat");
			T.AssertFalse(HOLGameStateTest::IsSlotAbsent(State, 1), "his seat is not absent");
			// The old connection closes late: nobody is put away under the new player.
			T.AssertFalse(HOLGameStateTest::KeepAbsentSlot(State, BobOld, 101.0, 180.0), "late Logout of the old connection: the seat is not kept");
			T.AssertFalse(HOLGameStateTest::IsSlotAbsent(State, 1), "... the seat stays present");
			T.AssertTrue(HOLTestWorld::FindHeldBot(TestWorld, 1) == nullptr, "... no AI on it");
			T.AssertTrue(DisconnectTest::SameRes(HOLGameStateTest::GetResources(BobNew), BobStock), "... his stock is untouched");
		}

		// A duplicate that was NOT handed over (killed another way): its late Logout is no absence either, the new holder is there.
		AHOLPlayerState Duplicate = DisconnectTest2::Seat(TestWorld, State, 1, true);
		T.AssertTrue(Duplicate != nullptr, "duplicate");
		if (Duplicate != nullptr)
		{
			T.AssertFalse(HOLGameStateTest::KeepAbsentSlot(State, Duplicate, 102.0, 180.0), "a duplicate closing late: the seat is not kept");
			T.AssertFalse(HOLGameStateTest::IsSlotAbsent(State, 1), "... the seat stays present");
		}
		T.AssertTrue(DisconnectTest::SameRes(HOLGameStateTest::GetResources(Alice), Starting), "Alice's stock is untouched");
		T.AssertFalse(HOLGameStateTest::IsSlotAbsent(State, 0), "Alice is not absent");
	}
	HOLTestWorld::DestroyTestWorld(TestWorld);
}

// T9: hol.Net.FakeAbsent Back <Slot>, the return without a second client, on the real chain (the console commands).
// Difference: none (the count of player states of his seat is CountPlayerStatesOfSlot, over the PlayerArray as the C++; FindLiveHolder checks the holder too).
void Test_DisconnectFakeBack(FUnitTest& T)
{
	UHOLTestWorldHandle TestWorld = HOLTestWorld::CreateTestWorld();
	T.AssertTrue(TestWorld != nullptr, "a test world is created");
	if (TestWorld == nullptr)
		return;
	AHOLGameState State = HOLTestWorld::SpawnGameState(TestWorld, 0, 0);
	AHOLPlayerState Alice = DisconnectTest2::Seat(TestWorld, State, 0, true);
	T.AssertTrue(State != nullptr && Alice != nullptr, "game state, Alice");
	if (State != nullptr && Alice != nullptr)
	{
		T.AssertTrue(HOLGameStateTest::InitVictoryFromJson(State, DisconnectTest::MakeNamedJson(false)), "the configuration is given to InitVictory");
		FHOLResources AliceStock = DisconnectTest::Res(200, 200, 200, 200);
		HOLGameStateTest::GiveSlotStock(State, Alice, AliceStock, 0.0);

		HOLTestWorld::RunConsoleCommand(TestWorld, "hol.Net.FakeAbsent Leave 1");
		T.AssertTrue(HOLTestWorld::FindHeldBot(TestWorld, 1) != nullptr, "Leave 1: the AI holds Bob's seat");
		T.AssertTrue(HOLGameStateTest::HasAbsentNotice(State, 1, false), "Leave 1: his banner is up");
		T.AssertTrue(HOLTestWorld::AddSlotStock(TestWorld, 1, DisconnectTest::Res(0, 100, 0, 0)), "the AI's stock grows (a deposit)");
		FHOLResources Stock = HOLTestWorld::GetSlotStock(TestWorld, 1);

		HOLTestWorld::RunConsoleCommand(TestWorld, "hol.Net.FakeAbsent Back 1");
		T.AssertTrue(HOLTestWorld::FindHeldBot(TestWorld, 1) == nullptr, "Back 1: the AI's player state is gone");
		T.AssertTrue(HOLGameStateTest::GetSlotsInUse(State, nullptr).Contains(1), "Back 1: the seat is a player's again");
		T.AssertTrue(HOLTestWorld::HasSlotStock(TestWorld, 1), "Back 1: his stock is read");
		FHOLResources Read = HOLTestWorld::GetSlotStock(TestWorld, 1);
		T.AssertTrue(DisconnectTest::SameRes(Read, Stock), "Back 1: the stock as the AI left it (the deposit is there)");
		T.AssertTrue(DisconnectTest::SameRes(HOLGameStateTest::GetResources(Alice), AliceStock), "Back 1: Alice's stock is untouched");
		AHOLPlayerState Returned = HOLTestWorld::FindLiveHolder(TestWorld, 1, nullptr);
		T.AssertEquals(1, HOLTestWorld::CountPlayerStatesOfSlot(TestWorld, 1), "Back 1: one player state for his seat (counted over the PlayerArray)");
		T.AssertTrue(Returned != nullptr && !HOLGameStateTest::IsABot(Returned) && Returned.GetPlayerName() == "Bob", "Back 1: that player state is Bob's, not an AI's");
		// Nobody away any more: a second Back has nothing to give back.
		int Notices = HOLGameStateTest::CountAbsentNotices(State);
		HOLTestWorld::RunConsoleCommand(TestWorld, "hol.Net.FakeAbsent Back 1");
		T.AssertEquals(Notices, HOLGameStateTest::CountAbsentNotices(State), "a second Back changes nothing");
	}
	HOLTestWorld::DestroyTestWorld(TestWorld);
}

// D718 (T5): the end of the window with an AI seat in Bob's camp: the AI keeps the seat; a real 1v1: his defeat.
void Test_DisconnectLateReturn(FUnitTest& T)
{
	FHOLResources Starting = DisconnectTest::Res(2000, 2000, 500, 500);
	{
		UHOLTestWorldHandle TestWorld = HOLTestWorld::CreateTestWorld();
		T.AssertTrue(TestWorld != nullptr, "a test world is created");
		if (TestWorld == nullptr)
			return;
		AHOLGameState State = HOLTestWorld::SpawnGameState(TestWorld, 0, 0);
		UHOLAIBrainSubsystem Brain = HOLTestWorld::GetAIBrain(TestWorld);
		AHOLPlayerState Alice = DisconnectTest2::Seat(TestWorld, State, 0, true);
		AHOLPlayerState Bob = DisconnectTest2::Seat(TestWorld, State, 1, true);
		T.AssertTrue(State != nullptr && Brain != nullptr && Alice != nullptr && Bob != nullptr, "game state, the script brain, player states");
		if (State != nullptr && Brain != nullptr && Alice != nullptr && Bob != nullptr)
		{
			T.AssertTrue(HOLGameStateTest::InitVictoryFromJson(State, DisconnectTest::MakeNamedJson(false)), "the configuration is given to InitVictory");
			HOLGameStateTest::GiveSlotStock(State, Alice, Starting, 0.0);
			HOLGameStateTest::GiveSlotStock(State, Bob, Starting, 0.0);
			T.AssertTrue(HOLGameStateTest::KeepAbsentSlot(State, Bob, 10.0, 180.0), "Bob leaves at 10");
			Bob.DestroyActor();
			T.AssertTrue(Brain.IsHeld(1), "in the window: the seat is held (hold mode)");
			T.AssertTrue(HOLGameStateTest::HasAbsentNotice(State, 1, false), "in the window: his banner is up");

			// The window runs out (10 + 180): the AI keeps the seat.
			TArray<int> Defeated = HOLGameStateTest::DefeatExpiredAbsences(State, 191.0, 180.0);
			T.AssertEquals(0, Defeated.Num(), "end of the window: nobody defeated (his camp has an AI seat)");
			T.AssertFalse(HOLGameStateTest::IsSlotDefeated(State, 1), "... his seat is not defeated");
			T.AssertTrue(HOLTestWorld::FindHeldBot(TestWorld, 1) != nullptr, "... the AI's player state still holds it");
			T.AssertFalse(Brain.IsHeld(1), "... the hold mode is over");
				T.AssertEquals(1, HOLTestWorld::GetSlotLevel(TestWorld, 1), "... at the Normal level");
			T.AssertTrue(Brain.GetThinkingSlots().Contains(1), "... the brain thinks for it, the full brain");
			T.AssertFalse(HOLGameStateTest::HasAbsentNotice(State, 1, false), "... his banner is out");
			HOLGameStateTest::DefeatExpiredAbsences(State, 300.0, 180.0);
			T.AssertFalse(HOLGameStateTest::IsSlotDefeated(State, 1), "a second pass later: still not defeated");
			Brain.ThinkNow(1);

			// He comes back AFTER the window and takes the seat.
			AHOLPlayerState BobAgain = DisconnectTest2::Seat(TestWorld, State, 1, true);
			T.AssertTrue(BobAgain != nullptr, "new player state");
			if (BobAgain != nullptr)
			{
				T.AssertTrue(HOLGameStateTest::GiveSlotStock(State, BobAgain, Starting, 500.0), "late return at 500");
				T.AssertTrue(DisconnectTest::SameRes(HOLGameStateTest::GetResources(BobAgain), Starting), "... his stock");
				T.AssertTrue(HOLTestWorld::FindHeldBot(TestWorld, 1) == nullptr, "... the AI's player state gone");
				T.AssertFalse(Brain.GetThinkingSlots().Contains(1), "... the brain no longer thinks for it");
					T.AssertEquals(-1, HOLTestWorld::GetSlotLevel(TestWorld, 1), "... a human seat has no level again");
				T.AssertFalse(HOLGameStateTest::IsSlotAbsent(State, 1), "... the seat is no longer absent");
				T.AssertFalse(HOLGameStateTest::IsSlotDefeated(State, 1), "... and never defeated");
			}
		}
		HOLTestWorld::DestroyTestWorld(TestWorld);
	}

	// A 1v1: Bob alone in his camp (the AI seat closed): his window's end is his defeat.
	{
		UHOLTestWorldHandle TestWorld = HOLTestWorld::CreateTestWorld();
		T.AssertTrue(TestWorld != nullptr, "a second test world is created");
		if (TestWorld == nullptr)
			return;
		AHOLGameState State = HOLTestWorld::SpawnGameState(TestWorld, 0, 0);
		AHOLPlayerState Alice = DisconnectTest2::Seat(TestWorld, State, 0, true);
		AHOLPlayerState Bob = DisconnectTest2::Seat(TestWorld, State, 1, true);
		T.AssertTrue(State != nullptr && Alice != nullptr && Bob != nullptr, "game state, player states");
		if (State != nullptr && Alice != nullptr && Bob != nullptr)
		{
			T.AssertTrue(HOLGameStateTest::InitVictoryFromJson(State, DisconnectTest::MakeNamedJson(true)), "the 1v1 configuration is given to InitVictory");
			HOLGameStateTest::GiveSlotStock(State, Alice, Starting, 0.0);
			HOLGameStateTest::GiveSlotStock(State, Bob, Starting, 0.0);
			T.AssertTrue(HOLGameStateTest::KeepAbsentSlot(State, Bob, 10.0, 180.0), "1v1: Bob leaves at 10");
			Bob.DestroyActor();
			TArray<int> Defeated = HOLGameStateTest::DefeatExpiredAbsences(State, 191.0, 180.0);
			T.AssertTrue(Defeated.Contains(1) && HOLGameStateTest::IsSlotDefeated(State, 1), "1v1: the end of the window is his defeat");
		}
		HOLTestWorld::DestroyTestWorld(TestWorld);
	}
}
