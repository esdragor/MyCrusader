// D807 (Q-chat-ui = A): the pure rule of the chat input field (GLORFINDEL-1120g).
// States: closed / open on Allies / open on All / open on Private (the channels are HOLChat::Allies, All, Private of Script\Chat\HOLChatRules.as).
// Enter on a closed field opens it on Allies (All without an ally, HOLChat::OpeningChannel); Tab walks Allies > All > Private > Allies
// (HOLChat::NextChannel: a channel nobody would read is skipped; Private only with a player chosen in the Diplomacy panel, D387);
// Escape closes; Enter on an open field with a text sends and closes. The screen wiring (a text field built by script, Enter / Tab / Escape
// capture, RequestChat or the hol.Chat.Say command) is NOT here: this file only decides.

struct FHOLChatInputState
{
	bool bOpen = false;
	// The channel the field is on while open; after a send it keeps the channel sent on (the caller routes with it and with TargetSlot).
	int Channel = 1;
}

namespace HOLChatInput
{
	// What Enter did.
	const int None = 0;
	const int Opened = 1;
	const int Send = 2;
	const int Dismissed = 3;

	// Enter. Closed: opens on Allies (All without an ally). Open with a blank text: closes, nothing sent (Dismissed). Open on a channel nobody
	// would read any more (the ally died, the chosen player changed): falls back to All and sends NOTHING (None), so the audience never
	// widens without the player seeing it; he presses Enter again. Otherwise Send: the field closes, State.Channel is the channel to send on.
	int PressEnter(FHOLChatInputState& State, const FString& Text, bool bHasAlly, bool bHasPrivateTarget)
	{
		if (!State.bOpen)
		{
			State.bOpen = true;
			State.Channel = HOLChat::OpeningChannel(bHasAlly);
			return Opened;
		}
		if (Text.TrimStartAndEnd().IsEmpty())
		{
			State.bOpen = false;
			return Dismissed;
		}
		if ((State.Channel == HOLChat::Allies && !bHasAlly) || (State.Channel == HOLChat::Private && !bHasPrivateTarget))
		{
			State.Channel = HOLChat::All;
			return None;
		}
		State.bOpen = false;
		return Send;
	}

	// Tab: next channel while open (true when the field is open); nothing on a closed field.
	bool PressTab(FHOLChatInputState& State, bool bHasAlly, bool bHasPrivateTarget)
	{
		if (!State.bOpen)
		{
			return false;
		}
		State.Channel = HOLChat::NextChannel(State.Channel, bHasAlly, bHasPrivateTarget);
		return true;
	}

	// Escape: closes an open field without sending (true when it was open).
	bool PressEscape(FHOLChatInputState& State)
	{
		if (!State.bOpen)
		{
			return false;
		}
		State.bOpen = false;
		return true;
	}

	// The slot a Private message goes to: the player chosen in the Diplomacy panel (-1: none); -1 on any other channel.
	int TargetSlot(const FHOLChatInputState& State, int ChosenSlot)
	{
		return State.Channel == HOLChat::Private ? ChosenSlot : -1;
	}
}
