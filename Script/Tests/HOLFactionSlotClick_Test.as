// The click on the faction slot's power boxes (GALADRIEL-1120f; HOLFactionSlot::ClickPower in Script/UI/HOLFactionSlot.as). Every view is built by
// the real HOLFactionSlot::Make (real HOLFactionResource::Make and HOLPowerBook::StateOf): a box is "ready" because the book says so
// (stock, cool-down, tier, coded effect), never because the test says so. The effect asserted is the next aim and the cast.

namespace FactionSlotClickTest
{
	FHOLPowerRow Row(FString Key, int Cost, int MinTier, bool bCoded)
	{
		FHOLPowerRow Result;
		Result.Key = Key;
		Result.Cost = Cost;
		Result.MinTier = MinTier;
		Result.bEffectCoded = bCoded;
		return Result;
	}

	// Five boxes at stock 30, faction tier 1: 0 ready (cost 10), 1 short (cost 50), 2 tier-locked, 3 soon (no coded effect), 4 cooling
	// (4 200 ms left), 5 ready (cost 20).
	FHOLFactionSlotView View(int CooldownOfFirstMs)
	{
		TArray<FHOLPowerRow> Rows;
		Rows.Add(Row("a", 10, 0, true));
		Rows.Add(Row("b", 50, 0, true));
		Rows.Add(Row("c", 5, 2, true));
		Rows.Add(Row("d", 1, 0, false));
		Rows.Add(Row("e", 5, 0, true));
		Rows.Add(Row("f", 20, 0, true));
		TArray<int> Cooldowns;
		Cooldowns.Add(CooldownOfFirstMs);
		Cooldowns.Add(0);
		Cooldowns.Add(0);
		Cooldowns.Add(0);
		Cooldowns.Add(4200);
		Cooldowns.Add(0);
		return HOLFactionSlot::Make(EHOLFaction::DawnOrder, HOLFactionResource::Make(30, 0, 100), Rows, Cooldowns, 1, false, false);
	}
}

// A click on a ready box enters the aim; the click on the ground then casts that power and the aim ends; a second ground click does nothing.
void Test_FactionSlotClickReadyBoxThenGroundCasts(FUnitTest& T)
{
	const FHOLFactionSlotView Slot = FactionSlotClickTest::View(0);
	T.AssertTrue(Slot.Powers.Num() == 6 && Slot.Powers[0].State == EHOLPowerState::Ready, "the book says box 0 is ready");
	const FHOLFactionClickResult Armed = HOLFactionSlot::ClickPower(Slot, -1, EHOLFactionClickKind::Box, 0);
	T.AssertTrue(Armed.Action == EHOLFactionClickAction::Aim, "a click on a ready box enters the aim");
	T.AssertEquals(0, Armed.AimIndex, "aiming box 0");
	T.AssertEquals("aim 0", Armed.Describe, "aim line");
	const FHOLFactionClickResult Launch = HOLFactionSlot::ClickPower(Slot, Armed.AimIndex, EHOLFactionClickKind::Ground, -1);
	T.AssertTrue(Launch.Action == EHOLFactionClickAction::Launch, "a ground click while aiming casts");
	T.AssertEquals(0, Launch.CastIndex, "the power cast is box 0");
	T.AssertEquals(-1, Launch.AimIndex, "the aim ends with the cast");
	T.AssertEquals("cast 0", Launch.Describe, "cast line");
	const FHOLFactionClickResult Again = HOLFactionSlot::ClickPower(Slot, Launch.AimIndex, EHOLFactionClickKind::Ground, -1);
	T.AssertTrue(Again.Action == EHOLFactionClickAction::Nothing, "a ground click without aim is not for the slot");
	T.AssertEquals(-1, Again.CastIndex, "and casts nothing");
}

// Escape ends the aim without a cast; the ground click after it is not for the slot; Escape without aim is not for the slot.
void Test_FactionSlotClickEscapeCancels(FUnitTest& T)
{
	const FHOLFactionSlotView Slot = FactionSlotClickTest::View(0);
	const FHOLFactionClickResult Armed = HOLFactionSlot::ClickPower(Slot, -1, EHOLFactionClickKind::Box, 5);
	T.AssertEquals(5, Armed.AimIndex, "aiming box 5");
	const FHOLFactionClickResult Escaped = HOLFactionSlot::ClickPower(Slot, Armed.AimIndex, EHOLFactionClickKind::Escape, -1);
	T.AssertTrue(Escaped.Action == EHOLFactionClickAction::Cancel, "Escape cancels the aim");
	T.AssertEquals(-1, Escaped.AimIndex, "no aim left");
	const FHOLFactionClickResult After = HOLFactionSlot::ClickPower(Slot, Escaped.AimIndex, EHOLFactionClickKind::Ground, -1);
	T.AssertTrue(After.Action == EHOLFactionClickAction::Nothing, "no cast after Escape");
	const FHOLFactionClickResult Idle = HOLFactionSlot::ClickPower(Slot, -1, EHOLFactionClickKind::Escape, -1);
	T.AssertTrue(Idle.Action == EHOLFactionClickAction::Nothing, "Escape without aim is left to the game");
}

// The same box again cancels the aim; another ready box switches it.
void Test_FactionSlotClickSameBoxCancelsOtherBoxSwitches(FUnitTest& T)
{
	const FHOLFactionSlotView Slot = FactionSlotClickTest::View(0);
	const FHOLFactionClickResult Same = HOLFactionSlot::ClickPower(Slot, 0, EHOLFactionClickKind::Box, 0);
	T.AssertTrue(Same.Action == EHOLFactionClickAction::Cancel, "the same box again cancels");
	T.AssertEquals(-1, Same.AimIndex, "no aim left");
	const FHOLFactionClickResult Other = HOLFactionSlot::ClickPower(Slot, 0, EHOLFactionClickKind::Box, 5);
	T.AssertTrue(Other.Action == EHOLFactionClickAction::Aim, "another ready box switches the aim");
	T.AssertEquals(5, Other.AimIndex, "now aiming box 5");
}

// A box that is cooling, short of gauge, tier-locked or without coded effect refuses, whatever the aim was; an index outside the book is not for the slot.
void Test_FactionSlotClickRefusesBoxesThatAreNotReady(FUnitTest& T)
{
	const FHOLFactionSlotView Slot = FactionSlotClickTest::View(0);
	TArray<int> NotReady;
	NotReady.Add(1);
	NotReady.Add(2);
	NotReady.Add(3);
	NotReady.Add(4);
	for (int Index = 0; Index < NotReady.Num(); Index++)
	{
		const int Box = NotReady[Index];
		T.AssertTrue(Slot.Powers[Box].State != EHOLPowerState::Ready, f"the book says box {Box} is not ready");
		const FHOLFactionClickResult Idle = HOLFactionSlot::ClickPower(Slot, -1, EHOLFactionClickKind::Box, Box);
		T.AssertTrue(Idle.Action == EHOLFactionClickAction::Refuse, f"box {Box} refuses");
		T.AssertEquals(-1, Idle.AimIndex, f"box {Box}: no aim entered");
		const FHOLFactionClickResult Aiming = HOLFactionSlot::ClickPower(Slot, 0, EHOLFactionClickKind::Box, Box);
		T.AssertTrue(Aiming.Action == EHOLFactionClickAction::Refuse, f"box {Box} refuses while aiming another power");
		T.AssertEquals(0, Aiming.AimIndex, f"box {Box}: the aim on box 0 is kept");
	}
	T.AssertEquals("refuse cooling", HOLFactionSlot::ClickPower(Slot, -1, EHOLFactionClickKind::Box, 4).Describe, "the refusal names the book's state");
	T.AssertEquals("refuse soon", HOLFactionSlot::ClickPower(Slot, -1, EHOLFactionClickKind::Box, 3).Describe, "no coded effect reads soon");
	T.AssertTrue(HOLFactionSlot::ClickPower(Slot, -1, EHOLFactionClickKind::Box, 99).Action == EHOLFactionClickAction::Nothing, "outside the book");
	T.AssertTrue(HOLFactionSlot::ClickPower(Slot, -1, EHOLFactionClickKind::Box, -1).Action == EHOLFactionClickAction::Nothing, "before the book");
}

// The power went into cool-down while the player was aiming (a fresh view): the ground click ends the aim and casts nothing.
void Test_FactionSlotClickGroundAfterPowerCooledDoesNotCast(FUnitTest& T)
{
	const FHOLFactionSlotView Cooling = FactionSlotClickTest::View(3000);
	T.AssertTrue(Cooling.Powers[0].State == EHOLPowerState::Cooling, "the book says box 0 now cools down");
	const FHOLFactionClickResult Result = HOLFactionSlot::ClickPower(Cooling, 0, EHOLFactionClickKind::Ground, -1);
	T.AssertTrue(Result.Action == EHOLFactionClickAction::Cancel, "the aim ends");
	T.AssertEquals(-1, Result.CastIndex, "nothing is cast");
	T.AssertEquals(-1, Result.AimIndex, "no aim left");
	FHOLFactionSlotView Empty;
	T.AssertTrue(HOLFactionSlot::ClickPower(Empty, -1, EHOLFactionClickKind::Box, 0).Action == EHOLFactionClickAction::Nothing, "an empty slot takes no click");
}
