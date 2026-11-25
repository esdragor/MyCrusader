// SAURON-1125b (Dawn A7, D919): the rule of the Knight's challenge (HOLKnightChallenge). The duration comes from the table row, never copied.

// 8000 ms at 50 ms a step is 160 steps: the end step is CALCULATED by the rule.
void Test_KnightChallengeEndStep(FUnitTest& T)
{
	const int64 End = HOLKnightChallenge::EndStepOf(1000, 8000, 50);
	T.AssertEquals(int64(1160), End, "8000 ms at 50 ms a step: 160 steps after the cast");
	const int64 EndTable = HOLKnightChallenge::EndStepOf(200, HOLDawnValues::KnightChallengeDurationMs(), 50);
	T.AssertTrue(EndTable > 200, "the duration of the table row lasts at least one step");
	const int64 EndLater = HOLKnightChallenge::EndStepOf(300, HOLDawnValues::KnightChallengeDurationMs(), 50);
	T.AssertEquals(EndTable + 100, EndLater, "the end moves with the cast step, one for one");
}

// The Knight casts only alive and with its cool-down over; the table's cool-down outlasts the challenge.
void Test_KnightChallengeCanCast(FUnitTest& T)
{
	T.AssertTrue(HOLKnightChallenge::CanCast(true, 0), "alive, cool-down over");
	T.AssertFalse(HOLKnightChallenge::CanCast(true, 1), "cool-down running");
	T.AssertFalse(HOLKnightChallenge::CanCast(false, 0), "dead");
	T.AssertTrue(HOLDawnValues::CooldownMs() > HOLDawnValues::KnightChallengeDurationMs(), "the cool-down outlasts the challenge");
}
