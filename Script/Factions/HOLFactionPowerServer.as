// S4-a: the SERVER part of a faction power cast, ONE source called by the RPC ServerCastFactionPower (the player's order, after its sender check) and by
// the AI brain (HOLAIBrain::ThinkPowers): the same rule (HOLFactionPowerCast::Decide), the same effect (HOLZoneEffect::StartSacredLight), the same
// writes (gauge, cool-down). The AI gets no other path and no discount (D688).
namespace HOLFactionPowerServer
{
	// True when the power was put down (the effect started FIRST: a failed start costs nothing, then the gauge and the cool-down are written).
	// GLORFINDEL-1123n: the ONE cast origin of a faction power, for the player AND the AI (it replaces HOLAIPowerCall::OriginOf). A planted banner is the
	// origin; with none planted (it reads (0, 0): the AI never plants one, the player may not have) the origin is the centre of Own, the caller's points:
	// the AI passes its army, the player the points of its units (an empty army: the hero's place, HeroPlaceOf, GLORFINDEL-1123pd;
	// the choice is Retours/Q-sacred-light-values.md). No points: the banner as is.
	FVector CastOriginOf(const FIntPoint& Banner, const TArray<FVector>& Own)
	{
		return CastOriginOf(Banner, Own, false, FVector(0.0, 0.0, 0.0));
	}

	// GLORFINDEL-1123pd: the cast origin with the hero as the LAST resort (banner, else the centre of Own, else the hero when bHeroKnown, else the banner as is).
	FVector CastOriginOf(const FIntPoint& Banner, const TArray<FVector>& Own, bool bHeroKnown, const FVector& HeroPlace)
	{
		if ((Banner.X != 0 || Banner.Y != 0) || (Own.Num() == 0 && !bHeroKnown))
		{
			return FVector(double(Banner.X), double(Banner.Y), 0.0);
		}
		if (Own.Num() == 0)
		{
			return FVector(HeroPlace.X, HeroPlace.Y, 0.0);
		}
		FVector Sum = FVector(0.0, 0.0, 0.0);
		for (int Index = 0; Index < Own.Num(); Index++)
		{
			Sum += Own[Index];
		}
		return FVector(Sum.X / double(Own.Num()), Sum.Y / double(Own.Num()), 0.0);
	}

	// GLORFINDEL-1123pd: where the slot's hero stands (the ONE reader, for the player's cast origin and the AI's banner place); false without a living hero.
	bool HeroPlaceOf(int Slot, FVector&out OutPlace)
	{
		return HOLHero::GetHeroLocation(Slot, OutPlace);
	}

	// The player's cast (after the RPC's sender check) and the AI's share the ONE origin: CastOriginOf(planted banner, else the centre of the slot's living units).
	bool TryCastPower(int Slot, int PowerIndex, FVector Point)
	{
		FHOLAISummary Summary = HOLAISummary::ReadAISummary(Slot);
		TArray<FVector> Own;
		for (int Each = 0; Each < Summary.Army.Num(); Each++)
		{
			if (Summary.Army[Each].Health > 0)
			{
				Own.Add(Summary.Army[Each].Location);
			}
		}
		FVector HeroPlace = FVector(0.0, 0.0, 0.0);
		const bool bHeroKnown = HeroPlaceOf(Slot, HeroPlace);
		const FVector Origin = CastOriginOf(HOLHero::GetHeroBannerLocation(Slot), Own, bHeroKnown, HeroPlace);
		return TryCastPowerFrom(Slot, PowerIndex, Point, FIntPoint(int(Origin.X), int(Origin.Y)));
	}

	// GLORFINDEL-1123j: PLACEHOLDER look of the Sacred Light zone (the zone had no drawing at all): a golden debug circle on the ground, the zone's radius, for
	// the zone's duration (both read from the zone constants, HOLZoneEffect::GetSacredLight*). Solo and host only (the debug line lives in the world that casts;
	// a dedicated server draws nothing, a client sees nothing: same limit as the Monk's heal lines). Listed in Placeholders.txt until real art exists.
	void DrawSacredLightPlaceholder(FVector Point)
	{
		const float Radius = float(HOLZoneEffect::GetSacredLightRadiusCm());
		const float Seconds = float(HOLZoneEffect::GetSacredLightDurationMs()) / 1000.f;
		DrawZoneCircles(Point, Radius, Seconds, FLinearColor(1.0, 0.8, 0.1, 1.0));
		Log(n"LogHOLMatch", f"Sacred Light placeholder: golden ring of {int(Radius)} cm for {Seconds} s at ({int(Point.X)}, {int(Point.Y)})");
	}

	// GLORFINDEL-1123k: PLACEHOLDER look of the Seal of Light zone (the slowing zone the Monk puts down): pale blue, so it reads apart from the golden Sacred
	// Light; radius and duration read from the zone constants (HOLZoneEffect::GetLightSeal*), never copied. Same limits and Placeholders.txt line as above.
	void DrawLightSealPlaceholder(FVector Point)
	{
		const float Radius = float(HOLZoneEffect::GetLightSealRadiusCm());
		const float Seconds = float(HOLZoneEffect::GetLightSealDurationMs()) / 1000.f;
		DrawZoneCircles(Point, Radius, Seconds, FLinearColor(0.35, 0.65, 1.0, 1.0));
		Log(n"LogHOLMatch", f"Seal of Light placeholder: pale blue ring of {int(Radius)} cm for {Seconds} s at ({int(Point.X)}, {int(Point.Y)})");
	}

	// The ONE drawing of a placeholder zone: an outer circle of the zone's radius and an inner one at half of it, 30 cm above the ground, for Seconds.
	void DrawZoneCircles(FVector Point, float Radius, float Seconds, FLinearColor Colour)
	{
		const FVector Centre = FVector(Point.X, Point.Y, Point.Z + 30.0);
		System::DrawDebugCircle(Centre, Radius, 64, Colour, Seconds, 25.f, FVector(1.0, 0.0, 0.0), FVector(0.0, 1.0, 0.0), false);
		System::DrawDebugCircle(Centre, Radius * 0.5f, 48, Colour, Seconds, 15.f, FVector(1.0, 0.0, 0.0), FVector(0.0, 1.0, 0.0), false);
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