// The unit abilities (L4 step 4, Tools/Data/UnitAbilities.csv -> HOLUnitAbilityTable.as made by Tools/Data/make_unit_abilities.py; rules in
// C++ HOLUnitAbilityRules.h, read through HOLUnitAbilityRead). The numbers are read from the table, never copied: only the relations and the effects on the real rules are asserted.

// The effect named by a row of the table is a real row of the morale table, a temporary one of the ability's time, that helps the unit.
void Test_UnitAbilityEffectIsAMoraleEffect(FUnitTest& T)
{
	T.AssertTrue(HOLUnitAbilityTable::Count >= 1, "the necrotic rage is there");
	for (int Id = 0; Id < HOLUnitAbilityTable::Count; Id++)
	{
		const HOLUnitAbilityTable::FHOLUnitAbilityRow Row = HOLUnitAbilityTable::Row(Id);
		T.AssertFalse(Row.Key.IsEmpty(), f"ability {Id} has a key");
		T.AssertFalse(HOLUnitAbilityTable::Name(Id).ToString().IsEmpty(), f"{Row.Key} has a name");
		T.AssertTrue(Row.CooldownMs > Row.DurationMs && Row.DurationMs > 0, f"{Row.Key}: the cooldown outlasts the effect");
		int MoraleId = -1;
		for (int M = 0; M < HOLMoraleTable::Count; M++)
		{
			if (HOLMoraleTable::Row(M).Key == Row.Effect)
				MoraleId = M;
		}
		T.AssertTrue(MoraleId >= 0, f"{Row.Key}: the effect {Row.Effect} is a row of the morale table");
		if (MoraleId >= 0)
		{
			const HOLMoraleTable::FHOLMoraleRow Effect = HOLMoraleTable::Row(MoraleId);
			T.AssertFalse(Effect.bAura, f"{Row.Key}: a temporary effect");
			T.AssertEquals(Row.DurationMs, Effect.DurationMs, f"{Row.Key}: the ability and its effect last the same time");
			T.AssertFalse(Effect.bMalusSign, f"{Row.Key}: the effect helps the unit");
		}
	}
}

// Casting, through the library (the rule is C++, HOLUnitAbility::CanCastRage, D836 (2)): refused when dead, under the refusal floor or while the cool-down lasts.
void Test_UnitAbilityCanCast(FUnitTest& T)
{
	const float MinHp = float(HOLUnitAbilityRead::GetRageMinHp());
	T.AssertTrue(HOLUnitAbilityRead::CanCastRage(true, MinHp, 0), "at the refusal floor, never cast: allowed");
	T.AssertFalse(HOLUnitAbilityRead::CanCastRage(true, MinHp - 1.f, 0), "under the floor: refused");
	T.AssertFalse(HOLUnitAbilityRead::CanCastRage(false, MinHp + 50.f, 0), "dead: refused");
	T.AssertFalse(HOLUnitAbilityRead::CanCastRage(true, MinHp + 50.f, 1), "one ms of cool-down left: refused");
	T.AssertTrue(HOLUnitAbilityRead::CanCastRage(true, MinHp + 50.f, 0), "cool-down over: allowed");
}

// The price of the blows (HOLUnitAbility::GetRageHitCost): HpCostPerHit each, never under the stop floor (the rage never kills).
void Test_UnitAbilityHitCostNeverKills(FUnitTest& T)
{
	const float Stop = float(HOLUnitAbilityRead::GetRageStopHp());
	const float Cost = float(HOLUnitAbilityRead::GetRageHpCostPerHit());
	float Health = float(HOLUnitAbilityRead::GetRageMinHp());
	int Blows = 0;
	while (Health > Stop && Blows < 100)
	{
		const float Paid = HOLUnitAbilityRead::GetRageHitCost(Health);
		T.AssertTrue(Paid > 0.f && Paid <= Cost, "a blow costs hit points, at most the cost per hit");
		Health -= Paid;
		T.AssertTrue(Health >= Stop, "the rage never takes the unit under the stop floor");
		Blows++;
	}
	T.AssertEquals(int(Stop), int(Health), "paying blow after blow ends exactly at the stop floor");
	T.AssertEquals(0, int(HOLUnitAbilityRead::GetRageHitCost(Stop)), "no hit points left to pay at the floor");
	T.AssertEquals(0, int(HOLUnitAbilityRead::GetRageHitCost(1.f)), "under the floor: nothing to pay");
}
// On the real morale chain: the rage lays the effect of its row; damage dealt and cadence go up, nothing on the damage taken.
void Test_UnitAbilityRageActsOnTheRealMoraleChain(FUnitTest& T)
{
	const HOLUnitAbilityTable::FHOLUnitAbilityRow Rage = HOLUnitAbilityTable::Row(HOLUnitAbilityTable::NecroticRage);
	int MoraleId = -1;
	for (int M = 0; M < HOLMoraleTable::Count; M++)
	{
		if (HOLMoraleTable::Row(M).Key == Rage.Effect)
			MoraleId = M;
	}
	T.AssertTrue(MoraleId >= 0, "the rage's effect is in the morale table");
	HOLMorale::FHOLMoraleState State;
	T.AssertEquals(100, HOLMorale::Multiplier(State, HOLMorale::LeverDealt), "without the rage: the damage dealt is normal");
	HOLMorale::Apply(State, HOLMoraleTable::Effect(MoraleId));
	T.AssertTrue(HOLMorale::Multiplier(State, HOLMorale::LeverDealt) > 100, "with the rage: more damage dealt");
	T.AssertTrue(HOLMorale::Multiplier(State, HOLMorale::LeverRate) > 100, "with the rage: faster cadence");
	T.AssertEquals(100, HOLMorale::Multiplier(State, HOLMorale::LeverTaken), "the rage does not protect: the damage taken is normal");
}
