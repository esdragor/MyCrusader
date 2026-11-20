// The Peasants that join the Nomad foundation (D817 Q-nomad-ia = A + A, HOLAINomad::HelpersFor in HOLAINomadRules.as): the idle ones, never
// the builder already sent, the nearest first, the builders held to MaxBuilders (hol.AI.NomadMaxBuilders). The asserted helpers are the
// ones the rule CHOSE: the tests check their count, their absence from Assigned and their distance against the Peasants left out, all
// computed from the places, never retyped.

FHOLAIUnit NmhPeasant(int Id, float X, bool bIdle)
{
	FHOLAIUnit Unit;
	Unit.Id = Id;
	Unit.Type = n"Peasant";
	Unit.Location = FVector(X, 0.f, 0.f);
	Unit.bIdle = bIdle;
	return Unit;
}

// Six Peasants on a line from the foundation at the origin: id 1 at 9 m, id 2 at 3 m, id 3 at 15 m, id 4 at 6 m, id 5 at 12 m, id 6 at 1 m.
TArray<FHOLAIUnit> NmhSix()
{
	TArray<FHOLAIUnit> Peasants;
	Peasants.Add(NmhPeasant(1, 900.f, true));
	Peasants.Add(NmhPeasant(2, 300.f, true));
	Peasants.Add(NmhPeasant(3, 1500.f, true));
	Peasants.Add(NmhPeasant(4, 600.f, true));
	Peasants.Add(NmhPeasant(5, 1200.f, true));
	Peasants.Add(NmhPeasant(6, 100.f, true));
	return Peasants;
}

float NmhDistanceOf(const TArray<FHOLAIUnit>& Peasants, int Id)
{
	for (int Index = 0; Index < Peasants.Num(); Index++)
	{
		if (Peasants[Index].Id == Id)
			return Peasants[Index].Location.Size();
	}
	return -1.f;
}

bool NmhHas(const TArray<int>& Ids, int Id)
{
	for (int Index = 0; Index < Ids.Num(); Index++)
	{
		if (Ids[Index] == Id)
			return true;
	}
	return false;
}

void Test_AINomadHelpersFillTheBuildersToTheCapNearestFirst(FUnitTest& T)
{
	const TArray<FHOLAIUnit> Peasants = NmhSix();
	TArray<int> Assigned;
	Assigned.Add(2);
	const int Cap = 4;
	const TArray<int> Helpers = HOLAINomad::HelpersFor(FVector::ZeroVector, Peasants, Assigned, Cap);
	T.AssertEquals(Helpers.Num(), Cap - Assigned.Num(), "the builders are brought up to the cap, the one already sent counted");
	T.AssertTrue(!NmhHas(Helpers, 2), "the builder already sent is never a helper, though it is the nearest but one");
	for (int Chosen = 0; Chosen < Helpers.Num(); Chosen++)
	{
		const float Near = NmhDistanceOf(Peasants, Helpers[Chosen]);
		for (int Other = 0; Other < Peasants.Num(); Other++)
		{
			const int OtherId = Peasants[Other].Id;
			if (OtherId == 2 || NmhHas(Helpers, OtherId))
				continue;
			const float Far = NmhDistanceOf(Peasants, OtherId);
			T.AssertTrue(Near <= Far, "a helper chosen is at least as near the foundation as every idle Peasant left out");
		}
	}
	if (Helpers.Num() > 1)
	{
		const float First = NmhDistanceOf(Peasants, Helpers[0]);
		const float Second = NmhDistanceOf(Peasants, Helpers[1]);
		T.AssertTrue(First <= Second, "the nearest first");
	}
}

void Test_AINomadHelpersAreIdleOnly(FUnitTest& T)
{
	TArray<FHOLAIUnit> Peasants = NmhSix();
	Peasants[5].bIdle = false;
	Peasants[1].bIdle = false;
	TArray<int> Assigned;
	const TArray<int> Helpers = HOLAINomad::HelpersFor(FVector::ZeroVector, Peasants, Assigned, 4);
	T.AssertTrue(!NmhHas(Helpers, 6) && !NmhHas(Helpers, 2), "a Peasant at work elsewhere does not leave it for the foundation");
	T.AssertEquals(Helpers.Num(), 4, "the cap is still reached with the idle ones left");
}

void Test_AINomadHelpersStopAtTheCapOrWhenNoneIsFree(FUnitTest& T)
{
	const TArray<FHOLAIUnit> Peasants = NmhSix();
	TArray<int> Full;
	Full.Add(1);
	Full.Add(2);
	Full.Add(3);
	Full.Add(4);
	const TArray<int> None = HOLAINomad::HelpersFor(FVector::ZeroVector, Peasants, Full, 4);
	T.AssertEquals(None.Num(), 0, "the builders already at the cap: nobody else is sent");
	TArray<int> One;
	One.Add(1);
	const TArray<int> NoRoom = HOLAINomad::HelpersFor(FVector::ZeroVector, Peasants, One, 0);
	T.AssertEquals(NoRoom.Num(), 0, "a cap of 0: nobody joins");
	TArray<FHOLAIUnit> Two;
	Two.Add(NmhPeasant(7, 500.f, true));
	Two.Add(NmhPeasant(8, 800.f, true));
	Two.Add(NmhPeasant(9, 100.f, false));
	TArray<int> Nobody;
	const TArray<int> All = HOLAINomad::HelpersFor(FVector::ZeroVector, Two, Nobody, 4);
	T.AssertEquals(All.Num(), 2, "fewer idle Peasants than room: all the idle ones join, no more");
}

void Test_AINomadHelpersUseTheRuleCap(FUnitTest& T)
{
	const int Cap = HOLAINomad::MaxBuildersFromCVar();
	TArray<FHOLAIUnit> Crowd;
	for (int Index = 0; Index < Cap + 3; Index++)
		Crowd.Add(NmhPeasant(10 + Index, 100.f * float(Index + 1), true));
	TArray<int> Assigned;
	Assigned.Add(10);
	const TArray<int> Helpers = HOLAINomad::HelpersFor(FVector::ZeroVector, Crowd, Assigned, Cap);
	T.AssertEquals(Helpers.Num(), Math::Max(0, Cap - 1), "the cap of hol.AI.NomadMaxBuilders holds the builders, the first one included");
}
