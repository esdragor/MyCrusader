// The army rules of the AI brain (IA2.3), pure functions over a summary (IA2.1) and a data struct, tested alone in script
// (HOLAIArmyRules_Test.as): how much a unit type hurts another per second, who wins a one-on-one, what an army is worth against
// the enemy's, which military unit to queue next and from which building. The brain (IA2.4 on) reads the figures from the type
// tables (LoadFigures, UHOLAIOrderLibrary::ReadAIUnitFigures) and asks these rules; the numbers (weights, exposure, caps) are
// provisional, IA4 tunes them on AI-only games.

// A unit type to queue and the building that makes it.
struct FHOLAIComposition
{
	// A choice was made.
	bool bValid = false;

	// The enemy's army was seen and the type counters it; false when the fixed mix decided.
	bool bCounter = false;

	// The unit type to queue.
	FName Type;

	// The building type that produces it (the brain brings it when the slot has none).
	FName Building;

	// The type's score (counter rule: capped matchup per cost; fixed mix: minus the units it has over its share).
	float Score = 0.f;
}

// The numbers of the army rules.
struct FHOLAIArmyData
{
	// The unit types whose figures are known (the enemy types are looked up here; a type not listed has no value) and their figures,
	// parallel arrays. LoadFigures fills the figures from the type tables.
	TArray<FName> Types;
	TArray<FHOLAITypeFigures> Figures;

	// The types the slot can queue, with the building that makes each and its share in the fixed mix (parallel arrays).
	TArray<FName> Candidates;
	TArray<FName> CandidateBuildings;
	TArray<int> CandidateRatios;

	// Weight of each resource in a unit's value (gold is dearer).
	float FoodWeight = 1.f;
	float WoodWeight = 1.f;
	float StoneWeight = 1.f;
	float GoldWeight = 1.5f;

	// Share of its hits a melee unit lands on a shooter (the rest of the shooters stand behind the front line).
	float ShooterExposure = 0.5f;

	// Beyond this, a matchup is overkill: it counts no more (and its inverse no less).
	float MatchupCap = 5.f;

	// The ratio given when the enemy has no army.
	float LargeRatio = 1000.f;

	// Base defence and regrouping (IA2.5), provisional (IA4 tunes them).
	// An enemy soldier this close to a building of the slot (or to its centre) threatens the base (cm).
	float DefenceRadius = 3000.f;

	// A soldier idle farther than this from the assembly point is sent back to it; an enemy this close to the assembly point is
	// fought even when the army is the weaker (cm).
	float AssemblyRadius = 1000.f;

	// The threatening enemies within this of the nearest one count as one group: the defence aims at their centroid (cm).
	float ClusterRadius = 1000.f;

	// A soldier is sent back to the assembly point at most once in this time (s).
	float RegroupEvery = 10.f;

	// The defence orders are sent again at most this often (s), to the soldiers not striking.
	float DefenceEvery = 3.f;

	// The defence ends after this long with no threat (s).
	float DefenceQuiet = 5.f;

	// Below this power ratio of our army over the threatening enemies, the army does not go to meet them (it holds the assembly point).
	float DefenceMinRatio = 0.3f;
}

// What threatens the base now (HOLAIArmy::Threat).
struct FHOLAIThreat
{
	// An enemy soldier stands within the defence radius of a building or of the centre.
	bool bThreatened = false;

	// Where to go: the nearest threatening enemy to the centre, or the centre of the group around it.
	FVector Location;

	// Our power ratio over the threatening enemies alone (PowerRatio on them).
	float Ratio = 0.f;

	// The threatening enemies.
	int Count = 0;
}

// What the defence remembers between two thoughts of a slot (the brain keeps one per slot).
struct FHOLAIDefence
{
	// The army is defending (until the quiet delay has passed with no threat).
	bool bDefending = false;

	// Game time of the last thought that wanted the defence (s).
	float LastWanted = -1000.f;

	// Game time of the last defence order (s).
	float LastOrder = -1000.f;

	// The first order of this defence was sent (the next ones leave out the soldiers that strike).
	bool bOrdered = false;

	// The soldiers sent back to the assembly point, and when (parallel arrays).
	TArray<int> RegroupIds;
	TArray<float> RegroupTimes;
}

namespace HOLAIArmy
{
	// The numbers of the rules with the four types of the prototype; the figures are empty (LoadFigures reads them from the tables).
	FHOLAIArmyData DefaultData()
	{
		FHOLAIArmyData Data;
		Data.Types.Add(n"Spearman");
		Data.Types.Add(n"ManAtArms");
		Data.Types.Add(n"Archer");
		Data.Types.Add(n"Crossbowman");
		Data.Candidates.Add(n"Spearman");
		Data.CandidateBuildings.Add(n"Barracks");
		Data.CandidateRatios.Add(2);
		Data.Candidates.Add(n"ManAtArms");
		Data.CandidateBuildings.Add(n"Barracks");
		Data.CandidateRatios.Add(1);
		Data.Candidates.Add(n"Archer");
		Data.CandidateBuildings.Add(n"ArcheryRange");
		Data.CandidateRatios.Add(2);
		Data.Candidates.Add(n"Crossbowman");
		Data.CandidateBuildings.Add(n"ArcheryRange");
		Data.CandidateRatios.Add(1);
		return Data;
	}

	// The figures of every type of the data, read from the type tables (the row the combat code reads). A type the table does not
	// know keeps empty figures (no health: it counts for nothing).
	void LoadFigures(FHOLAIArmyData& Data)
	{
		Data.Figures.Empty();
		for (int Index = 0; Index < Data.Types.Num(); Index++)
		{
			FHOLAITypeFigures Figures;
			HOLAIOrder::ReadAIUnitFigures(Data.Types[Index], Figures);
			Data.Figures.Add(Figures);
		}
	}

	// The index of a type in the data, -1 when it has no usable figures (unknown, or no health).
	int TypeIndex(const FHOLAIArmyData& Data, FName Type)
	{
		for (int Index = 0; Index < Data.Types.Num() && Index < Data.Figures.Num(); Index++)
		{
			if (Data.Types[Index] == Type && Data.Figures[Index].Health > 0)
			{
				return Index;
			}
		}
		return -1;
	}

	// Damage of one hit of the attacker on the defender, as HOLCombat::GetDamage: the flat bonuses of the categories the defender has,
	// less the defender's armour of the hit's kind (ranged or melee), never under 1. (The hero's reduction and levels are left out.)
	float HitDamage(const FHOLAITypeFigures& Attacker, const FHOLAITypeFigures& Defender)
	{
		float Hit = float(Attacker.Damage);
		for (int Index = 0; Index < Attacker.BonusCategories.Num() && Index < Attacker.BonusDamage.Num(); Index++)
		{
			if (Attacker.BonusDamage[Index] > 0.f && Defender.ArmorCategories.FindIndex(Attacker.BonusCategories[Index]) >= 0)
			{
				Hit += Attacker.BonusDamage[Index];
			}
		}
		if (Attacker.bRanged)
		{
			Hit -= float(Defender.RangedArmor);
		}
		else
		{
			Hit -= float(Defender.MeleeArmor);
		}
		return Math::Max(Hit, 1.f);
	}

	// Damage per second of the attacker on the defender: the hit over the interval (0 without an interval).
	float DamagePerSecond(const FHOLAITypeFigures& Attacker, const FHOLAITypeFigures& Defender)
	{
		if (Attacker.AttackInterval <= 0.f)
		{
			return 0.f;
		}
		return HitDamage(Attacker, Defender) / Attacker.AttackInterval;
	}

	// Damage per second the attacker really lands: a melee unit reaches only ShooterExposure of the hits on a shooter.
	float EffectiveDps(const FHOLAIArmyData& Data, const FHOLAITypeFigures& Attacker, const FHOLAITypeFigures& Defender)
	{
		float Dps = DamagePerSecond(Attacker, Defender);
		if (!Attacker.bRanged && Defender.bRanged)
		{
			Dps = Dps * Data.ShooterExposure;
		}
		return Dps;
	}

	// Health the defender loses before the contact: a shooter that outranges a melee unit fires while it closes in, over the range
	// gained divided by the defender's speed.
	float FreeDamage(const FHOLAITypeFigures& Attacker, const FHOLAITypeFigures& Defender)
	{
		if (!Attacker.bRanged || Defender.bRanged || Attacker.AttackRange <= Defender.AttackRange || Defender.Speed <= 0.f)
		{
			return 0.f;
		}
		return DamagePerSecond(Attacker, Defender) * (Attacker.AttackRange - Defender.AttackRange) / Defender.Speed;
	}

	// How the attacker's type fares against the defender's one on one: (damage per second x health) over the other's, the health
	// reduced by the free shots of the other side. Above 1 the attacker's type wins; the two sides of a pair are inverse.
	float Matchup(const FHOLAIArmyData& Data, const FHOLAITypeFigures& Attacker, const FHOLAITypeFigures& Defender)
	{
		const float Mine = EffectiveDps(Data, Attacker, Defender) * Math::Max(float(Attacker.Health) - FreeDamage(Defender, Attacker), 1.f);
		const float Theirs = EffectiveDps(Data, Defender, Attacker) * Math::Max(float(Defender.Health) - FreeDamage(Attacker, Defender), 1.f);
		if (Theirs <= 0.f)
		{
			if (Mine <= 0.f)
			{
				return 1.f;
			}
			return Data.LargeRatio;
		}
		return Mine / Theirs;
	}

	// A matchup held between 1 / cap and the cap (overkill counts no more).
	float CappedMatchup(const FHOLAIArmyData& Data, float Value)
	{
		return Math::Clamp(Value, 1.f / Data.MatchupCap, Data.MatchupCap);
	}

	// What a unit costs, the resources weighted by the data (gold dearer): the unit's weight in an army.
	float UnitValue(const FHOLAIArmyData& Data, const FHOLAITypeFigures& Figures)
	{
		return float(Figures.CostFood) * Data.FoodWeight + float(Figures.CostWood) * Data.WoodWeight + float(Figures.CostStone) * Data.StoneWeight + float(Figures.CostGold) * Data.GoldWeight;
	}

	// The health of a unit over its type's (1 at most).
	float HealthFraction(const FHOLAITypeFigures& Figures, int Health)
	{
		if (Figures.Health <= 0)
		{
			return 0.f;
		}
		return Math::Clamp(float(Health) / float(Figures.Health), 0.f, 1.f);
	}

	// The value of the units of a list, by type of the data: the sum of unit value x health fraction (a Peasant or a type the data
	// does not know counts for nothing).
	TArray<float> ValueByType(const FHOLAIArmyData& Data, const TArray<FHOLAIUnit>& Units)
	{
		TArray<float> Values;
		for (int Index = 0; Index < Data.Types.Num(); Index++)
		{
			Values.Add(0.f);
		}
		for (int Index = 0; Index < Units.Num(); Index++)
		{
			const int At = TypeIndex(Data, Units[Index].Type);
			if (At >= 0 && Units[Index].Type != n"Peasant")
			{
				Values[At] = Values[At] + UnitValue(Data, Data.Figures[At]) * HealthFraction(Data.Figures[At], Units[Index].Health);
			}
		}
		return Values;
	}

	float SumOf(const TArray<float>& Values)
	{
		float Total = 0.f;
		for (int Index = 0; Index < Values.Num(); Index++)
		{
			Total += Values[Index];
		}
		return Total;
	}

	// The value of the slot's army: unit value x health fraction, summed.
	float ArmyValue(const FHOLAIArmyData& Data, const FHOLAISummary& Summary)
	{
		return SumOf(ValueByType(Data, Summary.Army));
	}

	// The value of the enemy's army (Peasants left out), each type's value divided by how strongly OUR army counters it: the
	// average of our types' matchups against it (capped), weighted by our value. Our army beating it 4 to 1 makes it count a quarter;
	// without an army of ours, the matchup is 1.
	float EnemyValue(const FHOLAIArmyData& Data, const FHOLAISummary& Summary)
	{
		const TArray<float> Ours = ValueByType(Data, Summary.Army);
		const TArray<float> Theirs = ValueByType(Data, Summary.Enemies);
		const float OurTotal = SumOf(Ours);
		float Total = 0.f;
		for (int Enemy = 0; Enemy < Theirs.Num(); Enemy++)
		{
			if (Theirs[Enemy] <= 0.f)
			{
				continue;
			}
			float Counter = 1.f;
			if (OurTotal > 0.f)
			{
				Counter = 0.f;
				for (int Mine = 0; Mine < Ours.Num(); Mine++)
				{
					if (Ours[Mine] > 0.f)
					{
						Counter += Ours[Mine] / OurTotal * CappedMatchup(Data, Matchup(Data, Data.Figures[Mine], Data.Figures[Enemy]));
					}
				}
			}
			Total += Theirs[Enemy] / Counter;
		}
		return Total;
	}

	// Our army's value over the enemy's: above 1 we are ahead. The data's large value when the enemy has no army.
	float PowerRatio(const FHOLAIArmyData& Data, const FHOLAISummary& Summary)
	{
		const float Enemy = EnemyValue(Data, Summary);
		if (Enemy <= 0.f)
		{
			return Data.LargeRatio;
		}
		return Math::Min(ArmyValue(Data, Summary) / Enemy, Data.LargeRatio);
	}

	// The units of a type the slot has or is making (alive in the army and waiting in a queue).
	int CountOfType(const FHOLAISummary& Summary, FName Type)
	{
		int Count = 0;
		for (int Index = 0; Index < Summary.Army.Num(); Index++)
		{
			if (Summary.Army[Index].Type == Type)
			{
				Count++;
			}
		}
		for (int Index = 0; Index < Summary.Buildings.Num(); Index++)
		{
			for (int InQueue = 0; InQueue < Summary.Buildings[Index].Queue.Num(); InQueue++)
			{
				if (Summary.Buildings[Index].Queue[InQueue] == Type)
				{
					Count++;
				}
			}
		}
		return Count;
	}

	// The building types that make the candidates, each once (in the order of the candidates).
	TArray<FName> BuildingsOf(const FHOLAIArmyData& Data)
	{
		TArray<FName> Found;
		for (int Index = 0; Index < Data.CandidateBuildings.Num(); Index++)
		{
			if (!Found.Contains(Data.CandidateBuildings[Index]))
			{
				Found.Add(Data.CandidateBuildings[Index]);
			}
		}
		return Found;
	}

	// The figures of a type of the data (empty when it has none).
	FHOLAITypeFigures FiguresOf(const FHOLAIArmyData& Data, FName Type)
	{
		const int At = TypeIndex(Data, Type);
		FHOLAITypeFigures Figures;
		if (At >= 0)
		{
			Figures = Data.Figures[At];
		}
		return Figures;
	}

	// The unit type to queue next. With an enemy army in view: the candidate whose capped matchup against the enemy's types, weighted
	// by each type's value in the enemy army, is the highest per unit of cost. Without: the candidate the most below its share of
	// the fixed mix (units alive + queued over the ratio; a tie: the first of the list).
	FHOLAIComposition ChooseComposition(const FHOLAISummary& Summary, const FHOLAIArmyData& Data)
	{
		return ChooseComposition(Summary, Data, NAME_None);
	}

	// The same, among the candidates the building OnlyBuilding makes (NAME_None: all of them; the brain asks per military building).
	FHOLAIComposition ChooseComposition(const FHOLAISummary& Summary, const FHOLAIArmyData& Data, FName OnlyBuilding)
	{
		FHOLAIComposition Choice;
		const TArray<float> Theirs = ValueByType(Data, Summary.Enemies);
		const float TheirTotal = SumOf(Theirs);
		int Best = -1;
		float BestScore = 0.f;
		for (int Index = 0; Index < Data.Candidates.Num(); Index++)
		{
			if (OnlyBuilding != NAME_None && (Index >= Data.CandidateBuildings.Num() || Data.CandidateBuildings[Index] != OnlyBuilding))
			{
				continue;
			}
			float Score = 0.f;
			if (TheirTotal > 0.f)
			{
				const int At = TypeIndex(Data, Data.Candidates[Index]);
				if (At < 0 || UnitValue(Data, Data.Figures[At]) <= 0.f)
				{
					continue;
				}
				for (int Enemy = 0; Enemy < Theirs.Num(); Enemy++)
				{
					if (Theirs[Enemy] > 0.f)
					{
						Score += Theirs[Enemy] / TheirTotal * CappedMatchup(Data, Matchup(Data, Data.Figures[At], Data.Figures[Enemy]));
					}
				}
				Score = Score / UnitValue(Data, Data.Figures[At]);
			}
			else
			{
				if (Index >= Data.CandidateRatios.Num() || Data.CandidateRatios[Index] <= 0)
				{
					continue;
				}
				Score = -float(CountOfType(Summary, Data.Candidates[Index])) / float(Data.CandidateRatios[Index]);
			}
			if (Best < 0 || Score > BestScore)
			{
				Best = Index;
				BestScore = Score;
			}
		}
		if (Best < 0)
		{
			return Choice;
		}
		Choice.bValid = true;
		Choice.bCounter = TheirTotal > 0.f;
		Choice.Type = Data.Candidates[Best];
		if (Best < Data.CandidateBuildings.Num())
		{
			Choice.Building = Data.CandidateBuildings[Best];
		}
		Choice.Score = BestScore;
		return Choice;
	}

	// ---------------------------------------------------------------------------------------------------------------------------
	// Regrouping and base defence (IA2.5). The brain keeps one FHOLAIDefence per slot and asks, each thought: Threat (who is near the
	// base), ShouldDefend (whether the army goes to meet them), UpdateDefence (the state, ended by a quiet delay), then DefenceOrders
	// (while defending) or RegroupOrders (at home). Peasants are not ordered here (no flight yet); only enemy soldiers threaten.

	// The distance on the ground.
	float FlatDistance(FVector A, FVector B)
	{
		FVector Line = A - B;
		Line.Z = 0;
		return Line.Size();
	}

	// An enemy that threatens a base: any unit that is not a Peasant (a scout or a builder is no attack).
	bool IsThreatening(const FHOLAIUnit& Enemy)
	{
		return Enemy.Type != n"Peasant" && Enemy.Health > 0;
	}

	// The enemy soldiers within the defence radius of a building of the slot or of its centre; their place is that of the nearest to the
	// centre, or the centre of the group (the threatening enemies within ClusterRadius of it); the ratio is our army's power over them alone.
	FHOLAIThreat Threat(const FHOLAISummary& Summary, const FHOLAIArmyData& Data)
	{
		FHOLAIThreat Result;
		TArray<FVector> Guarded;
		if (Summary.bHasCentre)
		{
			Guarded.Add(Summary.CentreLocation);
		}
		for (int Index = 0; Index < Summary.Buildings.Num(); Index++)
		{
			Guarded.Add(Summary.Buildings[Index].Location);
		}
		TArray<FHOLAIUnit> Near;
		for (int Index = 0; Index < Summary.Enemies.Num(); Index++)
		{
			const FHOLAIUnit Enemy = Summary.Enemies[Index];
			if (!IsThreatening(Enemy))
			{
				continue;
			}
			for (int Place = 0; Place < Guarded.Num(); Place++)
			{
				if (FlatDistance(Enemy.Location, Guarded[Place]) <= Data.DefenceRadius)
				{
					Near.Add(Enemy);
					break;
				}
			}
		}
		if (Near.Num() == 0)
		{
			return Result;
		}
		FVector From = Summary.CentreLocation;
		int Nearest = 0;
		for (int Index = 1; Index < Near.Num(); Index++)
		{
			if (FlatDistance(Near[Index].Location, From) < FlatDistance(Near[Nearest].Location, From))
			{
				Nearest = Index;
			}
		}
		FVector Sum = FVector(0, 0, 0);
		int InGroup = 0;
		for (int Index = 0; Index < Near.Num(); Index++)
		{
			if (FlatDistance(Near[Index].Location, Near[Nearest].Location) <= Data.ClusterRadius)
			{
				Sum += Near[Index].Location;
				InGroup++;
			}
		}
		FHOLAISummary Subset;
		Subset.Army = Summary.Army;
		Subset.Enemies = Near;
		Result.bThreatened = true;
		Result.Location = Sum / float(InGroup);
		Result.Ratio = PowerRatio(Data, Subset);
		Result.Count = Near.Num();
		return Result;
	}

	// Whether the army goes to meet the threat: a threat, and either the army is not far behind (ratio at least DefenceMinRatio) or an
	// enemy soldier has come to the assembly point (then it fights there, being cornered).
	bool ShouldDefend(const FHOLAISummary& Summary, const FHOLAIArmyData& Data, const FHOLAIThreat& Threat, FVector AssemblyPoint)
	{
		if (!Threat.bThreatened)
		{
			return false;
		}
		if (Threat.Ratio >= Data.DefenceMinRatio)
		{
			return true;
		}
		for (int Index = 0; Index < Summary.Enemies.Num(); Index++)
		{
			if (IsThreatening(Summary.Enemies[Index]) && FlatDistance(Summary.Enemies[Index].Location, AssemblyPoint) <= Data.AssemblyRadius)
			{
				return true;
			}
		}
		return false;
	}

	// The defence state: it begins as soon as the defence is wanted and ends once DefenceQuiet seconds have passed since it last was.
	void UpdateDefence(FHOLAIDefence& State, bool bWanted, float Now, const FHOLAIArmyData& Data)
	{
		if (bWanted)
		{
			if (!State.bDefending)
			{
				State.bOrdered = false;
			}
			State.bDefending = true;
			State.LastWanted = Now;
		}
		else if (State.bDefending && Now - State.LastWanted >= Data.DefenceQuiet)
		{
			State.bDefending = false;
		}
	}

	// The soldiers to send against the threat now: none without a threat or before DefenceEvery has passed since the last order; at the
	// first order of a defence every soldier (those assembling and those striking too), after it only those not striking.
	TArray<int> DefenceOrders(FHOLAIDefence& State, const FHOLAISummary& Summary, const FHOLAIArmyData& Data, const FHOLAIThreat& Threat, float Now)
	{
		TArray<int> Ids;
		if (!Threat.bThreatened || Now - State.LastOrder < Data.DefenceEvery)
		{
			return Ids;
		}
		for (int Index = 0; Index < Summary.Army.Num(); Index++)
		{
			if (!State.bOrdered || Summary.Army[Index].TargetId < 0)
			{
				Ids.Add(Summary.Army[Index].Id);
			}
		}
		if (Ids.Num() > 0)
		{
			State.LastOrder = Now;
			State.bOrdered = true;
		}
		return Ids;
	}

	// The soldiers to send back to the assembly point: idle (not moving, not striking), farther from it than AssemblyRadius, and not sent
	// in the last RegroupEvery seconds. Marks them as sent.
	TArray<int> RegroupOrders(FHOLAIDefence& State, const FHOLAISummary& Summary, const FHOLAIArmyData& Data, FVector AssemblyPoint, float Now)
	{
		for (int Index = State.RegroupIds.Num() - 1; Index >= 0; Index--)
		{
			if (Now - State.RegroupTimes[Index] >= Data.RegroupEvery)
			{
				State.RegroupIds.RemoveAt(Index);
				State.RegroupTimes.RemoveAt(Index);
			}
		}
		TArray<int> Ids;
		for (int Index = 0; Index < Summary.Army.Num(); Index++)
		{
			const FHOLAIUnit Soldier = Summary.Army[Index];
			if (Soldier.bIdle && Soldier.TargetId < 0 && FlatDistance(Soldier.Location, AssemblyPoint) > Data.AssemblyRadius && !State.RegroupIds.Contains(Soldier.Id))
			{
				Ids.Add(Soldier.Id);
				State.RegroupIds.Add(Soldier.Id);
				State.RegroupTimes.Add(Now);
			}
		}
		return Ids;
	}
}
