// GLORFINDEL-1123pf (D897, user's request 2026-11-23 15:59): the Sacred Light lands at the AIMED point (the ground click); the cast origin (banner, else army centre,
// else hero: HOLFactionPowerServer::CastOriginOf) only measures the range (D890, 1200 cm). Out of range = refusal (default of Retours/Q-lumiere-sacree-visee.md).
// Pure rule HOLFactionPowerCast::DecideAimed.

void Test_AimedCastFromHeroAlonePosesAtTheClick(FUnitTest& T)
{
	TArray<FVector> NoOwn;
	const FVector Hero = FVector(14900.0, 200.0, 0.0);
	const FVector Origin = HOLFactionPowerServer::CastOriginOf(FIntPoint(0, 0), NoOwn, true, Hero);
	FHOLFactionResource Full = FactionPowerCastTest::FullGauge();
	const FHOLAimedCastDecision Decision = HOLFactionPowerCast::DecideAimed(FactionPowerCastTest::Dawn(), 0, Full, 0, 0, int(Origin.X), int(Origin.Y), 15400, 200);
	T.AssertTrue(Decision.bAccepted, "hero alone, click 500 cm away: accepted");
	T.AssertEquals(Decision.PoseXCm, 15400, "the zone is put down at the clicked X, not at the hero");
	T.AssertEquals(Decision.PoseYCm, 200, "the zone is put down at the clicked Y");
}

void Test_AimedCastBeyondTheRangeIsRefused(FUnitTest& T)
{
	FHOLFactionResource Full = FactionPowerCastTest::FullGauge();
	const FHOLAimedCastDecision Decision = HOLFactionPowerCast::DecideAimed(FactionPowerCastTest::Dawn(), 0, Full, 0, 0, 14900, 200, 16200, 200);
	T.AssertFalse(Decision.bAccepted, "click 1300 cm from the origin (past the 1200 cm range): refused");
	T.AssertEquals(Decision.Refusal, HOLFactionPowerCast::RefusalOutOfRange(), "the refusal is the out-of-range one");
	T.AssertEquals(Decision.NewGaugeValue, Full.Value, "a refused cast costs no Honor");
}

void Test_AimedCastFromBannerAcceptsUpToTheSacredLightRange(FUnitTest& T)
{
	TArray<FVector> Army;
	Army.Add(FVector(13700.0, 200.0, 0.0));
	const FVector Origin = HOLFactionPowerServer::CastOriginOf(FIntPoint(500, 700), Army, true, FVector(14900.0, 0.0, 0.0));
	FHOLFactionResource Full = FactionPowerCastTest::FullGauge();
	const FHOLAimedCastDecision Decision = HOLFactionPowerCast::DecideAimed(FactionPowerCastTest::Dawn(), 0, Full, 0, 0, int(Origin.X), int(Origin.Y), 1600, 700);
	T.AssertTrue(Decision.bAccepted, "banner planted, click 1100 cm from it (inside 1200 cm): accepted");
	T.AssertEquals(Decision.PoseXCm, 1600, "the zone is put down at the clicked X, not at the banner");
	T.AssertEquals(Decision.PoseYCm, 700, "the zone is put down at the clicked Y");
}
