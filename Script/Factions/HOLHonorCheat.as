// hol.Cheat.Honor, the effect on the game state (CELEBORN-1120b, D859: cap 100, floor 0, tunable after a try, D345). Pure script: the write is AHOLGameState::SetFactionResource
// (authority only; the RPC ServerCheatHonor runs on the server), the bounds are the pure rule HOLFactionResource::CheatTarget, the faction test HOLFactionResource::HasResource.
namespace HOLHonorCheat
{
	// The Dawn's Honor gauge: floor 0 (no debt), cap 100 (D688, D859).
	int HonorCap()
	{
		return 100;
	}

	// The server side of ServerCheatHonor (CELEBORN-1120d), pure of the RPC so a test drives it on a game state: the faction is the SENDER's, read from the state (GetSlotFaction, Dawn by default),
	// never from a client or a display text. Returns the value stored, or minus the refusal of HOLRpc::CheatHonorRefusal (-1 cheats off, -2 no slot, -3 no gauge), or -4 when nothing was written (no state).
	int ServerApply(AHOLGameState State, bool bCheatsAllowed, int Sender, int Value)
	{
		const EHOLFaction Faction = State != nullptr ? EHOLFaction(State.GetSlotFaction(Sender)) : EHOLFaction::DawnOrder;
		const int Refusal = HOLRpc::CheatHonorRefusal(bCheatsAllowed, Sender, HOLFactionResource::HasResource(Faction));
		if (Refusal != 0)
			return -Refusal;
		const int Stored = Apply(State, Sender, Faction, Value);
		return Stored < 0 ? -4 : Stored;
	}

	// Sets the slot's gauge to Requested held between the floor and the cap; returns the value stored, or -1 when nothing was written (no state, a slot below 0, or a faction without a gauge).
	// A slot beyond the table is ignored by SetFactionResource itself.
	int Apply(AHOLGameState State, int Slot, EHOLFaction Faction, int Requested)
	{
		if (State == nullptr || Slot < 0 || !HOLFactionResource::HasResource(Faction))
			return -1;
		const FHOLFactionResource Gauge = HOLFactionResource::Make(State.GetFactionResource(Slot), 0, HonorCap());
		const int Target = HOLFactionResource::CheatTarget(Requested, Gauge);
		State.SetFactionResource(Slot, Target);
		return Target;
	}
}