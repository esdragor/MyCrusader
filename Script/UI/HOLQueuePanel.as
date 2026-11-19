// NOT COMPILED YET (SAURON-1119d, H-7 of the HUD plan): this file calls bindings that do not exist before the next C++ lot, written here as they will be:
//   UHOLScriptScreen::GetQueuePanel() -> UBorder                  (the C++ screen's QueuePanel, kept in its CommandColumn: name and place do not move),
//   UHOLScriptScreen::GetQueueNow(TArray<FName>& Types, TArray<bool>& Research, TArray<FString>& TechNames, TArray<FString>& UnitNames, bool& bHeadStarted,
//                                 float32& HeadProgress, bool& bBlockedAtFull, bool& bPopulationShort) -> bool   (false: no own building shown),
//   UHOLScriptScreen::SetQueueReport(const FString& Line)         (the line Describe() prints after "queue "),
//   UHOLScriptScreen::GetAccentColor() -> FLinearColor            (HOLUIStyle::Accent),
//   UHOLScriptScreen::CancelProduction(int) -> bool, SetQueueLive(bool), event QueueFrameTick() (HUD H-7 binding, SAURON-1119e),
// and the clicks' AHOLPlayerController::RequestCancelProduction(int) is called as the duel's RequestDuelAnswer is (it must be exposed). The recipe is in
// Docs\Rapports\SAURON-1119d.md. Until the lot exists the whole script module would fail to compile: do NOT build with this file present and without it.
//
// The production queue panel of the game HUD (RefreshQueue + UpdateHead + HandleQueueItemClicked + the QueuePanel part of the C++ screen's layout), in script:
// up to 5 items (a button each, its label the unit's or the research's name), the head's progress bar, the note "Population pleine" that takes the bar's
// place when the head waits for room, the hint "Clic : annuler". A click on an item cancels it (the C++ production keeps the queue and the cancel, D282).
// The rules are pure (namespace HOLQueuePanel); UHOLProductionQueuePanel builds the same widgets as the C++ screen (same names: QueueColumn, QueueRow,
// QueueItem0..4, QueueStatus, QueueStatusLine, QueueProgressBox, QueueProgress, QueueNote, QueueHint; same places, same styles, same texts) in the C++
// screen's QueuePanel, and refreshes them from GetQueueNow. The journal line is the C++ one, word for word, written by MenuLog (level "Display").

namespace HOLQueuePanelText
{
	// Same namespace and keys as the C++ LOCTEXT of UHOLGameScreen ("HOLGameScreen,<key>"): the translations stay.
	FText PopulationFull() { return NSLOCTEXT("HOLGameScreen", "PopulationFull", "Population pleine"); }
	FText CancelHint() { return NSLOCTEXT("HOLGameScreen", "CancelHint", "Clic : annuler"); }
}

namespace HOLQueuePanel
{
	// The head's states (EHeadState of the C++ screen, in its order). Functions, not constants: not visible from another file otherwise.
	int HeadNone() { return 0; }
	int HeadStarting() { return 1; }
	int HeadInProgress() { return 2; }
	int HeadPopulationFull() { return 3; }

	// UHOLGameScreen::NumQueueItems, QueueItemWidth, QueueItemHeight (HOLGameScreen.h:150, HOLGameScreen.cpp:249-250).
	int ItemCount() { return 5; }
	float ItemWidth() { return 56.0; }
	float ItemHeight() { return 36.0; }

	// QueueItemName(Index): the item buttons' widget names (hol.HUD.Click QueueItem2 finds them).
	FString ItemName(int Index)
	{
		return f"QueueItem{Index}";
	}

	// A research waits in the queue as a unit does (T6): its name, not the unit's.
	FString ItemLabel(bool bResearch, FString TechnologyName, FString UnitName)
	{
		return bResearch ? TechnologyName : UnitName;
	}

	// The panel is there while the building's queue holds something (bQueueShown).
	bool PanelShown(int QueueCount)
	{
		return QueueCount > 0;
	}

	// How many buttons are shown: the queue's length, at most the five the panel has.
	int ItemsShown(int QueueCount)
	{
		return Math::Min(QueueCount, ItemCount());
	}

	bool ItemShown(int Index, int QueueCount)
	{
		return Index >= 0 && Index < Math::Min(QueueCount, ItemCount());
	}

	// What makes the buttons' labels change: the queue's types in order (a label is set again only when this changes).
	FString Signature(TArray<FString> Types)
	{
		FString Result = f"{Types.Num()}";
		for (int Index = 0; Index < Types.Num(); ++Index)
			Result += "|" + Types[Index];
		return Result;
	}

	bool SignatureChanged(FString Shown, FString Now)
	{
		return Shown != Now;
	}

	// UpdateHead: none for an empty queue; a started head is in progress, held at 100 % when the population has no room for it (D829); a head not started
	// waits for room (population full) or starts at the server's next step.
	int HeadState(int QueueCount, bool bHeadStarted, float Progress, bool bBlockedAtFull, bool bPopulationShort)
	{
		if (QueueCount <= 0)
			return HeadNone();
		if (bHeadStarted)
			return (Progress >= 1.0 && bBlockedAtFull) ? HeadPopulationFull() : HeadInProgress();
		return bPopulationShort ? HeadPopulationFull() : HeadStarting();
	}

	// The bar's percent: the head's progress once started, 0 otherwise (a head not started has no progress, even when it waits for room).
	float HeadPercent(int State, float Progress)
	{
		if (State == HeadNone() || State == HeadStarting())
			return 0.0;
		return Progress;
	}

	// D829: a unit held at 100 % keeps its full bar, the note beside it; a head waiting at 0 shows the note alone.
	bool ProgressShown(int State, float Progress)
	{
		return State != HeadPopulationFull() || Progress > 0.0;
	}

	bool NoteShown(int State)
	{
		return State == HeadPopulationFull();
	}

	// "%.2f" of the C++ report, by hand (0.5 -> "0.50", 1 -> "1.00").
	FString TwoDecimals(float Value)
	{
		const int Hundredths = Math::RoundToInt(Value * 100.0);
		const int Rest = Hundredths % 100;
		return f"{Hundredths / 100}." + (Rest < 10 ? "0" : "") + f"{Rest}";
	}

	// Describe's state word of the item at Index: the head's state, "waiting" for the others.
	FString ItemState(int Index, int State, float Progress)
	{
		if (Index != 0)
			return "waiting";
		if (State == HeadInProgress())
			return TwoDecimals(Progress);
		if (State == HeadPopulationFull())
			return "population full";
		return "starting";
	}

	// Describe's queue part, read by quick_check after "queue ": "2 [Peasant 0.40, Peasant waiting]", "0 []".
	FString ReportLine(TArray<FString> Types, int State, float Progress)
	{
		FString Items = "";
		for (int Index = 0; Index < Types.Num(); ++Index)
			Items += (Index > 0 ? ", " : "") + Types[Index] + " " + ItemState(Index, State, Progress);
		return f"{Types.Num()} [{Items}]";
	}

	// The journal line of a click on an item (HandleQueueItemClicked), written by UHOLGameMenuScreen::MenuLog (level Display).
	FString ClickLine(int Index)
	{
		return f"HUD: queue item {Index} clicked (cancel)";
	}

	// ClickWidget: a click by name goes to an item only when the panel is shown and the button visible; others are refused.
	bool ClickAllowed(bool bPanelShown, bool bItemVisible)
	{
		return bPanelShown && bItemVisible;
	}
}

// The queue panel. Built once in the script screen's OnInitialized (Build, into the C++ screen's QueuePanel), refreshed in its Refresh (Refresh).
class UHOLProductionQueuePanel : UObject
{
	UHOLScriptScreen Screen;

	UBorder QueuePanel;
	TArray<UHOLQueueItem> Items;
	UProgressBar QueueProgress;
	UCommonTextBlock QueueNote;

	// What is shown, for the report and the tests: the queue's types, the head's state and progress, the labels' signature ("" never shown yet).
	TArray<FString> ShownTypes;
	int ShownState = 0;
	float ShownProgress = 0.0;
	FString ShownSignature = "";
	bool bQueueShown = false;

	// AddToRow of the C++ screen: centred vertically, a right padding.
	void AddToRow(UHorizontalBox Row, UWidget Widget, float RightPadding)
	{
		UHorizontalBoxSlot Slot = Row.AddChildToHorizontalBox(Widget);
		Slot.SetVerticalAlignment(EVerticalAlignment::VAlign_Center);
		Slot.SetPadding(FMargin(0.0, 0.0, RightPadding, 0.0));
	}

	// InPanel: the C++ screen's QueuePanel (GetQueuePanel), already in its CommandColumn with its fill alignment and its 6 px bottom padding; collapsed.
	void Build(UHOLScriptScreen InScreen, UBorder InPanel)
	{
		Screen = InScreen;
		QueuePanel = InPanel;
		UVerticalBox QueueColumn = Cast<UVerticalBox>(Screen.ConstructWidget(UVerticalBox, n"QueueColumn"));
		QueuePanel.SetContent(QueueColumn);
		UHorizontalBox QueueRow = Cast<UHorizontalBox>(Screen.ConstructWidget(UHorizontalBox, n"QueueRow"));
		QueueColumn.AddChildToVerticalBox(QueueRow);
		for (int Index = 0; Index < HOLQueuePanel::ItemCount(); ++Index)
		{
			USizeBox ItemBox = Cast<USizeBox>(Screen.ConstructWidget(USizeBox, FName(f"QueueItemBox{Index}")));
			ItemBox.SetWidthOverride(HOLQueuePanel::ItemWidth());
			ItemBox.SetHeightOverride(HOLQueuePanel::ItemHeight());
			UHOLQueueItem Item = Cast<UHOLQueueItem>(Screen.ConstructWidget(UHOLQueueItem, FName(HOLQueuePanel::ItemName(Index))));
			Item.OnButtonBaseClicked.AddUFunction(this, FName(f"QueueItem{Index}Clicked"));
			Item.SetVisibility(ESlateVisibility::Collapsed);
			ItemBox.SetContent(Item);
			Items.Add(Item);
			AddToRow(QueueRow, ItemBox, 4.0);
		}

		// The bar and the note share one line of a fixed height, so that the panel does not move when one replaces the other.
		USizeBox StatusBox = Cast<USizeBox>(Screen.ConstructWidget(USizeBox, n"QueueStatus"));
		StatusBox.SetHeightOverride(18.0);
		QueueColumn.AddChildToVerticalBox(StatusBox).SetPadding(FMargin(0.0, 6.0, 0.0, 0.0));
		UOverlay Status = Cast<UOverlay>(Screen.ConstructWidget(UOverlay, n"QueueStatusLine"));
		StatusBox.SetContent(Status);

		// The C++ bar: background (0.08, 0.08, 0.1) and the accent fill, both rounded by 2; MakeBar gives the fill's colour (the background and the corners:
		// to compare on the capture, lesson 3).
		QueueProgress = Screen.MakeBar(n"QueueProgress", UHOLScriptScreen::GetAccentColor());
		// The C++ box has a height only (the bar fills the line's width): UHOLScriptScreen::SizedBar does not exist (the hero block's SizedBar is the script screen's own).
		USizeBox BarBox = Cast<USizeBox>(Screen.ConstructWidget(USizeBox, n"QueueProgressBox"));
		BarBox.SetHeightOverride(6.0);
		BarBox.SetContent(QueueProgress);
		UOverlaySlot BarSlot = Status.AddChildToOverlay(BarBox);
		BarSlot.SetHorizontalAlignment(EHorizontalAlignment::HAlign_Fill);
		BarSlot.SetVerticalAlignment(EVerticalAlignment::VAlign_Center);

		QueueNote = Screen.MakeText(n"QueueNote", UHOLTextStyleHUDAlert);
		QueueNote.SetText(HOLQueuePanelText::PopulationFull());
		QueueNote.SetVisibility(ESlateVisibility::Collapsed);
		UOverlaySlot NoteSlot = Status.AddChildToOverlay(QueueNote);
		NoteSlot.SetHorizontalAlignment(EHorizontalAlignment::HAlign_Left);
		NoteSlot.SetVerticalAlignment(EVerticalAlignment::VAlign_Center);

		UCommonTextBlock Hint = Screen.MakeText(n"QueueHint", UHOLTextStyleHUDLabel);
		Hint.SetText(HOLQueuePanelText::CancelHint());
		UVerticalBoxSlot HintSlot = QueueColumn.AddChildToVerticalBox(Hint);
		HintSlot.SetHorizontalAlignment(EHorizontalAlignment::HAlign_Right);
		HintSlot.SetPadding(FMargin(0.0, 4.0, 0.0, 0.0));
		QueuePanel.SetVisibility(ESlateVisibility::Collapsed);
	}

	// One handler a button (the C++ bound the index with the delegate); a function's name is the only handle AddUFunction takes.
	UFUNCTION()
	void QueueItem0Clicked(UCommonButtonBase Button) { ClickItem(0); }
	UFUNCTION()
	void QueueItem1Clicked(UCommonButtonBase Button) { ClickItem(1); }
	UFUNCTION()
	void QueueItem2Clicked(UCommonButtonBase Button) { ClickItem(2); }
	UFUNCTION()
	void QueueItem3Clicked(UCommonButtonBase Button) { ClickItem(3); }
	UFUNCTION()
	void QueueItem4Clicked(UCommonButtonBase Button) { ClickItem(4); }

	// HandleQueueItemClicked: the journal line, then the cancel (the controller; nothing without one, no line either).
	bool ClickItem(int Index)
	{
		// Screen.CancelProduction is AHOLPlayerController::RequestCancelProduction (not a UFUNCTION); false without a controller, no journal line either.
		if (Cast<AHOLPlayerController>(Screen.GetOwningPlayer()) == nullptr)
			return false;
		UHOLGameMenuScreen::MenuLog(HOLQueuePanel::ClickLine(Index));
		return Screen.CancelProduction(Index);
	}

	// hol.HUD.Click's names (ClickWidget): QueueItem0..4; false when it is not one of ours or when the panel or the button is hidden.
	bool ClickName(FName WidgetName)
	{
		for (int Index = 0; Index < Items.Num(); ++Index)
		{
			if (Items[Index] != nullptr && Items[Index].GetName() == WidgetName)
			{
				if (!HOLQueuePanel::ClickAllowed(bQueueShown, Items[Index].IsVisible()))
					return false;
				return ClickItem(Index);
			}
		}
		return false;
	}

	// RefreshQueue + UpdateHead in one, from the selected own building's queue (GetQueueNow). Cheap when nothing changed: the labels are set again only
	// when the signature changes, the bar's percent only when it differs.
	void Refresh()
	{
		if (QueuePanel == nullptr)
			return;
		TArray<FName> Types;
		TArray<bool> Research;
		TArray<FString> TechNames;
		TArray<FString> UnitNames;
		bool bHeadStarted = false;
		float32 HeadProgress = 0.0;
		bool bBlockedAtFull = false;
		bool bPopulationShort = false;
		if (!Screen.GetQueueNow(Types, Research, TechNames, UnitNames, bHeadStarted, HeadProgress, bBlockedAtFull, bPopulationShort))
		{
			Types.Reset();
		}
		TArray<FString> Names;
		for (int Index = 0; Index < Types.Num(); ++Index)
			Names.Add(Types[Index].ToString());
		const int Count = Names.Num();

		bQueueShown = HOLQueuePanel::PanelShown(Count);
		QueuePanel.SetVisibility(bQueueShown ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
		ShownTypes = Names;

		const FString Signature = HOLQueuePanel::Signature(Names);
		const bool bLabels = HOLQueuePanel::SignatureChanged(ShownSignature, Signature);
		for (int Index = 0; Index < Items.Num(); ++Index)
		{
			const bool bShown = HOLQueuePanel::ItemShown(Index, Count);
			Items[Index].SetVisibility(bShown ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
			if (bShown && bLabels)
				Items[Index].SetLabel(FText::FromString(HOLQueuePanel::ItemLabel(Research[Index], TechNames[Index], UnitNames[Index])));
		}
		ShownSignature = Signature;

		// UpdateHead: with no queue the head is none and the widgets keep what they had (the panel is collapsed).
		const int State = HOLQueuePanel::HeadState(Count, bHeadStarted, HeadProgress, bBlockedAtFull, bPopulationShort);
		const float Progress = HOLQueuePanel::HeadPercent(State, HeadProgress);
		ShownState = State;
		ShownProgress = Progress;
		if (State != HOLQueuePanel::HeadNone())
		{
			if (QueueProgress.GetPercent() != Progress)
				QueueProgress.SetPercent(Progress);
			QueueProgress.SetVisibility(HOLQueuePanel::ProgressShown(State, Progress) ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
			QueueNote.SetVisibility(HOLQueuePanel::NoteShown(State) ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
		}
		Screen.SetQueueReport(Describe());
		// The head's bar moves every frame: the screen calls QueueFrameTick (-> FrameTick) only while a queue is shown (the C++ UpdateHead ran every frame).
		Screen.SetQueueLive(bQueueShown);
	}

	void FrameTick()
	{
		Refresh();
	}

	// Describe's queue part, as the C++ screen printed it.
	FString Describe() const
	{
		return HOLQueuePanel::ReportLine(ShownTypes, ShownState, ShownProgress);
	}
}
