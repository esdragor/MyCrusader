// NOT COMPILED YET (SAURON-1119m, H-10 of the HUD plan, wave 1: new files only, no switch-over): this file calls bindings that do not exist before the next C++ lot
// (UHOLScriptScreen::GetGridPanelHost, GetGridFrame, ApplyGridCell, ClickGridCell, RegisterGridCells; the struct FHOLGridCellInfoView; the RefreshGridCells event).
// The recipe, with the exact signatures, is in Docs\Rapports\SAURON-1119m.md; until the lot exists the whole script module would fail to compile with this file
// present: do NOT build with it and without the lot.
//
// The command grid of the game HUD (3 rows of 4 square cells, keys A Z E R / Q S D F / W X C V), in script, as the VIEW only. What each cell holds (which command,
// greyed or not, cost, missing, tooltip) stays computed by the C++ already there (HOLCommandGrid and the extracted body of UHOLGameScreen::RefreshGrid: one source,
// it migrates at step 5). The cells stay C++ (UHOLGridCell: Slate painting, D839). The script builds the panel and the cells (same names GridPanel, Grid, GridCell00..11),
// puts the keys, hides or shows the panel, fills each cell from the frame the C++ computed, relays the click and writes the journal line (UHOLGameMenuScreen::MenuLog:
// "LogHOLUI: Display: <line>", the level kept as the C++ UE_LOG wrote it).

namespace HOLCommandGridPanelRules
{
	// Functions, not constants: a constant of a namespace is not visible from another file (the test).
	// The C++ constants of the screen (HOLGameScreen.cpp:242-249: GridKeys, GridColumns, CellSize; CellGap).
	int NumCells() { return 12; }
	int GridColumns() { return 4; }
	float CellSize() { return 76.0; }
	float CellGap() { return 2.0; }

	// The cell kinds of the C++ frame (FHOLGridCellInfoView.Kind): 0 nothing (an empty cell stays, dark, at its key's place), 1 a command (name, state, tooltip).
	int KindNone() { return 0; }
	int KindCommand() { return 1; }

	// EHOLGridCellState as a number: 0 Ready, 1 Greyed, 2 Disabled.
	int StateReady() { return 0; }
	int StateGreyed() { return 1; }
	int StateDisabled() { return 2; }

	// The key a cell carries, row by row as on an AZERTY keyboard (D30); "?" outside the grid.
	FString CellKey(int Cell)
	{
		FString Keys = "AZERQSDFWXCV";
		if (Cell < 0 || Cell >= Keys.Len())
			return "?";
		return Keys.Mid(Cell, 1);
	}

	// GridCell00 .. GridCell11: hol.HUD.Click, hol.Mouse.Hover, DescribeRects and the scenarios find the cells by this name.
	FString CellNameText(int Cell)
	{
		FString Digits = f"{Cell}";
		if (Cell < 10)
			Digits = "0" + Digits;
		return "GridCell" + Digits;
	}

	// The cell's state as the report words it (State of the cell read back): enabled / greyed / disabled.
	FString StateWord(int State)
	{
		if (State == StateReady())
			return "enabled";
		if (State == StateGreyed())
			return "greyed";
		return "disabled";
	}

	// A cell with no command is dark: Disabled, whatever state the C++ frame carried; a command keeps the frame's state.
	int EffectiveState(int Kind, int FrameState)
	{
		if (Kind == KindNone())
			return StateDisabled();
		return FrameState;
	}

	// A cell is filled (name, tooltip) only when it holds a command; an empty one is cleared (ClearUnit), its tooltip gone with it.
	bool NeedsFill(int Kind)
	{
		return Kind == KindCommand();
	}

	// The panel's visibility: Visible while the building's grid or the units' orders show, else Hidden, never Collapsed (its place in the bottom bar kept).
	bool PanelVisible(bool bGridShown, bool bOrdersShown)
	{
		return bGridShown || bOrdersShown;
	}

	// The units' orders take the click only while no building's grid shows and the cell holds an order (C++ HandleGridCellClicked).
	bool IsOrderClick(bool bGridShown, bool bOrdersShown, FName OrderType)
	{
		return !bGridShown && bOrdersShown && OrderType != NAME_None;
	}

	// The journal line of a click, mot pour mot as the C++ wrote it ("HUD: grid cell A clicked" / "HUD: order cell Q clicked"); "?" for a cell outside the grid.
	FString ClickLine(int Cell, bool bOrder)
	{
		if (bOrder)
			return "HUD: order cell " + CellKey(Cell) + " clicked";
		return "HUD: grid cell " + CellKey(Cell) + " clicked";
	}

	// hol.HUD.Click GridCell<NN> (C++ ClickWidget): nothing while neither the grid nor the orders show, nor on a disabled cell.
	bool ClickAllowed(bool bGridShown, bool bOrdersShown, bool bCellEnabled)
	{
		if (!bGridShown && !bOrdersShown)
			return false;
		return bCellEnabled;
	}

	// The report's "HUD cells" part of a cell: "A: Peasant" (the cell's content, its breaks replaced by a space), empty content nothing.
	FString ContentPart(int Cell, FString Content)
	{
		if (Content.IsEmpty())
			return "";
		return CellKey(Cell) + ": " + Content.Replace("\n", " ");
	}
}

class UHOLCommandGridPanel : UObject
{
	UHOLScriptScreen Screen;
	UVerticalBox Host;
	UBorder Panel;
	TArray<UHOLGridCell> Cells;

	// What is shown, for the tests and the report (a script test cannot read a widget).
	bool bGridShown = false;
	bool bOrdersShown = false;
	TArray<int> ShownKinds;
	TArray<int> ShownStates;
	TArray<FName> ShownNames;

	// BuildGrid of the C++ screen (HOLGameScreen.cpp:2797-2829), same names, places and styles. Called from OnInitialized after the queue panel, so GridPanel
	// lands under QueuePanel in the command column (the host keeps that place). The keys are set here at once: UHOLGridCell keeps one given before its own
	// initialisation (the 15e4094 fix: a key given before NativeOnInitialized is shown).
	void Build(UHOLScriptScreen InScreen)
	{
		Screen = InScreen;
		Host = Screen.GetGridPanelHost();
		Panel = Screen.MakePanel(n"GridPanel");
		Panel.SetPadding(FMargin(6.0));
		UVerticalBoxSlot PanelSlot = Host.AddChildToVerticalBox(Panel);
		PanelSlot.SetHorizontalAlignment(EHorizontalAlignment::HAlign_Left);
		UUniformGridPanel Grid = Cast<UUniformGridPanel>(Screen.ConstructWidget(UUniformGridPanel, n"Grid"));
		Grid.SetSlotPadding(FMargin(HOLCommandGridPanelRules::CellGap()));
		Panel.SetContent(Grid);
		for (int Index = 0; Index < HOLCommandGridPanelRules::NumCells(); Index++)
		{
			USizeBox Box = Cast<USizeBox>(Screen.ConstructWidget(USizeBox, NAME_None));
			Box.SetWidthOverride(HOLCommandGridPanelRules::CellSize());
			Box.SetHeightOverride(HOLCommandGridPanelRules::CellSize());
			UHOLGridCell Cell = Cast<UHOLGridCell>(Screen.ConstructWidget(UHOLGridCell, FName(HOLCommandGridPanelRules::CellNameText(Index))));
			Cell.SetKey(FText::FromString(HOLCommandGridPanelRules::CellKey(Index)));
			Cell.OnCellClicked.AddUFunction(this, n"OnCellClicked");
			Box.SetContent(Cell);
			Grid.AddChildToUniformGrid(Box, Index / HOLCommandGridPanelRules::GridColumns(), Index % HOLCommandGridPanelRules::GridColumns());
			Cells.Add(Cell);
			ShownKinds.Add(HOLCommandGridPanelRules::KindNone());
			ShownStates.Add(HOLCommandGridPanelRules::StateDisabled());
			ShownNames.Add(NAME_None);
		}
		Panel.SetVisibility(ESlateVisibility::Hidden);
		// The C++ screen keeps reading the cells for its report (DescribeCells, DescribeTooltips, "grid A=Peasant enabled": the cell's own state): it is given them.
		Screen.RegisterGridCells(Cells);
	}

	// A cell's click (UHOLGridCell::OnCellClicked): which cell, then the journal line and the controller's activation (C++ ClickGridCell decides order or grid).
	UFUNCTION()
	void OnCellClicked(UHOLGridCell Cell)
	{
		for (int Index = 0; Index < Cells.Num(); Index++)
		{
			if (Cells[Index] == Cell)
			{
				ClickCell(Index);
				return;
			}
		}
	}

	void ClickCell(int Index)
	{
		if (Index < 0 || Index >= Cells.Num())
			return;
		FName OrderType = NAME_None;
		if (ShownNames.IsValidIndex(Index))
			OrderType = ShownNames[Index];
		bool bOrder = HOLCommandGridPanelRules::IsOrderClick(bGridShown, bOrdersShown, OrderType);
		UHOLGameMenuScreen::MenuLog(HOLCommandGridPanelRules::ClickLine(Index, bOrder));
		Screen.ClickGridCell(Index);
	}

	// hol.HUD.Click GridCell<NN> (from the script screen's ClickWidget): true when the name is one of the cells and the click was taken (false: not ours, or refused).
	bool ClickName(FName WidgetName)
	{
		for (int Index = 0; Index < Cells.Num(); Index++)
		{
			if (Cells[Index].GetName() != WidgetName)
				continue;
			if (!HOLCommandGridPanelRules::ClickAllowed(bGridShown, bOrdersShown, Cells[Index].GetIsEnabled()))
				return false;
			ClickCell(Index);
			return true;
		}
		return false;
	}

	// RefreshGrid of the C++ screen, view part: the C++ has just computed the frame (contents, states, tooltips, the Shown* mirrors of its report); the panel shows it.
	// Called by the C++ at each change of selection or stock and by its 250 ms tick for the hero / rage / deploy cells (UHOLGameScreen::Tick, HOLGameScreen.cpp:494-505).
	void Refresh()
	{
		TArray<FHOLGridCellInfoView> Frame;
		Screen.GetGridFrame(Frame, bGridShown, bOrdersShown);
		if (HOLCommandGridPanelRules::PanelVisible(bGridShown, bOrdersShown))
			Panel.SetVisibility(ESlateVisibility::Visible);
		else
			Panel.SetVisibility(ESlateVisibility::Hidden);
		for (int Index = 0; Index < Cells.Num(); Index++)
		{
			int Kind = HOLCommandGridPanelRules::KindNone();
			int State = HOLCommandGridPanelRules::StateDisabled();
			FName Name = NAME_None;
			if (Frame.IsValidIndex(Index))
			{
				Kind = Frame[Index].Kind;
				State = Frame[Index].State;
				Name = Frame[Index].OrderType;
			}
			ShownKinds[Index] = Kind;
			ShownStates[Index] = HOLCommandGridPanelRules::EffectiveState(Kind, State);
			ShownNames[Index] = Name;
			if (HOLCommandGridPanelRules::NeedsFill(Kind))
				Screen.ApplyGridCell(Index, Cells[Index]);
			else
				Cells[Index].ClearUnit();
		}
	}
}
