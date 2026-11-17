// MIG-C-Mouvement (CELEBORN-1117k): the sanity audit's rules of HOLSanityRules.h, tested through HOLMovementRules:: (UHOLMovementRulesLibrary, envelopes only).
// Port of HOLSanityTests.cpp (same vectors): Spin, Shake, Motion, StallFinalLeg, NoStrike, Colour. NOT PORTED YET: Sanity.Stall (lines 243-515 of the C++ test:
// HasFreePlaceAround / IsBoxedIn of HOLCombat, bodies rings) stays C++ and is registered in Docs/Rapports/ARWEN-1117k.md.
// The rules stay C++ (the audit and the Mass processors read the same traces); no C++ test is removed by this file.
// NEEDS THE C++ LOT (Source/HeroesOfLiberty/Simulation/HOLMovementRulesLibrary.*): compile only after the build.

// ---- Spin / Shake (HOLSanity::FUnitTrack through JudgeTrack, 30 Hz) ----
// Kinds of place, as the C++ lambdas: 0 straight walk 400 cm/s, 1 a detour, 2 circle of 100 cm at 400 cm/s, 3 back and forth 4 m each way (0.8 s),
// 4 standing, 5 pushed 30 cm/s, 6 settling 0.5 cm, 7 tremor of 3 cm a step.

FVector2D SanPlace(int Kind, double T, int Sample)
{
	const double Speed = 400.0;
	if (Kind == 0)
		return FVector2D(Speed * T, 0.0);
	if (Kind == 1)
	{
		if (T < 2.0)
			return FVector2D(Speed * T, 0.0);
		if (T < 3.0)
			return FVector2D(Speed * 2.0, Speed * (T - 2.0));
		return FVector2D(Speed * 2.0 + Speed * (T - 3.0), Speed);
	}
	if (Kind == 2)
	{
		const double Angle = Speed * T / 100.0;
		return FVector2D(100.0 * Math::Cos(Angle), 100.0 * Math::Sin(Angle));
	}
	if (Kind == 3)
	{
		const double Phase = T - 0.8 * Math::FloorToInt(T / 0.8);
		return FVector2D(Phase < 0.4 ? 800.0 * Phase : 800.0 * (0.8 - Phase), 0.0);
	}
	if (Kind == 4)
		return FVector2D(100.0, 100.0);
	if (Kind == 5)
		return FVector2D(100.0 + 30.0 * T, 100.0);
	if (Kind == 6)
		return FVector2D(100.0 + 0.5 * Math::Sin(T * 60.0), 100.0);
	return FVector2D(100.0 + ((Sample & 1) == 1 ? 3.0 : -3.0), 100.0);
}

// Motion: 0 Ordered, 1 Idle, 2 Busy.
FString SanJudge(int Motion, double Seconds, int Kind)
{
	TArray<double> Times;
	TArray<FVector2D> Places;
	int Sample = 0;
	for (double Time = 0.0; Time <= Seconds; Time += 1.0 / 30.0)
	{
		Times.Add(Time);
		Places.Add(SanPlace(Kind, Time, Sample));
		Sample++;
	}
	return HOLMovementRules::JudgeTrack(Motion, Times, Places);
}

void Test_SanitySpin(FUnitTest& T)
{
	T.AssertTrue(SanJudge(0, 10.0, 0).IsEmpty(), "straight walk is sound");
	T.AssertTrue(SanJudge(0, 10.0, 1).IsEmpty(), "a detour is sound");
	const FString Spin = SanJudge(0, 10.0, 2);
	T.AssertTrue(Spin.StartsWith("spins"), "a circle with an order is found: " + Spin);
	const FString Zigzag = SanJudge(0, 10.0, 3);
	T.AssertTrue(Zigzag.StartsWith("spins"), "back and forth with an order is found: " + Zigzag);
	T.AssertTrue(SanJudge(2, 10.0, 2).IsEmpty(), "a circle on a unit busy with something else is not a spin");
}

void Test_SanityShake(FUnitTest& T)
{
	T.AssertTrue(SanJudge(1, 6.0, 4).IsEmpty(), "standing still is sound");
	T.AssertTrue(SanJudge(1, 6.0, 5).IsEmpty(), "being pushed along is sound");
	T.AssertTrue(SanJudge(1, 6.0, 6).IsEmpty(), "a tiny settling is sound");
	const FString Shake = SanJudge(1, 6.0, 7);
	T.AssertTrue(Shake.StartsWith("shakes"), "a tremor of 3 cm a step is found: " + Shake);
	T.AssertFalse(SanJudge(0, 6.0, 7).StartsWith("shakes"), "the same on a unit with an order is not a tremor");
}

// ---- Motion (HOLSanity::Analyse, 20 Hz, traces made by hand) ----
// Kinds: 0 walk 20 cm a step, 1 circle 100 cm at 0.25 rad a step, 2 tremble 2 cm to and fro, 3 wobble 35 ahead +14/0, 4 weave (S of +-0.8 rad every 24 steps),
// 5 corner at step 80, 6 fidget on a 2 cm circle, 7 loop on 80 cm.

FHOLSanityTraceView SanMake(int Kind, bool bOrdered, int Steps)
{
	FHOLSanityTraceView Trace;
	double WX = 0.0;
	double WY = 0.0;
	for (int I = 0; I < Steps; I++)
	{
		FVector2D Place;
		const double A = 0.25 * I;
		if (Kind == 0)
			Place = FVector2D(20.0 * I, 0.0);
		else if (Kind == 1)
			Place = FVector2D(100.0 * Math::Cos(A), 100.0 * Math::Sin(A));
		else if (Kind == 2)
			Place = FVector2D((I % 2) == 1 ? 2.0 : 0.0, 0.0);
		else if (Kind == 3)
			Place = FVector2D(35.0 * I, (I % 2) == 1 ? 14.0 : 0.0);
		else if (Kind == 4)
		{
			if (I > 0)
			{
				const double Heading = 0.8 * Math::Sin(2.0 * 3.14159265358979323846 * (I - 1) / 24.0);
				WX += 35.0 * Math::Cos(Heading);
				WY += 35.0 * Math::Sin(Heading);
			}
			Place = FVector2D(WX, WY);
		}
		else if (Kind == 5)
			Place = I < 80 ? FVector2D(20.0 * I, 0.0) : FVector2D(1580.0, 20.0 * (I - 80));
		else if (Kind == 6)
			Place = FVector2D(2.0 * Math::Cos(A), 2.0 * Math::Sin(A));
		else
			Place = FVector2D(80.0 * Math::Cos(A), 80.0 * Math::Sin(A));
		Trace.Places.Add(Place);
		Trace.Ordered.Add(bOrdered ? 1 : 0);
	}
	return Trace;
}

FHOLSanityResultView SanAnalyse(FHOLSanityTraceView Trace)
{
	return HOLMovementRules::Analyse(Trace, 0.05f, FHOLSanitySettingsView());
}

FHOLSanityTraceView SanFlank(int Kind, bool bFlank)
{
	FHOLSanityTraceView Trace = SanMake(Kind, true, 160);
	for (int I = 0; I < Trace.Places.Num(); I++)
		Trace.Flanking.Add(bFlank ? 1 : 0);
	return Trace;
}

FHOLSanityTraceView SanPlaceWalking(int Kind)
{
	FHOLSanityTraceView Trace = SanMake(Kind, true, 160);
	for (int I = 0; I < Trace.Places.Num(); I++)
		Trace.PlaceWalk.Add(1);
	return Trace;
}

void SanMove(TArray<double>& Lengths, TArray<double>& Degrees, double Length, double Angle)
{
	Lengths.Add(Length);
	Degrees.Add(Angle);
}

void Test_SanityMotion(FUnitTest& T)
{
	const FHOLSanityResultView Walk = SanAnalyse(SanMake(0, true, 160));
	T.AssertFalse(Walk.bSpin, "a straight walk does not spin");
	T.AssertFalse(Walk.bShake, "a straight walk does not shake");
	const FHOLSanityResultView Round = SanAnalyse(SanMake(1, true, 160));
	T.AssertTrue(Round.bSpin, "an ordered unit going round in a circle spins (turn " + Round.Turn + ")");
	T.AssertFalse(SanAnalyse(SanMake(1, false, 160)).bSpin, "the same circle with no order is not a spin");
	const FHOLSanityResultView Tremble = SanAnalyse(SanMake(2, false, 160));
	T.AssertTrue(Tremble.bShake, "a body going to and fro by small steps shakes (" + Tremble.Reversals + " reversals)");
	const FHOLSanityResultView Wobble = SanAnalyse(SanMake(3, true, 160));
	T.AssertTrue(Wobble.bShake, "a walk whose heading wobbles at each step shakes (turn " + Wobble.Turn + " rad)");
	const FHOLSanityResultView Slalom = SanAnalyse(SanMake(4, true, 160));
	T.AssertTrue(Slalom.Turn / 8.f >= 2.f, "the weave turns a lot (" + Slalom.Turn + " rad), as the old rule judged it");
	T.AssertFalse(Slalom.bShake, "a weave through a crowd does not shake (turn " + Slalom.Turn + ", to and fro " + Slalom.Zigzag + " rad)");
	T.AssertTrue(Wobble.Zigzag > 0.9f * Wobble.Turn, "a wobble is mostly to and fro (" + Wobble.Zigzag + " of " + Wobble.Turn + " rad)");

	// A man at arms jostled in a crowd (steps 180-219 of unit #11): its turns cancel, it is not a spin.
	TArray<double> Lengths;
	TArray<double> Degrees;
	for (int I = 0; I < 4; I++)
	{
		Lengths.Add(0.0);
		Degrees.Add(0.0);
	}
	SanMove(Lengths, Degrees, 10.0, -36.0);
	SanMove(Lengths, Degrees, 21.0, -32.0);
	SanMove(Lengths, Degrees, 24.0, -22.0);
	SanMove(Lengths, Degrees, 31.0, 23.0);
	SanMove(Lengths, Degrees, 35.0, 8.0);
	SanMove(Lengths, Degrees, 35.0, -1.0);
	for (int I = 0; I < 19; I++)
	{
		Lengths.Add(0.0);
		Degrees.Add(0.0);
	}
	SanMove(Lengths, Degrees, 10.0, 58.0);
	SanMove(Lengths, Degrees, 19.0, 36.0);
	SanMove(Lengths, Degrees, 25.0, 24.0);
	SanMove(Lengths, Degrees, 29.0, 174.0);
	SanMove(Lengths, Degrees, 13.0, -160.0);
	SanMove(Lengths, Degrees, 19.0, 167.0);
	SanMove(Lengths, Degrees, 27.0, 150.0);
	SanMove(Lengths, Degrees, 14.0, 175.0);
	SanMove(Lengths, Degrees, 21.0, 149.0);
	SanMove(Lengths, Degrees, 29.0, 135.0);
	SanMove(Lengths, Degrees, 34.0, 118.0);
	FHOLSanityTraceView Jostled;
	double JX = 0.0;
	double JY = 0.0;
	Jostled.Places.Add(FVector2D(JX, JY));
	Jostled.Ordered.Add(1);
	for (int I = 0; I < Lengths.Num(); I++)
	{
		JX += Lengths[I] * Math::Cos(Math::DegreesToRadians(Degrees[I]));
		JY += Lengths[I] * Math::Sin(Math::DegreesToRadians(Degrees[I]));
		Jostled.Places.Add(FVector2D(JX, JY));
		Jostled.Ordered.Add(1);
	}
	const FHOLSanityResultView Jostle = SanAnalyse(Jostled);
	T.AssertTrue(Jostle.Turn >= 2.f * 3.14159265f, "the jostled unit does turn a lot (" + Jostle.Turn + " rad), as the old rule judged it");
	T.AssertFalse(Jostle.bSpin, "a unit jostled to and fro in a crowd does not spin");

	const FHOLSanityResultView Corner = SanAnalyse(SanMake(5, true, 160));
	T.AssertFalse(Corner.bSpin, "a walk round a corner does not spin");
	T.AssertFalse(Corner.bShake, "a walk round a corner does not shake");

	// Q-flank-demi-tour = A.
	T.AssertFalse(SanAnalyse(SanFlank(1, true)).bSpin, "a circle made all along on a flank is not a spin");
	T.AssertFalse(SanAnalyse(SanFlank(3, true)).bShake, "a wobbling walk made all along on a flank is not a tremor");
	T.AssertTrue(SanAnalyse(SanFlank(1, false)).bSpin, "a circle with no flank flag is still a spin");
	T.AssertTrue(SanAnalyse(SanFlank(3, false)).bShake, "a wobbling walk with no flank flag is still a tremor");
	// Q-sanity-circling = C.
	T.AssertFalse(SanAnalyse(SanPlaceWalking(1)).bSpin, "a circle made all along on a place walk (one side) is not a spin");
	T.AssertTrue(SanAnalyse(SanMake(1, true, 160)).bSpin, "the same circle without the place-walk flag is still a spin");
	T.AssertTrue(SanAnalyse(SanPlaceWalking(3)).bShake, "a wobbling (to and fro) walk on a place walk is still a tremor");
	// Q-sanity-micro = A.
	FHOLSanitySettingsView NoFloor;
	NoFloor.SpinMinPathRadii = 0.f;
	T.AssertTrue(HOLMovementRules::Analyse(SanMake(6, true, 160), 0.05f, NoFloor).bSpin, "without the floor (the rule as it was) a 2 cm fidget is a spin");
	T.AssertFalse(SanAnalyse(SanMake(6, true, 160)).bSpin, "with the floor a 2 cm fidget is not a spin");
	const FHOLSanityResultView Real = SanAnalyse(SanMake(7, true, 40));
	T.AssertTrue(Real.bSpin, "a real loop of 503 cm a turn is still a spin (path " + Real.Path + " cm, net turn " + Real.SignedTurn + " rad)");
}

// ---- Stall (FinalLeg) ----

bool SanStallRun(bool bFinalLegWalk, int Steps)
{
	FHOLStallView State;
	bool bTold = false;
	for (int Step = 0; Step < Steps; Step++)
		bTold = HOLMovementRules::StepStall(State, true, 0.05, HOLMovementRules::StallLimitSeconds(0.5, bFinalLegWalk, 2.0, 0.2)) || bTold;
	return bTold;
}

void Test_SanityStallFinalLeg(FUnitTest& T)
{
	const double Base = 0.5;
	const double Wait = 2.0;
	const double Margin = 0.2;
	T.AssertTrue(AvdNear2(HOLMovementRules::StallLimitSeconds(Base, false, Wait, Margin), Base), "no FinalLeg walk: the limit stays 0.5 s");
	T.AssertTrue(AvdNear2(HOLMovementRules::StallLimitSeconds(Base, true, Wait, Margin), 2.2), "a FinalLeg walk: the limit is the wait plus the margin (2.2 s)");
	T.AssertTrue(AvdNear2(HOLMovementRules::StallLimitSeconds(Base, true, Wait, -1.0), Base), "margin < 0: the tolerance is off");
	T.AssertTrue(AvdNear2(HOLMovementRules::StallLimitSeconds(5.0, true, Wait, Margin), 5.0), "a base limit already above the wait is kept");
	T.AssertTrue(AvdNear2(HOLMovementRules::StallLimitSeconds(0.0, true, Wait, Margin), 0.0), "the check off (0) stays off");
	T.AssertFalse(SanStallRun(true, 30), "a 1.5 s wait on a FinalLeg walk is not a stall");
	T.AssertTrue(SanStallRun(true, 48), "a 2.4 s wait is a stall even on a FinalLeg walk");
	T.AssertTrue(SanStallRun(false, 30), "a 1.5 s wait off a FinalLeg walk is still a stall");
}

bool AvdNear2(double A, double B)
{
	return Math::Abs(A - B) <= 1.e-9;
}

// ---- NoStrike ----

void Test_SanityNoStrike(FUnitTest& T)
{
	FHOLNoStrikeView Idle;
	int Told = 0;
	for (int Index = 0; Index < 80; Index++)
		Told += HOLMovementRules::StepNoStrike(Idle, true, 3, 0.05, 3.0) ? 1 : 0;
	T.AssertEquals(1, Told, "4 s in reach with no blow struck is told once");
	T.AssertEquals(4, Math::RoundToInt(Idle.Longest), "the run counts the 4 s");

	FHOLNoStrikeView Striker;
	int Hits = 0;
	int StrikerTold = 0;
	for (int Index = 0; Index < 400; Index++)
	{
		Hits += (Index % 27 == 26) ? 1 : 0;
		StrikerTold += HOLMovementRules::StepNoStrike(Striker, true, Hits, 0.05, 3.0) ? 1 : 0;
	}
	T.AssertEquals(0, StrikerTold, "a soldier striking every 1.35 s is never told");
	T.AssertTrue(Striker.Longest < 1.5, "its longest wait is under 1.5 s");

	FHOLNoStrikeView Looping;
	int LoopTold = 0;
	for (int Index = 0; Index < 200; Index++)
		LoopTold += HOLMovementRules::StepNoStrike(Looping, true, 7, 0.05, 3.0) ? 1 : 0;
	T.AssertEquals(1, LoopTold, "the blow begun again and again, none struck, is told once");

	FHOLNoStrikeView Away;
	for (int Index = 0; Index < 100; Index++)
		HOLMovementRules::StepNoStrike(Away, Index % 50 != 49, 1, 0.05, 3.0);
	T.AssertTrue(Away.Longest < 3.0 && Away.Runs == 0, "a walk away every 2.5 s keeps the run under the limit");

	FHOLNoStrikeView Fresh;
	T.AssertFalse(HOLMovementRules::StepNoStrike(Fresh, true, 0, 100.0, 0.0), "limit 0 is off");
}

// ---- Colour (SAN-Q1 = A, D616) ----

void Test_SanityColour(FUnitTest& T)
{
	TArray<uint8> Colours;
	for (int I = 0; I < 8; I++)
		Colours.Add(uint8(I));
	TArray<int> Players;
	Players.Add(0);
	Players.Add(1);
	TArray<int> Neutral8;
	Neutral8.Add(8);
	TArray<int> Neutral200;
	Neutral200.Add(200);
	TArray<int> NeutralMinus;
	NeutralMinus.Add(-1);
	TArray<int> OwnerOne;
	OwnerOne.Add(1);
	T.AssertTrue(HOLMovementRules::FindColourClashes(Neutral8, Players, Colours).Num() == 0, "the neutral owner (8) is grey: no clash with slots 0 and 1");
	T.AssertTrue(HOLMovementRules::FindColourClashes(Neutral200, Players, Colours).Num() == 0, "an owner far out of the slots (200) is grey too");
	T.AssertTrue(HOLMovementRules::FindColourClashes(NeutralMinus, Players, Colours).Num() == 0, "a negative owner is grey too");
	const TArray<FString> Clash = HOLMovementRules::FindColourClashes(OwnerOne, Players, Colours);
	T.AssertEquals(1, Clash.Num(), "an owner that is slot 1, taken for a neutral one, clashes with slot 1 only");
	T.AssertTrue(Clash.Num() > 0 && Clash[0].Contains("owner 1"), "the clash line names the owner");
	TArray<uint8> Same;
	Same.Add(3);
	Same.Add(3);
	Same.Add(2);
	Same.Add(2);
	Same.Add(1);
	Same.Add(1);
	Same.Add(0);
	Same.Add(0);
	T.AssertTrue(HOLMovementRules::FindColourClashes(Neutral8, Players, Same).Num() == 0, "slots sharing a colour do not make the grey neutral clash");
}
