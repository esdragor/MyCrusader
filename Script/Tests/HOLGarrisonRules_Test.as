// Garrison, pure rule (F7, HOLGarrisonRules.as; D693, D374). The building belongs to slot 0; slot 1 is an ally of slot 0, slot 2 an enemy
// (the rule refuses both alike). Every count is computed by HOLGarrison::AdmittedInto / Admitted, the list the order will put inside.

TArray<int> GarrisonInts(int A, int B = -9, int C = -9, int D = -9, int E = -9, int F = -9)
{
	TArray<int> Values;
	Values.Add(A);
	if (B != -9) Values.Add(B);
	if (C != -9) Values.Add(C);
	if (D != -9) Values.Add(D);
	if (E != -9) Values.Add(E);
	if (F != -9) Values.Add(F);
	return Values;
}

// Five own foot soldiers ask a Watch tower (default 3 places) holding one: two enter, the first two asked; a full tower takes nobody.
void Test_GarrisonEntersUpToTheFreePlacesAndNoMore(FUnitTest& T)
{
	TArray<int> Slots = GarrisonInts(0, 0, 0, 0, 0);
	TArray<int> Kinds = GarrisonInts(HOLGarrison::KindRangedInfantry, HOLGarrison::KindRangedInfantry, HOLGarrison::KindMeleeInfantry,
		HOLGarrison::KindRangedInfantry, HOLGarrison::KindMeleeInfantry);
	TArray<int> In = HOLGarrison::AdmittedInto(n"TowerWood", Slots, Kinds, 0, 1);
	T.AssertEquals(In.Num(), 2, "a Watch tower of 3 places holding 1 takes 2 more");
	T.AssertTrue(In.Num() == 2 && In[0] == 0 && In[1] == 1, "the first two asked enter");
	T.AssertEquals(HOLGarrison::AdmittedInto(n"TowerWood", Slots, Kinds, 0, 3).Num(), 0, "a full Watch tower takes nobody");
	T.AssertEquals(HOLGarrison::AdmittedInto(n"TowerStone", Slots, Kinds, 0, 0).Num(), 5, "a Stone tower (5 places) takes all five");
	T.AssertEquals(HOLGarrison::AdmittedInto(n"House", Slots, Kinds, 0, 0).Num(), 0, "a House has no garrison");
}

// An ally's soldier (D374) and an enemy's are refused and take no place: in a tower of 3, three own soldiers enter around them.
void Test_GarrisonRefusesAllyAndEnemy(FUnitTest& T)
{
	TArray<int> Slots = GarrisonInts(2, 0, 1, 0, 0, 0);
	TArray<int> Kinds = GarrisonInts(HOLGarrison::KindRangedInfantry, HOLGarrison::KindRangedInfantry, HOLGarrison::KindRangedInfantry,
		HOLGarrison::KindRangedInfantry, HOLGarrison::KindRangedInfantry, HOLGarrison::KindRangedInfantry);
	TArray<int> In = HOLGarrison::AdmittedInto(n"TowerWood", Slots, Kinds, 0, 0);
	T.AssertEquals(In.Num(), 3, "three own soldiers enter");
	T.AssertFalse(In.Contains(0), "the enemy (slot 2) does not enter");
	T.AssertFalse(In.Contains(2), "the ally (slot 1) does not enter");
	T.AssertTrue(In.Contains(1) && In.Contains(3) && In.Contains(4), "the first three own soldiers enter");
	T.AssertFalse(In.Contains(5), "the fourth own soldier finds no place");
}

// Kinds by default (D793, Q-garrison Q2 B): everything but siege enters, cavalry included; siege never, whatever the settings.
void Test_GarrisonKindsByDefault(FUnitTest& T)
{
	TArray<int> Slots = GarrisonInts(0, 0, 0, 0, 0);
	TArray<int> Kinds = GarrisonInts(HOLGarrison::KindSiege, HOLGarrison::KindCavalry, HOLGarrison::KindHero, HOLGarrison::KindMeleeInfantry,
		HOLGarrison::KindRangedInfantry);
	TArray<int> In = HOLGarrison::AdmittedInto(n"TowerReinforced", Slots, Kinds, 0, 0);
	T.AssertEquals(In.Num(), 4, "cavalry, hero, melee and ranged foot enter a Reinforced tower");
	T.AssertTrue(In.Contains(1), "the rider enters by default (D793 Q2 B)");
	T.AssertFalse(In.Contains(0), "siege stays out");
	TArray<int> Riders = HOLGarrison::AdmittedInto(n"TowerWood", GarrisonInts(0, 0, 0, 0), GarrisonInts(HOLGarrison::KindCavalry,
		HOLGarrison::KindSiege, HOLGarrison::KindCavalry, HOLGarrison::KindCavalry), 0, 1);
	T.AssertTrue(Riders.Num() == 2 && Riders[0] == 0 && Riders[1] == 2, "two riders fill a Watch tower holding one; the siege engine takes no place");
	T.AssertTrue(HOLGarrison::KindMayEnter(HOLGarrison::KindCavalry, true, true, true), "Cavalry 1: the rider enters");
	T.AssertFalse(HOLGarrison::KindMayEnter(HOLGarrison::KindSiege, true, true, true), "siege never enters");
	T.AssertFalse(HOLGarrison::KindMayEnter(HOLGarrison::KindHero, false, false, false), "Q2 C: no hero");
	T.AssertTrue(HOLGarrison::KindMayEnter(HOLGarrison::KindRangedInfantry, false, false, false), "Q2 C: ranged foot still enter");
}

// MIG-J-Garrison: a unit's kind from its type, each of the 16 combinations of the four marks the server reads (siege, hero, cavalry, ranged
// attack), in the C++ twin's order; then through the real chain: a mounted shooter is a rider, kept out when the cavalry setting is off.
void Test_GarrisonKindOfEachUnitType(FUnitTest& T)
{
	int Siege = 0;
	int Hero = 0;
	int Cavalry = 0;
	int Ranged = 0;
	int Melee = 0;
	for (int Marks = 0; Marks < 16; ++Marks)
	{
		const int Kind = HOLGarrison::KindOf((Marks & 1) != 0, (Marks & 2) != 0, (Marks & 4) != 0, (Marks & 8) != 0);
		if (Kind == HOLGarrison::KindSiege) ++Siege;
		if (Kind == HOLGarrison::KindHero) ++Hero;
		if (Kind == HOLGarrison::KindCavalry) ++Cavalry;
		if (Kind == HOLGarrison::KindRangedInfantry) ++Ranged;
		if (Kind == HOLGarrison::KindMeleeInfantry) ++Melee;
	}
	T.AssertEquals(Siege, 8, "every siege mark is siege (the Ram, the Mangonel), whatever else");
	T.AssertEquals(Hero, 4, "a hero not siege is a hero, mounted or shooting");
	T.AssertEquals(Cavalry, 2, "a rider neither siege nor hero is cavalry, shooting or not");
	T.AssertEquals(Ranged, 1, "a shooter on foot is ranged infantry");
	T.AssertEquals(Melee, 1, "no mark: melee infantry");
	T.AssertEquals(HOLGarrison::KindOf(false, false, true, true), HOLGarrison::KindCavalry, "a mounted shooter is a rider");
	T.AssertEquals(HOLGarrison::KindOf(false, true, true, false), HOLGarrison::KindHero, "a mounted hero is a hero");
	T.AssertEquals(HOLGarrison::KindOf(true, true, false, true), HOLGarrison::KindSiege, "siege before everything");
	TArray<int> Kinds;
	Kinds.Add(HOLGarrison::KindOf(false, false, true, true));
	Kinds.Add(HOLGarrison::KindOf(false, false, false, true));
	TArray<int> In = HOLGarrison::AdmittedWith(GarrisonInts(0, 0), Kinds, 0, 5, 0, true, true, false);
	T.AssertTrue(In.Num() == 1 && In[0] == 1, "cavalry setting off: the foot archer enters, the mounted one stays out");
}

// F7 lot 1: the places come from the building type table's GarrisonSlots column (Tools/Data/BuildingTypes.csv, FHOLBuildingTypeRow). A row
// with 4 places admits through the same chain as a type name; a row left at 0 (every real row for now: no tower row exists) admits nobody.
void Test_GarrisonPlacesComeFromTheTypeTable(FUnitTest& T)
{
	TArray<int> Slots = GarrisonInts(0, 0, 0, 0, 0, 0);
	TArray<int> Kinds = GarrisonInts(HOLGarrison::KindRangedInfantry, HOLGarrison::KindMeleeInfantry, HOLGarrison::KindRangedInfantry,
		HOLGarrison::KindRangedInfantry, HOLGarrison::KindMeleeInfantry, HOLGarrison::KindRangedInfantry);
	FHOLBuildingTypeRow Tower;
	Tower.GarrisonSlots = 4;
	TArray<int> In = HOLGarrison::AdmittedIntoRow(Tower, Slots, Kinds, 0, 1);
	T.AssertEquals(In.Num(), 3, "a row of 4 places holding 1 takes 3 more");
	T.AssertTrue(In.Num() == 3 && In[0] == 0 && In[2] == 2, "the first three asked enter");
	FHOLBuildingTypeRow Closed;
	T.AssertEquals(HOLGarrison::AdmittedIntoRow(Closed, Slots, Kinds, 0, 0).Num(), 0, "a row with no GarrisonSlots takes nobody");
	FHOLBuildingTypeRow House;
	T.AssertTrue(HOLBuildingType::FindRow(n"House", House), "the building type table has a House row");
	T.AssertEquals(HOLGarrison::AdmittedIntoRow(House, Slots, Kinds, 0, 0).Num(), 0, "the table's House row (0 places) takes nobody");
	T.AssertFalse(HOLBuildingType::FindRow(n"NoSuchBuilding", House), "a missing row is reported, not invented");
}
