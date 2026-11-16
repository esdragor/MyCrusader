// Chat channels, pure rule (A4; GDD 15 Communication, D385 channels Allies, All and private messages; D376 and D357 the spectator): who
// receives a message, from the slots' teams (a camp = a team >= 0, mutual allies; -1 = no team) and which slots are spectators (an eliminated
// player). No world access, tested in Script\Tests\HOLChatRules_Test.as. Not here: the network (who sends what to whom), the display, the
// shortcuts, the diplomacy announcements in the feed.
// The sender always sees its own message. A spectator (D376) talks with the spectators alone in free-for-all and Diplomacy, and with his
// teammates too in a team game (D357, bTeamGame). D792 (the user's answer to Q-chat-channels, 1 A and 2 A): a living player's All reaches the
// spectators too (a message to all leaks nothing); a living player's Allies reaches a spectating teammate only in a team game.
// D806 (Q-chat-rules A): a spectator without a seat (a slot outside the seats) is treated as a defeated one: All and Allies with the
// spectators, never a private message, the living players' All read (C++ mirror: Source/HeroesOfLiberty/Match/HOLChatRules.cpp).
// MIG-P1a (D830, D835): the server's view too, a camp = a mutual alliance of two seats (MateByPair[From * Slots + To], the Diplomacy
// stances, not transitive), and the server's gate CheckPost (length, rate, channel, target): ReachesBySeats, RecipientsBySeats, CheckPost
// are the rule the server calls after the switch MIG-P1b; the team numbers above are one case of it (MatesFromTeams).
namespace HOLChat
{
	const int All = 0;
	const int Allies = 1;
	const int Private = 2;

	// The server's answers to a post (logged as "Chat: slot S channel C answer A").
	const int Ok = 0;
	const int Empty = 1;
	const int TooLong = 2;
	const int TooFast = 3;
	const int BadChannel = 4;
	const int BadTarget = 5;

	// PROVISIONAL, the gate's defaults (cvars hol.Chat.MaxLength, hol.Chat.MaxPerWindow, hol.Chat.WindowMs; Tools/Data/ValueSheet.csv).
	const int MaxTextLength = 200;
	const int MaxPerWindow = 5;
	const int WindowMs = 10000;

	// D806 (Q-chat-rules A): a slot outside the seats (-1, INDEX_NONE on the server, or past the seats for a second observer) is a
	// spectator without a seat (a pure observer, D376), as the C++ FSeats::IsSpectator says.
	bool IsSeated(const TArray<bool>& SpectatorBySlot, int Slot)
	{
		return SpectatorBySlot.IsValidIndex(Slot);
	}

	bool IsSpectator(const TArray<bool>& SpectatorBySlot, int Slot)
	{
		return !IsSeated(SpectatorBySlot, Slot) || SpectatorBySlot[Slot];
	}

	// Two slots of the same camp: the same team, a real one (>= 0).
	bool AreMates(const TArray<int>& TeamBySlot, int A, int B)
	{
		return TeamBySlot.IsValidIndex(A) && TeamBySlot.IsValidIndex(B) && TeamBySlot[A] >= 0 && TeamBySlot[A] == TeamBySlot[B];
	}

	// Two seats in a mutual alliance (the server's matrix, From * Slots + To; Slots = SpectatorBySlot.Num()).
	bool AreSeatMates(const TArray<bool>& MateByPair, int Slots, int A, int B)
	{
		return A >= 0 && B >= 0 && A < Slots && B < Slots && MateByPair.IsValidIndex(A * Slots + B) && MateByPair[A * Slots + B];
	}

	// The alliance matrix of a game in teams (two different slots of the same real team are mates).
	TArray<bool> MatesFromTeams(const TArray<int>& TeamBySlot, int Slots)
	{
		TArray<bool> MateByPair;
		for (int From = 0; From < Slots; ++From)
		{
			for (int To = 0; To < Slots; ++To)
			{
				MateByPair.Add(From != To && AreMates(TeamBySlot, From, To));
			}
		}
		return MateByPair;
	}

	// Does the message of Sender on Channel (to Target for a private one) reach Receiver? The seats as the server sees them: per pair a
	// mutual alliance, per slot a spectator (defeated); a slot outside them is an observer without a seat (D806).
	bool ReachesBySeats(int Channel, int Sender, int Target, int Receiver, const TArray<bool>& MateByPair, const TArray<bool>& SpectatorBySlot, bool bTeamGame)
	{
		// D806: an observer never talks privately, not even to a spectator (before the sender's own check: two observers share -1).
		if (Channel == Private && !IsSeated(SpectatorBySlot, Sender))
		{
			return false;
		}
		if (Receiver == Sender)
		{
			return true;
		}
		// D376: a spectator talks with the spectators, and with his teammates in a team game (D357). A living player may reach anyone.
		const int Slots = SpectatorBySlot.Num();
		if (IsSpectator(SpectatorBySlot, Sender) && !IsSpectator(SpectatorBySlot, Receiver)
			&& !(bTeamGame && AreSeatMates(MateByPair, Slots, Sender, Receiver)))
		{
			return false;
		}
		if (Channel == All)
		{
			return true;
		}
		if (Channel == Allies)
		{
			// D806: an observer is on the spectators' side of the Allies channel, every spectator is his mate (never a living player).
			if (!IsSeated(SpectatorBySlot, Sender) || !IsSeated(SpectatorBySlot, Receiver))
			{
				return IsSpectator(SpectatorBySlot, Sender) && IsSpectator(SpectatorBySlot, Receiver);
			}
			// Mates; a spectator and a living player are not on the same side of the Allies channel outside a team game (D792 2 A).
			return AreSeatMates(MateByPair, Slots, Sender, Receiver)
				&& (bTeamGame || IsSpectator(SpectatorBySlot, Sender) == IsSpectator(SpectatorBySlot, Receiver));
		}
		if (Channel == Private)
		{
			return Receiver == Target;
		}
		return false;
	}

	// Every seat the message reaches, the sender's first.
	TArray<int> RecipientsBySeats(int Channel, int Sender, int Target, const TArray<bool>& MateByPair, const TArray<bool>& SpectatorBySlot, bool bTeamGame)
	{
		TArray<int> Slots;
		Slots.Add(Sender);
		for (int Slot = 0; Slot < SpectatorBySlot.Num(); ++Slot)
		{
			if (Slot != Sender && ReachesBySeats(Channel, Sender, Target, Slot, MateByPair, SpectatorBySlot, bTeamGame))
			{
				Slots.Add(Slot);
			}
		}
		return Slots;
	}

	// Only blanks (C++ FString::TrimStartAndEnd: spaces, tabs, line ends).
	bool IsBlank(const FString& Text)
	{
		return Text.Replace(" ", "").Replace("\t", "").Replace("\n", "").Replace("\r", "").IsEmpty();
	}

	// The server's gate for a post of Sender at NowMs among Slots seats: Ok (and NowMs kept in RecentMs, the sender's recent posts), or
	// why it is refused (an unknown channel, a private message to no other seat, an empty text, longer than MaxLength characters, more
	// than InMaxPerWindow posts within InWindowMs). A refused post never counts in the rate.
	int CheckPost(int Slots, int Channel, int Sender, int Target, const FString& Text, TArray<int>& RecentMs, int NowMs, int MaxLength,
		int InMaxPerWindow, int InWindowMs)
	{
		if (Channel != All && Channel != Allies && Channel != Private)
		{
			return BadChannel;
		}
		if (Channel == Private && (Target < 0 || Target >= Slots || Target == Sender))
		{
			return BadTarget;
		}
		if (IsBlank(Text))
		{
			return Empty;
		}
		if (Text.Len() > MaxLength)
		{
			return TooLong;
		}
		for (int Index = RecentMs.Num() - 1; Index >= 0; --Index)
		{
			if (NowMs - RecentMs[Index] >= InWindowMs)
			{
				RecentMs.RemoveAt(Index);
			}
		}
		if (RecentMs.Num() >= InMaxPerWindow)
		{
			return TooFast;
		}
		RecentMs.Add(NowMs);
		return Ok;
	}

	// Does the message of Sender on Channel (to Target for a private one) reach Receiver? A camp = a team number (a game in teams).
	bool Reaches(int Channel, int Sender, int Target, int Receiver, const TArray<int>& TeamBySlot, const TArray<bool>& SpectatorBySlot, bool bTeamGame)
	{
		return ReachesBySeats(Channel, Sender, Target, Receiver, MatesFromTeams(TeamBySlot, SpectatorBySlot.Num()), SpectatorBySlot, bTeamGame);
	}

	// D807 (Q-chat-ui A): the channel the input field opens on with Enter: Allies, or All for a player without an ally (free-for-all).
	int OpeningChannel(bool bHasAlly)
	{
		return bHasAlly ? Allies : All;
	}

	// D807: Tab in the field: Allies -> All -> Private (to the player chosen in the Diplomacy panel, D387) -> Allies; a channel nobody
	// would read is skipped (Allies without an ally, Private without a chosen player). All is always there.
	int NextChannel(int Current, bool bHasAlly, bool bHasPrivateTarget)
	{
		int Channel = Current;
		for (int Step = 0; Step < 3; ++Step)
		{
			Channel = Channel == Allies ? All : (Channel == All ? Private : Allies);
			if ((Channel != Allies || bHasAlly) && (Channel != Private || bHasPrivateTarget))
			{
				return Channel;
			}
		}
		return All;
	}

	// Every slot the message reaches, the sender's first.
	TArray<int> Recipients(int Channel, int Sender, int Target, const TArray<int>& TeamBySlot, const TArray<bool>& SpectatorBySlot, bool bTeamGame)
	{
		TArray<int> Slots;
		Slots.Add(Sender);
		for (int Slot = 0; Slot < TeamBySlot.Num(); ++Slot)
		{
			if (Slot != Sender && Reaches(Channel, Sender, Target, Slot, TeamBySlot, SpectatorBySlot, bTeamGame))
			{
				Slots.Add(Slot);
			}
		}
		return Slots;
	}
}
