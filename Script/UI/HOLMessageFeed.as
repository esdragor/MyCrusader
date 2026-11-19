// HUD H-5b, the thread of messages (V4.2, V4.3; the C++ screen's MessagesList / MessageLine<i> / MessageRow<i>); hooked by HOLGameScreenScript.as (MessageFeed), uncompiled.
// C++ bindings, members of UHOLScriptScreen (same pattern as HOLAlertBanner.as), written by ELROND-1119m:
//   UVerticalBox GetMessagesList() const;   // the C++ RightColumn's MessagesList, under the players' table (a host, as PlayersPanel): the script fills it
//   int GetMessageState(int& OutRevision, TArray<bool>& OutAlerts, TArray<int>& OutSlots, TArray<float32>& OutOpacities) const;
//                                           // the C++ thread (UHOLGameScreen::MessageThread): the line count; per line: carries an alert, slot touched, opacity now
//   bool GetMessageLine(int Index, TArray<FHOLForumPieceValue>& OutPieces, TArray<FLinearColor>& OutColors) const;  // the pieces of a line (SplitNames), a colour per piece
//   void BindMessageClick(UBorder Panel);   // the C++ receives the left click on a line and calls this screen's ClickWidget(MessageFeed)
//   void FocusCameraOnForum(int Slot);      // (H-5a)
//   MessageFrameTick(float32): once a frame while the C++ thread holds a line (and once more when it empties): the screen's override calls Refresh.
//
// The STATE (FHOLForumThread: Add, Expire after 8 s, the 5 lines, GetOpacity's last half second, Describe for "HUD messages: ...") and the SOURCE of the
// messages (the Handle*Message of the C++ screen) STAY C++ (one source, D836, D839); the script is the VIEW, the click and the journal line:
//   "HUD: alert of slot <n> clicked (camera on its forum)"  (what FocusAlertSlot wrote; UHOLGameMenuScreen::MenuLog writes "LogHOLUI: Display: <line>")

namespace HOLMessageFeed
{
	// FHOLForumThread::MaxLines: the panels built (a line's own panel and row each).
	int MaxLines() { return 5; }

	// BuildLayout: a line's panel is as dark as the table's (HUDPanel at 0.8), padding (8, 2), 2 px between two lines, right aligned.
	float LineOpacity() { return 0.8; }
	float PaddingX() { return 8.0; }
	float PaddingY() { return 2.0; }
	float TopPadding(int Index) { return Index > 0 ? 2.0 : 0.0; }

	// The list is up while the thread holds a line (RebuildMessages: SetShown(MessagesList, !Lines.IsEmpty(), SelfHitTestInvisible)).
	bool ListShown(int Count) { return Count > 0; }

	// Line Index is up when the thread holds it.
	bool LineShown(int Index, int Count) { return Index >= 0 && Index < Count; }

	// Only a line carrying an alert takes the clicks (Visible); the others let them through (HitTestInvisible).
	bool LineTakesClick(bool bAlert) { return bAlert; }

	// The lines are redrawn when the C++ thread says it changed (UHOLGameScreen::MessageRevision, bumped by each RebuildMessages), not each frame.
	bool NeedsRedraw(int Revision, int ShownRevision) { return Revision != ShownRevision; }

	// HandleMessageLineMouseDown: the FIRST line, among the Count shown, that is under the mouse AND carries an alert; -1: none (the click goes through).
	int HoveredAlertLine(TArray<bool> Hovered, TArray<bool> Alerts, int Count)
	{
		for (int Index = 0; Index < Count && Index < Hovered.Num() && Index < Alerts.Num(); Index++)
		{
			if (Hovered[Index] && Alerts[Index])
				return Index;
		}
		return -1;
	}

	// FocusAlertSlot's journal line (the banner's click writes the same one, HOLAlertBanner::ClickLogLine).
	FString ClickLogLine(int Slot)
	{
		return f"HUD: alert of slot {Slot} clicked (camera on its forum)";
	}

	// A piece is a player's name (its slot is not INDEX_NONE = -1): in the names' style and that player's colour; a word: the label's style.
	bool PieceIsName(int Slot)
	{
		return Slot != -1;
	}

	// The report's line "HUD messages: [...]" stays the C++ screen's DescribeMessages(): the C++ owns the thread it prints.
}

class UHOLMessageFeedPanel : UObject
{
	UHOLScriptScreen Screen;
	UVerticalBox List;
	TArray<UBorder> Lines;
	TArray<UHorizontalBox> Rows;

	// What is drawn, for the report and the tests (a script test cannot read a widget).
	int ShownRevision = -1;
	int ShownCount = 0;
	TArray<bool> ShownAlerts;
	TArray<int> ShownSlots;
	FString LastLine = "";

	void Build(UHOLScriptScreen InScreen)
	{
		Screen = InScreen;
		List = Screen.GetMessagesList();
		if (List == nullptr)
			return;
		for (int Index = 0; Index < HOLMessageFeed::MaxLines(); Index++)
		{
			UBorder Line = Screen.MakePanel(FName(f"MessageLine{Index}"));
			Line.SetBrushColor(FLinearColor(1.0, 1.0, 1.0, HOLMessageFeed::LineOpacity()));
			Line.SetPadding(FMargin(HOLMessageFeed::PaddingX(), HOLMessageFeed::PaddingY()));
			Line.SetVisibility(ESlateVisibility::Collapsed);
			Screen.BindMessageClick(Line);
			UHorizontalBox Row = Cast<UHorizontalBox>(Screen.ConstructWidget(UHorizontalBox, FName(f"MessageRow{Index}")));
			Line.SetContent(Row);
			UVerticalBoxSlot LineSlot = List.AddChildToVerticalBox(Line);
			LineSlot.SetHorizontalAlignment(EHorizontalAlignment::HAlign_Right);
			LineSlot.SetPadding(FMargin(0.0, HOLMessageFeed::TopPadding(Index), 0.0, 0.0));
			Lines.Add(Line);
			Rows.Add(Row);
		}
	}

	// Each frame while the thread holds a line (the last half second of a line is a fade): the C++ state read once, the lines drawn from it.
	void Refresh()
	{
		if (List == nullptr)
			return;
		int Revision = 0;
		TArray<bool> Alerts;
		TArray<int> Slots;
		TArray<float32> Opacities;
		const int Count = Screen.GetMessageState(Revision, Alerts, Slots, Opacities);
		List.SetVisibility(HOLMessageFeed::ListShown(Count) ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
		if (HOLMessageFeed::NeedsRedraw(Revision, ShownRevision))
		{
			for (int Index = 0; Index < Lines.Num(); Index++)
			{
				if (HOLMessageFeed::LineShown(Index, Count) && Index < Alerts.Num())
				{
					FillRow(Index);
					Lines[Index].SetVisibility(HOLMessageFeed::LineTakesClick(Alerts[Index]) ? ESlateVisibility::Visible : ESlateVisibility::HitTestInvisible);
				}
				else
				{
					Rows[Index].ClearChildren();
					Lines[Index].SetVisibility(ESlateVisibility::Collapsed);
				}
			}
			ShownRevision = Revision;
			ShownCount = Count;
			ShownAlerts = Alerts;
			ShownSlots = Slots;
		}
		for (int Index = 0; Index < Lines.Num() && Index < Count && Index < Opacities.Num(); Index++)
			Lines[Index].SetRenderOpacity(Opacities[Index]);
	}

	// FillMessageRow: the plain words in the label's style, each name in the names' style and its player's colour (set after it joins the row).
	void FillRow(int Index)
	{
		TArray<FHOLForumPieceValue> Pieces;
		TArray<FLinearColor> Colors;
		Screen.GetMessageLine(Index, Pieces, Colors);
		UHorizontalBox Row = Rows[Index];
		Row.ClearChildren();
		for (int PieceIndex = 0; PieceIndex < Pieces.Num(); PieceIndex++)
		{
			const bool bName = HOLMessageFeed::PieceIsName(Pieces[PieceIndex].Slot);
			UCommonTextBlock Piece = Cast<UCommonTextBlock>(Screen.ConstructWidget(UCommonTextBlock, NAME_None));
			if (bName)
				Piece.SetStyle(UHOLTextStyleHUDPlayerName);
			else
				Piece.SetStyle(UHOLTextStyleHUDLabel);
			Piece.SetText(FText::FromString(Pieces[PieceIndex].Text));
			UHorizontalBoxSlot PieceSlot = Row.AddChildToHorizontalBox(Piece);
			PieceSlot.SetVerticalAlignment(EVerticalAlignment::VAlign_Center);
			if (bName && PieceIndex < Colors.Num())
				Piece.SetColorAndOpacity(Colors[PieceIndex]);
		}
	}

	// The click, mouse (UHOLScriptScreen::HandleMessageMouseDown, left button only) or hol.HUD.Click MessageFeed: true when a line carrying an alert is
	// under the mouse (the camera centres on its forum); false lets the click through.
	bool ClickName(FName Name)
	{
		if (Name != n"MessageFeed")
			return false;
		TArray<bool> Hovered;
		for (int Index = 0; Index < Lines.Num(); Index++)
			Hovered.Add(Lines[Index].IsHovered());
		const int Hit = HOLMessageFeed::HoveredAlertLine(Hovered, ShownAlerts, ShownCount);
		if (Hit < 0 || Hit >= ShownSlots.Num())
			return false;
		LastLine = HOLMessageFeed::ClickLogLine(ShownSlots[Hit]);
		UHOLGameMenuScreen::MenuLog(LastLine);
		Screen.FocusCameraOnForum(ShownSlots[Hit]);
		return true;
	}
}
