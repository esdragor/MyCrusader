// ELROND-1125mc (ENONCES-1125m G-3): the ground click of an armed mode becomes the right order. The mode and the caster come from the REAL chain
// (HOLCommandGridRules::GetCommand -> HOLGridActionBridgeScript::DecideFromBridge, the answer the C++ hook gives SetArmedMode); the click is then
// turned into an order by HOLGridGroundClick::OnGroundClick; the ability id is READ from HOLUnitAbilityTable, never copied.
// RED before OnGroundClick answers (the stub answers None). To SEE RED again: make OnGroundClick return the empty order.

// HeroesOfLiberty.GridGroundClick.SealCastsAtThePoint
void Test_GGCSealCastsAtThePoint(FUnitTest& T)
{
	FHOLGridContextView Selection;
	Selection.bOwnUnits = true;
	Selection.bMonkUnit = true;
	FHOLGridCommandView Command = HOLCommandGridRules::GetCommand(Selection, HOLCommandGridRules::GetMonkSealCell());
	TArray<int> Selected;
	Selected.Add(7);
	FHOLGridActionAnswer Armed = HOLGridActionBridgeScript::DecideFromBridge(int(Command.Command), Command.Ability, Command.bGreyed, Command.CooldownSeconds, Command.TypeName, Selected);
	T.AssertEquals(2, Armed.Kind, "the chain arms the Seal");

	FHOLGridGroundOrder Order = HOLGridGroundClick::OnGroundClick(Armed.AimMode, Armed.UnitId, FVector(2000.0, -300.0, 10.0), Selected);
	T.AssertTrue(Order.Kind == EHOLGridGroundKind::CastUnitAbility, "the click of the Seal's mode casts a unit ability");
	T.AssertEquals(HOLUnitAbilityTable::LightSeal, Order.AbilityId, "the ability id is the table's Seal");
	T.AssertEquals(7, Order.UnitId, "the caster is the Monk the chain chose");
	T.AssertEquals(2000, int(Order.Point.X), "the point aimed is the point cast");
	T.AssertEquals(-300, int(Order.Point.Y), "the point aimed is the point cast (Y)");
}

// HeroesOfLiberty.GridGroundClick.BannerAndAttackMove
void Test_GGCBannerAndAttackMove(FUnitTest& T)
{
	FHOLGridContextView Hero;
	Hero.bOwnUnits = true;
	Hero.bHero = true;
	// Like Test_GABAbilitiesAndOrders: a hero below the ability's level (or with no cooldown entries) has the Banner greyed, which arms nothing.
	Hero.HeroLevel = 99;
	Hero.HeroCooldownSeconds.Add(0);
	Hero.HeroCooldownSeconds.Add(0);
	Hero.HeroCooldownSeconds.Add(0);
	TArray<int> Selected;
	Selected.Add(3);
	Selected.Add(4);
	FHOLGridCommandView Banner = HOLCommandGridRules::GetCommand(Hero, HOLCommandGridRules::GetHeroAbilityCell(2));
	FHOLGridActionAnswer BannerArmed = HOLGridActionBridgeScript::DecideFromBridge(int(Banner.Command), Banner.Ability, Banner.bGreyed, Banner.CooldownSeconds, Banner.TypeName, Selected);
	T.AssertEquals(HOLCommandGridAction::GetAimBanner(), BannerArmed.AimMode, "the chain arms the Banner");
	FHOLGridGroundOrder BannerOrder = HOLGridGroundClick::OnGroundClick(BannerArmed.AimMode, BannerArmed.UnitId, FVector(500.0, 600.0, 0.0), Selected);
	T.AssertTrue(BannerOrder.Kind == EHOLGridGroundKind::HeroAbility, "the click of the Banner's mode casts the hero ability");
	T.AssertEquals(int(EHOLHeroAbility::Banner), BannerOrder.AbilityId, "the hero ability is the Banner");
	T.AssertEquals(500, int(BannerOrder.Point.X), "the Banner goes to the clicked point");

	int AttackMode = HOLCommandGridAction::GetAimAttackMove();
	FHOLGridGroundOrder MoveOrder = HOLGridGroundClick::OnGroundClick(AttackMode, 0, FVector(900.0, 100.0, 0.0), Selected);
	T.AssertTrue(MoveOrder.Kind == EHOLGridGroundKind::AttackMove, "the click of the attack-move mode sends an attack-move");
	T.AssertEquals(2, MoveOrder.UnitIds.Num(), "the whole selection attack-moves");

	FHOLGridGroundOrder FactionPower = HOLGridGroundClick::OnGroundClick(3, 0, FVector(0.0, 0.0, 0.0), Selected);
	T.AssertTrue(FactionPower.Kind == EHOLGridGroundKind::None, "a faction power's mode (below 100) is not the grid's");
}

// HeroesOfLiberty.GridGroundClick.BridgeGetIsTheScriptClass
// The C++ hook finds its script rule through UHOLGridActionBridge::Get(): it must be the script subclass, and its DecideGridAction (the override the hook calls) must arm the Seal.
void Test_GGCBridgeGetIsTheScriptClass(FUnitTest& T)
{
	UHOLGridActionBridge Bridge = UHOLGridActionBridge::Get();
	T.AssertTrue(Bridge != nullptr, "Get() finds a bridge instance");
	UHOLGridActionScript ScriptBridge = Cast<UHOLGridActionScript>(Bridge);
	T.AssertTrue(ScriptBridge != nullptr, "the instance Get() gives is the script class UHOLGridActionScript");
}