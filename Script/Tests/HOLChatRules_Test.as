// Chat channels, pure rule (A4, HOLChatRules.as; D385, D376, D357). Four slots: 0 and 1 allies (team 0), 2 and 3 their enemies (team 1) unless a
// test says otherwise. Every case goes through HOLChat::Recipients, the list the network will send to.

TArray<int> ChatTeams(int T0 = 0, int T1 = 0, int T2 = 1, int T3 = 1)
{
	TArray<int> Teams;
	Teams.Add(T0);
	Teams.Add(T1);
	Teams.Add(T2);
	Teams.Add(T3);
	return Teams;
}

TArray<bool> ChatSpectators(bool S0 = false, bool S1 = false, bool S2 = false, bool S3 = false)
{
	TArray<bool> Spectators;
	Spectators.Add(S0);
	Spectators.Add(S1);
	Spectators.Add(S2);
	Spectators.Add(S3);
	return Spectators;
}

// An Allies message reaches the sender and his ally, never an enemy.
void Test_ChatAlliesNotToEnemies(FUnitTest& T)
{
	TArray<int> To = HOLChat::Recipients(HOLChat::Allies, 0, -1, ChatTeams(), ChatSpectators(), false);
	T.AssertTrue(To.Contains(0) && To.Contains(1), "Allies: the sender and his ally");
	T.AssertFalse(To.Contains(2) || To.Contains(3), "Allies: no enemy");
}

// D376, free-for-all or Diplomacy: an Allies message of a living player does not reach a spectator, even his former ally.
void Test_ChatAlliesNotToSpectators(FUnitTest& T)
{
	TArray<int> To = HOLChat::Recipients(HOLChat::Allies, 0, -1, ChatTeams(0, 0, 1, 0), ChatSpectators(false, false, false, true), false);
	T.AssertTrue(To.Contains(1), "Allies: the living ally");
	T.AssertFalse(To.Contains(3), "Allies: not the spectating former ally");
	T.AssertFalse(To.Contains(2), "Allies: not the enemy");
}

// A private message reaches its target alone (and its sender).
void Test_ChatPrivateToTheTargetAlone(FUnitTest& T)
{
	TArray<int> To = HOLChat::Recipients(HOLChat::Private, 0, 2, ChatTeams(), ChatSpectators(), false);
	T.AssertTrue(To.Num() == 2 && To.Contains(0) && To.Contains(2), "Private: the sender and the target only");
	T.AssertFalse(To.Contains(1), "Private: not the sender's ally");
}

// D376: a spectator talks with the spectators alone in free-for-all, even on All and even privately to a living player.
void Test_ChatSpectatorTalksToSpectatorsOnly(FUnitTest& T)
{
	TArray<bool> Spectators = ChatSpectators(false, true, false, true);
	TArray<int> ToAll = HOLChat::Recipients(HOLChat::All, 1, -1, ChatTeams(), Spectators, false);
	T.AssertTrue(ToAll.Num() == 2 && ToAll.Contains(1) && ToAll.Contains(3), "spectator's All: the spectators only");
	TArray<int> ToPlayer = HOLChat::Recipients(HOLChat::Private, 1, 0, ChatTeams(), Spectators, false);
	T.AssertTrue(ToPlayer.Num() == 1 && ToPlayer[0] == 1, "spectator's private to a living player: nobody but himself");
}

// D357: in a team game a spectator still talks with his teammates (Allies), not with the enemies.
void Test_ChatTeamGameSpectatorTalksToTeammates(FUnitTest& T)
{
	TArray<int> To = HOLChat::Recipients(HOLChat::Allies, 1, -1, ChatTeams(), ChatSpectators(false, true, false, false), true);
	T.AssertTrue(To.Contains(0), "team game: the spectator reaches his living teammate");
	T.AssertFalse(To.Contains(2) || To.Contains(3), "team game: never the enemies");
	TArray<int> Back = HOLChat::Recipients(HOLChat::Allies, 0, -1, ChatTeams(), ChatSpectators(false, true, false, false), true);
	T.AssertTrue(Back.Contains(1), "team game: the living teammate's Allies reaches the spectating one");
}

// A living player's All reaches everybody, the spectators too.
void Test_ChatAllReachesEverybody(FUnitTest& T)
{
	TArray<int> To = HOLChat::Recipients(HOLChat::All, 2, -1, ChatTeams(), ChatSpectators(false, false, false, true), false);
	T.AssertTrue(To.Num() == 4, "All: the four slots");
}
