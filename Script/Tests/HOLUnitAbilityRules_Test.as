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
		// The morale-lever checks below are those of a self ability (the rage lays a bonus effect on its caster). An aimed ability (Target Ground, e.g.
		// the Light Seal) lays a zone whose slow is simulation C++ (the lot), not a morale row yet: its Effect stays empty until that row exists.
		if (Row.Target != HOLUnitAbilityTable::TargetSelf)
			continue;
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
	FHOLMoraleStateData State;
	T.AssertEquals(100, HOLMorale::Multiplier(State, HOLMorale::LeverDealt), "without the rage: the damage dealt is normal");
	HOLMorale::Apply(State, HOLMoraleTable::Effect(MoraleId));
	T.AssertTrue(HOLMorale::Multiplier(State, HOLMorale::LeverDealt) > 100, "with the rage: more damage dealt");
	T.AssertTrue(HOLMorale::Multiplier(State, HOLMorale::LeverRate) > 100, "with the rage: faster cadence");
	T.AssertEquals(100, HOLMorale::Multiplier(State, HOLMorale::LeverTaken), "the rage does not protect: the damage taken is normal");
}

// L6a extended (Docs/Rapports/LOT-CPP-socle-factions-1119.md, piece 5): the table says what an ability is aimed at. The existing rage is on its caster
// (Self, radius 0, range 0: no number decided); a FIXTURE row built here (not a real ability, numbers of the fixture) aimed at the ground is judged
// by the pure rules of HOLGroundAbility with the fields of its row.
void Test_UnitAbilityTableGroundColumns(FUnitTest& T)
{
	const HOLUnitAbilityTable::FHOLUnitAbilityRow Rage = HOLUnitAbilityTable::Row(HOLUnitAbilityTable::NecroticRage);
	T.AssertEquals(HOLUnitAbilityTable::TargetSelf, Rage.Target, "the rage is on its caster");
	T.AssertEquals(0, Rage.RadiusCm, "the rage lays no zone");
	T.AssertEquals(0, Rage.RangeCm, "the rage has no throw range");

	HOLUnitAbilityTable::FHOLUnitAbilityRow Fixture;
	Fixture.Key = "FixtureGround";
	Fixture.Target = HOLUnitAbilityTable::TargetGround;
	Fixture.RadiusCm = 300;
	Fixture.RangeCm = 1200;
	T.AssertEquals(HOLUnitAbilityTable::TargetGround, Fixture.Target, "the fixture is aimed at the ground");
	T.AssertEquals(0, HOLGroundAbility::TargetRefusal(true, int64(Fixture.RangeCm), int64(Fixture.RangeCm), 0), "a point at the range edge is accepted");
	T.AssertEquals(3, HOLGroundAbility::TargetRefusal(true, int64(Fixture.RangeCm) + 1, int64(Fixture.RangeCm), 0), "a point past the range is refused: out of range");
	T.AssertTrue(HOLZoneEffect::IsInZone(1000, 1000, Fixture.RadiusCm, 1000 + Fixture.RadiusCm, 1000), "the zone edge counts as inside");
	T.AssertFalse(HOLZoneEffect::IsInZone(1000, 1000, Fixture.RadiusCm, 1000 + Fixture.RadiusCm + 1, 1000), "past the radius: outside");
	// A self ability (radius 0) is a zone of one point.
	T.AssertTrue(HOLZoneEffect::IsInZone(5, 5, Rage.RadiusCm, 5, 5), "a zero radius holds only its centre");
	T.AssertFalse(HOLZoneEffect::IsInZone(5, 5, Rage.RadiusCm, 6, 5), "a zero radius holds nothing else");
}

// D859 step 1c: the Light Seal (Dawn) is a real row of the table, aimed at the ground, and the pure ground rules read ITS fields: the zone holds a
// point 5.9 m from the centre and not one at 6.1 m, a target 8.1 m from the caster is refused out of range (code 3), and the zone ends within the
// row's duration. The -40 % slow is simulation C++ (the lot), not asserted here. The row is looked up by key so that a missing row is a red assertion.
void Test_LightSealRowInTable(FUnitTest& T)
{
	int Id = -1;
	for (int I = 0; I < HOLUnitAbilityTable::Count; I++)
	{
		if (HOLUnitAbilityTable::Row(I).Key == "LightSeal")
			Id = I;
	}
	T.AssertTrue(Id >= 0, "the table has the LightSeal row");
	if (Id < 0)
		return;
	const HOLUnitAbilityTable::FHOLUnitAbilityRow Seal = HOLUnitAbilityTable::Row(Id);
	T.AssertEquals(HOLUnitAbilityTable::TargetGround, Seal.Target, "the seal is aimed at the ground");
	T.AssertTrue(Seal.Faction == "Dawn", "the seal is a Dawn ability");
	const int CenterX = 2000;
	const int CenterY = -500;
	T.AssertTrue(HOLZoneEffect::IsInZone(CenterX, CenterY, Seal.RadiusCm, CenterX + 590, CenterY), "5.9 m from the centre: inside the zone");
	T.AssertFalse(HOLZoneEffect::IsInZone(CenterX, CenterY, Seal.RadiusCm, CenterX + 610, CenterY), "6.1 m from the centre: outside the zone");
	T.AssertEquals(0, HOLGroundAbility::TargetRefusal(true, int64(790), int64(Seal.RangeCm), 0), "7.9 m from the caster: accepted");
	T.AssertEquals(3, HOLGroundAbility::TargetRefusal(true, int64(810), int64(Seal.RangeCm), 0), "8.1 m from the caster: refused, out of range");
	T.AssertTrue(HOLZoneEffect::ZoneActive(1000, Seal.DurationMs, 1000 + Seal.DurationMs - 1000), "the zone stands a second before its end");
	T.AssertFalse(HOLZoneEffect::ZoneActive(1000, Seal.DurationMs, 1000 + Seal.DurationMs + 1), "the zone is gone after its duration");
	T.AssertTrue(Seal.CooldownMs > Seal.DurationMs && Seal.DurationMs > 0, "the cooldown outlasts the zone");
}
