// Tier specialization choice, common mechanism, pure rules (SP1, HOLSpecializationRules.as; GDD 9.5, D07, D332, D683, D684, with D131).
// When a tier opens, a button lights up; the player picks one of 2 options (3 commanderies for the Templars, D131) when he decides, for free;
// the unlock runs at the Forum beside its queue and the choice is applied after 30 / 45 / 60 s (tiers 1 / 2 / 3, D684); it can be
// cancelled at no cost before the end and the choice reopens; once applied it is final (D07). The player's tier is the caller's input
// (AHOLGameState::GetTier in C++, from the hero's level, HOLHeroXP::GetTier). The delays and the provisional answers are the rules'
// constants, read from them (a test does not retype a value).

void Test_SpecializationChoiceOpensWithTheTier(FUnitTest& T)
{
	FHOLSpecializationState State = HOLSpecialization::NewState(HOLSpecialization::DefaultOptionCount);
	T.AssertEquals(2, HOLSpecialization::DefaultOptionCount, "a choice between 2 (D07)");
	T.AssertTrue(!HOLSpecialization::IsOpen(State, 1, 0), "no choice before tier 1");
	T.AssertTrue(HOLSpecialization::IsOpen(State, 1, 1), "tier 1 reached: its button lights up (D332)");
	T.AssertTrue(!HOLSpecialization::IsOpen(State, 2, 1), "not tier 2's yet");
	T.AssertTrue(HOLSpecialization::IsOpen(State, 1, 3), "the choice waits as long as the player wants (D332)");
	T.AssertTrue(!HOLSpecialization::IsOpen(State, 2, 3) && !HOLSpecialization::IsOpen(State, 3, 3),
		"tiers 2 and 3 reached: their buttons stay out until the tier before is applied (D785)");
	T.AssertTrue(!HOLSpecialization::IsOpen(State, 0, 3) && !HOLSpecialization::IsOpen(State, 4, 3), "tiers 1 to 3 only");
}

void Test_SpecializationUnlockDelayGrowsWithTheTier(FUnitTest& T)
{
	// The GDD's values, written here on purpose (D684: 30 / 45 / 60 s): the test fails if the rules drift from them.
	T.AssertEquals(30000, HOLSpecialization::DelayMs(1), "tier 1 delay (D684: 30 s)");
	T.AssertEquals(45000, HOLSpecialization::DelayMs(2), "tier 2 delay (D684: 45 s)");
	T.AssertEquals(60000, HOLSpecialization::DelayMs(3), "tier 3 delay (D684: 60 s)");
	T.AssertTrue(HOLSpecialization::DelayMs(1) < HOLSpecialization::DelayMs(2) && HOLSpecialization::DelayMs(2) < HOLSpecialization::DelayMs(3),
		"growing with the tier (D684)");

	// The choice is applied at the end of the delay, not one millisecond before.
	FHOLSpecializationState State = HOLSpecialization::NewState(HOLSpecialization::DefaultOptionCount);
	T.AssertEquals(HOLSpecialization::Ok, HOLSpecialization::Start(State, 1, 1, 1), "the player clicks option 2 of tier 1");
	T.AssertEquals(1, State.PendingTier, "the unlock runs");
	T.AssertEquals(0, HOLSpecialization::Advance(State, HOLSpecialization::DelayMs(1) - 1, true), "nothing applied at 29.999 s");
	T.AssertEquals(HOLSpecialization::None, HOLSpecialization::ChosenOption(State, 1), "not chosen yet");
	T.AssertEquals(1, HOLSpecialization::RemainingMs(State), "1 ms left on the bar");
	T.AssertEquals(1, HOLSpecialization::Advance(State, 1, true), "tier 1's choice applied at 30 s");
	T.AssertEquals(1, HOLSpecialization::ChosenOption(State, 1), "option 2 is the tier's specialization");
	T.AssertEquals(0, State.PendingTier, "nothing runs any more");
	T.AssertTrue(!HOLSpecialization::IsOpen(State, 1, 1), "the button goes out");
}

void Test_SpecializationThreeTiersLandAtThirtyFortyFiveSixtySeconds(FUnitTest& T)
{
	// The three tiers in order through the real Start and Advance, in match milliseconds: 30 s, 45 s, 60 s (D684), not 1 ms before.
	FHOLSpecializationState State = HOLSpecialization::NewState(HOLSpecialization::DefaultOptionCount);
	T.AssertEquals(HOLSpecialization::Ok, HOLSpecialization::Start(State, 1, 0, 3), "tier 1 starts");
	T.AssertEquals(0, HOLSpecialization::Advance(State, 29999, true), "tier 1 not at 29.999 s");
	T.AssertEquals(1, HOLSpecialization::Advance(State, 1, true), "tier 1 at 30 s (D684)");
	T.AssertEquals(HOLSpecialization::Ok, HOLSpecialization::Start(State, 2, 1, 3), "tier 2 starts");
	T.AssertEquals(45000, HOLSpecialization::RemainingMs(State), "45 s on the bar");
	T.AssertEquals(0, HOLSpecialization::Advance(State, 44999, true), "tier 2 not at 44.999 s");
	T.AssertEquals(2, HOLSpecialization::Advance(State, 1, true), "tier 2 at 45 s (D684)");
	T.AssertEquals(HOLSpecialization::Ok, HOLSpecialization::Start(State, 3, 0, 3), "tier 3 starts");
	T.AssertEquals(60000, HOLSpecialization::RemainingMs(State), "60 s on the bar");
	T.AssertEquals(0, HOLSpecialization::Advance(State, 59999, true), "tier 3 not at 59.999 s");
	T.AssertEquals(3, HOLSpecialization::Advance(State, 1, true), "tier 3 at 60 s (D684)");
	T.AssertEquals(0, HOLSpecialization::ChosenOption(State, 1), "tier 1's choice");
	T.AssertEquals(1, HOLSpecialization::ChosenOption(State, 2), "tier 2's choice");
	T.AssertEquals(0, HOLSpecialization::ChosenOption(State, 3), "tier 3's choice");
}

void Test_SpecializationCancelGivesTheChoiceBack(FUnitTest& T)
{
	FHOLSpecializationState State = HOLSpecialization::NewState(HOLSpecialization::DefaultOptionCount);
	T.AssertEquals(HOLSpecialization::NothingToCancel, HOLSpecialization::Cancel(State), "nothing to cancel");
	// Tier 1 applied first (D785), then tier 2's unlock is cancelled.
	HOLSpecialization::Start(State, 1, 0, 2);
	T.AssertEquals(1, HOLSpecialization::Advance(State, HOLSpecialization::DelayMs(1), true), "tier 1 applied");
	T.AssertEquals(HOLSpecialization::Ok, HOLSpecialization::Start(State, 2, 0, 2), "tier 2 starts");
	HOLSpecialization::Advance(State, HOLSpecialization::DelayMs(2) - 1, true);
	T.AssertEquals(HOLSpecialization::Ok, HOLSpecialization::Cancel(State), "cancelled before the end, at no cost (D684)");
	T.AssertEquals(0, State.PendingTier, "nothing runs");
	T.AssertEquals(HOLSpecialization::None, HOLSpecialization::ChosenOption(State, 2), "nothing chosen");
	T.AssertTrue(HOLSpecialization::IsOpen(State, 2, 2), "the choice reopens (D684)");
	T.AssertEquals(0, HOLSpecialization::Advance(State, HOLSpecialization::DelayMs(2), true), "a cancelled unlock never lands");

	// The other option may now be started; it runs its full delay.
	T.AssertEquals(HOLSpecialization::Ok, HOLSpecialization::Start(State, 2, 1, 2), "the other option");
	T.AssertEquals(HOLSpecialization::DelayMs(2), HOLSpecialization::RemainingMs(State), "a full delay");
	T.AssertEquals(2, HOLSpecialization::Advance(State, HOLSpecialization::DelayMs(2), true), "applied");
	T.AssertEquals(1, HOLSpecialization::ChosenOption(State, 2), "the second pick is the one kept");
}

void Test_SpecializationOneChoicePerTier(FUnitTest& T)
{
	FHOLSpecializationState State = HOLSpecialization::NewState(HOLSpecialization::DefaultOptionCount);
	HOLSpecialization::Start(State, 1, 0, 1);
	HOLSpecialization::Advance(State, HOLSpecialization::DelayMs(1), true);
	T.AssertEquals(HOLSpecialization::AlreadyChosen, HOLSpecialization::Start(State, 1, 1, 1), "a second choice of tier 1 is refused (D07)");
	T.AssertEquals(HOLSpecialization::NothingToCancel, HOLSpecialization::Cancel(State), "an applied choice is final (D07)");
	T.AssertEquals(0, HOLSpecialization::ChosenOption(State, 1), "the first choice stays");
	T.AssertTrue(HOLSpecialization::IsOpen(State, 2, 2), "tier 2's own choice is still to make");
}

void Test_SpecializationRefusedStarts(FUnitTest& T)
{
	FHOLSpecializationState State = HOLSpecialization::NewState(HOLSpecialization::DefaultOptionCount);
	T.AssertEquals(HOLSpecialization::BadTier, HOLSpecialization::Start(State, 0, 0, 3), "tier 0 has no choice");
	T.AssertEquals(HOLSpecialization::BadTier, HOLSpecialization::Start(State, 4, 0, 3), "no tier 4");
	T.AssertEquals(HOLSpecialization::TierNotReached, HOLSpecialization::Start(State, 2, 0, 1), "tier 2 not reached");
	T.AssertEquals(HOLSpecialization::BadOption, HOLSpecialization::Start(State, 1, 2, 1), "a third option where there are 2");
	T.AssertEquals(HOLSpecialization::BadOption, HOLSpecialization::Start(State, 1, -1, 1), "no option");
	T.AssertEquals(0, State.PendingTier, "nothing started by the refusals");

	// The Templars pick one of 3 commanderies per tier (D131).
	FHOLSpecializationState Templars = HOLSpecialization::NewState(3);
	T.AssertEquals(HOLSpecialization::Ok, HOLSpecialization::Start(Templars, 1, 2, 1), "the third commandery (D131)");
}

void Test_SpecializationTierOneFirst(FUnitTest& T)
{
	// D785 (answer of 2026-11-12 to Q-specialization Q1 = C): a player who reached tier 2 before choosing tier 1: tier 1 first, tier 2's
	// button lights up only once tier 1 is applied (the order is imposed, one bar).
	FHOLSpecializationState State = HOLSpecialization::NewState(HOLSpecialization::DefaultOptionCount);
	T.AssertTrue(!HOLSpecialization::IsOpen(State, 2, 2), "tier 1 not applied: tier 2's button stays out (D785)");
	T.AssertTrue(HOLSpecialization::Start(State, 2, 0, 2) != HOLSpecialization::Ok, "tier 2 refused before tier 1 (D785)");
	T.AssertEquals(0, State.PendingTier, "nothing runs after the refusal");
	T.AssertEquals(HOLSpecialization::Ok, HOLSpecialization::Start(State, 1, 0, 2), "tier 1's unlock runs");
	T.AssertTrue(!HOLSpecialization::IsOpen(State, 2, 2), "tier 1 running but not applied: tier 2 still out (D785)");
	T.AssertTrue(HOLSpecialization::Start(State, 2, 0, 2) != HOLSpecialization::Ok, "tier 2 still refused while tier 1 runs");
	T.AssertEquals(HOLSpecialization::Busy, HOLSpecialization::Start(State, 1, 1, 2), "the running tier is not restarted: cancel first");
	T.AssertEquals(1, HOLSpecialization::Advance(State, 30000, true), "tier 1 applied at 30 s");
	T.AssertTrue(HOLSpecialization::IsOpen(State, 2, 2), "tier 1 applied: tier 2's button lights up (D785)");
	T.AssertTrue(!HOLSpecialization::IsOpen(State, 3, 3), "tier 3 waits for tier 2 (D785)");
	T.AssertEquals(HOLSpecialization::Ok, HOLSpecialization::Start(State, 2, 0, 2), "tier 2 starts now");
	T.AssertEquals(2, State.PendingTier, "the bar shows tier 2");
}

void Test_SpecializationForumDownFollowsQ2(FUnitTest& T)
{
	// The countdown while the Forum is down (D684 leaves it to the chapter; Q-specialization Q2).
	FHOLSpecializationState State = HOLSpecialization::NewState(HOLSpecialization::DefaultOptionCount);
	HOLSpecialization::Start(State, 1, 0, 1);
	const int Applied = HOLSpecialization::Advance(State, HOLSpecialization::DelayMs(1), false);
	if (HOLSpecialization::ForumDown == HOLSpecialization::ForumDownPauses)
	{
		T.AssertEquals(0, Applied, "paused while the Forum is down");
		T.AssertEquals(HOLSpecialization::DelayMs(1), HOLSpecialization::RemainingMs(State), "the bar keeps its place");
		T.AssertEquals(1, HOLSpecialization::Advance(State, HOLSpecialization::DelayMs(1), true), "resumes once it stands again");
	}
	else if (HOLSpecialization::ForumDown == HOLSpecialization::ForumDownRuns)
	{
		T.AssertEquals(1, Applied, "the countdown runs on");
	}
	else
	{
		T.AssertEquals(0, Applied, "the unlock is cancelled");
		T.AssertEquals(0, State.PendingTier, "nothing runs");
		T.AssertTrue(HOLSpecialization::IsOpen(State, 1, 1), "the choice reopens");
	}
}
