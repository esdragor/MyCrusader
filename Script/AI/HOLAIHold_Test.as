// The AI that holds the position of a player who left (DisconnectInProgress.md T2/T3, D717): base economy and defence only, no far
// attack, no expansion, no duel accepted. Each expectation compares the held thought with the same rule on the same summary
// not held (the numbers of the rules are not typed in; the fixtures are those of HOLAIOpening_Test.as and HOLAIAttack_Test.as).

// The wood source of OpeningSummary() moved to a place, with the nearest deposit far enough for the camp rule.
FHOLAISummary HoldSummaryWoodAt(FVector Where)
{
	FHOLAISummary Summary = OpeningSummary();
	Summary.Wood.Sources[0].Location = Where;
	Summary.Wood.Sources[0].DepositDistance = Where.Size();
	return Summary;
}

void Test_HoldKeepsTheBaseEconomy(FUnitTest& T)
{
	const FHOLAISummary Summary = HoldSummaryWoodAt(FVector(1000, 500, 0));
	const FHOLAIOpeningData Data = HOLAIOpening::DefaultData();
	const FHOLAIBuildChoice Normal = HOLAIOpening::ChooseBuild(Summary, Data);
	T.AssertTrue(Normal.bBuild, "precondition: the opening builds something (the lumber camp)");
	const FHOLAIBuildChoice Held = HOLAIHold::FilterBuild(true, Normal, Summary, HOLAIHold::DefaultRules());
	T.AssertTrue(Held.bBuild, "held: the lumber camp beside the base wood is still built");
	T.AssertTrue(Held.Type == Normal.Type, "held: the same building");
	T.AssertTrue(HOLAIOpening::ChooseProduction(Summary, Data).bProduce, "precondition: a Peasant is queued");
	T.AssertTrue(HOLAIHold::Allows(true, EHOLAIHoldPart::Economy), "the economy runs while holding");
	T.AssertTrue(HOLAIHold::Allows(true, EHOLAIHoldPart::Defence), "the defence runs while holding");
	T.AssertTrue(HOLAIHold::Allows(true, EHOLAIHoldPart::Military), "the defence units are produced while holding");
}

void Test_HoldRefusesACampBeyondTheBase(FUnitTest& T)
{
	const FHOLAIHoldRules Rules = HOLAIHold::DefaultRules();
	const FHOLAISummary Far = HoldSummaryWoodAt(FVector(Rules.BaseRadius + 3000.f, 0, 0));
	const FHOLAIOpeningData Data = HOLAIOpening::DefaultData();
	const FHOLAIBuildChoice Normal = HOLAIOpening::ChooseBuild(Far, Data);
	T.AssertTrue(Normal.bBuild, "precondition: not held, the camp is built at the far wood (an expansion)");
	T.AssertFalse(HOLAIHold::FilterBuild(true, Normal, Far, Rules).bBuild, "held: no camp beyond the base");
	T.AssertTrue(HOLAIHold::FilterBuild(false, Normal, Far, Rules).bBuild, "not held: the same camp is built");
}

void Test_HoldBlocksTheFarParts(FUnitTest& T)
{
	TArray<EHOLAIHoldPart> Far;
	Far.Add(EHOLAIHoldPart::Market);
	Far.Add(EHOLAIHoldPart::Siege);
	Far.Add(EHOLAIHoldPart::Cavalry);
	Far.Add(EHOLAIHoldPart::Relays);
	Far.Add(EHOLAIHoldPart::Altar);
	Far.Add(EHOLAIHoldPart::AttackWave);
	Far.Add(EHOLAIHoldPart::DuelChallenge);
	Far.Add(EHOLAIHoldPart::DuelAccept);
	for (int Index = 0; Index < Far.Num(); Index++)
	{
		T.AssertFalse(HOLAIHold::Allows(true, Far[Index]), f"held: part {int(Far[Index])} does not run");
		T.AssertTrue(HOLAIHold::Allows(false, Far[Index]), f"not held: part {int(Far[Index])} runs as before");
	}
}

void Test_HoldLaunchesNoAttack(FUnitTest& T)
{
	FHOLAIWave NormalWave;
	const FHOLAIWaveStep Normal = HOLAIHold::WaveStep(false, NormalWave, AtkSummary(10), AtkData(), false, false, AtkPoint(), 500.f);
	T.AssertTrue(Normal.bLaunched && Normal.AttackIds.Num() > 0, "precondition: not held, ten soldiers far ahead launch the wave");
	FHOLAIWave HeldWave;
	const FHOLAIWaveStep Held = HOLAIHold::WaveStep(true, HeldWave, AtkSummary(10), AtkData(), false, false, AtkPoint(), 500.f);
	T.AssertFalse(Held.bLaunched, "held: no wave launched");
	T.AssertEquals(0, Held.AttackIds.Num(), "held: no soldier is sent to the enemy");
	T.AssertTrue(HeldWave.Phase == EHOLAIWavePhase::Home, "held: the army stays home");
}

void Test_HoldCallsBackAWaveThatIsOut(FUnitTest& T)
{
	const FHOLAISummary Summary = AtkSummary(10);
	FHOLAIWave Out = AtkWaveOut(Summary, 500.f);
	const FHOLAIWaveStep Step = HOLAIHold::WaveStep(true, Out, Summary, AtkData(), false, false, AtkPoint(), 510.f);
	T.AssertTrue(Out.Phase != EHOLAIWavePhase::Out, "held: a wave out is called back");
	T.AssertEquals(0, Step.AttackIds.Num(), "held: the wave called back attacks nothing");
}

void Test_HoldRefusesTheDuel(FUnitTest& T)
{
	const FHOLAIDuelSettings S = HOLAIDuel::Preset(EHOLAIDifficulty::Normal);
	FHOLAIDuelView Good = DuelEven();
	Good.MyLevel = 6;
	Good.EnemyLevel = 2;
	Good.bBattleNear = true;
	Good.TroopsNearMe = 20;
	T.AssertTrue(HOLAIHold::Answer(false, S, Good, 0).bYes, "precondition: not held, a good duel is accepted");
	T.AssertTrue(HOLAIHold::Challenge(false, S, Good, true, 0).bYes, "precondition: not held, a good duel is sought");
	const FHOLAIDuelDecision Answered = HOLAIHold::Answer(true, S, Good, 0);
	T.AssertFalse(Answered.bYes, "held: the same duel is refused");
	T.AssertTrue(Answered.Reason.Len() > 0, "held: the refusal says why");
	T.AssertFalse(HOLAIHold::Challenge(true, S, Good, true, 0).bYes, "held: no duel is sought");
}

void Test_HoldTwoRunsAreTheSame(FUnitTest& T)
{
	const FHOLAISummary Far = HoldSummaryWoodAt(FVector(9000, 0, 0));
	const FHOLAIBuildChoice Normal = HOLAIOpening::ChooseBuild(Far, HOLAIOpening::DefaultData());
	const FHOLAIBuildChoice A = HOLAIHold::FilterBuild(true, Normal, Far, HOLAIHold::DefaultRules());
	const FHOLAIBuildChoice B = HOLAIHold::FilterBuild(true, Normal, Far, HOLAIHold::DefaultRules());
	T.AssertTrue(A.bBuild == B.bBuild && A.Type == B.Type && A.Nothing == B.Nothing, "two runs, the same choice");
}
