// The stances of the Diplomacy mode (D369, D370, D375, D401, D367; GDD 14.3): Ally / Neutral / Enemy of each player toward each other,
// the relation of a pair (the most hostile of the two), the immediate break, and what is announced to all (HOLDiplomacyRules.as). Pure.

// A book of Slots humans, in the Diplomacy mode (unlocked), every stance at the default.
FHOLDiplomacyBook HumansBook(int Slots)
{
	TArray<bool> NoAI;
	for (int Slot = 0; Slot < Slots; Slot++)
		NoAI.Add(false);
	return HOLDiplomacy::NewBook(Slots, NoAI);
}

// The default stance is the same for every pair, and one is always allied with oneself.
void Test_DiplomacyDefaultStance(FUnitTest& T)
{
	FHOLDiplomacyBook Book = HumansBook(4);
	T.AssertEquals(4, Book.Slots, "four slots");
	for (int From = 0; From < 4; From++)
	{
		for (int To = 0; To < 4; To++)
		{
			if (From == To)
				T.AssertEquals(HOLDiplomacy::Ally, HOLDiplomacy::Relation(Book, From, To), "alone with oneself: ally");
			else
			{
				T.AssertEquals(HOLDiplomacy::DefaultStance, HOLDiplomacy::StanceOf(Book, From, To), "the default stance");
				T.AssertEquals(HOLDiplomacy::DefaultStance, HOLDiplomacy::Relation(Book, From, To), "the default relation");
			}
		}
	}
	// The order of the stances: the lower the more hostile (the relation is the minimum).
	T.AssertTrue(HOLDiplomacy::Enemy < HOLDiplomacy::Neutral, "enemy is more hostile than neutral");
	T.AssertTrue(HOLDiplomacy::Neutral < HOLDiplomacy::Ally, "neutral is more hostile than ally");
}

// Allies only if BOTH say Ally (D369); the answer does not depend on who is asked.
void Test_DiplomacyAllyNeedsBoth(FUnitTest& T)
{
	FHOLDiplomacyBook Book = HumansBook(3);
	FHOLStanceChange First = HOLDiplomacy::SetStance(Book, 0, 1, HOLDiplomacy::Ally);
	T.AssertEquals(HOLDiplomacy::Ok, First.Result, "player 0 offers an alliance");
	T.AssertFalse(HOLDiplomacy::IsAlly(Book, 0, 1), "one side is not an alliance");
	T.AssertFalse(HOLDiplomacy::IsAlly(Book, 1, 0), "nor the other way round");
	T.AssertFalse(First.bFormed, "nothing formed yet");
	FHOLStanceChange Second = HOLDiplomacy::SetStance(Book, 1, 0, HOLDiplomacy::Ally);
	T.AssertTrue(Second.bFormed, "the second says Ally: formed");
	T.AssertTrue(HOLDiplomacy::IsAlly(Book, 0, 1), "allied");
	T.AssertTrue(HOLDiplomacy::IsAlly(Book, 1, 0), "allied the other way round");
	T.AssertFalse(HOLDiplomacy::IsAlly(Book, 0, 2), "a third player is not part of it");
	T.AssertFalse(HOLDiplomacy::IsAlly(Book, 2, 1), "nor the other way round");
}

// One relation per pair: the most hostile of the two stances applies to both (D375).
void Test_DiplomacyMostHostile(FUnitTest& T)
{
	for (int A = HOLDiplomacy::Enemy; A <= HOLDiplomacy::Ally; A++)
	{
		for (int B = HOLDiplomacy::Enemy; B <= HOLDiplomacy::Ally; B++)
		{
			FHOLDiplomacyBook Book = HumansBook(2);
			HOLDiplomacy::SetStance(Book, 0, 1, A);
			HOLDiplomacy::SetStance(Book, 1, 0, B);
			const int Expected = Math::Min(A, B);
			T.AssertEquals(Expected, HOLDiplomacy::Relation(Book, 0, 1), "the most hostile applies (0 to 1)");
			T.AssertEquals(Expected, HOLDiplomacy::Relation(Book, 1, 0), "and to both (1 to 0)");
		}
	}
	FHOLDiplomacyBook Book = HumansBook(2);
	HOLDiplomacy::SetStance(Book, 0, 1, HOLDiplomacy::Ally);
	HOLDiplomacy::SetStance(Book, 1, 0, HOLDiplomacy::Neutral);
	T.AssertEquals(HOLDiplomacy::Neutral, HOLDiplomacy::Relation(Book, 0, 1), "Ally against Neutral: Neutral, nothing shared");
	HOLDiplomacy::SetStance(Book, 1, 0, HOLDiplomacy::Enemy);
	T.AssertEquals(HOLDiplomacy::Enemy, HOLDiplomacy::Relation(Book, 0, 1), "Ally against Enemy: Enemy, the betrayed fire back");
}

// The break is immediate (D370): Enemy takes effect at the very call, without passing through Neutral, and says it broke.
void Test_DiplomacyBreakIsImmediate(FUnitTest& T)
{
	FHOLDiplomacyBook Book = HumansBook(3);
	HOLDiplomacy::SetStance(Book, 0, 1, HOLDiplomacy::Ally);
	HOLDiplomacy::SetStance(Book, 1, 0, HOLDiplomacy::Ally);
	T.AssertTrue(HOLDiplomacy::IsAlly(Book, 0, 1), "allies");
	T.AssertFalse(HOLDiplomacy::FiresAtOnce(Book, 0, 1), "allies do not fire");

	FHOLStanceChange Betrayal = HOLDiplomacy::SetStance(Book, 1, 0, HOLDiplomacy::Enemy);
	T.AssertEquals(HOLDiplomacy::Ok, Betrayal.Result, "the betrayal is allowed");
	T.AssertTrue(Betrayal.bBroke, "it broke an alliance");
	T.AssertFalse(Betrayal.bFormed, "it formed nothing");
	T.AssertEquals(HOLDiplomacy::Ally, Betrayal.Before, "the relation before");
	T.AssertEquals(HOLDiplomacy::Enemy, Betrayal.After, "straight to Enemy, not through Neutral");
	T.AssertFalse(HOLDiplomacy::IsAlly(Book, 0, 1), "no longer allies at once");
	T.AssertFalse(HOLDiplomacy::IsAlly(Book, 1, 0), "from either side");
	T.AssertTrue(HOLDiplomacy::FiresAtOnce(Book, 0, 1), "the betrayed one's units fire back at once");
	T.AssertTrue(HOLDiplomacy::FiresAtOnce(Book, 1, 0), "and the traitor's too");
	T.AssertEquals(HOLDiplomacy::Ally, HOLDiplomacy::StanceOf(Book, 0, 1), "the betrayed one's own stance is not touched");

	// Neutral also breaks an alliance (it is no longer one), but does not make the units fire.
	FHOLDiplomacyBook Calm = HumansBook(2);
	HOLDiplomacy::SetStance(Calm, 0, 1, HOLDiplomacy::Ally);
	HOLDiplomacy::SetStance(Calm, 1, 0, HOLDiplomacy::Ally);
	FHOLStanceChange Cooling = HOLDiplomacy::SetStance(Calm, 0, 1, HOLDiplomacy::Neutral);
	T.AssertTrue(Cooling.bBroke, "Neutral breaks the alliance too");
	T.AssertEquals(HOLDiplomacy::Neutral, Cooling.After, "to Neutral");
	T.AssertFalse(HOLDiplomacy::FiresAtOnce(Calm, 0, 1), "but nobody fires");

	// A break between 0 and 1 leaves 0 and 2, or 1 and 2, as they were.
	FHOLDiplomacyBook Trio = HumansBook(3);
	HOLDiplomacy::SetStance(Trio, 0, 2, HOLDiplomacy::Ally);
	HOLDiplomacy::SetStance(Trio, 2, 0, HOLDiplomacy::Ally);
	HOLDiplomacy::SetStance(Trio, 0, 1, HOLDiplomacy::Enemy);
	T.AssertTrue(HOLDiplomacy::IsAlly(Trio, 0, 2), "the other pair is untouched");
}

// What is announced to all: every real change of a stance, not a repeat (D369).
void Test_DiplomacyAnnouncement(FUnitTest& T)
{
	FHOLDiplomacyBook Book = HumansBook(3);
	FHOLStanceChange Offer = HOLDiplomacy::SetStance(Book, 2, 0, HOLDiplomacy::Neutral);
	T.AssertTrue(Offer.bAnnounce, "a new stance is announced");
	T.AssertEquals(2, Offer.From, "who");
	T.AssertEquals(0, Offer.To, "toward whom");
	T.AssertEquals(HOLDiplomacy::Neutral, Offer.Stance, "which stance");

	FHOLStanceChange Repeat = HOLDiplomacy::SetStance(Book, 2, 0, HOLDiplomacy::Neutral);
	T.AssertEquals(HOLDiplomacy::Ok, Repeat.Result, "a repeat is not an error");
	T.AssertFalse(Repeat.bAnnounce, "but it is not announced again");
	T.AssertFalse(Repeat.bBroke, "and breaks nothing");

	// A stance changed on one side only is announced even when the relation does not move (the other side stays the most hostile).
	FHOLStanceChange Hidden = HOLDiplomacy::SetStance(Book, 2, 0, HOLDiplomacy::Ally);
	T.AssertTrue(Hidden.bAnnounce, "Ally while the other is still Enemy is announced");
	T.AssertEquals(Hidden.Before, Hidden.After, "the relation did not move");
	T.AssertFalse(Hidden.bFormed, "no alliance");
	T.AssertFalse(Hidden.bBroke, "no break");
}

// Neutral is an adversary that does not fire on its own (D401, D369); an ally is no adversary; nobody is their own adversary.
void Test_DiplomacyNeutralIsAdversary(FUnitTest& T)
{
	FHOLDiplomacyBook Book = HumansBook(4);
	HOLDiplomacy::SetStance(Book, 0, 1, HOLDiplomacy::Neutral);
	HOLDiplomacy::SetStance(Book, 1, 0, HOLDiplomacy::Neutral);
	HOLDiplomacy::SetStance(Book, 0, 2, HOLDiplomacy::Ally);
	HOLDiplomacy::SetStance(Book, 2, 0, HOLDiplomacy::Ally);
	// 0 and 3: both Enemy by default.
	T.AssertFalse(HOLDiplomacy::FiresAtOnce(Book, 0, 1), "Neutral: no automatic fire");
	T.AssertTrue(HOLDiplomacy::IsAdversary(Book, 0, 1), "Neutral: still an adversary (ruin, relay, altar, duel)");
	T.AssertTrue(HOLDiplomacy::IsAdversary(Book, 1, 0), "Neutral: from the other side too");
	T.AssertFalse(HOLDiplomacy::FiresAtOnce(Book, 0, 2), "Ally: no automatic fire");
	T.AssertFalse(HOLDiplomacy::IsAdversary(Book, 0, 2), "Ally: not an adversary");
	T.AssertTrue(HOLDiplomacy::FiresAtOnce(Book, 0, 3), "Enemy: fire");
	T.AssertTrue(HOLDiplomacy::IsAdversary(Book, 0, 3), "Enemy: adversary");
	T.AssertFalse(HOLDiplomacy::FiresAtOnce(Book, 0, 0), "oneself: no fire");
	T.AssertFalse(HOLDiplomacy::IsAdversary(Book, 0, 0), "oneself: not an adversary");
	T.AssertFalse(HOLDiplomacy::FiresAtOnce(Book, 0, 9), "outside the match: no fire");
	T.AssertFalse(HOLDiplomacy::IsAdversary(Book, 0, 9), "outside the match: nobody");
}

// The AIs are alone against all (D367, D433): Enemy toward everybody, whatever stance is taken toward them, and they take none.
void Test_DiplomacyAIsAreEnemies(FUnitTest& T)
{
	TArray<bool> IsAI;
	IsAI.Add(false);
	IsAI.Add(false);
	IsAI.Add(true);
	IsAI.Add(true);
	FHOLDiplomacyBook Book = HOLDiplomacy::NewBook(4, IsAI);

	FHOLStanceChange Offer = HOLDiplomacy::SetStance(Book, 0, 2, HOLDiplomacy::Ally);
	T.AssertEquals(HOLDiplomacy::Ok, Offer.Result, "a human may say Ally to an AI (it is just ignored)");
	T.AssertEquals(HOLDiplomacy::Enemy, HOLDiplomacy::Relation(Book, 0, 2), "the AI stays Enemy");
	T.AssertEquals(HOLDiplomacy::Enemy, HOLDiplomacy::Relation(Book, 2, 0), "from its side too");
	T.AssertFalse(HOLDiplomacy::IsAlly(Book, 0, 2), "never allied with an AI");
	T.AssertTrue(HOLDiplomacy::FiresAtOnce(Book, 2, 0), "the AI fires on the human");

	FHOLStanceChange Own = HOLDiplomacy::SetStance(Book, 2, 0, HOLDiplomacy::Ally);
	T.AssertEquals(HOLDiplomacy::SenderIsAI, Own.Result, "an AI takes no stance");
	FHOLDiplomacyBook Fresh = HOLDiplomacy::NewBook(4, IsAI);
	T.AssertEquals(HOLDiplomacy::Enemy, HOLDiplomacy::Relation(Fresh, 2, 3), "two AIs: enemies (alone against all)");

	// Both humans allied: the AIs do not change that.
	HOLDiplomacy::SetStance(Book, 0, 1, HOLDiplomacy::Ally);
	HOLDiplomacy::SetStance(Book, 1, 0, HOLDiplomacy::Ally);
	T.AssertTrue(HOLDiplomacy::IsAlly(Book, 0, 1), "the humans can ally against the AIs");
}

// The refusals, in the order they are told, and a refusal changes nothing.
void Test_DiplomacyRefusals(FUnitTest& T)
{
	FHOLDiplomacyBook Book = HumansBook(3);
	T.AssertEquals(HOLDiplomacy::NoSuchSlot, HOLDiplomacy::SetStance(Book, -1, 1, HOLDiplomacy::Ally).Result, "negative sender");
	T.AssertEquals(HOLDiplomacy::NoSuchSlot, HOLDiplomacy::SetStance(Book, 3, 1, HOLDiplomacy::Ally).Result, "sender beyond the slots");
	T.AssertEquals(HOLDiplomacy::NoSuchSlot, HOLDiplomacy::SetStance(Book, 0, 3, HOLDiplomacy::Ally).Result, "target beyond the slots");
	T.AssertEquals(HOLDiplomacy::NoSuchSlot, HOLDiplomacy::SetStance(Book, 0, -1, HOLDiplomacy::Ally).Result, "negative target");
	T.AssertEquals(HOLDiplomacy::Self, HOLDiplomacy::SetStance(Book, 1, 1, HOLDiplomacy::Ally).Result, "toward oneself");
	T.AssertEquals(HOLDiplomacy::BadStance, HOLDiplomacy::SetStance(Book, 0, 1, 3).Result, "no such stance");
	T.AssertEquals(HOLDiplomacy::BadStance, HOLDiplomacy::SetStance(Book, 0, 1, -1).Result, "no such stance (negative)");
	// The order: the slots, then oneself, then the stance.
	T.AssertEquals(HOLDiplomacy::NoSuchSlot, HOLDiplomacy::SetStance(Book, 7, 7, 9).Result, "the slots come first");
	T.AssertEquals(HOLDiplomacy::Self, HOLDiplomacy::SetStance(Book, 1, 1, 9).Result, "then oneself");
	// A refusal changes nothing, and announces nothing.
	FHOLStanceChange Refused = HOLDiplomacy::SetStance(Book, 0, 1, 3);
	T.AssertFalse(Refused.bAnnounce, "a refusal is not announced");
	T.AssertEquals(HOLDiplomacy::DefaultStance, HOLDiplomacy::StanceOf(Book, 0, 1), "the stance did not move");
}

// Outside the Diplomacy mode the teams are fixed: the stances are locked (D387), same team = allies, the rest enemies, team 0 = alone,
// and an AI on a team is a teammate (D367 is the Diplomacy mode's rule only).
void Test_DiplomacyLockedTeams(FUnitTest& T)
{
	TArray<int> Teams;
	Teams.Add(1);
	Teams.Add(1);
	Teams.Add(2);
	Teams.Add(0);
	Teams.Add(0);
	TArray<bool> IsAI;
	IsAI.Add(false);
	IsAI.Add(true);
	IsAI.Add(false);
	IsAI.Add(false);
	IsAI.Add(true);
	FHOLDiplomacyBook Book = HOLDiplomacy::NewTeamBook(Teams, IsAI);
	T.AssertTrue(Book.bLocked, "the stances are locked");
	T.AssertTrue(HOLDiplomacy::IsAlly(Book, 0, 1), "same team: allies (even with an AI)");
	T.AssertTrue(HOLDiplomacy::IsAlly(Book, 1, 0), "both ways");
	T.AssertTrue(HOLDiplomacy::IsAdversary(Book, 0, 2), "another team: adversary");
	T.AssertTrue(HOLDiplomacy::FiresAtOnce(Book, 2, 1), "and enemy");
	T.AssertTrue(HOLDiplomacy::FiresAtOnce(Book, 3, 4), "team 0 is alone: enemy of the other team 0");
	T.AssertFalse(HOLDiplomacy::IsAlly(Book, 3, 4), "not allied");
	T.AssertEquals(HOLDiplomacy::Locked, HOLDiplomacy::SetStance(Book, 0, 2, HOLDiplomacy::Ally).Result, "no stance can be set");
	T.AssertEquals(HOLDiplomacy::Enemy, HOLDiplomacy::Relation(Book, 0, 2), "and nothing moved");
}

// Two players allied both ways, in a book of Slots humans.
void MakeAllies(FHOLDiplomacyBook& Book, int A, int B)
{
	HOLDiplomacy::SetStance(Book, A, B, HOLDiplomacy::Ally);
	HOLDiplomacy::SetStance(Book, B, A, HOLDiplomacy::Ally);
}

// The vision is a box of its own (D371): ticked toward one ally, one-sided, announced, only between allies; allies are not given it by
// being allies. The break cuts it at that second, and an alliance formed again does not give it back unasked.
void Test_DiplomacyVisionIsShared(FUnitTest& T)
{
	FHOLDiplomacyBook Book = HumansBook(3);
	MakeAllies(Book, 0, 1);
	T.AssertFalse(HOLDiplomacy::SharesVision(Book, 0, 1), "allies, but nobody ticked: nothing shared");
	T.AssertTrue(HOLDiplomacy::SharesVision(Book, 0, 0), "one sees oneself");

	FHOLVisionChange Tick = HOLDiplomacy::SetVision(Book, 0, 1, true);
	T.AssertEquals(HOLDiplomacy::Ok, Tick.Result, "player 0 opens the vision to 1");
	T.AssertTrue(Tick.bAnnounce, "announced to all");
	T.AssertEquals(0, Tick.From, "who");
	T.AssertEquals(1, Tick.To, "toward whom");
	T.AssertTrue(Tick.bShare, "opened");
	T.AssertTrue(HOLDiplomacy::SharesVision(Book, 0, 1), "1 sees what 0 sees");
	T.AssertFalse(HOLDiplomacy::SharesVision(Book, 1, 0), "one-sided: 0 does not see what 1 sees");
	T.AssertFalse(HOLDiplomacy::SharesVision(Book, 0, 2), "nor a third player");

	FHOLVisionChange Again = HOLDiplomacy::SetVision(Book, 0, 1, true);
	T.AssertEquals(HOLDiplomacy::Ok, Again.Result, "a repeat is not an error");
	T.AssertFalse(Again.bAnnounce, "and not announced again");

	// The break cuts it at once, from the very call of the stance (D370, D371).
	HOLDiplomacy::SetStance(Book, 1, 0, HOLDiplomacy::Enemy);
	T.AssertFalse(HOLDiplomacy::SharesVision(Book, 0, 1), "cut at that second");
	MakeAllies(Book, 0, 1);
	T.AssertTrue(HOLDiplomacy::IsAlly(Book, 0, 1), "allied again");
	T.AssertFalse(HOLDiplomacy::SharesVision(Book, 0, 1), "but the vision must be opened again");

	FHOLVisionChange Closing = HOLDiplomacy::SetVision(Book, 0, 1, true);
	T.AssertTrue(Closing.bAnnounce, "opened again: announced");
	FHOLVisionChange Close = HOLDiplomacy::SetVision(Book, 0, 1, false);
	T.AssertTrue(Close.bAnnounce, "closing is announced too");
	T.AssertFalse(Close.bShare, "closed");
	T.AssertFalse(HOLDiplomacy::SharesVision(Book, 0, 1), "no more vision");
}

// The refusals of the vision, in order; and in the standard modes the vision of teammates is automatic.
void Test_DiplomacyVisionRefusals(FUnitTest& T)
{
	FHOLDiplomacyBook Book = HumansBook(3);
	MakeAllies(Book, 0, 1);
	HOLDiplomacy::SetStance(Book, 0, 2, HOLDiplomacy::Ally);
	T.AssertEquals(HOLDiplomacy::NoSuchSlot, HOLDiplomacy::SetVision(Book, 0, 5, true).Result, "nobody there");
	T.AssertEquals(HOLDiplomacy::NoSuchSlot, HOLDiplomacy::SetVision(Book, -1, 1, true).Result, "nobody there (sender)");
	T.AssertEquals(HOLDiplomacy::Self, HOLDiplomacy::SetVision(Book, 1, 1, true).Result, "toward oneself");
	T.AssertEquals(HOLDiplomacy::NotAlly, HOLDiplomacy::SetVision(Book, 0, 2, true).Result, "an offer not accepted is not an alliance");
	T.AssertFalse(HOLDiplomacy::SharesVision(Book, 0, 2), "nothing shared");
	T.AssertEquals(HOLDiplomacy::NotAlly, HOLDiplomacy::SetVision(Book, 2, 1, true).Result, "not allied at all");

	TArray<bool> IsAI;
	IsAI.Add(false);
	IsAI.Add(true);
	FHOLDiplomacyBook WithAI = HOLDiplomacy::NewBook(2, IsAI);
	T.AssertEquals(HOLDiplomacy::SenderIsAI, HOLDiplomacy::SetVision(WithAI, 1, 0, true).Result, "an AI shares nothing");

	TArray<int> Teams;
	Teams.Add(1);
	Teams.Add(1);
	Teams.Add(2);
	TArray<bool> NoAI;
	NoAI.Add(false);
	NoAI.Add(false);
	NoAI.Add(false);
	FHOLDiplomacyBook Locked = HOLDiplomacy::NewTeamBook(Teams, NoAI);
	T.AssertTrue(HOLDiplomacy::SharesVision(Locked, 0, 1), "teammates see together, automatically");
	T.AssertTrue(HOLDiplomacy::SharesVision(Locked, 1, 0), "both ways");
	T.AssertFalse(HOLDiplomacy::SharesVision(Locked, 0, 2), "not the other team");
	T.AssertEquals(HOLDiplomacy::Locked, HOLDiplomacy::SetVision(Locked, 0, 1, false).Result, "no box to tick: locked");
}

// Garrison and ramparts (D374): never a garrison in an ally's building; the doors and ramparts of an ally are open, those of a former
// ally are shut at the very second of the break, and the units left on its ramparts cannot get down.
void Test_DiplomacyGarrisonAndRamparts(FUnitTest& T)
{
	FHOLDiplomacyBook Book = HumansBook(3);
	MakeAllies(Book, 0, 1);
	T.AssertTrue(HOLDiplomacy::CanGarrison(0, 0), "in one's own building");
	T.AssertFalse(HOLDiplomacy::CanGarrison(0, 1), "never in an ally's");
	T.AssertFalse(HOLDiplomacy::CanGarrison(0, 2), "nor in another's");

	T.AssertTrue(HOLDiplomacy::DoorsOpenFor(Book, 1, 1), "one's own doors");
	T.AssertTrue(HOLDiplomacy::DoorsOpenFor(Book, 1, 0), "an ally's doors and ramparts are open");
	T.AssertTrue(HOLDiplomacy::DoorsOpenFor(Book, 0, 1), "both ways");
	T.AssertFalse(HOLDiplomacy::DoorsOpenFor(Book, 1, 2), "a stranger's are not");

	// Player 0's units climb on 1's ramparts; 1 betrays: they cannot get down, and 1's units cannot cross 0's gates either.
	T.AssertTrue(HOLDiplomacy::CanLeaveRampart(Book, 0, 1), "allied: the unit gets down when it wants");
	HOLDiplomacy::SetStance(Book, 1, 0, HOLDiplomacy::Enemy);
	T.AssertFalse(HOLDiplomacy::CanLeaveRampart(Book, 0, 1), "after the break: it stays there");
	T.AssertFalse(HOLDiplomacy::DoorsOpenFor(Book, 0, 1), "the gates are shut to the other");
	T.AssertTrue(HOLDiplomacy::CanLeaveRampart(Book, 1, 1), "the owner's own units come and go");
	// Neutral is not allied either: the doors are shut.
	FHOLDiplomacyBook Calm = HumansBook(2);
	HOLDiplomacy::SetStance(Calm, 0, 1, HOLDiplomacy::Ally);
	HOLDiplomacy::SetStance(Calm, 1, 0, HOLDiplomacy::Neutral);
	T.AssertFalse(HOLDiplomacy::DoorsOpenFor(Calm, 0, 1), "Ally against Neutral: Neutral, shut");
}

// The allied victory (D373): all the survivors allied to each other AND at least one player beaten; at once, no box, no countdown.
void Test_DiplomacyAlliedVictory(FUnitTest& T)
{
	TArray<bool> Defeated;
	Defeated.Add(false);
	Defeated.Add(false);
	Defeated.Add(false);
	Defeated.Add(true);
	FHOLDiplomacyBook Book = HumansBook(4);
	// Nobody allied: three enemy survivors, the game goes on.
	T.AssertFalse(HOLDiplomacy::AlliedVictory(Book, Defeated).bEnded, "three enemy survivors: no victory");

	MakeAllies(Book, 0, 1);
	T.AssertFalse(HOLDiplomacy::AlliedVictory(Book, Defeated).bEnded, "two allied, one apart: no victory");
	MakeAllies(Book, 1, 2);
	T.AssertFalse(HOLDiplomacy::AlliedVictory(Book, Defeated).bEnded, "0-1 and 1-2 allied but not 0-2: no chain");
	MakeAllies(Book, 0, 2);
	FHOLAlliedVictory Win = HOLDiplomacy::AlliedVictory(Book, Defeated);
	T.AssertTrue(Win.bEnded, "all three survivors allied to each other, one beaten: victory at once");
	T.AssertEquals(3, Win.Winners.Num(), "the three win together");
	T.AssertEquals(0, Win.Winners[0], "in order");
	T.AssertEquals(2, Win.Winners[2], "in order");

	// The traitor who breaks just before the last common enemy falls wins alone: the others are no longer all allied.
	HOLDiplomacy::SetStance(Book, 2, 0, HOLDiplomacy::Enemy);
	T.AssertFalse(HOLDiplomacy::AlliedVictory(Book, Defeated).bEnded, "a break: not all allied any more");

	// Nobody beaten: a general alliance does not end the game (D373).
	TArray<bool> NoOneDown;
	NoOneDown.Add(false);
	NoOneDown.Add(false);
	FHOLDiplomacyBook Pair = HumansBook(2);
	MakeAllies(Pair, 0, 1);
	T.AssertFalse(HOLDiplomacy::AlliedVictory(Pair, NoOneDown).bEnded, "everybody allied but nobody beaten: it goes on");
	NoOneDown[1] = true;
	FHOLAlliedVictory Last = HOLDiplomacy::AlliedVictory(Pair, NoOneDown);
	T.AssertTrue(Last.bEnded, "one beaten, one survivor: it ends");
	T.AssertEquals(1, Last.Winners.Num(), "the last standing");
	T.AssertEquals(0, Last.Winners[0], "player 0");

	// Everybody beaten: no survivor, no allied victory (the standard rule settles it).
	TArray<bool> AllDown;
	AllDown.Add(true);
	AllDown.Add(true);
	T.AssertFalse(HOLDiplomacy::AlliedVictory(Pair, AllDown).bEnded, "no survivor: not an allied victory");
	// A list shorter than the slots: the missing ones are alive.
	TArray<bool> Short;
	Short.Add(true);
	FHOLAlliedVictory FromShort = HOLDiplomacy::AlliedVictory(Pair, Short);
	T.AssertTrue(FromShort.bEnded, "player 0 beaten, 1 alive by default");
	T.AssertEquals(1, FromShort.Winners[0], "player 1 wins");
}

// An AI still alive blocks the allied victory (D367): it is Enemy to all, so the humans must eliminate it; the teams of the standard
// modes give the same result as the team victory (D357).
void Test_DiplomacyAlliedVictoryAIsAndTeams(FUnitTest& T)
{
	TArray<bool> IsAI;
	IsAI.Add(false);
	IsAI.Add(false);
	IsAI.Add(true);
	IsAI.Add(false);
	FHOLDiplomacyBook Book = HOLDiplomacy::NewBook(4, IsAI);
	MakeAllies(Book, 0, 1);
	TArray<bool> Defeated;
	Defeated.Add(false);
	Defeated.Add(false);
	Defeated.Add(false);
	Defeated.Add(true);
	T.AssertFalse(HOLDiplomacy::AlliedVictory(Book, Defeated).bEnded, "two allied humans, an AI alive and one beaten: no victory");
	Defeated[2] = true;
	FHOLAlliedVictory Win = HOLDiplomacy::AlliedVictory(Book, Defeated);
	T.AssertTrue(Win.bEnded, "the AI eliminated: the allied humans win together");
	T.AssertEquals(2, Win.Winners.Num(), "both");

	// Teams 1 / 1 / 2 / 2: team 2 beaten down to nothing, team 1 wins; with one of team 2 alive, it goes on.
	TArray<int> Teams;
	Teams.Add(1);
	Teams.Add(1);
	Teams.Add(2);
	Teams.Add(2);
	TArray<bool> NoAI;
	for (int Slot = 0; Slot < 4; Slot++)
		NoAI.Add(false);
	FHOLDiplomacyBook TeamBook = HOLDiplomacy::NewTeamBook(Teams, NoAI);
	TArray<bool> Down;
	Down.Add(false);
	Down.Add(true);
	Down.Add(true);
	Down.Add(false);
	T.AssertFalse(HOLDiplomacy::AlliedVictory(TeamBook, Down).bEnded, "one of each team alive: it goes on");
	Down[3] = true;
	FHOLAlliedVictory TeamWin = HOLDiplomacy::AlliedVictory(TeamBook, Down);
	T.AssertTrue(TeamWin.bEnded, "only team 1 is left");
	T.AssertEquals(1, TeamWin.Winners.Num(), "its survivor");
}
