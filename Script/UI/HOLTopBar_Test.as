// NOT COMPILED YET (needs the C++ lot of ELROND-1119a for HOLTopBar.as, same module). H-3: the top bar's rules (HOLTopBar.as), pure, in literals copied from the
// C++ screen's behaviour (HOLGameScreen.cpp RefreshTopBar / RefreshIdleWorkers / Describe): the amounts without grouping, "(3)", "+90 bois/min", "12 / 50", the
// report's economy and workers lines (read by quick_check: ", gatherers F 0 W 1 S 0 G 0, idle 0"), the idle counter's label and enabled state.

FHOLResources TopBarRes(int Food, int Wood, int Stone, int Gold)
{
	FHOLResources Result;
	Result.Food = Food;
	Result.Wood = Wood;
	Result.Stone = Stone;
	Result.Gold = Gold;
	return Result;
}

void Test_TopBarAmountsHaveNoGroupingAndRatioIsSpaced(FUnitTest& T)
{
	T.AssertEquals(HOLTopBar::FormatAmount(1200), "1200", "AoE's way: no thousands separator");
	T.AssertEquals(HOLTopBar::FormatAmount(0), "0", "zero");
	T.AssertEquals(HOLTopBar::FormatRatio(12, 50), "12 / 50", "the population, used / cap");
	T.AssertEquals(HOLTopBar::FormatRatio(1000, 1000), "1000 / 1000", "no grouping in the ratio either");
	T.AssertEquals(HOLTopBar::GatherersText(3), "(3)", "a resource's gatherers in brackets");
}

void Test_TopBarIncomeTipNamesTheResource(FUnitTest& T)
{
	T.AssertEquals(HOLTopBar::IncomeTip(90, HOLTopBar::ResourceWord(HOLTopBar::Wood())), "+90 bois/min", "the wood tip");
	T.AssertEquals(HOLTopBar::IncomeTip(0, HOLTopBar::ResourceWord(HOLTopBar::Gold())), "+0 or/min", "the gold tip, nothing held");
	T.AssertEquals(HOLTopBar::ResourceWord(HOLTopBar::Food()), "nourriture", "food");
	T.AssertEquals(HOLTopBar::ResourceWord(HOLTopBar::Stone()), "pierre", "stone");
	FHOLResources Stock = TopBarRes(10, 20, 30, 40);
	T.AssertEquals(HOLTopBar::AmountOf(Stock, HOLTopBar::Stone()), 30, "the stone's amount of a stock");
	T.AssertEquals(HOLTopBar::AmountOf(Stock, HOLTopBar::Gold()), 40, "the gold's amount of a stock");
	T.AssertEquals(HOLTopBar::ResourceName(HOLTopBar::Stone()), "Stone", "the widget names' prefix");
}

void Test_TopBarReportLinesAreTheCppOnes(FUnitTest& T)
{
	T.AssertEquals(HOLTopBar::ResourcesLine(TopBarRes(120, 85, 0, 30)), "F 120 W 85 S 0 G 30", "FHOLResources::ToString");
	T.AssertEquals(HOLTopBar::EconomyLine(true, TopBarRes(120, 85, 0, 30), 12, 50), "F 120 W 85 S 0 G 30, population 12/50", "economy line, no spaces round the slash");
	T.AssertEquals(HOLTopBar::EconomyLine(false, TopBarRes(1, 2, 3, 4), 5, 6), "none", "no player state yet: none");
	T.AssertEquals(HOLTopBar::WorkersLine(true, TopBarRes(0, 1, 0, 0), 0), ", gatherers F 0 W 1 S 0 G 0, idle 0", "after the order: one on wood, none idle");
	T.AssertEquals(HOLTopBar::WorkersLine(true, TopBarRes(0, 0, 0, 0), 1), ", gatherers F 0 W 0 S 0 G 0, idle 1", "before the order: one idle");
	T.AssertEquals(HOLTopBar::WorkersLine(true, TopBarRes(0, 0, 0, 0), -1), ", gatherers F 0 W 0 S 0 G 0, idle 0", "never shown yet: idle 0, not -1");
	T.AssertEquals(HOLTopBar::WorkersLine(false, TopBarRes(0, 0, 0, 0), 1), "", "no player state: nothing");
	T.AssertTrue(HOLTopBar::BarShown(true), "the bar shows with a player state");
	T.AssertFalse(HOLTopBar::BarShown(false), "and not before");
}

void Test_TopBarIdleCounterLabelEnabledAndChange(FUnitTest& T)
{
	T.AssertEquals(HOLTopBar::IdleLabel(0), "Inactifs : 0", "the label at zero");
	T.AssertEquals(HOLTopBar::IdleLabel(3), "Inactifs : 3", "the label with three");
	T.AssertFalse(HOLTopBar::IdleEnabled(0), "no idle worker: the button is disabled (a click is refused)");
	T.AssertTrue(HOLTopBar::IdleEnabled(1), "one idle worker: enabled");
	T.AssertTrue(HOLTopBar::IdleChanged(-1, 0), "the first count is applied even at 0");
	T.AssertFalse(HOLTopBar::IdleChanged(2, 2), "the same count: nothing to redraw");
	T.AssertTrue(HOLTopBar::IdleChanged(2, 1), "a different count: redrawn");
	T.AssertEquals(HOLTopBar::IdleClickLine(), "HUD: idle workers clicked", "the journal line of a click");
}

void Test_TopBarIncomeIsSetAgainOnlyWhenItChanges(FUnitTest& T)
{
	FHOLResources Shown = TopBarRes(0, 90, 0, 10);
	T.AssertFalse(HOLTopBar::IncomeChanged(Shown, TopBarRes(0, 90, 0, 10)), "same income: the tooltips are kept");
	T.AssertTrue(HOLTopBar::IncomeChanged(Shown, TopBarRes(0, 90, 0, 20)), "gold changes: set again");
	T.AssertTrue(HOLTopBar::IncomeChanged(Shown, TopBarRes(5, 90, 0, 10)), "food changes: set again");
}

// D814 (Q-income-tooltip-taille = B): the income tooltip is a HUD panel, computed by the rule from the income and the resource: the text names the income and
// the resource, the fill is dark and near-opaque (the HUD's), the border is gold with a width.
void Test_TopBarIncomeTipIsAHudPanel(FUnitTest& T)
{
	HOLTopBar::FIncomeTipView View = HOLTopBar::IncomeTipView(90, HOLTopBar::Wood());
	T.AssertTrue(View.Text.Contains(HOLTopBar::FormatAmount(90)), "the tooltip text carries the income");
	T.AssertTrue(View.Text.Contains(HOLTopBar::ResourceWord(HOLTopBar::Wood())), "the tooltip text names the resource");
	T.AssertTrue(View.Fill.A >= 0.9 && View.Fill.R < 0.15 && View.Fill.G < 0.15 && View.Fill.B < 0.15, "the fill is the HUD's dark, near-opaque one");
	T.AssertTrue(View.Border.R > 0.7 && View.Border.R > View.Border.G && View.Border.G > View.Border.B && View.Border.A > 0.9, "the border is gold");
	T.AssertTrue(View.BorderWidth >= 1.0 && View.Padding > 0.0, "the border has a width and the text a padding");
}
