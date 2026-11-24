// S4-a: the SERVER part of a faction power cast, ONE source called by the RPC ServerCastFactionPower (the player's order, after its sender check) and by
// the AI brain (HOLAIBrain::ThinkPowers): the same rule (HOLFactionPowerCast::Decide), the same effect (HOLZoneEffect::StartSacredLight), the same
// writes (gauge, cool-down). The AI gets no other path and no discount (D688).
// FARAMIR-1124a (D900, answer C of Q-lumiere-sacree-visee): an aimed cast beyond the range is no longer refused, the hero walks up to the range, then casts.
struct FHOLFactionApproach
{
	bool bNeedsWalk = false;
	FVector StopPoint = FVector(0.0, 0.0, 0.0);
}

// GLORFINDEL-1124e (D900, F3): a cast waiting for its hero to arrive (one per slot). bPaid: the Honor and cool-down were written at the order (hol.Faction.ApproachPays 1).
struct FHOLPendingCast
{
	int Slot = 0;
	int PowerIndex = 0;
	FVector Point = FVector(0.0, 0.0, 0.0);
	bool bPaid = false;
	int ExpireMs = 0;
}

// GLORFINDEL-1124f: the casts waiting for their hero (one per slot), read each server tick by TickPendingCasts. A script global cannot be mutable (D900 pass integ-68:
// "Global variable 'PendingCasts' must be const"): the list lives in a world subsystem (UHOLFactionPendingHolder::Get().Pending), as UHOLFollowBookHolder does.
class UHOLFactionPendingHolder : UScriptWorldSubsystem
{
	UPROPERTY()
	TArray<FHOLPendingCast> Pending;
}

// GLORFINDEL-1124e: the cvar hol.Faction.ApproachPays, default 0 = the Honor is paid on ARRIVAL (nothing before); 1 = paid at the order (Retours/Q-lumiere-sacree-approche.md).
const FConsoleVariable CVar_FactionApproachPays("hol.Faction.ApproachPays", 0);

// FARAMIR-1124b (S2-c, D404): the order Sacred Light hands to the morale bridge (HOLMorale::ClearMalusAround) when the zone is put down.
struct FHOLSacredLightClearOrder
{
	int CenterXCm = 0;
	int CenterYCm = 0;
	int RadiusCm = 0;
	int Slot = 0;
}

namespace HOLFactionPowerServer
{
	// FARAMIR-1124b: the clearing order of a Sacred Light put down at Pose by Slot (centre = the pose, radius = the zone's radius, camp = the caster).
	FHOLSacredLightClearOrder ClearMalusOrderOf(int Slot, const FVector& Pose)
	{
		FHOLSacredLightClearOrder Order;
		Order.CenterXCm = int(Pose.X);
		Order.CenterYCm = int(Pose.Y);
		Order.RadiusCm = HOLZoneEffect::GetSacredLightRadiusCm();
		Order.Slot = Slot;
		return Order;
	}

	// FARAMIR-1124a: the pure rule of the approach. Origin = the hero's place, Point = the aimed point, RangeCm = the cast range. Beyond the range the hero
	// walks and stops (RangeCm - 50) cm from the point on the line to it (in range on arrival, the 50 cm absorb the stop's slack); inside, no walk.
	FHOLFactionApproach ApproachPointOf(const FVector& Origin, const FVector& Point, double RangeCm)
	{
		FHOLFactionApproach Result;
		Result.StopPoint = Origin;
		const double Dx = Point.X - Origin.X;
		const double Dy = Point.Y - Origin.Y;
		const double Distance = Math::Sqrt(Dx * Dx + Dy * Dy);
		if (Distance <= RangeCm || Distance < 1.0)
		{
			return Result;
		}
		const double StopFromPoint = Math::Max(0.0, RangeCm - 50.0);
		const double Walk = Distance - StopFromPoint;
		Result.bNeedsWalk = true;
		Result.StopPoint = FVector(Origin.X + Dx / Distance * Walk, Origin.Y + Dy / Distance * Walk, Origin.Z);
		return Result;
	}

	// GLORFINDEL-1124e: the step of the slot's pending cast. 0 = none pending, 1 = the hero still walks, 2 = arrived (within RangeCm of the aimed point): cast now.
	int PendingStepOf(const TArray<FHOLPendingCast>& Pending, int Slot, const FVector& HeroPlace, double RangeCm)
	{
		for (int Each = 0; Each < Pending.Num(); Each++)
		{
			if (Pending[Each].Slot == Slot)
			{
				const double Dx = Pending[Each].Point.X - HeroPlace.X;
				const double Dy = Pending[Each].Point.Y - HeroPlace.Y;
				return Math::Sqrt(Dx * Dx + Dy * Dy) <= RangeCm ? 2 : 1;
			}
		}
		return 0;
	}

	// GLORFINDEL-1124e: a second aimed cast of the slot replaces its pending one (one per slot).
	void SetPending(TArray<FHOLPendingCast>& Pending, const FHOLPendingCast& Order)
	{
		CancelPending(Pending, Order.Slot);
		Pending.Add(Order);
	}

	// GLORFINDEL-1124e: any new order of the slot's hero cancels its pending cast (the C++ hook that calls this on a hero order is TYPES_A_EXPOSER, see the report).
	void CancelPending(TArray<FHOLPendingCast>& Pending, int Slot)
	{
		for (int Each = Pending.Num() - 1; Each >= 0; Each--)
		{
			if (Pending[Each].Slot == Slot)
			{
				Pending.RemoveAt(Each);
			}
		}
	}

	// GLORFINDEL-1124e (D900): the aimed point is out of range: the hero walks to the stop point of ApproachPointOf, the cast waits for him. Default: the Honor is
	// written on ARRIVAL (nothing before); hol.Faction.ApproachPays 1 writes it at the order. False when no walk is needed or the order is refused.
	bool BeginApproach(AHOLGameState State, int Slot, int PowerIndex, const FVector& Point, const FVector& HeroPlace, int HeroId, int NowMs)
	{
		const FHOLFactionApproach Walk = ApproachPointOf(HeroPlace, Point, double(HOLDawnPowers::SacredLightRangeCm()));
		if (!Walk.bNeedsWalk)
		{
			return false;
		}
		FHOLPendingCast Order;
		Order.Slot = Slot;
		Order.PowerIndex = PowerIndex;
		Order.Point = Point;
		Order.ExpireMs = NowMs + 60000;
		FHOLFactionPowerDecision Paid;
		const bool bPayAtOrder = CVar_FactionApproachPays.GetInt() != 0;
		if (bPayAtOrder)
		{
			// Paid at the order: the decision is measured from the point itself (range 0), the row's Honor, level and cool-down rules still apply.
			FHOLFactionResource Gauge = HOLFactionResource::Make(State.GetFactionResource(Slot), 0, HOLDawnValues::HonorCap());
			Paid = HOLFactionPowerCast::Decide(State.GetSlotFaction(Slot), PowerIndex, Gauge, State.GetPowerCooldownEndMs(Slot, PowerIndex), NowMs,
				int(Point.X), int(Point.Y), int(Point.X), int(Point.Y));
			if (!Paid.bAccepted)
			{
				Log(n"LogHOLMatch", f"RpcPower: approach refused ({Paid.Refusal}) for slot {Slot} power {PowerIndex}");
				return false;
			}
		}
		TArray<int> HeroIds;
		HeroIds.Add(HeroId);
		if (!HOLPlayerOrders::Move(HOLCommandScript::GetCommands(), Slot, HeroIds, Walk.StopPoint, false, FVector2D(0.0, 0.0), false))
		{
			return false;
		}
		if (bPayAtOrder)
		{
			State.SetFactionResource(Slot, Paid.NewGaugeValue);
			State.SetPowerCooldownEndMs(Slot, PowerIndex, Paid.NewCooldownEndMs);
			Order.bPaid = true;
		}
		SetPending(UHOLFactionPendingHolder::Get().Pending, Order);
		Log(n"LogHOLMatch", f"RpcPower: slot {Slot} hero {HeroId} walks to ({int(Walk.StopPoint.X)}, {int(Walk.StopPoint.Y)}) to cast power {PowerIndex} at ({int(Point.X)}, {int(Point.Y)}), paid at order: {bPayAtOrder}");
		return true;
	}

	// GLORFINDEL-1124e: called each server tick (HOLRpcPlayer Tick, authority): a pending cast whose hero is in range puts the zone down AT THE CLICKED POINT (the Honor
	// is written ONCE: now, or already at the order when bPaid); a dead hero or an expired cast (60 s) is dropped.
	void TickPendingCasts()
	{
		UHOLFactionPendingHolder Holder = UHOLFactionPendingHolder::Get();
		if (Holder == nullptr || Holder.Pending.Num() == 0)
		{
			return;
		}
		AHOLGameState State = Cast<AHOLGameState>(Gameplay::GetGameState());
		if (State == nullptr)
		{
			return;
		}
		const int NowMs = int(Gameplay::GetTimeSeconds() * 1000.0);
		for (int Each = Holder.Pending.Num() - 1; Each >= 0; Each--)
		{
			const FHOLPendingCast Order = Holder.Pending[Each];
			FVector HeroPlace = FVector(0.0, 0.0, 0.0);
			if (NowMs > Order.ExpireMs || !HeroPlaceOf(Order.Slot, HeroPlace))
			{
				Holder.Pending.RemoveAt(Each);
				continue;
			}
			if (PendingStepOf(Holder.Pending, Order.Slot, HeroPlace, double(HOLDawnPowers::SacredLightRangeCm())) != 2)
			{
				continue;
			}
			Holder.Pending.RemoveAt(Each);
			FHOLFactionPowerDecision Decision;
			if (Order.bPaid)
			{
				Decision.bAccepted = true;
				Decision.NewGaugeValue = State.GetFactionResource(Order.Slot);
				Decision.NewCooldownEndMs = State.GetPowerCooldownEndMs(Order.Slot, Order.PowerIndex);
			}
			else
			{
				FHOLFactionResource Gauge = HOLFactionResource::Make(State.GetFactionResource(Order.Slot), 0, HOLDawnValues::HonorCap());
				const FHOLAimedCastDecision Aimed = HOLFactionPowerCast::DecideAimed(State.GetSlotFaction(Order.Slot), Order.PowerIndex, Gauge,
					State.GetPowerCooldownEndMs(Order.Slot, Order.PowerIndex), NowMs, int(HeroPlace.X), int(HeroPlace.Y), int(Order.Point.X), int(Order.Point.Y));
				Decision.bAccepted = Aimed.bAccepted;
				Decision.Refusal = Aimed.Refusal;
				Decision.NewGaugeValue = Aimed.NewGaugeValue;
				Decision.NewCooldownEndMs = Aimed.NewCooldownEndMs;
			}
			PutDown(State, Order.Slot, Order.PowerIndex, Decision, Order.Point);
		}
	}

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
		// D897: the zone is put down AT THE AIMED POINT; the origin only measures the range (HOLFactionPowerCast::DecideAimed, measured here on the server).
		AHOLGameState State = Cast<AHOLGameState>(Gameplay::GetGameState());
		if (State == nullptr)
			return false;
		const int NowMs = int(Gameplay::GetTimeSeconds() * 1000.0);
		FHOLFactionResource Gauge = HOLFactionResource::Make(State.GetFactionResource(Slot), 0, HOLDawnValues::HonorCap());
		const FHOLAimedCastDecision Aimed = HOLFactionPowerCast::DecideAimed(State.GetSlotFaction(Slot), PowerIndex, Gauge,
			State.GetPowerCooldownEndMs(Slot, PowerIndex), NowMs, int(Origin.X), int(Origin.Y), int(Point.X), int(Point.Y));
		// GLORFINDEL-1124e (D900): out of range is no longer a mute refusal: the hero walks up to the range, then casts (TickPendingCasts).
		if (!Aimed.bAccepted && Aimed.Refusal == HOLFactionPowerCast::RefusalOutOfRange() && bHeroKnown)
		{
			const int HeroId = HOLHero::GetHeroUnitId(Slot);
			if (HeroId >= 0 && BeginApproach(State, Slot, PowerIndex, Point, HeroPlace, HeroId, NowMs))
			{
				return true;
			}
		}
		FHOLFactionPowerDecision Decision;
		Decision.bAccepted = Aimed.bAccepted;
		Decision.Refusal = Aimed.Refusal;
		Decision.NewGaugeValue = Aimed.NewGaugeValue;
		Decision.NewCooldownEndMs = Aimed.NewCooldownEndMs;
		return PutDown(State, Slot, PowerIndex, Decision, FVector(double(Aimed.PoseXCm), double(Aimed.PoseYCm), Point.Z));
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
		return PutDown(State, Slot, PowerIndex, Decision, Point);
	}

	// The ONE writing of an accepted cast (the AI's TryCastPowerFrom and the player's aimed TryCastPower): refusal log, effect FIRST at Pose, then gauge and cool-down.
	bool PutDown(AHOLGameState State, int Slot, int PowerIndex, const FHOLFactionPowerDecision& Decision, FVector Pose)
	{
		if (!Decision.bAccepted)
		{
			Log(n"LogHOLMatch", f"RpcPower: cast refused ({Decision.Refusal}) for slot {Slot} power {PowerIndex}");
			return false;
		}
		// S1-b: the faction power is the book's Sacred Light (started FIRST: a failed start costs nothing), then the Honor and the row's cool-down are written.
		if (!HOLZoneEffect::StartSacredLight(Slot, Pose))
		{
			Log(n"LogHOLMatch", f"RpcPower: Sacred Light not started for slot {Slot} (no world, subsystem or slot)");
			return false;
		}
		// S2-c (D404): the light takes the malus off the caster's camp where it lands (queued; the morale processor lays it at its next step), after the start succeeded.
		const FHOLSacredLightClearOrder Clear = ClearMalusOrderOf(Slot, Pose);
		HOLMorale::ClearMalusAround(State, Clear.CenterXCm, Clear.CenterYCm, Clear.RadiusCm, Clear.Slot);
		DrawSacredLightPlaceholder(Pose);
		State.SetFactionResource(Slot, Decision.NewGaugeValue);
		State.SetPowerCooldownEndMs(Slot, PowerIndex, Decision.NewCooldownEndMs);
		Log(n"LogHOLMatch", f"RpcPower: Sacred Light put down by slot {Slot}, Honor now {Decision.NewGaugeValue}");
		return true;
	}
}