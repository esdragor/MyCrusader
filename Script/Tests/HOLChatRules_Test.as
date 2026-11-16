// Chat channels, pure rule (A4, HOLChatRules.as; D385, D376, D357). Four slots: 0 and 1 allies (team 0), 2 and 3 their enemies (team 1) unless a
// test says otherwise. Every case goes through HOLChat::Recipients, the list the network will send to.

TArray<int> ChatTeams(int T0 = 0, int T1 = 0, int T2 = 1, int T3 = 1)
{
	TArray<int> Teams;
	Teams.Add(T0);
	Teams.Add(T1);
	Teams.Add(T2);
	Teams.Add(T3);
	return Teams;
}

TArray<bool> ChatSpectators(bool S0 = false, bool S1 = false, bool S2 = false, bool S3 = false)
{
	TArray<bool> Spectators;
	Spectators.Add(S0);
	Spectators.Add(S1);
	Spectators.Add(S2);
	Spectators.Add(S3);
	return Spectators;
}

// An Allies message reaches the sender and his ally, never an enemy.
void Test_ChatAlliesNotToEnemies(FUnitTest& T)
{
	TArray<int> To = HOLChat::Recipients(HOLChat::Allies, 0, -1, ChatTeams(), ChatSpectators(), false);
	T.AssertTrue(To.Contains(0) && To.Contains(1), "Allies: the sender and his ally");
	T.AssertFalse(To.Contains(2) || To.Contains(3), "Allies: no enemy");
}

// D376, free-for-all or Diplomacy: an Allies message of a living player does not reach a spectator, even his former ally.
void Test_ChatAlliesNotToSpectators(FUnitTest& T)
{
	TArray<int> To = HOLChat::Recipients(HOLChat::Allies, 0, -1, ChatTeams(0, 0, 1, 0), ChatSpectators(false, false, false, true), false);
	T.AssertTrue(To.Contains(1), "Allies: the living ally");
	T.AssertFalse(To.Contains(3), "Allies: not the spectating former ally");
	T.AssertFalse(To.Contains(2), "Allies: not the enemy");
}

// A private message reaches its target alone (and its sender).
void Test_ChatPrivateToTheTargetAlone(FUnitTest& T)
{
	TArray<int> To = HOLChat::Recipients(HOLChat::Private, 0, 2, ChatTeams(), ChatSpectators(), false);
	T.AssertTrue(To.Num() == 2 && To.Contains(0) && To.Contains(2), "Private: the sender and the target only");
	T.AssertFalse(To.Contains(1), "Private: not the sender's ally");
}

// D376: a spectator talks with the spectators alone in free-for-all, even on All and even privately to a living player.
void Test_ChatSpectatorTalksToSpectatorsOnly(FUnitTest& T)
{
	TArray<bool> Spectators = ChatSpectators(false, true, false, true);
	TArray<int> ToAll = HOLChat::Recipients(HOLChat::All, 1, -1, ChatTeams(), Spectators, false);
	T.AssertTrue(ToAll.Num() == 2 && ToAll.Contains(1) && ToAll.Contains(3), "spectator's All: the spectators only");
	TArray<int> ToPlayer = HOLChat::Recipients(HOLChat::Private, 1, 0, ChatTeams(), Spectators, false);
	T.AssertTrue(ToPlayer.Num() == 1 && ToPlayer[0] == 1, "spectator's private to a living player: nobody but himself");
}

// D357: in a team game a spectator still talks with his teammates (Allies), not with the enemies.
void Test_ChatTeamGameSpectatorTalksToTeammates(FUnitTest& T)
{
	TArray<int> To = HOLChat::Recipients(HOLChat::Allies, 1, -1, ChatTeams(), ChatSpectators(false, true, false, false), true);
	T.AssertTrue(To.Contains(0), "team game: the spectator reaches his living teammate");
	T.AssertFalse(To.Contains(2) || To.Contains(3), "team game: never the enemies");
	TArray<int> Back = HOLChat::Recipients(HOLChat::Allies, 0, -1, ChatTeams(), ChatSpectators(false, true, false, false), true);
	T.AssertTrue(Back.Contains(1), "team game: the living teammate's Allies reaches the spectating one");
}

// D806 (Q-chat-rules A): a spectator without a seat (a pure observer, D376) is a slot outside the seats (-1 on the server, or one past
// them for a second observer); he is treated as a defeated player: he talks and reads with the spectators on All and Allies, never
// privately, and he reads the living players' All, never their Allies. Slot 3 is defeated, 0 to 2 alive.
void Test_ChatSeatlessSpectatorIsADefeatedOne(FUnitTest& T)
{
	TArray<int> Teams = ChatTeams();
	TArray<bool> Spectators = ChatSpectators(false, false, false, true);
	TArray<int> ToAll = HOLChat::Recipients(HOLChat::All, -1, -1, Teams, Spectators, false);
	T.AssertTrue(ToAll.Num() == 2 && ToAll.Contains(-1) && ToAll.Contains(3), "seatless All: himself and the defeated slot, no living player");
	TArray<int> ToAllies = HOLChat::Recipients(HOLChat::Allies, -1, -1, Teams, Spectators, false);
	T.AssertTrue(ToAllies.Num() == 2 && ToAllies.Contains(3), "seatless Allies: the defeated slot, no living player");
	TArray<int> ToPrivate = HOLChat::Recipients(HOLChat::Private, -1, 3, Teams, Spectators, false);
	T.AssertTrue(ToPrivate.Num() == 1 && ToPrivate[0] == -1, "seatless private, even to a spectator: nobody but himself");
	T.AssertTrue(HOLChat::Reaches(HOLChat::All, -1, -1, 4, Teams, Spectators, false), "two observers: the All of one reaches the other");
	T.AssertTrue(HOLChat::Reaches(HOLChat::Allies, -1, -1, 4, Teams, Spectators, false), "two observers: the Allies of one reaches the other");
	T.AssertTrue(HOLChat::Reaches(HOLChat::Allies, 3, -1, -1, Teams, Spectators, false), "the defeated slot's Allies reaches the observer");
	T.AssertTrue(HOLChat::Reaches(HOLChat::All, 0, -1, -1, Teams, Spectators, false), "D792 1 A: the observer reads a living player's All");
	T.AssertFalse(HOLChat::Reaches(HOLChat::Allies, 0, -1, -1, Teams, Spectators, true), "never a living player's Allies, team game too");
}

// A living player's All reaches everybody, the spectators too.
void Test_ChatAllReachesEverybody(FUnitTest& T)
{
	TArray<int> To = HOLChat::Recipients(HOLChat::All, 2, -1, ChatTeams(), ChatSpectators(false, false, false, true), false);
	T.AssertTrue(To.Num() == 4, "All: the four slots");
}

// D807 (Q-chat-ui A): Enter opens the field on Allies (All without an ally); Tab goes Allies -> All -> Private (the player chosen in the
// Diplomacy panel) -> Allies, skipping a channel nobody would read.
void Test_ChatInputChannelCycle(FUnitTest& T)
{
	int Channel = HOLChat::OpeningChannel(true);
	T.AssertTrue(Channel == HOLChat::Allies, "Enter with an ally: the field opens on Allies");
	Channel = HOLChat::NextChannel(Channel, true, true);
	T.AssertTrue(Channel == HOLChat::All, "Tab: Allies -> All");
	Channel = HOLChat::NextChannel(Channel, true, true);
	T.AssertTrue(Channel == HOLChat::Private, "Tab: All -> Private");
	Channel = HOLChat::NextChannel(Channel, true, true);
	T.AssertTrue(Channel == HOLChat::Allies, "Tab: Private -> Allies");
	T.AssertTrue(HOLChat::NextChannel(HOLChat::All, true, false) == HOLChat::Allies, "Tab without a chosen player: All -> Allies");
	int Alone = HOLChat::OpeningChannel(false);
	T.AssertTrue(Alone == HOLChat::All, "Enter without an ally (free-for-all): the field opens on All");
	T.AssertTrue(HOLChat::NextChannel(Alone, false, true) == HOLChat::Private, "without an ally: All -> Private");
	T.AssertTrue(HOLChat::NextChannel(HOLChat::Private, false, true) == HOLChat::All, "without an ally: Private -> All, never Allies");
	T.AssertTrue(HOLChat::NextChannel(Alone, false, false) == HOLChat::All, "alone and no chosen player: Tab stays on All");
}

// MIG-P1a (D830, D835): the server's view, ported from Source/HeroesOfLiberty/Tests/HOLChatTransportTests.cpp (Chat.Route, Chat.Gate, same
// vectors) and from the 10 assertions of Docs/Patches/D806_cpp_twin_observer.patch (Chat.Observer, never applied). Three seats as netsig3:
// Alice (0) and Bob (1) mutual allies, Carol (2) apart; DefeatedSlot a spectator (D376).
TArray<bool> ChatThreeMates()
{
	TArray<bool> MateByPair;
	for (int Index = 0; Index < 9; ++Index)
	{
		MateByPair.Add(Index == 0 * 3 + 1 || Index == 1 * 3 + 0);
	}
	return MateByPair;
}

TArray<bool> ChatThreeSpectators(int DefeatedSlot = -1)
{
	TArray<bool> Spectators;
	for (int Slot = 0; Slot < 3; ++Slot)
	{
		Spectators.Add(Slot == DefeatedSlot);
	}
	return Spectators;
}

FString ChatSlots(const TArray<int>& Slots)
{
	FString Text;
	for (int Index = 0; Index < Slots.Num(); ++Index)
	{
		Text += Index > 0 ? f",{Slots[Index]}" : f"{Slots[Index]}";
	}
	return Text;
}

FString ChatBySeats(int Channel, int Sender, int Target, int DefeatedSlot, bool bTeamGame)
{
	return ChatSlots(HOLChat::RecipientsBySeats(Channel, Sender, Target, ChatThreeMates(), ChatThreeSpectators(DefeatedSlot), bTeamGame));
}

// Chat.Route: who reads, from the alliance matrix (Diplomacy unless a team game is said).
void Test_ChatRouteBySeats(FUnitTest& T)
{
	TArray<bool> Mates = ChatThreeMates();
	TArray<bool> Nobody = ChatThreeSpectators();
	T.AssertEquals("1,0", ChatBySeats(HOLChat::Allies, 1, -1, -1, false), "Bob's Allies: Bob then Alice, not Carol");
	T.AssertEquals("1,0,2", ChatBySeats(HOLChat::All, 1, -1, -1, false), "Bob's All: every seat");
	T.AssertEquals("1,2", ChatBySeats(HOLChat::Private, 1, 2, -1, false), "Bob's private to Carol: Carol alone");
	T.AssertFalse(HOLChat::ReachesBySeats(HOLChat::Allies, 1, -1, -1, Mates, Nobody, false), "a pure spectator (no seat) never reads an Allies message");
	T.AssertTrue(HOLChat::ReachesBySeats(HOLChat::All, 1, -1, -1, Mates, Nobody, false), "D792 1 A: a pure spectator reads the All of a living player");
	// D376: Carol defeated in Diplomacy talks with the spectators alone; D792 1 A: Bob's All still reaches her.
	T.AssertEquals("2", ChatBySeats(HOLChat::All, 2, -1, 2, false), "defeated Carol's All: herself alone");
	T.AssertEquals("1,0,2", ChatBySeats(HOLChat::All, 1, -1, 2, false), "Bob's All reaches the spectator Carol");
	// Alice defeated: in Diplomacy Bob's Allies does not reach her (D376); in a team game it does (D792 2 A, D357).
	T.AssertEquals("1", ChatBySeats(HOLChat::Allies, 1, -1, 0, false), "Diplomacy: Bob's Allies, not the spectating Alice");
	T.AssertEquals("1,0", ChatBySeats(HOLChat::Allies, 1, -1, 0, true), "team game: Bob's Allies reaches his spectating teammate");
}

// Chat.Observer (D806, Q-chat-rules A): an observer without a seat (-1, or slot 3 past the three seats for a second one) is treated as a
// defeated one: All and Allies with the spectators, never a private message. Carol (2) defeated.
void Test_ChatObserverBySeats(FUnitTest& T)
{
	TArray<bool> Mates = ChatThreeMates();
	TArray<bool> CarolOut = ChatThreeSpectators(2);
	const int Observer = -1;
	const int SecondObserver = 3;
	T.AssertEquals("-1,2", ChatBySeats(HOLChat::Allies, Observer, -1, 2, false), "the observer's Allies: the defeated Carol reads it, no living player");
	T.AssertTrue(HOLChat::ReachesBySeats(HOLChat::Allies, Observer, -1, SecondObserver, Mates, CarolOut, false), "the observer's Allies reaches a second observer");
	T.AssertTrue(HOLChat::ReachesBySeats(HOLChat::Allies, 2, -1, Observer, Mates, CarolOut, false), "the defeated Carol's Allies reaches the observer");
	T.AssertEquals("-1", ChatBySeats(HOLChat::Private, Observer, 2, 2, false), "the observer's private to the defeated Carol: nobody");
	T.AssertFalse(HOLChat::ReachesBySeats(HOLChat::Private, Observer, SecondObserver, SecondObserver, Mates, CarolOut, false), "the observer's private reaches no second observer");
	T.AssertFalse(HOLChat::ReachesBySeats(HOLChat::Private, Observer, 2, Observer, Mates, CarolOut, false), "two observers share -1: a private one is not even read back");
	T.AssertTrue(HOLChat::ReachesBySeats(HOLChat::All, Observer, -1, Observer, Mates, CarolOut, false), "two observers share -1: their All is read");
	T.AssertEquals("-1,2", ChatBySeats(HOLChat::All, Observer, -1, 2, false), "the observer's All: the spectators alone (D376)");
	T.AssertTrue(HOLChat::ReachesBySeats(HOLChat::All, 0, -1, Observer, Mates, CarolOut, false), "D792 1 A: Alice's All reaches the observer");
	T.AssertFalse(HOLChat::ReachesBySeats(HOLChat::Allies, 0, -1, Observer, Mates, CarolOut, false), "Alice's Allies never reaches the observer");
}

// A text of Count characters.
FString ChatText(int Count)
{
	FString Text;
	for (int Index = 0; Index < Count; ++Index)
	{
		Text += "a";
	}
	return Text;
}

// Chat.Gate: the server's refusals (Bob posts among three seats; length 12, 3 posts per 5000 ms as the C++ test).
void Test_ChatGateRefusals(FUnitTest& T)
{
	TArray<int> Recent;
	T.AssertEquals(HOLChat::Ok, HOLChat::CheckPost(3, HOLChat::All, 1, -1, ChatText(12), Recent, 0, 12, 3, 5000), "a text of the maximum length passes");
	T.AssertEquals(HOLChat::TooLong, HOLChat::CheckPost(3, HOLChat::All, 1, -1, ChatText(13), Recent, 0, 12, 3, 5000), "one character more is refused");
	T.AssertEquals(HOLChat::Empty, HOLChat::CheckPost(3, HOLChat::All, 1, -1, "  \t ", Recent, 0, 12, 3, 5000), "an empty text is refused");
	T.AssertEquals(HOLChat::BadTarget, HOLChat::CheckPost(3, HOLChat::Private, 1, 1, "hi", Recent, 0, 12, 3, 5000), "a private message to oneself is refused");
	T.AssertEquals(HOLChat::BadTarget, HOLChat::CheckPost(3, HOLChat::Private, 1, 3, "hi", Recent, 0, 12, 3, 5000), "a private message to no seat is refused");
	T.AssertEquals(HOLChat::BadChannel, HOLChat::CheckPost(3, 7, 1, -1, "hi", Recent, 0, 12, 3, 5000), "an unknown channel is refused");
	T.AssertEquals(1, Recent.Num(), "the refused posts never count in the rate");
}

// Chat.Gate: the rate, 3 posts within the window, the next refused; once the first is older than the window, one more passes.
void Test_ChatGateRate(FUnitTest& T)
{
	TArray<int> Rate;
	FString Answers;
	for (int Index = 0; Index <= 3; ++Index)
	{
		const int Answer = HOLChat::CheckPost(3, HOLChat::Allies, 1, -1, "go", Rate, Index * 100, 12, 3, 5000);
		Answers += f"{Answer},";
	}
	const int Last = HOLChat::CheckPost(3, HOLChat::Allies, 1, -1, "go", Rate, 5000, 12, 3, 5000);
	Answers += f"{Last}";
	T.AssertEquals(f"{HOLChat::Ok},{HOLChat::Ok},{HOLChat::Ok},{HOLChat::TooFast},{HOLChat::Ok}", Answers, "the rate: ok, ok, ok, too fast, then ok after the window");
	// The game's defaults (registry): 200 characters, 5 posts per 10 s.
	TArray<int> Defaults;
	T.AssertEquals(HOLChat::TooLong, HOLChat::CheckPost(3, HOLChat::All, 0, -1, ChatText(201), Defaults, 0, HOLChat::MaxTextLength, HOLChat::MaxPerWindow, HOLChat::WindowMs), "201 characters refused by default");
}
