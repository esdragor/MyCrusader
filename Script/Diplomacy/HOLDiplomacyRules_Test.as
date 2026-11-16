// The stances of the Diplomacy mode (D369, D370, D375, D401, D367; GDD 14.3): Ally / Neutral / Enemy of each player toward each other,
// the relation of a pair (the most hostile of the two), the immediate break, and what is announced to all (HOLDiplomacyRules.as). Pure.
// Redirected to HOLDiplomacyRead (CELEBORN-1116t, D836 (5)): the tests below run on the library (FHOLDiplomacyStances, the change fiches). Only the garrison, doors and allied victory
// tests stay on HOLDiplomacyRules.as (CanGarrison, DoorsOpenFor, AlliedVictory are not exposed by HOLDiplomacyRead yet).

// A book of Slots humans, in the Diplomacy mode (unlocked), every stance at the default.
FHOLDiplomacyBook HumansBook(int Slots)
{
	TArray<bool> NoAI;
	for (int Slot = 0; Slot < Slots; Slot++)
		NoAI.Add(false);
	return HOLDiplomacy::NewBook(Slots, NoAI);
}

// Two players allied both ways, in a book of Slots humans.
void MakeAllies(FHOLDiplomacyBook& Book, int A, int B)
{
	HOLDiplomacy::SetStance(Book, A, B, HOLDiplomacy::Ally);
	HOLDiplomacy::SetStance(Book, B, A, HOLDiplomacy::Ally);
}

// MIG-J-Diplomacy (D830/D835, 2026-11-16): the vectors of the C++ twin's test HeroesOfLiberty.Diplomacy.Rules
// (Source/HeroesOfLiberty/Tests/HOLDiplomacyRulesTests.cpp), in its order, so that both copies answer the same cases while two exist.
void DiploVecAllies(FHOLDiplomacyBook& Book, int A, int B)
{
	HOLDiplomacy::SetStance(Book, A, B, HOLDiplomacy::Ally);
	HOLDiplomacy::SetStance(Book, B, A, HOLDiplomacy::Ally);
}

// Count flags, flag i set when bit i of Bits is (the C++ test's brace lists, { false, true } = DiploVecFlags(2, 2)).
TArray<bool> DiploVecFlags(int Count, int Bits)
{
	TArray<bool> Flags;
	for (int Index = 0; Index < Count; Index++)
		Flags.Add(((Bits >> Index) & 1) == 1);
	return Flags;
}

FString DiploVecList(const TArray<int>& Values)
{
	FString Text = "";
	for (int Index = 0; Index < Values.Num(); Index++)
	{
		if (Index > 0)
			Text += ",";
		Text += f"{Values[Index]}";
	}
	return Text;
}

// A book of Slots humans, in the Diplomacy mode (unlocked), every stance at the default.
FHOLDiplomacyStances LibHumansBook(int Slots)
{
	TArray<bool> NoAI;
	for (int Slot = 0; Slot < Slots; Slot++)
		NoAI.Add(false);
	return HOLDiplomacyRead::NewBook(Slots, NoAI);
}

// Two players allied both ways, in a book of Slots humans.
void LibMakeAllies(FHOLDiplomacyStances& Book, int A, int B)
{
	HOLDiplomacyRead::SetStance(Book, A, B, HOLDiplomacyRead::GetStanceAlly());
	HOLDiplomacyRead::SetStance(Book, B, A, HOLDiplomacyRead::GetStanceAlly());
}

void LibDiploVecAllies(FHOLDiplomacyStances& Book, int A, int B)
{
	HOLDiplomacyRead::SetStance(Book, A, B, HOLDiplomacyRead::GetStanceAlly());
	HOLDiplomacyRead::SetStance(Book, B, A, HOLDiplomacyRead::GetStanceAlly());
}

// The default stance is the same for every pair, and one is always allied with oneself.
void Test_DiplomacyDefaultStance(FUnitTest& T)
{
	FHOLDiplomacyStances Book = LibHumansBook(4);
	T.AssertEquals(4, Book.Slots, "four slots");
	for (int From = 0; From < 4; From++)
	{
		for (int To = 0; To < 4; To++)
		{
			if (From == To)
				T.AssertEquals(HOLDiplomacyRead::GetStanceAlly(), HOLDiplomacyRead::Relation(Book, From, To), "alone with oneself: ally");
			else
			{
				T.AssertEquals(HOLDiplomacyRead::GetStanceDefault(), HOLDiplomacyRead::StanceOf(Book, From, To), "the default stance");
				T.AssertEquals(HOLDiplomacyRead::GetStanceDefault(), HOLDiplomacyRead::Relation(Book, From, To), "the default relation");
			}
		}
	}
	// The order of the stances: the lower the more hostile (the relation is the minimum).
	T.AssertTrue(HOLDiplomacyRead::GetStanceEnemy() < HOLDiplomacyRead::GetStanceNeutral(), "enemy is more hostile than neutral");
	T.AssertTrue(HOLDiplomacyRead::GetStanceNeutral() < HOLDiplomacyRead::GetStanceAlly(), "neutral is more hostile than ally");
}

// Allies only if BOTH say Ally (D369); the answer does not depend on who is asked.
void Test_DiplomacyAllyNeedsBoth(FUnitTest& T)
{
	FHOLDiplomacyStances Book = LibHumansBook(3);
	FHOLDiplomacyStanceChange First = HOLDiplomacyRead::ChangeStance(Book, 0, 1, HOLDiplomacyRead::GetStanceAlly());
	T.AssertEquals(HOLDiplomacyRead::GetResultOk(), First.Result, "player 0 offers an alliance");
	T.AssertFalse(HOLDiplomacyRead::IsAlly(Book, 0, 1), "one side is not an alliance");
	T.AssertFalse(HOLDiplomacyRead::IsAlly(Book, 1, 0), "nor the other way round");
	T.AssertFalse(First.bFormed, "nothing formed yet");
	FHOLDiplomacyStanceChange Second = HOLDiplomacyRead::ChangeStance(Book, 1, 0, HOLDiplomacyRead::GetStanceAlly());
	T.AssertTrue(Second.bFormed, "the second says Ally: formed");
	T.AssertTrue(HOLDiplomacyRead::IsAlly(Book, 0, 1), "allied");
	T.AssertTrue(HOLDiplomacyRead::IsAlly(Book, 1, 0), "allied the other way round");
	T.AssertFalse(HOLDiplomacyRead::IsAlly(Book, 0, 2), "a third player is not part of it");
	T.AssertFalse(HOLDiplomacyRead::IsAlly(Book, 2, 1), "nor the other way round");
}

// One relation per pair: the most hostile of the two stances applies to both (D375).
void Test_DiplomacyMostHostile(FUnitTest& T)
{
	for (int A = HOLDiplomacyRead::GetStanceEnemy(); A <= HOLDiplomacyRead::GetStanceAlly(); A++)
	{
		for (int B = HOLDiplomacyRead::GetStanceEnemy(); B <= HOLDiplomacyRead::GetStanceAlly(); B++)
		{
			FHOLDiplomacyStances Book = LibHumansBook(2);
			HOLDiplomacyRead::SetStance(Book, 0, 1, A);
			HOLDiplomacyRead::SetStance(Book, 1, 0, B);
			const int Expected = Math::Min(A, B);
			T.AssertEquals(Expected, HOLDiplomacyRead::Relation(Book, 0, 1), "the most hostile applies (0 to 1)");
			T.AssertEquals(Expected, HOLDiplomacyRead::Relation(Book, 1, 0), "and to both (1 to 0)");
		}
	}
	FHOLDiplomacyStances Book = LibHumansBook(2);
	HOLDiplomacyRead::SetStance(Book, 0, 1, HOLDiplomacyRead::GetStanceAlly());
	HOLDiplomacyRead::SetStance(Book, 1, 0, HOLDiplomacyRead::GetStanceNeutral());
	T.AssertEquals(HOLDiplomacyRead::GetStanceNeutral(), HOLDiplomacyRead::Relation(Book, 0, 1), "Ally against Neutral: Neutral, nothing shared");
	HOLDiplomacyRead::SetStance(Book, 1, 0, HOLDiplomacyRead::GetStanceEnemy());
	T.AssertEquals(HOLDiplomacyRead::GetStanceEnemy(), HOLDiplomacyRead::Relation(Book, 0, 1), "Ally against Enemy: Enemy, the betrayed fire back");
}

// The break is immediate (D370): Enemy takes effect at the very call, without passing through Neutral, and says it broke.
void Test_DiplomacyBreakIsImmediate(FUnitTest& T)
{
	FHOLDiplomacyStances Book = LibHumansBook(3);
	HOLDiplomacyRead::SetStance(Book, 0, 1, HOLDiplomacyRead::GetStanceAlly());
	HOLDiplomacyRead::SetStance(Book, 1, 0, HOLDiplomacyRead::GetStanceAlly());
	T.AssertTrue(HOLDiplomacyRead::IsAlly(Book, 0, 1), "allies");
	T.AssertFalse(HOLDiplomacyRead::FiresAtOnce(Book, 0, 1), "allies do not fire");

	FHOLDiplomacyStanceChange Betrayal = HOLDiplomacyRead::ChangeStance(Book, 1, 0, HOLDiplomacyRead::GetStanceEnemy());
	T.AssertEquals(HOLDiplomacyRead::GetResultOk(), Betrayal.Result, "the betrayal is allowed");
	T.AssertTrue(Betrayal.bBroke, "it broke an alliance");
	T.AssertFalse(Betrayal.bFormed, "it formed nothing");
	T.AssertEquals(HOLDiplomacyRead::GetStanceAlly(), Betrayal.Before, "the relation before");
	T.AssertEquals(HOLDiplomacyRead::GetStanceEnemy(), Betrayal.After, "straight to Enemy, not through Neutral");
	T.AssertFalse(HOLDiplomacyRead::IsAlly(Book, 0, 1), "no longer allies at once");
	T.AssertFalse(HOLDiplomacyRead::IsAlly(Book, 1, 0), "from either side");
	T.AssertTrue(HOLDiplomacyRead::FiresAtOnce(Book, 0, 1), "the betrayed one's units fire back at once");
	T.AssertTrue(HOLDiplomacyRead::FiresAtOnce(Book, 1, 0), "and the traitor's too");
	T.AssertEquals(HOLDiplomacyRead::GetStanceEnemy(), HOLDiplomacyRead::StanceOf(Book, 0, 1), "the betrayed one's stance drops to Enemy too: nobody keeps showing Ally (2026-11-04)");

	// Neutral also breaks an alliance (it is no longer one), but does not make the units fire.
	FHOLDiplomacyStances Calm = LibHumansBook(2);
	HOLDiplomacyRead::SetStance(Calm, 0, 1, HOLDiplomacyRead::GetStanceAlly());
	HOLDiplomacyRead::SetStance(Calm, 1, 0, HOLDiplomacyRead::GetStanceAlly());
	FHOLDiplomacyStanceChange Cooling = HOLDiplomacyRead::ChangeStance(Calm, 0, 1, HOLDiplomacyRead::GetStanceNeutral());
	T.AssertTrue(Cooling.bBroke, "Neutral breaks the alliance too");
	T.AssertEquals(HOLDiplomacyRead::GetStanceNeutral(), Cooling.After, "to Neutral");
	T.AssertFalse(HOLDiplomacyRead::FiresAtOnce(Calm, 0, 1), "but nobody fires");

	// A break between 0 and 1 leaves 0 and 2, or 1 and 2, as they were.
	FHOLDiplomacyStances Trio = LibHumansBook(3);
	HOLDiplomacyRead::SetStance(Trio, 0, 2, HOLDiplomacyRead::GetStanceAlly());
	HOLDiplomacyRead::SetStance(Trio, 2, 0, HOLDiplomacyRead::GetStanceAlly());
	HOLDiplomacyRead::SetStance(Trio, 0, 1, HOLDiplomacyRead::GetStanceEnemy());
	T.AssertTrue(HOLDiplomacyRead::IsAlly(Trio, 0, 2), "the other pair is untouched");
}

// What is announced to all: every real change of a stance, not a repeat (D369).
void Test_DiplomacyAnnouncement(FUnitTest& T)
{
	FHOLDiplomacyStances Book = LibHumansBook(3);
	FHOLDiplomacyStanceChange Offer = HOLDiplomacyRead::ChangeStance(Book, 2, 0, HOLDiplomacyRead::GetStanceNeutral());
	T.AssertTrue(Offer.bAnnounce, "a new stance is announced");
	T.AssertEquals(2, Offer.From, "who");
	T.AssertEquals(0, Offer.To, "toward whom");
	T.AssertEquals(HOLDiplomacyRead::GetStanceNeutral(), Offer.Stance, "which stance");

	FHOLDiplomacyStanceChange Repeat = HOLDiplomacyRead::ChangeStance(Book, 2, 0, HOLDiplomacyRead::GetStanceNeutral());
	T.AssertEquals(HOLDiplomacyRead::GetResultOk(), Repeat.Result, "a repeat is not an error");
	T.AssertFalse(Repeat.bAnnounce, "but it is not announced again");
	T.AssertFalse(Repeat.bBroke, "and breaks nothing");

	// A stance changed on one side only is announced even when the relation does not move (the other side stays the most hostile).
	FHOLDiplomacyStanceChange Hidden = HOLDiplomacyRead::ChangeStance(Book, 2, 0, HOLDiplomacyRead::GetStanceAlly());
	T.AssertTrue(Hidden.bAnnounce, "Ally while the other is still Enemy is announced");
	T.AssertEquals(Hidden.Before, Hidden.After, "the relation did not move");
	T.AssertFalse(Hidden.bFormed, "no alliance");
	T.AssertFalse(Hidden.bBroke, "no break");
}

// Neutral is an adversary that does not fire on its own (D401, D369); an ally is no adversary; nobody is their own adversary.
void Test_DiplomacyNeutralIsAdversary(FUnitTest& T)
{
	FHOLDiplomacyStances Book = LibHumansBook(4);
	HOLDiplomacyRead::SetStance(Book, 0, 1, HOLDiplomacyRead::GetStanceNeutral());
	HOLDiplomacyRead::SetStance(Book, 1, 0, HOLDiplomacyRead::GetStanceNeutral());
	HOLDiplomacyRead::SetStance(Book, 0, 2, HOLDiplomacyRead::GetStanceAlly());
	HOLDiplomacyRead::SetStance(Book, 2, 0, HOLDiplomacyRead::GetStanceAlly());
	// 0 and 3: both Enemy by default.
	T.AssertFalse(HOLDiplomacyRead::FiresAtOnce(Book, 0, 1), "Neutral: no automatic fire");
	T.AssertTrue(HOLDiplomacyRead::IsAdversary(Book, 0, 1), "Neutral: still an adversary (ruin, relay, altar, duel)");
	T.AssertTrue(HOLDiplomacyRead::IsAdversary(Book, 1, 0), "Neutral: from the other side too");
	T.AssertFalse(HOLDiplomacyRead::FiresAtOnce(Book, 0, 2), "Ally: no automatic fire");
	T.AssertFalse(HOLDiplomacyRead::IsAdversary(Book, 0, 2), "Ally: not an adversary");
	T.AssertTrue(HOLDiplomacyRead::FiresAtOnce(Book, 0, 3), "Enemy: fire");
	T.AssertTrue(HOLDiplomacyRead::IsAdversary(Book, 0, 3), "Enemy: adversary");
	T.AssertFalse(HOLDiplomacyRead::FiresAtOnce(Book, 0, 0), "oneself: no fire");
	T.AssertFalse(HOLDiplomacyRead::IsAdversary(Book, 0, 0), "oneself: not an adversary");
	T.AssertFalse(HOLDiplomacyRead::FiresAtOnce(Book, 0, 9), "outside the match: no fire");
	T.AssertFalse(HOLDiplomacyRead::IsAdversary(Book, 0, 9), "outside the match: nobody");
}

// The AIs are alone against all (D367, D433): Enemy toward everybody, whatever stance is taken toward them, and they take none.
void Test_DiplomacyAIsAreEnemies(FUnitTest& T)
{
	TArray<bool> IsAI;
	IsAI.Add(false);
	IsAI.Add(false);
	IsAI.Add(true);
	IsAI.Add(true);
	FHOLDiplomacyStances Book = HOLDiplomacyRead::NewBook(4, IsAI);

	FHOLDiplomacyStanceChange Offer = HOLDiplomacyRead::ChangeStance(Book, 0, 2, HOLDiplomacyRead::GetStanceAlly());
	T.AssertEquals(HOLDiplomacyRead::GetResultOk(), Offer.Result, "a human may say Ally to an AI (it is just ignored)");
	T.AssertEquals(HOLDiplomacyRead::GetStanceEnemy(), HOLDiplomacyRead::Relation(Book, 0, 2), "the AI stays Enemy");
	T.AssertEquals(HOLDiplomacyRead::GetStanceEnemy(), HOLDiplomacyRead::Relation(Book, 2, 0), "from its side too");
	T.AssertFalse(HOLDiplomacyRead::IsAlly(Book, 0, 2), "never allied with an AI");
	T.AssertTrue(HOLDiplomacyRead::FiresAtOnce(Book, 2, 0), "the AI fires on the human");

	FHOLDiplomacyStanceChange Own = HOLDiplomacyRead::ChangeStance(Book, 2, 0, HOLDiplomacyRead::GetStanceAlly());
	T.AssertEquals(HOLDiplomacyRead::GetResultSenderIsAI(), Own.Result, "an AI takes no stance");
	FHOLDiplomacyStances Fresh = HOLDiplomacyRead::NewBook(4, IsAI);
	T.AssertEquals(HOLDiplomacyRead::GetStanceEnemy(), HOLDiplomacyRead::Relation(Fresh, 2, 3), "two AIs: enemies (alone against all)");

	// Both humans allied: the AIs do not change that.
	HOLDiplomacyRead::SetStance(Book, 0, 1, HOLDiplomacyRead::GetStanceAlly());
	HOLDiplomacyRead::SetStance(Book, 1, 0, HOLDiplomacyRead::GetStanceAlly());
	T.AssertTrue(HOLDiplomacyRead::IsAlly(Book, 0, 1), "the humans can ally against the AIs");
}

// The refusals, in the order they are told, and a refusal changes nothing.
void Test_DiplomacyRefusals(FUnitTest& T)
{
	FHOLDiplomacyStances Book = LibHumansBook(3);
	T.AssertEquals(HOLDiplomacyRead::GetResultNoSuchSlot(), HOLDiplomacyRead::ChangeStance(Book, -1, 1, HOLDiplomacyRead::GetStanceAlly()).Result, "negative sender");
	T.AssertEquals(HOLDiplomacyRead::GetResultNoSuchSlot(), HOLDiplomacyRead::ChangeStance(Book, 3, 1, HOLDiplomacyRead::GetStanceAlly()).Result, "sender beyond the slots");
	T.AssertEquals(HOLDiplomacyRead::GetResultNoSuchSlot(), HOLDiplomacyRead::ChangeStance(Book, 0, 3, HOLDiplomacyRead::GetStanceAlly()).Result, "target beyond the slots");
	T.AssertEquals(HOLDiplomacyRead::GetResultNoSuchSlot(), HOLDiplomacyRead::ChangeStance(Book, 0, -1, HOLDiplomacyRead::GetStanceAlly()).Result, "negative target");
	T.AssertEquals(HOLDiplomacyRead::GetResultSelf(), HOLDiplomacyRead::ChangeStance(Book, 1, 1, HOLDiplomacyRead::GetStanceAlly()).Result, "toward oneself");
	T.AssertEquals(HOLDiplomacyRead::GetResultBadStance(), HOLDiplomacyRead::ChangeStance(Book, 0, 1, 3).Result, "no such stance");
	T.AssertEquals(HOLDiplomacyRead::GetResultBadStance(), HOLDiplomacyRead::ChangeStance(Book, 0, 1, -1).Result, "no such stance (negative)");
	// The order: the slots, then oneself, then the stance.
	T.AssertEquals(HOLDiplomacyRead::GetResultNoSuchSlot(), HOLDiplomacyRead::ChangeStance(Book, 7, 7, 9).Result, "the slots come first");
	T.AssertEquals(HOLDiplomacyRead::GetResultSelf(), HOLDiplomacyRead::ChangeStance(Book, 1, 1, 9).Result, "then oneself");
	// A refusal changes nothing, and announces nothing.
	FHOLDiplomacyStanceChange Refused = HOLDiplomacyRead::ChangeStance(Book, 0, 1, 3);
	T.AssertFalse(Refused.bAnnounce, "a refusal is not announced");
	T.AssertEquals(HOLDiplomacyRead::GetStanceDefault(), HOLDiplomacyRead::StanceOf(Book, 0, 1), "the stance did not move");
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
	FHOLDiplomacyStances Book = HOLDiplomacyRead::NewTeamBook(Teams, IsAI);
	T.AssertTrue(Book.bLocked, "the stances are locked");
	T.AssertTrue(HOLDiplomacyRead::IsAlly(Book, 0, 1), "same team: allies (even with an AI)");
	T.AssertTrue(HOLDiplomacyRead::IsAlly(Book, 1, 0), "both ways");
	T.AssertTrue(HOLDiplomacyRead::IsAdversary(Book, 0, 2), "another team: adversary");
	T.AssertTrue(HOLDiplomacyRead::FiresAtOnce(Book, 2, 1), "and enemy");
	T.AssertTrue(HOLDiplomacyRead::FiresAtOnce(Book, 3, 4), "team 0 is alone: enemy of the other team 0");
	T.AssertFalse(HOLDiplomacyRead::IsAlly(Book, 3, 4), "not allied");
	T.AssertEquals(HOLDiplomacyRead::GetResultLocked(), HOLDiplomacyRead::ChangeStance(Book, 0, 2, HOLDiplomacyRead::GetStanceAlly()).Result, "no stance can be set");
	T.AssertEquals(HOLDiplomacyRead::GetStanceEnemy(), HOLDiplomacyRead::Relation(Book, 0, 2), "and nothing moved");
}

// The vision is a box of its own (D371): ticked toward one ally, one-sided, announced, only between allies; allies are not given it by
// being allies. The break cuts it at that second, and an alliance formed again does not give it back unasked.
void Test_DiplomacyVisionIsShared(FUnitTest& T)
{
	FHOLDiplomacyStances Book = LibHumansBook(3);
	LibMakeAllies(Book, 0, 1);
	T.AssertFalse(HOLDiplomacyRead::SharesVision(Book, 0, 1), "allies, but nobody ticked: nothing shared");
	T.AssertTrue(HOLDiplomacyRead::SharesVision(Book, 0, 0), "one sees oneself");

	FHOLDiplomacyVisionChange Tick = HOLDiplomacyRead::ChangeVision(Book, 0, 1, true);
	T.AssertEquals(HOLDiplomacyRead::GetResultOk(), Tick.Result, "player 0 opens the vision to 1");
	T.AssertTrue(Tick.bAnnounce, "announced to all");
	T.AssertEquals(0, Tick.From, "who");
	T.AssertEquals(1, Tick.To, "toward whom");
	T.AssertTrue(Tick.bShare, "opened");
	T.AssertTrue(HOLDiplomacyRead::SharesVision(Book, 0, 1), "1 sees what 0 sees");
	T.AssertFalse(HOLDiplomacyRead::SharesVision(Book, 1, 0), "one-sided: 0 does not see what 1 sees");
	T.AssertFalse(HOLDiplomacyRead::SharesVision(Book, 0, 2), "nor a third player");

	FHOLDiplomacyVisionChange Again = HOLDiplomacyRead::ChangeVision(Book, 0, 1, true);
	T.AssertEquals(HOLDiplomacyRead::GetResultOk(), Again.Result, "a repeat is not an error");
	T.AssertFalse(Again.bAnnounce, "and not announced again");

	// The break cuts it at once, from the very call of the stance (D370, D371).
	HOLDiplomacyRead::SetStance(Book, 1, 0, HOLDiplomacyRead::GetStanceEnemy());
	T.AssertFalse(HOLDiplomacyRead::SharesVision(Book, 0, 1), "cut at that second");
	LibMakeAllies(Book, 0, 1);
	T.AssertTrue(HOLDiplomacyRead::IsAlly(Book, 0, 1), "allied again");
	T.AssertFalse(HOLDiplomacyRead::SharesVision(Book, 0, 1), "but the vision must be opened again");

	FHOLDiplomacyVisionChange Closing = HOLDiplomacyRead::ChangeVision(Book, 0, 1, true);
	T.AssertTrue(Closing.bAnnounce, "opened again: announced");
	FHOLDiplomacyVisionChange Close = HOLDiplomacyRead::ChangeVision(Book, 0, 1, false);
	T.AssertTrue(Close.bAnnounce, "closing is announced too");
	T.AssertFalse(Close.bShare, "closed");
	T.AssertFalse(HOLDiplomacyRead::SharesVision(Book, 0, 1), "no more vision");
}

// The refusals of the vision, in order; and in the standard modes the vision of teammates is automatic.
void Test_DiplomacyVisionRefusals(FUnitTest& T)
{
	FHOLDiplomacyStances Book = LibHumansBook(3);
	LibMakeAllies(Book, 0, 1);
	HOLDiplomacyRead::SetStance(Book, 0, 2, HOLDiplomacyRead::GetStanceAlly());
	T.AssertEquals(HOLDiplomacyRead::GetResultNoSuchSlot(), HOLDiplomacyRead::ChangeVision(Book, 0, 5, true).Result, "nobody there");
	T.AssertEquals(HOLDiplomacyRead::GetResultNoSuchSlot(), HOLDiplomacyRead::ChangeVision(Book, -1, 1, true).Result, "nobody there (sender)");
	T.AssertEquals(HOLDiplomacyRead::GetResultSelf(), HOLDiplomacyRead::ChangeVision(Book, 1, 1, true).Result, "toward oneself");
	T.AssertEquals(HOLDiplomacyRead::GetResultNotAlly(), HOLDiplomacyRead::ChangeVision(Book, 0, 2, true).Result, "an offer not accepted is not an alliance");
	T.AssertFalse(HOLDiplomacyRead::SharesVision(Book, 0, 2), "nothing shared");
	T.AssertEquals(HOLDiplomacyRead::GetResultNotAlly(), HOLDiplomacyRead::ChangeVision(Book, 2, 1, true).Result, "not allied at all");

	TArray<bool> IsAI;
	IsAI.Add(false);
	IsAI.Add(true);
	FHOLDiplomacyStances WithAI = HOLDiplomacyRead::NewBook(2, IsAI);
	T.AssertEquals(HOLDiplomacyRead::GetResultSenderIsAI(), HOLDiplomacyRead::ChangeVision(WithAI, 1, 0, true).Result, "an AI shares nothing");

	TArray<int> Teams;
	Teams.Add(1);
	Teams.Add(1);
	Teams.Add(2);
	TArray<bool> NoAI;
	NoAI.Add(false);
	NoAI.Add(false);
	NoAI.Add(false);
	FHOLDiplomacyStances Locked = HOLDiplomacyRead::NewTeamBook(Teams, NoAI);
	T.AssertTrue(HOLDiplomacyRead::SharesVision(Locked, 0, 1), "teammates see together, automatically");
	T.AssertTrue(HOLDiplomacyRead::SharesVision(Locked, 1, 0), "both ways");
	T.AssertFalse(HOLDiplomacyRead::SharesVision(Locked, 0, 2), "not the other team");
	T.AssertEquals(HOLDiplomacyRead::GetResultLocked(), HOLDiplomacyRead::ChangeVision(Locked, 0, 1, false).Result, "no box to tick: locked");
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

void Test_DiplomacyRulesVectors(FUnitTest& T)
{
	// The default stance, the order of the stances, oneself.
	FHOLDiplomacyStances Book = LibHumansBook(4);
	T.AssertEquals(4, Book.Slots, "four slots");
	T.AssertEquals(HOLDiplomacyRead::GetStanceDefault(), HOLDiplomacyRead::Relation(Book, 0, 3), "default relation");
	T.AssertEquals(HOLDiplomacyRead::GetStanceAlly(), HOLDiplomacyRead::Relation(Book, 2, 2), "oneself is an ally");
	T.AssertTrue(HOLDiplomacyRead::GetStanceEnemy() < HOLDiplomacyRead::GetStanceNeutral() && HOLDiplomacyRead::GetStanceNeutral() < HOLDiplomacyRead::GetStanceAlly(), "Enemy < Neutral < Ally");

	// Allied only if both say Ally; the most hostile of the two applies (D375), whoever is asked.
	FHOLDiplomacyStances Pair = LibHumansBook(2);
	FHOLDiplomacyStanceChange Offer = HOLDiplomacyRead::ChangeStance(Pair, 0, 1, HOLDiplomacyRead::GetStanceAlly());
	T.AssertEquals(HOLDiplomacyRead::GetResultOk(), Offer.Result, "an offer is allowed");
	T.AssertFalse(HOLDiplomacyRead::IsAlly(Pair, 0, 1) || HOLDiplomacyRead::IsAlly(Pair, 1, 0), "one side is not an alliance");
	T.AssertFalse(Offer.bFormed, "nothing formed yet");
	T.AssertTrue(HOLDiplomacyRead::ChangeStance(Pair, 1, 0, HOLDiplomacyRead::GetStanceAlly()).bFormed, "the second says Ally: formed");
	T.AssertTrue(HOLDiplomacyRead::IsAlly(Pair, 0, 1) && HOLDiplomacyRead::IsAlly(Pair, 1, 0), "allied both ways");
	for (int A = HOLDiplomacyRead::GetStanceEnemy(); A <= HOLDiplomacyRead::GetStanceAlly(); A++)
	{
		for (int B = HOLDiplomacyRead::GetStanceEnemy(); B <= HOLDiplomacyRead::GetStanceAlly(); B++)
		{
			FHOLDiplomacyStances Mixed = LibHumansBook(2);
			HOLDiplomacyRead::SetStance(Mixed, 0, 1, A);
			HOLDiplomacyRead::SetStance(Mixed, 1, 0, B);
			T.AssertEquals(A < B ? A : B, HOLDiplomacyRead::Relation(Mixed, 0, 1), f"the most hostile applies (0 to 1, {A} {B})");
			T.AssertEquals(A < B ? A : B, HOLDiplomacyRead::Relation(Mixed, 1, 0), f"and to both (1 to 0, {A} {B})");
		}
	}

	// The break is immediate, straight to Enemy, and says so (D370); it also cuts the vision and does not give it back (D371).
	T.AssertTrue(HOLDiplomacyRead::ChangeVision(Pair, 0, 1, true).bAnnounce, "vision opens between allies");
	T.AssertTrue(HOLDiplomacyRead::SharesVision(Pair, 0, 1), "1 sees what 0 sees");
	T.AssertFalse(HOLDiplomacyRead::SharesVision(Pair, 1, 0), "one-sided");
	T.AssertFalse(HOLDiplomacyRead::ChangeVision(Pair, 0, 1, true).bAnnounce, "a repeat is not announced");
	FHOLDiplomacyStanceChange Betrayal = HOLDiplomacyRead::ChangeStance(Pair, 1, 0, HOLDiplomacyRead::GetStanceEnemy());
	T.AssertTrue(Betrayal.bBroke && !Betrayal.bFormed, "it broke an alliance");
	T.AssertEquals(HOLDiplomacyRead::GetStanceAlly(), Betrayal.Before, "before");
	T.AssertEquals(HOLDiplomacyRead::GetStanceEnemy(), Betrayal.After, "straight to Enemy");
	T.AssertTrue(HOLDiplomacyRead::FiresAtOnce(Pair, 0, 1) && HOLDiplomacyRead::FiresAtOnce(Pair, 1, 0), "both fire at once");
	T.AssertFalse(HOLDiplomacyRead::SharesVision(Pair, 0, 1), "the vision is cut at that second");
	T.AssertEquals(HOLDiplomacyRead::GetStanceEnemy(), HOLDiplomacyRead::StanceOf(Pair, 0, 1), "the betrayed one's stance drops to Enemy too");
	LibDiploVecAllies(Pair, 0, 1);
	T.AssertTrue(HOLDiplomacyRead::IsAlly(Pair, 0, 1), "allied again");
	T.AssertFalse(HOLDiplomacyRead::SharesVision(Pair, 0, 1), "the vision must be opened again");
	T.AssertFalse(HOLDiplomacyRead::ChangeStance(Pair, 0, 1, HOLDiplomacyRead::GetStanceAlly()).bAnnounce, "a repeat of a stance is not announced");

	// Neutral: an adversary that does not fire (D401); an ally is no adversary; nobody is their own adversary.
	FHOLDiplomacyStances Calm = LibHumansBook(4);
	LibDiploVecAllies(Calm, 0, 2);
	HOLDiplomacyRead::SetStance(Calm, 0, 1, HOLDiplomacyRead::GetStanceNeutral());
	HOLDiplomacyRead::SetStance(Calm, 1, 0, HOLDiplomacyRead::GetStanceNeutral());
	T.AssertFalse(HOLDiplomacyRead::FiresAtOnce(Calm, 0, 1), "Neutral does not fire");
	T.AssertTrue(HOLDiplomacyRead::IsAdversary(Calm, 0, 1) && HOLDiplomacyRead::IsAdversary(Calm, 1, 0), "Neutral is an adversary");
	T.AssertFalse(HOLDiplomacyRead::IsAdversary(Calm, 0, 2) || HOLDiplomacyRead::FiresAtOnce(Calm, 0, 2), "an ally is no adversary");
	T.AssertTrue(HOLDiplomacyRead::FiresAtOnce(Calm, 0, 3) && HOLDiplomacyRead::IsAdversary(Calm, 0, 3), "Enemy fires");
	T.AssertFalse(HOLDiplomacyRead::IsAdversary(Calm, 0, 0) || HOLDiplomacyRead::IsAdversary(Calm, 0, 9) || HOLDiplomacyRead::FiresAtOnce(Calm, 0, 9), "oneself and outside the match: nobody");

	// The refusals, in order; a refusal changes nothing.
	FHOLDiplomacyStances Three = LibHumansBook(3);
	T.AssertEquals(HOLDiplomacyRead::GetResultNoSuchSlot(), HOLDiplomacyRead::ChangeStance(Three, 3, 1, HOLDiplomacyRead::GetStanceAlly()).Result, "sender beyond the slots");
	T.AssertEquals(HOLDiplomacyRead::GetResultNoSuchSlot(), HOLDiplomacyRead::ChangeStance(Three, 0, -1, HOLDiplomacyRead::GetStanceAlly()).Result, "negative target");
	T.AssertEquals(HOLDiplomacyRead::GetResultSelf(), HOLDiplomacyRead::ChangeStance(Three, 1, 1, HOLDiplomacyRead::GetStanceAlly()).Result, "toward oneself");
	T.AssertEquals(HOLDiplomacyRead::GetResultBadStance(), HOLDiplomacyRead::ChangeStance(Three, 0, 1, 3).Result, "no such stance");
	T.AssertEquals(HOLDiplomacyRead::GetResultNoSuchSlot(), HOLDiplomacyRead::ChangeStance(Three, 7, 7, 9).Result, "the slots come first");
	T.AssertEquals(HOLDiplomacyRead::GetResultSelf(), HOLDiplomacyRead::ChangeStance(Three, 1, 1, 9).Result, "then oneself");
	T.AssertFalse(HOLDiplomacyRead::ChangeStance(Three, 0, 1, 3).bAnnounce, "a refusal is not announced");
	T.AssertEquals(HOLDiplomacyRead::GetStanceDefault(), HOLDiplomacyRead::StanceOf(Three, 0, 1), "and changes nothing");
	LibDiploVecAllies(Three, 0, 1);
	HOLDiplomacyRead::SetStance(Three, 0, 2, HOLDiplomacyRead::GetStanceAlly());
	T.AssertEquals(HOLDiplomacyRead::GetResultNotAlly(), HOLDiplomacyRead::ChangeVision(Three, 0, 2, true).Result, "vision: an offer not accepted is not an alliance");
	T.AssertEquals(HOLDiplomacyRead::GetResultSelf(), HOLDiplomacyRead::ChangeVision(Three, 1, 1, true).Result, "vision: toward oneself");
	// The AIs are alone against all (D367): Enemy to everybody, they take no stance, no vision.
	FHOLDiplomacyStances WithAI = HOLDiplomacyRead::NewBook(4, DiploVecFlags(4, 12));
	T.AssertEquals(HOLDiplomacyRead::GetResultOk(), HOLDiplomacyRead::ChangeStance(WithAI, 0, 2, HOLDiplomacyRead::GetStanceAlly()).Result, "a human may say Ally to an AI");
	T.AssertEquals(HOLDiplomacyRead::GetStanceEnemy(), HOLDiplomacyRead::Relation(WithAI, 0, 2), "it stays Enemy");
	T.AssertTrue(HOLDiplomacyRead::FiresAtOnce(WithAI, 2, 0), "and fires");
	T.AssertEquals(HOLDiplomacyRead::GetResultSenderIsAI(), HOLDiplomacyRead::ChangeStance(WithAI, 2, 0, HOLDiplomacyRead::GetStanceAlly()).Result, "an AI takes no stance");
	T.AssertEquals(HOLDiplomacyRead::GetStanceEnemy(), HOLDiplomacyRead::Relation(WithAI, 2, 3), "two AIs are enemies");
	T.AssertEquals(HOLDiplomacyRead::GetResultSenderIsAI(), HOLDiplomacyRead::ChangeVision(WithAI, 2, 0, true).Result, "an AI shares nothing");
	// The standard modes: teams fixed, stances locked, teammates see together (an AI teammate included).
	TArray<int> Teams;
	Teams.Add(1); Teams.Add(1); Teams.Add(2); Teams.Add(0); Teams.Add(0);
	FHOLDiplomacyStances Locked = HOLDiplomacyRead::NewTeamBook(Teams, DiploVecFlags(5, 18));
	T.AssertTrue(Locked.bLocked, "locked");
	T.AssertTrue(HOLDiplomacyRead::IsAlly(Locked, 0, 1) && HOLDiplomacyRead::IsAlly(Locked, 1, 0), "same team: allies");
	T.AssertTrue(HOLDiplomacyRead::FiresAtOnce(Locked, 2, 1) && HOLDiplomacyRead::IsAdversary(Locked, 0, 2), "other teams: enemies");
	T.AssertTrue(HOLDiplomacyRead::FiresAtOnce(Locked, 3, 4) && !HOLDiplomacyRead::IsAlly(Locked, 3, 4), "team 0 is alone");
	T.AssertEquals(HOLDiplomacyRead::GetResultLocked(), HOLDiplomacyRead::ChangeStance(Locked, 0, 2, HOLDiplomacyRead::GetStanceAlly()).Result, "no stance can be set");
	T.AssertTrue(HOLDiplomacyRead::SharesVision(Locked, 0, 1) && HOLDiplomacyRead::SharesVision(Locked, 1, 0), "teammates see together");
	T.AssertFalse(HOLDiplomacyRead::SharesVision(Locked, 0, 2), "not the other team");
	T.AssertEquals(HOLDiplomacyRead::GetResultLocked(), HOLDiplomacyRead::ChangeVision(Locked, 0, 1, false).Result, "no box to tick");
}

// Garrison, ramparts and allied victory vectors: script rules only (not exposed by HOLDiplomacyRead).
void Test_DiplomacyRulesVectorsGarrisonVictory(FUnitTest& T)
{
	// Garrison and ramparts (D374).
	FHOLDiplomacyBook Trio = HumansBook(3);
	DiploVecAllies(Trio, 0, 1);
	T.AssertTrue(HOLDiplomacy::CanGarrison(0, 0), "garrison in one's own");
	T.AssertFalse(HOLDiplomacy::CanGarrison(0, 1), "never in an ally's");
	T.AssertTrue(HOLDiplomacy::DoorsOpenFor(Trio, 1, 0) && HOLDiplomacy::DoorsOpenFor(Trio, 0, 1), "an ally's doors are open");
	T.AssertFalse(HOLDiplomacy::DoorsOpenFor(Trio, 1, 2), "a stranger's are not");
	T.AssertTrue(HOLDiplomacy::CanLeaveRampart(Trio, 0, 1), "allied: down when it wants");
	HOLDiplomacy::SetStance(Trio, 1, 0, HOLDiplomacy::Enemy);
	T.AssertFalse(HOLDiplomacy::CanLeaveRampart(Trio, 0, 1), "after the break: it stays there");
	T.AssertTrue(HOLDiplomacy::CanLeaveRampart(Trio, 1, 1), "the owner's own units come and go");

	// The allied victory (D373).
	FHOLDiplomacyBook Four = HumansBook(4);
	const TArray<bool> OneDown = DiploVecFlags(4, 8);
	T.AssertFalse(HOLDiplomacy::AlliedVictory(Four, OneDown).bEnded, "three enemy survivors: no victory");
	DiploVecAllies(Four, 0, 1);
	DiploVecAllies(Four, 1, 2);
	T.AssertFalse(HOLDiplomacy::AlliedVictory(Four, OneDown).bEnded, "no chain: 0-2 not allied");
	DiploVecAllies(Four, 0, 2);
	FHOLAlliedVictory Win = HOLDiplomacy::AlliedVictory(Four, OneDown);
	T.AssertTrue(Win.bEnded, "all survivors allied, one beaten: victory");
	T.AssertEquals("0,1,2", DiploVecList(Win.Winners), "the three win together");
	HOLDiplomacy::SetStance(Four, 2, 0, HOLDiplomacy::Enemy);
	T.AssertFalse(HOLDiplomacy::AlliedVictory(Four, OneDown).bEnded, "a break: not all allied any more");
	FHOLDiplomacyBook Allies = HumansBook(2);
	DiploVecAllies(Allies, 0, 1);
	T.AssertFalse(HOLDiplomacy::AlliedVictory(Allies, DiploVecFlags(2, 0)).bEnded, "nobody beaten: it goes on");
	T.AssertEquals("0", DiploVecList(HOLDiplomacy::AlliedVictory(Allies, DiploVecFlags(2, 2)).Winners), "the last standing wins");
	T.AssertFalse(HOLDiplomacy::AlliedVictory(Allies, DiploVecFlags(2, 3)).bEnded, "nobody left: not an allied victory");
	T.AssertEquals("1", DiploVecList(HOLDiplomacy::AlliedVictory(Allies, DiploVecFlags(1, 1)).Winners), "a list shorter than the slots: the missing are alive");
	// An AI alive blocks it (D367); teams give the team victory.
	FHOLDiplomacyBook AIGame = HOLDiplomacy::NewBook(4, DiploVecFlags(4, 4));
	DiploVecAllies(AIGame, 0, 1);
	T.AssertFalse(HOLDiplomacy::AlliedVictory(AIGame, DiploVecFlags(4, 8)).bEnded, "an AI alive: no victory");
	T.AssertEquals("0,1", DiploVecList(HOLDiplomacy::AlliedVictory(AIGame, DiploVecFlags(4, 12)).Winners), "the AI eliminated: both win");
	TArray<int> TwoTeams;
	TwoTeams.Add(1); TwoTeams.Add(1); TwoTeams.Add(2); TwoTeams.Add(2);
	FHOLDiplomacyBook TeamGame = HOLDiplomacy::NewTeamBook(TwoTeams, DiploVecFlags(4, 0));
	T.AssertFalse(HOLDiplomacy::AlliedVictory(TeamGame, DiploVecFlags(4, 6)).bEnded, "one of each team alive: it goes on");
	T.AssertTrue(HOLDiplomacy::AlliedVictory(TeamGame, DiploVecFlags(4, 14)).bEnded, "only team 1 left");
}
