// HUD H-9, lot C: the selection block's CHIPS (G.1e, AoE4's way): a pool of 12 chips SelectionChip0..11, each in a frame SelectionChipFrame0..11 lit for the active
// subgroup, in a wrapping box SelectionChips, in place of the name; a click on a chip makes its subgroup active, as Tab. Several units (the player's units of several
// types, "Tous (8)" then a chip a type) or a group of the player's buildings ("Tous (7)", "Maison (3)", a main centre alone). Ported from UHOLGameScreen::RefreshChips,
// HandleChipClicked, the chips of RefreshUnitSelection and of RefreshBuildingGroup, and the chips' block of BuildLayout (HOLGameScreen.cpp).
//
// DOES NOT COMPILE ALONE: it needs the lot A class UHOLSelectionPanel (HOLSelection.as: ChipsHost, StatsHost, SetName, SetOwner, SetHealth, SetActivity, SetShownKind,
// SetShownActivity, SetShownHealth) and the C++ binding UHOLSelectionChipsReadLibrary (HOLSelectionChipsRead::, Source\HeroesOfLibertyUI\HUD\HOLSelectionChipsReadLibrary.h/.cpp,
// to compile once with the lots A and B's). The chips' labels (already localized: "Tous (8)", "Paysan (5)", a long name cut to 12 characters) and the lit chip's index come
// from C++ (the controller's Tab subgroups stay the one source of the rule); this file draws, clicks, and holds the pure rules of the VIEW (HOLSelectionChipsRules).
//
// CROCHET : what the switch pastes into HOLGameScreenScript.as (nothing is pasted by this lot):
//   member:   UHOLSelectionChipsPanel SelectionChips;      // after the lot A's UHOLSelectionPanel member
//   Build:    SelectionChips = Cast<UHOLSelectionChipsPanel>(NewObject(this, UHOLSelectionChipsPanel));
//             SelectionChips.Build(this, SelectionPanel);  // SelectionPanel = the lot A's UHOLSelectionPanel, built BEFORE (its ChipsHost must exist)
//   Refresh:  SelectionChips.Refresh();                    // AFTER the lot A's and B's Refresh of the block (the C++ RefreshChips ran after RefreshUnitSelection etc.:
//                                                          // it hides the name when there are chips, and the group of buildings writes its rows here)
//   ClickWidget (hol.HUD.Click): if (SelectionChips.ClickName(WidgetName)) return true;   // SelectionChip0..11, new: no tool clicked them before
// The C++ to remove at the switch: HOLGameScreen.cpp RefreshChips, HandleChipClicked, RefreshBuildingGroup, the chips of RefreshUnitSelection, ChipLabels / ActiveChip /
// ShownChips / ShownActiveChip / ChipItems / ChipFrames, the chips' block of BuildLayout (2705-2727), the binding 391-399, MakeChipLabel (the LOCTEXT "SelectionChip" lives on
// in HOLSelectionChipsReadLibrary.cpp, same namespace "HOLGameScreen").

namespace HOLSelectionChipsRules
{
	// The pool (NumChips of the C++ screen: "Tous" and 11 types), a chip's height, the gap between chips and between lines, the block's width (SelectionWidth 340,
	// the wrap box 24 narrower), a chip at most half of it (ChipMaxWidth), its frame's padding.
	int NumChips() { return 12; }
	float ChipHeight() { return 30.0; }
	float ChipGap() { return 4.0; }
	float WrapSize() { return 340.0 - 24.0; }
	float ChipMaxWidth() { return (340.0 - 24.0 - 4.0) / 2.0; }
	float FramePadding() { return 2.0; }
	float FrameRadius() { return 5.0; }
	// The wrap box's room under it (the C++ slot padding bottom 6).
	float ChipsBottomPadding() { return 6.0; }

	// The names the C++ screen gave (SelectionChip<N>, SelectionChipFrame<N>): hol.HUD.Click, hol.Mouse.Hover, DescribeRects look for them.
	FName ChipName(int Chip) { return FName(f"SelectionChip{Chip}"); }
	FName FrameName(int Chip) { return FName(f"SelectionChipFrame{Chip}"); }

	// The label the chip shows: the controller's label, "" beyond the pool (12) or beyond the labels there are.
	FString ShownLabel(TArray<FString> Labels, int Chip)
	{
		if (Chip < 0 || Chip >= NumChips() || Chip >= Labels.Num())
			return "";
		return Labels[Chip];
	}

	// Chips are shown as soon as there is a label (the C++ ChipLabels not empty).
	bool ChipsShown(TArray<FString> Labels)
	{
		return Labels.Num() > 0;
	}

	// The name gives way to the chips (SetShown(SelectionName, !bChips)).
	bool NameShown(bool bChips)
	{
		return !bChips;
	}

	// A chip's frame is shown with its label (a chip without label is collapsed).
	bool FrameShown(FString Label)
	{
		return Label != "";
	}

	// The lit frame: the active chip's. Active is INDEX_NONE (-1) for the units of another player (nothing to cycle) and for no chips: no frame lit.
	bool FrameLit(int Active, int Chip)
	{
		return Active >= 0 && Active == Chip;
	}

	// A chip's label is written only when its text changed (ShownChips of the C++ screen); a frame's brush only when its lit state changed.
	bool LabelChanged(FString Before, FString Now)
	{
		return Before != Now;
	}

	// The click: a chip of the pool; the controller refuses the rest (no active subgroup, a chip beyond the subgroups).
	bool ClickAccepted(int Chip)
	{
		return Chip >= 0 && Chip < NumChips();
	}
}

// The object a chip's click is bound to (one per chip: OnButtonBaseClicked names a function, not an argument).
class UHOLSelectionChipHandler : UObject
{
	UHOLSelectionChipsPanel Panel;
	int Chip = 0;

	UFUNCTION()
	void Clicked(UCommonButtonBase Button)
	{
		Panel.ClickChip(Chip);
	}
}

class UHOLSelectionChipsPanel : UObject
{
	UHOLScriptScreen Screen;
	UHOLSelectionPanel Owner;
	UWrapBox Chips;
	TArray<UBorder> Frames;
	TArray<UHOLQueueItem> Items;
	TArray<UHOLSelectionChipHandler> Handlers;

	// What each chip shows (the C++ ShownChips), the lit chip as last drawn (ShownActiveChip), whether the chips are shown (for the tests: a script test cannot read a widget).
	TArray<FString> Shown;
	int ShownActive = -1;
	bool bChipsShown = false;

	void Build(UHOLScriptScreen InScreen, UHOLSelectionPanel InOwner)
	{
		Screen = InScreen;
		Owner = InOwner;
		Chips = Cast<UWrapBox>(Screen.ConstructWidget(UWrapBox, n"SelectionChips"));
		Chips.SetInnerSlotPadding(FVector2D(HOLSelectionChipsRules::ChipGap(), HOLSelectionChipsRules::ChipGap()));
		Chips.SetExplicitWrapSize(true);
		Chips.SetWrapSize(HOLSelectionChipsRules::WrapSize());
		Chips.SetVisibility(ESlateVisibility::Collapsed);
		Owner.ChipsHost.AddChildToVerticalBox(Chips).SetPadding(FMargin(0.0, 0.0, 0.0, HOLSelectionChipsRules::ChipsBottomPadding()));

		for (int Chip = 0; Chip < HOLSelectionChipsRules::NumChips(); Chip++)
		{
			UBorder Frame = Cast<UBorder>(Screen.ConstructWidget(UBorder, HOLSelectionChipsRules::FrameName(Chip)));
			FSlateBrush Brush;
			Brush.DrawAs = ESlateBrushDrawType::RoundedBox;
			Brush.TintColor = FSlateColor(FLinearColor(1.0, 1.0, 1.0, 1.0));
			Brush.OutlineSettings.RoundingType = ESlateBrushRoundingType::FixedRadius;
			Brush.OutlineSettings.CornerRadii = FVector4(HOLSelectionChipsRules::FrameRadius(), HOLSelectionChipsRules::FrameRadius(), HOLSelectionChipsRules::FrameRadius(), HOLSelectionChipsRules::FrameRadius());
			Frame.SetBrush(Brush);
			Frame.SetBrushColor(FLinearColor(0.0, 0.0, 0.0, 0.0));
			Frame.SetPadding(FMargin(HOLSelectionChipsRules::FramePadding()));
			Frame.SetVisibility(ESlateVisibility::Collapsed);

			USizeBox Box = Cast<USizeBox>(Screen.ConstructWidget(USizeBox, FName(f"SelectionChipBox{Chip}")));
			Box.SetHeightOverride(HOLSelectionChipsRules::ChipHeight());
			Box.SetMaxDesiredWidth(HOLSelectionChipsRules::ChipMaxWidth() - 2.0 * HOLSelectionChipsRules::FramePadding());
			Box.SetClipping(EWidgetClipping::ClipToBounds);
			UHOLQueueItem Item = Cast<UHOLQueueItem>(Screen.ConstructWidget(UHOLQueueItem, HOLSelectionChipsRules::ChipName(Chip)));
			Box.SetContent(Item);
			Frame.SetContent(Box);
			Chips.AddChildToWrapBox(Frame);

			UHOLSelectionChipHandler Handler = Cast<UHOLSelectionChipHandler>(NewObject(this, UHOLSelectionChipHandler));
			Handler.Panel = this;
			Handler.Chip = Chip;
			Item.OnButtonBaseClicked.AddUFunction(Handler, n"Clicked");

			Shown.Add("");
			Frames.Add(Frame);
			Items.Add(Item);
			Handlers.Add(Handler);
		}
	}

	// The click, mouse or hol.HUD.Click (ClickName): the controller cycles its subgroup to the chip's and writes the journal line ("HUD: chip N clicked", Display, in C++).
	bool ClickChip(int Chip)
	{
		if (Screen == nullptr || !HOLSelectionChipsRules::ClickAccepted(Chip))
			return false;
		const bool bDone = HOLSelectionChipsRead::ClickChip(Screen.GetOwningPlayer(), Chip);
		if (bDone)
			Refresh();
		return bDone;
	}

	// hol.HUD.Click's names: SelectionChip0 to SelectionChip11 (true: the click was taken).
	bool ClickName(FName WidgetName)
	{
		for (int Chip = 0; Chip < Items.Num(); Chip++)
		{
			if (HOLSelectionChipsRules::ChipName(Chip) == WidgetName)
				return ClickChip(Chip);
		}
		return false;
	}

	private void SetShown(UWidget Widget, bool bShown, ESlateVisibility ShownVisibility)
	{
		const ESlateVisibility Wanted = bShown ? ShownVisibility : ESlateVisibility::Collapsed;
		if (Widget != nullptr && Widget.GetVisibility() != Wanted)
			Widget.SetVisibility(Wanted);
	}

	// The group of the player's buildings (RefreshBuildingGroup): its report string (the lot A's ShownSelection, by SetShownKind, word for word), "Vous", the colour of the
	// first building, the sites. The health row stays hidden (a group has none).
	private void RefreshBuildingGroup()
	{
		FString Report;
		FText OwnerText;
		FLinearColor Colour;
		FString ActivityShown;
		FText ActivityText;
		if (!HOLSelectionChipsRead::GetBuildingGroupView(Screen.GetOwningPlayer(), Report, OwnerText, Colour, ActivityShown, ActivityText))
			return;
		Owner.SetHealth(FText::FromString(""), false);
		Owner.SetShownHealth("");
		Owner.SetShownKind(Report);
		Owner.SetOwner(OwnerText, Colour, true);
		Owner.SetActivity(ActivityText, ActivityShown != "");
		Owner.SetShownActivity(ActivityShown);
	}

	// RefreshChips of the C++ screen, each frame the selection is read: the chips' texts written when they changed, the active frame lit, the name hidden.
	void Refresh()
	{
		if (Screen == nullptr || Owner == nullptr || Chips == nullptr)
			return;
		RefreshBuildingGroup();

		TArray<FText> Labels;
		int Active = -1;
		HOLSelectionChipsRead::GetChipLabels(Screen.GetOwningPlayer(), Labels, Active);
		TArray<FString> Strings;
		for (int Index = 0; Index < Labels.Num(); Index++)
			Strings.Add(Labels[Index].ToString());

		bChipsShown = HOLSelectionChipsRules::ChipsShown(Strings);
		SetShown(Owner.ChipsHost, bChipsShown, ESlateVisibility::SelfHitTestInvisible);
		SetShown(Chips, bChipsShown, ESlateVisibility::SelfHitTestInvisible);
		if (!HOLSelectionChipsRules::NameShown(bChipsShown))
			Owner.SetName(FText::FromString(""), false);

		for (int Chip = 0; Chip < Items.Num(); Chip++)
		{
			const FString Now = HOLSelectionChipsRules::ShownLabel(Strings, Chip);
			if (HOLSelectionChipsRules::LabelChanged(Shown[Chip], Now))
			{
				Shown[Chip] = Now;
				Items[Chip].SetLabel(Chip < Labels.Num() ? Labels[Chip] : FText::FromString(""));
				SetShown(Frames[Chip], HOLSelectionChipsRules::FrameShown(Now), ESlateVisibility::SelfHitTestInvisible);
			}
			if (HOLSelectionChipsRules::FrameLit(ShownActive, Chip) != HOLSelectionChipsRules::FrameLit(Active, Chip))
			{
				const bool bLit = HOLSelectionChipsRules::FrameLit(Active, Chip);
				Frames[Chip].SetBrushColor(bLit ? UHOLScriptScreen::GetAccentColor() : FLinearColor(0.0, 0.0, 0.0, 0.0));
			}
		}
		ShownActive = Active;
	}
}
