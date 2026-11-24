// The aim steps of the faction panel (ARWEN-1123a, S1-c; HOLFactionAim in Script/UI/HOLFactionAim.as). The views are built by the real
// HOLFactionSlot::Make (the power book says which box is ready); the steps come from the real HOLFactionSlot::ClickPower. The effect asserted
// is what the screen then does: arm the controller's aim, or send the cast with the clicked point.

namespace FactionAimTest
{
	FHOLFactionSlotView View()
	{
		TArray<FHOLPowerRow> Rows;
		FHOLPowerRow Ready;
		Ready.Key = "a";
		Ready.Cost = 10;
		Ready.bEffectCoded = true;
		Rows.Add(Ready);
		FHOLPowerRow Short;
		Short.Key = "b";
		Short.Cost = 50;
		Short.bEffectCoded = true;
		Rows.Add(Short);
		TArray<int> Cooldowns;
		return HOLFactionSlot::Make(EHOLFaction::DawnOrder, HOLFactionResource::Make(30, 0, 100), Rows, Cooldowns, 1, false, false);
	}
}

// A click on a ready box arms the controller's aim with the reach; a box short of Honor refuses and sets nothing.
void Test_FactionAimBoxArmsControllerAim(FUnitTest& T)
{
	const FHOLFactionSlotView Slot = FactionAimTest::View();
	T.AssertTrue(Slot.Powers.Num() == 2 && Slot.Powers[0].State == EHOLPowerState::Ready, "the book says box 0 is ready");
	const FHOLFactionAimStep Armed = HOLFactionAim::OnBox(Slot, -1, 0);
	T.AssertTrue(Armed.bSetAim, "the controller's aim is set");
	T.AssertEquals(0, Armed.AimIndex, "the aim is box 0");
	T.AssertTrue(Armed.RangeCm > 0.0, "the reach is drawn");
	T.AssertTrue(!Armed.bSend, "arming sends no cast");
	const FHOLFactionAimStep Refused = HOLFactionAim::OnBox(Slot, -1, 1);
	T.AssertTrue(!Refused.bSetAim && !Refused.bSend, "a box short of Honor sets nothing");
	const FHOLFactionAimStep Off = HOLFactionAim::OnBox(Slot, 0, 0);
	T.AssertTrue(Off.bSetAim && Off.AimIndex == -1, "the aimed box again ends the aim");
}

// The ground click kept while aiming sends the cast of the aimed power at the clicked point and ends the aim.
void Test_FactionAimGroundClickSendsCast(FUnitTest& T)
{
	const FHOLFactionSlotView Slot = FactionAimTest::View();
	const FVector Point = FVector(1200.0, -300.0, 0.0);
	const FHOLFactionAimStep Sent = HOLFactionAim::OnGround(Slot, 0, 0, Point);
	T.AssertTrue(Sent.bSend, "the ground click casts");
	T.AssertEquals(0, Sent.Slot, "for the clicking player's slot");
	T.AssertEquals(0, Sent.PowerIndex, "the aimed power");
	T.AssertTrue(Sent.Point.Equals(Point), "at the clicked point");
	T.AssertTrue(Sent.bSetAim && Sent.AimIndex == -1, "the aim ends with the cast");
	const FHOLFactionAimStep Idle = HOLFactionAim::OnGround(Slot, -1, 0, Point);
	T.AssertTrue(!Idle.bSend && !Idle.bSetAim, "no aim: the click is not for the slot");
}

// ARWEN-1124a (REGLAGE 2026-11-23): the aim preview under the mouse is a circle of the Sacred Light zone's radius (the real HOLZoneEffect getter), drawn in another colour when the point is out of the aim's reach (1200 cm from the origin).
void Test_FactionAimPreviewColoursOutOfReach(FUnitTest& T)
{
	const FVector Origin = FVector(0.0, 0.0, 0.0);
	const FHOLFactionAimPreview Near = HOLFactionAim::PreviewOf(Origin, FVector(500.0, 0.0, 0.0));
	const FHOLFactionAimPreview Far = HOLFactionAim::PreviewOf(Origin, FVector(2000.0, 0.0, 0.0));
	T.AssertTrue(Near.bInRange, "500 cm from the origin is in reach");
	T.AssertTrue(!Far.bInRange, "2000 cm from the origin is out of reach");
	T.AssertTrue(Math::Abs(Near.Radius - 600.0) < 0.01 && Math::Abs(Far.Radius - 600.0) < 0.01, "the circle is the zone's 6 m");
	const double Gap = Math::Abs(Near.Colour.R - Far.Colour.R) + Math::Abs(Near.Colour.G - Far.Colour.G) + Math::Abs(Near.Colour.B - Far.Colour.B);
	T.AssertTrue(Gap > 0.1, "another colour out of reach");
}
