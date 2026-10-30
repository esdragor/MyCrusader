// The army and the enemies of the AI summary, seen from script (IA2.1): a hand-made summary, read and written like the
// fields the brain's rules will use (IA2.3 and after). The C++ reading is tested in HeroesOfLiberty.AI.Summary.Army*, Enemies*.

FHOLAIUnit MakeArmyUnit(int Id, int OwnerSlot, FName Type, int Health, bool bIdle)
{
	FHOLAIUnit Unit;
	Unit.Id = Id;
	Unit.OwnerSlot = OwnerSlot;
	Unit.Type = Type;
	Unit.Health = Health;
	Unit.bIdle = bIdle;
	Unit.Location = FVector(100.0, 200.0, 0.0);
	return Unit;
}

// An empty summary: no army, no enemy, no enemy centre, an unset target.
void Test_ArmyEmptyByDefault(FUnitTest& T)
{
	FHOLAISummary Summary;
	T.AssertEquals(0, Summary.Army.Num(), "no soldier by default");
	T.AssertEquals(0, Summary.Enemies.Num(), "no enemy by default");
	T.AssertFalse(Summary.bHasEnemyCentre, "no enemy centre by default");
	FHOLAIUnit Unit;
	T.AssertEquals(-1, Unit.TargetId, "no target by default");
	T.AssertEquals(-1, Unit.Id, "no identifier by default");
}

// The army and the enemies are readable and writable from script, with their fields.
void Test_ArmyAndEnemiesReadable(FUnitTest& T)
{
	FHOLAISummary Summary;
	Summary.Army.Add(MakeArmyUnit(4, 0, n"Spearman", 75, true));
	Summary.Army.Add(MakeArmyUnit(9, 0, n"Archer", 40, false));
	Summary.Enemies.Add(MakeArmyUnit(30, 1, n"ManAtArms", 120, true));
	Summary.bHasEnemyCentre = true;
	Summary.EnemyCentreLocation = FVector(-10000.0, 0.0, 0.0);

	T.AssertEquals(2, Summary.Army.Num(), "two soldiers");
	T.AssertEquals(1, Summary.Enemies.Num(), "one enemy");
	T.AssertTrue(Summary.Army[1].Type == n"Archer", "the second soldier's type");
	T.AssertEquals(40, Summary.Army[1].Health, "the second soldier's health");
	T.AssertFalse(Summary.Army[1].bIdle, "the second soldier is busy");
	T.AssertEquals(1, Summary.Enemies[0].OwnerSlot, "the enemy's slot");
	T.AssertTrue(Summary.Enemies[0].Type == n"ManAtArms", "the enemy's type");
	T.AssertTrue(Summary.bHasEnemyCentre, "an enemy centre");
	T.AssertTrue(Math::Abs(Summary.EnemyCentreLocation.X + 10000.0) < 0.01, "the enemy centre's place");
}
