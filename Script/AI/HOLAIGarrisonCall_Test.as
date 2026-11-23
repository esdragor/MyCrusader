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

// S4-b wired: the plan is read from the AI's summary (own finished towers with places, own units with a garrison kind), the hit
// times of the towers being an input (empty = unknown = calm: no reader of the hits on a building yet, nothing is made up).
void Test_AIGarrisonPlanFromSummary(FUnitTest& T)
{
	FHOLAISummary Summary;
	FHOLAIBuilding Tower;
	Tower.OwnerPlayer = 1;
	Tower.TypeName = n"TowerWood";
	Tower.State = EHOLConstructionState::Finished;
	Tower.Location = FVector(0.f, 0.f, 0.f);
	Tower.GarrisonSlots = 2;
	Tower.Occupants = 0;
	Summary.Buildings.Add(Tower);
	FHOLAIBuilding Hall;
	Hall.OwnerPlayer = 1;
	Hall.TypeName = n"Barracks";
	Hall.Location = FVector(50.f, 0.f, 0.f);
	Hall.GarrisonSlots = 0;
	Summary.Buildings.Add(Hall);
	for (int Index = 0; Index < 3; Index++)
	{
		FHOLAIUnit Archer;
		Archer.Id = 200 + Index;
		Archer.OwnerSlot = 1;
		Archer.Type = n"Archer";
		Archer.Location = FVector(300.f * float(Index + 1), 0.f, 0.f);
		Archer.GarrisonKind = HOLGarrison::KindRangedInfantry;
		Summary.Army.Add(Archer);
	}
	FHOLAIUnit Unread;
	Unread.Id = 299;
	Unread.OwnerSlot = 1;
	Unread.Location = FVector(100.f, 0.f, 0.f);
	Summary.Army.Add(Unread);

	TArray<float> Firsts;
	TArray<float> Lasts;
	Firsts.Add(4.f);
	Firsts.Add(-1.f);
	Lasts.Add(1.f);
	Lasts.Add(-1.f);
	const float Radius = HOLAIGarrison::RadiusFromCVar();
	const TArray<FHOLAIGarrisonCall> Calls = HOLAIGarrisonCall::PlanFromSummary(1, HOLAIGarrison::RuleRangedFoot, EHOLAIDifficulty::Normal, Summary, Firsts, Lasts, Radius);
	T.AssertEquals(Calls.Num(), 1, "the attacked tower of the summary makes one call");
	if (Calls.Num() == 1)
	{
		T.AssertEquals(Calls[0].TowerIndex, 0, "the call is the first building's (the tower)");
		T.AssertEquals(AgcList(Calls[0].UnitIds), "200,201", "the two nearest Archers with a read kind fill the two places; never the unread unit");
	}
	TArray<float> None;
	T.AssertEquals(HOLAIGarrisonCall::PlanFromSummary(1, HOLAIGarrison::RuleRangedFoot, EHOLAIDifficulty::Normal, Summary, None, None, Radius).Num(), 0, "unknown hit times: no call, nothing made up");
}

// S4-b read: with no hit times given, the plan reads FHOLAIBuilding::SecondsSinceHit (the summary's reader of the hits): a tower hit within
// hol.AI.GarrisonAttackedSeconds is attacked and calls, a tower hit longer ago or never hit is calm.
void Test_AIGarrisonCallReadsSecondsSinceHit(FUnitTest& T)
{
	const float Attacked = HOLAIGarrison::AttackedSecondsFromCVar();
	TArray<float> None;
	const float Radius = HOLAIGarrison::RadiusFromCVar();
	for (int Case = 0; Case < 3; Case++)
	{
		FHOLAISummary Summary;
		FHOLAIBuilding Tower;
		Tower.OwnerPlayer = 1;
		Tower.TypeName = n"TowerWood";
		Tower.State = EHOLConstructionState::Finished;
		Tower.Location = FVector(0.f, 0.f, 0.f);
		Tower.GarrisonSlots = 2;
		Tower.Occupants = 0;
		Tower.SecondsSinceHit = (Case == 0) ? 1.f : ((Case == 1) ? Attacked + 1.f : -1.f);
		Summary.Buildings.Add(Tower);
		for (int Index = 0; Index < 2; Index++)
		{
			FHOLAIUnit Archer;
			Archer.Id = 200 + Index;
			Archer.OwnerSlot = 1;
			Archer.Location = FVector(300.f * float(Index + 1), 0.f, 0.f);
			Archer.GarrisonKind = HOLGarrison::KindRangedInfantry;
			Summary.Army.Add(Archer);
		}
		const TArray<FHOLAIGarrisonCall> Calls = HOLAIGarrisonCall::PlanFromSummary(1, HOLAIGarrison::RuleRangedFoot, EHOLAIDifficulty::Normal, Summary, None, None, Radius);
		if (Case == 0)
		{
			T.AssertEquals(Calls.Num(), 1, "a tower hit one second ago is attacked: it calls");
			if (Calls.Num() == 1)
				T.AssertEquals(AgcList(Calls[0].UnitIds), "200,201", "the two Archers enter the attacked tower");
		}
		else
		{
			T.AssertEquals(Calls.Num(), 0, "a tower hit long ago or never hit is calm: nobody is called");
		}
	}
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
