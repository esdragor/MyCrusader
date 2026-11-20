// The faction slot of the HUD as a pure view (A6a, D201, D823; HOLFactionSlot.as): empty or visible, the gauge text, one box per
// power in the book's state, the specialization button sharing the slot. Built on the real HOLFactionResource::Make and the real
// power book rule (HOLPowerBook::StateOf).

FHOLPowerRow FactionSlotRow(FString Key, int Cost, int MinTier, bool bCoded)
{
	FHOLPowerRow Row;
	Row.Key = Key;
	Row.Cost = Cost;
	Row.MinTier = MinTier;
	Row.bEffectCoded = bCoded;
	return Row;
}

// A faction without a gauge and without a specialization button to show: the slot is empty, nothing in it.
void Test_FactionSlotEmptyForFactionWithoutResource(FUnitTest& T)
{
	TArray<FHOLPowerRow> Rows;
	Rows.Add(FactionSlotRow("a", 10, 0, true));
	TArray<int> Cooldowns;
	Cooldowns.Add(0);
	const FHOLFactionSlotView View = HOLFactionSlot::Make(EHOLFaction::BlackLegions, HOLFactionResource::Make(0, 0, 0), Rows, Cooldowns, 1, false, true);
	T.AssertFalse(View.bVisible, "Legions without button: slot empty");
	T.AssertFalse(View.bHasGauge, "no gauge");
	T.AssertEquals("", View.GaugeText, "no gauge text");
	T.AssertEquals(0, View.Powers.Num(), "no power box");
	T.AssertFalse(View.bSpecializationLit, "no lit button");
	T.AssertEquals("slot empty", View.Describe, "described empty");
	// Whereas the Dawn has a gauge: never empty, whatever the button.
	const FHOLFactionSlotView Dawn = HOLFactionSlot::Make(EHOLFaction::DawnOrder, HOLFactionResource::Make(30, 0, 100), Rows, Cooldowns, 1, false, false);
	T.AssertTrue(Dawn.bVisible, "Dawn: slot visible");
}

// The gauge text is "value / cap" from the real gauge (start brought inside the floor and the cap), the fraction is value on cap.
void Test_FactionSlotShowsGaugeText(FUnitTest& T)
{
	TArray<FHOLPowerRow> Rows;
	TArray<int> Cooldowns;
	const FHOLFactionSlotView View = HOLFactionSlot::Make(EHOLFaction::DawnOrder, HOLFactionResource::Make(30, 0, 100), Rows, Cooldowns, 0, false, false);
	T.AssertTrue(View.bVisible, "visible");
	T.AssertTrue(View.bHasGauge, "has a gauge");
	T.AssertEquals(30, View.Value, "value 30");
	T.AssertEquals(100, View.Cap, "cap 100");
	T.AssertEquals("30 / 100", View.GaugeText, "30 / 100");
	T.AssertTrue(Math::Abs(View.Fraction - 0.3) < 0.001, "30 percent of the bar");
	T.AssertEquals("slot gauge 30 / 100 powers none", View.Describe, "described");
	// A start above the cap is brought to the cap by the gauge: a full bar.
	const FHOLFactionSlotView Full = HOLFactionSlot::Make(EHOLFaction::DawnOrder, HOLFactionResource::Make(250, 0, 100), Rows, Cooldowns, 0, false, false);
	T.AssertEquals("100 / 100", Full.GaugeText, "100 / 100");
	T.AssertTrue(Math::Abs(Full.Fraction - 1.0) < 0.001, "full bar");
}

// One box per power, in the order of the rows, each in the state the book's rule gives (ready, short, tier-locked, soon, cooling).
void Test_FactionSlotPowerBoxesFollowBookStates(FUnitTest& T)
{
	TArray<FHOLPowerRow> Rows;
	Rows.Add(FactionSlotRow("a", 10, 0, true));
	Rows.Add(FactionSlotRow("b", 50, 0, true));
	Rows.Add(FactionSlotRow("c", 5, 2, true));
	Rows.Add(FactionSlotRow("d", 1, 0, false));
	Rows.Add(FactionSlotRow("e", 5, 0, true));
	TArray<int> Cooldowns;
	Cooldowns.Add(0);
	Cooldowns.Add(0);
	Cooldowns.Add(0);
	Cooldowns.Add(0);
	Cooldowns.Add(4200);
	const FHOLFactionSlotView View = HOLFactionSlot::Make(EHOLFaction::DawnOrder, HOLFactionResource::Make(30, 0, 100), Rows, Cooldowns, 1, false, false);
	T.AssertEquals(5, View.Powers.Num(), "five boxes");
	if (View.Powers.Num() != 5)
		return;
	T.AssertEquals("a", View.Powers[0].Key, "first is a");
	T.AssertTrue(View.Powers[0].State == EHOLPowerState::Ready, "a ready (cost 10, stock 30)");
	T.AssertTrue(View.Powers[1].State == EHOLPowerState::ShortOfResource, "b short (cost 50)");
	T.AssertTrue(View.Powers[2].State == EHOLPowerState::TierLocked, "c tier-locked (tier 2, faction tier 1)");
	T.AssertTrue(View.Powers[3].State == EHOLPowerState::Soon, "d soon (no coded effect)");
	T.AssertTrue(View.Powers[4].State == EHOLPowerState::Cooling, "e cooling");
	T.AssertEquals(4200, View.Powers[4].CooldownLeftMs, "e: 4 200 ms left");
	T.AssertEquals("a cost 10 ready", View.Powers[0].Describe, "a described by the book");
	T.AssertEquals("e cost 5 cooling 5 s", View.Powers[4].Describe, "e described by the book");
	T.AssertEquals("slot gauge 30 / 100 powers a cost 10 ready; b cost 50 short; c cost 5 tier 2; d cost 1 soon; e cost 5 cooling 5 s",
		View.Describe, "described");
}

// D823: the permanent specialization button shares THIS slot: alone it makes the slot visible (a faction without a gauge), dark
// until a tier is open, lit once it is; with a gauge it sits beside it.
void Test_FactionSlotSpecializationButtonSharesTheSlot(FUnitTest& T)
{
	TArray<FHOLPowerRow> Rows;
	TArray<int> Cooldowns;
	const FHOLFactionSlotView Dark = HOLFactionSlot::Make(EHOLFaction::BlackLegions, HOLFactionResource::Make(0, 0, 0), Rows, Cooldowns, 0, true, false);
	T.AssertTrue(Dark.bVisible, "the button alone makes the slot visible");
	T.AssertFalse(Dark.bHasGauge, "still no gauge");
	T.AssertTrue(Dark.bHasSpecializationButton, "button shown");
	T.AssertFalse(Dark.bSpecializationLit, "dark while no tier is open");
	T.AssertEquals("slot spec dark", Dark.Describe, "described dark");
	const FHOLFactionSlotView Lit = HOLFactionSlot::Make(EHOLFaction::BlackLegions, HOLFactionResource::Make(0, 0, 0), Rows, Cooldowns, 0, true, true);
	T.AssertTrue(Lit.bSpecializationLit, "lit once a tier is open");
	T.AssertEquals("slot spec lit", Lit.Describe, "described lit");
	// With a gauge: the same slot carries both.
	const FHOLFactionSlotView Both = HOLFactionSlot::Make(EHOLFaction::DawnOrder, HOLFactionResource::Make(30, 0, 100), Rows, Cooldowns, 0, true, true);
	T.AssertTrue(Both.bHasGauge, "gauge");
	T.AssertTrue(Both.bSpecializationLit, "and the lit button");
	T.AssertEquals("slot gauge 30 / 100 powers none spec lit", Both.Describe, "described together");
	// A tier open with no button to show lights nothing.
	const FHOLFactionSlotView NoButton = HOLFactionSlot::Make(EHOLFaction::DawnOrder, HOLFactionResource::Make(30, 0, 100), Rows, Cooldowns, 0, false, true);
	T.AssertFalse(NoButton.bSpecializationLit, "no button, nothing lit");
}

// GALADRIEL-1120d (ETAPE0-1120-B 3b i): the slot's view built from a REAL game state. The gauge is made by the chain Make + Gain
// (a duel win, D859), written in the state, read back by FromGameState; the expected text is computed from that gauge.
void Test_FactionSlotFromGameStateValue(FUnitTest& T)
{
	UHOLTestWorldHandle TestWorld = HOLTestWorld::CreateTestWorld();
	T.AssertTrue(TestWorld != nullptr, "a test world is created");
	if (TestWorld == nullptr)
		return;
	AHOLGameState State = HOLTestWorld::SpawnGameState(TestWorld, 0, 0);
	T.AssertTrue(State != nullptr, "game state");
	if (State != nullptr)
	{
		FHOLFactionResource Gauge = HOLFactionResource::Make(HOLDawnValues::HonorStart(), 0, HOLDawnValues::HonorCap());
		HOLFactionResource::Gain(Gauge, HOLDawnValues::HonorDuelWin());
		State.SetFactionResource(0, Gauge.Value);
		State.SetFactionResource(1, Gauge.Value);
		const FHOLFactionSlotView Dawn = HOLFactionSlot::FromGameState(State, 0, false, false);
		const FString Expected = f"{Gauge.Value} / {Gauge.Cap}";
		const int RowCount = HOLDawnPowers::Rows().Num();
		T.AssertTrue(Dawn.bVisible, "the Dawn's slot is visible");
		T.AssertTrue(Dawn.bHasGauge, "with a gauge");
		T.AssertEquals(Expected, Dawn.GaugeText, "the gauge text is the state's value over the cap");
		T.AssertEquals(Gauge.Value, Dawn.Value, "the value is the state's");
		T.AssertEquals(RowCount, Dawn.Powers.Num(), "one box per row of the Dawn's book");
		// Slot 1 holds a value too; the test world has no launch configuration, so GetSlotFaction gives the Dawn's (the Legions' slot cannot be
		// built from script: needs a C++ test setter, see the report): the faction comes from the state, and the slot shows ITS value.
		const FHOLFactionSlotView DawnTwo = HOLFactionSlot::FromGameState(State, 1, false, false);
		T.AssertTrue(DawnTwo.bVisible, "slot 1 reads the state faction (the Dawn's): visible");
		T.AssertEquals(Expected, DawnTwo.GaugeText, "with its own gauge value");
		// A spectator (slot -1): empty.
		const FHOLFactionSlotView Spectator = HOLFactionSlot::FromGameState(State, -1, false, false);
		T.AssertEquals("slot empty", Spectator.Describe, "a spectator has no slot");
	}
	HOLTestWorld::DestroyTestWorld(TestWorld);
}
