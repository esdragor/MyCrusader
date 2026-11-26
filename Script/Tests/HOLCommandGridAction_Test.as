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
	T.AssertTrue(CoolingVerdict.Kind == EHOLGridActionKind::Refuse, "a greyed hero ability is refused (the player keeps his notice)");
	T.AssertEquals(n"cry", CoolingVerdict.Report, "the refusal of Close Ranks is told as the cry");
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

FHOLResources CGAXRes(int Food, int Wood, int Stone, int Gold)
{
	FHOLResources Out;
	Out.Food = Food;
	Out.Wood = Wood;
	Out.Stone = Stone;
	Out.Gold = Gold;
	return Out;
}

// HeroesOfLiberty.CommandGridAction.BuildRefused (GLORFINDEL-1125mb: Build's resource refusal, CheckBuildAnswer)
void Test_CGABuildRefused(FUnitTest& T)
{
	FHOLGridCommandView Build;
	Build.Command = EHOLGridCommandKind::Build;
	FHOLGridActionContext Nothing;
	FHOLGridCostFacts Poor;
	Poor.Stock = CGAXRes(500, 50, 500, 500);
	Poor.Cost = CGAXRes(0, 100, 0, 0);
	FHOLGridActionView Refused = HOLCommandGridAction::DecideCosted(Nothing, Poor, Build);
	T.AssertTrue(Refused.Kind == EHOLGridActionKind::Refuse, "50 wood for a 100-wood building: the click is refused, no placement");
	T.AssertTrue(Refused.Feedback.Kind == EHOLCommandFeedbackKind::NotEnoughResources, "the refusal says not enough resources");
	T.AssertTrue(Refused.Feedback.Resource == EHOLResource::Wood, "the missing resource is the wood");

	FHOLGridCostFacts Rich = Poor;
	Rich.Stock = CGAXRes(500, 100, 500, 500);
	FHOLGridActionView Placed = HOLCommandGridAction::DecideCosted(Nothing, Rich, Build);
	T.AssertTrue(Placed.Kind == EHOLGridActionKind::Place, "exactly the cost in stock: the building is placed");
}

// HeroesOfLiberty.CommandGridAction.ProduceResearchTrade (G-2 rest: the three kinds of G-5 have a verdict)
void Test_CGAProduceResearchTrade(FUnitTest& T)
{
	FHOLGridActionContext Nothing;
	FHOLGridCostFacts Facts;
	Facts.Stock = CGAXRes(500, 500, 500, 500);
	Facts.Cost = CGAXRes(50, 0, 0, 0);
	Facts.QueueLength = 0;
	Facts.MaxQueue = 5;
	Facts.PopulationUsed = 1;
	Facts.PopulationCap = 10;

	FHOLGridCommandView Produce;
	Produce.Command = EHOLGridCommandKind::Produce;
	FHOLGridActionView Queued = HOLCommandGridAction::DecideCosted(Nothing, Facts, Produce);
	T.AssertTrue(Queued.Kind == EHOLGridActionKind::Send, "an affordable unit with room in the queue is sent to the building");

	FHOLGridCostFacts Full = Facts;
	Full.QueueLength = 5;
	FHOLGridActionView FullVerdict = HOLCommandGridAction::DecideCosted(Nothing, Full, Produce);
	T.AssertTrue(FullVerdict.Kind == EHOLGridActionKind::Refuse, "a full queue refuses");
	T.AssertTrue(FullVerdict.Feedback.Kind == EHOLCommandFeedbackKind::QueueFull, "the refusal says queue full");

	FHOLGridCostFacts Broke = Facts;
	Broke.Stock = CGAXRes(10, 500, 500, 500);
	FHOLGridActionView BrokeVerdict = HOLCommandGridAction::DecideCosted(Nothing, Broke, Produce);
	T.AssertTrue(BrokeVerdict.Kind == EHOLGridActionKind::Refuse, "10 food for a 50-food unit refuses");
	T.AssertTrue(BrokeVerdict.Feedback.Resource == EHOLResource::Food, "the missing resource is the food");

	FHOLGridCostFacts Crowded = Facts;
	Crowded.PopulationUsed = 10;
	FHOLGridActionView CrowdedVerdict = HOLCommandGridAction::DecideCosted(Nothing, Crowded, Produce);
	T.AssertTrue(CrowdedVerdict.Kind == EHOLGridActionKind::Send, "a full population only waits in the queue (Blocks): still sent");

	FHOLGridCommandView Research;
	Research.Command = EHOLGridCommandKind::Research;
	FHOLGridActionView ResearchVerdict = HOLCommandGridAction::Decide(Nothing, Research);
	T.AssertTrue(ResearchVerdict.Kind == EHOLGridActionKind::Send, "a research is sent to the building");

	FHOLGridCommandView Trade;
	Trade.Command = EHOLGridCommandKind::Trade;
	FHOLGridActionView TradeVerdict = HOLCommandGridAction::Decide(Nothing, Trade);
	T.AssertTrue(TradeVerdict.Kind == EHOLGridActionKind::Send, "a market trade is sent");
}
// D923 (FARAMIR-1126m3): the Knight's Defi button. The chain gives the KnightChallenge command at the cell GetKnightChallengeCell (T, cell 4) for a selection with a Virtuous Knight;
// the verdict is a Send (no aim) carrying the ability id of HOLUnitAbilityTable and the Knight as caster; a greyed (cooling down) Defi does nothing; without a Knight, nothing.
// HeroesOfLiberty.CommandGridAction.KnightChallenge
void Test_CGAKnightChallenge(FUnitTest& T)
{
	FHOLGridContextView Selection;
	Selection.bOwnUnits = true;
	Selection.bKnightUnit = true;
	int ChallengeCell = HOLCommandGridRules::GetKnightChallengeCell();
	FHOLGridCommandView Command = HOLCommandGridRules::GetCommand(Selection, ChallengeCell);
	T.AssertTrue(Command.Command == EHOLGridCommandKind::KnightChallenge, "the chain gives the Knight's Defi command for a selection with a Virtuous Knight");
	T.AssertTrue(ChallengeCell != HOLCommandGridRules::GetMonkSealCell(), "the Defi never shares the cell of the Seal");

	FHOLGridActionContext WithKnight;
	WithKnight.bKnightUnit = true;
	WithKnight.KnightUnitId = 9;
	FHOLGridActionView Verdict = HOLCommandGridAction::Decide(WithKnight, Command);
	T.AssertTrue(Verdict.Kind == EHOLGridActionKind::Send, "a Knight is selected: the Defi is sent at once, nothing to aim");
	T.AssertEquals(HOLUnitAbilityTable::KnightChallenge, Verdict.AbilityId, "the ability id is the table's");
	T.AssertEquals(9, Verdict.UnitId, "the caster is the selected Knight");

	FHOLGridActionContext NoKnight;
	FHOLGridActionView Refused = HOLCommandGridAction::Decide(NoKnight, Command);
	T.AssertTrue(Refused.Kind == EHOLGridActionKind::None, "no Knight selected: nothing happens");

	FHOLGridCommandView Grey = Command;
	Grey.bGreyed = true;
	FHOLGridActionView GreyVerdict = HOLCommandGridAction::Decide(WithKnight, Grey);
	T.AssertTrue(GreyVerdict.Kind == EHOLGridActionKind::None, "a Defi cooling down does nothing");
}