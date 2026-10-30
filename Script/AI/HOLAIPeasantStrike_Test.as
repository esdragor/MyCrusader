// A Peasant finishes a weakened enemy soldier off, never a sound one (IA4, D572 confirmed, option A): on the REAL type tables
// (HOLAIArmy::LoadFigures reads the Peasant's and the soldiers' figures), no number of the combat typed in.

FHOLAIUnit StrikeUnit(int Id, FName Type, int Health, FVector Where, int Slot)
{
	FHOLAIUnit Unit;
	Unit.Id = Id;
	Unit.OwnerSlot = Slot;
	Unit.Type = Type;
	Unit.Health = Health;
	Unit.Location = Where;
	Unit.TargetId = -1;
	return Unit;
}

FHOLAIArmyData StrikeData()
{
	FHOLAIArmyData Data = HOLAIArmy::DefaultData();
	HOLAIArmy::LoadFigures(Data);
	return Data;
}

int StrikeHealthOf(const FHOLAIArmyData& Data, FName Type)
{
	return Data.Figures[Data.Types.FindIndex(Type)].Health;
}

// The duel at remaining health: a Spearman at 5 is beaten with the margin, a sound one is not; a wounded Peasant fares worse.
void Test_PeasantDuelUsesRemainingHealth(FUnitTest& T)
{
	const FHOLAIArmyData Data = StrikeData();
	T.AssertTrue(Data.PeasantFigures.Health > 0, "the Peasant's figures were read from the table");
	const int Full = StrikeHealthOf(Data, n"Spearman");
	const FHOLAIUnit Peasant = StrikeUnit(1, n"Peasant", Data.PeasantFigures.Health, FVector(0, 0, 0), 1);
	const float Sound = HOLAIArmy::DuelRatio(Data, Peasant, StrikeUnit(2, n"Spearman", Full, FVector(500, 0, 0), 0));
	const float Weak = HOLAIArmy::DuelRatio(Data, Peasant, StrikeUnit(3, n"Spearman", 5, FVector(500, 0, 0), 0));
	T.AssertTrue(Sound > 0.f && Sound < 1.f, "a sound Spearman beats a Peasant: the duel is under 1");
	T.AssertTrue(Weak >= Data.StrikeMargin, "a Spearman at 5 health: the Peasant wins with the margin");
	T.AssertTrue(Weak > Sound, "the less health the soldier has, the better the duel");
	const FHOLAIUnit Hurt = StrikeUnit(4, n"Peasant", 10, FVector(0, 0, 0), 1);
	T.AssertTrue(HOLAIArmy::DuelRatio(Data, Hurt, StrikeUnit(3, n"Spearman", 5, FVector(500, 0, 0), 0)) < Weak, "a wounded Peasant fares worse against the same soldier");
	T.AssertEquals(0.f, HOLAIArmy::DuelRatio(Data, Peasant, StrikeUnit(5, n"Nothing", 10, FVector(500, 0, 0), 0)), "a type the data does not know: no duel");
}

// Who is struck: the weak soldier within reach, not the sound one, not the one too far, never an enemy Peasant.
void Test_PeasantStrikesOnlyAWonDuel(FUnitTest& T)
{
	const FHOLAIArmyData Data = StrikeData();
	const int Full = StrikeHealthOf(Data, n"Spearman");
	FHOLAISummary Summary;
	Summary.Peasants.Add(StrikeUnit(10, n"Peasant", Data.PeasantFigures.Health, FVector(0, 0, 0), 1));
	const FHOLAIUnit Peasant = Summary.Peasants[0];
	T.AssertEquals(-1, HOLAIArmy::StrikeTargetFor(Summary, Data, Peasant), "no enemy: no target");

	Summary.Enemies.Add(StrikeUnit(50, n"Spearman", Full, FVector(600, 0, 0), 0));
	T.AssertEquals(-1, HOLAIArmy::StrikeTargetFor(Summary, Data, Peasant), "a sound Spearman: never");

	Summary.Enemies.Add(StrikeUnit(51, n"Spearman", 5, FVector(1000, 0, 0), 0));
	T.AssertEquals(51, HOLAIArmy::StrikeTargetFor(Summary, Data, Peasant), "a Spearman at 5 health: finished off");

	Summary.Enemies.Add(StrikeUnit(52, n"Spearman", 3, FVector(800, 0, 0), 0));
	T.AssertEquals(52, HOLAIArmy::StrikeTargetFor(Summary, Data, Peasant), "two weak ones: the nearest");

	FHOLAISummary Far;
	Far.Peasants.Add(Peasant);
	Far.Enemies.Add(StrikeUnit(53, n"Spearman", 5, FVector(Data.FleeRadius + 500.f, 0, 0), 0));
	T.AssertEquals(-1, HOLAIArmy::StrikeTargetFor(Far, Data, Peasant), "a weak Spearman out of reach: not chased");

	FHOLAISummary Workers;
	Workers.Peasants.Add(Peasant);
	Workers.Enemies.Add(StrikeUnit(54, n"Peasant", 5, FVector(600, 0, 0), 0));
	T.AssertEquals(-1, HOLAIArmy::StrikeTargetFor(Workers, Data, Peasant), "an enemy Peasant is no soldier");
}

// The orders: one per Peasant that can win, not again at once, and the Peasant that strikes does not run; the others run.
void Test_PeasantStrikeOrdersAndFlight(FUnitTest& T)
{
	const FHOLAIArmyData Data = StrikeData();
	const int Full = StrikeHealthOf(Data, n"Spearman");
	FHOLAISummary Summary;
	Summary.Peasants.Add(StrikeUnit(10, n"Peasant", Data.PeasantFigures.Health, FVector(0, 0, 0), 1));
	Summary.Peasants.Add(StrikeUnit(11, n"Peasant", 6, FVector(100, 0, 0), 1));
	Summary.Enemies.Add(StrikeUnit(50, n"Spearman", 5, FVector(700, 0, 0), 0));
	Summary.Enemies.Add(StrikeUnit(51, n"Spearman", Full, FVector(5000, 0, 0), 0));

	FHOLAIFlight State;
	TArray<FHOLAIStrike> Strikes = HOLAIArmy::StrikeOrders(State, Summary, Data, 100.f);
	T.AssertTrue(Strikes.Num() >= 1, "the sound Peasant finishes the weak Spearman");
	bool bTenStrikes = false;
	for (int Index = 0; Index < Strikes.Num(); Index++)
	{
		bTenStrikes = bTenStrikes || (Strikes[Index].PeasantId == 10 && Strikes[Index].TargetId == 50);
		T.AssertTrue(Strikes[Index].PeasantId != 11, "a Peasant at 6 health does not win against a Spearman at 5");
	}
	T.AssertTrue(bTenStrikes, "Peasant 10 is ordered on Spearman 50");
	T.AssertEquals(0, HOLAIArmy::StrikeOrders(State, Summary, Data, 101.f).Num(), "not ordered again at once");
	T.AssertEquals(Strikes.Num(), HOLAIArmy::StrikeOrders(State, Summary, Data, 100.f + Data.StrikeEvery).Num(), "again after the delay");

	// The Peasant that can win does not run; the one that cannot still does (the Spearman at 700 cm is within FleeRadius).
	FHOLAIFlight Flight;
	const TArray<int> Runners = HOLAIArmy::FleeOrders(Flight, Summary, Data, 100.f);
	T.AssertFalse(Runners.Contains(10), "the Peasant that strikes does not run");
	T.AssertTrue(Runners.Contains(11), "the Peasant that would lose still runs");
}

// A sound soldier alone: nobody strikes, everybody near runs, as before.
void Test_PeasantsStillRunFromSoundSoldiers(FUnitTest& T)
{
	const FHOLAIArmyData Data = StrikeData();
	FHOLAISummary Summary;
	Summary.Peasants.Add(StrikeUnit(10, n"Peasant", Data.PeasantFigures.Health, FVector(0, 0, 0), 1));
	Summary.Enemies.Add(StrikeUnit(50, n"Spearman", StrikeHealthOf(Data, n"Spearman"), FVector(700, 0, 0), 0));
	FHOLAIFlight State;
	T.AssertEquals(0, HOLAIArmy::StrikeOrders(State, Summary, Data, 100.f).Num(), "a sound Spearman: no strike");
	T.AssertTrue(HOLAIArmy::FleeOrders(State, Summary, Data, 100.f).Contains(10), "the Peasant runs");
}
