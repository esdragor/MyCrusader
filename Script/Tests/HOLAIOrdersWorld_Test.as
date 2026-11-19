// Script port of the "orders only" cases of HOLAIOrdersTests.cpp (D853 point 4, SAURON-1119cf).
//   AI.Orders.CombatFigures -> Test_AIOrdersCombatFigures.
// Same entries and same expected values as the C++ test (UHOLAIOrderLibrary::ReadAIUnitFigures / ReadAIBuildingFigures, the real unit and building tables).
// The C++ test stays until the main session has seen this one red then green.
//
// Not ported, and why: Gather, Cap, AttackMove, AttackBuilding, AttackUnit, NonWorkers all call PlaceUnits (UHOLUnitSubsystem::RequestUnits +
// ServeBirthRequests = a unit born by Mass), so they stay in C++ (D854). Rule is already script (Test_PlayerOrdersIsAccepted).
// "test world ready" of the C++: its world was only a holder of UHOLUnitSubsystem; the script reads the table through its own world context
// (as HOLAIFigures_Test.as), the Spearman being known asserts that the table is reachable.

namespace AIOrdersWorldTest
{
	// The C++ IndexOf: the position of a name in a list, -1 if absent.
	int IndexOf(const TArray<FName>& Names, FName Name)
	{
		return Names.FindIndex(Name);
	}
}

// The combat figures of a unit type, read from the same row the combat code reads: the numbers of the table, the bonuses as the flat damage a hit adds
// against a category.
void Test_AIOrdersCombatFigures(FUnitTest& T)
{
	FHOLAITypeFigures Spear;
	FHOLAITypeFigures Heavy;
	FHOLAITypeFigures Bow;
	FHOLAITypeFigures Bolt;
	FHOLAITypeFigures Worker;
	FHOLAITypeFigures Nothing;
	bool bSpear = HOLAIOrder::ReadAIUnitFigures(n"Spearman", Spear);
	bool bHeavy = HOLAIOrder::ReadAIUnitFigures(n"ManAtArms", Heavy);
	bool bBow = HOLAIOrder::ReadAIUnitFigures(n"Archer", Bow);
	bool bBolt = HOLAIOrder::ReadAIUnitFigures(n"Crossbowman", Bolt);
	bool bWorker = HOLAIOrder::ReadAIUnitFigures(n"Peasant", Worker);
	T.AssertTrue(bSpear, "the Spearman is known");
	T.AssertTrue(bHeavy, "the Man-at-arms is known");
	T.AssertTrue(bBow, "the Archer is known");
	T.AssertTrue(bBolt, "the Crossbowman is known");
	T.AssertTrue(bWorker, "the Peasant is known");
	if (!(bSpear && bHeavy && bBow && bBolt && bWorker))
		return;
	T.AssertFalse(HOLAIOrder::ReadAIUnitFigures(n"NoSuchUnit", Nothing), "unknown unit");

	T.AssertTrue(Spear.Health > 0 && Spear.Damage > 0 && Spear.AttackRange > 0.0 && Spear.AttackRange < 500.0, "Spearman: health > 0, damage > 0, a short reach");
	T.AssertTrue(Spear.AttackInterval > 0.0, "Spearman: a positive interval");
	T.AssertFalse(Spear.bRanged, "Spearman: melee");
	T.AssertTrue(AIOrdersWorldTest::IndexOf(Spear.ArmorCategories, n"LightInfantry") != -1, "Spearman: LightInfantry");
	int VsCavalry = AIOrdersWorldTest::IndexOf(Spear.BonusCategories, n"Cavalry");
	T.AssertTrue(VsCavalry != -1 && Spear.BonusDamage.IsValidIndex(VsCavalry) && Spear.BonusDamage[VsCavalry] > 0.0, "Spearman: a bonus against Cavalry");
	T.AssertEquals(Spear.BonusCategories.Num(), Spear.BonusDamage.Num(), "bonus arrays are parallel");

	T.AssertTrue(AIOrdersWorldTest::IndexOf(Heavy.ArmorCategories, n"Heavy") != -1, "Man-at-arms: Heavy category");
	T.AssertTrue(Heavy.Health > Spear.Health, "Man-at-arms: more health than the Spearman");
	T.AssertTrue(Heavy.MeleeArmor > Spear.MeleeArmor, "Man-at-arms: melee armour");

	T.AssertTrue(Bow.bRanged, "Archer: ranged");
	T.AssertTrue(Bow.AttackRange > Spear.AttackRange, "Archer: a longer reach than the Spearman");
	T.AssertTrue(AIOrdersWorldTest::IndexOf(Bow.ArmorCategories, n"Heavy") == -1, "Archer: not Heavy");

	T.AssertTrue(Bolt.bRanged, "Crossbowman: ranged");
	int VsHeavy = AIOrdersWorldTest::IndexOf(Bolt.BonusCategories, n"Heavy");
	T.AssertTrue(VsHeavy != -1 && Bolt.BonusDamage.IsValidIndex(VsHeavy) && Bolt.BonusDamage[VsHeavy] > 0.0, "Crossbowman: a bonus against Heavy");

	T.AssertEquals(50, Worker.CostFood, "Peasant: the cost as before");
	T.AssertTrue(Worker.CarryCapacity > 0 && Worker.GatherFood > 0.0, "Peasant: the gathering as before");
	T.AssertEquals(0, Worker.BonusCategories.Num(), "Peasant: no bonus");
	T.AssertTrue(AIOrdersWorldTest::IndexOf(Worker.ArmorCategories, n"Peasant") != -1, "Peasant: the Peasant category");

	FHOLAITypeFigures House;
	T.AssertTrue(HOLAIOrder::ReadAIBuildingFigures(n"House", House), "a building: its figures");
	T.AssertTrue(House.Health == 0 && House.Damage == 0 && House.ArmorCategories.Num() == 0 && House.BonusCategories.Num() == 0, "a building: no combat figures");
}
