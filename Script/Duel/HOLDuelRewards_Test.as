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

// Step 3a: the orders of EVERY duel that ended in one step, as the ledger's `Ended` loop collects them. Two duels over: a victory
// (slots 0 and 1: Triumph, Demoralised) and a refusal with its cost (slots 2 and 3: Hesitation on the target) -> three orders,
// each with the time the morale table gives (40 s, 40 s, 25 s).
void Test_OrdersOfEndedCoversEveryEndedDuel(FUnitTest& T)
{
	FHOLDuelSettings S = HOLDuel::Preset();
	FHOLDuel Won = HOLDuel::Advance(S, RewardDuelActive(S), 3000, 0, 40);
	FHOLDuel Refused = HOLDuel::Respond(S, HOLDuel::Challenge(S, 2, 3), false);
	TArray<FHOLDuelEnded> Ended;
	FHOLDuelEnded First;
	First.Duel = Won;
	First.Effects = HOLDuel::Effects(S, Won);
	Ended.Add(First);
	FHOLDuelEnded Second;
	Second.Duel = Refused;
	Second.Effects = HOLDuel::Effects(S, Refused);
	Ended.Add(Second);
	TArray<FHOLDuelMoraleOrder> Orders = HOLDuelRewards::OrdersOfEnded(S, Ended);
	T.AssertEquals(3, Orders.Num(), "three orders: Triumph, Demoralised, Hesitation");
	int TriumphMs = -1;
	int DemoralisedMs = -1;
	int HesitationMs = -1;
	for (int Index = 0; Index < Orders.Num(); Index++)
	{
		if (Orders[Index].EffectId == HOLMoraleTable::Triumph && Orders[Index].Slot == 1)
			TriumphMs = Orders[Index].DurationMs;
		if (Orders[Index].EffectId == HOLMoraleTable::Demoralised && Orders[Index].Slot == 0)
			DemoralisedMs = Orders[Index].DurationMs;
		if (Orders[Index].EffectId == HOLMoraleTable::Hesitation && Orders[Index].Slot == 3)
			HesitationMs = Orders[Index].DurationMs;
	}
	T.AssertEquals(HOLMoraleTable::Row(HOLMoraleTable::Triumph).DurationMs, TriumphMs, "Triumph on the winner, the table's time");
	T.AssertEquals(HOLMoraleTable::Row(HOLMoraleTable::Demoralised).DurationMs, DemoralisedMs, "Demoralised on the loser, the table's time");
	T.AssertEquals(HOLMoraleTable::Row(HOLMoraleTable::Hesitation).DurationMs, HesitationMs, "Hesitation on the refuser of the second duel");
	T.AssertEquals(40000, TriumphMs, "Triumph lasts 40 s");
	T.AssertEquals(40000, DemoralisedMs, "Demoralised lasts 40 s");
	T.AssertEquals(25000, HesitationMs, "Hesitation lasts 25 s");
}

// D859 n. 3, the Honor of the duel (pure; the ledger applies it to the gauges of the Dawn's slots alone). The expected values are
// CALCULATED by the gauge chain (HOLFactionResource::Make / Gain / Lose), never copied from the numbers.
int HonorExpectedGain(int Current, int Amount)
{
	FHOLFactionResource G = HOLFactionResource::Make(Current, 0, HOLDawnValues::HonorCap());
	HOLFactionResource::Gain(G, Amount);
	return G.Value;
}

int HonorExpectedLose(int Current, int Amount)
{
	FHOLFactionResource G = HOLFactionResource::Make(Current, 0, HOLDawnValues::HonorCap());
	HOLFactionResource::Lose(G, Amount);
	return G.Value;
}

TArray<FHOLDuelEnded> HonorTwoEnded(const FHOLDuelSettings& S)
{
	FHOLDuel Won = HOLDuel::Advance(S, RewardDuelActive(S), 3000, 0, 40);
	FHOLDuel Refused = HOLDuel::Respond(S, HOLDuel::Challenge(S, 2, 3), false);
	TArray<FHOLDuelEnded> Ended;
	FHOLDuelEnded First;
	First.Duel = Won;
	First.Effects = HOLDuel::Effects(S, Won);
	Ended.Add(First);
	FHOLDuelEnded Second;
	Second.Duel = Refused;
	Second.Effects = HOLDuel::Effects(S, Refused);
	Ended.Add(Second);
	return Ended;
}

int HonorDeltaOf(const TArray<FHOLDuelHonorOrder>& Orders, int Slot, int& OutNew)
{
	int Sum = 0;
	for (int Index = 0; Index < Orders.Num(); Index++)
	{
		if (Orders[Index].Slot == Slot)
		{
			Sum += Orders[Index].Delta;
			OutNew = Orders[Index].NewValue;
		}
	}
	return Sum;
}

// Slot 1 wins (the Dawn), slot 0 loses (Legions), slot 3 refuses (the Dawn, 50 Honor), slot 2 challenged (Legions).
void Test_HonorOfEndedWinAndRefusalOfTheDawn(FUnitTest& T)
{
	FHOLDuelSettings S = HOLDuel::Preset();
	TArray<bool> HasGauge;
	HasGauge.Add(false);
	HasGauge.Add(true);
	HasGauge.Add(false);
	HasGauge.Add(true);
	TArray<int> Current;
	Current.Add(0);
	Current.Add(0);
	Current.Add(0);
	Current.Add(50);
	TArray<FHOLDuelHonorOrder> Orders = HOLDuelRewards::HonorOfEnded(S, HonorTwoEnded(S), HasGauge, Current);
	int NewWinner = -1;
	int NewRefuser = -1;
	int NewLoser = -1;
	int WinDelta = HonorDeltaOf(Orders, 1, NewWinner);
	int RefuseDelta = HonorDeltaOf(Orders, 3, NewRefuser);
	int LoserDelta = HonorDeltaOf(Orders, 0, NewLoser);
	T.AssertEquals(HonorExpectedGain(0, HOLDawnValues::HonorDuelWin()), NewWinner, "the Dawn's winner: the gauge after Gain");
	T.AssertEquals(30, WinDelta, "a win of the Dawn is +30 Honor (D859)");
	T.AssertEquals(HonorExpectedLose(50, HOLDawnValues::HonorDuelRefuse()), NewRefuser, "the Dawn's refuser: the gauge after Lose");
	T.AssertEquals(-5, RefuseDelta, "a refusal that costs is -5 Honor (D859)");
	T.AssertEquals(0, LoserDelta, "the loser gains nothing");
}

void Test_HonorOfEndedLegionsSlotsGetNothing(FUnitTest& T)
{
	FHOLDuelSettings S = HOLDuel::Preset();
	TArray<bool> HasGauge;
	TArray<int> Current;
	for (int Slot = 0; Slot < 4; Slot++)
	{
		HasGauge.Add(false);
		Current.Add(50);
	}
	TArray<FHOLDuelHonorOrder> Orders = HOLDuelRewards::HonorOfEnded(S, HonorTwoEnded(S), HasGauge, Current);
	T.AssertEquals(0, Orders.Num(), "no slot with a gauge: no order, the Legions' slots stay at 0");
}

void Test_HonorOfEndedCapAndFloorHold(FUnitTest& T)
{
	FHOLDuelSettings S = HOLDuel::Preset();
	TArray<bool> HasGauge;
	for (int Slot = 0; Slot < 4; Slot++)
		HasGauge.Add(true);
	TArray<int> Current;
	Current.Add(0);
	Current.Add(90);
	Current.Add(0);
	Current.Add(3);
	TArray<FHOLDuelHonorOrder> Orders = HOLDuelRewards::HonorOfEnded(S, HonorTwoEnded(S), HasGauge, Current);
	int NewWinner = -1;
	int NewRefuser = -1;
	int WinDelta = HonorDeltaOf(Orders, 1, NewWinner);
	int RefuseDelta = HonorDeltaOf(Orders, 3, NewRefuser);
	T.AssertEquals(HonorExpectedGain(90, HOLDawnValues::HonorDuelWin()), NewWinner, "the cap: the gauge after Gain");
	T.AssertEquals(HOLDawnValues::HonorCap(), NewWinner, "the winner stops at the cap");
	T.AssertEquals(NewWinner - 90, WinDelta, "the delta is the real change, not 30");
	T.AssertEquals(HonorExpectedLose(3, HOLDawnValues::HonorDuelRefuse()), NewRefuser, "the floor: the gauge after Lose");
	T.AssertEquals(NewRefuser - 3, RefuseDelta, "the delta is the real change, not -5");
}

void Test_HonorOfAcceptGivesTheAcceptorOfTheDawn(FUnitTest& T)
{
	FHOLDuelHonorOrder Dawn = HOLDuelRewards::HonorOfAccept(3, true, 95);
	FHOLDuelHonorOrder Legions = HOLDuelRewards::HonorOfAccept(2, false, 95);
	T.AssertEquals(3, Dawn.Slot, "the order is for the acceptor");
	T.AssertEquals(HonorExpectedGain(95, HOLDawnValues::HonorDuelAccept()), Dawn.NewValue, "the gauge after Gain, the cap held");
	T.AssertEquals(Dawn.NewValue - 95, Dawn.Delta, "the delta is the real change");
	FHOLDuelHonorOrder Fresh = HOLDuelRewards::HonorOfAccept(3, true, 0);
	T.AssertEquals(10, Fresh.Delta, "an acceptance of the Dawn is +10 Honor (D859)");
	T.AssertEquals(-1, Legions.Slot, "a Legions slot: no order");
}