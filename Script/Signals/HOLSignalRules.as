// Map signals, pure rules (SG1; GDD 15, D386, with D369, D375, D370, D376, D357, D367): a wheel of typed signals (Alt held + click, on the
// world or the minimap): Attack here, Defend here, Need help, Rally, Scout here, each with its icon, its sound and a line in the feed; a
// plain Alt + click places a neutral signal. Seen by the sender and its allies only: in Diplomacy the mutual allies of D369, read from
// HOLDiplomacy::IsAlly (the relation is the most hostile of the two stances, D375; a break hides them at once, D370), in the team modes the
// teammates (an AI teammate included: it may answer later, D386). Limited against the spam (indicative: 3 every 5 s, D386).
// Times are integer milliseconds of the match (client and server agree); nothing here reads the world. The server applies Post (SG2), the
// clients draw SeenBy (SG3). Tested in Script/Tests/HOLSignalRules_Test.as.
// Provisional (Retours/Q-signals.md): the lifetime on the map (Q1, not given by the GDD) and the defeated teammate (Q2), one line each below.
struct FHOLSignal
{
	int Sender = -1;
	int Kind = 0;
	FVector Location;
	int PostedMs = 0;
}

// The signals on the map and the recent sends of each player (the anti-spam count lives apart from the lifetime).
struct FHOLSignalBoard
{
	TArray<FHOLSignal> Signals;
	TArray<int> SentBy;
	TArray<int> SentMs;
}

namespace HOLSignal
{
	// The kinds (D386): the plain Alt + click, then the five of the wheel.
	const int Neutral = 0;
	const int Attack = 1;
	const int Defend = 2;
	const int NeedHelp = 3;
	const int Rally = 4;
	const int Scout = 5;
	const int KindCount = 6;

	// PROVISIONAL (D386, "indicative: 3 every 5 s"): at most MaxPerWindow signals of one player within WindowMs.
	const int MaxPerWindow = 3;
	const int WindowMs = 5000;

	// D784 (Q-signals Q1 = A, 2026-11-12): how long a signal stays on the map. Equal to the window, a player never has more than
	// MaxPerWindow signals on the map at once (the feed keeps the trace).
	const int LifetimeMs = 5000;

	// D784 (Q-signals Q2 = A, 2026-11-12): a defeated player of a team match sees its team's signals but places none any more (it advises
	// in writing only, D357). In Diplomacy and free-for-all it talks to the spectators only (D376): never a signal either.
	const bool DefeatedTeammateSignals = false;

	// The answers of Post.
	const int Ok = 0;
	const int BadKind = 1;
	const int NoSuchSlot = 2;
	const int Silenced = 3;
	const int TooMany = 4;

	bool IsValidKind(int Kind)
	{
		return Kind >= 0 && Kind < KindCount;
	}

	// Viewer sees a signal of Sender: its own, or a mutual ally's (D386, D369). Never a Neutral or an Enemy.
	bool Sees(const FHOLDiplomacyBook& Book, int Sender, int Viewer, bool bViewerSpectator = false)
	{
		if (!HOLDiplomacy::InMatch(Book, Sender) || !HOLDiplomacy::InMatch(Book, Viewer))
			return false;
		return Sender == Viewer || HOLDiplomacy::IsAlly(Book, Sender, Viewer);
	}

	// Sender may place a signal: in the match, and alive or a defeated teammate of a team match (Q2; D376 for the others).
	bool CanSend(const FHOLDiplomacyBook& Book, int Sender, bool bDefeated)
	{
		if (!HOLDiplomacy::InMatch(Book, Sender))
			return false;
		return !bDefeated || (Book.bLocked && DefeatedTeammateSignals);
	}

	// The signals Sender placed in the window that ends at NowMs (the refused ones never count).
	int SentInWindow(const FHOLSignalBoard& Board, int Sender, int NowMs)
	{
		int Count = 0;
		for (int Index = 0; Index < Board.SentBy.Num(); Index++)
		{
			if (Board.SentBy[Index] == Sender && NowMs - Board.SentMs[Index] < WindowMs)
				Count++;
		}
		return Count;
	}

	// Forgets the faded signals and the sends out of the window.
	void Expire(FHOLSignalBoard& Board, int NowMs)
	{
		for (int Index = Board.Signals.Num() - 1; Index >= 0; Index--)
		{
			if (NowMs - Board.Signals[Index].PostedMs >= LifetimeMs)
				Board.Signals.RemoveAt(Index);
		}
		for (int Index = Board.SentBy.Num() - 1; Index >= 0; Index--)
		{
			if (NowMs - Board.SentMs[Index] >= WindowMs)
			{
				Board.SentBy.RemoveAt(Index);
				Board.SentMs.RemoveAt(Index);
			}
		}
	}

	// Sender places a signal of Kind at Location at NowMs. Ok: it is on the board; any other answer: nothing changed.
	int Post(FHOLSignalBoard& Board, const FHOLDiplomacyBook& Book, int Sender, bool bDefeated, int Kind, FVector Location, int NowMs)
	{
		if (!IsValidKind(Kind))
			return BadKind;
		if (!HOLDiplomacy::InMatch(Book, Sender))
			return NoSuchSlot;
		if (!CanSend(Book, Sender, bDefeated))
			return Silenced;
		Expire(Board, NowMs);
		if (SentInWindow(Board, Sender, NowMs) >= MaxPerWindow)
			return TooMany;
		FHOLSignal Signal;
		Signal.Sender = Sender;
		Signal.Kind = Kind;
		Signal.Location = Location;
		Signal.PostedMs = NowMs;
		Board.Signals.Add(Signal);
		Board.SentBy.Add(Sender);
		Board.SentMs.Add(NowMs);
		return Ok;
	}

	// The signals Viewer sees on the map at NowMs: alive, and its own or a mutual ally's.
	TArray<FHOLSignal> SeenBy(const FHOLSignalBoard& Board, const FHOLDiplomacyBook& Book, int Viewer, int NowMs, bool bViewerSpectator = false)
	{
		TArray<FHOLSignal> Result;
		for (const FHOLSignal& Signal : Board.Signals)
		{
			if (NowMs - Signal.PostedMs < LifetimeMs && NowMs >= Signal.PostedMs && Sees(Book, Signal.Sender, Viewer, bViewerSpectator))
				Result.Add(Signal);
		}
		return Result;
	}
}
