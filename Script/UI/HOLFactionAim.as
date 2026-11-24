// The aim of a faction power on the HUD (ARWEN-1123a, S1-c; D839). The faction panel's boxes are buttons: a click on a ready box arms the
// aim on the player controller (SetFactionAim: the next left click on the ground is kept for the script and never selects), and the kept
// ground click (ConsumeFactionAimClick) casts the power (ServerCastFactionPower). The rule stays HOLFactionSlot::ClickPower (pure); this
// namespace turns its result into the steps the screen applies to the controller and the RPC.

struct FHOLFactionAimStep
{
	// The slot's own answer (HOLFactionSlot::ClickPower).
	FHOLFactionClickResult Click;
	// The controller's aim must be set to AimIndex (-1 ends it) with RangeCm.
	bool bSetAim = false;
	int AimIndex = -1;
	double RangeCm = 0.0;
	// ServerCastFactionPower(Slot, PowerIndex, Point) must be sent.
	bool bSend = false;
	int Slot = -1;
	int PowerIndex = -1;
	FVector Point = FVector::ZeroVector;
}

// ARWEN-1124a: what the aim preview draws under the mouse (a circle of the zone's radius; another colour out of the aim's reach).
struct FHOLFactionAimPreview
{
	bool bInRange = true;
	double Radius = 0.0;
	FLinearColor Colour = FLinearColor(1.0, 1.0, 1.0, 1.0);
}

namespace HOLFactionAim
{
	// The reach drawn around the hero while a power is aimed (cm); the server decides the real range again (HOLFactionPowerCast::DecideAimed; D890, 1200 cm = HOLDawnPowers::SacredLightRangeCm).
	const double AimRangeCm = 1200.0;

	// A click on the box BoxIndex while the controller aims CurrentAim (-1 = none).
	FHOLFactionAimStep OnBox(FHOLFactionSlotView View, int CurrentAim, int BoxIndex)
	{
		FHOLFactionAimStep Step;
		Step.Click = HOLFactionSlot::ClickPower(View, CurrentAim, EHOLFactionClickKind::Box, BoxIndex);
		if (Step.Click.Action == EHOLFactionClickAction::Aim)
		{
			Step.bSetAim = true;
			Step.AimIndex = Step.Click.AimIndex;
			Step.RangeCm = AimRangeCm;
		}
		else if (Step.Click.Action == EHOLFactionClickAction::Cancel)
			Step.bSetAim = true;
		return Step;
	}

	// The ground click the controller kept (Point) while it aims CurrentAim, for the player Slot.
	FHOLFactionAimStep OnGround(FHOLFactionSlotView View, int CurrentAim, int Slot, FVector Point)
	{
		FHOLFactionAimStep Step;
		Step.Click = HOLFactionSlot::ClickPower(View, CurrentAim, EHOLFactionClickKind::Ground, -1);
		if (Step.Click.Action == EHOLFactionClickAction::Launch)
		{
			Step.bSend = true;
			Step.Slot = Slot;
			Step.PowerIndex = Step.Click.CastIndex;
			Step.Point = Point;
		}
		Step.bSetAim = Step.Click.Action == EHOLFactionClickAction::Launch || Step.Click.Action == EHOLFactionClickAction::Cancel;
		return Step;
	}

	// ARWEN-1124a: the preview of the point under the mouse while a power is aimed from Origin.
	FHOLFactionAimPreview PreviewOf(FVector Origin, FVector Point)
	{
		FHOLFactionAimPreview Preview;
		// The zone's own radius (the real getter), never a copy of the number.
		Preview.Radius = double(HOLZoneEffect::GetSacredLightRadiusCm());
		const double Dx = Point.X - Origin.X;
		const double Dy = Point.Y - Origin.Y;
		Preview.bInRange = Math::Sqrt(Dx * Dx + Dy * Dy) <= AimRangeCm;
		// PLACEHOLDER colours: green in reach, orange out of reach (the hero will walk to reach it, D900).
		Preview.Colour = Preview.bInRange ? FLinearColor(0.3, 1.0, 0.3, 1.0) : FLinearColor(1.0, 0.45, 0.1, 1.0);
		return Preview;
	}
}
