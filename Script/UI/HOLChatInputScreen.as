// D807 (Q-chat-ui = A): the chat input screen (GLORFINDEL-1120h). STUB of the pure part (red first): the bodies below do nothing yet.
// The widget (UEditableTextBox, Enter / Tab / Escape, send by ServerPostChat) comes with the fix.

namespace HOLChatInputScreen
{
	// The refusals of a text before it leaves the field.
	const int Ok = 0;
	const int Blank = 1;
	const int TooLong = 2;

	FString ChannelMark(int Channel, const FString& PlayerName)
	{
		return "";
	}

	FString MarkLine(int Channel, const FString& PlayerName)
	{
		return "";
	}

	FString SendText(const FString& Text)
	{
		return Text;
	}

	int SendRefusal(const FString& Text)
	{
		return Ok;
	}
}
