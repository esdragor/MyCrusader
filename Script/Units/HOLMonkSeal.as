// GLORFINDEL-1123a (S1-d): the PURE rule of the Monk's Seal of Light cast from the command grid (D862): the Seal is the Monk's unit ability
// (UnitAbilities.csv row LightSeal), cast at a point at most RangeCm (8 m) from the MONK (not from the hero banner), cool-down 40 s, NO Honor.
// ONE source (D839): range, cool-down and faction are the HOLUnitAbilityTable LightSeal row; the range / cool-down test is HOLGroundAbility.
// The RPC ServerCastUnitAbility (Script/Net/HOLRpcPlayer.as) reads the world (the Monk's position, the slot's cool-down), asks Decide, and only on an
// accepted decision puts the Seal down (HOLZoneEffect::StartLightSeal). Distances in centimetres, times in whole milliseconds.
// Tested in Script/Tests/HOLMonkSeal_Test.as. S1-b replaces the book power by the Holy Light; this rule stays the Seal's.
struct FHOLMonkSealDecision
{
	bool bAccepted = false;
	// 0 accepted; see HOLMonkSeal::Refusal* (the order they are checked in).
	int Refusal = 0;
	// What the caller writes back when accepted; the input unchanged otherwise.
	int NewCooldownEndMs = 0;
}

namespace HOLMonkSeal
{
	int RefusalNone()
	{
		return 0;
	}

	int RefusalNotDawn()
	{
		return 1;
	}

	int RefusalCooldown()
	{
		return 2;
	}

	int RefusalOutOfRange()
	{
		return 3;
	}

	int64 RangeCm()
	{
		return int64(HOLUnitAbilityTable::Row(HOLUnitAbilityTable::LightSeal).RangeCm);
	}

	int CooldownMs()
	{
		return HOLUnitAbilityTable::Row(HOLUnitAbilityTable::LightSeal).CooldownMs;
	}

	// Faction: the EHOLFaction index of the slot. CooldownEndMs: the slot's cool-down end for the Seal (0 = ready). NowMs: the clock it is written in.
	FHOLMonkSealDecision Decide(int Faction, int CooldownEndMs, int NowMs, int CasterXCm, int CasterYCm, int PointXCm, int PointYCm)
	{
		// RED STUB (the test comes first): refuses everything.
		FHOLMonkSealDecision Result;
		Result.NewCooldownEndMs = CooldownEndMs;
		Result.Refusal = 99;
		return Result;
	}
}
