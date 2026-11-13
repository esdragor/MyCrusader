// Chat channels, pure rule (A4; GDD 15 Communication, D385 channels Allies, All and private messages; D376 and D357 the spectator): who
// receives a message, from the slots' teams (a camp = a team >= 0, mutual allies; -1 = no team) and which slots are spectators (an eliminated
// player). No world access, tested in Script\Tests\HOLChatRules_Test.as. Not here: the network (who sends what to whom), the display, the
// shortcuts, the diplomacy announcements in the feed.
// The sender always sees its own message. A spectator (D376) talks with the spectators alone in free-for-all and Diplomacy, and with his
// teammates too in a team game (D357, bTeamGame). D792 (the user's answer to Q-chat-channels, 1 A and 2 A): a living player's All reaches the
// spectators too (a message to all leaks nothing); a living player's Allies reaches a spectating teammate only in a team game.
namespace HOLChat
{
	const int All = 0;
	const int Allies = 1;
	const int Private = 2;

	bool IsSpectator(const TArray<bool>& SpectatorBySlot, int Slot)
	{
		return SpectatorBySlot.IsValidIndex(Slot) && SpectatorBySlot[Slot];
	}

	// Two slots of the same camp: the same team, a real one (>= 0).
	bool AreMates(const TArray<int>& TeamBySlot, int A, int B)
	{
		return TeamBySlot.IsValidIndex(A) && TeamBySlot.IsValidIndex(B) && TeamBySlot[A] >= 0 && TeamBySlot[A] == TeamBySlot[B];
	}

	// What a spectator may reach (D376): the spectators, and his teammates in a team game. A living player may reach anyone.
	bool MayTalkTo(const TArray<int>& TeamBySlot, const TArray<bool>& SpectatorBySlot, bool bTeamGame, int Sender, int Receiver)
	{
		if (!IsSpectator(SpectatorBySlot, Sender))
		{
			return true;
		}
		return IsSpectator(SpectatorBySlot, Receiver) || (bTeamGame && AreMates(TeamBySlot, Sender, Receiver));
	}

	// Does the message of Sender on Channel (to Target for a private one) reach Receiver?
	bool Reaches(int Channel, int Sender, int Target, int Receiver, const TArray<int>& TeamBySlot, const TArray<bool>& SpectatorBySlot, bool bTeamGame)
	{
		if (Receiver == Sender)
		{
			return true;
		}
		if (!MayTalkTo(TeamBySlot, SpectatorBySlot, bTeamGame, Sender, Receiver))
		{
			return false;
		}
		if (Channel == All)
		{
			return true;
		}
		if (Channel == Allies)
		{
			// Mates; a spectator and a living player are not on the same side of the Allies channel outside a team game.
			return AreMates(TeamBySlot, Sender, Receiver)
				&& (bTeamGame || IsSpectator(SpectatorBySlot, Sender) == IsSpectator(SpectatorBySlot, Receiver));
		}
		if (Channel == Private)
		{
			return Receiver == Target;
		}
		return false;
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
