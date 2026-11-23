// Unlock of the Templars' tier 1 commandery units by the chosen lane (D893, D896, GDD 13.7 grid of D133): a lane unit can be produced only by
// the player who CHOSE its lane at tier 1 (Iron = TempleSergeant, Faith = InfirmaryBrother, Stone = PaviseCrossbowman); other lanes and other
// factions do not see it. PURE rule, the lane -> unit table lives here and nowhere else. The option index is the lane order of the GDD
// (0 Iron, 1 Faith, 2 Stone: the grid cells 5, 6, 7 of D868). The grid filter and the server refusal of production call IsUnlocked.
namespace HOLSpecUnlock
{
	const int NotALaneUnit = -1;

	// The option (lane) that unlocks a unit type, or NotALaneUnit when the type is not a lane unit (always available).
	int LaneOfUnit(FName UnitType)
	{
		return NotALaneUnit;
	}

	// The unit type may be produced by a seat of this faction whose tier 1 chosen option is ChosenOption (HOLSpecialization::None when none).
	bool IsUnlocked(FName UnitType, int ChosenOption, EHOLFaction Faction)
	{
		return true;
	}
}
