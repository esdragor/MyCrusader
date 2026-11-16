// L4 step 6: the AI plans the Damned Warrior (Legions only) and casts its necrotic rage in a melee. The rules are the real ones (HOLAIArmy::ChooseComposition,
// HOLAIRage::Casters); only the situation is built by hand (the Legions AI does not play a whole game yet, L12).

int WarriorPicks(FHOLAIArmyData Data, int Picks)
{
	FHOLAISummary Summary;
	Summary.Tier = 3;
	int Made = 0;
	for (int Pick = 0; Pick < Picks; Pick++)
	{
		const FHOLAIComposition Choice = HOLAIArmy::ChooseComposition(Summary, Data);
		if (!Choice.bValid)
		{
			break;
		}
		FHOLAIUnit Unit;
		Unit.Id = 100 + Pick;
		Unit.Type = Choice.Type;
		Unit.Health = 60;
		Unit.bIdle = true;
		Summary.Army.Add(Unit);
		if (Choice.Type == n"DamnedWarrior")
		{
			Made++;
		}
	}
	return Made;
}

void Test_RageLegionsAIPlansTheDamnedWarrior(FUnitTest& T)
{
	FHOLAIArmyData Legions = HOLAIArmy::DefaultData();
	Legions.Faction = n"Legions";
	T.AssertTrue(WarriorPicks(Legions, 32) >= 1, "a Legions AI makes Damned Warriors in its fixed mix");
	T.AssertEquals(0, WarriorPicks(HOLAIArmy::DefaultData(), 32), "a Dawn AI never plans one (the server refuses it)");
}

FHOLAISummary WarriorInAMelee(float EnemyDistance, FName EnemyType, int Health)
{
	FHOLAISummary Summary;
	FHOLAIUnit Warrior;
	Warrior.Id = 7;
	Warrior.Type = n"DamnedWarrior";
	Warrior.Health = Health;
	Warrior.Location = FVector(1000.f, 1000.f, 0.f);
	Summary.Army.Add(Warrior);
	FHOLAIUnit Enemy;
	Enemy.Id = 50;
	Enemy.Type = EnemyType;
	Enemy.Health = 80;
	Enemy.Location = FVector(1000.f + EnemyDistance, 1000.f, 0.f);
	Summary.Enemies.Add(Enemy);
	return Summary;
}

void Test_RageCastsInAMeleeOnly(FUnitTest& T)
{
	FHOLAIRageData Data = HOLAIRage::DefaultData();
	Data.MaxHealth = 100;
	FHOLAIRageMemory Memory;
	T.AssertEquals(1, HOLAIRage::Casters(WarriorInAMelee(300.f, n"Spearman", 100), Data, Memory, 10.f).Num(), "an enemy spearman at 3 m: the warrior casts");
	T.AssertEquals(0, HOLAIRage::Casters(WarriorInAMelee(5000.f, n"Spearman", 100), Data, Memory, 10.f).Num(), "an enemy 50 m away: no cast");
	T.AssertEquals(0, HOLAIRage::Casters(WarriorInAMelee(300.f, n"Archer", 100), Data, Memory, 10.f).Num(), "a shooter near is no melee: no cast");
	T.AssertEquals(0, HOLAIRage::Casters(WarriorInAMelee(300.f, n"Peasant", 100), Data, Memory, 10.f).Num(), "a Peasant near is no melee: no cast");
}

void Test_RageHealthAndCooldownGuards(FUnitTest& T)
{
	FHOLAIRageData Data = HOLAIRage::DefaultData();
	Data.MaxHealth = 100;
	FHOLAIRageMemory Memory;
	T.AssertEquals(0, HOLAIRage::Casters(WarriorInAMelee(300.f, n"Spearman", 55), Data, Memory, 10.f).Num(), "under 60 percent of its health: no cast");
	T.AssertEquals(0, HOLAIRage::Casters(WarriorInAMelee(300.f, n"Spearman", 25), Data, Memory, 10.f).Num(), "under the refusal floor: no cast");
	HOLAIRage::Remember(Memory, 7, 10.f);
	T.AssertEquals(0, HOLAIRage::Casters(WarriorInAMelee(300.f, n"Spearman", 100), Data, Memory, 20.f).Num(), "cast 10 s ago: the cooldown holds");
	T.AssertEquals(1, HOLAIRage::Casters(WarriorInAMelee(300.f, n"Spearman", 100), Data, Memory, 51.f).Num(), "cast 41 s ago: it casts again");
	Data.MaxHealth = 0;
	T.AssertEquals(0, HOLAIRage::Casters(WarriorInAMelee(300.f, n"Spearman", 100), Data, FHOLAIRageMemory(), 10.f).Num(), "unknown full health: never casts");
}

// D836 (2): the AI's rage numbers are the C++ rule's, read through HOLUnitAbilityRead (no copy): the defaults are the library's, and the real
// chain refuses under the floor and holds the cool-down at the library's value.
void Test_RageAIReadsTheLibrary(FUnitTest& T)
{
	FHOLAIRageData Data = HOLAIRage::DefaultData();
	const int MinHp = HOLUnitAbilityRead::GetRageMinHp();
	const int CooldownMs = HOLUnitAbilityRead::GetRageCooldownMs();
	T.AssertEquals(MinHp, Data.MinHealth, "the floor is the library's");
	T.AssertEquals(CooldownMs, int(Data.RecastEvery * 1000.f + 0.5f), "the recast delay is the library's cool-down");
	Data.MaxHealth = 40;
	FHOLAIRageMemory Memory;
	T.AssertEquals(1, HOLAIRage::Casters(WarriorInAMelee(300.f, n"Spearman", MinHp), Data, Memory, 10.f).Num(), "at the library's floor: casts");
	T.AssertEquals(0, HOLAIRage::Casters(WarriorInAMelee(300.f, n"Spearman", MinHp - 1), Data, Memory, 10.f).Num(), "one under the library's floor: no cast");
	HOLAIRage::Remember(Memory, 7, 10.f);
	const float CooldownSeconds = float(CooldownMs) / 1000.f;
	T.AssertEquals(0, HOLAIRage::Casters(WarriorInAMelee(300.f, n"Spearman", MinHp), Data, Memory, 10.f + CooldownSeconds - 1.f).Num(), "one second before the library's cool-down ends: no cast");
	T.AssertEquals(1, HOLAIRage::Casters(WarriorInAMelee(300.f, n"Spearman", MinHp), Data, Memory, 10.f + CooldownSeconds).Num(), "at the end of the library's cool-down: casts");
}