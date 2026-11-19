// HUD H-11b, the TEST SCENARIO's panel (hol.Scenario; note / waitkey / panel hide|show of Tools\Scenarios), a development tool: NEW FILE, NOT HOOKED, NOT COMPILED.
// Port of Source\HeroesOfLibertyUI\HUD\HOLScenarioPanel.cpp (the C++ stays until the switch). It cannot compile alone: it needs the C++ bindings below.
// C++ bindings to add, members of UHOLScriptScreen (same pattern as HOLTopBar.as / HOLAlertBanner.as), written by the C++ lot:
//   bool GetScenarioDisplay(FString& OutName, int& OutState, FString& OutError, TArray<FString>& OutNotes, FString& OutWaitingForKey,
//                           bool& bOutClosed, bool& bOutHidden) const;   // copy of HOLTestHooks::GetScenarioDisplay(); OutState 0 Preparing 1 Ready 2 Failed; false = no display at all
//   void CloseScenarioPanel();                                           // HOLTestHooks::GetScenarioDisplay().bClosed = true
//   float32 MeasureHeadingWidth(FText A, FText B);                       // max of the two widths in UHOLTextStyleHUDHeading's font (0 without a renderer): the state's width
// Restart: System::ExecuteConsoleCommand("hol.Scenario <Name>") (to verify; else a binding `void ExecConsole(FString)` = GEngine->Exec(GetWorld(), ...)).
// The scenario's state (HOLTestHooks::FScenarioDisplay, written by HOLScenario.cpp / HOLTestSequence.cpp) STAYS C++ (D839: a rule with one source, written by
// the game module that the interface module does not see); the script is the VIEW, read each frame (the C++ hook OnScenarioChanged may stay as a no-op).
// Journal lines, word for word, LEVEL INCLUDED (UHOLGameMenuScreen::MenuLog writes "LogHOLUI: Display: <line>", lesson 1 of LECONS-HUD-1119):
//   "Scenario panel: <Describe>"  (read by Tools\Scenarios\LaunchCheck.ps1:86 'Scenario panel: scenario \S+ failed')
//   "HUD: scenario panel closed"   "HUD: scenario <Name> restarted"

namespace HOLScenarioPanelRules
{
	// States as ints (a constant of a file is not visible from another one).
	int StatePreparing() { return 0; }
	int StateReady() { return 1; }
	int StateFailed() { return 2; }

	// The C++ constants (namespace UE::HeroesOfLiberty::UI::ScenarioPanel).
	float ScreenMargin() { return 16.0; }
	float AnchorGap() { return 6.0; }
	float MaxWidth() { return 860.0; }
	float CloseSize() { return 29.0; }
	float RestartMinWidth() { return 96.0; }
	float RestartGap() { return 6.0; }
	float NoteGap() { return 4.0; }
	FLinearColor WaitKeyColor() { return FLinearColor(0.35, 0.85, 0.95, 1.0); }

	// Refresh: shown only with a scenario name, not closed, not hidden by a `panel hide` step.
	bool IsShown(FString Name, bool bClosed, bool bHidden)
	{
		return !Name.IsEmpty() && !bClosed && !bHidden;
	}

	// The texts: the same namespace and keys as the C++ NSLOCTEXT (the translations of Game.locres are looked up by them).
	FText TitleText(FString Name)
	{
		return FText::Format(NSLOCTEXT("HOLScenarioPanel", "Title", "Scénario {0}"), FText::FromString(Name));
	}
	FText PreparingText() { return NSLOCTEXT("HOLScenarioPanel", "Preparing", "en préparation…"); }
	FText ReadyText() { return NSLOCTEXT("HOLScenarioPanel", "Ready", "prêt"); }
	FText FailedText(FString Error)
	{
		return FText::Format(NSLOCTEXT("HOLScenarioPanel", "Failed", "erreur : {0}"), FText::FromString(Error));
	}
	FText RestartLabel() { return NSLOCTEXT("HOLScenarioPanel", "Restart", "Relancer"); }
	FString CloseLabel() { return "×"; }

	FText StateText(int State, FString Error)
	{
		if (State == StateFailed())
			return FailedText(Error);
		if (State == StateReady())
			return ReadyText();
		return PreparingText();
	}

	// One line while preparing or ready; only an error (long text) wraps (recette 2026-11-13).
	bool StateWraps(int State)
	{
		return State == StateFailed();
	}

	// The notes are rebuilt only when they change (the C++ ShownNotes != Display.Notes).
	bool NotesEqual(TArray<FString> A, TArray<FString> B)
	{
		if (A.Num() != B.Num())
			return false;
		for (int Index = 0; Index < A.Num(); Index++)
		{
			if (A[Index] != B[Index])
				return false;
		}
		return true;
	}
	bool NotesColumnVisible(int Count) { return Count > 0; }
	bool WaitKeyVisible(FString Text) { return !Text.IsEmpty(); }

	// NativeTick: under the top bar (visible, its desired height) a gap below; the margin alone without it.
	float PanelTop(float AnchorHeight)
	{
		return AnchorHeight > 0.0 ? ScreenMargin() + AnchorHeight + AnchorGap() : ScreenMargin();
	}

	// Describe(), word for word (a line of hol.HUD.Report / the journal line).
	FString Describe(FString Name, bool bClosed, int State, int ShownNoteCount, FString WaitingForKey)
	{
		if (Name.IsEmpty())
			return "no scenario";
		if (bClosed)
			return f"scenario {Name} closed";
		FString StateName = "preparing";
		if (State == StateReady())
			StateName = "ready";
		else if (State == StateFailed())
			StateName = "failed";
		FString Waiting = WaitingForKey.IsEmpty() ? "" : ", waiting for the key";
		return f"scenario {Name} {StateName}, {ShownNoteCount} notes{Waiting}";
	}
	FString RefreshLogLine(FString Described) { return "Scenario panel: " + Described; }
	FString ClosedLogLine() { return "HUD: scenario panel closed"; }
	FString RestartedLogLine(FString Name) { return f"HUD: scenario {Name} restarted"; }
	FString RestartCommand(FString Name) { return "hol.Scenario " + Name; }

	// ClickWidget: 0 refused, 1 restart, 2 close; refused while the panel is collapsed or for another name.
	int ClickKind(bool bPanelShown, FName Name)
	{
		if (!bPanelShown)
			return 0;
		if (Name == n"ScenarioRestart")
			return 1;
		if (Name == n"ScenarioClose")
			return 2;
		return 0;
	}
}

class UHOLScenarioPanelScript : UObject
{
	UHOLScriptScreen Screen;
	UBorder ScenarioPanel;
	UOverlaySlot WidthSlot;
	UCommonTextBlock Title;
	UCommonTextBlock State;
	UVerticalBox NoteColumn;
	UCommonTextBlock WaitKeyLine;
	UWidget Anchor;

	// What is drawn, for the report and the tests (a script test cannot read a widget).
	FString ShownName = "";
	bool bShown = false;
	TArray<FString> ShownNotes;
	float PlacedAnchorHeight = -1.0;
	FString LastLine = "";

	UHOLQueueItem MakeButton(FName Name, FText Label)
	{
		UHOLQueueItem Button = Cast<UHOLQueueItem>(Screen.ConstructWidget(UHOLQueueItem, Name));
		Button.SetLabel(Label);
		Button.OnButtonBaseClicked.AddUFunction(this, FName(Name.ToString() + "Clicked"));
		return Button;
	}

	// Frame: the game screen's overlay; InAnchor: the top bar's widget (GetTopBar). Same widget names as the C++ (hol.HUD.Click, hol.Mouse.Hover look them up).
	void Build(UHOLScriptScreen InScreen, UOverlay Frame, UWidget InAnchor)
	{
		Screen = InScreen;
		Anchor = InAnchor;
		USizeBox WidthBox = Cast<USizeBox>(Screen.ConstructWidget(USizeBox, n"ScenarioWidth"));
		WidthBox.SetMaxDesiredWidth(HOLScenarioPanelRules::MaxWidth());
		WidthBox.SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		WidthSlot = Frame.AddChildToOverlay(WidthBox);
		WidthSlot.SetHorizontalAlignment(EHorizontalAlignment::HAlign_Center);
		WidthSlot.SetVerticalAlignment(EVerticalAlignment::VAlign_Top);
		WidthSlot.SetPadding(FMargin(HOLScenarioPanelRules::ScreenMargin()));

		// Screen.MakePanel: the HUD panel brush (HOLUIStyle::HUDPanel, rounded 4); in the HUD rects as the C++ panel's "scenario" box.
		ScenarioPanel = Screen.MakePanel(n"ScenarioPanel");
		ScenarioPanel.SetPadding(FMargin(14.0, 8.0, 8.0, 12.0));
		ScenarioPanel.SetVisibility(ESlateVisibility::Collapsed);
		WidthBox.SetContent(ScenarioPanel);

		UVerticalBox Column = Cast<UVerticalBox>(Screen.ConstructWidget(UVerticalBox, n"ScenarioColumn"));
		Column.SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		ScenarioPanel.SetContent(Column);

		UHorizontalBox Header = Cast<UHorizontalBox>(Screen.ConstructWidget(UHorizontalBox, n"ScenarioHeader"));
		Header.SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		Column.AddChildToVerticalBox(Header);

		Title = Screen.MakeText(n"ScenarioTitle", UHOLTextStyleHUDHeading);
		Header.AddChildToHorizontalBox(Title).SetVerticalAlignment(EVerticalAlignment::VAlign_Center);

		State = Screen.MakeText(n"ScenarioState", UHOLTextStyleHUDHeading);
		UHorizontalBoxSlot StateSlot = Header.AddChildToHorizontalBox(State);
		StateSlot.SetSize(FSlateChildSize(ESlateSizeRule::Fill));
		StateSlot.SetVerticalAlignment(EVerticalAlignment::VAlign_Center);
		StateSlot.SetPadding(FMargin(10.0, 0.0, 16.0, 0.0));

		USizeBox RestartBox = Cast<USizeBox>(Screen.ConstructWidget(USizeBox, n"ScenarioRestartBox"));
		RestartBox.SetMinDesiredWidth(HOLScenarioPanelRules::RestartMinWidth());
		RestartBox.SetHeightOverride(HOLScenarioPanelRules::CloseSize());
		RestartBox.SetContent(MakeButton(n"ScenarioRestart", HOLScenarioPanelRules::RestartLabel()));
		UHorizontalBoxSlot RestartSlot = Header.AddChildToHorizontalBox(RestartBox);
		RestartSlot.SetVerticalAlignment(EVerticalAlignment::VAlign_Top);
		RestartSlot.SetPadding(FMargin(0.0, 0.0, HOLScenarioPanelRules::RestartGap(), 0.0));

		USizeBox CloseBox = Cast<USizeBox>(Screen.ConstructWidget(USizeBox, n"ScenarioCloseBox"));
		CloseBox.SetWidthOverride(HOLScenarioPanelRules::CloseSize());
		CloseBox.SetHeightOverride(HOLScenarioPanelRules::CloseSize());
		CloseBox.SetContent(MakeButton(n"ScenarioClose", FText::FromString(HOLScenarioPanelRules::CloseLabel())));
		Header.AddChildToHorizontalBox(CloseBox).SetVerticalAlignment(EVerticalAlignment::VAlign_Top);

		NoteColumn = Cast<UVerticalBox>(Screen.ConstructWidget(UVerticalBox, n"ScenarioNotes"));
		NoteColumn.SetVisibility(ESlateVisibility::Collapsed);
		Column.AddChildToVerticalBox(NoteColumn).SetPadding(FMargin(0.0, 4.0, 0.0, 0.0));

		WaitKeyLine = Screen.MakeText(n"ScenarioWaitKey", UHOLTextStyleHUDHeading);
		WaitKeyLine.SetAutoWrapText(true);
		WaitKeyLine.SetColorAndOpacity(HOLScenarioPanelRules::WaitKeyColor());
		WaitKeyLine.SetVisibility(ESlateVisibility::Collapsed);
		Column.AddChildToVerticalBox(WaitKeyLine).SetPadding(FMargin(0.0, 8.0, 0.0, 0.0));
	}

	// Each frame (NativeTick + Refresh of the C++): the C++ display read once, the panel drawn from it.
	void Refresh()
	{
		FString Name = "";
		int StateValue = 0;
		FString Error = "";
		TArray<FString> Notes;
		FString WaitingForKey = "";
		bool bClosed = false;
		bool bHidden = false;
		Screen.GetScenarioDisplay(Name, StateValue, Error, Notes, WaitingForKey, bClosed, bHidden);
		PlaceUnderAnchor();
		const bool bNowShown = HOLScenarioPanelRules::IsShown(Name, bClosed, bHidden);
		// The C++ wrote its journal line on every Refresh (a hook call, not a frame): the script writes it when what it shows changes.
		const bool bChanged = bNowShown != bShown || Name != ShownName || !HOLScenarioPanelRules::NotesEqual(ShownNotes, Notes) || StateValue != DrawnState || WaitingForKey != DrawnWait;
		bShown = bNowShown;
		ScenarioPanel.SetVisibility(bNowShown ? ESlateVisibility::Visible : ESlateVisibility::Collapsed);
		if (!bNowShown)
		{
			ShownName = Name;
			return;
		}
		ShownName = Name;
		DrawnState = StateValue;
		DrawnWait = WaitingForKey;
		Title.SetText(HOLScenarioPanelRules::TitleText(Name));
		State.SetText(HOLScenarioPanelRules::StateText(StateValue, Error));
		if (StateValue == HOLScenarioPanelRules::StatePreparing())
			State.SetColorAndOpacity(HOLUIStyle::TextMuted());
		else if (StateValue == HOLScenarioPanelRules::StateReady())
			State.SetColorAndOpacity(HOLUIStyle::Accent());
		else
			State.SetColorAndOpacity(HOLUIStyle::Missing());
		const bool bWraps = HOLScenarioPanelRules::StateWraps(StateValue);
		State.SetAutoWrapText(bWraps);
		State.SetMinDesiredWidth(bWraps ? 0.0 : Screen.MeasureHeadingWidth(HOLScenarioPanelRules::PreparingText(), HOLScenarioPanelRules::ReadyText()));
		if (!HOLScenarioPanelRules::NotesEqual(ShownNotes, Notes))
		{
			ShownNotes = Notes;
			NoteColumn.ClearChildren();
			for (int Index = 0; Index < ShownNotes.Num(); Index++)
			{
				UCommonTextBlock Note = Cast<UCommonTextBlock>(Screen.ConstructWidget(UCommonTextBlock, NAME_None));
				Note.SetStyle(UHOLTextStyleBody);
				Note.SetAutoWrapText(true);
				Note.SetText(FText::FromString(ShownNotes[Index]));
				NoteColumn.AddChildToVerticalBox(Note).SetPadding(FMargin(0.0, HOLScenarioPanelRules::NoteGap(), 0.0, 0.0));
			}
			NoteColumn.SetVisibility(HOLScenarioPanelRules::NotesColumnVisible(ShownNotes.Num()) ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
		}
		WaitKeyLine.SetText(FText::FromString(WaitingForKey));
		WaitKeyLine.SetVisibility(HOLScenarioPanelRules::WaitKeyVisible(WaitingForKey) ? ESlateVisibility::SelfHitTestInvisible : ESlateVisibility::Collapsed);
		if (bChanged)
		{
			LastLine = HOLScenarioPanelRules::RefreshLogLine(HOLScenarioPanelRules::Describe(Name, bClosed, StateValue, ShownNotes.Num(), WaitingForKey));
			UHOLGameMenuScreen::MenuLog(LastLine);
		}
	}

	int DrawnState = -1;
	FString DrawnWait = "";

	void PlaceUnderAnchor()
	{
		float AnchorHeight = 0.0;
		if (Anchor != nullptr && Anchor.GetVisibility() != ESlateVisibility::Collapsed)
			AnchorHeight = Anchor.GetDesiredSize().Y;
		if (AnchorHeight != PlacedAnchorHeight)
		{
			PlacedAnchorHeight = AnchorHeight;
			const float Top = HOLScenarioPanelRules::PanelTop(AnchorHeight);
			const float Margin = HOLScenarioPanelRules::ScreenMargin();
			WidthSlot.SetPadding(FMargin(Margin, Top, Margin, 0.0));
		}
	}

	UFUNCTION()
	void ScenarioRestartClicked(UCommonButtonBase Button)
	{
		Restart();
	}

	UFUNCTION()
	void ScenarioCloseClicked(UCommonButtonBase Button)
	{
		Close();
	}

	void Close()
	{
		Screen.CloseScenarioPanel();
		Refresh();
		UHOLGameMenuScreen::MenuLog(HOLScenarioPanelRules::ClosedLogLine());
	}

	// The same path as the console's hol.Scenario <Name>: the steps are read from the file again; the map is opened again by a later step.
	void Restart()
	{
		if (ShownName.IsEmpty())
			return;
		UHOLGameMenuScreen::MenuLog(HOLScenarioPanelRules::RestartedLogLine(ShownName));
		System::ExecuteConsoleCommand(HOLScenarioPanelRules::RestartCommand(ShownName));
	}

	// hol.HUD.Click ScenarioRestart | ScenarioClose (the screen's ClickWidget hook): true when it took it.
	bool ClickName(FName Name)
	{
		const int Kind = HOLScenarioPanelRules::ClickKind(bShown, Name);
		if (Kind == 1)
			Restart();
		else if (Kind == 2)
			Close();
		return Kind != 0;
	}
}
