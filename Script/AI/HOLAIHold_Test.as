// The AI that holds the position of a player who left (DisconnectInProgress.md T2/T3, D717): base economy and defence only, no far
// attack, no expansion, no duel accepted. Each expectation compares the held thought with the same rule on the same summary
// not held (the numbers of the rules are not typed in; the fixtures are those of HOLAIOpening_Test.as and HOLAIAttack_Test.as).

// Fixtures copied from HOLAIOpening_Test.as, HOLAIAttack_Test.as and HOLAIDuel_Test.as (each test file is a module of its own).
FHOLAISourceNear OpeningSource(int SourceIndex, FVector Where)
{
	FHOLAISourceNear Source;
	Source.SourceIndex = SourceIndex;
	Source.Location = Where;
	Source.Remaining = 200;
	return Source;
}

// A slot with its finished centre at the origin, 5 Peasants (ids 1 to 5, all idle), pop 5 of 10, 200 of each resource, one
// source of food (#10) and of wood (#11) on the +X side.
FHOLAISummary OpeningSummary()
{
	FHOLAISummary Summary;
	Summary.Food.Stock = 200;
	Summary.Wood.Stock = 200;
	Summary.Stone.Stock = 200;
	Summary.Gold.Stock = 200;
	for (int Id = 1; Id <= 5; Id++)
	{
		Summary.IdlePeasants.Add(Id);
		Summary.PeasantIds.Add(Id);
	}
	Summary.Food.Sources.Add(OpeningSource(10, FVector(1000, 0, 0)));
	Summary.Wood.Sources.Add(OpeningSource(11, FVector(1000, 500, 0)));
	Summary.bHasPopulation = true;
	Summary.PopulationUsed = 5;
	Summary.PopulationCap = 10;
	Summary.bHasCentre = true;
	Summary.CentreLocation = FVector(0, 0, 0);
	FHOLAIBuilding Centre;
	Centre.TypeName = n"MainCentre";
	Centre.State = EHOLConstructionState::Finished;
	Centre.Location = FVector(0, 0, 0);
	Summary.Buildings.Add(Centre);
	return Summary;
}

FHOLAITypeFigures AtkFigures(int Health, int Damage, float Interval, float Range, int MeleeArmor, int RangedArmor, bool bRanged, float Speed, int Food, int Wood, int Gold)
{
	FHOLAITypeFigures Figures;
	Figures.Health = Health;
	Figures.Damage = Damage;
	Figures.AttackInterval = Interval;
	Figures.AttackRange = Range;
	Figures.MeleeArmor = MeleeArmor;
	Figures.RangedArmor = RangedArmor;
	Figures.bRanged = bRanged;
	Figures.Speed = Speed;
	Figures.CostFood = Food;
	Figures.CostWood = Wood;
	Figures.CostGold = Gold;
	return Figures;
}

FHOLAIArmyData AtkData()
{
	FHOLAIArmyData Data = HOLAIArmy::DefaultData();
	Data.Types.Empty();
	Data.Figures.Empty();
	FHOLAITypeFigures Spear = AtkFigures(80, 8, 1.875f, 130.f, 0, 0, false, 777.f, 60, 20, 0);
	Spear.ArmorCategories.Add(n"LightInfantry");
	Spear.BonusCategories.Add(n"Cavalry");
	Spear.BonusDamage.Add(17.f);
	Data.Types.Add(n"Spearman");
	Data.Figures.Add(Spear);
	FHOLAITypeFigures Heavy = AtkFigures(155, 12, 1.375f, 70.f, 4, 4, false, 697.f, 100, 0, 20);
	Heavy.ArmorCategories.Add(n"HeavyInfantry");
	Heavy.ArmorCategories.Add(n"Heavy");
	Data.Types.Add(n"ManAtArms");
	Data.Figures.Add(Heavy);
	FHOLAITypeFigures Bow = AtkFigures(70, 5, 1.625f, 2000.f, 0, 0, true, 779.f, 30, 50, 0);
	Bow.ArmorCategories.Add(n"Ranged");
	Bow.BonusCategories.Add(n"LightInfantry");
	Bow.BonusDamage.Add(5.f);
	Data.Types.Add(n"Archer");
	Data.Figures.Add(Bow);
	return Data;
}

// The units of a list: Count of a type at a place, identifiers from FirstId.
void AtkAdd(TArray<FHOLAIUnit>& List, int FirstId, FName Type, int Count, FVector Where, int Slot, bool bIdle, int TargetId)
{
	for (int Index = 0; Index < Count; Index++)
	{
		FHOLAIUnit Unit;
		Unit.Id = FirstId + Index;
		Unit.OwnerSlot = Slot;
		Unit.Type = Type;
		Unit.Health = Type == n"Archer" ? 70 : (Type == n"ManAtArms" ? 155 : 80);
		Unit.bIdle = bIdle;
		Unit.TargetId = TargetId;
		Unit.Location = Where;
		List.Add(Unit);
	}
}

// A slot with its centre at the origin, a House at (1500, 0) and a Barracks at (-1500, 1500) (the enemy centre at (-8000, 0): the assembly
// point is at (-1500, 0)).
FHOLAISummary AtkBase()
{
	FHOLAISummary Summary;
	Summary.bHasCentre = true;
	Summary.CentreLocation = FVector(0, 0, 0);
	Summary.bHasEnemyCentre = true;
	Summary.EnemyCentreLocation = FVector(-8000, 0, 0);
	FHOLAIBuilding House;
	House.TypeName = n"House";
	House.State = EHOLConstructionState::Finished;
	House.Location = FVector(1500, 0, 0);
	Summary.Buildings.Add(House);
	FHOLAIBuilding Barracks;
	Barracks.TypeName = n"Barracks";
	Barracks.State = EHOLConstructionState::Finished;
	Barracks.Location = FVector(-1500, 1500, 0);
	Summary.Buildings.Add(Barracks);
	return Summary;
}

FVector AtkPoint()
{
	return FVector(-1500, 0, 0);
}

bool AtkNear(FVector A, FVector B)
{
	return (A - B).Size() < 1.0;
}

// The summary of a slot ready to attack: ten idle Spearmen at the assembly point (-1500, 0), two enemy Spearmen at the enemy centre
// (-8000, 0). Ratio far above 1.5, game time 500 s.
FHOLAISummary AtkSummary(int Soldiers)
{
	FHOLAISummary Summary = AtkBase();
	AtkAdd(Summary.Army, 1, n"Spearman", Soldiers, AtkPoint(), 1, true, -1);
	AtkAdd(Summary.Enemies, 50, n"Spearman", 2, FVector(-8000, 500, 0), 0, false, -1);
	return Summary;
}

// A wave that is out with the soldiers of the summary (as the launch makes it).
FHOLAIWave AtkWaveOut(const FHOLAISummary& Summary, float Now)
{
	FHOLAIWave Wave;
	Wave.Phase = EHOLAIWavePhase::Out;
	Wave.Ids = HOLAIArmy::SoldierIds(Summary);
	Wave.LaunchedSize = Wave.Ids.Num();
	Wave.PhaseStart = Now;
	Wave.LastOrder = Now;
	Wave.NextLaunch = Now + 90.f;
	return Wave;
}

FHOLAIDuelView DuelEven()
{
	FHOLAIDuelView V;
	V.MyLevel = 4;
	V.EnemyLevel = 4;
	return V;
}


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
