// ELROND-1125mb (G-4 raccord): the C++ hook's question (command as int, ability, greyed, selection ids) answered by the script bridge from the rule
// HOLCommandGridAction::Decide. The commands come from the rule chain (HOLCommandGridRules::GetCommand), the answers are asserted (kind, mode, range, caster).
// RED before DecideFromBridge copies the verdict (the stub answers Kind None). To SEE RED again: make DecideFromBridge return an empty answer.

// HeroesOfLiberty.GridActionBridge.SealArmsWithFirstSelected
void Test_GABSealArms(FUnitTest& T)
{
	FHOLGridContextView Selection;
	Selection.bOwnUnits = true;
	Selection.bMonkUnit = true;
	FHOLGridCommandView Command = HOLCommandGridRules::GetCommand(Selection, HOLCommandGridRules::GetMonkSealCell());
	TArray<int> Selected;
	Selected.Add(7);
	Selected.Add(9);
	FHOLGridActionAnswer Answer = HOLGridActionBridgeScript::DecideFromBridge(int(Command.Command), Command.Ability, Command.bGreyed, Command.CooldownSeconds, Command.TypeName, Selected);
	T.AssertEquals(2, Answer.Kind, "the Seal arms the ground click (kind 2 = Arm)");
	int SealAim = HOLCommandGridAction::GetAimSeal();
	T.AssertEquals(SealAim, Answer.AimMode, "the armed mode is the Seal's");
	T.AssertEquals(800, int(Answer.RangeCm), "the range is the table's 8 m");
	T.AssertEquals(7, Answer.UnitId, "the caster is the first selected unit");

	TArray<int> Nobody;
	FHOLGridActionAnswer NoCaster = HOLGridActionBridgeScript::DecideFromBridge(int(Command.Command), Command.Ability, Command.bGreyed, Command.CooldownSeconds, Command.TypeName, Nobody);
	T.AssertEquals(0, NoCaster.Kind, "no selection: nothing happens");

	FHOLGridActionAnswer Greyed = HOLGridActionBridgeScript::DecideFromBridge(int(Command.Command), Command.Ability, true, 5, Command.TypeName, Selected);
	T.AssertEquals(0, Greyed.Kind, "a greyed Seal does nothing");
}

// HeroesOfLiberty.GridActionBridge.AbilitiesAndOrders
void Test_GABAbilitiesAndOrders(FUnitTest& T)
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
	FHOLGridActionAnswer BannerAnswer = HOLGridActionBridgeScript::DecideFromBridge(int(Banner.Command), Banner.Ability, Banner.bGreyed, Banner.CooldownSeconds, Banner.TypeName, Selected);
	T.AssertEquals(2, BannerAnswer.Kind, "the Banner arms the ground click");
	int BannerAim = HOLCommandGridAction::GetAimBanner();
	T.AssertEquals(BannerAim, BannerAnswer.AimMode, "the armed mode is the Banner's");

	FHOLGridCommandView CloseRanks = HOLCommandGridRules::GetCommand(Hero, HOLCommandGridRules::GetHeroAbilityCell(1));
	FHOLGridActionAnswer CloseAnswer = HOLGridActionBridgeScript::DecideFromBridge(int(CloseRanks.Command), CloseRanks.Ability, CloseRanks.bGreyed, CloseRanks.CooldownSeconds, CloseRanks.TypeName, Selected);
	T.AssertEquals(1, CloseAnswer.Kind, "Close Ranks is sent at once (kind 1 = Send)");

	FHOLGridContextView Soldiers;
	Soldiers.bOwnUnits = true;
	Soldiers.bSoldiers = true;
	FHOLGridCommandView Attack = HOLCommandGridRules::GetOrder(Soldiers, HOLCommandGridRules::GetAttackMoveCell());
	FHOLGridActionAnswer AttackAnswer = HOLGridActionBridgeScript::DecideFromBridge(int(Attack.Command), Attack.Ability, Attack.bGreyed, Attack.CooldownSeconds, Attack.TypeName, Selected);
	int AttackAim = HOLCommandGridAction::GetAimAttackMove();
	T.AssertEquals(AttackAim, AttackAnswer.AimMode, "the attack-move cell arms the attack-move mode");
}
