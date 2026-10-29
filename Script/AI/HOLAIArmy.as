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
}
