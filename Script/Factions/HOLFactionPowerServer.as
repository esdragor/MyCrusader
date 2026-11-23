// S4-a: the SERVER part of a faction power cast, ONE source called by the RPC ServerCastFactionPower (the player's order, after its sender check) and by
// the AI brain (HOLAIBrain::ThinkPowers): the same rule (HOLFactionPowerCast::Decide), the same effect (HOLZoneEffect::StartSacredLight), the same
// writes (gauge, cool-down). The AI gets no other path and no discount (D688).
namespace HOLFactionPowerServer
{
	// True when the power was put down (the effect started FIRST: a failed start costs nothing, then the gauge and the cool-down are written).
	bool TryCastPower(int Slot, int PowerIndex, FVector Point)
	{
		return TryCastPowerFrom(Slot, PowerIndex, Point, HOLHero::GetHeroBannerLocation(Slot));
	}

	// GLORFINDEL-1123j: PLACEHOLDER look of the Sacred Light zone (the zone had no drawing at all): a golden debug circle on the ground, the zone's radius, for
	// the zone's duration (both read from the zone constants, HOLZoneEffect::GetSacredLight*). Solo and host only (the debug line lives in the world that casts;
	// a dedicated server draws nothing, a client sees nothing: same limit as the Monk's heal lines). Listed in Placeholders.txt until real art exists.
	void DrawSacredLightPlaceholder(FVector Point)
	{
		const float Radius = float(HOLZoneEffect::GetSacredLightRadiusCm());
		const float Seconds = float(HOLZoneEffect::GetSacredLightDurationMs()) / 1000.f;
		const FVector Centre = FVector(Point.X, Point.Y, Point.Z + 30.0);
		const FLinearColor Gold = FLinearColor(1.0, 0.8, 0.1, 1.0);
		System::DrawDebugCircle(Centre, Radius, 64, Gold, Seconds, 25.f, FVector(1.0, 0.0, 0.0), FVector(0.0, 1.0, 0.0), false);
		System::DrawDebugCircle(Centre, Radius * 0.5f, 48, Gold, Seconds, 15.f, FVector(1.0, 0.0, 0.0), FVector(0.0, 1.0, 0.0), false);
		Log(n"LogHOLMatch", f"Sacred Light placeholder: golden ring of {int(Radius)} cm for {Seconds} s at ({int(Point.X)}, {int(Point.Y)})");
	}

	bool TryCastPowerFrom(int Slot, int PowerIndex, FVector Point, FIntPoint Origin)
	{
		AHOLGameState State = Cast<AHOLGameState>(Gameplay::GetGameState());
		if (State == nullptr)
			return false;
		// The origin is the caller's: the player's banner, or the AI's army centre (it plants no banner).
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
		DrawSacredLightPlaceholder(Point);
		State.SetFactionResource(Slot, Decision.NewGaugeValue);
		State.SetPowerCooldownEndMs(Slot, PowerIndex, Decision.NewCooldownEndMs);
		Log(n"LogHOLMatch", f"RpcPower: Sacred Light put down by slot {Slot}, Honor now {Decision.NewGaugeValue}");
		return true;
	}
}