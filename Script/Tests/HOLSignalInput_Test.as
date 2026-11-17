// SG3 (GDD 15, D386, D784), port of the C++ test HeroesOfLiberty.Signals.AltClick (HOLSignalInputTests.cpp, E3-11, D830, D839):
// the Alt + click's kind and the received icons' life, read through UHOLSignalInputReadLibrary (HOLSignalInputRead::, wrappers of the
// real C++ functions HOLSignalInput::KindOfAltClick / Expire / ColorOf that the controller calls).

// A drag of 60 px from Press, clockwise from the top (screen y downwards).
FVector2D SignalDragFrom(FVector2D Press, double Degrees)
{
	double Radians = Math::DegreesToRadians(Degrees);
	return FVector2D(Press.X + Math::Sin(Radians) * 60.0, Press.Y - Math::Cos(Radians) * 60.0);
}

void Test_SignalAltClick(FUnitTest& T)
{
	// Kinds as HOLSignalRules.as (the GDD's order of the wheel).
	const int Neutral = HOLSignal::Neutral;
	const FVector2D Press(500.0, 400.0);

	// A brief Alt + click (a jitter of 3 px) is the neutral signal (GDD 15: "un simple Alt + clic").
	T.AssertEquals(Neutral, HOLSignalInputRead::KindOfAltClick(Press, FVector2D(502.0, 398.0)), "a brief Alt + click places the neutral signal");

	// A drag of 60 px picks the wheel's kind by its direction, 72 degrees each.
	T.AssertEquals(HOLSignal::Attack, HOLSignalInputRead::KindOfAltClick(Press, SignalDragFrom(Press, 0.0)), "dragged up: attack here");
	T.AssertEquals(HOLSignal::Defend, HOLSignalInputRead::KindOfAltClick(Press, SignalDragFrom(Press, 72.0)), "dragged up-right: defend here");
	T.AssertEquals(HOLSignal::NeedHelp, HOLSignalInputRead::KindOfAltClick(Press, SignalDragFrom(Press, 144.0)), "dragged down-right: need help");
	T.AssertEquals(HOLSignal::Rally, HOLSignalInputRead::KindOfAltClick(Press, SignalDragFrom(Press, 216.0)), "dragged down-left: rally");
	T.AssertEquals(HOLSignal::Scout, HOLSignalInputRead::KindOfAltClick(Press, SignalDragFrom(Press, 288.0)), "dragged up-left: scout here");
	T.AssertEquals(HOLSignal::Attack, HOLSignalInputRead::KindOfAltClick(Press, SignalDragFrom(Press, 350.0)), "dragged a little left of the top: still attack");

	// The received icons: one at 10.0 s, one at 12.0 s; at 14.9 s both are lit, at 15.0 s the first is gone (5 s, D784), at 17.0 s none.
	TArray<FHOLReceivedSignal> Signals;
	Signals.Add(HOLSignalInputRead::MakeSignal(HOLSignal::Attack, 10.0));
	Signals.Add(HOLSignalInputRead::MakeSignal(HOLSignal::Rally, 12.0));
	HOLSignalInputRead::Expire(Signals, 14.9);
	T.AssertEquals(2, Signals.Num(), "both icons are lit before 5 s");
	HOLSignalInputRead::Expire(Signals, 15.0);
	T.AssertEquals(1, Signals.Num(), "the first icon is gone at 5 s");
	if (Signals.Num() == 1)
		T.AssertEquals(HOLSignal::Rally, Signals[0].Kind, "the second one is left");
	HOLSignalInputRead::Expire(Signals, 17.0);
	T.AssertEquals(0, Signals.Num(), "none at 17 s");

	// Each kind its own colour (default A of Q2): the attack's is not the neutral's.
	FLinearColor AttackColor = HOLSignalInputRead::ColorOf(HOLSignal::Attack);
	FLinearColor NeutralColor = HOLSignalInputRead::ColorOf(Neutral);
	T.AssertTrue(AttackColor.R != NeutralColor.R || AttackColor.G != NeutralColor.G || AttackColor.B != NeutralColor.B || AttackColor.A != NeutralColor.A,
		"the attack's icon is not coloured as the neutral one");
}
