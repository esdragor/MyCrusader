// What the AI does with the ancient altar of a volcano site (V11, D427, D639, D640; VolcanoInProgress.md): a squad of its free soldiers goes
// and stands in the altar's zone, the way the relay detachment does (HOLAIRelay.as), and is left out of the army the other rules see (Without)
// so that the regrouping and the waves do not take it back. The numbers of the squad are the simulation's (HOLVolcano::DecideAltarSquad: the
// contest of an altar an adversary holds, D639; the free altar held by one soldier in 4, one in 8 in Easy, D640), all PROVISIONAL, behind
// FHOLVolcanoSettings. Inert without a lit altar in the world (no site in the map yet, V4b, or the Catastrophe option off). Pure, tested in
// Script/Tests/HOLAIAltar_Test.as; the brain reads the world (HOLVolcano::ReadAIAltars) and sends the orders.

// The numbers of the rules.
struct FHOLAIAltarData
{
	// The squad is ordered again (the idle ones away from the zone) at most this often (s).
	float ReissueSeconds = 5.f;

	// After a recall nothing is sent for this long (s), so that an attacked forum does not make the squad come and go.
	float PauseAfterRecallSeconds = 30.f;
}

// What the AI remembers between two thoughts of a slot.
struct FHOLAIAltarMemory
{
	// The soldiers at the altar, and the site whose altar it is (an index of the world's sites, -1 when there is no squad).
	TArray<int> Ids;
	int Site = -1;
	float LastOrder = -1000.f;
	float NextSend = 0.f;
}

// What a thought decides: the orders and what to write in the journal.
struct FHOLAIAltarStep
{
	bool bSent = false;
	bool bRecalled = false;
	bool bEnded = false;
	bool bDiversion = false;

	// The soldiers to attack-move to Where (the new ones when the squad grows, the idle ones away from the zone when ordered again).
	TArray<int> MoveIds;
	FVector Where;
	int Site = -1;
	FString Reason;
}

namespace HOLAIAltar
{
	// The level as the simulation's altar rule reads it: 0 Easy, 1 Normal, 2 Hard (Evolving is resolved before, by HOLAILives::Played).
	int LevelCode(EHOLAIDifficulty Level)
	{
		if (Level == EHOLAIDifficulty::Easy)
		{
			return 0;
		}
		if (Level == EHOLAIDifficulty::Hard)
		{
			return 2;
		}
		return 1;
	}

	// The soldiers of the army (alive, no Peasant).
	int SoldierCount(const FHOLAISummary& Summary)
	{
		int Count = 0;
		for (int Index = 0; Index < Summary.Army.Num(); Index++)
		{
			if (Summary.Army[Index].Type != n"Peasant" && Summary.Army[Index].Health > 0)
			{
				Count++;
			}
		}
		return Count;
	}

	// The summary without the squad: what the other army rules (defence, regrouping, waves, the relays) see.
	FHOLAISummary Without(const FHOLAISummary& Summary, const FHOLAIAltarMemory& Memory)
	{
		if (Memory.Ids.Num() == 0)
		{
			return Summary;
		}
		FHOLAISummary Rest = Summary;
		Rest.Army.Empty();
		for (int Index = 0; Index < Summary.Army.Num(); Index++)
		{
			if (!Memory.Ids.Contains(Summary.Army[Index].Id))
			{
				Rest.Army.Add(Summary.Army[Index]);
			}
		}
		return Rest;
	}

	// The altar to go to: the one the squad is at when it still stands among the lit ones, else the nearest to From. -1 without one.
	int ChooseAltar(const FHOLAIAltarMemory& Memory, const TArray<FHOLAIAltar>& Altars, const FVector& From)
	{
		int Best = -1;
		float BestDistance = 0.f;
		for (int Index = 0; Index < Altars.Num(); Index++)
		{
			if (Memory.Ids.Num() > 0 && Altars[Index].SiteIndex == Memory.Site)
			{
				return Index;
			}
			const float Distance = (Altars[Index].Location - From).Size();
			if (Best < 0 || Distance < BestDistance)
			{
				Best = Index;
				BestDistance = Distance;
			}
		}
		return Best;
	}

	// One thought of the slot about the altars. Summary: the army not in a wave (the brain removes the wave's soldiers first). Altars: the lit
	// ones, as the world measures them for this slot. Level: the level played now. bForumAttacked: enemy soldiers at the base. From: where the
	// nearest altar is measured (the centre). The memory is updated; the step holds the orders to send and the reason line.
	FHOLAIAltarStep Step(FHOLAIAltarMemory& Memory, EHOLAIDifficulty Level, const FHOLAISummary& Summary, const TArray<FHOLAIAltar>& Altars, bool bForumAttacked, float Now, const FVector& From, const FHOLAIAltarData& Data)
	{
		FHOLAIAltarStep Result;

		// The soldiers of the squad that are still alive.
		TArray<int> Alive;
		for (int Index = 0; Index < Memory.Ids.Num(); Index++)
		{
			for (int Soldier = 0; Soldier < Summary.Army.Num(); Soldier++)
			{
				if (Summary.Army[Soldier].Id == Memory.Ids[Index] && Summary.Army[Soldier].Health > 0)
				{
					Alive.Add(Memory.Ids[Index]);
					break;
				}
			}
		}
		const bool bHadSquad = Memory.Ids.Num() > 0;
		Memory.Ids = Alive;
		if (bHadSquad && Memory.Ids.Num() == 0)
		{
			Memory.Site = -1;
			Result.bEnded = true;
			Result.Reason = "the altar squad is lost, no soldier left";
			return Result;
		}

		const int Chosen = ChooseAltar(Memory, Altars, From);
		if (bHadSquad && bForumAttacked)
		{
			Result.bRecalled = true;
			Result.Site = Memory.Site;
			Result.Reason = f"recall: the forum is attacked, {Memory.Ids.Num()} altar soldiers back";
			Memory.Ids.Empty();
			Memory.Site = -1;
			Memory.NextSend = Now + Data.PauseAfterRecallSeconds;
			return Result;
		}
		if (Chosen < 0 || (bHadSquad && Altars[Chosen].SiteIndex != Memory.Site))
		{
			// No lit altar any more (held and woken, or the site is gone): the squad is free again.
			if (bHadSquad)
			{
				Result.bEnded = true;
				Result.Site = Memory.Site;
				Result.Reason = f"the altar of site {Memory.Site} is not lit any more, the squad of {Memory.Ids.Num()} ends";
				Memory.Ids.Empty();
				Memory.Site = -1;
			}
			return Result;
		}
		if (!bHadSquad && (bForumAttacked || Now < Memory.NextSend))
		{
			return Result;
		}

		const FHOLAIAltar Altar = Altars[Chosen];
		const int Free = SoldierCount(Summary);
		int Squad = 0;
		bool bDiversion = false;
		HOLVolcano::DecideAltarSquad(Altar, Free, Memory.Ids.Num(), LevelCode(Level), Squad, bDiversion);
		Result.bDiversion = bDiversion;
		Result.Site = Altar.SiteIndex;
		Result.Where = Altar.Location;

		// The squad grows to the size asked (never shrinks: the rule keeps the soldiers already sent), the soldiers nearest to the altar first.
		if (Squad > Memory.Ids.Num())
		{
			while (Memory.Ids.Num() < Squad)
			{
				int Nearest = -1;
				float NearestDistance = 0.f;
				for (int Soldier = 0; Soldier < Summary.Army.Num(); Soldier++)
				{
					const FHOLAIUnit Unit = Summary.Army[Soldier];
					if (Unit.Type == n"Peasant" || Unit.Health <= 0 || Memory.Ids.Contains(Unit.Id))
					{
						continue;
					}
					const float Distance = (Unit.Location - Altar.Location).Size();
					if (Nearest < 0 || Distance < NearestDistance)
					{
						Nearest = Soldier;
						NearestDistance = Distance;
					}
				}
				if (Nearest < 0)
				{
					break;
				}
				Memory.Ids.Add(Summary.Army[Nearest].Id);
				Result.MoveIds.Add(Summary.Army[Nearest].Id);
			}
			Memory.Site = Altar.SiteIndex;
			Memory.LastOrder = Now;
			Result.bSent = Result.MoveIds.Num() > 0;
			const FString Why = Altar.bAdversary ? f"an adversary of strength {Altar.AdversaryStrength} holds it" : f"free altar, the nearest trace {int(Altar.TraceDistance)} cm away";
			Result.Reason = f"{Memory.Ids.Num()} of {Free} soldiers go to the altar of site {Altar.SiteIndex} ({Why})";
			return Result;
		}
		if (bDiversion && Memory.Ids.Num() == 0)
		{
			Result.Reason = f"altar of site {Altar.SiteIndex}: an army of strength {Altar.AdversaryStrength} holds it, its camp is bare, no squad (diversion)";
			return Result;
		}

		// On its way: the idle soldiers away from the zone are ordered again, at most every ReissueSeconds.
		if (Memory.Ids.Num() > 0 && Now - Memory.LastOrder >= Data.ReissueSeconds)
		{
			for (int Soldier = 0; Soldier < Summary.Army.Num(); Soldier++)
			{
				const FHOLAIUnit Unit = Summary.Army[Soldier];
				if (Memory.Ids.Contains(Unit.Id) && Unit.bIdle && (Unit.Location - Altar.Location).Size() > Altar.Radius * 0.5f)
				{
					Result.MoveIds.Add(Unit.Id);
				}
			}
			if (Result.MoveIds.Num() > 0)
			{
				Memory.LastOrder = Now;
				Result.Reason = f"{Result.MoveIds.Num()} altar soldiers ordered again toward site {Altar.SiteIndex}";
			}
		}
		return Result;
	}
}
