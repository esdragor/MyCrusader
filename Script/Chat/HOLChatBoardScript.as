// The server's chat board (A4, ChatInProgress.md; MIG-P1b, D830/D835), the script side of UHOLChatSubsystem: the C++ part gives the seats
// (FHOLChatSeats, read from the replicated game state), the sender's recent posts and the match time; the rules are HOLChatRules.as
// (HOLChat::CheckPost, ReachesBySeats). Server and solo only (AHOLPlayerController::ServerPostChat calls it; a client never decides).
// A limit <= 0 (the cvars hol.Chat.MaxLength, MaxPerWindow, WindowMs at 0) is the script's default. Tested by HeroesOfLiberty.Chat.ServerBoard.
class UHOLChatBoardScript : UHOLChatSubsystem
{
	UFUNCTION(BlueprintOverride)
	FHOLChatGateAnswer ChatCheckPost(const FHOLChatSeats& Seats, int Channel, int Sender, int Target, const FString& Text, const TArray<int>& RecentMs,
		int NowMs, int MaxLength, int MaxPerWindow, int WindowMs)
	{
		FHOLChatGateAnswer Gate;
		Gate.RecentMs = RecentMs;
		Gate.Answer = HOLChat::CheckPost(Seats.Slots, Channel, Sender, Target, Text, Gate.RecentMs, NowMs,
			MaxLength > 0 ? MaxLength : HOLChat::MaxTextLength,
			MaxPerWindow > 0 ? MaxPerWindow : HOLChat::MaxPerWindow,
			WindowMs > 0 ? WindowMs : HOLChat::WindowMs);
		return Gate;
	}

	UFUNCTION(BlueprintOverride)
	bool ChatReaches(const FHOLChatSeats& Seats, int Channel, int Sender, int Target, int Receiver)
	{
		return HOLChat::ReachesBySeats(Channel, Sender, Target, Receiver, Seats.MateByPair, Seats.SpectatorBySlot, Seats.bTeamGame);
	}
}
