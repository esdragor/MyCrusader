// S4-a: the SERVER part of a faction power cast, ONE source called by the RPC ServerCastFactionPower (the player's order, after its sender check) and by
// the AI brain (HOLAIBrain::ThinkPowers): the same rule (HOLFactionPowerCast::Decide), the same effect (HOLZoneEffect::StartSacredLight), the same
// writes (gauge, cool-down). The AI gets no other path and no discount (D688).
namespace HOLFactionPowerServer
{
	// True when the power was put down (the effect started FIRST: a failed start costs nothing, then the gauge and the cool-down are written).
	bool TryCastPower(int Slot, int PowerIndex, FVector Point)
	{
		AHOLGameState State = Cast<AHOLGameState>(Gameplay::GetGameState());
		if (State == nullptr)
			return false;
		const FIntPoint Origin = HOLHero::GetHeroBannerLocation(Slot);
		const int NowMs = int(Gameplay::GetTimeSeconds() * 1000.0);
		FHOLFactionResource Gauge = HOLFactionResource::Make(State.GetFactionResource(Slot), 0, HOLDawnValues::HonorCap());
		FHOLFactionPowerDecision Decision = HOLFactionPowerCast::Decide(State.GetSlotFaction(Slot), PowerIndex, Gauge,
			State.GetPowerCooldownEndMs(Slot, PowerIndex), NowMs, Origin.X, Origin.Y, int(Point.X), int(Point.Y));
		if (!Decision.bAccepted)
		{
			Log(n"LogHOLMatch", f"RpcPower: cast refused ({Decision.Refusal}) for slot {Slot} power {PowerIndex}");
			return false;
		}
		// S1-b: the faction power is the book's Sacred Light (started FIRST: a failed start costs nothing), then the Honor and the row's cool-down are written.
		if (!HOLZoneEffect::StartSacredLight(Slot, Point))
		{
			Log(n"LogHOLMatch", f"RpcPower: Sacred Light not started for slot {Slot} (no world, subsystem or slot)");
			return false;
		}
		State.SetFactionResource(Slot, Decision.NewGaugeValue);
		State.SetPowerCooldownEndMs(Slot, PowerIndex, Decision.NewCooldownEndMs);
		Log(n"LogHOLMatch", f"RpcPower: Sacred Light put down by slot {Slot}, Honor now {Decision.NewGaugeValue}");
		return true;
	}
}