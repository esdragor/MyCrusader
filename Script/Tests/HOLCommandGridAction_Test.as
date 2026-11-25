// GLORFINDEL-1125ma (ENONCES-1125m G-1 / G-2): the verdict of a click on a grid cell (HOLCommandGridAction::Decide), through the real chain: the command comes
// from HOLCommandGridRules::GetCommand / GetOrder (never written by hand), the verdict is asserted (kind, mode, range, caster). NOT COMPILED when written.
// RED before Decide answers (the stub answers None for everything). To SEE RED again: make Decide return None for LightSeal.

// HeroesOfLiberty.CommandGridAction.LightSeal
void Test_CGALightSeal(FUnitTest& T)
{
	FHOLGridContextView Selection;
	Selection.bOwnUnits = true;
	Selection.bMonkUnit = true;
	int SealCell = HOLCommandGridRules::GetMonkSealCell();
	FHOLGridCommandView Command = HOLCommandGridRules::GetCommand(Selection, SealCell);
	T.AssertTrue(Command.Command == EHOLGridCommandKind::LightSeal, "the chain gives the Seal of Light command for a selection with a Monk");

	FHOLGridActionContext WithMonk;
	WithMonk.bMonkUnit = true;
	WithMonk.MonkUnitId = 7;
	FHOLGridActionView Verdict = HOLCommandGridAction::Decide(WithMonk, Command);
	T.AssertTrue(Verdict.Kind == EHOLGridActionKind::Arm, "a Monk is selected: the Seal arms the ground click");
	int SealAim = HOLCommandGridAction::GetAimSeal();
	T.AssertEquals(SealAim, Verdict.AimMode, "the armed mode is the Seal's");
	T.AssertEquals(800, Verdict.RangeCm, "UnitAbilities.csv: the Seal is thrown from 8 m (800 cm) at most");
	T.AssertEquals(7, Verdict.UnitId, "the caster is the selected Monk");

	FHOLGridActionContext NoMonk;
	FHOLGridActionView Refused = HOLCommandGridAction::Decide(NoMonk, Command);
	T.AssertTrue(Refused.Kind == EHOLGridActionKind::None, "no Monk selected: nothing happens");

	FHOLGridCommandView Grey = Command;
	Grey.bGreyed = true;
	FHOLGridActionView GreyVerdict = HOLCommandGridAction::Decide(WithMonk, Grey);
	T.AssertTrue(GreyVerdict.Kind == EHOLGridActionKind::None, "a greyed Seal does nothing");
}

// HeroesOfLiberty.CommandGridAction.HeroAbility
void Test_CGAHeroAbility(FUnitTest& T)
{
	FHOLGridContextView Hero;
	Hero.bOwnUnits = true;
	Hero.bHero = true;
	Hero.HeroLevel = 99;
	Hero.HeroCooldownSeconds.Add(0);
	Hero.HeroCooldownSeconds.Add(0);
	Hero.HeroCooldownSeconds.Add(0);
	FHOLGridActionContext Nothing;

	FHOLGridCommandView CloseRanks = HOLCommandGridRules::GetCommand(Hero, HOLCommandGridRules::GetHeroAbilityCell(1));
	FHOLGridActionView CloseVerdict = HOLCommandGridAction::Decide(Nothing, CloseRanks);
	T.AssertTrue(CloseVerdict.Kind == EHOLGridActionKind::Send, "Close Ranks (no ground) is sent at once");

	FHOLGridCommandView Banner = HOLCommandGridRules::GetCommand(Hero, HOLCommandGridRules::GetHeroAbilityCell(2));
	FHOLGridActionView BannerVerdict = HOLCommandGridAction::Decide(Nothing, Banner);
	T.AssertTrue(BannerVerdict.Kind == EHOLGridActionKind::Arm, "the Banner arms the ground click");
	int BannerAim = HOLCommandGridAction::GetAimBanner();
	T.AssertEquals(BannerAim, BannerVerdict.AimMode, "the armed mode is the Banner's");

	Hero.HeroCooldownSeconds[1] = 30;
	FHOLGridCommandView Cooling = HOLCommandGridRules::GetCommand(Hero, HOLCommandGridRules::GetHeroAbilityCell(1));
	T.AssertTrue(Cooling.bGreyed, "the chain greys Close Ranks while it cools down");
	FHOLGridActionView CoolingVerdict = HOLCommandGridAction::Decide(Nothing, Cooling);
	T.AssertTrue(CoolingVerdict.Kind == EHOLGridActionKind::None, "a greyed hero ability does nothing");
}

// HeroesOfLiberty.CommandGridAction.OrdersAndSweep
// Every command kind of the grid (a sweep of EHOLGridCommandKind) has a verdict, except None and the three of G-5 (Produce, Research, Trade).
void Test_CGAOrdersAndSweep(FUnitTest& T)
{
	FHOLGridContextView Units;
	Units.bOwnUnits = true;
	Units.bSoldiers = true;
	Units.bDeployables = true;
	FHOLGridCommandView AttackMove = HOLCommandGridRules::GetOrder(Units, HOLCommandGridRules::GetAttackMoveCell());
	FHOLGridActionContext Nothing;
	FHOLGridActionView AttackVerdict = HOLCommandGridAction::Decide(Nothing, AttackMove);
	T.AssertTrue(AttackVerdict.Kind == EHOLGridActionKind::Arm, "attack-move arms the ground click");
	int AttackAim = HOLCommandGridAction::GetAimAttackMove();
	T.AssertEquals(AttackAim, AttackVerdict.AimMode, "the armed mode is the attack-move's");

	FHOLGridCommandView Stop = HOLCommandGridRules::GetOrder(Units, HOLCommandGridRules::GetStopCell());
	FHOLGridActionView StopVerdict = HOLCommandGridAction::Decide(Nothing, Stop);
	T.AssertTrue(StopVerdict.Kind == EHOLGridActionKind::Send, "Stop is sent");

	FHOLGridActionContext WithMonk;
	WithMonk.bMonkUnit = true;
	WithMonk.MonkUnitId = 3;
	int Silent = 0;
	FString First = "";
	for (int Index = 1; Index <= 15; Index++)
	{
		EHOLGridCommandKind Kind = EHOLGridCommandKind(Index);
		if (Kind == EHOLGridCommandKind::Produce || Kind == EHOLGridCommandKind::Research || Kind == EHOLGridCommandKind::Trade)
		{
			continue;
		}
		FHOLGridCommandView Command;
		Command.Command = Kind;
		Command.Ability = 1;
		FHOLGridActionView Verdict = HOLCommandGridAction::Decide(WithMonk, Command);
		if (Verdict.Kind == EHOLGridActionKind::None)
		{
			Silent++;
			if (First.IsEmpty())
			{
				First = f"kind {Index}";
			}
		}
	}
	T.AssertEquals(0, Silent, f"every command kind has a verdict (first silent: {First})");

	FHOLGridCommandView Nothing2;
	FHOLGridActionView NoneVerdict = HOLCommandGridAction::Decide(WithMonk, Nothing2);
	T.AssertTrue(NoneVerdict.Kind == EHOLGridActionKind::None, "the empty cell does nothing");
}
