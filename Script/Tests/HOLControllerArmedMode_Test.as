// CLAUDE-1125s (point 3, D839: a new test is written in script; ported from HOLFactionAimTests.cpp, FARAMIR-1120e / GALADRIEL-1120f / CLAUDE-1125s):
// the controller's own state, on a controller born in the test world (HOLTestWorld::SpawnPlayerController: no BeginPlay, no viewport).
// 1. The aim of a faction power: SetFactionAim(i, range) arms power i (-1: none); Escape (the real HandleDeselect, through ScriptEscape) and a right
//    click (any other order, ScriptRightClick) leave it; no click came in, none to consume.
// 2. Since G-5a (a493984) the grid arms the attack-move and the banner through SetArmedMode, as the script numbers them
//    (HOLCommandGridAction::GetAimAttackMove 100, GetAimBanner 101): IsAttackMoveArmed / IsBannerArmed read that mode, and the cursor PlayerTick
//    applies (GetWantedMouseCursorIndex) is the red attack cross while the attack-move waits for its point (A.2).

// HeroesOfLiberty.ControllerArmedMode.AimStateAndEscape
void Test_ControllerAimStateAndEscape(FUnitTest& T)
{
	UHOLTestWorldHandle TestWorld = HOLTestWorld::CreateTestWorld();
	T.AssertTrue(TestWorld != nullptr, "the test world");
	if (TestWorld == nullptr)
		return;
	AHOLPlayerController Controller = HOLTestWorld::SpawnPlayerController(TestWorld);
	T.AssertTrue(Controller != nullptr, "the controller");
	if (Controller != nullptr)
	{
		T.AssertEquals(-1, Controller.GetFactionAim(), "no aim at first");

		Controller.SetFactionAim(2, 800.0);
		T.AssertEquals(2, Controller.GetFactionAim(), "the aim of power 2 is armed");
		T.AssertTrue(Controller.GetFactionAimRangeCm() == 800.0, "with the reach it was given");

		Controller.SetFactionAim(0, 650.0);
		T.AssertEquals(0, Controller.GetFactionAim(), "aiming another power switches the aim");
		T.AssertTrue(Controller.GetFactionAimRangeCm() == 650.0, "and its reach");

		FVector Point = FVector::ZeroVector;
		int Index = -7;
		T.AssertFalse(Controller.ConsumeFactionAimClick(Point, Index), "no click came in, none to consume");

		Controller.ScriptEscape();
		T.AssertEquals(-1, Controller.GetFactionAim(), "Escape leaves the aim");

		Controller.SetFactionAim(1, 800.0);
		Controller.SetFactionAim(-1, 800.0);
		T.AssertEquals(-1, Controller.GetFactionAim(), "SetFactionAim(-1) leaves the aim");

		Controller.SetFactionAim(1, 800.0);
		Controller.ScriptRightClick();
		T.AssertEquals(-1, Controller.GetFactionAim(), "a right click (any other order) leaves the aim");
	}
	HOLTestWorld::DestroyTestWorld(TestWorld);
}

// HeroesOfLiberty.ControllerArmedMode.Cursor
void Test_ControllerArmedModeCursor(FUnitTest& T)
{
	UHOLTestWorldHandle TestWorld = HOLTestWorld::CreateTestWorld();
	T.AssertTrue(TestWorld != nullptr, "the test world");
	if (TestWorld == nullptr)
		return;
	AHOLPlayerController Controller = HOLTestWorld::SpawnPlayerController(TestWorld);
	T.AssertTrue(Controller != nullptr, "the controller");
	if (Controller != nullptr)
	{
		const int Arrow = HOLCommandGridRead::GetCursorDefault();
		const int Cross = HOLCommandGridRead::GetCursorCrosshairs();
		T.AssertTrue(Cross != Arrow, "the cross is not the arrow");
		T.AssertEquals(Arrow, Controller.GetWantedMouseCursorIndex(), "nothing armed: the arrow");

		Controller.SetArmedMode(HOLCommandGridAction::GetAimAttackMove(), 0.0, 0);
		T.AssertTrue(Controller.IsAttackMoveArmed(), "the grid's attack-move is the attack-move armed");
		T.AssertFalse(Controller.IsBannerArmed(), "it is not the banner");
		T.AssertEquals(Cross, Controller.GetWantedMouseCursorIndex(), "Q waits for its point: the red attack cross");

		Controller.SetArmedMode(HOLCommandGridAction::GetAimBanner(), 1600.0, 0);
		T.AssertTrue(Controller.IsBannerArmed(), "the grid's banner is the banner armed");
		T.AssertFalse(Controller.IsAttackMoveArmed(), "arming the banner leaves the attack-move");
		T.AssertEquals(Arrow, Controller.GetWantedMouseCursorIndex(), "the banner aims with the arrow");

		Controller.SetArmedMode(HOLCommandGridAction::GetAimAttackMove(), 0.0, 0);
		Controller.ScriptEscape();
		T.AssertFalse(Controller.IsAttackMoveArmed(), "Escape leaves the attack-move");
		T.AssertEquals(Arrow, Controller.GetWantedMouseCursorIndex(), "and the arrow comes back");
	}
	HOLTestWorld::DestroyTestWorld(TestWorld);
}
