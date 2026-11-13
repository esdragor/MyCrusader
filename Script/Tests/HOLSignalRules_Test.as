// Map signals, pure rules (SG1, HOLSignalRules.as; GDD 15, D386, with D369, D375, D376, D367).
// A wheel of typed signals (Attack here, Defend here, Need help, Rally, Scout here) plus the plain Alt + click neutral signal; seen by the
// sender and its allies only (in Diplomacy, the mutual allies of D369, read from HOLDiplomacy::IsAlly, never a Neutral or an Enemy);
// limited against the spam (indicative: 3 every 5 s, D386). The stances are set by the real HOLDiplomacy::SetStance, the values (limit,
// window, lifetime) are the rules' provisional constants, read from them (a test does not retype a value).

void Test_SignalKindsAreTheWheelAndTheNeutral(FUnitTest& T)
{
	T.AssertEquals(6, HOLSignal::KindCount, "five typed signals of the wheel and the neutral one (D386)");
	T.AssertTrue(HOLSignal::IsValidKind(HOLSignal::Neutral), "the plain Alt + click signal");
	T.AssertTrue(HOLSignal::IsValidKind(HOLSignal::Attack), "Attack here");
	T.AssertTrue(HOLSignal::IsValidKind(HOLSignal::Defend), "Defend here");
	T.AssertTrue(HOLSignal::IsValidKind(HOLSignal::NeedHelp), "Need help");
	T.AssertTrue(HOLSignal::IsValidKind(HOLSignal::Rally), "Rally");
	T.AssertTrue(HOLSignal::IsValidKind(HOLSignal::Scout), "Scout here");
	T.AssertTrue(!HOLSignal::IsValidKind(-1) && !HOLSignal::IsValidKind(HOLSignal::KindCount), "nothing else");
}

// A Diplomacy match of four humans: 0 and 1 mutual allies; 0 says Ally to 2 but 2 says Neutral (one-sided); 3 stays Enemy.
FHOLDiplomacyBook SignalTestBook()
{
	TArray<bool> IsAI;
	for (int Slot = 0; Slot < 4; Slot++)
		IsAI.Add(false);
	FHOLDiplomacyBook Book = HOLDiplomacy::NewBook(4, IsAI);
	HOLDiplomacy::SetStance(Book, 0, 1, HOLDiplomacy::Ally);
	HOLDiplomacy::SetStance(Book, 1, 0, HOLDiplomacy::Ally);
	HOLDiplomacy::SetStance(Book, 0, 2, HOLDiplomacy::Ally);
	HOLDiplomacy::SetStance(Book, 2, 0, HOLDiplomacy::Neutral);
	return Book;
}

void Test_SignalSeenByMutualAlliesOnly(FUnitTest& T)
{
	FHOLDiplomacyBook Book = SignalTestBook();
	FHOLSignalBoard Board;
	T.AssertEquals(HOLSignal::Ok, HOLSignal::Post(Board, Book, 0, false, HOLSignal::Attack, FVector(100.0, 200.0, 0.0), 0),
		"player 0 places Attack here");

	TArray<FHOLSignal> Seen0 = HOLSignal::SeenBy(Board, Book, 0, 10);
	TArray<FHOLSignal> Seen1 = HOLSignal::SeenBy(Board, Book, 1, 10);
	T.AssertEquals(1, Seen0.Num(), "the sender sees its own signal");
	T.AssertEquals(1, Seen1.Num(), "its mutual ally sees it (D369)");
	if (Seen1.Num() == 1)
	{
		T.AssertEquals(HOLSignal::Attack, Seen1[0].Kind, "with its kind");
		T.AssertEquals(0, Seen1[0].Sender, "and its sender");
	}
	T.AssertEquals(0, HOLSignal::SeenBy(Board, Book, 2, 10).Num(), "a one-sided ally (2 says Neutral) does not see it (D375)");
	T.AssertEquals(0, HOLSignal::SeenBy(Board, Book, 3, 10).Num(), "an Enemy does not see it");

	// The break is immediate (D370): once 1 says Enemy, it no longer sees 0's signals.
	HOLDiplomacy::SetStance(Book, 1, 0, HOLDiplomacy::Enemy);
	T.AssertEquals(0, HOLSignal::SeenBy(Board, Book, 1, 20).Num(), "a broken alliance hides the signals at once");
}

void Test_SignalSeenByTeammatesInTeamModes(FUnitTest& T)
{
	TArray<int> Teams;
	Teams.Add(1);
	Teams.Add(1);
	Teams.Add(2);
	Teams.Add(2);
	TArray<bool> IsAI;
	IsAI.Add(false);
	IsAI.Add(true);
	IsAI.Add(false);
	IsAI.Add(false);
	FHOLDiplomacyBook Book = HOLDiplomacy::NewTeamBook(Teams, IsAI);
	FHOLSignalBoard Board;
	T.AssertEquals(HOLSignal::Ok, HOLSignal::Post(Board, Book, 0, false, HOLSignal::Rally, FVector(0.0, 0.0, 0.0), 0), "Rally");
	T.AssertEquals(1, HOLSignal::SeenBy(Board, Book, 1, 1).Num(), "an AI teammate receives it (it may answer later, D386)");
	T.AssertEquals(0, HOLSignal::SeenBy(Board, Book, 2, 1).Num(), "the other team does not");

	// D784 (answer of 2026-11-12 to Q-signals Q2 = A): a defeated teammate of a team match sees its team's signals but places none any
	// more (it advises in writing only, D357).
	T.AssertEquals(HOLSignal::Silenced, HOLSignal::Post(Board, Book, 3, true, HOLSignal::NeedHelp, FVector(0.0, 0.0, 0.0), 2),
		"a defeated teammate places no signal (D784)");
	T.AssertEquals(0, HOLSignal::SeenBy(Board, Book, 2, 3).Num(), "so its living teammate sees nothing from it (D784)");
	T.AssertEquals(HOLSignal::Ok, HOLSignal::Post(Board, Book, 2, false, HOLSignal::Defend, FVector(0.0, 0.0, 0.0), 3),
		"its living teammate places one");
	T.AssertEquals(1, HOLSignal::SeenBy(Board, Book, 3, 4).Num(), "the defeated teammate still sees its team's signal (D784)");
	T.AssertEquals(1, HOLSignal::SeenBy(Board, Book, 3, 4, false).Num(), "a defeated teammate is no spectator: its team's signal only");
}

void Test_SignalSpectatorsSeeEverySignal(FUnitTest& T)
{
	// Answer of 2026-11-13 to Q-signals-spectators (A): a player defeated in Diplomacy or free-for-all and a pure spectator see every
	// signal of every player, as they see the whole map (they talk to the spectators only, D376: nothing leaks to the players).
	FHOLDiplomacyBook Book = SignalTestBook();
	FHOLSignalBoard Board;
	T.AssertEquals(HOLSignal::Ok, HOLSignal::Post(Board, Book, 0, false, HOLSignal::Attack, FVector(0.0, 0.0, 0.0), 0), "0 places Attack here");
	T.AssertEquals(HOLSignal::Ok, HOLSignal::Post(Board, Book, 1, false, HOLSignal::Defend, FVector(0.0, 0.0, 0.0), 1), "1 places Defend here");
	T.AssertEquals(HOLSignal::Ok, HOLSignal::Post(Board, Book, 2, false, HOLSignal::Neutral, FVector(0.0, 0.0, 0.0), 2), "2 places a plain one");
	T.AssertEquals(0, HOLSignal::SeenBy(Board, Book, 3, 10).Num(), "3 alive (Enemy of all) sees none of them");
	T.AssertEquals(3, HOLSignal::SeenBy(Board, Book, 3, 10, true).Num(), "3 defeated in Diplomacy, a spectator: every player's signal");
	T.AssertEquals(3, HOLSignal::SeenBy(Board, Book, -1, 10, true).Num(), "a pure spectator (no seat): every player's signal");
	T.AssertEquals(1, HOLSignal::SeenBy(Board, Book, 2, 10).Num(), "a living player still sees its own and its mutual allies' only");
	T.AssertEquals(0, HOLSignal::SeenBy(Board, Book, -1, 10 + HOLSignal::LifetimeMs, true).Num(), "faded for the spectators too");

	// Free-for-all (no team): the defeated player sees the signals of the living ones.
	TArray<int> Teams;
	TArray<bool> IsAI;
	for (int Slot = 0; Slot < 3; Slot++)
	{
		Teams.Add(0);
		IsAI.Add(false);
	}
	FHOLDiplomacyBook FreeForAll = HOLDiplomacy::NewTeamBook(Teams, IsAI);
	FHOLSignalBoard FreeBoard;
	HOLSignal::Post(FreeBoard, FreeForAll, 0, false, HOLSignal::Rally, FVector(0.0, 0.0, 0.0), 0);
	HOLSignal::Post(FreeBoard, FreeForAll, 1, false, HOLSignal::Scout, FVector(0.0, 0.0, 0.0), 0);
	T.AssertEquals(0, HOLSignal::SeenBy(FreeBoard, FreeForAll, 2, 1).Num(), "free-for-all, 2 alive: nobody's signal");
	T.AssertEquals(2, HOLSignal::SeenBy(FreeBoard, FreeForAll, 2, 1, true).Num(), "free-for-all, 2 defeated: both signals");
}

void Test_SignalStaysFiveSeconds(FUnitTest& T)
{
	// D784 (Q1 = A): a signal stays 5 s on the map (the window of the limit of 3), seen through the real board.
	FHOLDiplomacyBook Book = SignalTestBook();
	FHOLSignalBoard Board;
	T.AssertEquals(HOLSignal::Ok, HOLSignal::Post(Board, Book, 0, false, HOLSignal::Attack, FVector(0.0, 0.0, 0.0), 0), "placed at 0");
	T.AssertEquals(1, HOLSignal::SeenBy(Board, Book, 1, 4999).Num(), "still there at 4.999 s");
	T.AssertEquals(0, HOLSignal::SeenBy(Board, Book, 1, 5000).Num(), "gone at 5 s (D784)");
	T.AssertEquals(5000, HOLSignal::WindowMs, "the lifetime is the window of the limit (D386, D784)");
}

void Test_FourthSignalInFiveSecondsIsRefused(FUnitTest& T)
{
	FHOLDiplomacyBook Book = SignalTestBook();
	FHOLSignalBoard Board;
	const int Step = HOLSignal::WindowMs / (HOLSignal::MaxPerWindow + 1);
	int Now = 0;
	for (int Index = 0; Index < HOLSignal::MaxPerWindow; Index++)
	{
		T.AssertEquals(HOLSignal::Ok, HOLSignal::Post(Board, Book, 0, false, HOLSignal::Neutral, FVector(0.0, 0.0, 0.0), Now),
			"within the limit");
		Now += Step;
	}
	const int Last = HOLSignal::WindowMs - 1;
	T.AssertEquals(HOLSignal::TooMany, HOLSignal::Post(Board, Book, 0, false, HOLSignal::Attack, FVector(0.0, 0.0, 0.0), Last),
		"one more inside the window is refused (D386: 3 every 5 s)");
	T.AssertEquals(HOLSignal::MaxPerWindow, HOLSignal::SeenBy(Board, Book, 1, Last).Num(), "the ally sees the allowed ones only");

	// Another player has its own count: player 1 still places one.
	T.AssertEquals(HOLSignal::Ok, HOLSignal::Post(Board, Book, 1, false, HOLSignal::Defend, FVector(0.0, 0.0, 0.0), Last),
		"the limit is per player");

	// Once the first one leaves the window, player 0 may place again (the refused one did not count).
	T.AssertEquals(HOLSignal::Ok, HOLSignal::Post(Board, Book, 0, false, HOLSignal::Attack, FVector(0.0, 0.0, 0.0),
		HOLSignal::WindowMs), "the first one left the window");
}

void Test_SignalFadesAfterItsLifetime(FUnitTest& T)
{
	FHOLDiplomacyBook Book = SignalTestBook();
	FHOLSignalBoard Board;
	HOLSignal::Post(Board, Book, 0, false, HOLSignal::Scout, FVector(0.0, 0.0, 0.0), 1000);
	T.AssertEquals(1, HOLSignal::SeenBy(Board, Book, 1, 1000 + HOLSignal::LifetimeMs - 1).Num(), "still on the map");
	T.AssertEquals(0, HOLSignal::SeenBy(Board, Book, 1, 1000 + HOLSignal::LifetimeMs).Num(), "gone after its lifetime");
	HOLSignal::Expire(Board, 1000 + HOLSignal::LifetimeMs + HOLSignal::WindowMs);
	T.AssertEquals(0, Board.Signals.Num(), "the board forgets it");
}

void Test_RefusedSignals(FUnitTest& T)
{
	FHOLDiplomacyBook Book = SignalTestBook();
	FHOLSignalBoard Board;
	T.AssertEquals(HOLSignal::BadKind, HOLSignal::Post(Board, Book, 0, false, HOLSignal::KindCount, FVector(0.0, 0.0, 0.0), 0),
		"an unknown kind");
	T.AssertEquals(HOLSignal::NoSuchSlot, HOLSignal::Post(Board, Book, 7, false, HOLSignal::Attack, FVector(0.0, 0.0, 0.0), 0),
		"a slot outside the match");
	// A defeated player of Diplomacy or free-for-all talks to the spectators only (D376): no signal to its former allies.
	T.AssertEquals(HOLSignal::Silenced, HOLSignal::Post(Board, Book, 0, true, HOLSignal::Attack, FVector(0.0, 0.0, 0.0), 0),
		"a defeated player of Diplomacy places no signal (D376)");
	T.AssertEquals(0, HOLSignal::SeenBy(Board, Book, 1, 1).Num(), "nothing placed by the refusals");
}
