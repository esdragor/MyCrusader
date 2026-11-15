// The garrison rule of the AI alone (F7 lot 4c, HOLAIGarrisonRules.as; D696 Normal and Hard garrison their towers, Easy never; D793
// who may enter; D374 never an ally's building). The places come from HOLGarrison::SlotsOf (the building table, else the tower settings),
// the admission from HOLGarrison::AdmittedWith (own slot, kind, free places): the asserted lists are computed by the rules, never retyped.
// Not wired to the game: no tower row exists in BuildingTypes.csv (GarrisonSlots 0 everywhere), FortificationsInProgress.md F7 lot 4c.

FHOLAIGarrisonTower AgrTower(int Slots, int Occupants)
{
	FHOLAIGarrisonTower Tower;
	Tower.Slot = 1;
	Tower.Place = FVector(0.f, 0.f, 0.f);
	Tower.bFinished = true;
	Tower.Slots = Slots;
	Tower.Occupants = Occupants;
	Tower.SinceFirstHit = 4.f;
	Tower.SinceLastHit = 1.f;
	return Tower;
}

// Eight units around a tower of slot 1: own Archers at 900, 300, 5000 (far), 600 and 1200 cm; an own rider at 100, an own engine at 50,
// an enemy Archer (slot 0) at 200.
void AgrUnits(TArray<int>& Slots, TArray<int>& Kinds, TArray<FVector>& Places)
{
	Slots.Add(1); Kinds.Add(HOLGarrison::KindRangedInfantry); Places.Add(FVector(900.f, 0.f, 0.f));
	Slots.Add(1); Kinds.Add(HOLGarrison::KindRangedInfantry); Places.Add(FVector(0.f, 300.f, 0.f));
	Slots.Add(1); Kinds.Add(HOLGarrison::KindCavalry); Places.Add(FVector(100.f, 0.f, 0.f));
	Slots.Add(0); Kinds.Add(HOLGarrison::KindRangedInfantry); Places.Add(FVector(200.f, 0.f, 0.f));
	Slots.Add(1); Kinds.Add(HOLGarrison::KindRangedInfantry); Places.Add(FVector(5000.f, 0.f, 0.f));
	Slots.Add(1); Kinds.Add(HOLGarrison::KindRangedInfantry); Places.Add(FVector(-600.f, 0.f, 0.f));
	Slots.Add(1); Kinds.Add(HOLGarrison::KindRangedInfantry); Places.Add(FVector(0.f, -1200.f, 0.f));
	Slots.Add(1); Kinds.Add(HOLGarrison::KindSiege); Places.Add(FVector(50.f, 0.f, 0.f));
}

FString AgrList(const TArray<int>& List)
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

void Test_AIGarrisonSendsTheNearestArchersIntoAnAttackedTower(FUnitTest& T)
{
	TArray<int> Slots;
	TArray<int> Kinds;
	TArray<FVector> Places;
	AgrUnits(Slots, Kinds, Places);
	// A Watch Tower: its places from the garrison rule (no row yet, so hol.Garrison.SlotsWatchTower).
	const FHOLAIGarrisonTower Tower = AgrTower(HOLGarrison::SlotsOf(n"TowerWood"), 0);
	const TArray<int> In = HOLAIGarrison::Entrants(HOLAIGarrison::RuleRangedFoot, EHOLAIDifficulty::Normal, Tower, Slots, Kinds, Places,
		HOLAIGarrison::RadiusFromCVar());
	T.AssertEquals(In.Num(), HOLGarrison::FreeSlots(Tower.Slots, Tower.Occupants), "an attacked tower is filled to its free places");
	T.AssertEquals(AgrList(In), "1,5,0", "the nearest own Archers enter, nearest first; not the rider, the engine, the foe, the far Archer");

	// Two places already taken: one Archer more.
	const TArray<int> One = HOLAIGarrison::Entrants(HOLAIGarrison::RuleRangedFoot, EHOLAIDifficulty::Normal,
		AgrTower(HOLGarrison::SlotsOf(n"TowerWood"), 2), Slots, Kinds, Places, HOLAIGarrison::RadiusFromCVar());
	T.AssertEquals(AgrList(One), "1", "a tower with one free place takes the nearest Archer only");
}

void Test_AIGarrisonOtherKindsFollowTheRuleAndTheGarrisonAdmission(FUnitTest& T)
{
	TArray<int> Slots;
	TArray<int> Kinds;
	TArray<FVector> Places;
	AgrUnits(Slots, Kinds, Places);
	const float Radius = HOLAIGarrison::RadiusFromCVar();
	// Any kind the garrison admits, nearest first: the rider (cavalry admitted, D793 Q2 B), never the engine (siege never).
	const TArray<int> Any = HOLAIGarrison::Entrants(HOLAIGarrison::RuleAnyNearest, EHOLAIDifficulty::Normal,
		AgrTower(HOLGarrison::SlotsOf(n"TowerWood"), 0), Slots, Kinds, Places, Radius);
	T.AssertEquals(AgrList(Any), "2,1,5", "any admitted kind, nearest first, the engine stays out");
	// Archers first, then the others to fill a Reinforced Tower.
	const TArray<int> Fill = HOLAIGarrison::Entrants(HOLAIGarrison::RuleRangedFirst, EHOLAIDifficulty::Normal,
		AgrTower(HOLGarrison::SlotsOf(n"TowerReinforced"), 0), Slots, Kinds, Places, Radius);
	T.AssertEquals(AgrList(Fill), "1,5,0,6,2", "the Archers in reach first, then the rider; never the foe, the engine, the far Archer");
	// The rule off: nobody.
	const TArray<int> Off = HOLAIGarrison::Entrants(HOLAIGarrison::RuleNone, EHOLAIDifficulty::Normal,
		AgrTower(HOLGarrison::SlotsOf(n"TowerWood"), 0), Slots, Kinds, Places, Radius);
	T.AssertEquals(Off.Num(), 0, "rule 0: the AI never garrisons");
}

void Test_AIGarrisonOnlyWhenTheTowerIsAttackedAndByLevel(FUnitTest& T)
{
	const int Slots = HOLGarrison::SlotsOf(n"TowerWood");
	FHOLAIGarrisonTower Tower = AgrTower(Slots, 0);
	T.AssertTrue(HOLAIGarrison::TowerCalls(EHOLAIDifficulty::Normal, Tower), "Normal: a finished tower hit 1 s ago, first hit 4 s ago, calls");
	T.AssertFalse(HOLAIGarrison::TowerCalls(EHOLAIDifficulty::Easy, Tower), "Easy never garrisons (D696)");
	Tower.SinceFirstHit = (HOLAIGarrison::ReactSeconds(EHOLAIDifficulty::Normal) + HOLAIGarrison::ReactSeconds(EHOLAIDifficulty::Hard)) / 2.f;
	Tower.SinceLastHit = 0.f;
	T.AssertTrue(HOLAIGarrison::TowerCalls(EHOLAIDifficulty::Hard, Tower), "Hard reacts sooner (D696)");
	T.AssertFalse(HOLAIGarrison::TowerCalls(EHOLAIDifficulty::Normal, Tower), "Normal has not reacted yet");

	FHOLAIGarrisonTower Calm = AgrTower(Slots, 0);
	Calm.SinceLastHit = HOLAIGarrison::AttackedSecondsFromCVar() + 1.f;
	T.AssertFalse(HOLAIGarrison::TowerCalls(EHOLAIDifficulty::Normal, Calm), "a tower not hit lately calls nobody");
	FHOLAIGarrisonTower Never = AgrTower(Slots, 0);
	Never.SinceFirstHit = -1.f;
	Never.SinceLastHit = -1.f;
	T.AssertFalse(HOLAIGarrison::TowerCalls(EHOLAIDifficulty::Normal, Never), "a tower never hit calls nobody");
	FHOLAIGarrisonTower Full = AgrTower(Slots, Slots);
	T.AssertFalse(HOLAIGarrison::TowerCalls(EHOLAIDifficulty::Normal, Full), "a full tower calls nobody");
	FHOLAIGarrisonTower Site = AgrTower(Slots, 0);
	Site.bFinished = false;
	T.AssertFalse(HOLAIGarrison::TowerCalls(EHOLAIDifficulty::Normal, Site), "a foundation calls nobody");
	FHOLAIGarrisonTower House = AgrTower(0, 0);
	T.AssertFalse(HOLAIGarrison::TowerCalls(EHOLAIDifficulty::Normal, House), "a building without places calls nobody");

	TArray<int> USlots;
	TArray<int> UKinds;
	TArray<FVector> UPlaces;
	AgrUnits(USlots, UKinds, UPlaces);
	T.AssertEquals(HOLAIGarrison::Entrants(HOLAIGarrison::RuleRangedFoot, EHOLAIDifficulty::Easy, AgrTower(Slots, 0), USlots, UKinds,
		UPlaces, HOLAIGarrison::RadiusFromCVar()).Num(), 0, "Easy sends nobody in");
}

void Test_AIGarrisonOccupantsLeaveOnceTheTowerIsCalm(FUnitTest& T)
{
	const float Calm = HOLAIGarrison::ExitCalmSecondsFromCVar();
	T.AssertTrue(Calm > 0.f, "by default the occupants come out after a calm (Q-garrison-ia Q2 A)");
	T.AssertFalse(HOLAIGarrison::OccupantsLeave(Calm / 2.f, Calm), "still under fire lately: they stay");
	T.AssertTrue(HOLAIGarrison::OccupantsLeave(Calm + 1.f, Calm), "calm long enough: they come out");
	T.AssertFalse(HOLAIGarrison::OccupantsLeave(Calm + 1.f, 0.f), "calm 0: they stay until the tower falls (Q2 B)");
}
