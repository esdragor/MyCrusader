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

// GLORFINDEL-1123pd (recette 1123pa, defect a): hero and peasants only (no banner, no soldier: ReadAISummary(Slot).Army is empty), the origin was (0, 0) and the cast
// was refused (4). The origin is now the hero's place. PURE functions (the script test world has no living hero yet: HOLTestWorld::SpawnTestHero is on another branch):
// the C++ reader HOLHero::GetHeroLocation is checked by the main session's run, not here.
void Test_CastOriginIsTheHeroWhenNoBannerNorArmy(FUnitTest& T)
{
	TArray<FVector> NoOwn;
	const FVector Hero = FVector(14900.0, 0.0, 0.0);
	const FVector Origin = HOLFactionPowerServer::CastOriginOf(FIntPoint(0, 0), NoOwn, true, Hero);
	T.AssertTrue(Math::Abs(Origin.X - 14900.0) < 1.0 && Math::Abs(Origin.Y) < 1.0, "no banner, no army: the origin is the hero's place, not (0, 0)");
	FHOLFactionResource Full = FactionPowerCastTest::FullGauge();
	const FHOLFactionPowerDecision Decision = HOLFactionPowerCast::Decide(FactionPowerCastTest::Dawn(), 0, Full, 0, 0, int(Origin.X), int(Origin.Y), 14950, 30);
	T.AssertTrue(Decision.bAccepted, "hero and peasants only: a cast at the hero's feet is accepted");
}

void Test_CastOriginOrderIsBannerThenArmyThenHero(FUnitTest& T)
{
	TArray<FVector> Army;
	Army.Add(FVector(13700.0, 200.0, 0.0));
	TArray<FVector> NoOwn;
	const FVector Hero = FVector(14900.0, 0.0, 0.0);
	const FVector WithBanner = HOLFactionPowerServer::CastOriginOf(FIntPoint(500, 700), Army, true, Hero);
	T.AssertTrue(Math::Abs(WithBanner.X - 500.0) < 1.0 && Math::Abs(WithBanner.Y - 700.0) < 1.0, "a planted banner beats the army and the hero");
	const FVector WithArmy = HOLFactionPowerServer::CastOriginOf(FIntPoint(0, 0), Army, true, Hero);
	T.AssertTrue(Math::Abs(WithArmy.X - 13700.0) < 1.0 && Math::Abs(WithArmy.Y - 200.0) < 1.0, "with no banner, the army's centre beats the hero");
	const FVector Nobody = HOLFactionPowerServer::CastOriginOf(FIntPoint(0, 0), NoOwn, false, Hero);
	T.AssertTrue(Math::Abs(Nobody.X) < 1.0 && Math::Abs(Nobody.Y) < 1.0, "no banner, no army, no hero found: the banner as is (0, 0), the hero's place is not invented");
}
