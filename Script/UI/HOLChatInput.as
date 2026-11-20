// D807 (Q-chat-ui = A): the pure rule of the chat input field (GLORFINDEL-1120g). STUB: compiles, does nothing (the test is red first).
// States: closed / open on Allies / open on All / open on Private (the channels are HOLChat::Allies, All, Private of Script\Chat\HOLChatRules.as).
// The screen wiring (a text field built by script, Enter / Tab / Escape capture) is NOT here: this file only decides.

struct FHOLChatInputState
{
	bool bOpen = false;
	// The channel the field is on while open; after a send it keeps the channel sent on (the caller reads it with TargetSlot to route).
	int Channel = 1;
}

namespace HOLChatInput
{
	// What Enter did.
	const int None = 0;
	const int Opened = 1;
	const int Send = 2;
	const int Dismissed = 3;

	int PressEnter(FHOLChatInputState& State, const FString& Text, bool bHasAlly, bool bHasPrivateTarget)
	{
		return None;
	}

	bool PressTab(FHOLChatInputState& State, bool bHasAlly, bool bHasPrivateTarget)
	{
		return false;
	}

	bool PressEscape(FHOLChatInputState& State)
	{
		return false;
	}

	int TargetSlot(const FHOLChatInputState& State, int ChosenSlot)
	{
		return ChosenSlot;
	}
}
