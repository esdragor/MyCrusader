// NOT COMPILED YET (GALADRIEL-1119a, H-8 of the HUD plan): this file calls bindings that do not exist before the next C++ lot (UHOLScriptScreen::GetAgePanelsHost,
// GetAgesContext, GetChosenAge, GetTier, ClickAge, FillAgeCell, SetAgesReport, SetRoundedBrush, GetStyleColor; UHOLGridCell UFUNCTIONs SetKey / SetCommand /
// ClearUnit / GetState and its OnCellClicked event; FHOLGridContextView and HOLCommandGridRead::IsBuildGrid exposed). The recipe is in
// Docs\Rapports\GALADRIEL-1119a.md; until then the whole script module would fail to compile: do NOT build with this file present and without the lot.
//
// The builders' age panels of the game HUD (AoE4's way, the user's trials of G.1), in script: right of the command panel, I to IV side by side, each its
// medallion (numeral in a disc, its key A Z E R in a square beside; no click) on its top edge over its buildings at their keys' places Q S D F / W X C V
// (only the columns its buildings use, its 3 rows always kept); an age not reached faded, the chosen one's medallion and edge lit; a click on a building
// is the key's. The rules are pure (namespace HOLAgePanels: the texts, the looks, the columns packed, the journal and report lines); UHOLAgePanelsPanel
// builds the same widgets as the C++ screen (same names AgePanel<age>, AgeMedallionRow<age>, AgeMedallion<age>, AgeNumeral<age>, AgeKey<age>,
// AgeCell<age><cell>; same places, same styles) and refreshes them. The cells stay C++ (UHOLGridCell: painting, tooltip), built here as the C++ did.
// The rule of what an age holds (reached, building at a key, opacity) is HOLCommandGridRead's (one source): called here, not copied.

namespace HOLAgePanelsText
{
	// Same namespace and keys as the C++ LOCTEXT of UHOLGameScreen ("HOLGameScreen,<key>"): the translations stay.
	FText Age0() { return NSLOCTEXT("HOLGameScreen", "Age0", "Fondation"); }
	FText Age1() { return NSLOCTEXT("HOLGameScreen", "Age1", "Essor"); }
	FText Age2() { return NSLOCTEXT("HOLGameScreen", "Age2", "Puissance"); }
	FText Age3() { return NSLOCTEXT("HOLGameScreen", "Age3", "Légende"); }
	FText Header() { return NSLOCTEXT("HOLGameScreen", "AgeHeaderName", "Palier {0} — {1}"); }
	FText Locked() { return NSLOCTEXT("HOLGameScreen", "AgeLocked", "Palier non atteint"); }
}

namespace HOLAgePanels
{
	// Functions, not constants: a constant of a namespace is not visible from another file (the test).
	// The C++ constants of the screen (HOLGameScreen.cpp:246-248, 263, 267-269, 315).
	int NumAges() { return 4; }
	int NumCells() { return 12; }
	int GridColumns() { return 4; }
	float MedallionSize() { return 40.0; }
	float KeyBadgeSize() { return 24.0; }
	float MedallionRise() { return 26.0; }
	float CellSize() { return 76.0; }
	float CellGap() { return 2.0; }
	float BarGap() { return 6.0; }

	// The medallion's looks: its age not reached (faded), reached, chosen (lit).
	int LookLocked() { return 0; }
	int LookReached() { return 1; }
	int LookChosen() { return 2; }

	// The ages' numerals (I to IV).
	FString Numeral(int Age)
	{
		if (Age == 0)
			return "I";
		if (Age == 1)
			return "II";
		if (Age == 2)
			return "III";
		return "IV";
	}

	// The grid's keys, one per cell (GridKeys: A Z E R / Q S D F / W X C V); an age's medallion carries the first row's key of its number.
	FString CellKey(int Cell)
	{
		FString Keys = "AZERQSDFWXCV";
		if (Cell < 0 || Cell >= Keys.Len())
			return "?";
		return Keys.Mid(Cell, 1);
	}

	FText AgeName(int Age)
	{
		if (Age == 0)
			return HOLAgePanelsText::Age0();
		if (Age == 1)
			return HOLAgePanelsText::Age1();
		if (Age == 2)
			return HOLAgePanelsText::Age2();
		return HOLAgePanelsText::Age3();
	}

	// "Palier II — Essor".
	FString HeaderName(int Age)
	{
		return HOLAgePanelsText::Header().ToString().Replace("{0}", Numeral(Age)).Replace("{1}", AgeName(Age).ToString());
	}

	// The medallion's tooltip as set: the header, and "Palier non atteint" under it while not reached.
	FString TooltipText(int Age, bool bReached)
	{
		if (bReached)
			return HeaderName(Age);
		return HeaderName(Age) + "\n" + HOLAgePanelsText::Locked().ToString();
	}

	// The same as the report keeps it (ShownAgeTooltips): one line, " | " for the break.
	FString TooltipLine(int Age, bool bReached)
	{
		if (bReached)
			return HeaderName(Age);
		return HeaderName(Age) + " | " + HOLAgePanelsText::Locked().ToString();
	}

	// Not reached: Locked; the chosen age (only while the panels are shown, else -1): Chosen; else Reached.
	int Look(bool bReached, int Age, int ChosenAge)
	{
		if (!bReached)
			return LookLocked();
		if (Age == ChosenAge)
			return LookChosen();
		return LookReached();
	}

	// The chain of the rule: the context's age reached (HOLCommandGridRead, the one source) then its look.
	int LookIn(FHOLGridContextView Context, int Age, int ChosenAge)
	{
		return Look(HOLCommandGridRead::IsAgeReached(Context, Age), Age, ChosenAge);
	}

	// The columns an age's buildings use, packed: HasBuilding is the 12 cells' flags (a building at that key); the panel is the width of its content, an empty age
	// one column (the 1st), the 1st column also kept when only itself is unused by others (C++: bUsed[0] |= !bUsed[1] && !bUsed[2] && !bUsed[3]).
	TArray<bool> UsedColumns(TArray<bool> HasBuilding)
	{
		TArray<bool> Used;
		for (int Column = 0; Column < GridColumns(); Column++)
			Used.Add(false);
		for (int Index = 0; Index < HasBuilding.Num() && Index < NumCells(); Index++)
		{
			if (HasBuilding[Index])
				Used[Index % GridColumns()] = true;
		}
		if (!Used[1] && !Used[2] && !Used[3])
			Used[0] = true;
		return Used;
	}

	// A cell's column once the unused ones before it are collapsed.
	int PackedColumn(TArray<bool> Used, int Column)
	{
		int Packed = 0;
		for (int Before = 0; Before < Column && Before < Used.Num(); Before++)
		{
			if (Used[Before])
				Packed++;
		}
		return Packed;
	}

	// A building cell is Ready only when its age is reached and the stock covers the cost (Greyed: the rule's bGreyed, or something missing).
	bool IsCellReady(bool bBuildingGreyed, bool bNothingMissing)
	{
		return !bBuildingGreyed && bNothingMissing;
	}

	// EHOLGridCellState as the report words it: 0 Ready, 1 Greyed, else disabled.
	FString StateWord(int State)
	{
		if (State == 0)
			return "enabled";
		if (State == 1)
			return "greyed";
		return "disabled";
	}

	// "A=House enabled" (a cell of the report's "ages" line).
	FString CellPart(int Cell, FString TypeName, int State)
	{
		return CellKey(Cell) + "=" + TypeName + " " + StateWord(State);
	}

	// "I=open [A=House enabled Z=LumberCamp greyed]" (Cells already joined by a space).
	FString AgeSegment(int Age, bool bReached, FString Cells)
	{
		FString Word = "locked";
		if (bReached)
			Word = "open";
		return Numeral(Age) + "=" + Word + " [" + Cells + "]";
	}

	// "ages I=open [...] II=locked [] (chosen I)": ChosenAge outside 0..3 is "none".
	FString GridLine(TArray<FString> Segments, int ChosenAge)
	{
		FString Joined = "";
		for (int Index = 0; Index < Segments.Num(); Index++)
		{
			if (Index > 0)
				Joined += " ";
			Joined += Segments[Index];
		}
		FString Chosen = "none";
		if (ChosenAge >= 0 && ChosenAge < NumAges())
			Chosen = Numeral(ChosenAge);
		return "ages " + Joined + " (chosen " + Chosen + ")";
	}

	// The journal line of a click on a building of an age panel (HandleAgeClicked): "HUD: age I A clicked"; "?" for a key outside the grid.
	FString ClickLine(int Age, int Cell)
	{
		FString Key = "?";
		if (Cell >= 0 && Cell < NumCells())
			Key = CellKey(Cell);
		return "HUD: age " + Numeral(Age) + " " + Key + " clicked";
	}

	// A click on a cell (ClickWidget): nothing while the panels are hidden or the cell holds no building (an age not reached takes the click and does nothing, as a greyed cell).
	bool ClickAllowed(bool bAgesShown, bool bCellHasBuilding)
	{
		return bAgesShown && bCellHasBuilding;
	}

	// The widget names (hol.HUD.Click, hol.Mouse.Hover, DescribeRects find them by name).
	FName PanelName(int Age) { return FName(f"AgePanel{Age}"); }
	FName MedallionRowName(int Age) { return FName(f"AgeMedallionRow{Age}"); }
	FName MedallionName(int Age) { return FName(f"AgeMedallion{Age}"); }
	FName NumeralName(int Age) { return FName(f"AgeNumeral{Age}"); }
	FName KeyName(int Age) { return FName(f"AgeKey{Age}"); }

	// AgeCell<age><cell, 2 digits>: AgeCell000 the house (age 0, A), AgeCell004 the barracks (Q), AgeCell111 age II's last cell.
	FString CellNameText(int Age, int Cell)
	{
		FString Digits = f"{Cell}";
		if (Cell < 10)
			Digits = "0" + Digits;
		return f"AgeCell{Age}{Digits}";
	}

	// The "HUD content" part of the report: "I A: Maison" (the cell's content, its breaks replaced by a space).
	FString ContentPart(int Age, int Cell, FString Content)
	{
		return Numeral(Age) + " " + CellKey(Cell) + ": " + Content.Replace("\n", " ");
	}

	// The report's tooltip part of a medallion: "I A: <header>" (the 1st cell's key stands for the age's: A Z E R).
	FString MedallionPart(int Age, FString Tooltip)
	{
		return Numeral(Age) + " " + CellKey(Age) + ": " + Tooltip;
	}

	// What is written to the screen each time the picture changes: one signature so that an unchanged refresh writes nothing.
	FString Signature(bool bShown, int ChosenAge, int Tier, FString Stock, TArray<FString> CellTypes)
	{
		FString Result = f"{bShown}/{ChosenAge}/{Tier}/{Stock}";
		for (int Index = 0; Index < CellTypes.Num(); Index++)
			Result += "/" + CellTypes[Index];
		return Result;
	}
}

class UHOLAgePanelsPanel : UObject
{
	UHOLScriptScreen Screen;
	UHorizontalBox Host;

	TArray<UBorder> Panels;
	TArray<UBorder> Medallions;
	TArray<UHOLGridCell> Cells;
	TArray<USizeBox> CellBoxes;

	// What is shown, for the report and the tests (a script test cannot read a widget).
	bool bAgesShown = false;
	int ShownChosenAge = -1;
	int ShownTier = -1;
	TArray<FName> ShownTypes;
	TArray<bool> ShownReached;
	TArray<int> ShownLooks;
	TArray<FString> ShownTooltips;
	TArray<float> ShownOpacities;
	FString ShownSignature;

	// AddToBar of the C++ screen: at the bar's bottom, a left padding (the first panel a bar gap after the command panel, the others 4).
	void AddToHost(UWidget Widget, float LeftPadding)
	{
		UHorizontalBoxSlot Slot = Host.AddChildToHorizontalBox(Widget);
		Slot.SetVerticalAlignment(EVerticalAlignment::VAlign_Bottom);
		Slot.SetPadding(FMargin(LeftPadding, 0.0, 0.0, 0.0));
	}

	void AddToRow(UHorizontalBox Row, UWidget Widget, float RightPadding)
	{
		UHorizontalBoxSlot Slot = Row.AddChildToHorizontalBox(Widget);
		Slot.SetVerticalAlignment(EVerticalAlignment::VAlign_Center);
		Slot.SetPadding(FMargin(0.0, 0.0, RightPadding, 0.0));
	}

	// The medallion's disc: dark, or lit (the accent) when chosen; its outline the text colour when chosen, muted else.
	void PaintMedallion(UBorder Medallion, bool bChosen)
	{
		FLinearColor Fill = FLinearColor(0.1, 0.11, 0.14, 1.0);
		FLinearColor Outline = UHOLScriptScreen::GetStyleColor(2);
		if (bChosen)
		{
			Fill = UHOLScriptScreen::GetStyleColor(0);
			Outline = UHOLScriptScreen::GetStyleColor(1);
		}
		Screen.SetRoundedBrush(Medallion, Fill, HOLAgePanels::MedallionSize() / 2.0, Outline, 1.5);
	}

	// The panel's edge: lit (accent, 2) when chosen; none else.
	void PaintPanel(UBorder Panel, bool bChosen)
	{
		if (bChosen)
			Screen.SetRoundedBrush(Panel, UHOLScriptScreen::GetStyleColor(3), 4.0, UHOLScriptScreen::GetStyleColor(0), 2.0);
		else
			Screen.SetRoundedBrush(Panel, UHOLScriptScreen::GetStyleColor(3), 4.0, FLinearColor(0.0, 0.0, 0.0, 0.0), 0.0);
	}

	// BuildAgePanels of the C++ screen (HOLGameScreen.cpp:3207-3254), same names, places and styles; the panels start Collapsed, nothing moves while absent.
	void Build(UHOLScriptScreen InScreen)
	{
		Screen = InScreen;
		Host = Screen.GetAgePanelsHost();
		for (int Age = 0; Age < HOLAgePanels::NumAges(); Age++)
		{
			UBorder Panel = Screen.MakePanel(HOLAgePanels::PanelName(Age));
			Panel.SetPadding(FMargin(6.0));
			float Left = 4.0;
			if (Age == 0)
				Left = HOLAgePanels::BarGap();
			AddToHost(Panel, Left);
			UVerticalBox Column = Cast<UVerticalBox>(Screen.ConstructWidget(UVerticalBox, NAME_None));
			Panel.SetContent(Column);
			// The medallion rises over the panel's top edge (drawn higher, its place in the panel mostly given back).
			UHorizontalBox Row = Cast<UHorizontalBox>(Screen.ConstructWidget(UHorizontalBox, HOLAgePanels::MedallionRowName(Age)));
			Row.SetRenderTranslation(FVector2D(0.0, -HOLAgePanels::MedallionRise()));
			UVerticalBoxSlot RowSlot = Column.AddChildToVerticalBox(Row);
			RowSlot.SetHorizontalAlignment(EHorizontalAlignment::HAlign_Center);
			RowSlot.SetPadding(FMargin(0.0, 0.0, 0.0, -HOLAgePanels::MedallionRise() + 4.0));
			USizeBox MedallionBox = Cast<USizeBox>(Screen.ConstructWidget(USizeBox, NAME_None));
			MedallionBox.SetWidthOverride(HOLAgePanels::MedallionSize());
			MedallionBox.SetHeightOverride(HOLAgePanels::MedallionSize());
			UBorder Medallion = Cast<UBorder>(Screen.ConstructWidget(UBorder, HOLAgePanels::MedallionName(Age)));
			PaintMedallion(Medallion, false);
			Medallion.SetPadding(FMargin(0.0));
			Medallion.SetHorizontalAlignment(EHorizontalAlignment::HAlign_Center);
			Medallion.SetVerticalAlignment(EVerticalAlignment::VAlign_Center);
			UCommonTextBlock Numeral = Screen.MakeText(HOLAgePanels::NumeralName(Age), UHOLTextStyleHUDHeading);
			Numeral.SetText(FText::FromString(HOLAgePanels::Numeral(Age)));
			Medallion.SetContent(Numeral);
			MedallionBox.SetContent(Medallion);
			AddToRow(Row, MedallionBox, 3.0);
			USizeBox KeyBox = Cast<USizeBox>(Screen.ConstructWidget(USizeBox, NAME_None));
			KeyBox.SetWidthOverride(HOLAgePanels::KeyBadgeSize());
			KeyBox.SetHeightOverride(HOLAgePanels::KeyBadgeSize());
			UBorder KeyBadge = Cast<UBorder>(Screen.ConstructWidget(UBorder, HOLAgePanels::KeyName(Age)));
			Screen.SetRoundedBrush(KeyBadge, FLinearColor(0.06, 0.065, 0.08, 1.0), 3.0, UHOLScriptScreen::GetStyleColor(2), 1.0);
			KeyBadge.SetPadding(FMargin(0.0));
			KeyBadge.SetHorizontalAlignment(EHorizontalAlignment::HAlign_Center);
			KeyBadge.SetVerticalAlignment(EVerticalAlignment::VAlign_Center);
			UCommonTextBlock Key = Screen.MakeText(NAME_None, UHOLTextStyleHUDSmall);
			Key.SetText(FText::FromString(HOLAgePanels::CellKey(Age)));
			KeyBadge.SetContent(Key);
			KeyBox.SetContent(KeyBadge);
			AddToRow(Row, KeyBox, 0.0);
			UUniformGridPanel Grid = Cast<UUniformGridPanel>(Screen.ConstructWidget(UUniformGridPanel, NAME_None));
			Grid.SetSlotPadding(FMargin(HOLAgePanels::CellGap()));
			UVerticalBoxSlot GridSlot = Column.AddChildToVerticalBox(Grid);
			GridSlot.SetHorizontalAlignment(EHorizontalAlignment::HAlign_Left);
			for (int Index = 0; Index < HOLAgePanels::NumCells(); Index++)
			{
				USizeBox Box = Cast<USizeBox>(Screen.ConstructWidget(USizeBox, NAME_None));
				Box.SetWidthOverride(HOLAgePanels::CellSize());
				Box.SetHeightOverride(HOLAgePanels::CellSize());
				UHOLGridCell Cell = Cast<UHOLGridCell>(Screen.ConstructWidget(UHOLGridCell, FName(HOLAgePanels::CellNameText(Age, Index))));
				Cell.SetKey(FText::FromString(HOLAgePanels::CellKey(Index)));
				Cell.OnCellClicked.AddUFunction(this, n"OnCellClicked");
				Box.SetContent(Cell);
				Grid.AddChildToUniformGrid(Box, Index / HOLAgePanels::GridColumns(), Index % HOLAgePanels::GridColumns());
				Cells.Add(Cell);
				CellBoxes.Add(Box);
				ShownTypes.Add(NAME_None);
			}
			Panel.SetVisibility(ESlateVisibility::Collapsed);
			Panels.Add(Panel);
			Medallions.Add(Medallion);
			ShownReached.Add(false);
			ShownLooks.Add(-1);
			ShownTooltips.Add("");
			ShownOpacities.Add(1.0);
		}
	}

	// A cell's click (UHOLGridCell::OnCellClicked): the index of the cell in the 4 x 12 table, then the journal line and the controller's ClickAge.
	UFUNCTION()
	void OnCellClicked(UHOLGridCell Cell)
	{
		for (int AgeIndex = 0; AgeIndex < Cells.Num(); AgeIndex++)
		{
			if (Cells[AgeIndex] == Cell)
			{
				ClickCell(AgeIndex / HOLAgePanels::NumCells(), AgeIndex % HOLAgePanels::NumCells());
				return;
			}
		}
	}

	void ClickCell(int Age, int Cell)
	{
		if (Age < 0 || Age >= HOLAgePanels::NumAges())
			return;
		UHOLGameMenuScreen::MenuLog(HOLAgePanels::ClickLine(Age, Cell));
		Screen.ClickAge(Age, Cell);
	}

	// hol.HUD.Click AgeCell<age><cell> (from the script screen's ClickWidget): true when the name is one of the cells and the click was taken (false: not ours, or a cell without a building).
	bool ClickName(FName WidgetName)
	{
		for (int AgeIndex = 0; AgeIndex < Cells.Num(); AgeIndex++)
		{
			if (Cells[AgeIndex].GetFName() != WidgetName)
				continue;
			bool bHasBuilding = ShownTypes.IsValidIndex(AgeIndex) && ShownTypes[AgeIndex] != NAME_None;
			if (!HOLAgePanels::ClickAllowed(bAgesShown, bHasBuilding))
				return false;
			ClickCell(AgeIndex / HOLAgePanels::NumCells(), AgeIndex % HOLAgePanels::NumCells());
			return true;
		}
		return false;
	}

	// RefreshAges of the C++ screen. The context is the builders' one (nothing of the player's selected: the units' context), the panels shown while it is a build grid.
	void Refresh()
	{
		FHOLGridContextView Context;
		Screen.GetAgesContext(Context);
		bAgesShown = HOLCommandGridRead::IsBuildGrid(Context);
		int Chosen = -1;
		if (bAgesShown)
			Chosen = Screen.GetChosenAge();
		int Tier = Screen.GetTier();
		FHOLResources Stock;
		int PopulationUsed = 0;
		int PopulationCap = 0;
		FHOLResources Gatherers;
		FHOLResources Income;
		Screen.GetEconomyNow(Stock, PopulationUsed, PopulationCap, Gatherers, Income);

		// What every cell holds, from the rule (one source): a building or none.
		TArray<FHOLGridCommandView> Buildings;
		TArray<FString> TypeNames;
		for (int Age = 0; Age < HOLAgePanels::NumAges(); Age++)
		{
			for (int Index = 0; Index < HOLAgePanels::NumCells(); Index++)
			{
				FHOLGridCommandView Building = HOLCommandGridRead::GetAgeBuilding(Context, Age, Index);
				Buildings.Add(Building);
				TypeNames.Add(f"{Building.TypeName}{Building.bGreyed}");
			}
		}
		FString Signature = HOLAgePanels::Signature(bAgesShown, Chosen, Tier, HOLTopBar::ResourcesLine(Stock), TypeNames);
		bool bChanged = Signature != ShownSignature;
		ShownSignature = Signature;
		ShownChosenAge = Chosen;
		ShownTier = Tier;

		for (int Age = 0; Age < HOLAgePanels::NumAges(); Age++)
		{
			UBorder Panel = Panels[Age];
			ESlateVisibility Wanted = ESlateVisibility::Collapsed;
			if (bAgesShown)
				Wanted = ESlateVisibility::Visible;
			if (Panel.GetVisibility() != Wanted)
				Panel.SetVisibility(Wanted);
			bool bReached = HOLCommandGridRead::IsAgeReached(Context, Age);
			ShownReached[Age] = bReached;
			// The medallion's look, written when it changed: its disc (lit when chosen), the panel's edge.
			int Look = HOLAgePanels::Look(bReached, Age, Chosen);
			if (ShownLooks[Age] != Look)
			{
				ShownLooks[Age] = Look;
				bool bChosen = Look == HOLAgePanels::LookChosen();
				PaintMedallion(Medallions[Age], bChosen);
				PaintPanel(Panel, bChosen);
			}
			// The panel faded only while nothing on it can be built (D797); written when it changed.
			float Opacity = HOLCommandGridRead::GetAgePanelOpacity(Context, Age);
			if (ShownOpacities[Age] != Opacity)
			{
				ShownOpacities[Age] = Opacity;
				Panel.SetRenderOpacity(Opacity);
			}
			// The medallion's tooltip, read on every refresh (the language may change), written when it changed.
			FString Line = HOLAgePanels::TooltipLine(Age, bReached);
			if (ShownTooltips[Age] != Line)
			{
				ShownTooltips[Age] = Line;
				Medallions[Age].SetToolTipText(FText::FromString(HOLAgePanels::TooltipText(Age, bReached)));
			}
			if (!bChanged)
				continue;
			// The columns its buildings use, packed; its 3 rows always kept.
			TArray<bool> HasBuilding;
			for (int Index = 0; Index < HOLAgePanels::NumCells(); Index++)
				HasBuilding.Add(Buildings[Age * HOLAgePanels::NumCells() + Index].TypeName != NAME_None);
			TArray<bool> Used = HOLAgePanels::UsedColumns(HasBuilding);
			for (int Index = 0; Index < HOLAgePanels::NumCells(); Index++)
			{
				int AgeIndex = Age * HOLAgePanels::NumCells() + Index;
				FHOLGridCommandView Building = Buildings[AgeIndex];
				ShownTypes[AgeIndex] = NAME_None;
				if (bAgesShown)
					ShownTypes[AgeIndex] = Building.TypeName;
				int Column = Index % HOLAgePanels::GridColumns();
				USizeBox Box = CellBoxes[AgeIndex];
				ESlateVisibility BoxWanted = ESlateVisibility::Collapsed;
				if (Used[Column])
					BoxWanted = ESlateVisibility::Visible;
				Box.SetVisibility(BoxWanted);
				UUniformGridSlot GridSlot = Cast<UUniformGridSlot>(Box.Slot);
				if (GridSlot != nullptr)
				{
					GridSlot.SetColumn(HOLAgePanels::PackedColumn(Used, Column));
					GridSlot.SetRow(Index / HOLAgePanels::GridColumns());
				}
				if (Building.TypeName == NAME_None)
					Cells[AgeIndex].ClearUnit();
				else
					Screen.FillAgeCell(Cells[AgeIndex], Building); // name, cost, missing, effect, tooltip: C++ (cost rule, names)
			}
		}
		if (bChanged)
			Screen.SetAgesReport(bAgesShown, Chosen, ShownTypes, ShownReached, ShownTooltips);
	}

	// What the report's "ages" line holds for one age (HOLAgePanels::AgeSegment), from the cells as drawn.
	FString Segment(int Age)
	{
		FString CellsText = "";
		for (int Index = 0; Index < HOLAgePanels::NumCells(); Index++)
		{
			int AgeIndex = Age * HOLAgePanels::NumCells() + Index;
			if (ShownTypes[AgeIndex] == NAME_None)
				continue;
			if (CellsText != "")
				CellsText += " ";
			CellsText += HOLAgePanels::CellPart(Index, f"{ShownTypes[AgeIndex]}", int(Cells[AgeIndex].GetState()));
		}
		return HOLAgePanels::AgeSegment(Age, ShownReached[Age], CellsText);
	}

	// "ages I=open [...] ... (chosen I)": the grid part of hol.HUD.Report while the panels are shown (the C++ keeps Describe; it may take this line from SetAgesReport's mirror).
	FString GridLine()
	{
		TArray<FString> Segments;
		for (int Age = 0; Age < HOLAgePanels::NumAges(); Age++)
			Segments.Add(Segment(Age));
		return HOLAgePanels::GridLine(Segments, ShownChosenAge);
	}
}
