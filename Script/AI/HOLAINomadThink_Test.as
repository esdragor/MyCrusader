// What the Nomad seat's thought sends to the foundation (D817 Q-nomad-ia = A + A, CELEBORN-1120pa): ONE builder places the forum (the
// Peasant nearest the spot), then the idle Peasants join it nearest first, the builders held to MaxBuilders (hol.AI.NomadMaxBuilders)
// with the ones already at work on the foundation counted. HOLAINomadThink::Think only calls these two functions and sends the ids they
// return, so the ids asserted here are the ones the orders carry. Every expected number is computed from the places of the Peasants
// (a distance, a count of the Peasants at work near the foundation), never retyped. The C++ chain (HeroesOfLiberty.AI.Nomad.*) stays.

FHOLAIUnit NthPeasant(int Id, float X, bool bIdle)
{
	FHOLAIUnit Unit;
	Unit.Id = Id;
	Unit.Type = n"Peasant";
	Unit.Location = FVector(X, 0.f, 0.f);
	Unit.bIdle = bIdle;
	return Unit;
}

bool NthHas(const TArray<int>& Ids, int Id)
{
	for (int Index = 0; Index < Ids.Num(); Index++)
	{
		if (Ids[Index] == Id)
			return true;
	}
	return false;
}

float NthDistanceOf(const TArray<FHOLAIUnit>& Peasants, int Id)
{
	for (int Index = 0; Index < Peasants.Num(); Index++)
	{
		if (Peasants[Index].Id == Id)
			return Peasants[Index].Location.Size();
	}
	return -1.f;
}

// The foundation at the origin: id 1 at work 2 m from it (the builder already sent), id 2 at work 50 m away (gathering elsewhere), and five
// idle Peasants at 9 / 3 / 15 / 6 / 12 m (ids 3 to 7).
FHOLAISummary NthSummary()
{
	FHOLAISummary Summary;
	Summary.Peasants.Add(NthPeasant(1, 200.f, false));
	Summary.Peasants.Add(NthPeasant(2, 5000.f, false));
	Summary.Peasants.Add(NthPeasant(3, 900.f, true));
	Summary.Peasants.Add(NthPeasant(4, 300.f, true));
	Summary.Peasants.Add(NthPeasant(5, 1500.f, true));
	Summary.Peasants.Add(NthPeasant(6, 600.f, true));
	Summary.Peasants.Add(NthPeasant(7, 1200.f, true));
	for (int Index = 0; Index < Summary.Peasants.Num(); Index++)
	{
		if (Summary.Peasants[Index].bIdle)
			Summary.IdlePeasants.Add(Summary.Peasants[Index].Id);
	}
	return Summary;
}

// The Peasants at work within Radius of the foundation, counted from the places (the builders the cap has to count).
int NthWorkersNear(const FHOLAISummary& Summary, float Radius)
{
	int Count = 0;
	for (int Index = 0; Index < Summary.Peasants.Num(); Index++)
	{
		if (!Summary.Peasants[Index].bIdle && Summary.Peasants[Index].Location.Size() <= Radius)
			Count++;
	}
	return Count;
}

void Test_NomadThinkJoinersFillTheBuildersToTheCap(FUnitTest& T)
{
	const FHOLAISummary Summary = NthSummary();
	const int Cap = 4;
	const float Radius = 700.f;
	const TArray<int> Joiners = HOLAINomadThink::JoinersFor(FVector::ZeroVector, Summary, Cap, Radius);
	const int Expected = Cap - NthWorkersNear(Summary, Radius);
	T.AssertEquals(Joiners.Num(), Expected, "the builders are brought up to the cap, the Peasant already at work on the foundation counted");
	for (int Chosen = 0; Chosen < Joiners.Num(); Chosen++)
	{
		T.AssertTrue(NthHas(Summary.IdlePeasants, Joiners[Chosen]), "a Peasant sent is an idle one");
		const float Near = NthDistanceOf(Summary.Peasants, Joiners[Chosen]);
		for (int Other = 0; Other < Summary.IdlePeasants.Num(); Other++)
		{
			const int OtherId = Summary.IdlePeasants[Other];
			if (NthHas(Joiners, OtherId))
				continue;
			T.AssertTrue(Near <= NthDistanceOf(Summary.Peasants, OtherId), "a Peasant sent is at least as near the foundation as every idle one left out");
		}
	}
}

void Test_NomadThinkAPeasantAtWorkFarAwayDoesNotCountAsABuilder(FUnitTest& T)
{
	FHOLAISummary Summary = NthSummary();
	const int Cap = 4;
	const float Radius = 700.f;
	// The builder near the foundation goes idle too: nobody builds yet, only the far Peasant (id 2) is at work.
	Summary.Peasants[0].bIdle = true;
	Summary.IdlePeasants.Add(1);
	const TArray<int> Joiners = HOLAINomadThink::JoinersFor(FVector::ZeroVector, Summary, Cap, Radius);
	T.AssertEquals(Joiners.Num(), Cap - NthWorkersNear(Summary, Radius), "the Peasant at work 50 m away is not a builder of the foundation: the whole cap is open");
	T.AssertTrue(!NthHas(Joiners, 2), "the Peasant at work elsewhere is never sent");
}

void Test_NomadThinkFirstBuilderIsTheNearestPeasantToTheSpot(FUnitTest& T)
{
	const FHOLAISummary Summary = NthSummary();
	const int First = HOLAINomadThink::FirstBuilderFor(FVector::ZeroVector, Summary.Peasants);
	T.AssertTrue(First != -1, "a builder is chosen when there are Peasants");
	if (First == -1)
		return;
	const float Near = NthDistanceOf(Summary.Peasants, First);
	for (int Index = 0; Index < Summary.Peasants.Num(); Index++)
	{
		T.AssertTrue(Near <= NthDistanceOf(Summary.Peasants, Summary.Peasants[Index].Id), "the first builder is at least as near the spot as every other Peasant");
	}
	TArray<FHOLAIUnit> Nobody;
	T.AssertEquals(HOLAINomadThink::FirstBuilderFor(FVector::ZeroVector, Nobody), -1, "no Peasant, no builder");
}
