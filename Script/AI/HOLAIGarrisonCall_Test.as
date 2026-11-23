// The AI's garrison call (HOLAIGarrisonCall.as, S4-b): an attacked tower of the AI calls the units near it, a calm one or an Easy AI
// calls nobody, and a unit is never promised to two towers. The towers and units are plain inputs; the call runs the real rule chain
// (HOLAIGarrison::TowerCalls then Entrants then HOLGarrison::AdmittedWith).

FHOLAIGarrisonTower AgcTower(int Slots, float SinceFirstHit, float SinceLastHit, FVector Place)
{
	FHOLAIGarrisonTower Tower;
	Tower.Slot = 1;
	Tower.Place = Place;
	Tower.bFinished = true;
	Tower.Slots = Slots;
	Tower.Occupants = 0;
	Tower.SinceFirstHit = SinceFirstHit;
	Tower.SinceLastHit = SinceLastHit;
	return Tower;
}

// Four own Archers near the origin (ids 100 to 103, the id is 100 plus the index) and one of the foe (id 104).
void AgcUnits(TArray<int>& Ids, TArray<int>& Slots, TArray<int>& Kinds, TArray<FVector>& Places)
{
	for (int Index = 0; Index < 4; Index++)
	{
		Ids.Add(100 + Index);
		Slots.Add(1);
		Kinds.Add(HOLGarrison::KindRangedInfantry);
		Places.Add(FVector(300.f * float(Index + 1), 0.f, 0.f));
	}
	Ids.Add(104);
	Slots.Add(0);
	Kinds.Add(HOLGarrison::KindRangedInfantry);
	Places.Add(FVector(100.f, 0.f, 0.f));
}

FString AgcList(const TArray<int>& List)
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

void Test_AIGarrisonsAttackedTower(FUnitTest& T)
{
	TArray<int> Ids;
	TArray<int> Slots;
	TArray<int> Kinds;
	TArray<FVector> Places;
	AgcUnits(Ids, Slots, Kinds, Places);
	const float Radius = HOLAIGarrison::RadiusFromCVar();
	const int Free = HOLGarrison::SlotsOf(n"TowerWood");

	// An attacked, finished tower: the nearest own Archers go in, up to its free places; not the foe's Archer.
	TArray<FHOLAIGarrisonTower> Towers;
	Towers.Add(AgcTower(Free, 4.f, 1.f, FVector(0.f, 0.f, 0.f)));
	const TArray<FHOLAIGarrisonCall> Calls = HOLAIGarrisonCall::Plan(HOLAIGarrison::RuleRangedFoot, EHOLAIDifficulty::Normal, Towers, Ids, Slots, Kinds, Places, Radius);
	T.AssertEquals(Calls.Num(), 1, "the attacked tower makes one call");
	if (Calls.Num() == 1)
	{
		T.AssertEquals(Calls[0].TowerIndex, 0, "the call is the tower's");
		T.AssertEquals(Calls[0].UnitIds.Num(), Free, "the tower is filled to its free places");
		T.AssertEquals(AgcList(Calls[0].UnitIds), "100,101,102", "the nearest own Archers, nearest first, never the foe's");
	}

	// A calm tower, then an Easy AI: nobody is called.
	TArray<FHOLAIGarrisonTower> Calm;
	Calm.Add(AgcTower(Free, 40.f, HOLAIGarrison::AttackedSecondsFromCVar() + 1.f, FVector(0.f, 0.f, 0.f)));
	T.AssertEquals(HOLAIGarrisonCall::Plan(HOLAIGarrison::RuleRangedFoot, EHOLAIDifficulty::Normal, Calm, Ids, Slots, Kinds, Places, Radius).Num(), 0, "a calm tower calls nobody");
	T.AssertEquals(HOLAIGarrisonCall::Plan(HOLAIGarrison::RuleRangedFoot, EHOLAIDifficulty::Easy, Towers, Ids, Slots, Kinds, Places, Radius).Num(), 0, "an Easy AI never garrisons");

	// Two attacked towers with one place each: a unit goes to one tower only.
	TArray<FHOLAIGarrisonTower> Two;
	Two.Add(AgcTower(1, 4.f, 1.f, FVector(0.f, 0.f, 0.f)));
	Two.Add(AgcTower(1, 4.f, 1.f, FVector(0.f, 100.f, 0.f)));
	const TArray<FHOLAIGarrisonCall> Both = HOLAIGarrisonCall::Plan(HOLAIGarrison::RuleRangedFoot, EHOLAIDifficulty::Hard, Two, Ids, Slots, Kinds, Places, Radius);
	T.AssertEquals(Both.Num(), 2, "each attacked tower calls");
	if (Both.Num() == 2)
	{
		T.AssertEquals(Both[0].UnitIds.Num(), 1, "one place, one Archer");
		T.AssertEquals(Both[1].UnitIds.Num(), 1, "one place, one Archer");
		T.AssertTrue(Both[0].UnitIds[0] != Both[1].UnitIds[0], "a unit is never sent to two towers");
	}
}
