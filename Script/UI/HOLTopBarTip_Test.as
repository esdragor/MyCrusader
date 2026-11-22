// GALADRIEL-1122d: the income tooltip is dropped under the "Scenario" banner only when ITS rectangle crosses the banner horizontally (before, SetBannerRect gave
// every one of the 8 tooltips the banner's own left edge and width, so all 8 dropped: the wood tooltip sat ~55 px under the bar at x 230 while the banner
// starts at x 435). The rectangle of a tooltip is its cell's cursor point (the cell's centre) and its width (the real FR/EN text through the view). Every value
// is computed by the real rule (HOLTopBar::IncomeTipDropForCell, IncomeTipView, IncomeTipWidth, IncomeTipNaturalTop), none copied.

namespace HOLTopBarTipTest
{
	float WidthOf(int Resource, int Income)
	{
		HOLTopBar::FIncomeTipView View = HOLTopBar::IncomeTipView(Income, Resource);
		return HOLTopBar::IncomeTipWidth(View.Text, View.Padding);
	}
}

void Test_TopBarTipOutsideTheBannerKeepsItsNaturalPlace(FUnitTest& T)
{
	// A 1280 screen: the banner spans x 435-845, bottom 78 (the income_tooltip scenario); the wood cell's cursor point is x 222 (FR), far left of it.
	const float Top = HOLTopBar::IncomeTipNaturalTop();
	const float WoodWidth = HOLTopBarTipTest::WidthOf(HOLTopBar::Wood(), 377);
	const float Drop = HOLTopBar::IncomeTipDropForCell(222.0, WoodWidth, 435.0, 845.0, 78.0);
	T.AssertEquals(Drop, 0.0, "a tooltip wholly left of the banner keeps its natural place (no drop)");
	const float RightDrop = HOLTopBar::IncomeTipDropForCell(900.0, WoodWidth, 435.0, 845.0, 78.0);
	T.AssertEquals(RightDrop, 0.0, "a tooltip wholly right of the banner keeps its natural place");
	T.AssertTrue(Top + Drop < 78.0, "the kept place is the natural one, above the banner's bottom edge");
}

void Test_TopBarTipCrossingTheBannerGoesUnderIt(FUnitTest& T)
{
	const float Top = HOLTopBar::IncomeTipNaturalTop();
	const float GoldWidth = HOLTopBarTipTest::WidthOf(HOLTopBar::Gold(), 0);
	// The gold cell's cursor point is x 443, inside the banner 435-845: the tooltip crosses it and goes under its bottom edge plus the gap.
	const float Drop = HOLTopBar::IncomeTipDropForCell(443.0, GoldWidth, 435.0, 845.0, 78.0);
	T.AssertTrue(Drop > 0.0, "a tooltip crossing the banner is moved");
	T.AssertEquals(Top + Drop, 78.0 + HOLTopBar::IncomeTipGap(), "its top lands under the banner's bottom edge by the gap");
	// A tooltip that starts left of the banner but whose width reaches into it crosses it too.
	const float EdgeDrop = HOLTopBar::IncomeTipDropForCell(400.0, GoldWidth, 435.0, 845.0, 78.0);
	T.AssertTrue(EdgeDrop > 0.0, "a tooltip starting left of the banner and reaching into it is moved");
}

void Test_TopBarTipWithoutBannerKeepsItsPlace(FUnitTest& T)
{
	const float GoldWidth = HOLTopBarTipTest::WidthOf(HOLTopBar::Gold(), 0);
	const float NoBanner = HOLTopBar::IncomeTipDropForCell(443.0, GoldWidth, 0.0, 0.0, 0.0);
	T.AssertEquals(NoBanner, 0.0, "no banner: the tooltip keeps its place");
}
