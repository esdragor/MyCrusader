// L4 step 6 (LegionsInProgress.md): the AI's use of the Damned Warrior's necrotic rage. Pure rules, no world access, tested in HOLAIRage_Test.as.
// A Damned Warrior casts its rage (the order is the player's button, the server checks the owner, the type, the health and the cooldown) when it is
// in a melee: an enemy melee soldier within RangeCm, its health above MinHealthPercent of its full health and above the refusal floor (UnitAbilities.csv
// MinHp), and its last cast at least RecastEvery seconds ago (the cooldown of UnitAbilities.csv, counted from the start of the effect; the brain
// remembers the cast times because the summary does not carry the abilities' state). Every number is a default awaiting the user's answer (Retours\Q-legions-ai-warrior.md; one line each, FHOLAIRageData).
struct FHOLAIRageData
{
	// An enemy melee soldier this close to the warrior makes a melee (cm). A default: about two cells and a half of the grid.
	float RangeCm = 600.f;

	// The warrior casts only above this share of its full health (the rage costs 3 health points per blow).
	int MinHealthPercent = 60;

	// UnitAbilities.csv, NecroticRage: MinHp (the server refuses under it) and CooldownMs / 1000.
	int MinHealth = 30;
	float RecastEvery = 40.f;

	// The warrior's full health (HOLAIOrder::ReadAIUnitFigures); 0 = unknown, the rule never casts.
	int MaxHealth = 0;

	// The enemy types that count as melee soldiers (the shooters and the Peasants do not make a melee).
	TArray<FName> MeleeTypes;
}

// What each slot's brain remembers: the game second of each warrior's last cast (parallel arrays).
struct FHOLAIRageMemory
{
	TArray<int> Ids;
	TArray<float> Times;
}

namespace HOLAIRage
{
	FHOLAIRageData DefaultData()
	{
		FHOLAIRageData Data;
		Data.MeleeTypes.Add(n"Spearman");
		Data.MeleeTypes.Add(n"ManAtArms");
		Data.MeleeTypes.Add(n"Halberdier");
		Data.MeleeTypes.Add(n"LightCavalry");
		Data.MeleeTypes.Add(n"DamnedWarrior");
		return Data;
	}

	// The game second of the unit's last cast; far in the past when it never cast.
	float LastCast(const FHOLAIRageMemory& Memory, int UnitId)
	{
		for (int Index = 0; Index < Memory.Ids.Num(); Index++)
		{
			if (Memory.Ids[Index] == UnitId)
			{
				return Memory.Times[Index];
			}
		}
		return -1000.f;
	}

	void Remember(FHOLAIRageMemory& Memory, int UnitId, float Now)
	{
		for (int Index = 0; Index < Memory.Ids.Num(); Index++)
		{
			if (Memory.Ids[Index] == UnitId)
			{
				Memory.Times[Index] = Now;
				return;
			}
		}
		Memory.Ids.Add(UnitId);
		Memory.Times.Add(Now);
	}

	bool InMelee(const FHOLAISummary& Summary, const FHOLAIRageData& Data, const FHOLAIUnit& Warrior)
	{
		for (int Index = 0; Index < Summary.Enemies.Num(); Index++)
		{
			const FHOLAIUnit& Enemy = Summary.Enemies[Index];
			if (Enemy.Health <= 0 || !Data.MeleeTypes.Contains(Enemy.Type))
			{
				continue;
			}
			const float DX = Enemy.Location.X - Warrior.Location.X;
			const float DY = Enemy.Location.Y - Warrior.Location.Y;
			if (Math::Sqrt(DX * DX + DY * DY) <= Data.RangeCm)
			{
				return true;
			}
		}
		return false;
	}

	// The warriors of the slot that cast their rage now.
	TArray<int> Casters(const FHOLAISummary& Summary, const FHOLAIRageData& Data, const FHOLAIRageMemory& Memory, float Now)
	{
		TArray<int> Result;
		if (Data.MaxHealth <= 0)
		{
			return Result;
		}
		for (int Index = 0; Index < Summary.Army.Num(); Index++)
		{
			const FHOLAIUnit& Warrior = Summary.Army[Index];
			if (Warrior.Type != n"DamnedWarrior" || Warrior.Health < Data.MinHealth)
			{
				continue;
			}
			if (Warrior.Health * 100 <= Data.MaxHealth * Data.MinHealthPercent)
			{
				continue;
			}
			if (Now - LastCast(Memory, Warrior.Id) < Data.RecastEvery)
			{
				continue;
			}
			if (InMelee(Summary, Data, Warrior))
			{
				Result.Add(Warrior.Id);
			}
		}
		return Result;
	}
}
