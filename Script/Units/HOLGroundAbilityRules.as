// L6a extended (LegionsInProgress.md, Docs/Priorite_Socle_Factions_2026-11-16.md priority 2): the PURE rule of a unit ability aimed at the ground
// (zone, duration, cool-down). No simulation effect, no server command, no table column here: those come with the C++ batch.
// Units follow the rage rule (HOLUnitAbilityRules.h): distances in centimetres, times in whole simulation milliseconds.
// No number is decided here (radius, duration, range, cool-down of the Dawn light seal A3 and the Knight's challenge A7 are not in the GDD).
//
// STUB (red commit): final signatures, wrong returns.
namespace HOLGroundAbility
{
	// The reasons, in the order they are checked: 0 accepted, 1 caster dead, 2 cool-down running, 3 target out of range.
	int TargetRefusal(bool bCasterAlive, int64 DistanceCm, int64 RangeCm, int CooldownLeftMs)
	{
		return -1;
	}

	// Is the point inside the zone (centre and point in cm; the edge counts as inside).
	bool IsInZone(int CenterXCm, int CenterYCm, int RadiusCm, int PointXCm, int PointYCm)
	{
		return false;
	}

	// Is a zone put down at StartMs, lasting DurationMs, still active at NowMs (over at StartMs + DurationMs, like IsRageEffectEnded).
	bool ZoneActive(int StartMs, int DurationMs, int NowMs)
	{
		return false;
	}

	// When the cool-down of an ability cast at StartMs ends (counted from the cast, like GetRageCooldownEndMs).
	int CooldownEndMs(int StartMs, int CooldownMs)
	{
		return 0;
	}
}
