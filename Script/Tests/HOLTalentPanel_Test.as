// The talent panel's view (T5, HOLTalentPanel.as): what the screen shows of the hero's talents, from his level and his picks alone.
// Families: 0 Guardian, 1 Captain, 2 Champion. The prototype shows the rows its maximum level reaches (HOLHero::GetMaxLevel, 6: rows 2 to 6).

TArray<int> PanelPicks(int F0 = -1, int F1 = -1, int F2 = -1, int F3 = -1, int F4 = -1)
{
	TArray<int> Picks;
	if (F0 >= 0) Picks.Add(F0);
	if (F1 >= 0) Picks.Add(F1);
	if (F2 >= 0) Picks.Add(F2);
	if (F3 >= 0) Picks.Add(F3);
	if (F4 >= 0) Picks.Add(F4);
	return Picks;
}

// Level 1: nothing to take, no reminder, no advice, the rows of the prototype shown closed.
void Test_TalentPanelLevelOne(FUnitTest& T)
{
	const FHOLTalentPanelView View = HOLTalentPanel::Make(1, PanelPicks());
	T.AssertEquals(0, View.Points, "no point at level 1");
	T.AssertFalse(View.bReminder, "no reminder");
	T.AssertFalse(View.bAdviceEnabled, "nothing to advise");
	T.AssertEquals(HOLHero::GetMaxLevel() - 1, View.Rows.Num(), "the rows of the prototype: 2 to its maximum level");
	T.AssertEquals("points 0 | w . . . . | advised none | seals 0 keys 0", View.Describe, "the stable line");
}

// A point earned and kept (D105): the reminder is on, the next row can be taken, no advice before a first talent.
void Test_TalentPanelPointInReserve(FUnitTest& T)
{
	const FHOLTalentPanelView View = HOLTalentPanel::Make(3, PanelPicks(1));
	T.AssertEquals(1, View.Points, "level 3, one taken: one kept");
	T.AssertTrue(View.bReminder, "the reminder of an unspent point");
	T.AssertTrue(View.Rows[0].bTaken, "row 2 taken");
	T.AssertEquals(1, View.Rows[0].Family, "by the Captain");
	T.AssertTrue(View.Rows[1].bNext && View.Rows[1].bCanPick, "row 3 next and takeable");
	T.AssertFalse(View.Rows[2].bNext, "row 4 not next");
	T.AssertTrue(View.bAdviceEnabled, "a first talent exists and a point is kept: the advice is on");
	T.AssertEquals(1, View.AdvisedFamily, "the advice is the Captain");
	T.AssertEquals("points 1 | 1 n . . . | advised 1 | seals 0 keys 0", View.Describe, "the stable line");
}

// No point left: no reminder, the next row not takeable, no advice button (nothing to spend it on).
void Test_TalentPanelNoPointLeft(FUnitTest& T)
{
	const FHOLTalentPanelView View = HOLTalentPanel::Make(3, PanelPicks(1, 1));
	T.AssertEquals(0, View.Points, "level 3, two taken: none kept");
	T.AssertFalse(View.bReminder, "no reminder");
	T.AssertTrue(View.Rows[2].bNext, "row 4 is the next row");
	T.AssertFalse(View.Rows[2].bCanPick, "but no point to take it");
	T.AssertFalse(View.bAdviceEnabled, "no point, no advice button");
	T.AssertEquals("points 0 | 1 1 w . . | advised 1 | seals 0 keys 0", View.Describe, "the stable line (w: next row, waiting for a point)");
}

// The seal at three of a family and the key at five (D103) on the sheet; the prototype reaches the key at level 6 with one family.
void Test_TalentPanelSealAndKey(FUnitTest& T)
{
	const FHOLTalentPanelView Seal = HOLTalentPanel::Make(4, PanelPicks(0, 2, 0));
	T.AssertEquals(0, Seal.Seals, "two Guardian talents, one Champion: no seal");
	const FHOLTalentPanelView WithSeal = HOLTalentPanel::Make(4, PanelPicks(0, 0, 0));
	T.AssertEquals(1, WithSeal.Seals, "three Guardian talents: a seal");
	T.AssertEquals(0, WithSeal.Keys, "no key yet");
	const FHOLTalentPanelView Key = HOLTalentPanel::Make(HOLHero::GetMaxLevel(), PanelPicks(2, 2, 2, 2, 2));
	T.AssertEquals(1, Key.Keys, "five Champion talents: the key talent");
	T.AssertEquals(1, Key.Seals, "... and the seal that comes with it");
	T.AssertEquals("points 0 | 2 2 2 2 2 | advised 2 | seals 1 keys 1", Key.Describe, "the stable line");
}

// The advice follows the most filled family, a tie going to the latest (TQ1 = A), as the rules (HOLTalent::AdvisedFamily).
void Test_TalentPanelAdviceFollowsTheRules(FUnitTest& T)
{
	const FHOLTalentPanelView Tie = HOLTalentPanel::Make(5, PanelPicks(0, 1));
	T.AssertEquals(HOLTalent::AdvisedFamily(PanelPicks(0, 1)), Tie.AdvisedFamily, "the same advice as the rules");
	T.AssertEquals(1, Tie.AdvisedFamily, "a tie: the latest, the Captain");
	const FHOLTalentPanelView Most = HOLTalentPanel::Make(5, PanelPicks(0, 1, 0));
	T.AssertEquals(0, Most.AdvisedFamily, "two Guardian against one Captain: the Guardian");
}

// A key talent out of reach in the prototype: with two families started by row 3, the key can no longer come (D103) and the panel says so.
void Test_TalentPanelKeyReach(FUnitTest& T)
{
	const FHOLTalentPanelView Open = HOLTalentPanel::Make(3, PanelPicks(0, 0));
	T.AssertTrue(Open.FamilyKeyReachable[0], "Guardian only: the key is reachable");
	T.AssertFalse(Open.FamilyKeyReachable[1], "the Captain has none and 3 points left at most: out of reach");
	const FHOLTalentPanelView Lost = HOLTalentPanel::Make(5, PanelPicks(0, 1));
	T.AssertFalse(Lost.FamilyKeyReachable[0], "one Guardian, one Captain: neither reaches five in 5 points");
	T.AssertFalse(Lost.FamilyKeyReachable[1], "... nor the Captain");
}

// TQ4 (PROVISIONAL, not decided): where the very first talent is chosen. Option B (default, read from the game's setting): three family buttons while a
// point is kept, the advised button still waiting for a first talent. Option C: the advised button takes the Captain (the family with effects).
void Test_TalentPanelFirstChoiceOptions(FUnitTest& T)
{
	T.AssertEquals(HOLTalentPanel::ModeButtons, HOLHero::GetTalentFirstChoiceMode(), "the setting's default is B, three buttons");
	const FHOLTalentPanelView Buttons = HOLTalentPanel::MakeFor(2, PanelPicks(), HOLTalentPanel::ModeButtons);
	T.AssertTrue(Buttons.bFamilyButtons, "B: the three buttons while a point is kept");
	T.AssertFalse(Buttons.bAdviceEnabled, "B: no advice before a first talent");
	T.AssertEquals(2, Buttons.NextRow, "the first talent fills row 2");
	T.AssertFalse(HOLTalentPanel::MakeFor(1, PanelPicks(), HOLTalentPanel::ModeButtons).bFamilyButtons, "no point, no buttons");
	const FHOLTalentPanelView Default = HOLTalentPanel::MakeFor(2, PanelPicks(), HOLTalentPanel::ModeDefault);
	T.AssertFalse(Default.bFamilyButtons, "C: no family buttons");
	T.AssertTrue(Default.bAdviceEnabled, "C: the advised button is on");
	T.AssertEquals(1, Default.AdvisedFamily, "C: the Captain, the family with effects in the game");
	T.AssertEquals(HOLAITalent::EffectFamilies()[0], Default.AdvisedFamily, "the same family the AI draws from");
	const FHOLTalentPanelView After = HOLTalentPanel::MakeFor(3, PanelPicks(0), HOLTalentPanel::ModeDefault);
	T.AssertEquals(0, After.AdvisedFamily, "C: once a talent is taken the most filled family wins");
	T.AssertEquals(HOLTalentPanel::Make(2, PanelPicks()).Describe, Buttons.Describe, "Make is option B");
	T.AssertTrue(HOLTalentText::FamilyTooltip(Buttons, 0).Contains("2"), "the tooltip names the row");
	T.AssertTrue(HOLTalentText::FamilyName(0) != HOLTalentText::FamilyName(1), "three distinct names");
}