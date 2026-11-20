// The short message of the HUD (D313) and the production held "population full" note (D829, D202), in script (migration step 4, panel H-2, D851).
// The C++ screen (HOLGameScreen.cpp ShowMessage / UpdateMessage / CheckPopulationBlocked, MessagePanel / MessageText) stays the one that KNOWS what a refused
// command says (GetFeedbackKey / GetFeedbackText keep the keys and the LOCTEXT texts); it hands the key and the text to the script (event ShortMessage),
// the script shows the text in its own MessagePanel, fades and hides it after HOLShortMessage::Seconds, and writes the journal lines the C++ wrote.
// Pure rules in the namespace (no world, tested by HOLShortMessage_Test.as); the view functions below take the screen and the parts built by Build.
// Hook to put in UHOLGameScreenScript: Docs/Rapports/GLORFINDEL-1118t.md (recipe a).

// What is on the screen: Active from Show until Seconds have gone by. As the C++ UpdateMessage (empty key: no clock), a message with an EMPTY key never ends (D839 fidelity, a question for the user).
struct FHOLShortMessageState
{
	bool bActive = false;
	// Names the message in the logs and in the Describe line ("NotEnoughWood", "PopulationFull"...).
	FString Key = "";
	// Seconds since Show.
	float Age = 0.0;
}

// What the population held rule remembers between two readings: the production held at 100 % was already said.
struct FHOLPopulationHeldWatch
{
	bool bWasHeld = false;
}

// The widgets Build made (the screen keeps them as members: MessagePanel, MessageText, MessageSlot).
struct FHOLShortMessageParts
{
	UBorder Panel;
	UCommonTextBlock Text;
	UOverlaySlot PanelSlot;
}

namespace HOLShortMessage
{
	// The C++ UHOLGameScreen::MessageSeconds / MessageFadeSeconds (HOLGameScreen.h:143-144).
	const float Seconds = 3.0;
	const float FadeSeconds = 0.5;
	// The C++ slot's bottom padding: 6 between the message and the queue (or the grid) under it (HOLGameScreen.cpp:3652).
	const float BottomGap = 6.0;
	// The key of the population full message (GetFeedbackKey, EHOLCommandFeedback::PopulationFull).
	const FString PopulationKey = "PopulationFull";
	// The C++ wrapped the text at GridWidth - 24 (332 - 24).
	const float WrapWidth = 308.0;

	// A new message always replaces the one on the screen and starts its clock again (the C++ ShowMessage overwrote MessageKey and set MessageAge 0): the latest wins.
	void Show(FHOLShortMessageState& State, FString Key)
	{
		State.bActive = true;
		State.Key = Key;
		State.Age = 0.0;
	}

	// The panel's opacity Age seconds after Show: full, then a fade over the last FadeSeconds (C++: Left < Fade ? Left / Fade : 1).
	float Opacity(float Age)
	{
		const float Left = Seconds - Age;
		return Left < FadeSeconds ? Math::Max(Left, 0.0) / FadeSeconds : 1.0;
	}

	// One step of DeltaSeconds: true while the message is still to be drawn (its opacity is then Opacity(State.Age)); false once Seconds are over
	// (the state is reset at once) or when there is none.
	bool Advance(FHOLShortMessageState& State, float DeltaSeconds)
	{
		if (!State.bActive)
			return false;
		// The C++ UpdateMessage returned at once for an empty key: no clock, no fade, no end. Same here (even if it looks like a flaw).
		if (State.Key.IsEmpty())
			return true;
		State.Age += DeltaSeconds;
		if (State.Age >= Seconds)
		{
			State.bActive = false;
			State.Key = "";
			return false;
		}
		return true;
	}

	// The key the Describe line (hol.HUD.Report, ", message <key>") shows: the key while the message lasts, nothing after.
	FString ShownKey(FHOLShortMessageState& State)
	{
		return State.bActive ? State.Key : "";
	}

	// True when the population held message and the thread line go now: the production has just become held (the head at 100 % with no place, D829).
	// Held again without a free moment in between stays quiet; once the production goes on (not held) and is held again, it says it again (C++ bPopulationBlocked).
	bool ShouldShowPopulationHeld(FHOLPopulationHeldWatch& Watch, bool bHeldNow)
	{
		const bool bGoes = bHeldNow && !Watch.bWasHeld;
		Watch.bWasHeld = bHeldNow;
		return bGoes;
	}

	// The journal line of a message, word for word the C++'s UE_LOG(LogHOLUI, Display, "HUD message: %s \"%s\"") (quick_check and the scenarios read it).
	FString MessageLogLine(FString Key, FString Text)
	{
		return "HUD message: " + Key + " \"" + Text + "\"";
	}

	// The journal line of the held production, word for word the C++'s ("100 %%" in the UE_LOG format is "100 %").
	FString HeldLogLine()
	{
		return "HUD: production held at 100 % (population full), line to the thread";
	}

	// The text of the population full message (the C++ GetFeedbackText: LOCTEXT("PopulationMax", ...) of HOLGameScreen.cpp, same namespace and key).
	FText PopulationMaxText()
	{
		return NSLOCTEXT("HOLGameScreen", "PopulationMax", "Population maximale — construisez des maisons");
	}

	// Where the panel's bottom sits: BottomGap over the top of the box under it (the queue's, or the grid's when no queue), in Slate units; ViewHeight is
	// the window's. Never negative.
	float BottomPadding(float ViewHeight, float AnchorTop)
	{
		return Math::Max(ViewHeight - AnchorTop + BottomGap, 0.0);
	}

	// The panel under which the message sits: the queue's when it is drawn, else the grid's. 0: none drawn (the message then sits at the window's bottom).
	// The room taken is the box's top; the left edge is the box's (the C++ column's left edge).
	bool FindAnchor(UHOLScriptScreen Screen, FVector2D& Low)
	{
		FVector2D High;
		if (Screen.GetGameScreenBox(n"QueuePanel", Low, High))
			return true;
		return Screen.GetGameScreenBox(n"GridPanel", Low, High);
	}

	// The same widgets as the C++ (MessagePanel, MessageText), hidden, in the script's frame; the panel lets the clicks through when shown (HitTestInvisible).
	FHOLShortMessageParts Build(UHOLScriptScreen Screen, UOverlay Frame)
	{
		FHOLShortMessageParts Parts;
		Parts.Panel = Screen.MakePanel(n"MessagePanel");
		Parts.Panel.SetVisibility(ESlateVisibility::Collapsed);
		Parts.PanelSlot = Frame.AddChildToOverlay(Parts.Panel);
		Parts.PanelSlot.SetHorizontalAlignment(EHorizontalAlignment::HAlign_Left);
		Parts.PanelSlot.SetVerticalAlignment(EVerticalAlignment::VAlign_Bottom);
		Parts.Text = Screen.MakeText(n"MessageText", UHOLTextStyleHUDAlert);
		Screen.SetTextWrapWidth(Parts.Text, WrapWidth);
		Parts.Panel.SetContent(Parts.Text);
		return Parts;
	}

	// Shows Text under Key: the panel at full opacity, the text, the clock at 0, the journal line (as the C++ ShowMessage).
	void ShowText(FHOLShortMessageParts& Parts, FHOLShortMessageState& State, FString Key, FText Text)
	{
		Show(State, Key);
		Parts.Text.SetText(Text);
		Parts.Panel.SetRenderOpacity(1.0);
		Parts.Panel.SetVisibility(ESlateVisibility::HitTestInvisible);
		UHOLGameMenuScreen::MenuLog(MessageLogLine(Key, Text.ToString()));
	}

	// One frame (or one refresh): the fade, the hiding after Seconds; true when the message has just ended (the screen then clears the key it mirrors to the C++).
	bool Tick(FHOLShortMessageParts& Parts, FHOLShortMessageState& State, float DeltaSeconds)
	{
		if (!State.bActive)
			return false;
		if (Advance(State, DeltaSeconds))
		{
			// No key: the C++ left the opacity as ShowMessage set it (1).
			if (!State.Key.IsEmpty())
				Parts.Panel.SetRenderOpacity(Opacity(State.Age));
			return false;
		}
		Parts.Panel.SetVisibility(ESlateVisibility::Collapsed);
		return true;
	}

	// The place of the panel while shown: over the queue or the grid, as the C++ column put it. Call from the screen's Refresh.
	void Place(UHOLScriptScreen Screen, FHOLShortMessageParts& Parts)
	{
		FVector2D Low;
		const FVector2D View = Screen.GetViewportSlateSize();
		if (FindAnchor(Screen, Low))
			Parts.PanelSlot.SetPadding(FMargin(Low.X, 0.0, 0.0, BottomPadding(View.Y, Low.Y)));
		else
			Parts.PanelSlot.SetPadding(FMargin(16.0, 0.0, 0.0, BottomPadding(View.Y, View.Y)));
	}

	// The held production (bHeldNow: IsProductionHeldForRoom, the C++ read): the message, then the thread line, then the journal line, once (as CheckPopulationBlocked).
	// True when it went (the screen mirrors the key to the C++).
	bool CheckPopulationHeld(UHOLScriptScreen Screen, FHOLShortMessageParts& Parts, FHOLShortMessageState& State, FHOLPopulationHeldWatch& Watch, bool bHeldNow)
	{
		if (!ShouldShowPopulationHeld(Watch, bHeldNow))
			return false;
		ShowText(Parts, State, PopulationKey, PopulationMaxText());
		// D833: the same line stays 15 s here too (one constant for the line).
		Screen.AddThreadLine(HOLAlertRules::PopulationFullText(), HOLAlertRules::PopulationFullLineSeconds);
		UHOLGameMenuScreen::MenuLog(HeldLogLine());
		return true;
	}
}
