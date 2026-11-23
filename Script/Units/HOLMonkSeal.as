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
		FHOLMonkSealDecision Result;
		Result.NewCooldownEndMs = CooldownEndMs;
		if (Faction != int(EHOLFaction::DawnOrder))
		{
			Result.Refusal = RefusalNotDawn();
			return Result;
		}
		// Distance from the MONK to the point. Squares in double (a delta up to 2^32 - 1 squares past an int64), capped under the int range.
		const double DeltaX = double(PointXCm) - double(CasterXCm);
		const double DeltaY = double(PointYCm) - double(CasterYCm);
		const double ExactCm = Math::Sqrt(DeltaX * DeltaX + DeltaY * DeltaY);
		const int64 DistanceCm = int64(Math::CeilToInt(Math::Min(ExactCm, 2000000000.0)));
		// Cool-down then range, by the shared ground-ability rule (1 dead caster, 2 cool-down, 3 range): the caster is alive here (the RPC read his position).
		const int Reason = HOLGroundAbility::TargetRefusal(true, DistanceCm, RangeCm(), Math::Max(CooldownEndMs - NowMs, 0));
		if (Reason == 2)
		{
			Result.Refusal = RefusalCooldown();
			return Result;
		}
		if (Reason == 3)
		{
			Result.Refusal = RefusalOutOfRange();
			return Result;
		}
		// D862: NO Honor is paid; only the cool-down is written.
		Result.bAccepted = true;
		Result.NewCooldownEndMs = HOLGroundAbility::CooldownEndMs(NowMs, CooldownMs());
		return Result;
	}
}
