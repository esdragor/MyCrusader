// H-4: the players' table (top right under the menu's button): a row per occupied slot, its name in its colour, its lives, its forum's phase.
// Rules (namespace HOLPlayerTable, pure, tested in HOLPlayerTable_Test.as) and view (UHOLPlayerTablePanel, built once in the script screen's
// OnInitialized, refreshed in its Refresh), ported from UHOLGameScreen::RefreshPlayersTable and the table block of BuildLayout (HOLGameScreen.cpp).
//
// DOES NOT COMPILE ALONE: it calls a C++ binding that does not exist yet (HOLPlayersTableRead::, FHOLPlayersTableRowRead, see HALDIR-1119a.md,
// section (a)) and UHOLScriptScreen::GetPlayersPanel(). It compiles with the NEXT lot (the C++ binding, the hook, the removal of the C++ block).
// The ROWS (who is shown, in which order, grouped by team, the lives and phase texts, the relation, the pastilles) stay computed by the C++
// HOLForumRules::GetPlayersTable (shared with its C++ tests and the HUD report): one source. The script owns how a row is DRAWN and the journal.

// One row as the script's rules see it (the adapter FromRead fills it from the C++ row).
struct FHOLPlayerRow
{
	int Slot = -1;
	int ColorIndex = 0;
	FString Name = "";
	FString Lives = "";
	FString State = "";
	// The slot's colour (HOLPlayerColors::GetColor(ColorIndex) as a linear colour, read by the binding).
	FLinearColor Color = FLinearColor(1.0, 1.0, 1.0, 1.0);
	bool bGreyed = false;
	bool bStruck = false;
	// EHOLPlayerRelation as its number: None 0, Self 1, Ally 2, Enemy 3, Neutral 4 (HOLRelationIconsRead::GetRelationX()).
	int Relation = 0;
	bool bAway = false;
	bool bAIHeld = false;
	bool bNewGroup = false;
	// The C++ row's report text ("slot 1 you colour 1 lives 1 ruin"): the table's change signature and what hol.HUD.Report prints.
	FString Shown = "";
}

// What one row looks like once drawn.
struct FHOLPlayerRowView
{
	bool bVisible = false;
	// The gap above the row (a new team: 6 px).
	float TopGap = 0.0;
	// The whole row faded once defeated.
	float Opacity = 1.0;
	FString Name = "";
	FLinearColor NameColor = FLinearColor(1.0, 1.0, 1.0, 1.0);
	// A defeated player's name struck through, in the name's colour.
	bool bStrike = false;
	// RelationKind: 0 no image (hidden), 1 ally's shield, 2 enemy's swords, 3 neutral's grey dot.
	int RelationKind = 0;
	// The 16 px room before the name: on every row of a player's table, none in a spectator's.
	bool bRelationRoom = false;
	FString Lives = "";
	FString State = "";
}

// What the table remembers between two refreshes (the C++ members ShownPlayersTable, ShownPlayerNames, ShownPlayerStates).
struct FHOLPlayerTableMemory
{
	FString Shown = "";
	FString Names = "";
	FString States = "";
}

// What one refresh decides.
struct FHOLPlayerTablePlan
{
	// False: the signature did not change, nothing is redrawn (the C++ early return).
	bool bRedraw = false;
	// The panel is shown iff there is a row.
	bool bPanelVisible = false;
	bool bLogNames = false;
	FString NamesLog = "";
	bool bLogStates = false;
	FString StatesLog = "";
	FHOLPlayerTableMemory Memory;
}

namespace HOLPlayerTable
{
	// The C++ constants: a gap of 6 px above the first row of a new team, a defeated row at 45 %.
	float NewGroupGap() { return 6.0; }
	float GreyedOpacity() { return 0.45; }

	// The relation kinds (functions, not constants: a constant of a namespace is not seen from another file).
	int KindNone() { return 0; }
	int KindAlly() { return 1; }
	int KindEnemy() { return 2; }
	int KindNeutral() { return 3; }

	// E4-1: an ally's green shield, an enemy's red swords, the Diplomacy mode's Neutral a grey dot; the viewer's own row (Self) and a spectator's
	// table (None) draw nothing in the room.
	int RelationKind(int Relation)
	{
		if (Relation == HOLRelationIconsRead::GetRelationAlly())
			return KindAlly();
		if (Relation == HOLRelationIconsRead::GetRelationEnemy())
			return KindEnemy();
		if (Relation == HOLRelationIconsRead::GetRelationNeutral())
			return KindNeutral();
		return KindNone();
	}

	// The room exists on every row of a player's table, on none of a spectator's.
	bool RelationRoom(int Relation)
	{
		return Relation != HOLRelationIconsRead::GetRelationNone();
	}

	FHOLPlayerRowView RowView(FHOLPlayerRow Row)
	{
		FHOLPlayerRowView View;
		View.bVisible = true;
		View.TopGap = Row.bNewGroup ? NewGroupGap() : 0.0;
		View.Opacity = Row.bGreyed ? GreyedOpacity() : 1.0;
		View.Name = Row.Name;
		View.NameColor = Row.Color;
		View.bStrike = Row.bStruck;
		View.RelationKind = RelationKind(Row.Relation);
		View.bRelationRoom = RelationRoom(Row.Relation);
		View.Lives = Row.Lives;
		View.State = Row.State;
		return View;
	}

	// The widgets of a seat with no row: hidden.
	FHOLPlayerRowView NoRowView()
	{
		FHOLPlayerRowView View;
		View.bVisible = false;
		return View;
	}

	// The view of seat Index: its row, or hidden past the last row.
	FHOLPlayerRowView ViewAt(TArray<FHOLPlayerRow> Rows, int Index)
	{
		if (Index < 0 || Index >= Rows.Num())
			return NoRowView();
		return RowView(Rows[Index]);
	}

	// The table's change signature: the rows' reports joined by " | " (the C++ Shown).
	FString Signature(TArray<FHOLPlayerRow> Rows)
	{
		FString Result = "";
		for (int Index = 0; Index < Rows.Num(); ++Index)
			Result += (Index > 0 ? " | " : "") + Rows[Index].Shown;
		return Result;
	}

	// M6-Q12: the names shown, joined by ", ".
	FString NamesLine(TArray<FHOLPlayerRow> Rows)
	{
		FString Result = "";
		for (int Index = 0; Index < Rows.Num(); ++Index)
			Result += (Index > 0 ? ", " : "") + Rows[Index].Name;
		return Result;
	}

	// T8: the pastilles of the rows away or held by the AI, "Bob=Connexion perdue, Carol=IA" (the AI wins over the lost connection); "" when none.
	FString StatesLine(TArray<FHOLPlayerRow> Rows)
	{
		FString Result = "";
		for (int Index = 0; Index < Rows.Num(); ++Index)
		{
			if (!Rows[Index].bAway && !Rows[Index].bAIHeld)
				continue;
			Result += (Result.IsEmpty() ? "" : ", ") + Rows[Index].Name + "=" + (Rows[Index].bAIHeld ? "IA" : "Connexion perdue");
		}
		return Result;
	}

	// The journal lines of the C++ screen, word for word.
	FString NamesLogLine(FString Names)
	{
		return "Players table: " + Names;
	}

	FString StatesLogLine(FString States)
	{
		return "Players table state: " + (States.IsEmpty() ? "none" : States);
	}

	// One refresh: nothing when the signature is the one shown; otherwise a redraw, the names logged when they changed, the pastilles logged when
	// they changed (the C++ order and conditions: both logs only after a change of signature).
	FHOLPlayerTablePlan Plan(FHOLPlayerTableMemory Memory, TArray<FHOLPlayerRow> Rows)
	{
		FHOLPlayerTablePlan Result;
		Result.Memory = Memory;
		const FString Shown = Signature(Rows);
		if (Shown == Memory.Shown)
			return Result;
		Result.bRedraw = true;
		Result.bPanelVisible = Rows.Num() > 0;
		const FString Names = NamesLine(Rows);
		if (Names != Memory.Names)
		{
			Result.bLogNames = true;
			Result.NamesLog = NamesLogLine(Names);
			Result.Memory.Names = Names;
		}
		Result.Memory.Shown = Shown;
		const FString States = StatesLine(Rows);
		if (States != Memory.States)
		{
			Result.bLogStates = true;
			Result.StatesLog = StatesLogLine(States);
			Result.Memory.States = States;
		}
		return Result;
	}

	// The adapter from the C++ binding's row.
	FHOLPlayerRow FromRead(FHOLPlayersTableRowRead Read)
	{
		FHOLPlayerRow Row;
		Row.Slot = Read.Slot;
		Row.ColorIndex = Read.ColorIndex;
		Row.Name = Read.Name;
		Row.Lives = Read.Lives;
		Row.State = Read.State;
		Row.Color = Read.Color;
		Row.bGreyed = Read.bGreyed;
		Row.bStruck = Read.bStruck;
		Row.Relation = Read.Relation;
		Row.bAway = Read.bAway;
		Row.bAIHeld = Read.bAIHeld;
		Row.bNewGroup = Read.bNewGroup;
		Row.Shown = Read.Shown;
		return Row;
	}
}

// The table's widgets, the C++ screen's names and places: PlayersList in the C++ PlayersPanel (kept in C++: it sits in the RightColumn above the
// messages' thread), then per seat PlayerRow<N> (PlayerRelationBox<N> > PlayerRelation<N>, PlayerNameBox<N> > PlayerNameOverlay<N> > PlayerName<N> +
// PlayerNameStrikeBox<N> > PlayerNameStrike<N>, PlayerLivesBox<N> > PlayerLives<N>, PlayerStateBox<N> > PlayerState<N>).
class UHOLPlayerTablePanel : UObject
{
	UHOLScriptScreen Screen;
	UBorder Host;
	UVerticalBox List;
	TArray<UHorizontalBox> RowBoxes;
	TArray<UVerticalBoxSlot> RowSlots;
	TArray<UImage> Relations;
	TArray<USizeBox> RelationBoxes;
	TArray<UCommonTextBlock> NameTexts;
	TArray<UImage> Strikes;
	TArray<UCommonTextBlock> LivesTexts;
	TArray<UCommonTextBlock> StateTexts;
	UTexture2D AllyIcon;
	UTexture2D EnemyIcon;

	FHOLPlayerTableMemory Memory;
	// What hol.HUD.Report prints for the table, and the last journal lines (a script test cannot read the journal).
	FString ShownText = "";
	FString LastNamesLine = "";
	FString LastStatesLine = "";

	// Host: the C++ screen's PlayersPanel (Screen.GetPlayersPanel()), already in its column with its brush, its 8/4 padding, collapsed.
	void Build(UHOLScriptScreen Screen_, UBorder PlayersPanel)
	{
		Screen = Screen_;
		Host = PlayersPanel;
		List = Cast<UVerticalBox>(Screen.ConstructWidget(UVerticalBox, n"PlayersList"));
		Host.SetContent(List);
		AllyIcon = Cast<UTexture2D>(LoadObject(nullptr, HOLRelationIconsRead::GetAllyPath()));
		EnemyIcon = Cast<UTexture2D>(LoadObject(nullptr, HOLRelationIconsRead::GetEnemyPath()));
		const int Seats = HOLPlayersTableRead::GetMaxSlots();
		for (int Seat = 0; Seat < Seats; ++Seat)
		{
			UHorizontalBox Row = Cast<UHorizontalBox>(Screen.ConstructWidget(UHorizontalBox, FName(f"PlayerRow{Seat}")));
			Row.SetVisibility(ESlateVisibility::Collapsed);
			RowSlots.Add(List.AddChildToVerticalBox(Row));
			RowBoxes.Add(Row);

			// E4-1: the relation's icon before the name, in a fixed 16 px room kept on the viewer's own row so that the names line up.
			UImage Relation = Cast<UImage>(Screen.ConstructWidget(UImage, FName(f"PlayerRelation{Seat}")));
			Relation.SetDesiredSizeOverride(FVector2D(16.0, 16.0));
			Relation.SetVisibility(ESlateVisibility::Hidden);
			USizeBox RelationBox = Cast<USizeBox>(Screen.ConstructWidget(USizeBox, FName(f"PlayerRelationBox{Seat}")));
			RelationBox.SetWidthOverride(16.0);
			RelationBox.SetHeightOverride(16.0);
			RelationBox.SetContent(Relation);
			RelationBox.SetVisibility(ESlateVisibility::Collapsed);
			AddToRow(Row, RelationBox, 4.0);
			Relations.Add(Relation);
			RelationBoxes.Add(RelationBox);

			// E4-2: the name in an overlay with a thin bar across it, its width, shown once defeated (a text has no strike).
			UCommonTextBlock Name = Screen.MakeText(FName(f"PlayerName{Seat}"), UHOLTextStyleHUDPlayerName);
			UOverlay NameOverlay = Cast<UOverlay>(Screen.ConstructWidget(UOverlay, FName(f"PlayerNameOverlay{Seat}")));
			NameOverlay.AddChildToOverlay(Name);
			UImage Strike = Cast<UImage>(Screen.ConstructWidget(UImage, FName(f"PlayerNameStrike{Seat}")));
			Strike.SetVisibility(ESlateVisibility::Collapsed);
			// A size box holds the bar to 2 px (the image alone took the whole line's height).
			USizeBox StrikeBox = Cast<USizeBox>(Screen.ConstructWidget(USizeBox, FName(f"PlayerNameStrikeBox{Seat}")));
			StrikeBox.SetHeightOverride(2.0);
			StrikeBox.SetContent(Strike);
			UOverlaySlot StrikeSlot = NameOverlay.AddChildToOverlay(StrikeBox);
			StrikeSlot.SetHorizontalAlignment(EHorizontalAlignment::HAlign_Fill);
			StrikeSlot.SetVerticalAlignment(EVerticalAlignment::VAlign_Center);
			Strikes.Add(Strike);
			USizeBox NameBox = Cast<USizeBox>(Screen.ConstructWidget(USizeBox, FName(f"PlayerNameBox{Seat}")));
			NameBox.SetMinDesiredWidth(64.0);
			NameBox.SetContent(NameOverlay);
			// The overlay as wide as the name, not the box (the bar the name's width).
			USizeBoxSlot NameSlot = Cast<USizeBoxSlot>(NameOverlay.Slot);
			if (NameSlot != nullptr)
				NameSlot.SetHorizontalAlignment(EHorizontalAlignment::HAlign_Left);
			AddToRow(Row, NameBox, 10.0);
			NameTexts.Add(Name);

			LivesTexts.Add(AddCell(Screen, Row, "Lives", UHOLTextStyleHUDLabel, Seat, 44.0, 10.0));
			StateTexts.Add(AddCell(Screen, Row, "State", UHOLTextStyleHUDLabel, Seat, 0.0, 0.0));
		}
	}

	// The C++ lambda AddCell: a text in a size box of a minimum width, "Player<Name><Seat>" and "Player<Name>Box<Seat>".
	UCommonTextBlock AddCell(UHOLScriptScreen Screen, UHorizontalBox Row, FString Cell, TSubclassOf<UCommonTextStyle> TextStyle, int Seat, float Width, float RightPadding)
	{
		UCommonTextBlock Text = Screen.MakeText(FName(f"Player{Cell}{Seat}"), TextStyle);
		USizeBox Box = Cast<USizeBox>(Screen.ConstructWidget(USizeBox, FName(f"Player{Cell}Box{Seat}")));
		Box.SetMinDesiredWidth(Width);
		Box.SetContent(Text);
		AddToRow(Row, Box, RightPadding);
		return Text;
	}

	// The C++ AddToRow (not read: centred vertically, the gap on the right, like the script screen's DipAdd).
	void AddToRow(UHorizontalBox Row, UWidget Widget, float RightPadding)
	{
		UHorizontalBoxSlot Added = Row.AddChildToHorizontalBox(Widget);
		Added.SetVerticalAlignment(EVerticalAlignment::VAlign_Center);
		Added.SetPadding(FMargin(0.0, 0.0, RightPadding, 0.0));
	}

	void SetShown(UWidget Widget, bool bShown, ESlateVisibility ShownVisibility)
	{
		Widget.SetVisibility(bShown ? ShownVisibility : ESlateVisibility::Collapsed);
	}

	// Called every frame by the script screen's Refresh: one read of the rows, a redraw only when the signature changes.
	void Refresh()
	{
		TArray<FHOLPlayersTableRowRead> Read;
		// The slot the C++ screen read the table for (its bound player state: a spectator who follows a player included).
		HOLPlayersTableRead::GetRows(Screen.GetShownSlot(), Read);
		TArray<FHOLPlayerRow> Rows;
		for (int Index = 0; Index < Read.Num(); ++Index)
			Rows.Add(HOLPlayerTable::FromRead(Read[Index]));
		const FHOLPlayerTablePlan Plan = HOLPlayerTable::Plan(Memory, Rows);
		if (!Plan.bRedraw)
			return;
		Memory = Plan.Memory;
		ShownText = HOLPlayerTable::Signature(Rows);
		if (Plan.bLogNames)
		{
			LastNamesLine = Plan.NamesLog;
			UHOLGameMenuScreen::MenuLog(LastNamesLine);
		}
		if (Plan.bLogStates)
		{
			LastStatesLine = Plan.StatesLog;
			UHOLGameMenuScreen::MenuLog(LastStatesLine);
		}
		SetShown(Host, Plan.bPanelVisible, ESlateVisibility::HitTestInvisible);
		for (int Index = 0; Index < RowBoxes.Num(); ++Index)
			DrawRow(Index, HOLPlayerTable::ViewAt(Rows, Index));
	}

	void DrawRow(int Index, FHOLPlayerRowView View)
	{
		SetShown(RowBoxes[Index], View.bVisible, ESlateVisibility::SelfHitTestInvisible);
		if (!View.bVisible)
			return;
		RowSlots[Index].SetPadding(FMargin(0.0, View.TopGap, 0.0, 0.0));
		RowBoxes[Index].SetRenderOpacity(View.Opacity);
		NameTexts[Index].SetText(FText::FromString(View.Name));
		NameTexts[Index].SetColorAndOpacity(View.NameColor);
		Strikes[Index].SetColorAndOpacity(View.NameColor);
		SetShown(Strikes[Index], View.bStrike, ESlateVisibility::HitTestInvisible);
		if (View.RelationKind == HOLPlayerTable::KindAlly() || View.RelationKind == HOLPlayerTable::KindEnemy())
		{
			Relations[Index].SetBrushFromTexture(View.RelationKind == HOLPlayerTable::KindAlly() ? AllyIcon : EnemyIcon, false);
			Relations[Index].SetDesiredSizeOverride(FVector2D(16.0, 16.0));
		}
		if (View.RelationKind == HOLPlayerTable::KindNeutral())
		{
			// The Diplomacy mode's Neutral: a small grey dot in the icon's room (no texture for it): a rounded 10 px box of radius 5.
			FSlateBrush Dot;
			Dot.DrawAs = ESlateBrushDrawType::RoundedBox;
			Dot.TintColor = FSlateColor(FLinearColor(0.62, 0.62, 0.62, 1.0));
			Dot.ImageSize = FVector2f(10.0, 10.0);
			Dot.OutlineSettings.RoundingType = ESlateBrushRoundingType::FixedRadius;
			Dot.OutlineSettings.CornerRadii = FVector4(5.0, 5.0, 5.0, 5.0);
			Relations[Index].SetBrush(Dot);
			Relations[Index].SetDesiredSizeOverride(FVector2D(16.0, 16.0));
		}
		const bool bDrawn = View.RelationKind != HOLPlayerTable::KindNone();
		const ESlateVisibility RelationVisibility = bDrawn ? ESlateVisibility::HitTestInvisible : ESlateVisibility::Hidden;
		if (Relations[Index].GetVisibility() != RelationVisibility)
			Relations[Index].SetVisibility(RelationVisibility);
		SetShown(RelationBoxes[Index], View.bRelationRoom, ESlateVisibility::HitTestInvisible);
		LivesTexts[Index].SetText(FText::FromString(View.Lives));
		StateTexts[Index].SetText(FText::FromString(View.State));
	}
}
