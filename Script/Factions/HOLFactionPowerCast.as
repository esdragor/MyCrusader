// FARAMIR-1120c (lot du socle des factions, morceau 3, serveur script): the PURE rule of a faction power cast, "power PowerIndex of
// the slot, on the point P". The RPC ServerCastFactionPower (Script/Net/HOLRpcPlayer.as) reads the world, asks Decide, and only on
// an accepted decision puts the Seal of Light down (HOLZoneEffect::StartLightSeal), pays the gauge and sets the cool-down.
// ONE source (D839): the cost is the row of HOLDawnPowers, the gauge arithmetic HOLFactionResource::TrySpend, the cool-down /
// range test HOLGroundAbility::TargetRefusal and CooldownEndMs; only the Seal's range and cool-down are numbers kept here.
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
	// Cast range from the caster's origin (UnitAbilities line, FARAMIR-1120b: "range 800") and the Seal's cool-down (40 s).
	int64 SealRangeCm()
	{
		return 800;
	}

	int SealCooldownMs()
	{
		return 40000;
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
		Result.Refusal = RefusalNotDawn();
		return Result;
	}
}
