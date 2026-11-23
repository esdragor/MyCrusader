// FARAMIR-1120c (lot du socle des factions, morceau 3, serveur script): the PURE rule of a faction power cast, "power PowerIndex of
// the slot, on the point P". The RPC ServerCastFactionPower (Script/Net/HOLRpcPlayer.as) reads the world, asks Decide, and only on
// an accepted decision puts the Seal of Light down (HOLZoneEffect::StartLightSeal) and sets the cool-down.
// D862: the Seal is the Monk's unit ability (no Honor cost, the gauge is never touched); ONE source (D839): its range and cool-down are
// the HOLUnitAbilityTable LightSeal row, the cool-down / range test is HOLGroundAbility::TargetRefusal and CooldownEndMs.
// Distances in centimetres, times in whole milliseconds. Tested in Script/Tests/HOLFactionPowerCast_Test.as.
struct FHOLFactionPowerDecision
{
	bool bAccepted = false;
	// 0 accepted; see HOLFactionPowerCast::Refusal* (the order they are checked in).
	int Refusal = 0;
	// What the caller writes back when accepted (the gauge after the spend, the cool-down end); the inputs unchanged otherwise.
	int NewGaugeValue = 0;
	int NewCooldownEndMs = 0;
}

namespace HOLFactionPowerCast
{
	// D862: the Seal is the Monk's unit ability (UnitAbilities.csv row LightSeal), so its cast range from the caster's origin and its
	// cool-down are that row's (one source, no copy here): range 800 cm, cool-down 40 s.
	int64 SealRangeCm()
	{
		return int64(HOLUnitAbilityTable::Row(HOLUnitAbilityTable::LightSeal).RangeCm);
	}

	int SealCooldownMs()
	{
		return HOLUnitAbilityTable::Row(HOLUnitAbilityTable::LightSeal).CooldownMs;
	}

	// The refusal reasons, in the order they are checked.
	int RefusalNone()
	{
		return 0;
	}

	int RefusalNotDawn()
	{
		return 1;
	}

	int RefusalNoSuchPower()
	{
		return 2;
	}

	int RefusalCooldown()
	{
		return 3;
	}

	int RefusalOutOfRange()
	{
		return 4;
	}

	// S1-b: the gauge cannot pay the book row's Honor cost (checked last).
	int RefusalGaugeShort()
	{
		return 5;
	}

	// Faction: the EHOLFaction index of the slot (AHOLGameState::GetSlotFaction). Gauge: the slot's gauge. CooldownEndMs: the slot's
	// PowerCooldownEndMs for this power (0 = ready). NowMs: the clock the cool-down end is written in.
	FHOLFactionPowerDecision Decide(int Faction, int PowerIndex, const FHOLFactionResource& Gauge, int CooldownEndMs, int NowMs,
		int OriginXCm, int OriginYCm, int PointXCm, int PointYCm)
	{
		FHOLFactionPowerDecision Result;
		Result.NewGaugeValue = Gauge.Value;
		Result.NewCooldownEndMs = CooldownEndMs;
		if (Faction != int(EHOLFaction::DawnOrder))
		{
			Result.Refusal = RefusalNotDawn();
			return Result;
		}
		TArray<FHOLPowerRow> Rows = HOLDawnPowers::Rows();
		if (PowerIndex < 0 || PowerIndex >= Rows.Num())
		{
			Result.Refusal = RefusalNoSuchPower();
			return Result;
		}
		// Cool-down then range, by the shared ground-ability rule (1 dead caster and 2 cool-down, 3 range): the caster is alive here.
		const int64 DeltaX = int64(PointXCm) - int64(OriginXCm);
		const int64 DeltaY = int64(PointYCm) - int64(OriginYCm);
		// Squares in double (a delta up to 2^32 - 1 squares to 1.8e19, past an int64, and wrapped negative before: NaN distance, cast
		// accepted from anywhere), then capped under the int range so the ceiling cannot overflow either.
		const double ExactCm = Math::Sqrt(double(DeltaX) * double(DeltaX) + double(DeltaY) * double(DeltaY));
		const int64 DistanceCm = int64(Math::CeilToInt(Math::Min(ExactCm, 2000000000.0)));
		const int Reason = HOLGroundAbility::TargetRefusal(true, DistanceCm, SealRangeCm(), Math::Max(CooldownEndMs - NowMs, 0));
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
		// D862: the Seal costs NO Honor, so the gauge is carried through unchanged (no TrySpend, no gauge refusal).
		Result.bAccepted = true;
		Result.NewGaugeValue = Gauge.Value;
		Result.NewCooldownEndMs = HOLGroundAbility::CooldownEndMs(NowMs, SealCooldownMs());
		return Result;
	}
}
