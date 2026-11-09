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

	// An enemy soldier this close to a building is taken to attack it (cm): the army then meets it from a lower ratio, the base being hit.
	float AttackedRadius = 800.f;
	float DefenceMinRatioAttacked = 0.05f;
	// The main centre's half width (cm): its footprint is 1600 cm (BuildingTypes.csv), so a soldier hitting its wall stands ~800 cm from the
	// centre's point, past AttackedRadius. Added to AttackedRadius for the centre only (found in a real game, 2026-10-31: the forum under
	// attack was not seen as attacked). Set to 0 first (red test), then 800.
	float ForumHalfSize = 800.f;

	// Peasants (IA2.6f): one within this of an enemy soldier runs to the centre (cm), ordered again at most this often (s).
	float FleeRadius = 1500.f;
	float FleeEvery = 4.f;

	// A Peasant strikes an enemy soldier only when the duel is won with this margin (IA4, D572 confirmed, option A): the one-on-one
	// matchup (Matchup, the building block of the power ratio, D553) at the units' REMAINING health is at least StrikeMargin. A Spearman
	// down to 5 health is finished off (matchup ~1.9), a sound soldier never (well under 1). The Peasant looks for such a soldier within
	// FleeRadius; ordered again at most every StrikeEvery seconds. Provisional: tuned on AI-only games.
	float StrikeMargin = 1.5f;
	float StrikeEvery = 4.f;
	// D584 (user's answer B): a Peasant finishes the wounded off only when it is ISOLATED: no other enemy soldier that is itself sound (one whose
	// duel the Peasant would not win with the margin) stands within this of the wounded one (cm, about 5 m). Stragglers and fugitives, not the
	// wounded in the middle of their army. Provisional, a setting.
	float StrikeIsolation = 500.f;

	// The Peasant's own figures (LoadFigures reads the table); empty (no health) = a Peasant never strikes.
	FHOLAITypeFigures PeasantFigures;

	// Attack wave and retreat (IA2.6), provisional (IA4 'Normal' curve).
	// No wave before this game time when enemy soldiers have been seen (s); and when none has been seen (s).
	float AttackMinSeconds = 420.f;
	float AttackMinSecondsUnseen = 300.f;

	// A wave needs at least this many soldiers.
	int AttackMinSoldiers = 8;

	// Our power ratio over the enemy soldiers seen needed to launch a wave (the margin), and the ratio under which the wave falls back
	// (lower than the launch ratio: hysteresis).
	float AttackLaunchRatio = 1.5f;
	float AttackRetreatRatio = 0.6f;

	// The wave falls back when it has lost more than this share of its size.
	float AttackLossShare = 0.6f;

	// The enemy soldiers within this of the wave's centroid are the ones the wave fights (cm).
	float AttackRetreatRadius = 3000.f;

	// No new wave within this long after a launch or the end of a wave (s); twice as long after a retreat.
	float AttackCooldown = 90.f;

	// The wave's idle soldiers are ordered again at most this often (s).
	float AttackReissue = 1.f;
	// D751: how far (cm) a free soldier of the wave may walk ahead of its rearmost soldier before it waits for the rest (0: no wait).
	float WaveSpread = 1200.f;

	// A retreat ends when most of the wave has reached the assembly point, or after this long (s).
	float AttackRetreatMax = 40.f;

	// A defence against at least this many enemy soldiers near the base calls the wave back.
	int AttackRecallCount = 3;

	// Helping a teammate (IA4, D571): the army goes to hold a teammate's fallen forum when its power over the enemy soldiers within
	// AllyHelpRadius of the ruin (cm) is at least AllyHelpMinRatio, the rule of its own ruin: on the spot, at least even.
	float AllyHelpMinRatio = 1.f;
	float AllyHelpRadius = 3000.f;

	// The weighing of the lives by level (IA4, D579, D329): Normal's feint, Hard's loss and snipe, Evolving's curve (HOLAILives.as).
	FHOLAILivesData Lives;
}

// Where the attack wave is (HOLAIArmy::UpdateWave).
enum EHOLAIWavePhase
{
	Home,
	Out,
	Back
}

// What the attack wave remembers between two thoughts of a slot (the brain keeps one per slot).
struct FHOLAIWave
{
	EHOLAIWavePhase Phase = EHOLAIWavePhase::Home;

	// The soldiers launched with the wave, and how many there were.
	TArray<int> Ids;
	int LaunchedSize = 0;

	// No wave launches before this game time (s).
	float NextLaunch = 0.f;

	// Game time the phase began (s), and of the last order given to the wave (s).
	float PhaseStart = 0.f;
	float LastOrder = -1000.f;

	// Hard (D579): the brain asks for a launch now whatever the ratio and the time (the attacker's home is punished); and the wave is that one
	// (it is not called back by the enemies at home: the army has gone on purpose).
	bool bForce = false;
	bool bPunishing = false;
}

// What the wave does now (HOLAIArmy::UpdateWave): the orders to send and what changed, for the brain's orders and log.
struct FHOLAIWaveStep
{
	bool bLaunched = false;
	bool bRetreat = false;
	bool bOver = false;
	bool bHome = false;

	// The soldiers to attack-move to AttackTarget, and the soldiers to move (not attack-move) to the assembly point.
	TArray<int> AttackIds;
	FVector AttackTarget;
	TArray<int> MoveIds;

	// D751: the free soldiers too far ahead of the wave's rear, each to stand where it is (HoldPoints[i] is the place of HoldIds[i]).
	TArray<int> HoldIds;
	TArray<FVector> HoldPoints;

	// The ratio that decided (launch ratio or the wave's own ratio) and the wave soldiers alive.
	float Ratio = 0.f;
	int Left = 0;
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

	// A threatening enemy stands within AttackedRadius of a building or of the centre: the base is being hit (IA2.6e).
	bool bAttackingBuilding = false;
}

// What the Peasants' flight remembers between two thoughts of a slot: when each Peasant was last sent running (IA2.6f).
struct FHOLAIFlight
{
	TMap<int, float> LastOrder;

	// When each Peasant was last ordered to strike a soldier (IA4, D572).
	TMap<int, float> LastStrike;
}

// A Peasant ordered to finish an enemy soldier off (IA4, D572).
struct FHOLAIStrike
{
	int PeasantId = -1;
	int TargetId = -1;
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
		// C1 (D672, NOT COMPILED): the cavalry is a type whose value and matchups count (own and enemy) in the power ratio; it is not a candidate of the
		// composition (its Stable is built and filled by HOLAICavalry, per level), and a table without the row leaves it with no health: it counts for nothing.
		Data.Types.Add(n"LightCavalry");
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
		HOLAIOrder::ReadAIUnitFigures(n"Peasant", Data.PeasantFigures);
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
			if (HOLUnitTier::MinTierOf(Data.Candidates[Index].ToString()) > int(Summary.Tier))
			{
				// G1 (D711): a unit the tier has not opened is not a candidate (the server refuses it).
				continue;
			}
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
		TArray<float> Reach;
		if (Summary.bHasCentre)
		{
			Guarded.Add(Summary.CentreLocation);
			Reach.Add(Data.AttackedRadius + Data.ForumHalfSize);
		}
		// IA3.4: in Regicide the King's place is guarded too (the army comes to the soldiers that near him).
		if (Summary.bRegicide && Summary.bHasKing)
		{
			Guarded.Add(Summary.King.Location);
			Reach.Add(Data.AttackedRadius);
		}
		for (int Index = 0; Index < Summary.Buildings.Num(); Index++)
		{
			Guarded.Add(Summary.Buildings[Index].Location);
			Reach.Add(Data.AttackedRadius);
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
		for (int Index = 0; Index < Near.Num() && !Result.bAttackingBuilding; Index++)
		{
			for (int Place = 0; Place < Guarded.Num(); Place++)
			{
				if (FlatDistance(Near[Index].Location, Guarded[Place]) <= Reach[Place])
				{
					Result.bAttackingBuilding = true;
					break;
				}
			}
		}
		Result.bThreatened = true;
		Result.Location = Sum / float(InGroup);
		Result.Ratio = PowerRatio(Data, Subset);
		Result.Count = Near.Num();
		return Result;
	}

	// How a Peasant fares one on one against an enemy soldier, at the health each has NOW (IA4, D572): the matchup of the figures
	// (damage per second x health over the other's), the Peasant's health and the soldier's remaining health in place of the maximum.
	// 0 for a Peasant without figures or a type the data does not know.
	float DuelRatio(const FHOLAIArmyData& Data, const FHOLAIUnit& Peasant, const FHOLAIUnit& Enemy)
	{
		const int At = TypeIndex(Data, Enemy.Type);
		if (At < 0 || Data.PeasantFigures.Health <= 0 || Peasant.Health <= 0 || Enemy.Health <= 0)
		{
			return 0.f;
		}
		FHOLAITypeFigures Mine = Data.PeasantFigures;
		Mine.Health = Math::Min(Peasant.Health, Mine.Health);
		FHOLAITypeFigures Theirs = Data.Figures[At];
		Theirs.Health = Math::Min(Enemy.Health, Theirs.Health);
		return Matchup(Data, Mine, Theirs);
	}

	// D584: the wounded soldier stands alone when no OTHER enemy soldier that is sound for this Peasant (its duel under the margin) is within
	// StrikeIsolation of it. Another wounded one (that the Peasant would also win against) is no escort.
	bool IsIsolated(const FHOLAISummary& Summary, const FHOLAIArmyData& Data, const FHOLAIUnit& Peasant, const FHOLAIUnit& Wounded)
	{
		for (int Index = 0; Index < Summary.Enemies.Num(); Index++)
		{
			const FHOLAIUnit Other = Summary.Enemies[Index];
			if (Other.Id == Wounded.Id || !IsThreatening(Other))
			{
				continue;
			}
			if (FlatDistance(Other.Location, Wounded.Location) <= Data.StrikeIsolation && DuelRatio(Data, Peasant, Other) < Data.StrikeMargin)
			{
				return false;
			}
		}
		return true;
	}

	// The soldier a Peasant may finish off: the nearest enemy soldier within FleeRadius whose duel is won with the margin and who is isolated (D584); -1 for none.
	int StrikeTargetFor(const FHOLAISummary& Summary, const FHOLAIArmyData& Data, const FHOLAIUnit& Peasant)
	{
		int Best = -1;
		float BestDistance = 0.f;
		for (int Index = 0; Index < Summary.Enemies.Num(); Index++)
		{
			const FHOLAIUnit Enemy = Summary.Enemies[Index];
			if (!IsThreatening(Enemy))
			{
				continue;
			}
			const float Distance = FlatDistance(Enemy.Location, Peasant.Location);
			if (Distance > Data.FleeRadius || DuelRatio(Data, Peasant, Enemy) < Data.StrikeMargin)
			{
				continue;
			}
			if ((Best < 0 || Distance < BestDistance) && IsIsolated(Summary, Data, Peasant, Enemy))
			{
				Best = Enemy.Id;
				BestDistance = Distance;
			}
		}
		return Best;
	}

	// The Peasants to send to finish a soldier off now (IA4, D572), not ordered in the last StrikeEvery seconds.
	TArray<FHOLAIStrike> StrikeOrders(FHOLAIFlight& State, const FHOLAISummary& Summary, const FHOLAIArmyData& Data, float Now)
	{
		TArray<FHOLAIStrike> Strikes;
		for (int Each = 0; Each < Summary.Peasants.Num(); Each++)
		{
			const FHOLAIUnit Peasant = Summary.Peasants[Each];
			float Last = -1000.f;
			State.LastStrike.Find(Peasant.Id, Last);
			if (Now - Last < Data.StrikeEvery)
			{
				continue;
			}
			const int Target = StrikeTargetFor(Summary, Data, Peasant);
			if (Target >= 0)
			{
				FHOLAIStrike Strike;
				Strike.PeasantId = Peasant.Id;
				Strike.TargetId = Target;
				Strikes.Add(Strike);
				State.LastStrike.Add(Peasant.Id, Now);
			}
		}
		return Strikes;
	}

	// The Peasants to send running to the centre now (IA2.6f): those within FleeRadius of an enemy soldier, not sent in the last FleeEvery seconds.
	TArray<int> FleeOrders(FHOLAIFlight& State, const FHOLAISummary& Summary, const FHOLAIArmyData& Data, float Now)
	{
		TArray<int> Ids;
		for (int Each = 0; Each < Summary.Peasants.Num(); Each++)
		{
			const FHOLAIUnit Peasant = Summary.Peasants[Each];
			float Last = -1000.f;
			State.LastOrder.Find(Peasant.Id, Last);
			if (Now - Last < Data.FleeEvery)
			{
				continue;
			}
			// IA4 (D572): a Peasant that wins its duel against a soldier near it strikes, it does not run.
			if (StrikeTargetFor(Summary, Data, Peasant) >= 0)
			{
				continue;
			}
			for (int Index = 0; Index < Summary.Enemies.Num(); Index++)
			{
				if (IsThreatening(Summary.Enemies[Index]) && FlatDistance(Summary.Enemies[Index].Location, Peasant.Location) <= Data.FleeRadius)
				{
					Ids.Add(Peasant.Id);
					State.LastOrder.Add(Peasant.Id, Now);
					break;
				}
			}
		}
		return Ids;
	}

	// Whether the army goes to meet the threat: a threat, and either the army is not far behind (ratio at least DefenceMinRatio) or an
	// enemy soldier has come to the assembly point (then it fights there, being cornered).
	bool ShouldDefend(const FHOLAISummary& Summary, const FHOLAIArmyData& Data, const FHOLAIThreat& Threat, FVector AssemblyPoint)
	{
		if (!Threat.bThreatened)
		{
			return false;
		}
		if (Threat.Ratio >= Data.DefenceMinRatio || (Threat.bAttackingBuilding && Threat.Ratio >= Data.DefenceMinRatioAttacked))
		{
			return true;
		}
		// IA3.2: the forum is down on its last life, the next fall is the defeat: the army fights for it whatever the ratio.
		if (Summary.bHasOwnForum && Summary.OwnForum.bDown && Summary.OwnForum.Lives <= 1)
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

	// ---------------------------------------------------------------------------------------------------------------------------
	// Attack wave and retreat (IA2.6). The brain keeps one FHOLAIWave per slot and asks, each thought, UpdateWave (home -> out ->
	// back -> home): it launches a wave when the rules allow (CanLaunch), keeps its idle soldiers on the way, calls it back when the
	// ratio near it collapses (AttackRetreatRatio, under the launch ratio: hysteresis) or when it has lost too much, and ends it when
	// nothing is left to attack. The defence has priority: no wave launches while defending, and a defence against AttackRecallCount
	// enemy soldiers or more near the base calls the wave back; a weaker threat is left to the soldiers at home (WithoutWave).

	// The ids of the slot's soldiers (the army of the summary without any Peasant), alive.
	TArray<int> SoldierIds(const FHOLAISummary& Summary)
	{
		TArray<int> Ids;
		for (int Index = 0; Index < Summary.Army.Num(); Index++)
		{
			if (Summary.Army[Index].Type != n"Peasant" && Summary.Army[Index].Health > 0)
			{
				Ids.Add(Summary.Army[Index].Id);
			}
		}
		return Ids;
	}

	// The soldiers of the wave still alive (in the summary's army).
	TArray<FHOLAIUnit> WaveUnits(const FHOLAIWave& Wave, const FHOLAISummary& Summary)
	{
		TArray<FHOLAIUnit> Units;
		for (int Index = 0; Index < Summary.Army.Num(); Index++)
		{
			if (Summary.Army[Index].Type != n"Peasant" && Summary.Army[Index].Health > 0 && Wave.Ids.Contains(Summary.Army[Index].Id))
			{
				Units.Add(Summary.Army[Index]);
			}
		}
		return Units;
	}

	// The wave soldiers alive.
	int WaveSize(const FHOLAIWave& Wave, const FHOLAISummary& Summary)
	{
		return WaveUnits(Wave, Summary).Num();
	}

	// The summary without the soldiers of a wave that is out (the defence and the regrouping order the soldiers at home only); the same
	// summary when no wave is out.
	FHOLAISummary WithoutWave(const FHOLAISummary& Summary, const FHOLAIWave& Wave)
	{
		if (Wave.Phase != EHOLAIWavePhase::Out)
		{
			return Summary;
		}
		FHOLAISummary Rest = Summary;
		Rest.Army.Empty();
		for (int Index = 0; Index < Summary.Army.Num(); Index++)
		{
			if (!Wave.Ids.Contains(Summary.Army[Index].Id))
			{
				Rest.Army.Add(Summary.Army[Index]);
			}
		}
		return Rest;
	}

	// The centre of a list of units (the origin without any).
	FVector CentroidOf(const TArray<FHOLAIUnit>& Units)
	{
		FVector Sum = FVector(0, 0, 0);
		for (int Index = 0; Index < Units.Num(); Index++)
		{
			Sum += Units[Index].Location;
		}
		if (Units.Num() == 0)
		{
			return Sum;
		}
		return Sum / float(Units.Num());
	}

	// The power ratio of the slot's army over the enemy soldiers within AllyHelpRadius of a place (the large value when none is there).
	float RatioAt(const FHOLAISummary& Summary, const FHOLAIArmyData& Data, FVector Place)
	{
		FHOLAISummary Local;
		Local.Army = Summary.Army;
		for (int Index = 0; Index < Summary.Enemies.Num(); Index++)
		{
			if (IsThreatening(Summary.Enemies[Index]) && FlatDistance(Summary.Enemies[Index].Location, Place) <= Data.AllyHelpRadius)
			{
				Local.Enemies.Add(Summary.Enemies[Index]);
			}
		}
		return PowerRatio(Data, Local);
	}

	// IA4 (D571): the place of the teammate's fallen forum (in ruin or rising again, with a known place) nearest to From where the army
	// holds its own against the enemy soldiers on the spot (RatioAt at least AllyHelpMinRatio); bFound false without one (From then).
	FVector AllyHelpTarget(const FHOLAISummary& Summary, const FHOLAIArmyData& Data, FVector From, bool& bFound)
	{
		bFound = false;
		FVector Best = From;
		for (int Index = 0; Index < Summary.AllyForums.Num(); Index++)
		{
			const FHOLAIForum Forum = Summary.AllyForums[Index];
			if (Forum.bDown && Forum.bHasLocation && (!bFound || FlatDistance(Forum.Location, From) < FlatDistance(Best, From))
				&& RatioAt(Summary, Data, Forum.Location) >= Data.AllyHelpMinRatio)
			{
				bFound = true;
				Best = Forum.Location;
			}
		}
		return Best;
	}

	// Whether a wave may be launched now: at home, not defending, the cooldown over, an enemy centre to go to (or a teammate's ruin to help
	// hold, IA4: then the minimum time does not count and the ratio is that of the spot), enough soldiers, late enough (earlier when no
	// enemy soldier has been seen: the ratio is then the large value) and our army ahead by the margin.
	bool CanLaunch(const FHOLAIWave& Wave, const FHOLAISummary& Summary, const FHOLAIArmyData& Data, bool bDefending, float Now)
	{
		bool bHelp = false;
		AllyHelpTarget(Summary, Data, Summary.CentreLocation, bHelp);
		// Hard's punishing wave (D579) obeys the cooldown too (found in a real game: it left, fell back and left again every 5 s).
		if (Wave.bForce && Wave.Phase == EHOLAIWavePhase::Home && Now >= Wave.NextLaunch && Summary.bHasEnemyCentre && SoldierIds(Summary).Num() > 0)
		{
			return true;
		}
		if (Wave.Phase != EHOLAIWavePhase::Home || bDefending || Now < Wave.NextLaunch || (!Summary.bHasEnemyCentre && !bHelp))
		{
			return false;
		}
		if (SoldierIds(Summary).Num() < Data.AttackMinSoldiers)
		{
			return false;
		}
		if (bHelp)
		{
			return true;
		}
		const bool bSeen = EnemyValue(Data, Summary) > 0.f;
		// IA3.3: an enemy forum lies fallen: the window is open, the minimum time does not count (the ratio and the soldiers still do).
		if (!HOLAIForum::EnemyRuin(Summary) && Now < (bSeen ? Data.AttackMinSeconds : Data.AttackMinSecondsUnseen))
		{
			return false;
		}
		return PowerRatio(Data, Summary) >= Data.AttackLaunchRatio;
	}

	// The power ratio of the wave's soldiers over the enemy soldiers within AttackRetreatRadius of their centroid (the large value when
	// none is near).
	float WaveRatio(const FHOLAIWave& Wave, const FHOLAISummary& Summary, const FHOLAIArmyData& Data)
	{
		FHOLAISummary Local;
		Local.Army = WaveUnits(Wave, Summary);
		const FVector Centre = CentroidOf(Local.Army);
		for (int Index = 0; Index < Summary.Enemies.Num(); Index++)
		{
			if (IsThreatening(Summary.Enemies[Index]) && FlatDistance(Summary.Enemies[Index].Location, Centre) <= Data.AttackRetreatRadius)
			{
				Local.Enemies.Add(Summary.Enemies[Index]);
			}
		}
		return PowerRatio(Data, Local);
	}

	// Where the wave's idle soldiers go: a teammate's fallen forum to help hold (IA4), a fallen enemy forum to hold (IA3.3), else the enemy centre, or when it is gone the enemy unit
	// nearest to the wave (bFound false without).
	FVector WaveTarget(const FHOLAISummary& Summary, const FHOLAIArmyData& Data, FVector From, bool& bFound)
	{
		bFound = true;
		// IA4: a teammate's fallen forum the army can hold comes first (a life of the team is at stake).
		bool bHelp = false;
		const FVector Help = AllyHelpTarget(Summary, Data, From, bHelp);
		if (bHelp)
		{
			return Help;
		}
		bool bRuin = false;
		const FVector Ruin = HOLAIForum::RuinTarget(Summary, From, bRuin);
		if (bRuin)
		{
			return Ruin;
		}
		// IA3.4: in Regicide, the enemy King (his death defeats his slot).
		bool bKing = false;
		const FVector King = HOLAIForum::KingTarget(Summary, From, bKing);
		if (Summary.bRegicide && bKing)
		{
			return King;
		}
		if (Summary.bHasEnemyCentre)
		{
			return Summary.EnemyCentreLocation;
		}
		int Nearest = -1;
		for (int Index = 0; Index < Summary.Enemies.Num(); Index++)
		{
			if (Summary.Enemies[Index].Health > 0 && (Nearest < 0 || FlatDistance(Summary.Enemies[Index].Location, From) < FlatDistance(Summary.Enemies[Nearest].Location, From)))
			{
				Nearest = Index;
			}
		}
		bFound = Nearest >= 0;
		return Nearest >= 0 ? Summary.Enemies[Nearest].Location : From;
	}

	// One thought of the wave: launches it, orders its idle soldiers again, falls it back or ends it, and reports what to order.
	// bRecall: the defence has a strong threat at home (the wave goes back to help).
	FHOLAIWaveStep UpdateWave(FHOLAIWave& Wave, const FHOLAISummary& Summary, const FHOLAIArmyData& Data, bool bDefending, bool bRecall, FVector AssemblyPoint, float Now)
	{
		FHOLAIWaveStep Step;
		if (Wave.Phase == EHOLAIWavePhase::Home)
		{
			if (CanLaunch(Wave, Summary, Data, bDefending, Now))
			{
				Wave.bPunishing = Wave.bForce;
				Wave.bForce = false;
				Wave.Phase = EHOLAIWavePhase::Out;
				Wave.Ids = SoldierIds(Summary);
				Wave.LaunchedSize = Wave.Ids.Num();
				Wave.PhaseStart = Now;
				Wave.LastOrder = Now;
				Wave.NextLaunch = Now + Data.AttackCooldown;
				Step.bLaunched = true;
				Step.AttackIds = Wave.Ids;
				bool bFound = false;
				Step.AttackTarget = WaveTarget(Summary, Data, Summary.CentreLocation, bFound);
				Step.Ratio = PowerRatio(Data, Summary);
				Step.Left = Wave.Ids.Num();
			}
			return Step;
		}
		const TArray<FHOLAIUnit> Alive = WaveUnits(Wave, Summary);
		Step.Left = Alive.Num();
		if (Wave.Phase == EHOLAIWavePhase::Out)
		{
			const int Lost = Wave.LaunchedSize - Alive.Num();
			const bool bLosses = float(Lost) > Data.AttackLossShare * float(Wave.LaunchedSize) + 0.001f;
			Step.Ratio = WaveRatio(Wave, Summary, Data);
			// A punishing wave (Hard, D579) that is still at our base sees the attackers it leaves behind: the ratio does not call it back there.
			const bool bLeaving = Wave.bPunishing && Summary.bHasCentre && FlatDistance(CentroidOf(Alive), Summary.CentreLocation) <= Data.DefenceRadius;
			if (bRecall || bLosses || (Step.Ratio < Data.AttackRetreatRatio && !bLeaving))
			{
				Wave.Phase = EHOLAIWavePhase::Back;
				Wave.PhaseStart = Now;
				Wave.LastOrder = Now;
				Step.bRetreat = true;
				for (int Index = 0; Index < Alive.Num(); Index++)
				{
					Step.MoveIds.Add(Alive[Index].Id);
				}
			}
			else if (!Summary.bHasEnemyCentre && Summary.Enemies.Num() == 0)
			{
				Wave.Phase = EHOLAIWavePhase::Home;
				Wave.Ids.Empty();
				Wave.LaunchedSize = 0;
				Wave.NextLaunch = Now + Data.AttackCooldown;
				Step.bOver = true;
			}
			else if (Now - Wave.LastOrder >= Data.AttackReissue)
			{
				bool bFound = false;
				const FVector Centroid = CentroidOf(Alive);
				const FVector Target = WaveTarget(Summary, Data, Centroid, bFound);
				// D751: the wave walks as a group, at its slowest member's speed. The engine paces one order's formation, but the soldiers that
				// stopped and are sent again leave on their own, faster than the engines (6741 cm front to rear measured by `aiwave`): a free
				// soldier more than WaveSpread ahead of the rearmost one stops where it stands (and is not sent on) until the rear has caught up.
				float DirX = Target.X - Centroid.X;
				float DirY = Target.Y - Centroid.Y;
				const float DirLength = Math::Sqrt(DirX * DirX + DirY * DirY);
				float Rear = 0.f;
				for (int Index = 0; bFound && DirLength > 1.f && Index < Alive.Num(); Index++)
				{
					const float Advance = ((Alive[Index].Location.X - Centroid.X) * DirX + (Alive[Index].Location.Y - Centroid.Y) * DirY) / DirLength;
					Rear = (Index == 0 || Advance < Rear) ? Advance : Rear;
				}
				for (int Index = 0; bFound && Index < Alive.Num(); Index++)
				{
					const float Advance = DirLength > 1.f ? ((Alive[Index].Location.X - Centroid.X) * DirX + (Alive[Index].Location.Y - Centroid.Y) * DirY) / DirLength : 0.f;
					const bool bAhead = Data.WaveSpread > 0.f && Advance - Rear > Data.WaveSpread;
					if (bAhead && Alive[Index].TargetId < 0)
					{
						Step.HoldIds.Add(Alive[Index].Id);
						Step.HoldPoints.Add(Alive[Index].Location);
					}
					else if (Alive[Index].bIdle && Alive[Index].TargetId < 0)
					{
						Step.AttackIds.Add(Alive[Index].Id);
					}
				}
				if (Step.AttackIds.Num() > 0 || Step.HoldIds.Num() > 0)
				{
					Step.AttackTarget = Target;
					Wave.LastOrder = Now;
				}
			}
			return Step;
		}
		// Back: home when most of the wave stands at the assembly point, or after AttackRetreatMax seconds; until then the soldiers
		// that stopped on the way are sent again.
		int Near = 0;
		for (int Index = 0; Index < Alive.Num(); Index++)
		{
			Near += FlatDistance(Alive[Index].Location, AssemblyPoint) <= Data.AssemblyRadius ? 1 : 0;
		}
		if (2 * Near >= Alive.Num() || Now - Wave.PhaseStart >= Data.AttackRetreatMax)
		{
			Wave.Phase = EHOLAIWavePhase::Home;
			Wave.Ids.Empty();
			Wave.LaunchedSize = 0;
			Wave.NextLaunch = Now + 2.f * Data.AttackCooldown;
			Step.bHome = true;
		}
		else if (Now - Wave.LastOrder >= Data.AttackReissue)
		{
			for (int Index = 0; Index < Alive.Num(); Index++)
			{
				if (Alive[Index].bIdle && Alive[Index].TargetId < 0 && FlatDistance(Alive[Index].Location, AssemblyPoint) > Data.AssemblyRadius)
				{
					Step.MoveIds.Add(Alive[Index].Id);
				}
			}
			if (Step.MoveIds.Num() > 0)
			{
				Wave.LastOrder = Now;
			}
		}
		return Step;
	}
}
