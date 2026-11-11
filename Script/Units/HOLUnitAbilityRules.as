// Unit abilities, pure rules (LegionsInProgress.md L4 step 4, LQ-3 / D678; first use: the Damned Warrior's necrotic rage, Q-legions-l4 = A).
// Integers only, no world access, tested in HOLUnitAbilityRules_Test.as. The numbers come from HOLUnitAbilityTable (UnitAbilities.csv).
// An ability is ordered by the player (a button of the selection, step 5); it can be cast if the unit is alive, above the refusal floor
// (MinHp) and its cooldown (counted from the START of the effect) is over; while the effect lasts every blow dealt costs HpCostPerHit hit
// points, never taking the unit under StopHp (the ability never kills); the effect ends when its time is over or when the next blow would
// go under StopHp. The effect itself (damage dealt, cadence) is a morale effect of the table named by the row's Effect (HOLMoraleTable).
namespace HOLUnitAbilityRules
{
	// Can the unit cast the ability now? Times are simulation milliseconds; CooldownEndMs is when the last cast's cooldown ends (0 = never cast).
	bool CanCast(const HOLUnitAbilityTable::FHOLUnitAbilityRow& Row, bool bAlive, int Health, int NowMs, int CooldownEndMs)
	{
		if (Row.Key.IsEmpty() || !bAlive)
			return false;
		if (Health < Row.MinHp)
			return false;
		return NowMs >= CooldownEndMs;
	}

	// When the cooldown of a cast made at StartMs ends (counted from the start of the effect).
	int CooldownEnd(const HOLUnitAbilityTable::FHOLUnitAbilityRow& Row, int StartMs)
	{
		return StartMs + Row.CooldownMs;
	}

	// When the effect of a cast made at StartMs ends by time.
	int EffectEnd(const HOLUnitAbilityTable::FHOLUnitAbilityRow& Row, int StartMs)
	{
		return StartMs + Row.DurationMs;
	}

	// The hit points paid for a blow dealt under the effect: the cost, but never so much that the unit goes under StopHp.
	int HitCost(const HOLUnitAbilityTable::FHOLUnitAbilityRow& Row, int Health)
	{
		const int Room = Health - Row.StopHp;
		if (Room <= 0)
			return 0;
		return Room < Row.HpCostPerHit ? Room : Row.HpCostPerHit;
	}

	// Is the effect over? By time, or because the unit is at (or under) the stop floor.
	bool EffectEnded(const HOLUnitAbilityTable::FHOLUnitAbilityRow& Row, int StartMs, int NowMs, int Health)
	{
		return NowMs >= EffectEnd(Row, StartMs) || Health <= Row.StopHp;
	}
}
