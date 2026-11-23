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

namespace HOLFactionAim
{
	// The reach drawn around the hero while a power is aimed (cm); the server decides the real range again (HOLFactionPowerCast::Decide).
	const double AimRangeCm = 800.0;

	// A click on the box BoxIndex while the controller aims CurrentAim (-1 = none).
	FHOLFactionAimStep OnBox(FHOLFactionSlotView View, int CurrentAim, int BoxIndex)
	{
		FHOLFactionAimStep Step;
		return Step;
	}

	// The ground click the controller kept (Point) while it aims CurrentAim, for the player Slot.
	FHOLFactionAimStep OnGround(FHOLFactionSlotView View, int CurrentAim, int Slot, FVector Point)
	{
		FHOLFactionAimStep Step;
		return Step;
	}
}
