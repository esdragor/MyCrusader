// D860 H2 (minimap): a signal's square pulses like the ground's mark (HOLSignalMarks.as). Values are computed by the rule
// (HOLSignalInputRead::PulseScale and its lifetime), none copied from a constant.

FHOLMinimapFrame SignalMarksFrame()
{
	return HOLMinimap::MakeFrame(-40000.0, -40000.0, 40000.0, 40000.0, 200.0);
}

void Test_MinimapSignalMarkPulses(FUnitTest& T)
{
	double Life = HOLSignalInputRead::MarkLifetime();
	FVector Where(12000.0, -3000.0, 0.0);
	double Received = 100.0;
	FHOLMinimapMark Born = HOLSignalMarks::Mark(SignalMarksFrame(), Where, FLinearColor::Red, Received, Received);
	FHOLMinimapMark Quarter = HOLSignalMarks::Mark(SignalMarksFrame(), Where, FLinearColor::Red, Received, Received + Life * 0.25);

	T.AssertTrue(Math::Abs(Born.Size - Quarter.Size) > 0.01, f"the square's size changes with the signal's age ({Born.Size} then {Quarter.Size})");

	// The size is the base size times the rule's scale at the age (Now - ReceivedAt).
	double Expected = double(HOLSignalMarks::BaseSize) * HOLSignalInputRead::PulseScale(Life * 0.25);
	T.AssertTrue(Math::Abs(Quarter.Size - Expected) < 0.001, f"the size at a quarter of the life is base x PulseScale ({Quarter.Size} vs {Expected})");

	// The centre does not move while the size does.
	T.AssertTrue(Math::Abs(Born.Position.X - Quarter.Position.X) < 0.001 && Math::Abs(Born.Position.Y - Quarter.Position.Y) < 0.001, "the square keeps its centre");

	// Back to the base size at the expiry.
	FHOLMinimapMark Expiry = HOLSignalMarks::Mark(SignalMarksFrame(), Where, FLinearColor::Red, Received, Received + Life);
	T.AssertTrue(Math::Abs(Expiry.Size - double(HOLSignalMarks::BaseSize)) < 0.001, "the square is back to its base size at the expiry");
}
