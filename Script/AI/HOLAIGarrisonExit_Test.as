// The AI's garrison exit (HOLAIGarrisonExit.as, S4-b): a tower of the AI with occupants lets them out once it has been calm for
// CalmSeconds; an attacked tower, an empty tower, a foe's tower and CalmSeconds 0 (they stay until the tower falls) empty nothing.
// The towers come from a real FHOLAISummary; the decision runs HOLAIGarrison::OccupantsLeave.

FHOLAIBuilding AgxTower(int Owner, int Occupants, float SecondsSinceHit, bool bFinished)
{
	FHOLAIBuilding Tower;
	Tower.OwnerPlayer = Owner;
	Tower.TypeName = n"TowerWood";
	Tower.State = bFinished ? EHOLConstructionState::Finished : EHOLConstructionState::UnderConstruction;
	Tower.Location = FVector(0.f, 0.f, 0.f);
	Tower.GarrisonSlots = 3;
	Tower.Occupants = Occupants;
	Tower.SecondsSinceHit = SecondsSinceHit;
	return Tower;
}

FString AgxList(const TArray<int>& List)
{
	FString Text = "";
	for (int Index = 0; Index < List.Num(); Index++)
	{
		if (Index > 0)
			Text += ",";
		Text += f"{List[Index]}";
	}
	return Text;
}

FHOLAISummary AgxSummary()
{
	FHOLAISummary Summary;
	Summary.Buildings.Add(AgxTower(1, 2, 20.f, true));   // 0: own, occupied, hit long ago: they leave
	Summary.Buildings.Add(AgxTower(1, 2, 3.f, true));    // 1: own, occupied, hit 3 s ago: still attacked, they stay
	Summary.Buildings.Add(AgxTower(1, 0, 20.f, true));   // 2: own, empty: nothing to let out
	Summary.Buildings.Add(AgxTower(0, 2, 20.f, true));   // 3: the foe's: never ours to empty
	Summary.Buildings.Add(AgxTower(1, 1, -1.f, true));   // 4: own, one occupant, never hit: calm, he leaves
	Summary.Buildings.Add(AgxTower(1, 2, 20.f, false));  // 5: own, not finished: no garrison
	return Summary;
}

void Test_AIGarrisonExitEmptiesTheCalmOccupiedTowers(FUnitTest& T)
{
	const float Calm = HOLAIGarrison::ExitCalmSecondsFromCVar();
	TArray<float> None;
	const FHOLAISummary Summary = AgxSummary();
	const TArray<int> Out = HOLAIGarrisonExit::TowersToEmpty(1, Summary, None, Calm);
	T.AssertEquals(AgxList(Out), "0,4", "only the own, finished, occupied towers calm for the delay let their occupants out");
}

void Test_AIGarrisonExitReadsTheGivenHitTimesFirstAndStaysWhenCalmIsZero(FUnitTest& T)
{
	const float Calm = HOLAIGarrison::ExitCalmSecondsFromCVar();
	const FHOLAISummary Summary = AgxSummary();
	// Given hit times override the summary's own: tower 0 was hit 1 s ago, tower 1 long ago (-1 = not given, the summary's own is read).
	TArray<float> Given;
	Given.Add(1.f);
	Given.Add(Calm + 5.f);
	const TArray<int> Out = HOLAIGarrisonExit::TowersToEmpty(1, Summary, Given, Calm);
	T.AssertEquals(AgxList(Out), "1,4", "the given hit times decide for towers 0 and 1; tower 4 reads its own (never hit)");
	const TArray<int> Never = HOLAIGarrisonExit::TowersToEmpty(1, Summary, Given, 0.f);
	T.AssertEquals(Never.Num(), 0, "CalmSeconds 0: the occupants stay until the tower falls, nothing is emptied");
}
