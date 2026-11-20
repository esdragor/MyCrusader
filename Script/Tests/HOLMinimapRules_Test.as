// The minimap's pure rules (HOLMinimapRules.as): the world <-> picture conversion, what a click does, the alerts that blink.
// Screen up is world +X and screen right is world +Y (the RTS camera has no rotation, D254).

// A square map of 800 m (+-400 m) on a 200 px picture: 4 m a pixel.
FHOLMinimapFrame MinimapTestFrame()
{
	return HOLMinimap::MakeFrame(-40000.0, -40000.0, 40000.0, 40000.0, 200.0);
}

// The centre is the middle of the picture; world +X goes up, world +Y goes right; the corners are the corners.
void Test_MinimapToPicture(FUnitTest& T)
{
	FHOLMinimapFrame F = MinimapTestFrame();
	FVector2D Centre = HOLMinimap::ToPicture(F, FVector2D(0.0, 0.0));
	T.AssertEquals(100.0, Centre.X, "centre x");
	T.AssertEquals(100.0, Centre.Y, "centre y");
	FVector2D North = HOLMinimap::ToPicture(F, FVector2D(40000.0, 0.0));
	T.AssertEquals(100.0, North.X, "world +X: same column");
	T.AssertEquals(0.0, North.Y, "world +X: top of the picture");
	FVector2D East = HOLMinimap::ToPicture(F, FVector2D(0.0, 40000.0));
	T.AssertEquals(200.0, East.X, "world +Y: right of the picture");
	T.AssertEquals(100.0, East.Y, "world +Y: same row");
	FVector2D Low = HOLMinimap::ToPicture(F, FVector2D(-40000.0, -40000.0));
	T.AssertEquals(0.0, Low.X, "world corner -X -Y: left");
	T.AssertEquals(200.0, Low.Y, "world corner -X -Y: bottom");
	// 100 m north and 20 m east of the centre: 25 px up, 5 px right.
	FVector2D Near = HOLMinimap::ToPicture(F, FVector2D(10000.0, 2000.0));
	T.AssertEquals(105.0, Near.X, "20 m east");
	T.AssertEquals(75.0, Near.Y, "100 m north");
}

// The way back is the same line, and a point off the picture is clamped to the map.
void Test_MinimapToWorld(FUnitTest& T)
{
	FHOLMinimapFrame F = MinimapTestFrame();
	FVector2D Back = HOLMinimap::ToWorld(F, FVector2D(105.0, 75.0));
	T.AssertEquals(10000.0, Back.X, "world x back");
	T.AssertEquals(2000.0, Back.Y, "world y back");
	FVector2D Out = HOLMinimap::ToWorld(F, FVector2D(-30.0, 500.0));
	T.AssertEquals(-40000.0, Out.X, "clamped to the map's lower edge (bottom of the picture)");
	T.AssertEquals(-40000.0, Out.Y, "clamped to the map's left edge");
	T.AssertTrue(HOLMinimap::Contains(F, FVector2D(0.0, 0.0)), "the corner is inside");
	T.AssertFalse(HOLMinimap::Contains(F, FVector2D(201.0, 10.0)), "off the picture");
	// An asymmetric map (a rectangle) keeps its ratio per axis.
	FHOLMinimapFrame R = HOLMinimap::MakeFrame(-10000.0, -30000.0, 10000.0, 30000.0, 120.0);
	FVector2D P = HOLMinimap::ToPicture(R, FVector2D(10000.0, 30000.0));
	T.AssertEquals(120.0, P.X, "rectangle: right");
	T.AssertEquals(0.0, P.Y, "rectangle: top");
}

// Left click: the camera, whatever is selected. Right click: a move order, only for someone who has units to order.
void Test_MinimapClickAction(FUnitTest& T)
{
	T.AssertEquals(HOLMinimap::ClickCamera, HOLMinimap::ClickAction(false, false, false), "left, nothing selected: camera");
	T.AssertEquals(HOLMinimap::ClickCamera, HOLMinimap::ClickAction(false, true, false), "left, units selected: camera");
	T.AssertEquals(HOLMinimap::ClickMove, HOLMinimap::ClickAction(true, true, false), "right, units selected: move");
	T.AssertEquals(HOLMinimap::ClickNothing, HOLMinimap::ClickAction(true, false, false), "right, nothing to order: nothing");
}

// D860 H3: Alt + left click on the picture places the same signal as Alt + click on the ground, with or without a selection;
// Alt + right stays an order (or nothing), it is not a signal.
void Test_MinimapClickActionAltPlacesSignal(FUnitTest& T)
{
	T.AssertEquals(HOLMinimap::ClickSignal, HOLMinimap::ClickAction(false, false, true), "Alt + left, nothing selected: signal");
	T.AssertEquals(HOLMinimap::ClickSignal, HOLMinimap::ClickAction(false, true, true), "Alt + left, units selected: signal");
	T.AssertEquals(HOLMinimap::ClickMove, HOLMinimap::ClickAction(true, true, true), "Alt + right, units selected: still a move");
	T.AssertEquals(HOLMinimap::ClickNothing, HOLMinimap::ClickAction(true, false, true), "Alt + right, nothing to order: nothing");
	T.AssertEquals(HOLMinimap::ClickCamera, HOLMinimap::ClickAction(false, true, false), "no Alt: the camera, as before");
}

// A unit that lost health makes an alert where it stands; a healed or untouched one does not; a new one does not.
FHOLMinimapSeen MinimapSeen(int Id, float X, float Y, int Health)
{
	FHOLMinimapSeen S;
	S.Id = Id;
	S.X = X;
	S.Y = Y;
	S.Health = Health;
	return S;
}

void Test_MinimapAlertFromDamage(FUnitTest& T)
{
	TArray<FHOLMinimapSeen> Before;
	Before.Add(MinimapSeen(1, 0.0, 0.0, 100));
	Before.Add(MinimapSeen(2, 5000.0, 0.0, 100));
	Before.Add(MinimapSeen(3, 9000.0, 9000.0, 50));
	TArray<FHOLMinimapSeen> After;
	After.Add(MinimapSeen(1, 100.0, 0.0, 90));
	After.Add(MinimapSeen(2, 5000.0, 0.0, 100));
	After.Add(MinimapSeen(3, 9000.0, 9000.0, 70));
	After.Add(MinimapSeen(4, -9000.0, -9000.0, 30));
	TArray<FHOLMinimapAlert> Alerts;
	HOLMinimap::Detect(Before, After, Alerts);
	T.AssertEquals(1, Alerts.Num(), "only the wounded one");
	T.AssertEquals(100.0, Alerts[0].X, "where it stands now");
	T.AssertEquals(0.0, Alerts[0].Age, "new");
}

// Two wounded close together make one alert (it renews); a far one makes another.
void Test_MinimapAlertsMerge(FUnitTest& T)
{
	TArray<FHOLMinimapSeen> Before;
	Before.Add(MinimapSeen(1, 0.0, 0.0, 100));
	Before.Add(MinimapSeen(2, 1000.0, 0.0, 100));
	Before.Add(MinimapSeen(3, 30000.0, 0.0, 100));
	TArray<FHOLMinimapSeen> After;
	After.Add(MinimapSeen(1, 0.0, 0.0, 80));
	After.Add(MinimapSeen(2, 1000.0, 0.0, 80));
	After.Add(MinimapSeen(3, 30000.0, 0.0, 80));
	TArray<FHOLMinimapAlert> Alerts;
	HOLMinimap::Detect(Before, After, Alerts);
	T.AssertEquals(2, Alerts.Num(), "two places");
	// Hit again later in the same place: the alert is renewed, not doubled.
	HOLMinimap::Age(Alerts, 3.0, HOLMinimap::AlertSeconds);
	T.AssertEquals(3.0, Alerts[0].Age, "aged");
	TArray<FHOLMinimapSeen> Later;
	Later.Add(MinimapSeen(1, 0.0, 0.0, 60));
	Later.Add(MinimapSeen(2, 1000.0, 0.0, 80));
	Later.Add(MinimapSeen(3, 30000.0, 0.0, 80));
	HOLMinimap::Detect(After, Later, Alerts);
	T.AssertEquals(2, Alerts.Num(), "still two");
	T.AssertEquals(0.0, Alerts[0].Age, "renewed");
}

// An alert lives AlertSeconds, then is gone; it blinks: lit the first half-period, dark the next.
void Test_MinimapAlertBlink(FUnitTest& T)
{
	TArray<FHOLMinimapAlert> Alerts;
	FHOLMinimapAlert A;
	A.X = 0.0;
	A.Y = 0.0;
	Alerts.Add(A);
	T.AssertTrue(HOLMinimap::IsLit(Alerts[0], HOLMinimap::BlinkSeconds), "lit at once");
	HOLMinimap::Age(Alerts, HOLMinimap::BlinkSeconds + 0.01, HOLMinimap::AlertSeconds);
	T.AssertEquals(1, Alerts.Num(), "still there");
	T.AssertFalse(HOLMinimap::IsLit(Alerts[0], HOLMinimap::BlinkSeconds), "dark in the second half-period");
	HOLMinimap::Age(Alerts, HOLMinimap::BlinkSeconds, HOLMinimap::AlertSeconds);
	T.AssertTrue(HOLMinimap::IsLit(Alerts[0], HOLMinimap::BlinkSeconds), "lit again");
	HOLMinimap::Age(Alerts, HOLMinimap::AlertSeconds, HOLMinimap::AlertSeconds);
	T.AssertEquals(0, Alerts.Num(), "gone after its time");
}

// The settings (hol.Minimap.AlertSeconds, hol.Minimap.AlertBlink): an alert lives as long as asked and blinks at the asked pace;
// a silly value (0, negative, huge) is brought back to a sane one, never a division by zero.
void Test_MinimapAlertSettings(FUnitTest& T)
{
	TArray<FHOLMinimapAlert> Alerts;
	FHOLMinimapAlert A;
	Alerts.Add(A);
	// A long half-period (1.5 s): still lit at 0.7 s, dark at 1.7 s; the default 0.5 s would be dark at 0.7 s.
	HOLMinimap::Age(Alerts, 0.7, 10.0);
	T.AssertTrue(HOLMinimap::IsLit(Alerts[0], 1.5), "lit at 0.7 s with a 1.5 s half-period");
	T.AssertFalse(HOLMinimap::IsLit(Alerts[0], 0.5), "dark at 0.7 s with the default half-period");
	HOLMinimap::Age(Alerts, 1.0, 10.0);
	T.AssertFalse(HOLMinimap::IsLit(Alerts[0], 1.5), "dark at 1.7 s with a 1.5 s half-period");
	// A short life (2 s): gone at 2.5 s; a long one (20 s): there at 15 s.
	HOLMinimap::Age(Alerts, 0.9, 2.0);
	T.AssertEquals(0, Alerts.Num(), "gone after a 2 s life");
	TArray<FHOLMinimapAlert> Long;
	Long.Add(A);
	HOLMinimap::Age(Long, 15.0, 20.0);
	T.AssertEquals(1, Long.Num(), "a 20 s alert is still there at 15 s");
	// Sane values.
	T.AssertEquals(0.5, HOLMinimap::SaneLifetime(0.0), "a life of 0 s is 0.5 s");
	T.AssertEquals(60.0, HOLMinimap::SaneLifetime(1000.0), "a life of 1000 s is 60 s");
	T.AssertEquals(6.0, HOLMinimap::SaneLifetime(6.0), "6 s stays");
	T.AssertEquals(0.1, HOLMinimap::SaneBlink(0.0), "a half-period of 0 is 0.1 s");
	T.AssertEquals(0.1, HOLMinimap::SaneBlink(-3.0), "a negative one is 0.1 s");
	T.AssertEquals(10.0, HOLMinimap::SaneBlink(99.0), "99 s is 10 s");
	FHOLMinimapAlert Fresh;
	T.AssertTrue(HOLMinimap::IsLit(Fresh, 0.0), "a 0 half-period does not divide by zero");
}
// The camera's frame: four ground points become four picture points in the same order.
void Test_MinimapCameraFrame(FUnitTest& T)
{
	FHOLMinimapFrame F = MinimapTestFrame();
	TArray<FVector2D> Ground;
	Ground.Add(FVector2D(10000.0, -10000.0));
	Ground.Add(FVector2D(10000.0, 10000.0));
	Ground.Add(FVector2D(-10000.0, 10000.0));
	Ground.Add(FVector2D(-10000.0, -10000.0));
	TArray<FVector2D> Picture = HOLMinimap::ToPictureAll(F, Ground);
	T.AssertEquals(4, Picture.Num(), "four corners");
	T.AssertEquals(75.0, Picture[0].X, "upper left x");
	T.AssertEquals(75.0, Picture[0].Y, "upper left y");
	T.AssertEquals(125.0, Picture[2].X, "lower right x");
	T.AssertEquals(125.0, Picture[2].Y, "lower right y");
}

// ---- Lot L17-05 (D830, D836, D839, GLORFINDEL-1117c): the script port of HOLMinimapAlertRingTests.cpp and HOLMinimapTerrainTests.cpp, against
// UHOLMinimapReadLibrary (HOLMinimapRead, S1h): AlertDiameter, GetSourcesSignature (parallel arrays Locations / Remaining, FHOLSource has no USTRUCT),
// GetConsoleFloat. Same vectors as the C++ tests.

bool MinimapNear(float A, float B, float Tolerance)
{
	return Math::Abs(A - B) <= Tolerance;
}

// HeroesOfLiberty.UI.Minimap.AlertRing (MM-Q1 / D624): the alert circle shrinks from its size (24 px) to 8 px, again and again, on the game's clock.
void Test_MinimapAlertRing(FUnitTest& T)
{
	T.AssertTrue(MinimapNear(HOLMinimapRead::AlertDiameter(0.0f, 1.0f, 24.0f), 24.0f, 0.001f), "age 0: the full size");
	T.AssertTrue(MinimapNear(HOLMinimapRead::AlertDiameter(0.5f, 1.0f, 24.0f), 16.0f, 0.001f), "half a period: halfway");
	T.AssertTrue(MinimapNear(HOLMinimapRead::AlertDiameter(0.999f, 1.0f, 24.0f), 8.0f, 0.05f), "just before the period: nearly 8 px");
	T.AssertTrue(MinimapNear(HOLMinimapRead::AlertDiameter(3.0f, 1.0f, 24.0f), 24.0f, 0.001f), "a period later: again the full size");
	T.AssertTrue(MinimapNear(HOLMinimapRead::AlertDiameter(3.0f, 6.0f, 24.0f), 16.0f, 0.001f), "a 6 s period, 3 s: halfway");
	// Sane against the console's values: a period of 0 would divide by zero; a size under 8 px never grows.
	const float ZeroPeriod = HOLMinimapRead::AlertDiameter(1.0f, 0.0f, 24.0f);
	T.AssertTrue(ZeroPeriod == ZeroPeriod && Math::Abs(ZeroPeriod) < 1.0e30f, "period 0 stays a number");
	T.AssertTrue(HOLMinimapRead::AlertDiameter(0.3f, 1.0f, 6.0f) <= 6.0f, "a size under 8 px does not grow past itself");
	T.AssertTrue(HOLMinimapRead::AlertDiameter(-1.0f, 1.0f, 24.0f) <= 24.0f, "a negative age is the start");
	// The user's choice (Q-minimap-alerts-vitesse, 2026-11-02): 24 px, one pass in 0.75 s, an alert lives 6 s, no blink.
	T.AssertTrue(MinimapNear(HOLMinimapRead::GetConsoleFloat("hol.Minimap.AlertShrinkSeconds"), 0.75f, 0.001f), "default pass of the shrinking");
	T.AssertTrue(MinimapNear(HOLMinimapRead::GetConsoleFloat("hol.Minimap.AlertSize"), 24.0f, 0.001f), "default size");
	T.AssertTrue(MinimapNear(HOLMinimapRead::GetConsoleFloat("hol.Minimap.AlertSeconds"), 6.0f, 0.001f), "default life");
	// Shrinking, strictly, over a period (the picture of a ring that moves).
	float Previous = 1000.0f;
	for (int Step = 0; Step < 4; ++Step)
	{
		const float Diameter = HOLMinimapRead::AlertDiameter(Step * 0.25f, 1.0f, 24.0f);
		T.AssertTrue(Diameter < Previous, "t = " + (Step * 0.25f) + " s is smaller than the one before");
		Previous = Diameter;
	}
}

// HeroesOfLiberty.UI.Minimap.TerrainSources (P2): the terrain layer is made again only when the standing sources change.
void Test_MinimapTerrainSources(FUnitTest& T)
{
	TArray<FVector> Places;
	Places.Add(FVector(0.0, 0.0, 0.0));
	Places.Add(FVector(500.0, 0.0, 0.0));
	Places.Add(FVector(0.0, 500.0, 0.0));
	TArray<int32> Left;
	Left.Add(100);
	Left.Add(100);
	Left.Add(100);
	const int64 Whole = HOLMinimapRead::GetSourcesSignature(Places, Left);
	T.AssertTrue(HOLMinimapRead::GetSourcesSignature(Places, Left) == Whole, "read twice: the same");

	Left[1] = 40;
	T.AssertTrue(HOLMinimapRead::GetSourcesSignature(Places, Left) == Whole, "gathered from, still standing: no change");

	Left[1] = 0;
	const int64 UsedUp = HOLMinimapRead::GetSourcesSignature(Places, Left);
	T.AssertTrue(UsedUp != Whole, "used up: changed");

	Left[2] = 0;
	T.AssertTrue(HOLMinimapRead::GetSourcesSignature(Places, Left) != UsedUp, "another used up: changed again");

	Left[1] = 100;
	Left[2] = 100;
	T.AssertTrue(HOLMinimapRead::GetSourcesSignature(Places, Left) == Whole, "standing again (a farm field): back to the whole");

	TArray<FVector> More = Places;
	TArray<int32> MoreLeft = Left;
	More.Add(FVector(900.0, 900.0, 0.0));
	MoreLeft.Add(100);
	T.AssertTrue(HOLMinimapRead::GetSourcesSignature(More, MoreLeft) != Whole, "a source added: changed");

	TArray<FVector> Moved = Places;
	Moved[2] = FVector(0.0, 800.0, 0.0);
	T.AssertTrue(HOLMinimapRead::GetSourcesSignature(Moved, Left) != Whole, "a source elsewhere: changed");

	TArray<FVector> NoPlace;
	TArray<int32> NoLeft;
	T.AssertTrue(HOLMinimapRead::GetSourcesSignature(NoPlace, NoLeft) == HOLMinimapRead::GetSourcesSignature(NoPlace, NoLeft), "no source: the empty signature, stable");
}
