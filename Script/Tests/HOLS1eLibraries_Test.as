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