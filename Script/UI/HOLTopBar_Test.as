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

// GALADRIEL-1121se (RECETTE-1121sb): "Inactifs : 0" stayed French after a live switch to EN/JA, because Refresh re-applied the idle label only when the COUNT
// changed (the income tips already follow the language via TipLanguageKey). The label must be applied again when the text shown differs from the text the
// current language gives for the same count. Pure, so it does not depend on the editor applying the game localization: the "shown" text is the label of
// ANOTHER language than the current one (a stale one), built here from the real IdleLabel for the "same" case.
void Test_TopBarIdleLabelIsSetAgainOnALanguageSwitchWithTheCountUnchanged(FUnitTest& T)
{
	const FString Current = HOLTopBar::IdleLabel(0);
	T.AssertFalse(HOLTopBar::IdleChanged(0, 0), "the count did not change: the count gate alone keeps the old text");
	T.AssertFalse(HOLTopBar::IdleLabelChanged(Current, 0), "the label shown is the current language one: nothing to redraw");
	T.AssertTrue(HOLTopBar::IdleLabelChanged("stale other-language label : 0", 0), "the label shown is another language one: the label is set again");
	T.AssertTrue(HOLTopBar::IdleLabelChanged("", 0), "never shown: set");
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
// RECETTE-1121sd: the French gold tooltip must not cover the "Scénario" banner. The banner's top comes from the real HOLScenarioPanelRules::PanelTop, the
// tooltip's width from the real FR text through the view, the drop from the real rule; the tooltip's rectangle (left, top + drop, width) is then checked
// against the banner's rectangle. A banner is 860 wide, centred on a 1920 screen (left 530, right 1390), 70 high.
bool TopBarTipOverlapsBanner(float TipLeft, float TipTop, float TipWidth, float TipHeight, float BannerLeft, float BannerRight, float BannerTop, float BannerBottom)
{
	return TipLeft < BannerRight && TipLeft + TipWidth > BannerLeft && TipTop < BannerBottom && TipTop + TipHeight > BannerTop;
}

void Test_TopBarGoldTipDoesNotCoverTheScenarioBanner(FUnitTest& T)
{
	HOLTopBar::FIncomeTipView View = HOLTopBar::IncomeTipView(0, HOLTopBar::Gold());
	const float TipWidth = HOLTopBar::IncomeTipWidth(View.Text, View.Padding);
	const float TipHeight = 28.0;
	const float BannerTop = HOLScenarioPanelRules::PanelTop(40.0);
	const float BannerBottom = BannerTop + 70.0;
	const float TipLeft = 650.0;
	const float TipTop = 56.0;
	const float Drop = HOLTopBar::IncomeTipDrop(TipLeft, TipTop, TipWidth, 530.0, 1390.0, BannerBottom);
	T.AssertTrue(TopBarTipOverlapsBanner(TipLeft, TipTop, TipWidth, TipHeight, 530.0, 1390.0, BannerTop, BannerBottom), "without the drop the FR gold tooltip is on the banner (the bug)");
	T.AssertFalse(TopBarTipOverlapsBanner(TipLeft, TipTop + Drop, TipWidth, TipHeight, 530.0, 1390.0, BannerTop, BannerBottom), "with the drop the tooltip is clear of the banner");
	T.AssertTrue(Drop > 0.0, "the tooltip is moved");
	const float NoBanner = HOLTopBar::IncomeTipDrop(TipLeft, TipTop, TipWidth, 0.0, 0.0, 0.0);
	T.AssertEquals(NoBanner, 0.0, "no banner: the tooltip's position is unchanged");
	const float Left = HOLTopBar::IncomeTipDrop(100.0, TipTop, 80.0, 530.0, 1390.0, BannerBottom);
	T.AssertEquals(Left, 0.0, "a tooltip wholly left of the banner is not moved");
}

void Test_TopBarIncomeTipIsAHudPanel(FUnitTest& T)
{
	HOLTopBar::FIncomeTipView View = HOLTopBar::IncomeTipView(90, HOLTopBar::Wood());
	T.AssertTrue(View.Text.Contains(HOLTopBar::FormatAmount(90)), "the tooltip text carries the income");
	T.AssertTrue(View.Text.Contains(HOLTopBar::ResourceWord(HOLTopBar::Wood())), "the tooltip text names the resource");
	T.AssertTrue(View.Fill.A >= 0.9 && View.Fill.R < 0.15 && View.Fill.G < 0.15 && View.Fill.B < 0.15, "the fill is the HUD's dark, near-opaque one");
	T.AssertTrue(View.Border.R > 0.7 && View.Border.R > View.Border.G && View.Border.G > View.Border.B && View.Border.A > 0.9, "the border is gold");
	T.AssertTrue(View.BorderWidth >= 1.0 && View.Padding > 0.0, "the border has a width and the text a padding");
}
