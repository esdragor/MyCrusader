// The faction resource gauge, common base of the factions (Docs/Priorite_Socle_Factions_2026-11-16.md, priority 1; first client:
// the Dawn's Honor, AubeInProgress.md A4): an integer between a floor and a cap, pure rules. The floor and the cap are PARAMETERS
// (the Honor's real cap and start value are not decided: D345, the user chooses); nothing here names the Honor and nothing reads
// the world. Gain stops at the cap (the surplus is lost), Lose stops at the floor, TrySpend refuses when the stock is short and
// leaves it untouched. Replication, state fingerprint and HUD come with the C++ batch of a later session.
// Tested in Script/Tests/HOLFactionResourceRules_Test.as.
struct FHOLFactionResource
{
	int Value = 0;
	int Floor = 0;
	int Cap = 0;
}

namespace HOLFactionResource
{
	// A gauge at Start, brought inside [Floor, Cap].
	FHOLFactionResource Make(int Start, int Floor, int Cap)
	{
		FHOLFactionResource State;
		State.Floor = Floor;
		State.Cap = Cap;
		State.Value = Math::Clamp(Start, Floor, Cap);
		return State;
	}

	// Adds Amount (a negative or null Amount changes nothing); returns the surplus lost above the cap, 0 when all fitted.
	int Gain(FHOLFactionResource& State, int Amount)
	{
		if (Amount <= 0)
			return 0;
		const int Room = Math::Max(State.Cap - State.Value, 0);
		if (Amount <= Room)
		{
			State.Value += Amount;
			return 0;
		}
		State.Value += Room;
		return Amount - Room;
	}

	// Removes Amount (a negative or null Amount changes nothing); returns what was really removed (less than Amount at the floor).
	int Lose(FHOLFactionResource& State, int Amount)
	{
		if (Amount <= 0)
			return 0;
		const int Taken = Math::Min(Amount, Math::Max(State.Value - State.Floor, 0));
		State.Value -= Taken;
		return Taken;
	}

	// Spends Amount when the stock allows it (Value - Amount stays at or above the floor); otherwise false and the stock is untouched.
	bool TrySpend(FHOLFactionResource& State, int Amount)
	{
		if (Amount < 0 || State.Value - Amount < State.Floor)
			return false;
		State.Value -= Amount;
		return true;
	}

	// Which faction owns a gauge: today the Dawn alone. Another faction without a rule of its own, or an empty name, reads as the
	// Dawn (HOLFactionRules.h); only the Legions have none.
	bool HasResource(const FString& Faction)
	{
		return Faction != "Legions";
	}

	// The stable line of the test log: "<name> 30/100".
	FString Describe(const FString& Name, const FHOLFactionResource& State)
	{
		return f"{Name} {State.Value}/{State.Cap}";
	}
}
