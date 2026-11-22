// NOT COMPILED YET (GALADRIEL-1119m, H-11a of the HUD plan, NEW FILE, nothing switched): the development cheats' panel in script. It calls bindings that do not exist before the next
// C++ lot (HOLCheatSettings::GetSavedOpen / SetSavedOpen / GetWindowHeight / IsServerLocal / GetServerBuildTime / GetServerSpeed, UHOLScriptScreen::SetCheatsReport); the recipe with the exact
// signatures is in Docs\Rapports\GALADRIEL-1119m.md. Until then the script module would fail to compile with this file present: do NOT build with it without the lot (same warning as HOLTopBar.as).
//
// Same panel as HOLCheatPanel.cpp: a toggle ("Triches  +" / "Triches  -"), under it (collapsed while folded) seven buttons CheatFood, CheatWood, CheatStone, CheatGold, CheatAll,
// CheatPopulation, CheatFastBuild, then two speed sliders (CheatUnitSpeed, CheatGatherSpeed) each under its label (CheatUnitSpeedLabel, CheatGatherSpeedLabel). A click is the SAME
// command as the console one (hol.Cheat.Food 1000 ...), planned by HOLControllerCommands::PlanOf and sent by HOLControllerCommands::Send (the script RPC route, D845), exactly what
// AHOLPlayerController::RequestCheat<X> does through the same route today. The rules are pure (namespace HOLCheatPanelRules); the view only builds widgets and calls them.

namespace HOLCheatPanelText
{
	// Same namespace and keys as the C++ NSLOCTEXT of HOLCheatPanel.cpp ("HOLCheatPanel,<key>"): the English and Japanese tables (fill_po.py) stay valid.
	FText CheatLabel(int Index)
	{
		if (Index == 0)
			return NSLOCTEXT("HOLCheatPanel", "CheatFood", "+1000 Nourriture");
		if (Index == 1)
			return NSLOCTEXT("HOLCheatPanel", "CheatWood", "+1000 Bois");
		if (Index == 2)
			return NSLOCTEXT("HOLCheatPanel", "CheatStone", "+1000 Pierre");
		if (Index == 3)
			return NSLOCTEXT("HOLCheatPanel", "CheatGold", "+1000 Or");
		if (Index == 4)
			return NSLOCTEXT("HOLCheatPanel", "CheatAll", "+1000 de tout");
		if (Index == 5)
			return NSLOCTEXT("HOLCheatPanel", "CheatPopulation", "Population +165");
		return FText::FromString("");
	}

	FText ToggleOpen() { return NSLOCTEXT("HOLCheatPanel", "ToggleOpen", "Triches  -"); }
	FText ToggleFolded() { return NSLOCTEXT("HOLCheatPanel", "ToggleFolded", "Triches  +"); }
	FText FastBuildOn() { return NSLOCTEXT("HOLCheatPanel", "FastBuildOn", "Construction rapide : oui"); }
	FText FastBuildOff() { return NSLOCTEXT("HOLCheatPanel", "FastBuildOff", "Construction rapide : non"); }
	FText UnitSpeed() { return NSLOCTEXT("HOLCheatPanel", "UnitSpeed", "Vitesse des unités"); }
	FText GatherSpeed() { return NSLOCTEXT("HOLCheatPanel", "GatherSpeed", "Vitesse de collecte"); }
	FText SpeedLabel() { return NSLOCTEXT("HOLCheatPanel", "SpeedLabel", "{0} : x{1}"); }
}

namespace HOLCheatPanelRules
{
	// The C++ constants (HOLCheatPanel.cpp:30-45), as functions (a constant of a namespace is not visible from another file).
	float ScreenMargin() { return 16.0; }
	float AnchorGap() { return 6.0; }
	float ButtonWidth() { return 190.0; }
	float ButtonHeight() { return 29.0; }
	float ButtonGap() { return 3.0; }
	float SliderHeight() { return 20.0; }
	float MinSpeed() { return 0.5; }
	float MaxSpeed() { return 5.0; }
	float SpeedStep() { return 0.5; }
	int ResourceAmount() { return 1000; }
	int PopulationPlaces() { return 165; }
	int NumButtons() { return 7; }
	int FastBuildIndex() { return 6; }
	int UnitSpeedIndex() { return 0; }
	int GatherSpeedIndex() { return 1; }

	// The widget names, in the order of the C++ ECheat (Food, Wood, Stone, Gold, All, Population, FastBuild); the toggle is CheatToggle.
	FString ToggleName() { return "CheatToggle"; }

	FString ButtonName(int Index)
	{
		if (Index == 0)
			return "CheatFood";
		if (Index == 1)
			return "CheatWood";
		if (Index == 2)
			return "CheatStone";
		if (Index == 3)
			return "CheatGold";
		if (Index == 4)
			return "CheatAll";
		if (Index == 5)
			return "CheatPopulation";
		return "CheatFastBuild";
	}

	// The index of a button by its widget name (hol.HUD.Click's ClickWidget); -1 when it is not one of the seven.
	int IndexOfButton(const FString& Name)
	{
		for (int Index = 0; Index < NumButtons(); Index++)
		{
			if (ButtonName(Index) == Name)
				return Index;
		}
		return -1;
	}

	// The speed sliders' widget names and the server's variables they follow (SpeedSpecs of the C++), "unit speed" / "gather speed" in the report and the journal.
	FString SpeedName(int Index) { return Index == 0 ? "CheatUnitSpeed" : "CheatGatherSpeed"; }
	FString SpeedVariable(int Index) { return Index == 0 ? "hol.Units.SpeedScale" : "hol.Gather.SpeedScale"; }
	FString SpeedDescribed(int Index) { return Index == 0 ? "unit speed" : "gather speed"; }
	FString SpeedCommand(int Index) { return Index == 0 ? "hol.Cheat.UnitSpeed" : "hol.Cheat.GatherSpeed"; }

	// The command a button runs (the console command, so the plan and the route are the controller commands' own): hol.Cheat.Food 1000 ... hol.Cheat.FastBuild <0|1>.
	FString CommandOf(int Index)
	{
		if (Index == 0)
			return "hol.Cheat.Food";
		if (Index == 1)
			return "hol.Cheat.Wood";
		if (Index == 2)
			return "hol.Cheat.Stone";
		if (Index == 3)
			return "hol.Cheat.Gold";
		if (Index == 4)
			return "hol.Cheat.Resources";
		if (Index == 5)
			return "hol.Cheat.Population";
		return "hol.Cheat.FastBuild";
	}

	// The command's argument: 1000 for a resource, 165 places for the population, and for the fast build the state it is going to take (a click flips it).
	TArray<FString> ArgsOf(int Index, bool bFastBuildBefore)
	{
		TArray<FString> Args;
		if (Index == FastBuildIndex())
			Args.Add(bFastBuildBefore ? "0" : "1");
		else if (Index == 5)
			Args.Add(f"{PopulationPlaces()}");
		else
			Args.Add(f"{ResourceAmount()}");
		return Args;
	}

	// The journal lines, word for word the C++ ones; the view writes them with UHOLGameMenuScreen::MenuLog (level Display, "LogHOLUI: Display: <line>", lesson 1).
	FString ClickLine(int Index) { return "HUD: cheat " + ButtonName(Index) + " clicked"; }
	FString ToggleLine(bool bOpen) { return bOpen ? "HUD: cheats open" : "HUD: cheats folded"; }

	// "%.1f" of the C++ (one decimal, a point, in every language); the scales are positive.
	FString OneDecimal(double Value)
	{
		const int Tenths = Math::RoundToInt(Value * 10.0);
		return f"{Tenths / 10}.{Tenths % 10}";
	}

	FString FoldLine(double WindowHeight)
	{
		return f"HUD: cheats folded (window {Math::RoundToInt(WindowHeight)} px high, the hero's block opened)";
	}

	FString SpeedLine(int Index, double Scale)
	{
		return "HUD: cheat " + SpeedDescribed(Index) + " x" + OneDecimal(Scale);
	}

	// A slider moves by steps: the scale it asks for, snapped to the step and clamped to its range (HandleSpeedChanged of the C++).
	double SnapSpeed(double Value)
	{
		const double Snapped = Math::RoundToInt(Value / SpeedStep()) * SpeedStep();
		return Math::Clamp(Snapped, MinSpeed(), MaxSpeed());
	}

	// One request a step, not one a frame while it is dragged: only a changed scale is sent.
	bool SpeedChanged(double Current, double Value)
	{
		return SnapSpeed(Value) != Current;
	}

	// The toggle's, the fast build's and a speed's label.
	FText ToggleLabel(bool bOpen) { return bOpen ? HOLCheatPanelText::ToggleOpen() : HOLCheatPanelText::ToggleFolded(); }
	FText FastBuildLabel(bool bFastBuild) { return bFastBuild ? HOLCheatPanelText::FastBuildOn() : HOLCheatPanelText::FastBuildOff(); }

	FString SpeedLabelText(int Index, double Scale)
	{
		const FText Name = Index == 0 ? HOLCheatPanelText::UnitSpeed() : HOLCheatPanelText::GatherSpeed();
		return HOLCheatPanelText::SpeedLabel().ToString().Replace("{0}", Name.ToString()).Replace("{1}", OneDecimal(Scale));
	}

	// The texts the panel's toggle, fast build and two speed labels show, joined: a text rewritten only on a click kept its language after a live switch,
	// so the panel compares what it shows with what the current language would show (the texts are passed in: the rule does not read the language).
	FString LabelsSignature(const FString& Toggle, const FString& FastBuild, const FString& Unit, const FString& Gather)
	{
		return Toggle + "~" + FastBuild + "~" + Unit + "~" + Gather;
	}

	bool LabelsStale(const FString& Shown, const FString& Toggle, const FString& FastBuild, const FString& Unit, const FString& Gather)
	{
		return Shown != LabelsSignature(Toggle, FastBuild, Unit, Gather);
	}

	// The labels of the six resource / population buttons (everything before the fast build), joined, in the current language.
	FString CheatLabelsText()
	{
		FString Joined = "";
		for (int Index = 0; Index < FastBuildIndex(); Index++)
			Joined += HOLCheatPanelText::CheatLabel(Index).ToString() + "|";
		return Joined;
	}

	// The same with the six +1000 / population buttons' labels (Cheats = CheatLabelsText()): a button label is part of what the language changes.
	FString LabelsSignature(const FString& Toggle, const FString& FastBuild, const FString& Unit, const FString& Gather, const FString& Cheats)
	{
		return LabelsSignature(Toggle, FastBuild, Unit, Gather) + "~" + Cheats;
	}

	bool LabelsStale(const FString& Shown, const FString& Toggle, const FString& FastBuild, const FString& Unit, const FString& Gather, const FString& Cheats)
	{
		return Shown != LabelsSignature(Toggle, FastBuild, Unit, Gather, Cheats);
	}

	// A cheat button (not the toggle) is refused while the panel is folded (ClickWidget of the C++ returns false); the toggle always answers.
	bool ButtonClickable(bool bOpen) { return bOpen; }

	// Solo or listen server: what the panel shows follows the server's settings. The fast build is "on" when hol.Prod.BuildTime is above 0.
	bool FastBuildFollows(bool bServerLocal, double ServerBuildTime, bool bFastBuild)
	{
		return bServerLocal && ((ServerBuildTime > 0.0) != bFastBuild);
	}

	// A speed follows the server's variable when it differs from what the panel holds (a client has no variable to read: it keeps what it asked).
	bool SpeedFollows(bool bServerLocal, double ServerSpeed, double Held)
	{
		return bServerLocal && ServerSpeed != Held;
	}

	// hol.HUD.Report's "HUD cheats: <this>" line, word for word the C++ Describe.
	FString Describe(bool bOpen, bool bFastBuild, double UnitScale, double GatherScale)
	{
		if (!bOpen)
			return "cheats folded";
		return "cheats open, fast build " + (bFastBuild ? "on" : "off") + ", " + SpeedDescribed(0) + " x" + OneDecimal(UnitScale) + ", " + SpeedDescribed(1) + " x" + OneDecimal(GatherScale);
	}

	// The panel's place: under the anchor (the top bar) when it is shown, else at the margin; the bottom edge a block stacked under the cheats starts at.
	float TopOf(float AnchorHeight)
	{
		return AnchorHeight > 0.0 ? ScreenMargin() + AnchorHeight + AnchorGap() : ScreenMargin();
	}

	float BottomOf(float PlacedAnchorHeight, float PanelHeight, bool bHasPanel)
	{
		return bHasPanel ? TopOf(PlacedAnchorHeight) + PanelHeight : 0.0;
	}
}

// The cheats' panel. Built once in the script screen's OnInitialized (Build), refreshed each tick (Refresh): follows the server's settings, folds under the low-window rule, keeps its place.
class UHOLCheatPanelScript : UObject
{
	UHOLScriptScreen Screen;
	UWidget Anchor;

	UBorder Panel;
	UHOLQueueItem Toggle;
	UVerticalBox CheatColumn;
	TArray<UHOLQueueItem> CheatButtons;
	UCommonTextBlock UnitSpeedLabel;
	UCommonTextBlock GatherSpeedLabel;
	USlider UnitSpeedSlider;
	USlider GatherSpeedSlider;

	// What is shown, for the report and the tests.
	bool bOpen = false;
	bool bFastBuild = false;
	double UnitScale = 1.0;
	double GatherScale = 1.0;
	bool bHeroBlockShown = false;
	bool bWasLowWindow = false;
	bool bWasHeroBlockShown = false;
	float PlacedAnchorHeight = -1.0;

	UHOLQueueItem MakeButton(FName Name, UVerticalBox Box, float TopPadding)
	{
		USizeBox ButtonBox = Cast<USizeBox>(Screen.ConstructWidget(USizeBox, FName(Name.ToString() + "Box")));
		ButtonBox.SetWidthOverride(HOLCheatPanelRules::ButtonWidth());
		ButtonBox.SetHeightOverride(HOLCheatPanelRules::ButtonHeight());
		UHOLQueueItem Button = Cast<UHOLQueueItem>(Screen.ConstructWidget(UHOLQueueItem, Name));
		ButtonBox.SetContent(Button);
		UVerticalBoxSlot BoxSlot = Box.AddChildToVerticalBox(ButtonBox);
		BoxSlot.SetPadding(FMargin(0.0, TopPadding, 0.0, 0.0));
		return Button;
	}

	USlider MakeSlider(int Index, UCommonTextBlock& OutLabel)
	{
		const FString Name = HOLCheatPanelRules::SpeedName(Index);
		OutLabel = Screen.MakeText(FName(Name + "Label"), UHOLTextStyleHUDLabel);
		UVerticalBoxSlot LabelSlot = CheatColumn.AddChildToVerticalBox(OutLabel);
		LabelSlot.SetPadding(FMargin(2.0, HOLCheatPanelRules::ButtonGap() * 3.0, 0.0, 0.0));
		USlider Slider = Cast<USlider>(Screen.ConstructWidget(USlider, FName(Name)));
		Slider.SetMinValue(HOLCheatPanelRules::MinSpeed());
		Slider.SetMaxValue(HOLCheatPanelRules::MaxSpeed());
		Slider.SetStepSize(HOLCheatPanelRules::SpeedStep());
		Slider.SetValue(1.0);
		// HOLCheatPanel.cpp:474-475: muted bar, accent handle (the engine's default handle is white).
		Slider.SetSliderBarColor(UHOLScriptScreen::GetMutedColor());
		Slider.SetSliderHandleColor(UHOLScriptScreen::GetAccentColor());
		USizeBox SliderBox = Cast<USizeBox>(Screen.ConstructWidget(USizeBox, FName(Name + "Box")));
		SliderBox.SetWidthOverride(HOLCheatPanelRules::ButtonWidth());
		SliderBox.SetHeightOverride(HOLCheatPanelRules::SliderHeight());
		SliderBox.SetContent(Slider);
		CheatColumn.AddChildToVerticalBox(SliderBox);
		return Slider;
	}

	void Build(UHOLScriptScreen InScreen, UOverlay Frame, UWidget InAnchor)
	{
		Screen = InScreen;
		Anchor = InAnchor;
		Panel = Screen.MakePanel(n"CheatPanel");
		// HOLCheatPanel.cpp:425: inner margin 8 on every side (MakePanel's common 12 x 8 is left alone, other panels use it).
		Panel.SetPadding(FMargin(8.0));
		UOverlaySlot PanelSlot = Frame.AddChildToOverlay(Panel);
		PanelSlot.SetHorizontalAlignment(EHorizontalAlignment::HAlign_Left);
		PanelSlot.SetVerticalAlignment(EVerticalAlignment::VAlign_Top);
		PanelSlot.SetPadding(FMargin(HOLCheatPanelRules::ScreenMargin()));
		UVerticalBox Column = Cast<UVerticalBox>(Screen.ConstructWidget(UVerticalBox, n"CheatPanelColumn"));
		Panel.SetContent(Column);

		Toggle = MakeButton(n"CheatToggle", Column, 0.0);
		Toggle.OnButtonBaseClicked.AddUFunction(this, n"ToggleClicked");
		CheatColumn = Cast<UVerticalBox>(Screen.ConstructWidget(UVerticalBox, n"CheatColumn"));
		CheatColumn.SetVisibility(ESlateVisibility::Collapsed);
		UVerticalBoxSlot ColumnSlot = Column.AddChildToVerticalBox(CheatColumn);
		ColumnSlot.SetPadding(FMargin(0.0, HOLCheatPanelRules::ButtonGap() * 2.0, 0.0, 0.0));
		for (int Index = 0; Index < HOLCheatPanelRules::NumButtons(); Index++)
		{
			UHOLQueueItem Button = MakeButton(FName(HOLCheatPanelRules::ButtonName(Index)), CheatColumn, Index == 0 ? 0.0 : HOLCheatPanelRules::ButtonGap());
			Button.SetLabel(HOLCheatPanelText::CheatLabel(Index));
			Button.OnButtonBaseClicked.AddUFunction(this, FName("Cheat" + Index + "Clicked"));
			CheatButtons.Add(Button);
		}
		UnitSpeedSlider = MakeSlider(HOLCheatPanelRules::UnitSpeedIndex(), UnitSpeedLabel);
		UnitSpeedSlider.OnValueChanged.AddUFunction(this, n"UnitSpeedChanged");
		GatherSpeedSlider = MakeSlider(HOLCheatPanelRules::GatherSpeedIndex(), GatherSpeedLabel);
		GatherSpeedSlider.OnValueChanged.AddUFunction(this, n"GatherSpeedChanged");

		// A low window starts folded (Q-heros-triches-600 = A), without losing the kept choice.
		const bool bSaved = HOLCheatSettings::GetSavedOpen();
		SetOpen(HOLHUDLayoutRead::CheatsStartOpen(bSaved, HOLCheatSettings::GetWindowHeight()), false);
		FollowServerSettings();
		RefreshLabels();
	}

	// The seven buttons' handlers (a dynamic delegate has no payload: one UFUNCTION a button, named "Cheat<i>Clicked").
	UFUNCTION()
	void Cheat0Clicked(UCommonButtonBase Button) { ClickButton(0); }
	UFUNCTION()
	void Cheat1Clicked(UCommonButtonBase Button) { ClickButton(1); }
	UFUNCTION()
	void Cheat2Clicked(UCommonButtonBase Button) { ClickButton(2); }
	UFUNCTION()
	void Cheat3Clicked(UCommonButtonBase Button) { ClickButton(3); }
	UFUNCTION()
	void Cheat4Clicked(UCommonButtonBase Button) { ClickButton(4); }
	UFUNCTION()
	void Cheat5Clicked(UCommonButtonBase Button) { ClickButton(5); }
	UFUNCTION()
	void Cheat6Clicked(UCommonButtonBase Button) { ClickButton(6); }

	UFUNCTION()
	void ToggleClicked(UCommonButtonBase Button)
	{
		ClickToggle();
	}

	UFUNCTION()
	void UnitSpeedChanged(float32 Value)
	{
		SpeedMoved(HOLCheatPanelRules::UnitSpeedIndex(), Value);
	}

	UFUNCTION()
	void GatherSpeedChanged(float32 Value)
	{
		SpeedMoved(HOLCheatPanelRules::GatherSpeedIndex(), Value);
	}

	void SetOpen(bool bInOpen, bool bKeep)
	{
		const bool bChanged = bKeep && bOpen != bInOpen;
		bOpen = bInOpen;
		if (CheatColumn != nullptr)
			CheatColumn.SetVisibility(bOpen ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
		RefreshLabels();
		if (bChanged)
			HOLCheatSettings::SetSavedOpen(bOpen);
	}

	// The toggle's click, mouse or hol.HUD.Click.
	bool ClickToggle()
	{
		SetOpen(!bOpen, true);
		UHOLGameMenuScreen::MenuLog(HOLCheatPanelRules::ToggleLine(bOpen));
		return true;
	}

	// A cheat button's click: same command as the console one, planned and sent by the controller commands (the only route).
	void ClickButton(int Index)
	{
		AHOLPlayerController Controller = Cast<AHOLPlayerController>(Gameplay::GetPlayerController(0));
		if (Controller == nullptr)
			return;
		UHOLGameMenuScreen::MenuLog(HOLCheatPanelRules::ClickLine(Index));
		const FHOLCheatPlan Plan = HOLControllerCommands::PlanOf(HOLCheatPanelRules::CommandOf(Index), HOLCheatPanelRules::ArgsOf(Index, bFastBuild));
		if (Plan.bValid)
			HOLControllerCommands::Send(Controller, Plan, FVector::ZeroVector); // The panel's commands (resources, population, fast build, speeds) need no point.
		if (Index == HOLCheatPanelRules::FastBuildIndex())
		{
			bFastBuild = !bFastBuild;
			RefreshLabels();
		}
	}

	// hol.HUD.Click's names: the toggle always answers, a cheat button only while open (false when folded away, as the C++).
	bool ClickName(FName WidgetName)
	{
		const FString Name = WidgetName.ToString();
		if (Panel == nullptr)
			return false;
		if (Name == HOLCheatPanelRules::ToggleName())
			return ClickToggle();
		const int Index = HOLCheatPanelRules::IndexOfButton(Name);
		if (Index < 0 || !HOLCheatPanelRules::ButtonClickable(bOpen))
			return false;
		ClickButton(Index);
		return true;
	}

	void SpeedMoved(int Index, double Value)
	{
		const double Held = Index == 0 ? UnitScale : GatherScale;
		if (!HOLCheatPanelRules::SpeedChanged(Held, Value))
			return;
		const double Scale = HOLCheatPanelRules::SnapSpeed(Value);
		if (Index == 0)
			UnitScale = Scale;
		else
			GatherScale = Scale;
		RefreshLabels();
		AHOLPlayerController Controller = Cast<AHOLPlayerController>(Gameplay::GetPlayerController(0));
		if (Controller == nullptr)
			return;
		UHOLGameMenuScreen::MenuLog(HOLCheatPanelRules::SpeedLine(Index, Scale));
		TArray<FString> Args;
		Args.Add(HOLCheatPanelRules::OneDecimal(Scale));
		const FHOLCheatPlan Plan = HOLControllerCommands::PlanOf(HOLCheatPanelRules::SpeedCommand(Index), Args);
		if (Plan.bValid)
			HOLControllerCommands::Send(Controller, Plan, FVector::ZeroVector); // The panel's commands (resources, population, fast build, speeds) need no point.
	}

	// The signature of the labels last written (empty before the first write), see HOLCheatPanelRules::LabelsStale.
	FString LabelsShown;

	FString LabelsNow()
	{
		return HOLCheatPanelRules::LabelsSignature(HOLCheatPanelRules::ToggleLabel(bOpen).ToString(), HOLCheatPanelRules::FastBuildLabel(bFastBuild).ToString(),
			HOLCheatPanelRules::SpeedLabelText(0, UnitScale), HOLCheatPanelRules::SpeedLabelText(1, GatherScale), HOLCheatPanelRules::CheatLabelsText());
	}

	// Called each Refresh: rewrites the labels when only the language changed (the stale rule), nothing otherwise.
	void RelabelOnLanguageChange()
	{
		if (Toggle == nullptr || UnitSpeedLabel == nullptr || GatherSpeedLabel == nullptr)
			return;
		if (LabelsShown != LabelsNow())
			RefreshLabels();
	}

	void RefreshLabels()
	{
		LabelsShown = LabelsNow();
		Toggle.SetLabel(HOLCheatPanelRules::ToggleLabel(bOpen));
		for (int Index = 0; Index < HOLCheatPanelRules::FastBuildIndex(); Index++)
			CheatButtons[Index].SetLabel(HOLCheatPanelText::CheatLabel(Index)); // the six +1000 / population buttons, in the current language
		CheatButtons[HOLCheatPanelRules::FastBuildIndex()].SetLabel(HOLCheatPanelRules::FastBuildLabel(bFastBuild));
		UnitSpeedLabel.SetText(FText::FromString(HOLCheatPanelRules::SpeedLabelText(0, UnitScale)));
		GatherSpeedLabel.SetText(FText::FromString(HOLCheatPanelRules::SpeedLabelText(1, GatherScale)));
	}

	// FollowServerSettings of the C++: solo / listen server only (a client never learns another player's change).
	void FollowServerSettings()
	{
		const bool bLocal = HOLCheatSettings::IsServerLocal();
		bool bChanged = false;
		if (HOLCheatPanelRules::FastBuildFollows(bLocal, HOLCheatSettings::GetServerBuildTime(), bFastBuild))
		{
			bFastBuild = !bFastBuild;
			bChanged = true;
		}
		for (int Index = 0; Index < 2; Index++)
		{
			const double Held = Index == 0 ? UnitScale : GatherScale;
			const double ServerSpeed = HOLCheatSettings::GetServerSpeed(Index);
			if (!HOLCheatPanelRules::SpeedFollows(bLocal, ServerSpeed, Held))
				continue;
			if (Index == 0)
			{
				UnitScale = ServerSpeed;
				UnitSpeedSlider.SetValue(ServerSpeed);
			}
			else
			{
				GatherScale = ServerSpeed;
				GatherSpeedSlider.SetValue(ServerSpeed);
			}
			bChanged = true;
		}
		if (bChanged)
			RefreshLabels();
	}

	// NativeTick of the C++, called from the script screen's Refresh: follows the server, folds in a low window when the hero's block opens (edge-triggered), keeps its place under the anchor.
	void Refresh(bool bInHeroBlockShown)
	{
		if (Panel == nullptr)
			return;
		bHeroBlockShown = bInHeroBlockShown;
		FollowServerSettings();
		RelabelOnLanguageChange();
		const float Height = HOLCheatSettings::GetWindowHeight();
		FHOLHUDFoldState Previous;
		Previous.bWasLow = bWasLowWindow;
		Previous.bWasShown = bWasHeroBlockShown;
		const FHOLHUDFoldState Now = HOLHUDLayoutRead::ShouldFoldCheats(Height, bHeroBlockShown, bOpen, Previous);
		bWasLowWindow = Now.bWasLow;
		bWasHeroBlockShown = Now.bWasShown;
		if (Now.bFold)
		{
			SetOpen(false, false);
			UHOLGameMenuScreen::MenuLog(HOLCheatPanelRules::FoldLine(Height));
		}
		const float AnchorHeight = (Anchor != nullptr && Anchor.GetVisibility() != ESlateVisibility::Collapsed) ? Anchor.GetDesiredSize().Y : 0.0;
		if (AnchorHeight != PlacedAnchorHeight)
		{
			PlacedAnchorHeight = AnchorHeight;
			UOverlaySlot PanelSlot = Cast<UOverlaySlot>(Panel.Slot);
			if (PanelSlot != nullptr)
				PanelSlot.SetPadding(FMargin(HOLCheatPanelRules::ScreenMargin(), HOLCheatPanelRules::TopOf(AnchorHeight), 0.0, 0.0));
		}
	}

	// The panel's bottom edge (what the AI level line is stacked under) and the report's line.
	float GetBottom() const
	{
		return HOLCheatPanelRules::BottomOf(PlacedAnchorHeight, Panel != nullptr ? Panel.GetDesiredSize().Y : 0.0, Panel != nullptr);
	}

	FString Describe() const
	{
		return HOLCheatPanelRules::Describe(bOpen, bFastBuild, UnitScale, GatherScale);
	}
}
