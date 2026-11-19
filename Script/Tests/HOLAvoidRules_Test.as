// MIG-C-Mouvement (CELEBORN-1117k): the pure avoidance rules of HOLAvoidRules.h, tested through HOLMovementRules:: (UHOLMovementRulesLibrary, envelopes only).
// Port of HOLAvoidTests.cpp (same vectors): MarchPairNoTremble, DetourSide, HeadOnAndBlockedPlace, DetourTurn, CrossingRole, TimeToContact, NoClearWay,
// DetourFootprint, LoopPatience. The rules stay C++ (the Mass processors step them per entity); no C++ test is removed by this file.
// NEEDS THE C++ LOT (Source/HeroesOfLiberty/Simulation/HOLMovementRulesLibrary.*): compile only after the build.
// MarchPair: the C++ test reads the game's cvars (hol.Units.AvoidTime 0.5, AvoidMaxAngle 60 -> 6 turns, MarchMemberDetourSteps 2, D855): here their shipped
// defaults, written down (a script FConsoleVariable would create a NEW variable, not read the game's).

double AvdRotX(double X, double Y, double Degrees)
{
	const double A = Math::DegreesToRadians(Degrees);
	return X * Math::Cos(A) - Y * Math::Sin(A);
}

double AvdRotY(double X, double Y, double Degrees)
{
	const double A = Math::DegreesToRadians(Degrees);
	return X * Math::Sin(A) + Y * Math::Cos(A);
}

FVector2D AvdRot(double X, double Y, double Degrees)
{
	return FVector2D(AvdRotX(X, Y, Degrees), AvdRotY(X, Y, Degrees));
}

bool AvdNear(double A, double B, double Eps)
{
	return Math::Abs(A - B) <= Eps;
}

// ---- MarchPairNoTremble ----

struct FAvdMarchWalker
{
	double VX = 0.0;
	double VY = 0.0;
	int Side = 0;
	int Turn = 0;
	int ClearSteps = 0;
	int MemberSteps = 0;
}

// One step of the walker's avoidance (HOLAvoidTests.cpp StepMarchMember): the way (unit) as FVector2D.
FVector2D AvdStepMarchMember(FAvdMarchWalker& Me, const FAvdMarchWalker& Other, FVector2D ToSlot, FVector2D ToOther, double Speed, double Contact, double AvoidTime, int Turns, int MemberMaxSteps)
{
	const double AngleStep = 10.0;
	const FVector2D Ahead = ToSlot.GetSafeNormal();
	const double Horizon = Math::Min(AvoidTime, ToSlot.Size() / Speed);
	const double OtherSpeed = Math::Sqrt(Other.VX * Other.VX + Other.VY * Other.VY);
	const bool bNear = (ToOther.X * Ahead.X + ToOther.Y * Ahead.Y) > -Contact
		&& (ToOther.X * ToOther.X + ToOther.Y * ToOther.Y) < Math::Square((Speed + OtherSpeed) * Horizon + Contact);

	if (Me.Turn > 0 && !bNear)
		Me.ClearSteps = Math::Max(Me.ClearSteps, HOLMovementRules::EaseBackSteps() - 1);

	if (Me.Turn == 0 && AvdIsClear(bNear, Ahead, ToOther, Other, Speed, Contact, Horizon))
		return Ahead;

	if (Me.Turn > 0)
	{
		const double Hand = Me.Side;
		TArray<bool> Table;
		for (int Each = 0; Each <= Turns; Each++)
		{
			const FVector2D Probe = AvdRot(Ahead.X, Ahead.Y, Hand * Each * AngleStep);
			Table.Add(AvdIsClear(bNear, Probe, ToOther, Other, Speed, Contact, Horizon));
		}
		const int Turn = HOLMovementRules::NextDetourTurn(Me.Turn, Turns, Me.ClearSteps, Table);
		Me.Turn = Turn != -1 ? Turn : HOLMovementRules::TurnWithoutClearWay(Me.Turn, Turns, false);
		if (Me.Turn == 0)
		{
			Me.Side = 0;
			Me.Turn = 0;
			Me.ClearSteps = 0;
			Me.MemberSteps = 0;
			return Ahead;
		}
		const FVector2D Way = AvdRot(Ahead.X, Ahead.Y, Hand * Me.Turn * AngleStep);
		if (Me.MemberSteps > 0 && HOLMovementRules::IsMemberDetourOver(Me.MemberSteps, MemberMaxSteps))
		{
			Me.Side = 0;
			Me.Turn = 0;
			Me.ClearSteps = 0;
			Me.MemberSteps = 0;
		}
		return Way;
	}

	const double Side = HOLMovementRules::ChooseDetourSide(Ahead, ToOther, Contact * 1.1);
	for (int Index = 0; Index < 2 * Turns; Index++)
	{
		const int Turn = Index / 2 + 1;
		const double Hand = Index % 2 == 0 ? Side : -Side;
		const FVector2D Way = AvdRot(Ahead.X, Ahead.Y, Hand * Turn * AngleStep);
		if (AvdIsClear(bNear, Way, ToOther, Other, Speed, Contact, Horizon))
		{
			if (HOLMovementRules::RemembersMemberDetour(MemberMaxSteps))
			{
				Me.Side = int(Hand);
				Me.Turn = Turn;
				Me.MemberSteps = 1;
			}
			return Way;
		}
	}
	return Ahead;
}

bool AvdIsClear(bool bNear, FVector2D Way, FVector2D ToOther, const FAvdMarchWalker& Other, double Speed, double Contact, double Horizon)
{
	if (!bNear)
		return true;
	const FVector2D Relative(Way.X * Speed - Other.VX, Way.Y * Speed - Other.VY);
	return HOLMovementRules::TimeToContact(ToOther, Relative, Contact, false) >= Horizon;
}

FHOLSanityResultView AvdRunMarchPair(int MemberMaxSteps, bool bRight)
{
	const double Speed = 697.0;
	const double Contact = 120.0;
	const float Step = 1.f / 30.f;
	const double AvoidTime = 0.5;
	const int Turns = 6;
	const FVector2D LeftToSlot(130.0, 650.0);
	const FVector2D RightToSlot(-166.0, 650.0);
	const FVector2D LeftToRight(229.0, 0.0);
	const FVector2D RightToLeft(-229.0, 0.0);
	FAvdMarchWalker Left;
	FAvdMarchWalker Right;
	FHOLSanityTraceView Trace;
	double PX = 0.0;
	double PY = 0.0;
	for (int Index = 0; Index < 240; Index++)
	{
		const FAvdMarchWalker LastLeft = Left;
		const FAvdMarchWalker LastRight = Right;
		const FVector2D WayLeft = AvdStepMarchMember(Left, LastRight, LeftToSlot, LeftToRight, Speed, Contact, AvoidTime, Turns, MemberMaxSteps);
		Left.VX = WayLeft.X * Speed;
		Left.VY = WayLeft.Y * Speed;
		const FVector2D WayRight = AvdStepMarchMember(Right, LastLeft, RightToSlot, RightToLeft, Speed, Contact, AvoidTime, Turns, MemberMaxSteps);
		Right.VX = WayRight.X * Speed;
		Right.VY = WayRight.Y * Speed;
		PX += (bRight ? Right.VX : Left.VX) * Step;
		PY += (bRight ? Right.VY : Left.VY) * Step;
		Trace.Places.Add(FVector2D(PX, PY));
		Trace.Ordered.Add(1);
	}
	return HOLMovementRules::Analyse(Trace, Step, FHOLSanitySettingsView());
}

void Test_AvoidMarchPairNoTremble(FUnitTest& T)
{
	const FHOLSanityResultView Before = AvdRunMarchPair(0, false);
	T.AssertTrue(Before.bShake, "the old rule (never remembered) trembles, as on the real chain (to and fro " + Before.Zigzag + " rad in 8 s)");
	const int MemberMaxSteps = 2;
	for (int Who = 0; Who < 2; Who++)
	{
		const bool bRight = Who == 1;
		const FHOLSanityResultView Now = AvdRunMarchPair(MemberMaxSteps, bRight);
		const FString Name = bRight ? "#10" : "#9";
		T.AssertFalse(Now.bShake, Name + " does not tremble with MarchMemberDetourSteps 2 (to and fro " + Now.Zigzag + " rad in 8 s, turn " + Now.Turn + ")");
		T.AssertTrue(Now.Net > 0.9f * 697.f * 8.f, Name + " still marches (net " + Now.Net + " cm in 8 s)");
	}
}

// ---- MV.2 ----

void Test_AvoidDetourSide(FUnitTest& T)
{
	const double Contact = 130.0;
	T.AssertTrue(AvdNear(HOLMovementRules::ChooseDetourSide(FVector2D(1.0, 0.0), FVector2D(300.0, 60.0), Contact), -1.0, 0.0), "a body on the right: turn left");
	T.AssertTrue(AvdNear(HOLMovementRules::ChooseDetourSide(FVector2D(1.0, 0.0), FVector2D(300.0, -60.0), Contact), 1.0, 0.0), "a body on the left: turn right");
	const double First = HOLMovementRules::ChooseDetourSide(FVector2D(1.0, 0.0), FVector2D(300.0, 5.0), Contact);
	const double Second = HOLMovementRules::ChooseDetourSide(FVector2D(-1.0, 0.0), FVector2D(-300.0, 5.0), Contact);
	T.AssertTrue(First == 1.0 && Second == 1.0, "face to face: both on their own right");
	T.AssertTrue(AvdNear(HOLMovementRules::ChooseDetourSide(FVector2D(1.0, 0.0), FVector2D(300.0, -5.0), Contact), 1.0, 0.0), "face to face the other way off the line: on its own right");
}

void Test_AvoidHeadOnAndBlockedPlace(FUnitTest& T)
{
	const FVector2D East(1.0, 0.0);
	T.AssertTrue(HOLMovementRules::IsHeadOn(East, FVector2D(200.0, 20.0), FVector2D(-650.0, 0.0), 360.0, 100.0), "a friend ahead walking back at 650 cm/s is head on");
	T.AssertFalse(HOLMovementRules::IsHeadOn(East, FVector2D(200.0, 20.0), FVector2D(650.0, 0.0), 360.0, 100.0), "a friend ahead walking the same way is not");
	T.AssertFalse(HOLMovementRules::IsHeadOn(East, FVector2D(200.0, 20.0), FVector2D(0.0, 0.0), 360.0, 100.0), "a friend ahead standing is not");
	T.AssertFalse(HOLMovementRules::IsHeadOn(East, FVector2D(-200.0, 20.0), FVector2D(650.0, 0.0), 360.0, 100.0), "a friend behind is not");
	T.AssertFalse(HOLMovementRules::IsHeadOn(East, FVector2D(500.0, 0.0), FVector2D(-650.0, 0.0), 360.0, 100.0), "a friend too far is not");
	T.AssertFalse(HOLMovementRules::IsHeadOn(East, FVector2D(200.0, 20.0), FVector2D(0.0, -650.0), 360.0, 100.0), "a friend crossing sideways is not");
	T.AssertTrue(HOLMovementRules::IsPlaceInBody(FVector2D(-10312.0, -55.0), FVector2D(-10312.0, -55.0), 60.0), "a place on a body's centre is in it");
	T.AssertTrue(HOLMovementRules::IsPlaceInBody(FVector2D(-10066.0, -216.0), FVector2D(-10066.0, -176.0), 60.0), "a place 40 cm from a body is in it");
	T.AssertFalse(HOLMovementRules::IsPlaceInBody(FVector2D(0.0, 0.0), FVector2D(80.0, 0.0), 60.0), "a place 80 cm from a body is not");
	T.AssertTrue(HOLMovementRules::IsSlotBlockedSeen(1, 7, 3, 7, 3), "a flag written on this order and leg counts");
	T.AssertFalse(HOLMovementRules::IsSlotBlockedSeen(1, 6, 3, 7, 3), "a flag of a previous order does not");
	T.AssertFalse(HOLMovementRules::IsSlotBlockedSeen(1, 7, 2, 7, 3), "a flag of a previous leg does not");
	T.AssertFalse(HOLMovementRules::IsSlotBlockedSeen(0, 7, 3, 7, 3), "no flag, no block");
}

// IsClear(Turn) = Turn >= Smallest, as a table over 0..8 (the C++ lambda; a turn past the table is not clear).
int AvdNext(int Remembered, int Smallest, int& Clear)
{
	TArray<bool> Table;
	for (int Turn = 0; Turn <= 8; Turn++)
		Table.Add(Turn >= Smallest);
	return HOLMovementRules::NextDetourTurn(Remembered, 6, Clear, Table);
}

void Test_AvoidDetourTurn(FUnitTest& T)
{
	int Clear = 0;
	bool bHeld = true;
	for (int Step = 1; Step < HOLMovementRules::EaseBackSteps(); Step++)
		bHeld = bHeld && AvdNext(4, 1, Clear) == 4;
	T.AssertTrue(bHeld, "held while the eased course has not been clear long enough");
	T.AssertEquals(3, AvdNext(4, 1, Clear), "eases back one turn after a run of clear steps");
	T.AssertEquals(0, Clear, "the run starts again after easing back");
	Clear = HOLMovementRules::EaseBackSteps() - 1;
	T.AssertEquals(3, AvdNext(3, 3, Clear), "a step not clear holds the turn");
	T.AssertEquals(0, Clear, "and starts the run again");
	Clear = HOLMovementRules::EaseBackSteps() - 1;
	T.AssertEquals(4, AvdNext(5, 2, Clear), "never more than one turn back a step");
	Clear = HOLMovementRules::EaseBackSteps() - 1;
	T.AssertEquals(0, AvdNext(1, 0, Clear), "the last turn back is the straight course");
	Clear = 2;
	T.AssertEquals(5, AvdNext(3, 5, Clear), "wider at once when the remembered course is not clear");
	T.AssertEquals(0, Clear, "widening starts the run again");
	Clear = 0;
	TArray<bool> OnlyTwo;
	for (int Turn = 0; Turn <= 8; Turn++)
		OnlyTwo.Add(Turn == 2);
	T.AssertEquals(2, HOLMovementRules::NextDetourTurn(3, 6, Clear, OnlyTwo), "narrower at once when only the eased course is clear");
	Clear = 0;
	T.AssertEquals(-1, AvdNext(2, 7, Clear), "nothing clear up to the largest turn (INDEX_NONE)");
}

int AvdRole(FVector2D MyPosition, FVector2D MyVelocity, FVector2D OtherPosition, FVector2D OtherVelocity)
{
	return HOLMovementRules::ChooseCrossingRole(MyPosition, MyVelocity, OtherPosition, OtherVelocity).Role;
}

void Test_AvoidCrossingRole(FUnitTest& T)
{
	const int None = 0;
	const int KeepCourse = 1;
	const int PassBehind = 2;
	const FVector2D APosition(0.0, 0.0);
	const FVector2D AVelocity(100.0, 0.0);
	const FVector2D BVelocity(0.0, 100.0);
	const FVector2D BLate(500.0, -800.0);
	T.AssertEquals(KeepCourse, AvdRole(APosition, AVelocity, BLate, BVelocity), "first: keeps its course");
	const FHOLCrossingChoiceView Late = HOLMovementRules::ChooseCrossingRole(BLate, BVelocity, APosition, AVelocity);
	T.AssertTrue(Late.Role == PassBehind && Late.Side == 1.0, "second: passes behind on the side the other comes from");
	const FVector2D BEarly(500.0, -300.0);
	const FHOLCrossingChoiceView Second = HOLMovementRules::ChooseCrossingRole(APosition, AVelocity, BEarly, BVelocity);
	T.AssertTrue(Second.Role == PassBehind && Second.Side == -1.0, "second the other way: passes behind on its left");
	T.AssertEquals(KeepCourse, AvdRole(BEarly, BVelocity, APosition, AVelocity), "first the other way: keeps its course");
	const FVector2D BTogether(500.0, -500.0);
	const FVector2D BNearly(500.0, -510.0);
	T.AssertEquals(KeepCourse, AvdRole(APosition, AVelocity, BTogether, BVelocity), "together: the one with the other on its left keeps its course");
	const FHOLCrossingChoiceView Right = HOLMovementRules::ChooseCrossingRole(BTogether, BVelocity, APosition, AVelocity);
	T.AssertTrue(Right.Role == PassBehind && Right.Side == 1.0, "together: the one with the other on its right passes behind");
	T.AssertTrue(AvdRole(APosition, AVelocity, BNearly, BVelocity) == KeepCourse && AvdRole(BNearly, BVelocity, APosition, AVelocity) == PassBehind, "nearly together: priority to the right still");
	T.AssertEquals(KeepCourse, AvdRole(FVector2D(600.0, 0.0), AVelocity, BEarly, BVelocity), "past the point: keeps its course");
	T.AssertEquals(PassBehind, AvdRole(BEarly, BVelocity, FVector2D(600.0, 0.0), AVelocity), "the other past the point: passes behind");
	T.AssertEquals(None, AvdRole(APosition, AVelocity, FVector2D(1000.0, 10.0), FVector2D(-100.0, 0.0)), "face to face: none");
	T.AssertEquals(None, AvdRole(APosition, AVelocity, FVector2D(1000.0, 100.0), AvdRot(-100.0, 0.0, 20.0)), "nearly face to face (20 degrees): none");
	T.AssertEquals(None, AvdRole(APosition, AVelocity, FVector2D(300.0, 10.0), FVector2D(50.0, 0.0)), "overtaking: none");
	T.AssertEquals(None, AvdRole(APosition, AVelocity, FVector2D(300.0, 10.0), FVector2D(0.0, 0.0)), "standing: none");
	T.AssertEquals(None, AvdRole(FVector2D(600.0, 0.0), AVelocity, FVector2D(500.0, 100.0), BVelocity), "both past the point: none");
}

// ---- MV.4 ----

void Test_AvoidTimeToContact(FUnitTest& T)
{
	const double Contact = 100.0;
	const double Never = HOLMovementRules::TimeNever();
	T.AssertTrue(AvdNear(HOLMovementRules::TimeToContact(FVector2D(300.0, 0.0), FVector2D(100.0, 0.0), Contact, true), 2.0, 1.e-6), "ahead out of contact: met when the gap is closed");
	T.AssertTrue(HOLMovementRules::TimeToContact(FVector2D(80.0, 0.0), FVector2D(100.0, 0.0), Contact, true) == 0.0, "in contact, closing in: met now");
	T.AssertTrue(HOLMovementRules::TimeToContact(FVector2D(80.0, 0.0), FVector2D(20.0, 100.0), Contact, true) == 0.0, "in contact, sliding along it at a slant inwards: met now");
	T.AssertTrue(HOLMovementRules::TimeToContact(FVector2D(80.0, 0.0), FVector2D(-100.0, 0.0), Contact, true) == Never, "in contact, moving away: never");
	T.AssertTrue(HOLMovementRules::TimeToContact(FVector2D(80.0, 0.0), FVector2D(0.0, 100.0), Contact, true) == Never, "in contact, along it: never");
	T.AssertTrue(HOLMovementRules::TimeToContact(FVector2D(300.0, 200.0), FVector2D(100.0, 0.0), Contact, true) == Never, "passing wide: never");
	T.AssertTrue(HOLMovementRules::TimeToContact(FVector2D(80.0, 0.0), FVector2D(100.0, 0.0), Contact, false) == Never, "a friend in contact, closing in: never (it gives way)");
	T.AssertTrue(AvdNear(HOLMovementRules::TimeToContact(FVector2D(300.0, 0.0), FVector2D(100.0, 0.0), Contact, false), 2.0, 1.e-6), "a friend ahead out of contact: met as an enemy");
	T.AssertTrue(HOLMovementRules::TimeToContact(FVector2D(80.0, 0.0), FVector2D(0.0, 0.0), Contact, true) == Never, "standing: never");
}

void Test_AvoidNoClearWay(FUnitTest& T)
{
	T.AssertEquals(4, HOLMovementRules::TurnWithoutClearWay(3, 6, true), "against an enemy: one turn wider");
	T.AssertEquals(6, HOLMovementRules::TurnWithoutClearWay(6, 6, true), "against an enemy: held at the largest turn");
	T.AssertEquals(2, HOLMovementRules::TurnWithoutClearWay(3, 6, false), "against a friend: one turn back, pushing");
}

void Test_AvoidDetourFootprint(FUnitTest& T)
{
	FHOLFootprintBoxView House;
	House.HalfExtent = FVector2D(200.0, 200.0);
	const double Keep = 40.0;
	T.AssertTrue(AvdNear(HOLMovementRules::GetFootprintDistance(House, Keep, FVector2D(0.0, -300.0)), 60.0, 1.e-6), "distance out of the wall, beyond the keep");
	T.AssertTrue(HOLMovementRules::GetFootprintDistance(House, Keep, FVector2D(0.0, 0.0)) < 0.0, "inside: negative");
	T.AssertFalse(HOLMovementRules::LeadsIntoFootprint(House, Keep, FVector2D(-1000.0, -240.0), FVector2D(1.0, 0.0), 450.0), "along the wall: not led in");
	T.AssertTrue(HOLMovementRules::LeadsIntoFootprint(House, Keep, FVector2D(-100.0, -240.0), AvdRot(1.0, 0.0, 30.0), 450.0), "turned towards the wall: led in");
	T.AssertFalse(HOLMovementRules::LeadsIntoFootprint(House, Keep, FVector2D(-100.0, -240.0), AvdRot(1.0, 0.0, -30.0), 450.0), "turned away from the wall: not led in");
	T.AssertFalse(HOLMovementRules::LeadsIntoFootprint(House, Keep, FVector2D(-3000.0, 0.0), FVector2D(1.0, 0.0), 450.0), "far from it: not led in");
	FHOLFootprintBoxView Turned = House;
	Turned.HalfExtent = FVector2D(100.0, 300.0);
	Turned.Cos = 0.0;
	Turned.Sin = 1.0;
	T.AssertTrue(AvdNear(HOLMovementRules::GetFootprintDistance(Turned, 0.0, FVector2D(0.0, -150.0)), 50.0, 1.e-6), "turned: its long side along x");
}

bool AvdLoopStep(FHOLLoopPatienceView& State, double Degrees, double Speed)
{
	return HOLMovementRules::StepLoopPatience(State, AvdRot(Speed, 0.0, Degrees), 0.05, 270.0, 2.5, 4.0);
}

void Test_AvoidLoopPatience(FUnitTest& T)
{
	{
		FHOLLoopPatienceView State;
		bool bHeld = false;
		for (int Index = 0; Index < 200; Index++)
			bHeld = AvdLoopStep(State, 0.0, 697.0) || bHeld;
		T.AssertFalse(bHeld, "a straight walk never waits");
	}
	{
		FHOLLoopPatienceView State;
		bool bHeld = false;
		for (int Index = 0; Index < 80; Index++)
		{
			double Degrees = 0.0;
			if (Index < 10)
				Degrees = Index * 6.0;
			else if (Index < 40)
				Degrees = 60.0;
			else if (Index < 50)
				Degrees = 60.0 - (Index - 40) * 6.0;
			bHeld = AvdLoopStep(State, Degrees, 697.0) || bHeld;
		}
		T.AssertFalse(bHeld, "a detour out and back does not wait");
	}
	{
		FHOLLoopPatienceView State;
		int HeldAt = -1;
		for (int Index = 0; Index < 40 && HeldAt == -1; Index++)
		{
			if (AvdLoopStep(State, Index * 12.0, 697.0))
				HeldAt = Index;
		}
		T.AssertTrue(HeldAt != -1, "a loop makes it wait");
		T.AssertTrue(HeldAt != -1 && HeldAt * 12 < 330, "before it closes the circle");
		int Steps = 0;
		while (AvdLoopStep(State, 0.0, 697.0) && Steps < 200)
			Steps++;
		T.AssertTrue(Steps >= 49 && Steps <= 51, "the wait lasts 2.5 s");
		T.AssertFalse(AvdLoopStep(State, 0.0, 697.0), "then it walks");
	}
	{
		FHOLLoopPatienceView State;
		bool bHeld = false;
		for (int Index = 0; Index < 30; Index++)
			bHeld = AvdLoopStep(State, (Index / 4) % 2 == 0 ? 0.0 : 180.0, 697.0) || bHeld;
		T.AssertTrue(bHeld, "a sway to and fro makes it wait");
	}
	{
		FHOLLoopPatienceView State;
		bool bHeld = false;
		for (int Index = 0; Index < 100; Index++)
			bHeld = AvdLoopStep(State, Index * 30.0, 10.0) || bHeld;
		T.AssertFalse(bHeld, "standing piles no turn");
		FHOLLoopPatienceView Off;
		bool bOffHeld = false;
		for (int Index = 0; Index < 100; Index++)
			bOffHeld = HOLMovementRules::StepLoopPatience(Off, AvdRot(697.0, 0.0, Index * 40.0), 0.05, 0.0, 2.5, 4.0) || bOffHeld;
		T.AssertFalse(bOffHeld, "0 degrees: off");
	}
}
