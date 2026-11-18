// Lot MIG-C-ForumMessages of the migration (D830, D836, D839): the script port of HOLForumMessagesTests.cpp (10 tests), run against
// UHOLForumMessagesReadLibrary (namespace HOLForumMessagesRead, Source/HeroesOfLiberty/Match/HOLForumMessagesReadLibrary.h) and its two
// holders (UHOLForumThreadHolder, UHOLForumAlertHolder), the read library of the rule that STAYS in C++ (HOLForumMessages).
// Same vectors as the C++ tests, one by one. The C++ file is removed at lot MIG-C++-FINAL (after red seen, then green).
// Event kinds: 0 Fell, 1 HoldStarted, 2 Razed, 3 Defeated, 4 KingDied. Hold kinds: 0 None, 1 Holding, 2 Contested, 3 Receding.

FHOLForumEventValue FmEv(int Kind, int Slot, int Other = -1, int Lives = 0)
{
	FHOLForumEventValue Event;
	Event.Kind = Kind;
	Event.Slot = Slot;
	Event.OtherSlot = Other;
	Event.Lives = Lives;
	return Event;
}

// Slots 0 and 1 in team 1 (Human, AI), slot 2 in team 2 (AI), every other slot closed.
FHOLForumScenario FmTeams()
{
	FHOLForumScenario Scenario;
	Scenario.States.Add(1);
	Scenario.States.Add(2);
	Scenario.States.Add(2);
	Scenario.Teams.Add(1);
	Scenario.Teams.Add(1);
	Scenario.Teams.Add(2);
	return Scenario;
}

// MakeTeams plus slot 3 an AI of team 2.
FHOLForumScenario FmTeamsWithSlot3()
{
	FHOLForumScenario Scenario = FmTeams();
	Scenario.States.Add(2);
	Scenario.Teams.Add(2);
	return Scenario;
}

// MakeTeams, slot 0 named, slot 1 a named human, slot 3 a human (team 0) named (Name3 empty: a human without a name).
FHOLForumScenario FmLobby(FString Name0, FString Name1, FString Name3)
{
	FHOLForumScenario Scenario = FmTeams();
	Scenario.States.Add(1);
	Scenario.Teams.Add(0);
	Scenario.States[1] = 1;
	Scenario.Names.Add(Name0);
	Scenario.Names.Add(Name1);
	Scenario.Names.Add("");
	Scenario.Names.Add(Name3);
	return Scenario;
}

FHOLForumMessageValue FmMsg(FHOLForumEventValue Event, FHOLForumScenario Scenario, int Viewer, bool bSees = false)
{
	return HOLForumMessagesRead::GetMessage(Event, Scenario, Viewer, bSees);
}

bool FmSlotsAre1(const TArray<int>& Slots, int A)
{
	return Slots.Num() == 1 && Slots[0] == A;
}

bool FmSlotsAre2(const TArray<int>& Slots, int A, int B)
{
	return Slots.Num() == 2 && Slots[0] == A && Slots[1] == B;
}

// HeroesOfLiberty.Victory.Messages.Fell
void Test_VictoryMessagesFell(FUnitTest& T)
{
	FHOLForumScenario Teams = FmTeams();
	FHOLForumEventValue Fell = FmEv(0, 0, -1, 1);

	FHOLForumMessageValue Message = FmMsg(Fell, Teams, 0);
	T.AssertEquals("Votre forum est tombé — 1 vie", Message.Source, "own: text");
	T.AssertTrue(Message.bAlert, "own: alert");
	T.AssertTrue(Message.Shown.StartsWith("Message: ") && Message.Shown.EndsWith(" (alert)"), "own: logged with (alert)");

	Message = FmMsg(Fell, Teams, 1);
	T.AssertEquals("Le forum de Joueur 1 est tombé — 1 vie", Message.Source, "ally: text");
	T.AssertTrue(Message.bAlert, "ally: alert");
	T.AssertTrue(FmSlotsAre1(Message.NamedSlots, 0), "ally: the slot named");

	Message = FmMsg(Fell, Teams, 2);
	T.AssertEquals("Le forum de Joueur 1 est tombé — 1 vie", Message.Source, "adversary: text");
	T.AssertFalse(Message.bAlert, "adversary: no alert");
	T.AssertTrue(Message.Shown.StartsWith("Message: ") && !Message.Shown.EndsWith("(alert)"), "adversary: logged without (alert)");

	Message = FmMsg(Fell, Teams, -1);
	T.AssertEquals("Le forum de Joueur 1 est tombé — 1 vie", Message.Source, "spectator: text");
	T.AssertFalse(Message.bAlert, "spectator: no alert");

	// V4.5b: its last life gone, its own loss: no alert, the thread's line alone; for its ally, the alert stays.
	FHOLForumEventValue Last = FmEv(0, 0, -1, 0);
	Message = FmMsg(Last, Teams, 0);
	T.AssertFalse(Message.bAlert, "own, 0 vies: no alert (its loss)");
	T.AssertEquals("Message: Votre forum est tombé — 0 vies", Message.Shown, "own, 0 vies: logged without (alert)");
	FHOLForumMessageValue LastAlly = FmMsg(Last, Teams, 1);
	T.AssertTrue(LastAlly.bAlert, "ally, 0 vies: alert");

	// The singular for exactly 1, the plural for 0 and 2 (as the players' table).
	FHOLForumMessageValue Zero = FmMsg(FmEv(0, 0, -1, 0), Teams, 2);
	T.AssertEquals("Le forum de Joueur 1 est tombé — 0 vies", Zero.Source, "0 vies");
	FHOLForumMessageValue Two = FmMsg(FmEv(0, 0, -1, 2), Teams, 0);
	T.AssertEquals("Votre forum est tombé — 2 vies", Two.Source, "2 vies, own");
}

// HeroesOfLiberty.Victory.Messages.HoldStarted
void Test_VictoryMessagesHoldStarted(FUnitTest& T)
{
	FHOLForumScenario Teams = FmTeams();
	// Slot 2 holds the ruin of slot 0.
	FHOLForumEventValue Hold = FmEv(1, 0, 2);

	FHOLForumMessageValue Message = FmMsg(Hold, Teams, 0);
	T.AssertEquals("IA Normal 3 tient votre ruine", Message.Source, "own: text");
	T.AssertTrue(Message.bAlert, "own: alert");
	T.AssertTrue(FmSlotsAre1(Message.NamedSlots, 2), "own: the attacker named");

	Message = FmMsg(Hold, Teams, 1);
	T.AssertEquals("IA Normal 3 tient la ruine de Joueur 1", Message.Source, "ally: text");
	T.AssertTrue(Message.bAlert, "ally: alert");
	T.AssertTrue(FmSlotsAre2(Message.NamedSlots, 2, 0), "ally: both named, in order");

	Message = FmMsg(Hold, Teams, 2);
	T.AssertEquals("Vous tenez la ruine de Joueur 1", Message.Source, "the attacker: text");
	T.AssertFalse(Message.bAlert, "the attacker: no alert");

	Message = FmMsg(Hold, Teams, -1);
	T.AssertEquals("IA Normal 3 tient la ruine de Joueur 1", Message.Source, "spectator: text");
	T.AssertFalse(Message.bAlert, "spectator: no alert");
}

// HeroesOfLiberty.Victory.Messages.Razed
void Test_VictoryMessagesRazed(FUnitTest& T)
{
	FHOLForumScenario Teams = FmTeams();
	FHOLForumEventValue Razed = FmEv(2, 2, -1, 0);

	FHOLForumMessageValue Message = FmMsg(Razed, Teams, 2);
	T.AssertEquals("Vos fondations sont rasées — 0 vies", Message.Source, "own: text");
	// V4.5b: its last life razed, its own loss: the thread's line alone.
	T.AssertFalse(Message.bAlert, "own, at 0 lives: no alert (its loss)");
	T.AssertEquals("Message: " + Message.Display, Message.Shown, "own, at 0 lives: logged without (alert)");
	FHOLForumMessageValue OneLife = FmMsg(FmEv(2, 2, -1, 1), Teams, 2);
	T.AssertTrue(OneLife.bAlert, "own, at 1 life: alert");

	Message = FmMsg(Razed, Teams, 0);
	T.AssertEquals("Fondations de IA Normal 3 rasées — 0 vies", Message.Source, "adversary: text");
	T.AssertFalse(Message.bAlert, "adversary: no alert");

	Message = FmMsg(FmEv(2, 2, -1, 1), Teams, -1);
	T.AssertEquals("Fondations de IA Normal 3 rasées — 1 vie", Message.Source, "spectator: 1 vie");
	T.AssertFalse(Message.bAlert, "spectator: no alert");
}

// HeroesOfLiberty.Victory.Messages.Defeated
void Test_VictoryMessagesDefeated(FUnitTest& T)
{
	FHOLForumScenario Teams = FmTeams();
	FHOLForumEventValue Defeated = FmEv(3, 1);

	FHOLForumMessageValue Message = FmMsg(Defeated, Teams, 1);
	T.AssertEquals("Vous êtes vaincu", Message.Source, "own: text");
	// V4.5b: its own defeat, the thread's line alone (its Defeat banner comes), whatever the replication's order.
	T.AssertFalse(Message.bAlert, "own: no alert (its loss)");
	T.AssertEquals("Message: Vous êtes vaincu", Message.Shown, "own: logged without (alert)");
	T.AssertTrue(Message.NamedSlots.Num() == 0 || FmSlotsAre1(Message.NamedSlots, 1), "own: nobody else named");

	Message = FmMsg(Defeated, Teams, 0);
	T.AssertEquals("IA Normal 2 est vaincu", Message.Source, "ally: text");
	T.AssertTrue(Message.bAlert, "ally: alert");

	Message = FmMsg(Defeated, Teams, 2);
	T.AssertEquals("IA Normal 2 est vaincu", Message.Source, "adversary: text");
	T.AssertFalse(Message.bAlert, "adversary: no alert");

	Message = FmMsg(Defeated, Teams, -1);
	T.AssertFalse(Message.bAlert, "spectator: no alert");
	T.AssertEquals("Message: " + Message.Display, Message.Shown, "spectator: logged");
}

// HeroesOfLiberty.Victory.Messages.HoldAnnounced: V4.2, HoldStarted once per attacker reaching Holding since the count was at 0.
void Test_VictoryMessagesHoldAnnounced(FUnitTest& T)
{
	int Announced = 0;
	// None -> Contested -> Holding: the attacker killed the defender, its hold announced.
	T.AssertFalse(HOLForumMessagesRead::ShouldAnnounceHold(2, 2, 0, Announced), "contested at 0: no message");
	T.AssertTrue(HOLForumMessagesRead::ShouldAnnounceHold(1, 2, 1, Announced), "then holding: a message");
	T.AssertFalse(HOLForumMessagesRead::ShouldAnnounceHold(1, 2, 2, Announced), "holding on: no second message");
	// Contested, receding, then the same attacker again: no new message.
	T.AssertFalse(HOLForumMessagesRead::ShouldAnnounceHold(2, 2, 2, Announced), "contested");
	T.AssertFalse(HOLForumMessagesRead::ShouldAnnounceHold(1, 2, 3, Announced), "the same again after a contest");
	T.AssertFalse(HOLForumMessagesRead::ShouldAnnounceHold(3, 2, 2, Announced), "receding");
	T.AssertFalse(HOLForumMessagesRead::ShouldAnnounceHold(1, 2, 3, Announced), "the same again after a recession");
	// Another attacker takes the lead: a message; the first back: none (already announced).
	T.AssertTrue(HOLForumMessagesRead::ShouldAnnounceHold(1, 1, 4, Announced), "another attacker");
	T.AssertFalse(HOLForumMessagesRead::ShouldAnnounceHold(1, 2, 5, Announced), "the first back");
	// Back to 0 (None): forgotten, the same attacker announced anew.
	T.AssertFalse(HOLForumMessagesRead::ShouldAnnounceHold(0, -1, 0, Announced), "none");
	T.AssertEquals(0, Announced, "forgotten at 0");
	T.AssertTrue(HOLForumMessagesRead::ShouldAnnounceHold(1, 2, 1, Announced), "anew from 0");
	// No attacker known: no message.
	Announced = 0;
	T.AssertFalse(HOLForumMessagesRead::ShouldAnnounceHold(1, -1, 1, Announced), "no holder");
}

// HeroesOfLiberty.Victory.Messages.Thread
void Test_VictoryMessagesThread(FUnitTest& T)
{
	FHOLForumScenario Teams = FmTeams();
	FHOLForumEventValue Fell = FmEv(0, 0, -1, 1);
	FHOLForumEventValue Defeated = FmEv(3, 1);

	UHOLForumThreadHolder Thread = UHOLForumThreadHolder::Create();
	T.AssertEquals("[]", Thread.Describe(), "empty");
	Thread.AddEvent(Fell, Teams, 0, 10.0);
	Thread.AddEvent(Defeated, Teams, 0, 12.0);
	int Count = Thread.GetLineCount();
	T.AssertEquals(2, Count, "two lines");
	if (Count != 2)
		return;
	T.AssertEquals("[Votre forum est tombé — 1 vie | IA Normal 2 est vaincu]", Thread.Describe(), "the newest at the bottom");
	T.AssertEquals(1.0, Thread.GetLineOpacity(0, 10.0), "opaque at first");
	double Fading = Thread.GetLineOpacity(0, 17.75);
	T.AssertTrue(Fading < 1.0 && Fading > 0.0, "fading near its end");
	T.AssertFalse(Thread.Expire(17.9), "nothing gone before 8 s");
	T.AssertTrue(Thread.Expire(18.0), "the first gone at 8 s");
	T.AssertEquals("[IA Normal 2 est vaincu]", Thread.Describe(), "the second left");
	T.AssertTrue(Thread.Expire(20.0), "the second gone at 20 s");
	Count = Thread.GetLineCount();
	T.AssertEquals(0, Count, "empty again");

	// At most 5 lines: the oldest dropped.
	for (int Index = 0; Index < 7; ++Index)
	{
		if (Index == 6)
			Thread.AddEvent(Defeated, Teams, 0, 30.0 + Index);
		else
			Thread.AddEvent(Fell, Teams, 0, 30.0 + Index);
	}
	Count = Thread.GetLineCount();
	int MaxLines = HOLForumMessagesRead::GetThreadMaxLines();
	T.AssertEquals(MaxLines, Count, "5 lines at most");
	if (Count != MaxLines)
		return;
	double Oldest = Thread.GetLineAddedSeconds(0);
	T.AssertEquals(32.0, Oldest, "the oldest dropped");
	FString NewestText = Thread.GetLineText(Count - 1);
	FHOLForumMessageValue DefeatedMessage = FmMsg(Defeated, Teams, 0);
	T.AssertEquals(DefeatedMessage.Display, NewestText, "the newest last");

	// The names cut out for their colours.
	TArray<FHOLForumPieceValue> Pieces = HOLForumMessagesRead::SplitNames(FmEv(1, 0, 2), Teams, 1);
	T.AssertEquals(3, Pieces.Num(), "pieces");
	if (Pieces.Num() == 3)
	{
		T.AssertEquals("IA Normal 3", Pieces[0].Text, "attacker");
		T.AssertEquals(2, Pieces[0].Slot, "attacker slot");
		T.AssertEquals(" tient la ruine de ", Pieces[1].Text, "words");
		T.AssertEquals(-1, Pieces[1].Slot, "words slot");
		T.AssertEquals("Joueur 1", Pieces[2].Text, "owner");
		T.AssertEquals(0, Pieces[2].Slot, "owner slot");
	}
	TArray<FHOLForumPieceValue> Own = HOLForumMessagesRead::SplitNames(Fell, Teams, 0);
	T.AssertTrue(Own.Num() == 1 && Own[0].Slot == -1, "no name: one piece");
}

// HeroesOfLiberty.Victory.Messages.LobbyNames: the names the lobby gave the slots, as the players' table, not "Joueur N".
void Test_VictoryMessagesLobbyNames(FUnitTest& T)
{
	// Not French: the patterns stay as they are (the elision, test Elision, is French only).
	FString Previous = HOLForumMessagesRead::SetCultureName("en");
	FHOLForumScenario Lobby = FmLobby("Aragorn", " Boromir ", "");
	// Slot 2 left without a name (an AI): its AI name; slot 3 a human without a name: Joueur 4.

	FHOLForumMessageValue Message = FmMsg(FmEv(3, 1), Lobby, 2);
	T.AssertEquals("Boromir est vaincu", Message.Source, "defeated, named");
	Message = FmMsg(FmEv(0, 0, -1, 1), Lobby, 1);
	T.AssertEquals("Le forum de Aragorn est tombé — 1 vie", Message.Source, "fell, named");
	Message = FmMsg(FmEv(1, 0, 1), Lobby, 1);
	T.AssertEquals("Vous tenez la ruine de Aragorn", Message.Source, "own: still Vous");
	Message = FmMsg(FmEv(3, 2), Lobby, 0);
	T.AssertTrue(Message.Source.StartsWith("IA "), "an AI: its AI name");
	Message = FmMsg(FmEv(3, 3), Lobby, 0);
	T.AssertEquals("Joueur 4 est vaincu", Message.Source, "a human without a name: Joueur N");

	TArray<FHOLForumPieceValue> Pieces = HOLForumMessagesRead::SplitNames(FmEv(1, 1, 2), Lobby, 0);
	T.AssertEquals(3, Pieces.Num(), "pieces");
	if (Pieces.Num() == 3)
	{
		T.AssertEquals(2, Pieces[0].Slot, "attacker slot");
		T.AssertEquals("Boromir", Pieces[2].Text, "owner, named");
		T.AssertEquals(1, Pieces[2].Slot, "owner slot");
	}
	HOLForumMessagesRead::SetCultureName(Previous);
}

// HeroesOfLiberty.Victory.Messages.Elision: V4 (answer B, 2026-10-30), the French elision "de" + a vowel or a silent h -> "d'", French only.
void Test_VictoryMessagesElision(FUnitTest& T)
{
	// The pure rule.
	TArray<FString> Elides;
	Elides.Add("Aragorn"); Elides.Add("Elrond"); Elides.Add("Ulf"); Elides.Add("Ivar"); Elides.Add("Olaf"); Elides.Add("Yvain");
	Elides.Add("Éowyn"); Elides.Add("Œdipe"); Elides.Add("Hugo"); Elides.Add("Héloïse"); Elides.Add("aragorn"); Elides.Add("IA Facile 2");
	for (int Index = 0; Index < Elides.Num(); ++Index)
		T.AssertTrue(HOLForumMessagesRead::ElidesDe(Elides[Index]), Elides[Index] + " elides");
	TArray<FString> Plain;
	Plain.Add("Bob"); Plain.Add("Joueur 2"); Plain.Add("Gimli"); Plain.Add("Hans"); Plain.Add("Harald"); Plain.Add("Hr"); Plain.Add("8"); Plain.Add(""); Plain.Add("-x");
	for (int Index = 0; Index < Plain.Num(); ++Index)
		T.AssertFalse(HOLForumMessagesRead::ElidesDe(Plain[Index]), Plain[Index] + " does not");
	FString Pattern = "{0} tient la ruine de {1}";
	FString Elided = HOLForumMessagesRead::ElidePattern(Pattern, 1, "Elrond");
	T.AssertEquals("{0} tient la ruine d'{1}", Elided, "pattern");
	Elided = HOLForumMessagesRead::ElidePattern(Pattern, 1, "Bob");
	T.AssertEquals("{0} tient la ruine de {1}", Elided, "pattern, a consonant");
	Elided = HOLForumMessagesRead::ElidePattern(Pattern, 0, "Elrond");
	T.AssertEquals("{0} tient la ruine de {1}", Elided, "pattern, the other argument");

	// The messages, in French.
	FString Previous = HOLForumMessagesRead::SetCultureName("fr");
	FHOLForumScenario Lobby = FmLobby("Aragorn", "Bob", "Elrond");
	FHOLForumMessageValue Message = FmMsg(FmEv(0, 0, -1, 1), Lobby, 1);
	T.AssertEquals("Le forum d'Aragorn est tombé — 1 vie", Message.Display, "fell");
	Message = FmMsg(FmEv(0, 1, -1, 2), Lobby, 0);
	T.AssertEquals("Le forum de Bob est tombé — 2 vies", Message.Display, "fell, a consonant");
	Message = FmMsg(FmEv(1, 0, 1), Lobby, 1);
	T.AssertEquals("Vous tenez la ruine d'Aragorn", Message.Display, "you hold");
	Message = FmMsg(FmEv(1, 3, 1), Lobby, 2);
	T.AssertEquals("Bob tient la ruine d'Elrond", Message.Display, "held");
	Message = FmMsg(FmEv(1, 1, 3), Lobby, 2);
	T.AssertEquals("Elrond tient la ruine de Bob", Message.Display, "held, the attacker is Elrond");
	Message = FmMsg(FmEv(2, 0, -1, 1), Lobby, 1);
	T.AssertEquals("Fondations d'Aragorn rasées — 1 vie", Message.Display, "razed");
	Message = FmMsg(FmEv(4, 0), Lobby, 1);
	T.AssertEquals("Le Roi d'Aragorn est mort", Message.Display, "king died");
	Message = FmMsg(FmEv(4, 1), Lobby, 0);
	T.AssertEquals("Le Roi de Bob est mort", Message.Display, "king died, a consonant");
	// An AI's name starts with "IA": "d'IA Facile 3".
	Message = FmMsg(FmEv(4, 2), Lobby, 0);
	T.AssertTrue(Message.Display.StartsWith("Le Roi d'IA "), "an AI");
	// The names stay cut for their colours.
	TArray<FHOLForumPieceValue> Pieces = HOLForumMessagesRead::SplitNames(FmEv(1, 3, 1), Lobby, 2);
	T.AssertEquals(3, Pieces.Num(), "three pieces");
	if (Pieces.Num() == 3)
	{
		T.AssertEquals("Elrond", Pieces[2].Text, "the owner is the last piece");
		T.AssertEquals(3, Pieces[2].Slot, "its slot");
	}

	// Not French: nothing changes.
	HOLForumMessagesRead::SetCultureName("en");
	Message = FmMsg(FmEv(0, 0, -1, 1), Lobby, 1);
	T.AssertEquals("Le forum de Aragorn est tombé — 1 vie", Message.Source, "English, no elision");
	HOLForumMessagesRead::SetCultureName("ja");
	Message = FmMsg(FmEv(4, 0), Lobby, 1);
	T.AssertEquals("Le Roi de Aragorn est mort", Message.Source, "Japanese, no elision");
	HOLForumMessagesRead::SetCultureName(Previous);
}

// HeroesOfLiberty.Victory.Messages.Alert: V4.3, the major alert's banner: only for an alert, 4 s then gone, a new one replacing it, none once the match is over for me.
void Test_VictoryMessagesAlert(FUnitTest& T)
{
	FHOLForumScenario Teams = FmTeams();
	// Slot 0 sees its own forum fall (an alert), its ally's (slot 1), slot 2's (an adversary's: no alert).
	FHOLForumEventValue Own = FmEv(0, 0, -1, 1);
	FHOLForumEventValue Ally = FmEv(0, 1, -1, 1);
	FHOLForumEventValue Adversary = FmEv(0, 2, -1, 1);
	UHOLForumAlertHolder Alert = UHOLForumAlertHolder::Create();
	T.AssertEquals("none", Alert.Describe(), "none at first");
	T.AssertFalse(Alert.ShowEvent(Adversary, Teams, 0, 2, false, 10.0), "an adversary's forum: no banner");
	T.AssertEquals("none", Alert.Describe(), "still none");
	T.AssertTrue(Alert.ShowEvent(Own, Teams, 0, 0, false, 10.0), "my forum: the banner");
	T.AssertEquals("Votre forum est tombé — 1 vie (slot 0)", Alert.Describe(), "its text and slot");
	T.AssertEquals(1.0, Alert.GetOpacity(10.0), "opaque at first");
	double Fading = Alert.GetOpacity(13.75);
	T.AssertTrue(Fading > 0.0 && Fading < 1.0, "fading near its end");
	T.AssertFalse(Alert.Expire(13.9), "still there before 4 s");
	T.AssertTrue(Alert.Expire(14.0), "gone at 4 s");
	T.AssertEquals("none", Alert.Describe(), "none again");
	T.AssertEquals(0.0, Alert.GetOpacity(14.0), "hidden: transparent");
	T.AssertFalse(Alert.Expire(15.0), "gone once only");

	// A new alert replaces the shown one, its 4 s from its own start.
	T.AssertTrue(Alert.ShowEvent(Own, Teams, 0, 0, false, 20.0), "mine");
	T.AssertTrue(Alert.ShowEvent(Ally, Teams, 0, 1, false, 22.0), "my ally's replaces it");
	int AllySlot = Alert.GetSlot();
	T.AssertEquals(1, AllySlot, "the ally's slot");
	T.AssertFalse(Alert.Expire(24.5), "not gone at the first one's 4 s");
	T.AssertTrue(Alert.Expire(26.0), "gone at its own 4 s");

	// The match over for me (my Victory / Defeat banner): the thread's line is enough.
	T.AssertFalse(Alert.ShowEvent(Own, Teams, 0, 0, true, 30.0), "match over for me: no banner");
	bool bShownThenOver = Alert.ShowEvent(Own, Teams, 0, 0, false, 40.0) && !Alert.ShowEvent(Ally, Teams, 0, 1, true, 41.0) && Alert.GetSlot() == 0;
	T.AssertTrue(bShownThenOver, "shown, then the match over for me: kept, its 4 s");

	// V4.5b: a banner kept 1.5 s at least; one come sooner waits, then is shown its 4 s; one sound for a burst.
	UHOLForumAlertHolder Burst = UHOLForumAlertHolder::Create();
	bool bFirst = Burst.ShowEvent(Own, Teams, 0, 0, false, 50.0) && Burst.HasSound();
	T.AssertTrue(bFirst, "burst: the fall shown");
	T.AssertFalse(Burst.ShowEvent(Ally, Teams, 0, 1, false, 50.04), "burst: the hold 40 ms later waits");
	int BurstSlot = Burst.GetSlot();
	T.AssertEquals(0, BurstSlot, "burst: the fall still shown");
	T.AssertFalse(Burst.Advance(51.4, false), "burst: still waiting at 1.4 s");
	T.AssertFalse(Burst.Expire(51.4), "burst: not hidden while one waits");
	T.AssertTrue(Burst.Advance(51.5, false), "burst: the hold shown at 1.5 s");
	BurstSlot = Burst.GetSlot();
	T.AssertEquals(1, BurstSlot, "burst: the hold's slot");
	T.AssertFalse(Burst.HasSound(), "burst: the hold silent (came within 1 s)");
	T.AssertFalse(Burst.Expire(55.4), "burst: the hold its 4 s");
	T.AssertTrue(Burst.Expire(55.5), "burst: gone at its 4 s");
	// Three more within 1 s: at most 2 waiting, the oldest dropped, one sound.
	bool bSecond = Burst.ShowEvent(Own, Teams, 0, 0, false, 60.0) && Burst.HasSound();
	T.AssertTrue(bSecond, "burst 2: shown");
	Burst.ShowEvent(Ally, Teams, 0, 1, false, 60.1);
	Burst.ShowEvent(Own, Teams, 0, 3, false, 60.2);
	Burst.ShowEvent(Ally, Teams, 0, 4, false, 60.3);
	int Waiting = Burst.GetWaitingCount();
	int MaxWaiting = HOLForumMessagesRead::GetAlertMaxWaiting();
	T.AssertEquals(MaxWaiting, Waiting, "burst 2: 2 waiting");
	bool bNext = Burst.Advance(61.5, false) && Burst.GetSlot() == 3 && !Burst.HasSound();
	T.AssertTrue(bNext, "burst 2: the next at 1.5 s, the oldest waiting dropped");
	T.AssertFalse(Burst.Advance(62.9, false), "burst 2: the last waits 1.5 s more");
	bool bLast = Burst.Advance(63.0, false) && Burst.GetSlot() == 4 && !Burst.HasSound();
	T.AssertTrue(bLast, "burst 2: the last at 3 s");
	// More than 1 s after: its own sound.
	T.AssertFalse(Burst.ShowEvent(Own, Teams, 0, 0, false, 64.2), "later: waits");
	bool bLater = Burst.Advance(64.5, false) && Burst.HasSound();
	T.AssertTrue(bLater, "later: shown with its sound");
	// The match over for me: the waiting ones dropped.
	Burst.ShowEvent(Ally, Teams, 0, 1, false, 64.6);
	T.AssertFalse(Burst.Advance(66.5, true), "match over: nothing more shown");
	Waiting = Burst.GetWaitingCount();
	T.AssertEquals(0, Waiting, "match over: none waiting");

	// A spectator is never alerted.
	UHOLForumAlertHolder Watching = UHOLForumAlertHolder::Create();
	T.AssertFalse(Watching.ShowEvent(Own, Teams, -1, 0, false, 10.0), "spectator: no banner");
}

// HeroesOfLiberty.Victory.Messages.Vision: F6e.4 (D353), who has the vision of a forum gets the major alert, its adversary included; who does not, the message only.
void Test_VictoryMessagesVision(FUnitTest& T)
{
	FHOLForumScenario Teams = FmTeamsWithSlot3();
	FHOLForumEventValue Fell = FmEv(0, 0, -1, 1);
	FHOLForumMessageValue NoVision = FmMsg(Fell, Teams, 2, false);
	FHOLForumMessageValue WithVision = FmMsg(Fell, Teams, 2, true);
	T.AssertFalse(NoVision.bAlert, "the adversary, no vision: the message only");
	T.AssertTrue(WithVision.bAlert, "the adversary, with the vision: the alert");
	T.AssertEquals(NoVision.Source, WithVision.Source, "the text is the same");
	FHOLForumMessageValue Message = FmMsg(FmEv(2, 0, -1, 1), Teams, 2, true);
	T.AssertTrue(Message.bAlert, "Razed, with the vision");
	Message = FmMsg(FmEv(1, 0, 2), Teams, 3, true);
	T.AssertTrue(Message.bAlert, "HoldStarted, with the vision");
	Message = FmMsg(Fell, Teams, -1, true);
	T.AssertFalse(Message.bAlert, "a spectator is never alerted");
	Message = FmMsg(FmEv(3, 0), Teams, 2, true);
	T.AssertFalse(Message.bAlert, "a defeat is no forum to see");
	Message = FmMsg(FmEv(4, 0), Teams, 2, true);
	T.AssertFalse(Message.bAlert, "nor a King's death");
	Message = FmMsg(FmEv(0, 0, -1, 0), Teams, 0, true);
	T.AssertFalse(Message.bAlert, "its own loss stays without alert");

	// The ally of the one who holds the ruin (D353), without the vision; the one who holds it has the vision or not as any other.
	FHOLForumEventValue Hold = FmEv(1, 0, 2);
	Message = FmMsg(Hold, Teams, 3, false);
	T.AssertTrue(Message.bAlert, "the attacker's ally: the alert");
	Message = FmMsg(Hold, Teams, 2, false);
	T.AssertFalse(Message.bAlert, "the attacker himself, no vision: as before");
	Message = FmMsg(Fell, Teams, 3, false);
	T.AssertFalse(Message.bAlert, "a Fell has no attacker: its adversary's ally has no alert");
}
