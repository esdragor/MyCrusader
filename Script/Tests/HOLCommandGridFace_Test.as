// GLORFINDEL-1125mc (ENONCES-1125m G-6): the face of a grid cell (HOLCommandGridFace::GetCellFace), through the real chain: the command comes from
// HOLCommandGridRules::GetCommand (never written by hand), the face is asserted (label, state, tooltip, report name). NOT COMPILED when written.
// RED before GetCellFace answers (the stub gives an empty face). To SEE RED again: make GetCellFace return the empty face for LightSeal.

// HeroesOfLiberty.CommandGridFace.LightSeal
void Test_CGFLightSeal(FUnitTest& T)
{
	FHOLGridContextView Selection;
	Selection.bOwnUnits = true;
	Selection.bMonkUnit = true;
	FHOLGridCommandView Command = HOLCommandGridRules::GetCommand(Selection, HOLCommandGridRules::GetMonkSealCell());
	T.AssertTrue(Command.Command == EHOLGridCommandKind::LightSeal, "the chain gives the Seal of Light command for a selection with a Monk");

	FHOLGridFaceFacts Ready;
	Ready.Numbers.Add(4);
	Ready.Numbers.Add(10);
	Ready.Numbers.Add(30);
	FHOLGridCellLookView ReadyFace = HOLCommandGridFace::GetCellFace(Command, Ready);
	FString ReadyLabel = ReadyFace.Label.ToString();
	T.AssertEquals("Sceau", ReadyLabel, "no cool-down: the label is the short name alone");
	T.AssertTrue(ReadyFace.State == EHOLGridFaceState::Ready, "no cool-down: the Seal is ready");
	T.AssertEquals(FName("LightSeal"), ReadyFace.ReportName, "the report name of the cell");
	FString TipName = ReadyFace.TooltipName.ToString();
	T.AssertEquals("Sceau de lumière", TipName, "the tooltip name");
	FString TipEffect = ReadyFace.TooltipEffect.ToString();
	T.AssertTrue(TipEffect.Contains("zone de 4 m de rayon pendant 10 s") && TipEffect.Contains("ralentis de 30 %"), "the tooltip carries the numbers of the facts");

	FHOLGridFaceFacts Cooling = Ready;
	Cooling.CooldownSeconds = 12;
	FHOLGridCellLookView CoolingFace = HOLCommandGridFace::GetCellFace(Command, Cooling);
	FString CoolingLabel = CoolingFace.Label.ToString();
	T.AssertEquals("Sceau\n12 s", CoolingLabel, "12 s left: the seconds show under the label");
	T.AssertTrue(CoolingFace.State == EHOLGridFaceState::Greyed, "12 s left: the Seal is greyed");

	FHOLGridCommandView Grey = Command;
	Grey.bGreyed = true;
	FHOLGridCellLookView GreyFace = HOLCommandGridFace::GetCellFace(Grey, Ready);
	T.AssertTrue(GreyFace.State == EHOLGridFaceState::Greyed, "a command greyed by the rule stays greyed");
}

// HeroesOfLiberty.CommandGridFace.NecroticRage
void Test_CGFNecroticRage(FUnitTest& T)
{
	FHOLGridContextView Selection;
	Selection.bOwnUnits = true;
	Selection.bRageUnit = true;
	FHOLGridCommandView Command = HOLCommandGridRules::GetCommand(Selection, HOLCommandGridRules::GetRageCell());
	T.AssertTrue(Command.Command == EHOLGridCommandKind::NecroticRage, "the chain gives the rage command for a selection with a Damned Warrior");

	FHOLGridFaceFacts Facts;
	FHOLGridCellLookView Face = HOLCommandGridFace::GetCellFace(Command, Facts);
	FString Label = Face.Label.ToString();
	T.AssertEquals("Rage", Label, "ready: the short name alone");
	T.AssertTrue(Face.State == EHOLGridFaceState::Ready, "ready");
	T.AssertEquals(FName("NecroticRage"), Face.ReportName, "the report name of the cell");

	Facts.CooldownSeconds = 7;
	FHOLGridCellLookView Cooling = HOLCommandGridFace::GetCellFace(Command, Facts);
	FString CoolingLabel = Cooling.Label.ToString();
	T.AssertEquals("Rage\n7 s", CoolingLabel, "7 s left: the seconds show under the label");
	T.AssertTrue(Cooling.State == EHOLGridFaceState::Greyed, "7 s left: greyed");
}

// HeroesOfLiberty.CommandGridFace.HeroAbility
void Test_CGFHeroAbility(FUnitTest& T)
{
	FHOLGridContextView Hero;
	Hero.bOwnUnits = true;
	Hero.bHero = true;
	Hero.HeroLevel = 99;
	Hero.HeroCooldownSeconds.Add(0);
	Hero.HeroCooldownSeconds.Add(0);
	Hero.HeroCooldownSeconds.Add(0);

	FHOLGridFaceFacts None;
	FHOLGridCommandView Banner = HOLCommandGridRules::GetCommand(Hero, HOLCommandGridRules::GetHeroAbilityCell(2));
	FHOLGridCellLookView BannerFace = HOLCommandGridFace::GetCellFace(Banner, None);
	FString BannerLabel = BannerFace.Label.ToString();
	T.AssertEquals("Bannière", BannerLabel, "the Banner's short name");
	T.AssertEquals(FName("Banner"), BannerFace.ReportName, "the Banner's report name");
	T.AssertTrue(BannerFace.State == EHOLGridFaceState::Ready, "the Banner is ready at level 99");

	Hero.HeroCooldownSeconds[1] = 30;
	FHOLGridCommandView Cooling = HOLCommandGridRules::GetCommand(Hero, HOLCommandGridRules::GetHeroAbilityCell(1));
	FHOLGridCellLookView CoolingFace = HOLCommandGridFace::GetCellFace(Cooling, None);
	FString CoolingLabel = CoolingFace.Label.ToString();
	T.AssertEquals("Rangs\n30 s", CoolingLabel, "Close Ranks cooling down shows its seconds");
	T.AssertTrue(CoolingFace.State == EHOLGridFaceState::Greyed, "the chain greys it, the face follows");
	T.AssertEquals(FName("CloseRanks"), CoolingFace.ReportName, "the Close Ranks report name");

	Hero.HeroLevel = 0;
	FHOLGridCommandView Locked = HOLCommandGridRules::GetCommand(Hero, HOLCommandGridRules::GetHeroAbilityCell(2));
	FHOLGridFaceFacts NeedsLevel;
	NeedsLevel.UnlockLevel = 3;
	FHOLGridCellLookView LockedFace = HOLCommandGridFace::GetCellFace(Locked, NeedsLevel);
	FString LockedEffect = LockedFace.TooltipEffect.ToString();
	T.AssertEquals("Niveau 3 requis", LockedEffect, "below its level the tooltip says the level needed");
	FString LockedLabel = LockedFace.Label.ToString();
	T.AssertEquals("Bannière", LockedLabel, "a locked ability shows no seconds");
}
