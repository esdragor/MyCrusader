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
	T.AssertEquals(HOLMinimap::ClickCamera, HOLMinimap::ClickAction(false, false), "left, nothing selected: camera");
	T.AssertEquals(HOLMinimap::ClickCamera, HOLMinimap::ClickAction(false, true), "left, units selected: camera");
	T.AssertEquals(HOLMinimap::ClickMove, HOLMinimap::ClickAction(true, true), "right, units selected: move");
	T.AssertEquals(HOLMinimap::ClickNothing, HOLMinimap::ClickAction(true, false), "right, nothing to order: nothing");
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
	HOLMinimap::Age(Alerts, 3.0);
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
	T.AssertTrue(HOLMinimap::IsLit(Alerts[0]), "lit at once");
	HOLMinimap::Age(Alerts, HOLMinimap::BlinkSeconds + 0.01);
	T.AssertEquals(1, Alerts.Num(), "still there");
	T.AssertFalse(HOLMinimap::IsLit(Alerts[0]), "dark in the second half-period");
	HOLMinimap::Age(Alerts, HOLMinimap::BlinkSeconds);
	T.AssertTrue(HOLMinimap::IsLit(Alerts[0]), "lit again");
	HOLMinimap::Age(Alerts, HOLMinimap::AlertSeconds);
	T.AssertEquals(0, Alerts.Num(), "gone after its time");
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
