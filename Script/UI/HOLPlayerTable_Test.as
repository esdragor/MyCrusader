// H-4: the players' table's rules (HOLPlayerTable.as), pure, in literals copied from the C++ screen (HOLGameScreen.cpp RefreshPlayersTable) and
// the HUD report's rows ("slot 0 you colour 1 lives 1 ruin", quick_check.ps1:3235). Compiles with the NEXT lot (HOLPlayerTable.as calls the C++ binding).

FHOLPlayerRow PT_Row(int Slot, FString Name, FString Report)
{
	FHOLPlayerRow Row;
	Row.Slot = Slot;
	Row.Name = Name;
	Row.Shown = Report;
	return Row;
}

void Test_PlayersTableRowFadesStrikesAndGapsLikeCpp(FUnitTest& T)
{
	FHOLPlayerRow Standing = PT_Row(0, "Vous", "slot 0 you colour 1 lives 1 standing");
	Standing.Color = FLinearColor(0.1, 0.3, 0.9, 1.0);
	Standing.Lives = "1 vie";
	const FHOLPlayerRowView Up = HOLPlayerTable::RowView(Standing);
	T.AssertTrue(Up.bVisible, "a row is drawn");
	T.AssertTrue(Up.Opacity == 1.0, "a standing row is not faded");
	T.AssertFalse(Up.bStrike, "a standing player's name is not struck");
	T.AssertTrue(Up.TopGap == 0.0, "no gap above a row of the same team");
	T.AssertTrue(Up.NameColor.B == 0.9 && Up.NameColor.R == 0.1, "the name takes the slot's colour");
	T.AssertEquals(Up.Lives, "1 vie", "lives text");

	FHOLPlayerRow Down = PT_Row(3, "Joueur 3", "slot 3 enemy colour 3 lives 0 defeated greyed struck");
	Down.bGreyed = true;
	Down.bStruck = true;
	Down.bNewGroup = true;
	Down.State = "Vaincu";
	const FHOLPlayerRowView Gone = HOLPlayerTable::RowView(Down);
	T.AssertTrue(Gone.Opacity == 0.45, "a defeated row is faded to 45 %");
	T.AssertTrue(Gone.bStrike, "a defeated player's name is struck through");
	T.AssertTrue(Gone.TopGap == 6.0, "the first row of a new team has a gap of 6 px above");
	T.AssertEquals(Gone.State, "Vaincu", "state text");

	TArray<FHOLPlayerRow> Rows;
	Rows.Add(Standing);
	const FHOLPlayerRowView Past = HOLPlayerTable::ViewAt(Rows, 1);
	T.AssertFalse(Past.bVisible, "a seat past the last row is hidden");
}

void Test_PlayersTableRelationIconAndRoom(FUnitTest& T)
{
	// EHOLPlayerRelation: None 0, Self 1, Ally 2, Enemy 3, Neutral 4 (HOLForumRules.h).
	T.AssertTrue(HOLPlayerTable::RelationKind(2) == HOLPlayerTable::KindAlly(), "ally: the green shield");
	T.AssertTrue(HOLPlayerTable::RelationKind(3) == HOLPlayerTable::KindEnemy(), "enemy: the red swords");
	T.AssertTrue(HOLPlayerTable::RelationKind(4) == HOLPlayerTable::KindNeutral(), "neutral: the grey dot");
	T.AssertTrue(HOLPlayerTable::RelationKind(1) == HOLPlayerTable::KindNone(), "self: nothing in the room");
	T.AssertTrue(HOLPlayerTable::RelationKind(0) == HOLPlayerTable::KindNone(), "spectator: nothing");
	T.AssertTrue(HOLPlayerTable::RelationRoom(1), "the viewer's own row keeps the room so that the names line up");
	T.AssertTrue(HOLPlayerTable::RelationRoom(2), "an ally's row has the room");
	T.AssertFalse(HOLPlayerTable::RelationRoom(0), "a spectator's table has no room at all");
}

void Test_PlayersTableSignatureJoinsReportsWithBars(FUnitTest& T)
{
	TArray<FHOLPlayerRow> Rows;
	Rows.Add(PT_Row(0, "Vous", "slot 0 you colour 1 lives 1 ruin"));
	Rows.Add(PT_Row(3, "Joueur 3", "slot 3 enemy colour 3 lives 0 defeated greyed struck"));
	const FString Signature = HOLPlayerTable::Signature(Rows);
	T.AssertEquals(Signature, "slot 0 you colour 1 lives 1 ruin | slot 3 enemy colour 3 lives 0 defeated greyed struck", "rows joined by ' | '");
	TArray<FHOLPlayerRow> None;
	const FString Empty = HOLPlayerTable::Signature(None);
	T.AssertEquals(Empty, "", "no row: empty signature");
}

void Test_PlayersTableNamesAndPastillesLines(FUnitTest& T)
{
	TArray<FHOLPlayerRow> Rows;
	Rows.Add(PT_Row(0, "Alice", "a"));
	Rows.Add(PT_Row(1, "Bob", "b"));
	Rows.Add(PT_Row(2, "Carol", "c"));
	const FString Names = HOLPlayerTable::NamesLine(Rows);
	T.AssertEquals(Names, "Alice, Bob, Carol", "names joined by ', '");
	const FString NoPastille = HOLPlayerTable::StatesLine(Rows);
	T.AssertEquals(NoPastille, "", "nobody away or held: empty");
	Rows[1].bAway = true;
	const FString Lost = HOLPlayerTable::StatesLine(Rows);
	T.AssertEquals(Lost, "Bob=Connexion perdue", "lost connection: the pastille 'Connexion perdue' (lobby_match.ps1:524)");
	Rows[1].bAIHeld = true;
	Rows[2].bAway = true;
	const FString Both = HOLPlayerTable::StatesLine(Rows);
	T.AssertEquals(Both, "Bob=IA, Carol=Connexion perdue", "the AI wins over the lost connection on one row; rows joined by ', '");
	const FString NamesLog = HOLPlayerTable::NamesLogLine("Vous, IA Normal 2");
	T.AssertEquals(NamesLog, "Players table: Vous, IA Normal 2", "the journal line lobby_match.ps1:242 reads");
	const FString NoneLog = HOLPlayerTable::StatesLogLine("");
	T.AssertEquals(NoneLog, "Players table state: none", "no pastille: 'none' (lobby_match.ps1:526)");
}

void Test_PlayersTablePlanRedrawsAndLogsOnlyOnChange(FUnitTest& T)
{
	FHOLPlayerTableMemory Memory;
	TArray<FHOLPlayerRow> None;
	FHOLPlayerTablePlan Empty = HOLPlayerTable::Plan(Memory, None);
	T.AssertFalse(Empty.bRedraw, "no row at the start: nothing to draw, nothing logged");
	T.AssertFalse(Empty.bLogNames, "no names log");

	TArray<FHOLPlayerRow> Rows;
	Rows.Add(PT_Row(0, "Alice", "slot 0 you colour 1 lives 2 standing"));
	Rows.Add(PT_Row(1, "Bob", "slot 1 ally colour 3 lives 2 standing"));
	FHOLPlayerTablePlan First = HOLPlayerTable::Plan(Memory, Rows);
	T.AssertTrue(First.bRedraw && First.bPanelVisible, "two rows: drawn, the panel shown");
	T.AssertTrue(First.bLogNames, "the names changed: logged");
	T.AssertEquals(First.NamesLog, "Players table: Alice, Bob", "names line");
	T.AssertFalse(First.bLogStates, "no pastille yet, none before: no state line");
	Memory = First.Memory;

	FHOLPlayerTablePlan Again = HOLPlayerTable::Plan(Memory, Rows);
	T.AssertFalse(Again.bRedraw, "same signature: not redrawn");
	T.AssertFalse(Again.bLogNames, "same signature: nothing logged");

	Rows[1].bAway = true;
	Rows[1].Shown = "slot 1 ally colour 3 lives 2 standing away";
	FHOLPlayerTablePlan Away = HOLPlayerTable::Plan(Memory, Rows);
	T.AssertTrue(Away.bRedraw, "a changed report: redrawn");
	T.AssertFalse(Away.bLogNames, "same names: not logged again");
	T.AssertTrue(Away.bLogStates, "a pastille appeared: logged");
	T.AssertEquals(Away.StatesLog, "Players table state: Bob=Connexion perdue", "state line");
	Memory = Away.Memory;

	Rows[1].bAway = false;
	Rows[1].Shown = "slot 1 ally colour 3 lives 2 standing";
	FHOLPlayerTablePlan Back = HOLPlayerTable::Plan(Memory, Rows);
	T.AssertTrue(Back.bLogStates, "the pastille left: logged");
	T.AssertEquals(Back.StatesLog, "Players table state: none", "'none' once nobody is away (lobby_match.ps1:526)");
	Memory = Back.Memory;

	FHOLPlayerTablePlan Left = HOLPlayerTable::Plan(Memory, None);
	T.AssertTrue(Left.bRedraw, "every row gone: redrawn");
	T.AssertFalse(Left.bPanelVisible, "the panel hides without a row");
	T.AssertEquals(Left.NamesLog, "Players table: ", "the names line with an empty list, as the C++");
}

// GALADRIEL-1121sf: the language-neutral report (Shown) does not change on a live language switch; the names / lives / states the rows DISPLAY do
// (« 2 vies » -> « 2 lives »). The table must redraw on the displayed text, not only on the report (same trap as the idle label and the income tips).
void Test_PlayersTableRedrawsWhenOnlyTheDisplayedTextChanges(FUnitTest& T)
{
	TArray<FHOLPlayerRow> French;
	French.Add(PT_Row(0, "Vous", "slot 0 you colour 1 lives 2 standing"));
	French.Add(PT_Row(1, "IA Normal 2", "slot 1 enemy colour 3 lives 2 standing"));
	French[0].Lives = "2 vies";
	French[1].Lives = "2 vies";

	TArray<FHOLPlayerRow> English;
	English.Add(PT_Row(0, "You", "slot 0 you colour 1 lives 2 standing"));
	English.Add(PT_Row(1, "AI Normal 2", "slot 1 enemy colour 3 lives 2 standing"));
	English[0].Lives = "2 lives";
	English[1].Lives = "2 lives";

	FHOLPlayerTableMemory Memory;
	FHOLPlayerTablePlan First = HOLPlayerTable::Plan(Memory, French);
	Memory = First.Memory;
	FHOLPlayerTablePlan Same = HOLPlayerTable::Plan(Memory, French);
	T.AssertFalse(Same.bRedraw, "same report, same texts: not redrawn");

	FHOLPlayerTablePlan Switched = HOLPlayerTable::Plan(Memory, English);
	T.AssertTrue(Switched.bRedraw, "same report, the displayed names / lives changed with the language: redrawn");
	Memory = Switched.Memory;
	FHOLPlayerTablePlan Settled = HOLPlayerTable::Plan(Memory, English);
	T.AssertFalse(Settled.bRedraw, "after the redraw in English: stable again");
}