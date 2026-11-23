// GLORFINDEL-1123m: the PLAYER's Sacred Light cast with no banner planted (recette 1123a: "RpcPower: cast refused (4)" at 0 cm of the hero, the origin read (0, 0)).
// The origin is the ONE HOLFactionPowerServer::CastOriginOf (also the AI's); the verdict and the Honor written back are computed by the rule chain (Decide).

void Test_PlayerCastsSacredLightAtTheHeroWithoutBanner(FUnitTest& T)
{
	TArray<FVector> Own;
	Own.Add(FVector(13700.0, 200.0, 0.0));
	const FVector Origin = HOLFactionPowerServer::CastOriginOf(FIntPoint(0, 0), Own);
	FHOLFactionResource Full = FactionPowerCastTest::FullGauge();
	const FHOLFactionPowerDecision Decision = HOLFactionPowerCast::Decide(FactionPowerCastTest::Dawn(), 0, Full, 0, 0, int(Origin.X), int(Origin.Y), 13700, 200);
	T.AssertTrue(Decision.bAccepted, "no banner planted, the point at the hero's feet: the player's Sacred Light is accepted");
	TArray<FHOLPowerRow> Rows = HOLDawnPowers::Rows();
	const int Cost = Rows[0].Cost;
	T.AssertEquals(Decision.NewGaugeValue, Full.Value - Cost, "the Honor is spent by the row's cost");
}

void Test_PlayerCastOriginKeepsAPlantedBanner(FUnitTest& T)
{
	TArray<FVector> Own;
	Own.Add(FVector(13700.0, 200.0, 0.0));
	const FVector Origin = HOLFactionPowerServer::CastOriginOf(FIntPoint(500, 700), Own);
	T.AssertTrue(Math::Abs(Origin.X - 500.0) < 1.0 && Math::Abs(Origin.Y - 700.0) < 1.0, "a planted banner is the origin");
}
