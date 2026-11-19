// NOT COMPILED, NOT LAUNCHED, NO HOOK (ELROND-1119e, HUD H-5, first half: the major alert's BANNER; the thread of messages is a later lot).
// THIS FILE CALLS C++ BINDINGS THAT DO NOT EXIST YET (recipe in Docs/Rapports/ELROND-1119e.md (a)): do not compile the script module with it before that lot.
// They are members of UHOLScriptScreen (same pattern as HOLTopBar.as):
//   bool GetAlertBanner(bool& bOutShown, int& OutSerial, int& OutSlot, float32& OutOpacity, bool& bOutSound, FString& OutText,
//                       TArray<FHOLForumPieceValue>& OutPieces, TArray<FLinearColor>& OutColors) const;
//   bool PlayAlertSound();                 // LoadObject(FHOLForumAlert::SoundPath) + PlaySound2D; true if the sound was found
//   void FocusCameraOnForum(int Slot);     // the owning controller's FocusCameraOnForum (journal line written by the script)
//
// The major alert's banner (V4.3, V4.5b; D202; the C++ screen's AlertPanel / AlertRow): top centre under the top bar (the wait banner's place),
// larger than the thread's lines (UHOLTextStyleHUDHeading for the words AND the names, each name in its player's colour); a click on it centres
// the camera on the touched forum. The STATE MACHINE (Show / Advance / Expire / opacity: 4 s shown, 0.5 s fade, 1.5 s minimum, 2 waiting, one sound
// per burst) STAYS C++ (FHOLForumAlert, D836: a rule with one source, shared with the tests and DescribeAlert); the script is the VIEW: it reads the
// state each frame and draws it. Nothing of the rules of HOLAlertRules.as is repeated: those are the minor alerts' (population, attack), which add
// lines to the thread, not banners.
// The journal lines are the C++ screen's, word for word, level included (UHOLGameMenuScreen::MenuLog writes "LogHOLUI: Display: <line>"):
//   "Alert: <text> (sound <path>)"  "Alert: <text> (sound <path>, missing)"  "Alert: <text> (no sound, its burst's played)"
//   "HUD: alert of slot <n> clicked (camera on its forum)"

namespace HOLAlertBanner
{
	// What a refresh does to the banner, from what is drawn so far and what the C++ state says. Functions, not constants (not visible from another file).
	int ChangeNone() { return 0; }
	int ChangePresent() { return 1; }
	int ChangeHide() { return 2; }
	int ChangeFade() { return 3; }

	// The C++ layout (BuildLayout): 16 px margin; the top = the margin + the queue's two rows (36 px each) = 88, the wait banner's place.
	float Margin() { return 16.0; }
	float Top() { return 88.0; }
	// FSlateRoundedBoxBrush(HOLUIStyle::HUDPanel.CopyWithNewOpacity(0.85f), 4.f); SetPadding(FMargin(16.f, 6.f)).
	float PanelOpacity() { return 0.85; }
	float PaddingX() { return 16.0; }
	float PaddingY() { return 6.0; }

	// FHOLForumAlert::SoundPath, the herald's voice's place until a real sound comes (provisional).
	FString SoundPath() { return "/Engine/EngineSounds/1kSineTonePing.1kSineTonePing"; }

	// Serial: the number of banners the C++ state has presented so far (0 never); ShownSerial: the one drawn (0 none); bVisible: the panel is up.
	// A banner is presented when the state shows one and its serial is new (the next waiting alert replaces the shown one with no hidden frame between);
	// hidden when the state shows none and the panel is up; otherwise only the opacity follows the fade.
	int Decide(bool bStateShown, int Serial, int ShownSerial, bool bVisible)
	{
		if (bStateShown)
			return Serial != ShownSerial ? ChangePresent() : ChangeFade();
		return bVisible ? ChangeHide() : ChangeNone();
	}

	// PresentAlert's journal line. bSoundFound: the sound asset loaded.
	FString PresentLogLine(FString Text, bool bSound, bool bSoundFound)
	{
		if (!bSound)
			return "Alert: " + Text + " (no sound, its burst's played)";
		return "Alert: " + Text + " (sound " + SoundPath() + (bSoundFound ? "" : ", missing") + ")";
	}

	// FocusAlertSlot's journal line.
	FString ClickLogLine(int Slot)
	{
		return f"HUD: alert of slot {Slot} clicked (camera on its forum)";
	}

	// HandleAlertMouseDown: only the left button, only while a banner is shown (the panel is Visible but a hidden one is Collapsed anyway).
	bool MouseClickAccepted(bool bStateShown, bool bLeftButton)
	{
		return bStateShown && bLeftButton;
	}

	// ClickWidget(AlertPanel) (hol.HUD.Click): refused (false) while no banner is shown.
	bool NamedClickAccepted(bool bStateShown)
	{
		return bStateShown;
	}

	// A piece of the message is a player's name (its slot is not INDEX_NONE = -1): drawn in that player's colour.
	bool PieceIsName(int Slot)
	{
		return Slot != -1;
	}

	// The report's line: stays the C++ screen's DescribeAlert() ("HUD alert: ..."), the C++ owns the state it prints.
}

class UHOLAlertBannerPanel : UObject
{
	UHOLScriptScreen Screen;
	UBorder AlertPanel;
	UHorizontalBox AlertRow;

	// What is drawn, for the report and the tests (a script test cannot read a widget).
	int ShownSerial = 0;
	bool bVisible = false;
	int ShownSlot = -1;
	FString LastLine = "";
	// How many pieces the row holds, and the last opacity set.
	int ShownPieceCount = 0;
	float ShownOpacity = 1.0;

	void Build(UHOLScriptScreen InScreen, UOverlay Frame)
	{
		Screen = InScreen;
		AlertPanel = Screen.MakePanel(n"AlertPanel");
		AlertPanel.SetBrushColor(FLinearColor(1.0, 1.0, 1.0, HOLAlertBanner::PanelOpacity()));
		AlertPanel.SetPadding(FMargin(HOLAlertBanner::PaddingX(), HOLAlertBanner::PaddingY()));
		AlertPanel.SetVisibility(ESlateVisibility::Collapsed);
		AlertPanel.OnMouseButtonDownEvent.BindUFunction(this, n"HandleAlertMouseDown");
		UOverlaySlot AlertSlot = Frame.AddChildToOverlay(AlertPanel);
		AlertSlot.SetHorizontalAlignment(EHorizontalAlignment::HAlign_Center);
		AlertSlot.SetVerticalAlignment(EVerticalAlignment::VAlign_Top);
		AlertSlot.SetPadding(FMargin(HOLAlertBanner::Margin(), HOLAlertBanner::Top(), HOLAlertBanner::Margin(), HOLAlertBanner::Margin()));
		AlertRow = Cast<UHorizontalBox>(Screen.ConstructWidget(UHorizontalBox, n"AlertRow"));
		AlertPanel.SetContent(AlertRow);
	}

	// Each frame (the fade is 0.5 s): the C++ state read once, the banner drawn from it.
	void Refresh()
	{
		bool bStateShown = false;
		int Serial = 0;
		int Slot = -1;
		float32 Opacity = 0.0;
		bool bSound = false;
		FString Text = "";
		TArray<FHOLForumPieceValue> Pieces;
		TArray<FLinearColor> Colors;
		Screen.GetAlertBanner(bStateShown, Serial, Slot, Opacity, bSound, Text, Pieces, Colors);
		const int Change = HOLAlertBanner::Decide(bStateShown, Serial, ShownSerial, bVisible);
		if (Change == HOLAlertBanner::ChangePresent())
		{
			FillRow(Pieces, Colors);
			ShownSerial = Serial;
			ShownSlot = Slot;
			bVisible = true;
			ShownOpacity = 1.0;
			AlertPanel.SetRenderOpacity(1.0);
			// Visible: a click on it centres the camera on the touched forum.
			AlertPanel.SetVisibility(ESlateVisibility::Visible);
			const bool bFound = bSound ? Screen.PlayAlertSound() : false;
			LastLine = HOLAlertBanner::PresentLogLine(Text, bSound, bFound);
			UHOLGameMenuScreen::MenuLog(LastLine);
		}
		else if (Change == HOLAlertBanner::ChangeHide())
		{
			bVisible = false;
			AlertPanel.SetVisibility(ESlateVisibility::Collapsed);
		}
		else if (Change == HOLAlertBanner::ChangeFade())
		{
			ShownOpacity = Opacity;
			AlertPanel.SetRenderOpacity(Opacity);
		}
	}

	// FillMessageRow: the words and the names in the heading style, each name in its player's colour (set after it joins the row).
	void FillRow(TArray<FHOLForumPieceValue> Pieces, TArray<FLinearColor> Colors)
	{
		AlertRow.ClearChildren();
		for (int Index = 0; Index < Pieces.Num(); Index++)
		{
			UCommonTextBlock Piece = Cast<UCommonTextBlock>(Screen.ConstructWidget(UCommonTextBlock, NAME_None));
			Piece.SetStyle(UHOLTextStyleHUDHeading);
			Piece.SetText(FText::FromString(Pieces[Index].Text));
			UHorizontalBoxSlot PieceSlot = AlertRow.AddChildToHorizontalBox(Piece);
			PieceSlot.SetVerticalAlignment(EVerticalAlignment::VAlign_Center);
			if (HOLAlertBanner::PieceIsName(Pieces[Index].Slot) && Index < Colors.Num())
				Piece.SetColorAndOpacity(Colors[Index]);
		}
		ShownPieceCount = Pieces.Num();
	}

	UFUNCTION()
	FEventReply HandleAlertMouseDown(FGeometry Geometry, const FPointerEvent& MouseEvent)
	{
		if (!HOLAlertBanner::MouseClickAccepted(bVisible, MouseEvent.GetEffectingButton() == EKeys::LeftMouseButton))
			return WidgetBlueprint::Unhandled();
		FocusShown();
		return WidgetBlueprint::Handled();
	}

	// hol.HUD.Click AlertPanel (the screen's ClickWidget hook): true when it took the click.
	bool ClickName(FName Name)
	{
		if (Name != n"AlertPanel")
			return false;
		if (!HOLAlertBanner::NamedClickAccepted(bVisible))
			return false;
		FocusShown();
		return true;
	}

	void FocusShown()
	{
		LastLine = HOLAlertBanner::ClickLogLine(ShownSlot);
		UHOLGameMenuScreen::MenuLog(LastLine);
		Screen.FocusCameraOnForum(ShownSlot);
	}
}
