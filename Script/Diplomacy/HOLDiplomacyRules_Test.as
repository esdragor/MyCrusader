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
