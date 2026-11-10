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

// E-diplomacy_locked (the user, 2026-11-05): no switch button, FOUR tribute buttons a line, one per resource (gold, food, wood, stone), each one giving its own
// resource when clicked. The panel's controls are told apart by a kind (the click) and a name (hol.HUD.Click): four distinct ones, each giving its resource back,
// none of them one of the panel's other kinds (0..10); the labels are the game's resource names, one each.
void Test_DiplomacyPanelFourTributeButtons(FUnitTest& T)
{
	T.AssertEquals(4, HOLDiplomacyPanel::TributeButtonCount, "four buttons");
	T.AssertEquals(3, HOLDiplomacyPanel::TributeResource(0), "gold first");
	TArray<bool> bSeen;
	bSeen.SetNum(4);
	for (int Index = 0; Index < HOLDiplomacyPanel::TributeButtonCount; Index++)
	{
		const int Resource = HOLDiplomacyPanel::TributeResource(Index);
		T.AssertTrue(Resource >= 0 && Resource < 4, "a game resource");
		if (Resource >= 0 && Resource < 4)
		{
			T.AssertFalse(bSeen[Resource], "each resource once");
			bSeen[Resource] = true;
		}
		const int Kind = HOLDiplomacyPanel::TributeKind(Resource);
		T.AssertTrue(Kind > 10, "a kind of its own, none of the panel's other controls (0..10)");
		T.AssertEquals(Resource, HOLDiplomacyPanel::TributeResourceOfKind(Kind), "the kind gives its resource back");
		T.AssertEquals(Resource, HOLDiplomacyPanel::TributeResourceOfName(HOLDiplomacyPanel::TributeControlName(1, Resource)), "so does the control's name");
		for (int Other = 0; Other < Index; Other++)
		{
			const int OtherResource = HOLDiplomacyPanel::TributeResource(Other);
			T.AssertTrue(HOLDiplomacyPanel::TributeKind(OtherResource) != Kind, "distinct kinds");
			T.AssertTrue(HOLDiplomacyPanel::TributeControlName(1, OtherResource) != HOLDiplomacyPanel::TributeControlName(1, Resource), "distinct names");
			T.AssertTrue(HOLDiplomacyPanelText::ResourceName(OtherResource) != HOLDiplomacyPanelText::ResourceName(Resource), "distinct labels");
		}
	}
	T.AssertTrue(HOLDiplomacyPanel::TributeControlName(1, 3) != HOLDiplomacyPanel::TributeControlName(2, 3), "a name a line");
	T.AssertEquals(-1, HOLDiplomacyPanel::TributeResourceOfKind(6), "the old single tribute kind gives none");
	T.AssertEquals(-1, HOLDiplomacyPanel::TributeResourceOfKind(10), "nor the old switch kind");
	T.AssertEquals(-1, HOLDiplomacyPanel::TributeResourceOfName(n"DipResource"), "no switch control any more");
	T.AssertTrue("Tribut 100 or" == HOLDiplomacyPanelText::Format(HOLDiplomacyPanelText::Tribute, HOLDiplomacyPanelText::ResourceName(3)), "the label is the existing one, \"Tribut 100 or\"");
}

// D768: each line of the Diplomacy mode tells how the OTHER considers the viewer ("Sa posture : Neutre"), read from the real book. Lowering a
// relation drops the other's stance toward the viewer too; raising it (Ally) never raises the other's.
void Test_DiplomacyPanelTheirStanceShown(FUnitTest& T)
{
	FHOLDiplomacyBook Book = DpHumans(4);
	FHOLDiplomacyRowView Start = DpRow(Book, 0, 1, false);
	T.AssertTrue(Start.bShowTheirStance, "the other's stance is shown on a human's line in the Diplomacy mode");
	T.AssertEquals(Start.TheirStance, HOLDiplomacy::Enemy, "at launch everybody is Enemy to everybody (D645)");
	T.AssertEquals("Sa posture : Ennemi", HOLDiplomacyPanelText::TheirStanceText(Start.TheirStance), "the label");
	// I say Ally: mine changes, his stays (an alliance needs both sides).
	HOLDiplomacy::SetStance(Book, 0, 1, HOLDiplomacy::Ally);
	FHOLDiplomacyRowView Offered = DpRow(Book, 0, 1, false);
	T.AssertEquals(Offered.MyStance, HOLDiplomacy::Ally, "my stance is Ally");
	T.AssertEquals(Offered.TheirStance, HOLDiplomacy::Enemy, "his stance did not follow me up");
	// He says Neutral, then Ally: the line follows what he said.
	HOLDiplomacy::SetStance(Book, 1, 0, HOLDiplomacy::Neutral);
	T.AssertEquals("Sa posture : Neutre", HOLDiplomacyPanelText::TheirStanceText(DpRow(Book, 0, 1, false).TheirStance), "he says Neutral");
	HOLDiplomacy::SetStance(Book, 1, 0, HOLDiplomacy::Ally);
	T.AssertEquals("Sa posture : Allié", HOLDiplomacyPanelText::TheirStanceText(DpRow(Book, 0, 1, false).TheirStance), "he says Ally");
	// I go Neutral: his Ally falls to Neutral with mine; he keeps showing what he has become.
	HOLDiplomacy::SetStance(Book, 0, 1, HOLDiplomacy::Neutral);
	T.AssertEquals(DpRow(Book, 0, 1, false).TheirStance, HOLDiplomacy::Neutral, "I went Neutral: his stance toward me dropped to Neutral");
	// I go Enemy: his stance drops to Enemy, and it is his line for me as well.
	HOLDiplomacy::SetStance(Book, 0, 1, HOLDiplomacy::Enemy);
	T.AssertEquals(DpRow(Book, 0, 1, false).TheirStance, HOLDiplomacy::Enemy, "I went Enemy: he is Enemy to me too (automatic switch)");
	T.AssertEquals(DpRow(Book, 1, 0, false).TheirStance, HOLDiplomacy::Enemy, "and from his side, I am Enemy");
	// Never shown: on my own line, toward an AI (always Enemy, D367), a defeated player, nor in the locked standard modes.
	T.AssertFalse(DpRow(Book, 0, 0, false).bShowTheirStance, "not on my own line");
	T.AssertFalse(DpRow(Book, 0, 1, false, true).bShowTheirStance, "not for a defeated player");
	TArray<bool> OneAI;
	OneAI.Add(false);
	OneAI.Add(true);
	FHOLDiplomacyBook WithAI = HOLDiplomacy::NewBook(2, OneAI);
	T.AssertFalse(DpRow(WithAI, 0, 1, false).bShowTheirStance, "not for an AI");
	TArray<int> Teams;
	TArray<bool> NoAI;
	for (int Slot = 0; Slot < 2; Slot++)
	{
		Teams.Add(1);
		NoAI.Add(false);
	}
	FHOLDiplomacyBook TeamBook = HOLDiplomacy::NewTeamBook(Teams, NoAI);
	T.AssertFalse(DpRow(TeamBook, 0, 1, true).bShowTheirStance, "not in the standard modes (locked teams)");
}
