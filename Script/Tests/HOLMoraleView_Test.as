// The morale pastille of the selected unit (S2-d, HOLMoraleView.as). Every state is built on the real chain (HOLMorale::Apply of the real table's rows,
// HOLMorale::GrantImmunity); the text asserted is CALCULATED by HOLMoraleView::Present from that state. NOT PLAYED when written (stale binary).

HOLMoraleView::FHOLMoraleView MoraleViewInject(const FHOLMoraleStateData& State, bool bOwn, bool bPeasant)
{
	HOLMoraleView::FHOLMoraleView View;
	View.bAvailable = true;
	View.bOwn = bOwn;
	View.bPeasant = bPeasant;
	View.State = State;
	return View;
}

FHOLMoraleStateData MoraleViewPose(const FHOLMoraleStateData& Before, int Id)
{
	FHOLMoraleStateData State = Before;
	HOLMorale::Apply(State, HOLMoraleTable::Effect(Id));
	return State;
}

// Until the replicated mask exists, the one reading point gives an empty view and the pastille stays hidden: the seam is in place.
void Test_MoraleViewEmptyUntilMask(FUnitTest& T)
{
	const HOLMoraleView::FHOLMoraleView View = HOLMoraleView::Read(0, true, false);
	T.AssertFalse(View.bAvailable, "Read gives nothing before the replicated effects mask exists");
	T.AssertFalse(HOLMoraleView::Present(View).bShown, "no pastille without a view");
}

// A view injected on our unit under the cry: the pastille appears, with the table's name, the seconds left and the green colour.
void Test_MoraleViewPastilleAppearsWhenInjected(FUnitTest& T)
{
	const FHOLMoraleStateData State = MoraleViewPose(FHOLMoraleStateData(), HOLMoraleTable::CloseRanks);
	const HOLMoraleView::FHOLMoralePastille Pastille = HOLMoraleView::Present(MoraleViewInject(State, true, false));
	T.AssertTrue(Pastille.bShown, "an effect on our unit shows the pastille");
	T.AssertTrue(Pastille.Text.Contains(HOLMoraleTable::Name(HOLMoraleTable::CloseRanks).ToString()), "the line carries the effect's name");
	const int Seconds = HOLMoraleBlock::SecondsOf(HOLMoraleTable::Effect(HOLMoraleTable::CloseRanks).DurationMs);
	T.AssertTrue(Pastille.Text.Contains(f"{Seconds} s"), "our unit shows the time left of the real effect");
	const HOLMoraleBlock::FHOLMoraleBadge Badge = HOLMoraleBlock::Badge(State, false);
	T.AssertTrue(Badge.bBonus && !Badge.bMalus, "the state is a bonus alone");
	T.AssertTrue(Pastille.Color.G > Pastille.Color.R, "green for a bonus");
}

// The labels of D866 (Triomphe, Demoralise, Hesitation) come out of the pastille, a malus makes it red, an enemy shows no time.
void Test_MoraleViewLabelsAndMalusColour(FUnitTest& T)
{
	const HOLMoraleView::FHOLMoralePastille Triumph = HOLMoraleView::Present(MoraleViewInject(MoraleViewPose(FHOLMoraleStateData(), HOLMoraleTable::Triumph), true, false));
	T.AssertTrue(Triumph.Text.Contains("Triomphe"), "Triumph is labelled Triomphe");
	const HOLMoraleView::FHOLMoralePastille Hesitation = HOLMoraleView::Present(MoraleViewInject(MoraleViewPose(FHOLMoraleStateData(), HOLMoraleTable::Hesitation), false, false));
	T.AssertTrue(Hesitation.Text.Contains("Hésitation"), "Hesitation is labelled Hesitation");
	T.AssertFalse(Hesitation.Text.Contains(" s"), "an enemy unit shows its effect without a time");
	T.AssertTrue(Hesitation.Color.R > Hesitation.Color.G, "red for a malus");
	const FHOLMoraleStateData Both = MoraleViewPose(MoraleViewPose(FHOLMoraleStateData(), HOLMoraleTable::CloseRanks), HOLMoraleTable::Demoralised);
	const HOLMoraleView::FHOLMoralePastille Mixed = HOLMoraleView::Present(MoraleViewInject(Both, true, false));
	T.AssertTrue(Mixed.Text.Contains("Démoralisé"), "Demoralised is labelled Demoralise");
	T.AssertTrue(Mixed.Text.Find(HOLMoraleTable::Name(HOLMoraleTable::CloseRanks).ToString()) < Mixed.Text.Find("Démoralisé"), "the bonus comes before the malus");
	T.AssertTrue(Mixed.Color.R > Mixed.Color.G, "a malus wins the colour");
}

// A Peasant never shows a pastille (D705); a view with no effect shows nothing; the immunity alone shows the gold pastille.
void Test_MoraleViewPeasantNoneAndImmunity(FUnitTest& T)
{
	const FHOLMoraleStateData State = MoraleViewPose(FHOLMoraleStateData(), HOLMoraleTable::Demoralised);
	T.AssertFalse(HOLMoraleView::Present(MoraleViewInject(State, true, true)).bShown, "a Peasant shows nothing, ever");
	T.AssertFalse(HOLMoraleView::Present(MoraleViewInject(FHOLMoraleStateData(), true, false)).bShown, "no effect, no pastille");
	FHOLMoraleStateData Immune;
	HOLMorale::GrantImmunity(Immune, 5000);
	const HOLMoraleView::FHOLMoralePastille Gold = HOLMoraleView::Present(MoraleViewInject(Immune, true, false));
	T.AssertTrue(Gold.bShown, "the immunity alone shows a pastille");
	T.AssertTrue(Gold.Color.R > Gold.Color.B && Gold.Color.G > Gold.Color.B, "gold: red and green above blue");
}
