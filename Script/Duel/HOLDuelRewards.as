// The morale rewards of a duel's end, as a pure rule (DuelInProgress.md D7, GDD 10.5, D16, D35): from the final state of a
// duel, the list of morale effects to pose. Nothing is applied here: the match applies the orders to the troops (the bridge
// to the simulation's morale is MoraleInProgress.md M7). Not here either: the XP bonus (no figure in the GDD) and the
// faction's effect (Honour for the Dawn).

// One morale effect to pose: on the troops around the hero of the slot, for DurationMs, within RadiusCm.
struct FHOLDuelMoraleOrder
{
	int Slot = -1;
	int EffectId = -1;
	int DurationMs = 0;
	int RadiusCm = 0;
}

namespace HOLDuelRewards
{
	// The orders of the duel's end: Triumph for the winner and Demoralised for the loser; Hesitation on the one who refuses
	// (a duel refused or ignored, when the refusal has its cost) and on nobody else; nothing for a double knock-out, a time
	// up or a duel not over. The durations are the morale table's; the radius is the one handed in (the match's choice).
	TArray<FHOLDuelMoraleOrder> Orders(const FHOLDuelSettings& S, const FHOLDuel& Duel, int RadiusCm)
	{
		TArray<FHOLDuelMoraleOrder> Result;
		return Result;
	}
}
