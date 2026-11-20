// D860 H2: the signal's mark pulses over its life. One rule, HOLSignalInput::PulseScale (C++, Player/HOLSignalInput.cpp), read here through
// UHOLSignalInputReadLibrary (HOLSignalInputRead::); the ground's mark (UpdateSignalMarks) and the minimap's square read the same one.
// Every asserted value is computed by the rule (and its bounds / lifetime accessors), none is copied from a constant.

void Test_SignalPulseScale(FUnitTest& T)
{
	double Life = HOLSignalInputRead::MarkLifetime();
	double Low = HOLSignalInputRead::PulseMin();
	double High = HOLSignalInputRead::PulseMax();

	// The scale varies over the life: born is not a quarter of the way through.
	double AtBirth = HOLSignalInputRead::PulseScale(0.0);
	double AtQuarter = HOLSignalInputRead::PulseScale(Life * 0.25);
	T.AssertTrue(Math::Abs(AtBirth - AtQuarter) > 0.001, "the mark's scale at 0 differs from its scale at a quarter of its life");

	// Over the whole life it stays within [min, max], goes above and below 1 (a pulse, not a drift).
	double SeenLow = 1000.0;
	double SeenHigh = -1000.0;
	for (int i = 0; i <= 200; i++)
	{
		double Scale = HOLSignalInputRead::PulseScale(Life * double(i) / 200.0);
		if (Scale < SeenLow)
			SeenLow = Scale;
		if (Scale > SeenHigh)
			SeenHigh = Scale;
	}
	T.AssertTrue(SeenLow >= Low - 0.0001, "the scale never goes under the lower bound");
	T.AssertTrue(SeenHigh <= High + 0.0001, "the scale never goes over the upper bound");
	T.AssertTrue(SeenLow < 0.999, "the mark shrinks below its normal size at some age");
	T.AssertTrue(SeenHigh > 1.001, "the mark grows above its normal size at some age");

	// Back to 1 at the expiry and after (and before birth).
	double AtExpiry = HOLSignalInputRead::PulseScale(Life);
	double After = HOLSignalInputRead::PulseScale(Life + 3.0);
	double Before = HOLSignalInputRead::PulseScale(-1.0);
	T.AssertTrue(Math::Abs(AtExpiry - 1.0) < 0.000001, "the scale is 1 at the expiry");
	T.AssertTrue(Math::Abs(After - 1.0) < 0.000001, "the scale is 1 after the expiry");
	T.AssertTrue(Math::Abs(Before - 1.0) < 0.000001, "the scale is 1 before the birth");
}
