// What the AI does with the neutral relays (R6; RelaysInProgress.md, RQ3 NOT DECIDED): pure rules, no world access, tested in
// HOLAIRelay_Test.as. PROVISIONAL: the three options of the question are built behind one setting (the console variable hol.AI.RelayMode,
// HOLRelayRules::GetAIRelayMode), the default being the question's recommendation (B). Building them is not choosing.
//   Mode 0 (A): the AI ignores the relays.
//   Mode 1 (B, default): by level (D579). Easy does not care. Normal sends a small detachment (a quarter of its soldiers) to the nearest relay
//     that belongs to nobody, when its forum is not attacked, and calls it back when the forum is attacked. Hard does the same and also goes
//     for the relays held by others, those taken from it first.
//   Mode 2 (C): the whole army chains the relays (every relay not its own, the nearest first) before the attack wave, whatever the level.
// The brain reads the relays (HOLRelayRules::ReadAIRelays), asks Step each thought, sends the move orders it returns and leaves the detached
// soldiers out of the army its other rules see (Without), so that the regrouping and the waves do not take them back. Every send, recall
// and end is a reason line for the decision journal. The numbers are a starting point, in one place (FHOLAIRelayData).
// The reading of "the nearest relay that is not hers" for Normal (only the neutral ones) is mine: the question's text does not separate
// Normal from Hard on that point except by "also retakes the relays taken from it".

namespace HOLAIRelay
{
	const int ModeIgnore = 0;
	const int ModeDetachment = 1;
	const int ModeWholeArmy = 2;

	// No relay.
	const int None = -1;
}

// The numbers of the rules.
struct FHOLAIRelayData
{
	// The share of its soldiers a detachment holds, percent (Normal and Hard, mode B), and with the whole army (mode C).
	int NormalSharePercent = 25;
	int HardSharePercent = 25;
	int WholeArmySharePercent = 100;

	// Mode B sends nothing with fewer soldiers than this (a detachment of one is not one).
	int MinArmyForDetachment = 4;

	// The soldiers away from the relay and idle are ordered again at most this often (s).
	float ReissueSeconds = 5.f;

	// After a recall nothing is sent for this long (s), so that an attacked forum does not make the detachment come and go.
	float PauseAfterRecallSeconds = 30.f;
}

// What the AI remembers between two thoughts of a slot.
struct FHOLAIRelayMemory
{
	// The slot this memory belongs to (the summary's PlayerIndex is not writable from script tests, so the brain sets it).
	int Slot = -1;

	// The detached soldiers and the relay they go to (an index of the relays, None when there is no detachment).
	TArray<int> Ids;
	int Target = -1;
	float LastOrder = -1000.f;

	// No detachment before this game time (s).
	float NextSend = 0.f;

	// The relays that were hers at the last thought and those that were hers and were taken (indexes): Hard retakes these first.
	TArray<int> HeldBefore;
	TArray<int> Lost;
}

// What a thought decides: the orders and what to write in the journal.
struct FHOLAIRelayStep
{
	bool bSent = false;
	bool bRecalled = false;
	bool bEnded = false;

	// The soldiers to attack-move to Where (all of them when sent, the idle ones away from the relay when ordered again).
	TArray<int> MoveIds;
	FVector Where;

	// The relay concerned and the reason line.
	int Relay = -1;
	FString Reason;
}

namespace HOLAIRelay
{
	// The relay is hers or an ally's.
	bool IsOurs(const FHOLAISummary& Summary, int Me, int Owner)
	{
		if (Owner < 0)
		{
			return false;
		}
		if (Owner == Me)
		{
			return true;
		}
		for (int Index = 0; Index < Summary.AllyForums.Num(); Index++)
		{
			if (Summary.AllyForums[Index].Slot == Owner)
			{
				return true;
			}
		}
		return false;
	}

	// The share of the army sent, percent: 0 when nothing is sent (A, Easy under B).
	int SharePercent(int Mode, EHOLAIDifficulty Level, const FHOLAIRelayData& Data)
	{
		if (Mode == ModeWholeArmy)
		{
			return Data.WholeArmySharePercent;
		}
		if (Mode != ModeDetachment)
		{
			return 0;
		}
		if (Level == EHOLAIDifficulty::Normal)
		{
			return Data.NormalSharePercent;
		}
		if (Level == EHOLAIDifficulty::Hard)
		{
			return Data.HardSharePercent;
		}
		return 0;
	}

	// The relay may be a target at this level and mode: Normal (B) wants the neutral ones; Hard (B) and the whole army (C) any that is not ours.
	bool IsTarget(const FHOLAISummary& Summary, int Me, const FHOLAIRelay& Relay, int Mode, EHOLAIDifficulty Level)
	{
		if (IsOurs(Summary, Me, Relay.Owner))
		{
			return false;
		}
		if (Mode == ModeDetachment && Level == EHOLAIDifficulty::Normal)
		{
			return Relay.Owner < 0;
		}
		return true;
	}

	// The nearest relay to From that may be a target; Hard (B) takes the relays it lost first. None without one.
	int ChooseRelay(const FHOLAISummary& Summary, int Me, const TArray<FHOLAIRelay>& Relays, const FHOLAIRelayMemory& Memory, int Mode, EHOLAIDifficulty Level, const FVector& From)
	{
		int Best = None;
		bool bBestLost = false;
		float BestDistance = 0.f;
		for (int Index = 0; Index < Relays.Num(); Index++)
		{
			if (!IsTarget(Summary, Me, Relays[Index], Mode, Level))
			{
				continue;
			}
			const bool bLost = Mode == ModeDetachment && Level == EHOLAIDifficulty::Hard && Memory.Lost.Contains(Index);
			const float Distance = (Relays[Index].Location - From).Size();
			if (Best == None || (bLost && !bBestLost) || (bLost == bBestLost && Distance < BestDistance))
			{
				Best = Index;
				bBestLost = bLost;
				BestDistance = Distance;
			}
		}
		return Best;
	}

	// The soldiers of the army (alive, no Peasant), by identifier.
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

	// The summary without the detached soldiers: what the other army rules (defence, regrouping, waves) see.
	FHOLAISummary Without(const FHOLAISummary& Summary, const FHOLAIRelayMemory& Memory)
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

	// Which relays were hers and which she lost (kept in the memory): called at the start of each thought.
	void Track(const FHOLAISummary& Summary, const TArray<FHOLAIRelay>& Relays, FHOLAIRelayMemory& Memory)
	{
		TArray<int> Held;
		for (int Index = 0; Index < Relays.Num(); Index++)
		{
			const bool bOurs = IsOurs(Summary, Memory.Slot, Relays[Index].Owner);
			if (bOurs)
			{
				Held.Add(Index);
				Memory.Lost.Remove(Index);
			}
			else if (Memory.HeldBefore.Contains(Index) && !Memory.Lost.Contains(Index))
			{
				Memory.Lost.Add(Index);
			}
		}
		Memory.HeldBefore = Held;
	}

	FString ModeName(int Mode)
	{
		if (Mode == ModeDetachment)
		{
			return "B detachment by level";
		}
		if (Mode == ModeWholeArmy)
		{
			return "C whole army";
		}
		return "A ignore";
	}

	// One thought of the slot about the relays. Summary: the army not in a wave (the brain removes the wave's soldiers first). Level: the
	// level played now (Evolving resolved). bForumAttacked: enemy soldiers at the base or the forum under attack. From: where the nearest is
	// measured (the centre). Mode: the setting. The memory is updated; the step holds the orders to send and the reason line.
	FHOLAIRelayStep Step(FHOLAIRelayMemory& Memory, int Mode, EHOLAIDifficulty Level, const FHOLAISummary& Summary, const TArray<FHOLAIRelay>& Relays, bool bForumAttacked, float Now, const FVector& From, const FHOLAIRelayData& Data)
	{
		FHOLAIRelayStep Result;
		Track(Summary, Relays, Memory);

		// The soldiers of the detachment that are still alive (in the army the brain handed over, the detached ones included).
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
		const bool bHadDetachment = Memory.Ids.Num() > 0;
		Memory.Ids = Alive;

		const int Share = SharePercent(Mode, Level, Data);
		if (bHadDetachment)
		{
			if (Memory.Ids.Num() == 0)
			{
				Memory.Target = None;
				Result.bEnded = true;
				Result.Reason = "the detachment is lost, no soldier left";
				return Result;
			}
			if (bForumAttacked || Share == 0)
			{
				Result.bRecalled = true;
				Result.Relay = Memory.Target;
				Result.Reason = bForumAttacked ? f"recall: the forum is attacked, {Memory.Ids.Num()} soldiers back" : f"recall: mode {ModeName(Mode)} sends nothing at this level";
				Memory.Ids.Empty();
				Memory.Target = None;
				Memory.NextSend = Now + Data.PauseAfterRecallSeconds;
				return Result;
			}
			const bool bValid = Memory.Target >= 0 && Memory.Target < Relays.Num() && IsTarget(Summary, Memory.Slot, Relays[Memory.Target], Mode, Level);
			if (!bValid)
			{
				Result.bEnded = true;
				Result.Relay = Memory.Target;
				const bool bOurs = Memory.Target >= 0 && Memory.Target < Relays.Num() && IsOurs(Summary, Memory.Slot, Relays[Memory.Target].Owner);
				Result.Reason = bOurs ? f"relay {Memory.Target} is ours, the detachment ends ({Memory.Ids.Num()} soldiers)" : f"relay {Memory.Target} is no longer a target, the detachment ends";
				Memory.Ids.Empty();
				Memory.Target = None;
				return Result;
			}
			// On its way: the idle soldiers away from the zone are ordered again, at most every ReissueSeconds.
			if (Now - Memory.LastOrder >= Data.ReissueSeconds)
			{
				const FHOLAIRelay Relay = Relays[Memory.Target];
				for (int Soldier = 0; Soldier < Summary.Army.Num(); Soldier++)
				{
					const FHOLAIUnit Unit = Summary.Army[Soldier];
					if (Memory.Ids.Contains(Unit.Id) && Unit.bIdle && (Unit.Location - Relay.Location).Size() > Relay.Radius * 0.5f)
					{
						Result.MoveIds.Add(Unit.Id);
					}
				}
				if (Result.MoveIds.Num() > 0)
				{
					Memory.LastOrder = Now;
					Result.Where = Relay.Location;
					Result.Relay = Memory.Target;
					Result.Reason = f"{Result.MoveIds.Num()} detached soldiers ordered again toward relay {Memory.Target}";
				}
			}
			return Result;
		}

		// No detachment: send one?
		if (Share == 0 || bForumAttacked || Now < Memory.NextSend)
		{
			return Result;
		}
		const int Soldiers = SoldierCount(Summary);
		if (Soldiers < (Mode == ModeWholeArmy ? 1 : Data.MinArmyForDetachment))
		{
			return Result;
		}
		const int Target = ChooseRelay(Summary, Memory.Slot, Relays, Memory, Mode, Level, From);
		if (Target == None)
		{
			return Result;
		}
		const int Count = Math::Max(1, Math::IntegerDivisionTrunc(Soldiers * Share + 99, 100));
		for (int Soldier = 0; Soldier < Summary.Army.Num() && Memory.Ids.Num() < Count; Soldier++)
		{
			if (Summary.Army[Soldier].Type != n"Peasant" && Summary.Army[Soldier].Health > 0)
			{
				Memory.Ids.Add(Summary.Army[Soldier].Id);
			}
		}
		Memory.Target = Target;
		Memory.LastOrder = Now;
		Result.bSent = true;
		Result.MoveIds = Memory.Ids;
		Result.Where = Relays[Target].Location;
		Result.Relay = Target;
		const FString LostNote = Memory.Lost.Contains(Target) ? " (one it lost)" : "";
		Result.Reason = f"sends {Count} of {Soldiers} soldiers to relay {Target}{LostNote}, mode {ModeName(Mode)}, level {int(Level)}";
		return Result;
	}
}
