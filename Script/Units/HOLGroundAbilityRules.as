// L6a extended (LegionsInProgress.md, Docs/Priorite_Socle_Factions_2026-11-16.md priority 2): the PURE rule of a unit ability aimed at the ground
// (zone, duration, cool-down). No simulation effect, no server command, no table column here: those come with the C++ batch.
// Units follow the rage rule (HOLUnitAbilityRules.h): distances in centimetres, times in whole simulation milliseconds.
// No number is decided here (radius, duration, range, cool-down of the Dawn light seal A3 and the Knight's challenge A7 are not in the GDD).
namespace HOLGroundAbility
{
	// The reasons, in the order they are checked: 0 accepted, 1 caster dead, 2 cool-down running, 3 target out of range.
	int TargetRefusal(bool bCasterAlive, int64 DistanceCm, int64 RangeCm, int CooldownLeftMs)
	{
		if (!bCasterAlive)
			return 1;
		if (CooldownLeftMs > 0)
			return 2;
		if (DistanceCm > RangeCm)
			return 3;
		return 0;
	}

	// Is the point inside the zone (centre and point in cm; the edge counts as inside). Squared distances in 64 bits: no root, no float.
	bool IsInZone(int CenterXCm, int CenterYCm, int RadiusCm, int PointXCm, int PointYCm)
	{
		const int64 Dx = int64(PointXCm) - int64(CenterXCm);
		const int64 Dy = int64(PointYCm) - int64(CenterYCm);
		const int64 Radius = int64(RadiusCm);
		return Dx * Dx + Dy * Dy <= Radius * Radius;
	}

	// Is a zone put down at StartMs, lasting DurationMs, still active at NowMs (over at StartMs + DurationMs, like IsRageEffectEnded).
	bool ZoneActive(int StartMs, int DurationMs, int NowMs)
	{
		return NowMs >= StartMs && NowMs < StartMs + DurationMs;
	}

	// When the cool-down of an ability cast at StartMs ends (counted from the cast, like GetRageCooldownEndMs).
	int CooldownEndMs(int StartMs, int CooldownMs)
	{
		return StartMs + CooldownMs;
	}
}
