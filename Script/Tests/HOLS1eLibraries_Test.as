// Socle step 1, lot S1e (D830, D836 (2) (5), D839, CELEBORN-1116s): one smoke test per addition to the read libraries HOLDiplomacyRead (constants, change
// fiches) and HOLUnitAbilityRead (the rage's time calculations). This file compiles only once the plugin is rebuilt with these additions: until then
// the script compile fails, as expected (build asked of the main session). The full redirection of the interface is the next lot.

// HOLDiplomacyRules.h: Enemy 0, Neutral 1, Ally 2, the default stance is Enemy (DQ-1), the codes of result in their order.
void Test_DiplomacyConstantsReadLibrarySmoke(FUnitTest& T)
{
	T.AssertEquals(0, HOLDiplomacyRead::GetStanceEnemy(), "Enemy is 0");
	T.AssertEquals(1, HOLDiplomacyRead::GetStanceNeutral(), "Neutral is 1");
	T.AssertEquals(2, HOLDiplomacyRead::GetStanceAlly(), "Ally is 2");
	T.AssertEquals(HOLDiplomacyRead::GetStanceEnemy(), HOLDiplomacyRead::GetStanceDefault(), "the default stance is Enemy");
	T.AssertEquals(0, HOLDiplomacyRead::GetResultOk(), "Ok is 0");
	T.AssertEquals(5, HOLDiplomacyRead::GetResultLocked(), "Locked is 5");
	T.AssertEquals(6, HOLDiplomacyRead::GetResultNotAlly(), "NotAlly is 6");
}

// HOLDiplomacyRules_Test.as (Test_DiplomacyAllyNeedsBoth): the first Ally is announced and forms nothing, the second forms the alliance (relation 0 -> 2).
void Test_DiplomacyChangeFicheReadLibrarySmoke(FUnitTest& T)
{
	TArray<bool> IsAI;
	IsAI.Add(false);
	IsAI.Add(false);
	FHOLDiplomacyStances Book = HOLDiplomacyRead::NewBook(2, IsAI);
	FHOLDiplomacyStanceChange First = HOLDiplomacyRead::ChangeStance(Book, 0, 1, HOLDiplomacyRead::GetStanceAlly());
	T.AssertEquals(HOLDiplomacyRead::GetResultOk(), First.Result, "player 0 offers an alliance: Ok");
	T.AssertTrue(First.bAnnounce, "a stance really changed: announced");
	T.AssertFalse(First.bFormed, "nothing formed yet");
	FHOLDiplomacyStanceChange Second = HOLDiplomacyRead::ChangeStance(Book, 1, 0, HOLDiplomacyRead::GetStanceAlly());
	T.AssertTrue(Second.bFormed, "the second says Ally: formed");
	T.AssertEquals(HOLDiplomacyRead::GetStanceEnemy(), Second.Before, "the relation before is Enemy");
	T.AssertEquals(HOLDiplomacyRead::GetStanceAlly(), Second.After, "the relation after is Ally");
	FHOLDiplomacyVisionChange Vision = HOLDiplomacyRead::ChangeVision(Book, 0, 1, true);
	T.AssertEquals(HOLDiplomacyRead::GetResultOk(), Vision.Result, "allies can share vision");
	T.AssertTrue(Vision.bShare, "the fiche says shared");
}

// HOLUnitAbilityRules.h: a rage cast at 5000 ms ends by time at 15000 (10 s), its cool-down at 45000 (40 s), or when the unit is at the stop floor (20 hp).
void Test_UnitAbilityRageTimesReadLibrarySmoke(FUnitTest& T)
{
	T.AssertEquals(45000, HOLUnitAbilityRead::GetRageCooldownEndMs(5000), "cool-down ends 40 s after the cast");
	T.AssertEquals(15000, HOLUnitAbilityRead::GetRageEffectEndMs(5000), "effect ends 10 s after the cast");
	T.AssertFalse(HOLUnitAbilityRead::IsRageEffectEnded(5000, 14999, 100.0), "just before the end, healthy: still raging");
	T.AssertTrue(HOLUnitAbilityRead::IsRageEffectEnded(5000, 15000, 100.0), "at the end: over");
	T.AssertTrue(HOLUnitAbilityRead::IsRageEffectEnded(5000, 6000, 20.0), "at the stop floor: over");
	T.AssertFalse(HOLUnitAbilityRead::IsRageEffectEnded(5000, 6000, 21.0), "above the stop floor: still raging");
}

// HOLDiplomacyRules_Test.as (Test_DiplomacyGarrisonAndRamparts, S1e part 2): garrison only at home, the doors of an ally are open, those of a stranger are not.
void Test_DiplomacyGarrisonDoorsReadLibrarySmoke(FUnitTest& T)
{
	TArray<bool> IsAI;
	IsAI.Add(false);
	IsAI.Add(false);
	IsAI.Add(false);
	FHOLDiplomacyStances Book = HOLDiplomacyRead::NewBook(3, IsAI);
	HOLDiplomacyRead::SetStance(Book, 0, 1, HOLDiplomacyRead::GetStanceAlly());
	HOLDiplomacyRead::SetStance(Book, 1, 0, HOLDiplomacyRead::GetStanceAlly());
	T.AssertTrue(HOLDiplomacyRead::CanGarrison(0, 0), "garrison in one's own building");
	T.AssertFalse(HOLDiplomacyRead::CanGarrison(0, 1), "never in an ally's");
	T.AssertTrue(HOLDiplomacyRead::DoorsOpenFor(Book, 1, 1), "one's own doors");
	T.AssertTrue(HOLDiplomacyRead::DoorsOpenFor(Book, 1, 0), "an ally's doors are open");
	T.AssertFalse(HOLDiplomacyRead::DoorsOpenFor(Book, 1, 2), "a stranger's are not");
	HOLDiplomacyRead::SetStance(Book, 1, 0, HOLDiplomacyRead::GetStanceEnemy());
	T.AssertFalse(HOLDiplomacyRead::DoorsOpenFor(Book, 0, 1), "the break shuts the doors at once");
}

// HOLDiplomacyRules_Test.as (Test_DiplomacyAlliedVictory, S1e part 2): all the survivors allied to each other and one beaten: the winners, together, in order.
void Test_DiplomacyAlliedVictoryReadLibrarySmoke(FUnitTest& T)
{
	TArray<bool> IsAI;
	IsAI.Add(false);
	IsAI.Add(false);
	FHOLDiplomacyStances Pair = HOLDiplomacyRead::NewBook(2, IsAI);
	HOLDiplomacyRead::SetStance(Pair, 0, 1, HOLDiplomacyRead::GetStanceAlly());
	HOLDiplomacyRead::SetStance(Pair, 1, 0, HOLDiplomacyRead::GetStanceAlly());
	TArray<bool> NoOneDown;
	NoOneDown.Add(false);
	NoOneDown.Add(false);
	T.AssertFalse(HOLDiplomacyRead::AlliedVictory(Pair, NoOneDown).bEnded, "everybody allied but nobody beaten: it goes on");
	TArray<bool> NoOneStanding;
	NoOneStanding.Add(true);
	NoOneStanding.Add(true);
	T.AssertFalse(HOLDiplomacyRead::AlliedVictory(Pair, NoOneStanding).bEnded, "no survivor: not an allied victory");
	TArray<bool> OneDown;
	OneDown.Add(false);
	OneDown.Add(true);
	FHOLDiplomacyAlliedVictory Win = HOLDiplomacyRead::AlliedVictory(Pair, OneDown);
	T.AssertTrue(Win.bEnded, "the last standing wins at once");
	T.AssertEquals(1, Win.Winners.Num(), "one winner");
	T.AssertEquals(0, Win.Winners[0], "player 0");
}

// HOLSignalRules.as (its own InMatch, S1e part 2): a seat of the book is 0 <= Slot < Slots.
void Test_DiplomacyInMatchReadLibrarySmoke(FUnitTest& T)
{
	TArray<bool> IsAI;
	IsAI.Add(false);
	IsAI.Add(true);
	FHOLDiplomacyStances Book = HOLDiplomacyRead::NewBook(2, IsAI);
	T.AssertTrue(HOLDiplomacyRead::InMatch(Book, 0), "seat 0");
	T.AssertTrue(HOLDiplomacyRead::InMatch(Book, 1), "seat 1 (an AI is a seat)");
	T.AssertFalse(HOLDiplomacyRead::InMatch(Book, 2), "beyond the book");
	T.AssertFalse(HOLDiplomacyRead::InMatch(Book, -1), "negative");
}