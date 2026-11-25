// ELROND-1125me: the armed Banner (mode 101) carries the BANNER's reach to the controller (SetArmedMode keeps the last positive range: a 0 left the Seal's 800 cm,
// so the "Aura rings:" report and the aim preview said 800 instead of 1600). The answer comes from the real chain (HOLCommandGridRules::GetCommand ->
// HOLGridActionBridgeScript::DecideFromBridge); the reach is judged by the server's own rule (HOLHeroAbilityRead::CanPlantBanner), no copied 1600.

// HeroesOfLiberty.GridBannerReach.BannerArmsWithItsOwnReach
void Test_GBRBannerArmsWithItsOwnReach(FUnitTest& T)
{
	FHOLGridContextView Hero;
	Hero.bOwnUnits = true;
	Hero.bHero = true;
	Hero.HeroLevel = 99;
	Hero.HeroCooldownSeconds.Add(0);
	Hero.HeroCooldownSeconds.Add(0);
	Hero.HeroCooldownSeconds.Add(0);
	TArray<int> Selected;
	Selected.Add(3);

	FHOLGridCommandView Banner = HOLCommandGridRules::GetCommand(Hero, HOLCommandGridRules::GetHeroAbilityCell(2));
	FHOLGridActionAnswer Answer = HOLGridActionBridgeScript::DecideFromBridge(int(Banner.Command), Banner.Ability, Banner.bGreyed, Banner.CooldownSeconds, Banner.TypeName, Selected);
	int BannerAim = HOLCommandGridAction::GetAimBanner();
	T.AssertEquals(BannerAim, Answer.AimMode, "the Banner arms mode 101");
	const int Reach = int(Answer.RangeCm);
	T.AssertTrue(Reach > 800, "the Banner's reach is not the Seal's 800 cm (the answer said " + Reach + ")");
	T.AssertTrue(HOLHeroAbilityRead::CanPlantBanner(float(Reach)), "the server accepts a banner at the answered reach");
	T.AssertFalse(HOLHeroAbilityRead::CanPlantBanner(float(Reach + 1)), "and refuses one cm past it: the answer is the edge of the real reach");
}
