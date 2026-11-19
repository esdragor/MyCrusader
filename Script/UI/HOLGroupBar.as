// HUD H-6: the control groups' bar (J7a.5, D30), bottom left over the selection's block: a chip per group not empty, "1 · 9", in the keys' order (1 to 9, then 0);
// a click on a chip selects the group; the group just selected lit for a moment. Rules (namespace HOLGroupBar, pure, tested in HOLGroupBar_Test.as) and view
// (UHOLGroupBarPanel, built once in the script screen's OnInitialized, refreshed in its Refresh), ported from UHOLGameScreen::RefreshControlGroups,
// HandleGroupSlotClicked, the ClickWidget branch and the groups' block of BuildLayout (HOLGameScreen.cpp). The groups themselves (creation, recall, keys) STAY
// in AHOLPlayerController (C++): the script only draws, clicks and writes the journal.
//
// DOES NOT COMPILE ALONE: it calls C++ bindings that do not exist yet, members of UHOLScriptScreen (see HALDIR-1119d.md, section (a)):
//   GetGroupsPanel(), GetNumControlGroups(), GetControlGroupsAvailable(), GetControlGroupNow(Group, Count, bBuildings), GetControlGroupTypes(Group, Names, Counts),
//   GetLastSelectedGroup(), GetLastSelectedGroupAge(), SelectControlGroup(Group).
// It compiles with the NEXT lot (the C++ binding, the hook, the removal of the C++ block).
//
// The host is the C++ screen's GroupsPanel (Screen.GetGroupsPanel()), which stays in C++ in its column (BottomLeft, its brush, its 6/4 padding, its 6 px gap, collapsed):
// the script puts into it the row (GroupsRow) and the ten boxes (ControlGroupBox<N>, 64 x 28.8) each holding a chip (ControlGroup<N>, UHOLQueueItem), the C++ names
// (hol.HUD.Click ControlGroup2, hol.Mouse.Hover). The text are not localised by a string table yet: same NSLOCTEXT keys as the C++ LOCTEXT ("HOLGameScreen,<key>").

namespace HOLGroupBarText
{
	FString Replace1(FText Pattern, FString Value)
	{
		return Pattern.ToString().Replace("{0}", Value);
	}

	// "{0} · {1}": the group's number, its size.
	FText Slot() { return NSLOCTEXT("HOLGameScreen", "ControlGroupSlot", "{0} · {1}"); }
	// The tooltip's head, then a line a type: "{0} × {1}".
	FText Tooltip() { return NSLOCTEXT("HOLGameScreen", "ControlGroupTooltip", "Groupe {0}"); }
	FText UnitCount() { return NSLOCTEXT("HOLGameScreen", "UnitCount", "{0} × {1}"); }
}

namespace HOLGroupBar
{
	// The group just selected stays lit this long (GroupLightSeconds), at this scale; the other chips are dimmed to this opacity meanwhile.
	float LightSeconds() { return 0.5; }
	float LitScale() { return 1.15; }
	float DimOpacity() { return 0.7; }
	// The list is rebuilt this often (GroupRefreshInterval of the C++ screen).
	float RefreshInterval() { return 0.25; }
	// A chip's box: 64 wide, 0.8 of a queue item's height (GroupSlotWidth, QueueItemHeight 36); the gap between boxes.
	float SlotWidth() { return 64.0; }
	float SlotHeight() { return 36.0 * 0.8; }
	float SlotGap() { return 4.0; }

	// The widget names the C++ screen gave (GroupSlotName, ControlGroupBox<N>).
	FName SlotName(int Group) { return FName(f"ControlGroup{Group}"); }
	FName BoxName(int Group) { return FName(f"ControlGroupBox{Group}"); }

	// The bar's order: the keys' (1 to 9, then 0), whatever the number of groups: Index in 1..Count, group Index % Count.
	int GroupAt(int Position, int Count)
	{
		return (Position + 1) % Count;
	}

	// What a group shows: nothing when empty, "b3" for three buildings (a group of buildings only), else its units' count.
	FString ShownText(bool bFull, bool bBuildings, int Count)
	{
		if (!bFull)
			return "";
		if (bBuildings)
			return f"b{Count}";
		return f"{Count}";
	}

	// A group's change is one of these strings differing from the last (the C++ ShownGroups): the chip is touched only then.
	bool ChipChanged(FString Before, FString Now)
	{
		return Before != Now;
	}

	// The chip's label, "1 · 9".
	FString ChipLabel(int Group, int Count)
	{
		return HOLGroupBarText::Slot().ToString().Replace("{0}", f"{Group}").Replace("{1}", f"{Count}");
	}

	// The tooltip's types: the most numerous first, a tie keeping the order the types were met (a stable sort, TMap::ValueStableSort of the C++ screen).
	// Returns the order of the indices.
	TArray<int> TypeOrder(TArray<int> Counts)
	{
		TArray<int> Order;
		for (int Index = 0; Index < Counts.Num(); Index++)
		{
			int At = Order.Num();
			while (At > 0 && Counts[Order[At - 1]] < Counts[Index])
				At--;
			Order.Insert(Index, At);
		}
		return Order;
	}

	// The tooltip: « Groupe 1 », then a line a type, "Name × N", in TypeOrder, one line each.
	FString TooltipText(int Group, TArray<FString> Names, TArray<int> Counts)
	{
		FString Text = HOLGroupBarText::Replace1(HOLGroupBarText::Tooltip(), f"{Group}");
		const TArray<int> Order = TypeOrder(Counts);
		for (int Index = 0; Index < Order.Num(); Index++)
		{
			const int Type = Order[Index];
			Text += "\n" + HOLGroupBarText::UnitCount().ToString().Replace("{0}", Names[Type]).Replace("{1}", f"{Counts[Type]}");
		}
		return Text;
	}

	// The report line: "<group>=<shown>" for each group not empty (the key's number, 0 for the last), joined by a space, "none" when no group is full.
	FString LinePart(int Group, FString Shown)
	{
		return f"{Group}={Shown}";
	}

	FString BarLine(TArray<FString> Parts)
	{
		if (Parts.Num() == 0)
			return "none";
		return FString::Join(Parts, " ");
	}

	// The journal lines (read by quick_check.ps1: "HUD groups: 1=9", "HUD: group 2 clicked"); written by UHOLGameMenuScreen::MenuLog (the level "Display" included).
	FString BarLogLine(FString Line)
	{
		return "HUD groups: " + Line;
	}

	FString ClickLine(int Group)
	{
		return f"HUD: group {Group} clicked";
	}

	// The group lit: the last selected one while it is younger than LightSeconds, else -1 (INDEX_NONE).
	int LitGroup(int Selected, float Age)
	{
		if (Selected >= 0 && Age < LightSeconds())
			return Selected;
		return -1;
	}

	float ItemScale(int Group, int Lit)
	{
		return Group == Lit ? LitScale() : 1.0;
	}

	float ItemOpacity(int Group, int Lit)
	{
		return (Lit == -1 || Group == Lit) ? 1.0 : DimOpacity();
	}

	// A click (mouse or hol.HUD.Click) is taken by a chip showing a group, refused by an empty one.
	bool ClickAccepted(FString Shown)
	{
		return Shown != "";
	}
}

// The object a chip's click is bound to (one per chip: OnButtonBaseClicked names a function, not an argument).
class UHOLGroupBarChip : UObject
{
	UHOLGroupBarPanel Bar;
	int Group = 0;

	UFUNCTION()
	void Clicked(UCommonButtonBase Button)
	{
		Bar.ClickGroup(Group);
	}
}

class UHOLGroupBarPanel : UObject
{
	UHOLScriptScreen Screen;
	UBorder Host;
	UHorizontalBox Row;
	TArray<USizeBox> Boxes;
	TArray<UHOLQueueItem> Chips;
	TArray<UHOLGroupBarChip> Handlers;

	// What each chip shows (the C++ ShownGroups: "" empty, "9", "b2"), the report line as last written, the group lit now, what the tooltip says (for the report and
	// the tests: a script test cannot read a widget).
	TArray<FString> Shown;
	FString ShownLine = "";
	int Lit = -1;
	bool bAnyShown = false;

	void Build(UHOLScriptScreen InScreen, UBorder GroupsPanel)
	{
		Screen = InScreen;
		Host = GroupsPanel;
		Row = Cast<UHorizontalBox>(Screen.ConstructWidget(UHorizontalBox, n"GroupsRow"));
		Host.SetContent(Row);
		const int Count = UHOLScriptScreen::GetNumControlGroups();
		for (int Group = 0; Group < Count; Group++)
		{
			Shown.Add("");
			Boxes.Add(nullptr);
			Chips.Add(nullptr);
			Handlers.Add(nullptr);
		}
		// In the keys' order: 1 to 9, then 0.
		for (int Position = 0; Position < Count; Position++)
		{
			const int Group = HOLGroupBar::GroupAt(Position, Count);
			USizeBox Box = Cast<USizeBox>(Screen.ConstructWidget(USizeBox, HOLGroupBar::BoxName(Group)));
			Box.SetWidthOverride(HOLGroupBar::SlotWidth());
			Box.SetHeightOverride(HOLGroupBar::SlotHeight());
			UHOLQueueItem Chip = Cast<UHOLQueueItem>(Screen.ConstructWidget(UHOLQueueItem, HOLGroupBar::SlotName(Group)));
			Box.SetVisibility(ESlateVisibility::Collapsed);
			Chip.SetRenderTransformPivot(FVector2D(0.5, 0.5));
			Box.SetContent(Chip);
			UHorizontalBoxSlot Slot = Row.AddChildToHorizontalBox(Box);
			Slot.SetVerticalAlignment(EVerticalAlignment::VAlign_Center);
			Slot.SetPadding(FMargin(0.0, 0.0, HOLGroupBar::SlotGap(), 0.0));

			UHOLGroupBarChip Handler = Cast<UHOLGroupBarChip>(NewObject(this, UHOLGroupBarChip));
			Handler.Bar = this;
			Handler.Group = Group;
			Chip.OnButtonBaseClicked.AddUFunction(Handler, n"Clicked");
			Boxes[Group] = Box;
			Chips[Group] = Chip;
			Handlers[Group] = Handler;
		}
	}

	// The click, mouse or hol.HUD.Click (ClickWidget): false for a chip showing no group. The journal line, then the controller's own selection.
	bool ClickGroup(int Group)
	{
		if (Group < 0 || Group >= Shown.Num() || !HOLGroupBar::ClickAccepted(Shown[Group]))
			return false;
		UHOLGameMenuScreen::MenuLog(HOLGroupBar::ClickLine(Group));
		Screen.SelectControlGroup(Group);
		return true;
	}

	// hol.HUD.Click's names: ControlGroup0 to ControlGroup9.
	bool ClickName(FName WidgetName)
	{
		for (int Group = 0; Group < Chips.Num(); Group++)
		{
			if (HOLGroupBar::SlotName(Group) == WidgetName)
				return ClickGroup(Group);
		}
		return false;
	}

	// RefreshControlGroups of the C++ screen: the lit group, then the chips whose group changed, the panel, the journal line.
	void Refresh()
	{
		if (Host == nullptr || !Screen.GetControlGroupsAvailable())
			return;
		const int Count = Chips.Num();

		// The group just selected, lit for a moment (its key, its chip).
		const int NewLit = HOLGroupBar::LitGroup(Screen.GetLastSelectedGroup(), Screen.GetLastSelectedGroupAge());
		if (NewLit != Lit)
		{
			for (int Group = 0; Group < Count; Group++)
			{
				const float Scale = HOLGroupBar::ItemScale(Group, NewLit);
				Chips[Group].SetRenderScale(FVector2D(Scale, Scale));
				Chips[Group].SetRenderOpacity(HOLGroupBar::ItemOpacity(Group, NewLit));
			}
			Lit = NewLit;
		}

		TArray<FString> Parts;
		bool bAny = false;
		for (int Position = 0; Position < Count; Position++)
		{
			const int Group = HOLGroupBar::GroupAt(Position, Count);
			int Size = 0;
			bool bBuildings = false;
			const bool bFull = Screen.GetControlGroupNow(Group, Size, bBuildings);
			const FString Now = HOLGroupBar::ShownText(bFull, bBuildings, Size);
			if (bFull)
			{
				bAny = true;
				Parts.Add(HOLGroupBar::LinePart(Group, Now));
			}
			if (!HOLGroupBar::ChipChanged(Shown[Group], Now))
				continue;
			Shown[Group] = Now;
			// The chip's box, which keeps its width even with the chip collapsed.
			Boxes[Group].SetVisibility(bFull ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
			if (!bFull)
				continue;
			Chips[Group].SetLabel(FText::FromString(HOLGroupBar::ChipLabel(Group, Size)));
			TArray<FString> Names;
			TArray<int> Counts;
			Screen.GetControlGroupTypes(Group, Names, Counts);
			Chips[Group].SetToolTipText(FText::FromString(HOLGroupBar::TooltipText(Group, Names, Counts)));
		}
		bAnyShown = bAny;
		Host.SetVisibility(bAny ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
		const FString Line = HOLGroupBar::BarLine(Parts);
		if (Line != ShownLine)
		{
			ShownLine = Line;
			UHOLGameMenuScreen::MenuLog(HOLGroupBar::BarLogLine(Line));
		}
	}
}
