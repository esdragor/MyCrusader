// S4-a (ETAPE0-1123): the AI's call of its faction powers (HOLAIPowerCall::Choose). One reserve per AI level, no cheating (D688):
// the stock left after paying must stay at or above the reserve of the level; a power without a coded effect is never cast (prepared,
// no effect). The rows and the reserves are fixtures; the book's state rule and the spend rule are the real ones.

FHOLPowerRow PowerCallRow(const FString& Key, int Cost, int MinTier, bool bCoded)
{
	FHOLPowerRow Row;
	Row.Key = Key;
	Row.Cost = Cost;
	Row.CooldownMs = 5000;
	Row.MinTier = MinTier;
	Row.bEffectCoded = bCoded;
	return Row;
}

TArray<FHOLPowerRow> PowerCallBook(bool bCoded)
{
	TArray<FHOLPowerRow> Rows;
	Rows.Add(PowerCallRow("Alpha", 30, 0, bCoded));
	return Rows;
}

TArray<int> PowerCallCooldowns()
{
	TArray<int> Result;
	Result.Add(0);
	return Result;
}

TArray<bool> PowerCallWanted(bool bWanted)
{
	TArray<bool> Result;
	Result.Add(bWanted);
	return Result;
}

FHOLAIPowerCallData PowerCallData()
{
	FHOLAIPowerCallData Data;
	Data.ReserveByLevel.Add(0);
	Data.ReserveByLevel.Add(40);
	Data.ReserveByLevel.Add(80);
	return Data;
}

void Test_AIPowerCallSpendsHonorByLevel(FUnitTest& T)
{
	const TArray<FHOLPowerRow> Rows = PowerCallBook(true);
	const TArray<int> Cooldowns = PowerCallCooldowns();
	const TArray<bool> Wanted = PowerCallWanted(true);
	const FHOLAIPowerCallData Data = PowerCallData();
	// Stock 100, cost 30: 70 left. Level 0 keeps 0, level 1 keeps 40: both cast; level 2 keeps 80: refused.
	const int Easy = HOLAIPowerCall::Choose(Rows, Cooldowns, Wanted, 100, 0, 0, Data);
	T.AssertEquals(0, Easy, "level 0 (reserve 0) casts with 70 left");
	const int Middle = HOLAIPowerCall::Choose(Rows, Cooldowns, Wanted, 100, 0, 1, Data);
	T.AssertEquals(0, Middle, "level 1 (reserve 40) casts with 70 left");
	const int Hard = HOLAIPowerCall::Choose(Rows, Cooldowns, Wanted, 100, 0, 2, Data);
	T.AssertEquals(-1, Hard, "level 2 (reserve 80) does not cast with 70 left");
	const int HardRich = HOLAIPowerCall::Choose(Rows, Cooldowns, Wanted, 120, 0, 2, Data);
	T.AssertEquals(0, HardRich, "level 2 casts once the stock leaves 90");
	// A level past the table reads the last reserve; a stock below the cost never casts (no cheating).
	const int Past = HOLAIPowerCall::Choose(Rows, Cooldowns, Wanted, 100, 0, 9, Data);
	T.AssertEquals(-1, Past, "a level past the table reads the last reserve (80)");
	const int Poor = HOLAIPowerCall::Choose(Rows, Cooldowns, Wanted, 20, 0, 0, Data);
	T.AssertEquals(-1, Poor, "the AI never casts what it cannot pay");
}

void Test_AIPowerCallNothingWithoutCodedEffect(FUnitTest& T)
{
	const TArray<FHOLPowerRow> Rows = PowerCallBook(false);
	const int Soon = HOLAIPowerCall::Choose(Rows, PowerCallCooldowns(), PowerCallWanted(true), 500, 0, 0, PowerCallData());
	T.AssertEquals(-1, Soon, "prepared without effect: a power whose effect is not coded is never cast");
	const TArray<FHOLPowerRow> Coded = PowerCallBook(true);
	const int Idle = HOLAIPowerCall::Choose(Coded, PowerCallCooldowns(), PowerCallWanted(false), 500, 0, 0, PowerCallData());
	T.AssertEquals(-1, Idle, "no trigger from the brain: nothing is cast");
}
