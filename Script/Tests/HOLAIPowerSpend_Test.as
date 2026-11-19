// L12 / A12: the AI's rule for spending the faction resource in a power (HOLAIPowerSpend::Choose). The rows are fixtures (invented keys,
// costs and thresholds: nothing here is a design value); the book's state rule is the real one (HOLPowerBook::StateOf).

FHOLPowerRow PowerSpendRow(const FString& Key, int Cost, int MinTier, bool bCoded)
{
	FHOLPowerRow Row;
	Row.Key = Key;
	Row.Cost = Cost;
	Row.CooldownMs = 5000;
	Row.MinTier = MinTier;
	Row.bEffectCoded = bCoded;
	return Row;
}

// Two powers: "Alpha" (cost 30, tier 0) first, "Beta" (cost 50, tier 2) second.
TArray<FHOLPowerRow> PowerSpendBook()
{
	TArray<FHOLPowerRow> Rows;
	Rows.Add(PowerSpendRow("Alpha", 30, 0, true));
	Rows.Add(PowerSpendRow("Beta", 50, 2, true));
	return Rows;
}

TArray<int> PowerSpendCooldowns(int First, int Second)
{
	TArray<int> Result;
	Result.Add(First);
	Result.Add(Second);
	return Result;
}

TArray<bool> PowerSpendWanted(bool First, bool Second)
{
	TArray<bool> Result;
	Result.Add(First);
	Result.Add(Second);
	return Result;
}

void Test_AIPowerSpendNothingWhenNoPowerReady(FUnitTest& T)
{
	TArray<FHOLPowerRow> Rows = PowerSpendBook();
	// Beta is tier-locked at tier 1, Alpha is cooling: nothing is ready although both are wanted and the stock is large.
	const int Locked = HOLAIPowerSpend::Choose(Rows, PowerSpendCooldowns(4000, 0), PowerSpendWanted(true, true), 500, 1, 0);
	T.AssertEquals(-1, Locked, "tier-locked and cooling: nothing to cast");
	// A power with no coded effect is "Soon": never cast.
	Rows[0].bEffectCoded = false;
	const int Soon = HOLAIPowerSpend::Choose(Rows, PowerSpendCooldowns(0, 0), PowerSpendWanted(true, true), 500, 1, 0);
	T.AssertEquals(-1, Soon, "a power without a coded effect is never cast");
	// No power at all.
	TArray<FHOLPowerRow> NoRows;
	T.AssertEquals(-1, HOLAIPowerSpend::Choose(NoRows, TArray<int>(), TArray<bool>(), 500, 3, 0), "an empty book casts nothing");
	// Control: the cool-down over, the same situation casts the first of the book once the cool-down is over.
	Rows[0].bEffectCoded = true;
	const int Open = HOLAIPowerSpend::Choose(Rows, PowerSpendCooldowns(0, 0), PowerSpendWanted(true, true), 500, 2, 0);
	T.AssertEquals(0, Open, "control: a ready wanted power is cast");
}

void Test_AIPowerSpendKeepsTheReserve(FUnitTest& T)
{
	const TArray<FHOLPowerRow> Rows = PowerSpendBook();
	// Stock 100, Alpha costs 30: 70 would be left.
	T.AssertEquals(0, HOLAIPowerSpend::Choose(Rows, PowerSpendCooldowns(0, 0), PowerSpendWanted(true, false), 100, 0, 70), "70 left = the reserve: cast");
	T.AssertEquals(-1, HOLAIPowerSpend::Choose(Rows, PowerSpendCooldowns(0, 0), PowerSpendWanted(true, false), 100, 0, 71), "one under the reserve: keep it");
	// The reserve is per level: a higher reserve skips the dear power and falls to the cheap one.
	T.AssertEquals(1, HOLAIPowerSpend::Choose(Rows, PowerSpendCooldowns(0, 0), PowerSpendWanted(false, true), 100, 2, 50), "Beta leaves 50: cast at reserve 50");
	T.AssertEquals(-1, HOLAIPowerSpend::Choose(Rows, PowerSpendCooldowns(0, 0), PowerSpendWanted(false, true), 100, 2, 51), "Beta would leave 50 < 51: keep it");
	T.AssertEquals(0, HOLAIPowerSpend::Choose(Rows, PowerSpendCooldowns(0, 0), PowerSpendWanted(true, true), 100, 2, 60), "the dear power is out of reach, the cheap one fits");
}

void Test_AIPowerSpendPicksAReadyPower(FUnitTest& T)
{
	const TArray<FHOLPowerRow> Rows = PowerSpendBook();
	T.AssertEquals(0, HOLAIPowerSpend::Choose(Rows, PowerSpendCooldowns(0, 0), PowerSpendWanted(true, true), 500, 2, 0), "both ready and wanted: the first of the book");
	T.AssertEquals(1, HOLAIPowerSpend::Choose(Rows, PowerSpendCooldowns(0, 0), PowerSpendWanted(false, true), 500, 2, 0), "only the second is wanted: it is cast");
	T.AssertEquals(1, HOLAIPowerSpend::Choose(Rows, PowerSpendCooldowns(3000, 0), PowerSpendWanted(true, true), 500, 2, 0), "the first is cooling: the second is cast");
	T.AssertEquals(-1, HOLAIPowerSpend::Choose(Rows, PowerSpendCooldowns(0, 0), PowerSpendWanted(false, false), 500, 2, 0), "ready but wanted by none: not spent as soon as possible");
	// The choice agrees with the book's own rule (the real chain).
	const int Chosen = HOLAIPowerSpend::Choose(Rows, PowerSpendCooldowns(0, 0), PowerSpendWanted(true, true), 500, 2, 0);
	const EHOLPowerState State = HOLPowerBook::StateOf(Rows[Chosen], 500, 0, 2);
	T.AssertTrue(State == EHOLPowerState::Ready, "the chosen power is Ready for the book");
}

void Test_AIPowerSpendNeverCheats(FUnitTest& T)
{
	const TArray<FHOLPowerRow> Rows = PowerSpendBook();
	// Control: enough stock, cool-down over: Alpha is cast.
	T.AssertEquals(0, HOLAIPowerSpend::Choose(Rows, PowerSpendCooldowns(0, 0), PowerSpendWanted(true, false), 30, 0, 0), "control: stock = cost casts");
	// Stock short by one: nothing, whatever the reserve.
	T.AssertEquals(-1, HOLAIPowerSpend::Choose(Rows, PowerSpendCooldowns(0, 0), PowerSpendWanted(true, false), 29, 0, 0), "stock one under the cost: nothing");
	T.AssertEquals(-1, HOLAIPowerSpend::Choose(Rows, PowerSpendCooldowns(0, 0), PowerSpendWanted(true, false), 29, 0, -100), "a negative reserve does not buy a power");
	// Cool-down running by one millisecond: nothing, however rich.
	T.AssertEquals(-1, HOLAIPowerSpend::Choose(Rows, PowerSpendCooldowns(1, 0), PowerSpendWanted(true, false), 500, 0, 0), "cool-down still running: nothing");
	// Tier not reached: nothing.
	T.AssertEquals(-1, HOLAIPowerSpend::Choose(Rows, PowerSpendCooldowns(0, 0), PowerSpendWanted(false, true), 500, 1, 0), "tier not reached: nothing");
	// Arrays that do not match the book: nothing.
	T.AssertEquals(-1, HOLAIPowerSpend::Choose(Rows, PowerSpendCooldowns(0, 0), TArray<bool>(), 500, 2, 0), "a trigger list of the wrong size casts nothing");
}
