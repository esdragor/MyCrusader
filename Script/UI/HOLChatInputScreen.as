// NOT COMPILED, NOT RUN (GLORFINDEL-1120h): D807 (Q-chat-ui = A), the chat input screen. Pure part (namespace HOLChatInputScreen, tested by
// HOLChatInputScreen_Test.as) + the widget helper UHOLChatInputScreen (a text field built by script, the state of HOLChatInput, the send by ServerPostChat).
//
// WIRING (the screen that owns it, not done here: HOLGameScreenScript.as belongs to another agent):
//   member:   UHOLChatInputScreen ChatInput;
//   OnInitialized: ChatInput = Cast<UHOLChatInputScreen>(NewObject(this, UHOLChatInputScreen)); ChatInput.Build(this, <a UVerticalBox of the frame>);
//   Refresh:  ChatInput.SetContext(<the local player has an ally>, <slot chosen in the Diplomacy panel, -1 none>, <its name>);
//   KeyPressed(FName KeyName): ChatInput.KeyPressed(KeyName);   (the C++ tick must send "Enter", see the report: HOLScriptScreen.cpp ~1034)
//   Describe: append ChatInput.Describe().
// Inside the field Enter / Escape / Tab come as the text box's OnTextCommitted (OnEnter / OnCleared / OnUserMovedFocus): script only, to verify at run time.

namespace HOLChatInputScreen
{
	// The refusals of a text before it leaves the field.
	const int Ok = 0;
	const int Blank = 1;
	const int TooLong = 2;

	// The mark of the channel in the field: "Allies", "Tous", or the chosen player's name (Private; "Privé" when no name is known).
	FString ChannelMark(int Channel, const FString& PlayerName)
	{
		if (Channel == HOLChat::Allies)
		{
			return "Allies";
		}
		if (Channel == HOLChat::Private)
		{
			return PlayerName.TrimStartAndEnd().IsEmpty() ? "Privé" : PlayerName;
		}
		return "Tous";
	}

	// What stands before the typed text.
	FString MarkLine(int Channel, const FString& PlayerName)
	{
		return f"{ChannelMark(Channel, PlayerName)} : ";
	}

	// The text that leaves the field: trimmed.
	FString SendText(const FString& Text)
	{
		return Text.TrimStartAndEnd();
	}

	// Blank: nothing to send. TooLong: more than the board takes (HOLChat::MaxTextLength); the field keeps its text so the player can cut it.
	int SendRefusal(const FString& Text)
	{
		const FString Trimmed = Text.TrimStartAndEnd();
		if (Trimmed.IsEmpty())
		{
			return Blank;
		}
		if (Trimmed.Len() > HOLChat::MaxTextLength)
		{
			return TooLong;
		}
		return Ok;
	}

	// The line shown in the field when "/w Name message" is refused (HOLChatInput::ParseWhisper): unknown player, or no text. "" for any other result.
	FString WhisperRefusalText(int WhisperResult)
	{
		if (WhisperResult == HOLChatInput::WhisperUnknown)
		{
			return "Joueur inconnu (/w Nom message) -";
		}
		if (WhisperResult == HOLChatInput::WhisperEmpty)
		{
			return "Message vide (/w Nom message) -";
		}
		return "";
	}

	// The pause after a commit of the field during which an Enter is still the same press (the polled Enter of the commit's own press). Kept short: the
	// 2nd Enter of a real player or of a scenario comes ~0.2 s later and must reopen the field (RECETTE-1120pa, ARWEN-1120pj; it was 0.25 s).
	const float SamePressSeconds = 0.1;

	// Whether an Enter heard while the field is closed opens it: the field is closed and the last commit is older than SamePressSeconds.
	bool EnterOpensField(bool bOpen, float Now, float LastCommitTime)
	{
		return !bOpen && Now - LastCommitTime > SamePressSeconds;
	}

	// True when some slot other than the viewer is present and the stance is Ally both ways (D369: an alliance needs both sides).
	// MyStances[S] = the viewer's stance toward S, TheirStances[S] = S's stance toward the viewer, Present[S] = S is in the match. A spectator (Viewer < 0) has no ally.
	bool HasAlly(int Viewer, const TArray<int>& MyStances, const TArray<int>& TheirStances, const TArray<bool>& Present)
	{
		if (Viewer < 0)
		{
			return false;
		}
		const int Ally = HOLDiplomacyRead::GetStanceAlly();
		for (int Slot = 0; Slot < Present.Num() && Slot < MyStances.Num() && Slot < TheirStances.Num(); ++Slot)
		{
			if (Slot != Viewer && Present[Slot] && MyStances[Slot] == Ally && TheirStances[Slot] == Ally)
			{
				return true;
			}
		}
		return false;
	}
}

// The field: a border holding "<mark> : " and a UEditableTextBox, collapsed until Enter opens it. Same construction as UHOLProductionQueuePanel.
class UHOLChatInputScreen : UObject
{
	UHOLScriptScreen Screen;
	UBorder Panel;
	UCommonTextBlock MarkText;
	UEditableTextBox Box;

	FHOLChatInputState State;
	bool bHasAlly = false;
	int ChosenSlot = -1;
	FString ChosenName = "";
	// The time of the last commit by the field itself: the polled Enter of the same press must not reopen it.
	float LastCommitTime = -10.0;
	FString LastSent = "";
	// How many times the field handed the keyboard focus back to the game viewport (GLORFINDEL-1120sa: after a send the collapsed box kept the Slate focus, the viewport lost it and the polled 2nd Enter never reached the player input).
	int FocusReturns = 0;
	// "/w Name message" (B of the 2026-11-20 answer): the players a whisper can name (Names[I] is the name of Slots[I]) and the refusal line shown in the mark.
	TArray<FString> WhisperNames;
	TArray<int> WhisperSlots;
	FString Refusal = "";

	void Build(UHOLScriptScreen InScreen, UVerticalBox Host)
	{
		Screen = InScreen;
		Panel = Cast<UBorder>(Screen.ConstructWidget(UBorder, n"ChatInputPanel"));
		Panel.SetVisibility(ESlateVisibility::Collapsed);
		Host.AddChildToVerticalBox(Panel);
		UHorizontalBox Row = Cast<UHorizontalBox>(Screen.ConstructWidget(UHorizontalBox, n"ChatInputRow"));
		Panel.SetContent(Row);
		MarkText = Cast<UCommonTextBlock>(Screen.ConstructWidget(UCommonTextBlock, n"ChatInputMark"));
		Row.AddChildToHorizontalBox(MarkText).SetVerticalAlignment(EVerticalAlignment::VAlign_Center);
		Box = Cast<UEditableTextBox>(Screen.ConstructWidget(UEditableTextBox, n"ChatInputBox"));
		UHorizontalBoxSlot BoxSlot = Row.AddChildToHorizontalBox(Box);
		FSlateChildSize FillSize;
		FillSize.SizeRule = ESlateSizeRule::Fill;
		FillSize.Value = 1.0;
		BoxSlot.SetSize(FillSize);
		Box.OnTextCommitted.AddUFunction(this, n"OnBoxCommitted");
	}

	// What the screen knows and this field does not: an ally, the player chosen in the Diplomacy panel (D387).
	void SetContext(bool bInHasAlly, int InChosenSlot, const FString& InChosenName)
	{
		bHasAlly = bInHasAlly;
		ChosenSlot = InChosenSlot;
		ChosenName = InChosenName;
		if (State.bOpen)
		{
			ShowMark();
		}
	}

	// The players "/w Name message" can name (the screen's Diplomacy rows: everybody in the match but the local player).
	void SetWhisperDirectory(const TArray<FString>& InNames, const TArray<int>& InSlots)
	{
		WhisperNames = InNames;
		WhisperSlots = InSlots;
	}

	void ShowMark()
	{
		const FString Line = HOLChatInputScreen::MarkLine(State.Channel, ChosenName);
		MarkText.SetText(FText::FromString(Refusal.IsEmpty() ? Line : f"{Refusal} {Line}"));
	}

	void Open()
	{
		Refusal = "";
		Box.SetText(FText::FromString(""));
		ShowMark();
		Panel.SetVisibility(ESlateVisibility::SelfHitTestInvisible);
		Box.SetKeyboardFocus();
	}

	// Stub (red): the focus is not handed back yet.
	void ReturnFocusToGame()
	{
	}

	void Close()
	{
		if (Panel != nullptr)
		{
			Panel.SetVisibility(ESlateVisibility::Collapsed);
		}
	}

	// A key the screen heard (KeyPressed of UHOLScriptScreen). Enter only opens: once open, the text box's own commit carries Enter, Escape and Tab.
	void KeyPressed(FName KeyName)
	{
		if (KeyName == n"Enter" && HOLChatInputScreen::EnterOpensField(State.bOpen, Gameplay::GetTimeSeconds(), LastCommitTime))
		{
			HandleEnter();
		}
		else if (KeyName == n"Tab")
		{
			HandleTab();
		}
		else if (KeyName == n"Escape")
		{
			HandleEscape();
		}
	}

	void HandleEnter()
	{
		const FString Text = State.bOpen ? Box.GetText().ToString() : "";
		Refusal = "";
		const int Result = HOLChatInput::PressEnter(State, Text, bHasAlly, ChosenSlot >= 0);
		if (Result == HOLChatInput::Opened)
		{
			Open();
		}
		else if (Result == HOLChatInput::Send)
		{
			Submit(Text);
		}
		else if (Result == HOLChatInput::Dismissed)
		{
			Close();
		}
		else
		{
			ShowMark();
		}
	}

	void HandleTab()
	{
		if (HOLChatInput::PressTab(State, bHasAlly, ChosenSlot >= 0))
		{
			ShowMark();
			Box.SetKeyboardFocus();
		}
	}

	void HandleEscape()
	{
		if (HOLChatInput::PressEscape(State))
		{
			Close();
		}
	}

	// The field has just closed for a send (State.Channel is the channel sent on). Too long: it opens again with its text.
	void Submit(const FString& TypedText)
	{
		// "/w Name message": a known name sends the text privately to that slot whatever the channel of the field; an unknown name or no text is refused.
		FString Text = TypedText;
		int Channel = State.Channel;
		int Target = HOLChatInput::TargetSlot(State, ChosenSlot);
		int WhisperSlot = -1;
		FString WhisperBody;
		const int Whisper = HOLChatInput::ParseWhisper(TypedText, WhisperNames, WhisperSlots, WhisperSlot, WhisperBody);
		if (Whisper == HOLChatInput::WhisperUnknown || Whisper == HOLChatInput::WhisperEmpty)
		{
			Refusal = HOLChatInputScreen::WhisperRefusalText(Whisper);
			State.bOpen = true;
			ShowMark();
			return;
		}
		if (Whisper == HOLChatInput::WhisperOk)
		{
			Text = WhisperBody;
			Channel = HOLChat::Private;
			Target = WhisperSlot;
		}
		if (HOLChatInputScreen::SendRefusal(Text) != HOLChatInputScreen::Ok)
		{
			State.bOpen = true;
			ShowMark();
			return;
		}
		AHOLRpcPlayer Rpc = Cast<AHOLRpcPlayer>(HOLRpcBridge::FindPlayerActor(Cast<AHOLPlayerController>(Screen.GetOwningPlayer())));
		if (Rpc == nullptr)
		{
			Log(n"LogHOLMatch", "ChatInput: no script player actor, message not sent");
			State.bOpen = true;
			return;
		}
		LastSent = HOLChatInputScreen::SendText(Text);
		Rpc.ServerPostChat(Channel, LastSent, Target);
		Close();
	}

	UFUNCTION()
	void OnBoxCommitted(const FText&in Text, ETextCommit CommitMethod)
	{
		if (!State.bOpen)
		{
			// The box kept the keyboard focus after the send (its panel is only collapsed): Slate gives it the next Enter, the polled path never sees it.
			// A closed field that hears Enter by its own commit opens (ARWEN-1120pj).
			if (CommitMethod == ETextCommit::OnEnter && HOLChatInputScreen::EnterOpensField(State.bOpen, Gameplay::GetTimeSeconds(), LastCommitTime))
			{
				LastCommitTime = Gameplay::GetTimeSeconds();
				HandleEnter();
			}
			return;
		}
		LastCommitTime = Gameplay::GetTimeSeconds();
		if (CommitMethod == ETextCommit::OnEnter)
		{
			HandleEnter();
		}
		else if (CommitMethod == ETextCommit::OnCleared)
		{
			HandleEscape();
		}
		else if (CommitMethod == ETextCommit::OnUserMovedFocus)
		{
			HandleTab();
		}
	}

	// One line for hol.HUD.Report: "chat: closed" or "chat: open <mark>".
	FString Describe()
	{
		if (!State.bOpen)
		{
			return f"chat: closed (last sent '{LastSent}')";
		}
		return f"chat: open {HOLChatInputScreen::ChannelMark(State.Channel, ChosenName)}";
	}
}
