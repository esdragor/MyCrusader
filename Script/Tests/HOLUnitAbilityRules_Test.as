// The unit abilities (L4 step 4, Tools/Data/UnitAbilities.csv -> HOLUnitAbilityTable.as made by Tools/Data/make_unit_abilities.py; rules in
// HOLUnitAbilityRules.as). The numbers are read from the table, never copied: only the relations and the effects on the real rules are asserted.

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

// Casting: refused when dead, under the refusal floor or in cooldown; allowed otherwise; the cooldown counts from the start of the effect.
void Test_UnitAbilityCanCast(FUnitTest& T)
{
	const HOLUnitAbilityTable::FHOLUnitAbilityRow Rage = HOLUnitAbilityTable::Row(HOLUnitAbilityTable::NecroticRage);
	T.AssertTrue(HOLUnitAbilityRules::CanCast(Rage, true, Rage.MinHp, 0, 0), "at the refusal floor, never cast: allowed");
	T.AssertFalse(HOLUnitAbilityRules::CanCast(Rage, true, Rage.MinHp - 1, 0, 0), "under the floor: refused");
	T.AssertFalse(HOLUnitAbilityRules::CanCast(Rage, false, Rage.MinHp + 50, 0, 0), "dead: refused");
	const int Start = 5000;
	const int CooldownEnd = HOLUnitAbilityRules::CooldownEnd(Rage, Start);
	T.AssertEquals(Start + Rage.CooldownMs, CooldownEnd, "the cooldown counts from the start of the effect");
	T.AssertFalse(HOLUnitAbilityRules::CanCast(Rage, true, Rage.MinHp + 50, CooldownEnd - 1, CooldownEnd), "one step before the end of the cooldown: refused");
	T.AssertTrue(HOLUnitAbilityRules::CanCast(Rage, true, Rage.MinHp + 50, CooldownEnd, CooldownEnd), "at the end of the cooldown: allowed");
	T.AssertFalse(HOLUnitAbilityRules::CanCast(HOLUnitAbilityTable::Row(HOLUnitAbilityTable::Count), true, 500, 0, 0), "an id out of the table: refused");
}

// The price of the blows: HpCostPerHit each, never under the stop floor (the rage never kills), and the effect ends by time or at the floor.
void Test_UnitAbilityHitCostNeverKills(FUnitTest& T)
{
	const HOLUnitAbilityTable::FHOLUnitAbilityRow Rage = HOLUnitAbilityTable::Row(HOLUnitAbilityTable::NecroticRage);
	int Health = Rage.MinHp;
	int Blows = 0;
	while (!HOLUnitAbilityRules::EffectEnded(Rage, 0, 0, Health) && Blows < 100)
	{
		const int Paid = HOLUnitAbilityRules::HitCost(Rage, Health);
		T.AssertTrue(Paid > 0 && Paid <= Rage.HpCostPerHit, "a blow costs hit points, at most the cost per hit");
		Health -= Paid;
		T.AssertTrue(Health >= Rage.StopHp, "the rage never takes the unit under the stop floor");
		Blows++;
	}
	T.AssertEquals(Rage.StopHp, Health, "paying blow after blow ends exactly at the stop floor");
	T.AssertTrue(HOLUnitAbilityRules::EffectEnded(Rage, 0, 0, Health), "and the effect is over there");
	T.AssertEquals(0, HOLUnitAbilityRules::HitCost(Rage, Rage.StopHp), "no hit points left to pay at the floor");
	T.AssertEquals(0, HOLUnitAbilityRules::HitCost(Rage, 1), "under the floor: nothing to pay");
	T.AssertFalse(HOLUnitAbilityRules::EffectEnded(Rage, 1000, 1000 + Rage.DurationMs - 1, Rage.MinHp), "one step before the time is over: still raging");
	T.AssertTrue(HOLUnitAbilityRules::EffectEnded(Rage, 1000, 1000 + Rage.DurationMs, Rage.MinHp), "the time is over: ended");
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
