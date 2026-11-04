// The Diplomacy panel's rows (D387; HOLDiplomacyPanel.as) on the real rules (HOLDiplomacyRules.as): the facts of a row come out of a book that
// SetStance / SetVision built, not from values written by hand. Pure.

// A row of Viewer toward Slot read from the book, as the screen reads the game: the stances of the pair, the vision, the Market.
FHOLDiplomacyRowView DpRow(FHOLDiplomacyBook& Book, int Viewer, int Slot, bool bHasMarket, bool bDefeated = false)
{
	return HOLDiplomacyPanel::MakeRow(!Book.bLocked, Viewer, false, Slot, Book.bAI[Slot], bDefeated, HOLDiplomacy::StanceOf(Book, Viewer, Slot),
		HOLDiplomacy::StanceOf(Book, Slot, Viewer), HOLDiplomacy::SharesVision(Book, Viewer, Slot), bHasMarket, 30);
}

FHOLDiplomacyBook DpHumans(int Slots)
{
	TArray<bool> NoAI;
	for (int Slot = 0; Slot < Slots; Slot++)
		NoAI.Add(false);
	return HOLDiplomacy::NewBook(Slots, NoAI);
}

// An alliance needs both sides (D369) but no waiting state is shown; a break drops both stances to Enemy at once.
void Test_DiplomacyPanelPendingAlliance(FUnitTest& T)
{
	FHOLDiplomacyBook Book = DpHumans(4);
	HOLDiplomacy::SetStance(Book, 0, 1, HOLDiplomacy::Ally);
	FHOLDiplomacyRowView Offered = DpRow(Book, 0, 1, false);
	T.AssertEquals(Offered.MyStance, HOLDiplomacy::Ally, "the viewer's stance shown is Ally (no waiting state: nothing else is written)");
	T.AssertEquals(Offered.Relation, HOLDiplomacy::Enemy, "but the relation is not an alliance yet");
	T.AssertFalse(Offered.bVisionEnabled, "no vision to share before the alliance");
	HOLDiplomacy::SetStance(Book, 1, 0, HOLDiplomacy::Neutral);
	T.AssertEquals(DpRow(Book, 0, 1, false).Relation, HOLDiplomacy::Neutral, "0 Ally, 1 Neutral: the most hostile (D375) is Neutral");
	HOLDiplomacy::SetStance(Book, 1, 0, HOLDiplomacy::Ally);
	FHOLDiplomacyRowView Allied = DpRow(Book, 0, 1, false);
	T.AssertEquals(Allied.Relation, HOLDiplomacy::Ally, "allied");
	T.AssertTrue(Allied.bVisionEnabled, "allies may share their vision");
	T.AssertFalse(Allied.bVisionChecked, "not ticked until asked");
	HOLDiplomacy::SetVision(Book, 0, 1, true);
	T.AssertTrue(DpRow(Book, 0, 1, false).bVisionChecked, "ticked once shared");
	HOLDiplomacy::SetStance(Book, 1, 0, HOLDiplomacy::Enemy);
	FHOLDiplomacyRowView Broken = DpRow(Book, 0, 1, false);
	T.AssertEquals(Broken.MyStance, HOLDiplomacy::Enemy, "the other broke: the viewer's row shows Enemy at once, not Ally (bug of 2026-11-04)");
	T.AssertEquals(Broken.TheirStance, HOLDiplomacy::Enemy, "and the other's stance is Enemy");
	T.AssertFalse(Broken.bVisionChecked, "the break cut the vision at once (D370, D371)");
	T.AssertFalse(Broken.bVisionEnabled, "and it cannot be ticked");
}

// Outside the Diplomacy mode (fixed teams) the stances are shown, greyed and locked; a teammate's vision is automatic (ticked, not changeable).
void Test_DiplomacyPanelLockedInStandard(FUnitTest& T)
{
	TArray<int> Teams;
	TArray<bool> NoAI;
	for (int Slot = 0; Slot < 4; Slot++)
	{
		Teams.Add(Slot < 2 ? 1 : 2);
		NoAI.Add(false);
	}
	FHOLDiplomacyBook Book = HOLDiplomacy::NewTeamBook(Teams, NoAI);
	FHOLDiplomacyRowView Mate = DpRow(Book, 0, 1, true);
	T.AssertFalse(Mate.bStanceEditable, "the postures are locked in the standard modes");
	T.AssertEquals(Mate.MyStance, HOLDiplomacy::Ally, "a teammate is shown Ally");
	T.AssertTrue(Mate.bVisionChecked, "his vision is shared by the team");
	T.AssertFalse(Mate.bVisionEnabled, "and not a choice");
	FHOLDiplomacyRowView Foe = DpRow(Book, 0, 2, true);
	T.AssertEquals(Foe.MyStance, HOLDiplomacy::Enemy, "the other team is shown Enemy");
	T.AssertFalse(Foe.bStanceEditable, "locked as well");
	T.AssertTrue(Foe.bTributeEnabled, "tributes stay available in the standard modes (D387)");
	T.AssertTrue(Foe.bMessageEnabled, "and the messages");
}

// The AIs are always Enemy and take no stance (D367); the viewer's own line has no control; a spectator or a defeated viewer sets nothing.
void Test_DiplomacyPanelAIsSelfAndSpectator(FUnitTest& T)
{
	TArray<bool> Ais;
	Ais.Add(false);
	Ais.Add(false);
	Ais.Add(true);
	FHOLDiplomacyBook Book = HOLDiplomacy::NewBook(3, Ais);
	FHOLDiplomacyRowView Machine = DpRow(Book, 0, 2, true);
	T.AssertFalse(Machine.bStanceEditable, "no stance toward an AI");
	T.AssertEquals(Machine.MyStance, HOLDiplomacy::Enemy, "an AI is Enemy");
	T.AssertFalse(Machine.bMessageEnabled, "no private message to an AI");
	T.AssertTrue(Machine.bTributeEnabled, "a tribute to an AI is allowed (any player, D372)");
	FHOLDiplomacyRowView Self = DpRow(Book, 0, 0, true);
	T.AssertTrue(Self.bSelf, "the viewer's own line");
	T.AssertFalse(Self.bStanceEditable || Self.bTributeEnabled || Self.bMessageEnabled || Self.bVisionEnabled, "no control on it");
	FHOLDiplomacyRowView Spectator = HOLDiplomacyPanel::MakeRow(true, -1, false, 1, false, false, HOLDiplomacy::Enemy, HOLDiplomacy::Enemy, false, false, 30);
	T.AssertFalse(Spectator.bStanceEditable || Spectator.bTributeEnabled || Spectator.bMessageEnabled, "a spectator sets nothing");
	FHOLDiplomacyRowView Beaten = HOLDiplomacyPanel::MakeRow(true, 0, true, 1, false, false, HOLDiplomacy::Enemy, HOLDiplomacy::Enemy, false, true, 30);
	T.AssertFalse(Beaten.bStanceEditable || Beaten.bTributeEnabled || Beaten.bMessageEnabled, "a defeated viewer sets nothing");
	FHOLDiplomacyRowView Dead = DpRow(Book, 0, 1, true, true);
	T.AssertFalse(Dead.bStanceEditable || Dead.bTributeEnabled || Dead.bMessageEnabled, "nothing toward a defeated player");
}

// The tribute is greyed without a finished Market (the tax stays shown), enabled with one; the stance cycle follows the GDD's order.
void Test_DiplomacyPanelTributeAndCycle(FUnitTest& T)
{
	FHOLDiplomacyBook Book = DpHumans(3);
	FHOLDiplomacyRowView NoMarket = DpRow(Book, 0, 1, false);
	T.AssertFalse(NoMarket.bTributeEnabled, "no Market: greyed");
	T.AssertTrue(NoMarket.bNoMarket, "and the reason is the Market");
	T.AssertEquals(NoMarket.TaxPercent, 30, "the tax is shown all the same");
	T.AssertTrue(DpRow(Book, 0, 1, true).bTributeEnabled, "with a Market: enabled");
	T.AssertTrue(NoMarket.Describe.Contains("tribute no market tax 30"), "the report says it");
	T.AssertEquals(HOLDiplomacyPanel::NextStance(HOLDiplomacy::Ally), HOLDiplomacy::Neutral, "Allié -> Neutre");
	T.AssertEquals(HOLDiplomacyPanel::NextStance(HOLDiplomacy::Neutral), HOLDiplomacy::Enemy, "Neutre -> Ennemi");
	T.AssertEquals(HOLDiplomacyPanel::NextStance(HOLDiplomacy::Enemy), HOLDiplomacy::Ally, "Ennemi -> Allié");
}
