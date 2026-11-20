// The morale rewards of a duel's end (DuelInProgress.md D7): the orders come from a duel really played to its end (Challenge,
// Respond, Advance), the durations and the radius from the morale table.

const int RewardChallenger = 0;
const int RewardTarget = 1;

// A duel accepted at once: the Active state to play from.
FHOLDuel RewardDuelActive(const FHOLDuelSettings& S)
{
	FHOLDuel Pending = HOLDuel::Challenge(S, RewardChallenger, RewardTarget);
	return HOLDuel::Respond(S, Pending, true);
}

// Victory for the target: the challenger falls (health 0), the target stands.
void Test_DuelRewardsWinnerTriumphLoserDemoralised(FUnitTest& T)
{
	FHOLDuelSettings S = HOLDuel::Preset();
	FHOLDuel D = RewardDuelActive(S);
	D = HOLDuel::Advance(S, D, 3000, 0, 40);
	TArray<FHOLDuelMoraleOrder> Orders = HOLDuelRewards::Orders(S, D);
	T.AssertEquals(2, Orders.Num(), "two orders: the winner and the loser");
	if (Orders.Num() != 2)
		return;
	int WinnerEffect = -1;
	int LoserEffect = -1;
	int WinnerMs = 0;
	int LoserMs = 0;
	for (int Index = 0; Index < Orders.Num(); Index++)
	{
		if (Orders[Index].Slot == RewardTarget)
		{
			WinnerEffect = Orders[Index].EffectId;
			WinnerMs = Orders[Index].DurationMs;
		}
		if (Orders[Index].Slot == RewardChallenger)
		{
			LoserEffect = Orders[Index].EffectId;
			LoserMs = Orders[Index].DurationMs;
		}
	}
	T.AssertEquals(HOLMoraleTable::Triumph, WinnerEffect, "Triumph for the winner");
	T.AssertEquals(HOLMoraleTable::Demoralised, LoserEffect, "Demoralised for the loser");
	T.AssertEquals(HOLMoraleTable::Row(HOLMoraleTable::Triumph).DurationMs, WinnerMs, "the winner's time is the table's");
	T.AssertEquals(HOLMoraleTable::Row(HOLMoraleTable::Demoralised).DurationMs, LoserMs, "the loser's time is the table's");
}

// A refusal, spoken or silent: Hesitation on the target alone, never on the challenger, never Triumph.
void Test_DuelRewardsRefusalHesitationOnRefuserOnly(FUnitTest& T)
{
	FHOLDuelSettings S = HOLDuel::Preset();
	FHOLDuel Pending = HOLDuel::Challenge(S, RewardChallenger, RewardTarget);
	FHOLDuel Refused = HOLDuel::Respond(S, Pending, false);
	FHOLDuel Silent = HOLDuel::Advance(S, Pending, S.ResponseMs, 100, 100);
	TArray<FHOLDuelMoraleOrder> AfterNo = HOLDuelRewards::Orders(S, Refused);
	TArray<FHOLDuelMoraleOrder> AfterSilence = HOLDuelRewards::Orders(S, Silent);
	T.AssertEquals(1, AfterNo.Num(), "a spoken refusal: one order");
	T.AssertEquals(1, AfterSilence.Num(), "a silent refusal: one order");
	if (AfterNo.Num() == 1)
	{
		T.AssertEquals(RewardTarget, AfterNo[0].Slot, "on the one who refuses");
		T.AssertEquals(HOLMoraleTable::Hesitation, AfterNo[0].EffectId, "Hesitation");
		T.AssertEquals(HOLMoraleTable::Row(HOLMoraleTable::Hesitation).DurationMs, AfterNo[0].DurationMs, "the table's time");
	}
	if (AfterSilence.Num() == 1)
	{
		T.AssertEquals(RewardTarget, AfterSilence[0].Slot, "silence: on the target too");
		T.AssertEquals(HOLMoraleTable::Hesitation, AfterSilence[0].EffectId, "silence: Hesitation");
	}
}

// Both heroes fall in the same step: no winner, no reward, whichever way DU-Q1 is set.
void Test_DuelRewardsDoubleKnockOutGivesNothing(FUnitTest& T)
{
	FHOLDuelSettings S = HOLDuel::Preset();
	FHOLDuel D = HOLDuel::Advance(S, RewardDuelActive(S), 50, 0, 0);
	T.AssertTrue(D.End == EHOLDuelEnd::DoubleKnockOut, "both fell");
	TArray<FHOLDuelMoraleOrder> BothFall = HOLDuelRewards::Orders(S, D);
	T.AssertEquals(0, BothFall.Num(), "nothing after a double knock-out");
	S.bDoubleKnockOutBothFall = false;
	TArray<FHOLDuelMoraleOrder> BothLeft = HOLDuelRewards::Orders(S, D);
	T.AssertEquals(0, BothLeft.Num(), "nothing either with both left at one health");
}

// While the duel is pending or going on, nothing is posed.
void Test_DuelRewardsNothingWhileNotOver(FUnitTest& T)
{
	FHOLDuelSettings S = HOLDuel::Preset();
	FHOLDuel Pending = HOLDuel::Challenge(S, RewardChallenger, RewardTarget);
	FHOLDuel Active = RewardDuelActive(S);
	TArray<FHOLDuelMoraleOrder> WhilePending = HOLDuelRewards::Orders(S, Pending);
	TArray<FHOLDuelMoraleOrder> WhileActive = HOLDuelRewards::Orders(S, Active);
	T.AssertEquals(0, WhilePending.Num(), "nothing while the window runs");
	T.AssertEquals(0, WhileActive.Num(), "nothing while the duel is on");
}

// D859 n. 8 (step 1d): the radius of the three duel effects is the morale table's (12 m in Effects.csv), read by Orders itself.
// Triumph and Demoralised from a duel won, Hesitation from a refusal with its cost: every order carries the table's radius.
void Test_DuelOrdersRadiusFromTable(FUnitTest& T)
{
	FHOLDuelSettings S = HOLDuel::Preset();
	FHOLDuel Won = HOLDuel::Advance(S, RewardDuelActive(S), 3000, 0, 40);
	FHOLDuel Refused = HOLDuel::Respond(S, HOLDuel::Challenge(S, RewardChallenger, RewardTarget), false);
	TArray<FHOLDuelMoraleOrder> WonOrders = HOLDuelRewards::Orders(S, Won);
	TArray<FHOLDuelMoraleOrder> RefusedOrders = HOLDuelRewards::Orders(S, Refused);
	T.AssertEquals(2, WonOrders.Num(), "a victory: Triumph and Demoralised");
	T.AssertEquals(1, RefusedOrders.Num(), "a refusal: Hesitation");
	TArray<FHOLDuelMoraleOrder> All;
	All.Append(WonOrders);
	All.Append(RefusedOrders);
	for (int Index = 0; Index < All.Num(); Index++)
	{
		const int TableRadius = HOLMoraleTable::Row(All[Index].EffectId).RadiusCm;
		T.AssertEquals(TableRadius, All[Index].RadiusCm, "the order's radius is the table's");
		T.AssertEquals(1200, All[Index].RadiusCm, "12 m (D859 n. 8)");
	}
}