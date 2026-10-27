using HOLManager.Lobbies;

namespace HOLManager.Tests;

/**
 * R2.11: back to the lobby after a match (choice 6 of PROTOCOL.md, M6-Q7). A stopped server (server_stopped) gives
 * the 2-min reservation; a finished match gives none (D2): its players are out, "Retour au salon" is a plain join.
 */
public class ReturnToLobbyTests
{
    private const string Version = LobbyEntryTests.Version;
    private readonly FakeClock clock = new();
    private readonly FakeLauncher launcher = new();
    private readonly LobbyStore store;

    public ReturnToLobbyTests() => store = new LobbyStore(Version, clock, launcher: launcher);

    private string matchId = "";

    /** Bob (leader), Alice (color 3, team 2), Chloé; Nomad on; match played, then its server stopped (no "finished"). */
    private (Lobby Lobby, string Bob, string Alice, string Chloe) Finished(bool closeOthers = false) =>
        Played(closeOthers, finished: false);

    /** Bob (leader), Alice (color 3, team 2), Chloé; Nomad on; match played; finished, or its server stopped. */
    private (Lobby Lobby, string Bob, string Alice, string Chloe) Played(bool closeOthers = false, bool finished = true)
    {
        var created = store.Create("Salon de Bob", "Bob", Version);
        var id = created.Lobby.Id;
        var alice = store.Join(id, "Alice", Version).Token;
        var chloe = store.Join(id, "Chloé", Version).Token;
        store.UpdateSettings(id, created.Token, nomad: true);
        if (closeOthers)
        {
            for (var i = 3; i < LobbyRules.SlotCount; i++)
            {
                store.SetSlot(id, created.Token, i, open: false);
            }
        }
        store.UpdateMe(id, alice, colorIndex: 3, team: 2);
        foreach (var token in new[] { created.Token, alice, chloe })
        {
            store.UpdateMe(id, token, ready: true);
        }
        var match = store.Start(id, created.Token);
        store.ServerListening(match.Id, 7778);
        clock.UtcNow += TimeSpan.FromMinutes(20);
        matchId = match.Id;
        if (finished)
        {
            store.ServerFinished(match.Id, [0]);
        }
        else
        {
            launcher.Alive.Remove(match.Id);
            store.Housekeep();
        }
        return (created.Lobby, created.Token, alice, chloe);
    }

    private static LobbyPlayer Player(Lobby lobby, string name) => lobby.Players.Single(p => p.Name == name);

    [Fact]
    public void PlayersKeepTheirLineAndAreNotRemovedBySilenceFor2Minutes()
    {
        var (lobby, _, _, _) = Finished();
        clock.UtcNow += LobbyRules.ReturnReservation - TimeSpan.FromSeconds(1);
        store.Housekeep();
        Assert.Equal(["Bob", "Alice", "Chloé"], lobby.Players.Select(p => p.Name));
        var alice = Player(lobby, "Alice");
        Assert.Equal((1, 3, 2, false), (alice.SlotIndex, alice.ColorIndex, alice.Team, alice.Ready));
        Assert.True(lobby.Settings.Nomad);
    }

    [Fact]
    public void ThoseNotBackIn2MinutesFreeTheirPlace()
    {
        var (lobby, _, alice, _) = Finished();
        clock.UtcNow += TimeSpan.FromSeconds(30);
        store.Read(lobby.Id, alice);
        for (var t = 0; t < 90; t++)
        {
            clock.UtcNow += TimeSpan.FromSeconds(1);
            store.Read(lobby.Id, alice);
        }
        store.Housekeep();
        Assert.Equal(["Alice"], lobby.Players.Select(p => p.Name));
        Assert.Equal("Alice", lobby.Leader!.Name);
        Assert.Equal(SlotState.Open, lobby.Slots[0].State);
    }

    [Fact]
    public void ALobbyNobodyComesBackToIsDeletedAfter2Minutes()
    {
        var (lobby, _, _, _) = Finished();
        clock.UtcNow += LobbyRules.ReturnReservation;
        store.Housekeep();
        Assert.Null(store.Find(lobby.Id));
    }

    [Fact]
    public void JoinWithAReservedNameTakesBackTheLineWithANewToken()
    {
        var (lobby, _, alice, _) = Finished();
        clock.UtcNow += TimeSpan.FromSeconds(60);
        var back = store.Join(lobby.Id, "  alice ", Version);
        Assert.Equal(1, back.Slot);
        Assert.NotEqual(alice, back.Token);
        Assert.Equal(3, lobby.Players.Count());
        var line = Player(lobby, "Alice");
        Assert.Equal((3, 2), (line.ColorIndex, line.Team));
        LobbyEntryTests.AssertError(LobbyErrors.NotInLobby, () => store.Read(lobby.Id, alice));
        Assert.Equal("Alice", store.Read(lobby.Id, back.Token).Players.Single(p => p.Token == back.Token).Name);
    }

    [Fact]
    public void AReservedNameComesBackEvenWhenTheLobbyIsFull()
    {
        var (lobby, _, _, _) = Finished(closeOthers: true);
        LobbyEntryTests.AssertError(LobbyErrors.LobbyFull, () => store.Join(lobby.Id, "Dan", Version));
        Assert.Equal(2, store.Join(lobby.Id, "Chloé", Version).Slot);
    }

    [Fact]
    public void ANewcomerTakesAnOpenSlotNotAReservedOne()
    {
        var (lobby, _, _, _) = Finished();
        var dan = store.Join(lobby.Id, "Dan", Version);
        Assert.Equal(3, dan.Slot);
        LobbyEntryTests.AssertError(LobbyErrors.NameTaken, () => store.Join(lobby.Id, "Dan", Version));
    }

    [Fact]
    public void AfterThe2MinutesANameInTheLobbyIsTakenAgain()
    {
        var (lobby, _, alice, _) = Finished();
        for (var t = 0; t <= LobbyRules.ReturnReservation.TotalSeconds; t++)
        {
            store.Read(lobby.Id, alice);
            clock.UtcNow += TimeSpan.FromSeconds(1);
        }
        store.Read(lobby.Id, alice);
        LobbyEntryTests.AssertError(LobbyErrors.NameTaken, () => store.Join(lobby.Id, "Alice", Version));
    }

    [Fact]
    public void TheFirstBackLeadsWhileTheOldLeaderIsAway()
    {
        var (lobby, bob, alice, chloe) = Finished();
        Assert.Equal("Bob", store.Read(lobby.Id, null).Leader!.Name);
        store.Read(lobby.Id, chloe);
        Assert.Equal("Chloé", lobby.Leader!.Name);
        store.Join(lobby.Id, "Alice", Version);
        Assert.Equal("Chloé", lobby.Leader!.Name);
        store.Read(lobby.Id, bob);
        Assert.Equal("Bob", lobby.Leader!.Name);
        _ = alice;
    }

    [Fact]
    public void TheFirstBackCanChangeTheSettings()
    {
        var (lobby, _, _, chloe) = Finished();
        store.UpdateSettings(lobby.Id, chloe, regicide: true);
        Assert.True(lobby.Settings.Regicide);
    }

    [Fact]
    public void AStoppedServerReservesTheNamesToo()
    {
        var created = store.Create("Salon de Bob", "Bob", Version);
        var alice = store.Join(created.Lobby.Id, "Alice", Version).Token;
        store.UpdateMe(created.Lobby.Id, created.Token, ready: true);
        store.UpdateMe(created.Lobby.Id, alice, ready: true);
        var match = store.Start(created.Lobby.Id, created.Token);
        launcher.Alive.Remove(match.Id);
        store.Housekeep();
        Assert.Equal(LobbyNotices.ServerStopped, created.Lobby.Notice);
        clock.UtcNow += TimeSpan.FromSeconds(30);
        store.Housekeep();
        Assert.Equal(2, created.Lobby.Players.Count());
        Assert.Equal(1, store.Join(created.Lobby.Id, "Alice", Version).Slot);
    }

    [Fact]
    public void LeaveGivesUpTheReservation()
    {
        var (lobby, _, alice, _) = Finished();
        store.Leave(lobby.Id, alice);
        Assert.Equal(SlotState.Open, lobby.Slots[1].State);
        var again = store.Join(lobby.Id, "Alice", Version);
        Assert.Equal(1, again.Slot);
        Assert.Equal(0 + 1, Player(lobby, "Alice").ColorIndex);
    }

    [Fact]
    public void AfterFinishedItsPlayersAreOutAndTheSettingsStay()
    {
        var (lobby, bob, _, _) = Played();
        Assert.Same(lobby, store.Find(lobby.Id));
        Assert.Equal(LobbyState.Lobby, lobby.State);
        Assert.Empty(lobby.Players);
        Assert.True(lobby.Settings.Nomad);
        Assert.Null(lobby.Notice);
        LobbyEntryTests.AssertError(LobbyErrors.NotInLobby, () => store.Read(lobby.Id, bob));
    }

    [Fact]
    public void TheLeaderBackStartsWithoutThoseStillInTheMatch()
    {
        var (lobby, _, _, _) = Played();
        var chloe = store.Join(lobby.Id, "Chloé", Version);
        var bob = store.Join(lobby.Id, "Bob", Version);
        Assert.Equal("Bob", lobby.LeaderName);
        store.UpdateMe(lobby.Id, chloe.Token, ready: true);
        store.UpdateMe(lobby.Id, bob.Token, ready: true);
        Assert.NotEqual(matchId, store.Start(lobby.Id, bob.Token).Id);
    }

    [Fact]
    public void ReturnToTheLobbyIsAPlainJoin()
    {
        var (lobby, _, _, _) = Played(closeOthers: true);
        var bob = store.Join(lobby.Id, "Bob", Version).Token;
        var alice = store.Join(lobby.Id, "Alice", Version).Token;
        // Alice's old line is not hers any more: she comes back like anyone else.
        Assert.Equal(0, Player(lobby, "Alice").Team);
        LobbyEntryTests.AssertError(LobbyErrors.NameTaken, () => store.Join(lobby.Id, "alice", Version));
        var chloe = store.Join(lobby.Id, "Chloé", Version).Token;
        LobbyEntryTests.AssertError(LobbyErrors.LobbyFull, () => store.Join(lobby.Id, "Dan", Version));
        store.Leave(lobby.Id, alice);
        foreach (var token in new[] { bob, chloe })
        {
            store.UpdateMe(lobby.Id, token, ready: true);
        }
        store.Start(lobby.Id, bob);
        LobbyEntryTests.AssertError(LobbyErrors.LobbyInGame, () => store.Join(lobby.Id, "Alice", Version));
    }

    [Fact]
    public void ThoseBackTakeTheirMatchColorAgain()
    {
        // E2-1: Bob 0, Alice 3, Chloé 2 in the match; back in another order, each finds his color again.
        var (lobby, _, _, _) = Played();
        store.Join(lobby.Id, "Chloé", Version);
        store.Join(lobby.Id, "alice", Version);
        store.Join(lobby.Id, "Bob", Version);
        Assert.Equal([2, 3, 0], new[] { "Chloé", "alice", "Bob" }.Select(n => Player(lobby, n).ColorIndex));
        Assert.Equal(0, Player(lobby, "alice").Team);
    }

    [Fact]
    public void AMatchColorTakenMeanwhileGivesTheFirstFreeOne()
    {
        var (lobby, _, _, _) = Played();
        var dan = store.Join(lobby.Id, "Dan", Version).Token;
        Assert.Equal(0, Player(lobby, "Dan").ColorIndex);
        store.UpdateMe(lobby.Id, dan, colorIndex: 3);
        store.Join(lobby.Id, "Bob", Version);
        store.Join(lobby.Id, "Alice", Version);
        Assert.Equal((0, 1), (Player(lobby, "Bob").ColorIndex, Player(lobby, "Alice").ColorIndex));
    }

    [Fact]
    public void ThoseOnRandomFindRandomAgain()
    {
        // E2-2: "Aléatoire" is drawn at launch, but the lobby keeps the choice; both on it find it again.
        var created = store.Create("Salon de Bob", "Bob", Version);
        var id = created.Lobby.Id;
        var alice = store.Join(id, "Alice", Version).Token;
        var chloe = store.Join(id, "Chloé", Version).Token;
        store.UpdateMe(id, alice, colorIndex: LobbyRules.RandomColor);
        store.UpdateMe(id, chloe, colorIndex: LobbyRules.RandomColor);
        foreach (var token in new[] { created.Token, alice, chloe })
        {
            store.UpdateMe(id, token, ready: true);
        }
        var match = store.Start(id, created.Token);
        store.ServerListening(match.Id, 7778);
        clock.UtcNow += TimeSpan.FromMinutes(20);
        store.ServerFinished(match.Id, [0]);

        store.Join(id, "Chloé", Version);
        store.Join(id, "Alice", Version);
        store.Join(id, "Bob", Version);
        var lobby = created.Lobby;
        Assert.Equal([LobbyRules.RandomColor, LobbyRules.RandomColor, 0],
            new[] { "Chloé", "Alice", "Bob" }.Select(n => Player(lobby, n).ColorIndex));
    }

    [Fact]
    public void AnEmptyLobbyStaysWhileItsFinishedServerRuns()
    {
        var (lobby, _, _, _) = Played();
        clock.UtcNow += TimeSpan.FromMinutes(30);
        store.Housekeep();
        Assert.Same(lobby, store.Find(lobby.Id));
        launcher.Alive.Remove(matchId);
        store.Housekeep();
        Assert.Null(store.Find(lobby.Id));
    }

    [Fact]
    public void ALobbySomeoneCameBackToOutlivesItsServer()
    {
        var (lobby, _, _, _) = Played();
        var bob = store.Join(lobby.Id, "Bob", Version).Token;
        launcher.Alive.Remove(matchId);
        store.Read(lobby.Id, bob);
        Assert.Equal(["Bob"], lobby.Players.Select(p => p.Name));
        clock.UtcNow += LobbyRules.SilenceLimit;
        store.Housekeep();
        Assert.Null(store.Find(lobby.Id));
    }

    /** Bob (the match's leader) away; Chloé, then Alice back. */
    private (Lobby Lobby, string Chloe, string Alice) BackWithoutTheLeader(bool closeOthers = false)
    {
        var (lobby, _, _, _) = Played(closeOthers);
        var chloe = store.Join(lobby.Id, "Chloé", Version).Token;
        var alice = store.Join(lobby.Id, "Alice", Version).Token;
        return (lobby, chloe, alice);
    }

    [Fact]
    public void TheLeaderStillInTheFinishedMatchStaysLeader()
    {
        var (lobby, _, _, _) = Played();
        Assert.Equal("Bob", lobby.LeaderName);
        store.Join(lobby.Id, "Chloé", Version);
        clock.UtcNow += TimeSpan.FromMinutes(10);
        Assert.Equal("Bob", store.Read(lobby.Id, null).LeaderName);
    }

    [Fact]
    public void ThoseBackCanNeitherSetNorStartWhileTheLeaderIsAway()
    {
        var (lobby, chloe, alice) = BackWithoutTheLeader();
        store.UpdateMe(lobby.Id, chloe, ready: true);
        store.UpdateMe(lobby.Id, alice, ready: true);
        foreach (var token in new[] { chloe, alice })
        {
            LobbyEntryTests.AssertError(LobbyErrors.NotLeader, () => store.UpdateSettings(lobby.Id, token, regicide: true));
            LobbyEntryTests.AssertError(LobbyErrors.NotLeader, () => store.SetSlot(lobby.Id, token, 7, open: false));
            LobbyEntryTests.AssertError(LobbyErrors.NotLeader, () => store.Start(lobby.Id, token));
        }
    }

    [Fact]
    public void TheLeaderBackTakesTheLeadAgain()
    {
        var (lobby, chloe, alice) = BackWithoutTheLeader();
        var bob = store.Join(lobby.Id, "Bob", Version).Token;
        Assert.Equal("Bob", lobby.LeaderName);
        Assert.Equal("Bob", lobby.Leader!.Name);
        store.UpdateSettings(lobby.Id, bob, regicide: true);
        foreach (var token in new[] { bob, chloe, alice })
        {
            store.UpdateMe(lobby.Id, token, ready: true);
        }
        store.Start(lobby.Id, bob);
    }

    [Fact]
    public void ALobbyFullWithoutTheLeaderGetsANewOne()
    {
        var (lobby, _, _) = BackWithoutTheLeader(closeOthers: true);
        Assert.Equal("Bob", lobby.LeaderName);
        store.Join(lobby.Id, "Dan", Version);
        Assert.Equal("Chloé", lobby.LeaderName);
        Assert.Equal(LobbyNotices.LeaderChanged, lobby.Notice);
        LobbyEntryTests.AssertError(LobbyErrors.LobbyFull, () => store.Join(lobby.Id, "Bob", Version));
    }

    [Fact]
    public void TheLeaderLeavingTheMatchWithoutComingBackGivesANewOne()
    {
        var (lobby, chloe, alice) = BackWithoutTheLeader();
        store.ServerLeft(matchId, " bob ");
        // He may be on his way back ("Retour au salon" leaves the server first).
        for (var t = 1; t < LobbyRules.LeaderReturnGrace.TotalSeconds; t++)
        {
            clock.UtcNow += TimeSpan.FromSeconds(1);
            store.Read(lobby.Id, alice);
            Assert.Equal("Bob", store.Read(lobby.Id, chloe).LeaderName);
        }
        clock.UtcNow += TimeSpan.FromSeconds(1);
        Assert.Equal("Chloé", store.Read(lobby.Id, chloe).LeaderName);
        store.UpdateSettings(lobby.Id, chloe, regicide: true);
        // Back later, he is a player like the others.
        store.Join(lobby.Id, "Bob", Version);
        Assert.Equal("Chloé", lobby.LeaderName);
    }

    [Fact]
    public void TheLeaderLeavingThenComingBackInTimeKeepsTheLead()
    {
        var (lobby, _, _) = BackWithoutTheLeader();
        store.ServerLeft(matchId, "Bob");
        clock.UtcNow += TimeSpan.FromSeconds(3);
        var bob = store.Join(lobby.Id, "Bob", Version).Token;
        for (var t = 0; t < LobbyRules.LeaderReturnGrace.TotalSeconds; t++)
        {
            clock.UtcNow += TimeSpan.FromSeconds(1);
            store.Read(lobby.Id, bob);
        }
        Assert.Equal("Bob", store.Read(lobby.Id, bob).LeaderName);
    }

    [Fact]
    public void AnotherPlayerLeavingTheMatchChangesNothing()
    {
        var (lobby, chloe, alice) = BackWithoutTheLeader();
        store.ServerLeft(matchId, "Zoé");
        store.ServerLeft(matchId, "Alice");
        clock.UtcNow += TimeSpan.FromSeconds(1);
        store.Read(lobby.Id, alice);
        Assert.Equal("Bob", store.Read(lobby.Id, chloe).LeaderName);
    }

    [Fact]
    public void TheLeaderLeavingDuringTheMatchLeadsNoMoreAfterIt()
    {
        var created = store.Create("Salon de Bob", "Bob", Version);
        var alice = store.Join(created.Lobby.Id, "Alice", Version).Token;
        store.UpdateMe(created.Lobby.Id, created.Token, ready: true);
        store.UpdateMe(created.Lobby.Id, alice, ready: true);
        var match = store.Start(created.Lobby.Id, created.Token);
        store.ServerListening(match.Id, 7778);
        store.ServerLeft(match.Id, "Bob");
        clock.UtcNow += TimeSpan.FromMinutes(10);
        store.ServerFinished(match.Id, [1]);
        var back = store.Join(created.Lobby.Id, "Alice", Version).Token;
        Assert.Equal("Alice", created.Lobby.LeaderName);
        store.UpdateSettings(created.Lobby.Id, back, regicide: true);
    }

    [Fact]
    public void TheLeaderBackOnTheServerDuringTheMatchStillLeadsAfterIt()
    {
        // M6-Q15: back within R7's window, he counts as present.
        var created = store.Create("Salon de Bob", "Bob", Version);
        var alice = store.Join(created.Lobby.Id, "Alice", Version).Token;
        store.UpdateMe(created.Lobby.Id, created.Token, ready: true);
        store.UpdateMe(created.Lobby.Id, alice, ready: true);
        var match = store.Start(created.Lobby.Id, created.Token);
        store.ServerListening(match.Id, 7778);
        store.ServerLeft(match.Id, "Bob");
        clock.UtcNow += TimeSpan.FromMinutes(1);
        store.ServerBack(match.Id, " bob ");
        Assert.Empty(match.LeftBeforeEnd);
        clock.UtcNow += TimeSpan.FromMinutes(10);
        store.ServerFinished(match.Id, [1]);
        var back = store.Join(created.Lobby.Id, "Alice", Version).Token;
        for (var t = 0; t <= LobbyRules.LeaderReturnGrace.TotalSeconds; t++)
        {
            clock.UtcNow += TimeSpan.FromSeconds(1);
            store.Read(created.Lobby.Id, back);
        }
        Assert.Equal("Bob", created.Lobby.LeaderName);
        LobbyEntryTests.AssertError(LobbyErrors.NotLeader, () => store.UpdateSettings(created.Lobby.Id, back, regicide: true));
        var bob = store.Join(created.Lobby.Id, "Bob", Version).Token;
        Assert.Equal("Bob", created.Lobby.Leader!.Name);
        store.UpdateSettings(created.Lobby.Id, bob, regicide: true);
    }

    [Fact]
    public void AnotherPlayerBackOnTheServerCountsAsPresent()
    {
        var created = store.Create("Salon de Bob", "Bob", Version);
        var alice = store.Join(created.Lobby.Id, "Alice", Version).Token;
        store.UpdateMe(created.Lobby.Id, created.Token, ready: true);
        store.UpdateMe(created.Lobby.Id, alice, ready: true);
        var match = store.Start(created.Lobby.Id, created.Token);
        store.ServerListening(match.Id, 7778);
        store.ServerLeft(match.Id, "Alice");
        store.ServerLeft(match.Id, "Bob");
        store.ServerBack(match.Id, "Alice");
        store.ServerBack(match.Id, "Zoé");
        Assert.Equal(["Bob"], match.LeftBeforeEnd);
    }

    [Fact]
    public void TheAwayLeaderBackOnTheFinishedServerLeadsAgain()
    {
        // Left the finished match's server, then back on it: no 30 s count any more.
        var (lobby, _, _, _) = Played();
        store.ServerLeft(matchId, "Bob");
        store.ServerBack(matchId, "Bob");
        clock.UtcNow += LobbyRules.LeaderReturnGrace + TimeSpan.FromSeconds(1);
        var chloe = store.Join(lobby.Id, "Chloé", Version).Token;
        clock.UtcNow += TimeSpan.FromSeconds(1);
        store.Read(lobby.Id, chloe);
        Assert.Equal("Bob", lobby.LeaderName);
        LobbyEntryTests.AssertError(LobbyErrors.NotLeader, () => store.UpdateSettings(lobby.Id, chloe, regicide: true));
    }

    [Fact]
    public void TheLeaderLeftDuringTheMatchAndBackAfterItLeadsAgain()
    {
        var created = store.Create("Salon de Bob", "Bob", Version);
        var alice = store.Join(created.Lobby.Id, "Alice", Version).Token;
        store.UpdateMe(created.Lobby.Id, created.Token, ready: true);
        store.UpdateMe(created.Lobby.Id, alice, ready: true);
        var match = store.Start(created.Lobby.Id, created.Token);
        store.ServerListening(match.Id, 7778);
        store.ServerLeft(match.Id, "Bob");
        store.ServerFinished(match.Id, [1]);
        store.ServerBack(match.Id, "Bob");
        var back = store.Join(created.Lobby.Id, "Alice", Version).Token;
        Assert.Equal("Bob", created.Lobby.LeaderName);
        LobbyEntryTests.AssertError(LobbyErrors.NotLeader, () => store.UpdateSettings(created.Lobby.Id, back, regicide: true));
    }

    [Fact]
    public void AnotherPlayerBackOnTheFinishedServerChangesNothing()
    {
        var (lobby, _, _, _) = Played();
        store.ServerLeft(matchId, "Bob");
        store.ServerBack(matchId, "Alice");
        clock.UtcNow += LobbyRules.LeaderReturnGrace;
        Assert.Equal("Bob", store.Read(lobby.Id, null).LeaderName);
        store.Join(lobby.Id, "Chloé", Version);
        Assert.Equal("Chloé", lobby.LeaderName);
        LobbyEntryTests.AssertError(LobbyErrors.BadRequest, () => store.ServerBack(matchId, " "));
        LobbyEntryTests.AssertError(LobbyErrors.MatchNotFound, () => store.ServerBack("M99", "Bob"));
    }

    [Fact]
    public void TheServerStoppingWithoutTheLeaderBackGivesANewOne()
    {
        var (lobby, chloe, alice) = BackWithoutTheLeader();
        launcher.Alive.Remove(matchId);
        store.Read(lobby.Id, alice);
        for (var t = 1; t < LobbyRules.LeaderReturnGrace.TotalSeconds; t++)
        {
            clock.UtcNow += TimeSpan.FromSeconds(1);
            store.Read(lobby.Id, alice);
            Assert.Equal("Bob", store.Read(lobby.Id, chloe).LeaderName);
        }
        clock.UtcNow += TimeSpan.FromSeconds(1);
        Assert.Equal("Chloé", store.Read(lobby.Id, chloe).LeaderName);
    }

    [Fact]
    public void NobodyBackKeepsTheAwayLeadersNameThenTheFirstBackLeads()
    {
        var (lobby, _, _, _) = Played();
        store.ServerLeft(matchId, "Bob");
        clock.UtcNow += LobbyRules.LeaderReturnGrace;
        Assert.Equal("Bob", store.Read(lobby.Id, null).LeaderName);
        var chloe = store.Join(lobby.Id, "Chloé", Version).Token;
        Assert.Equal("Chloé", lobby.LeaderName);
        store.UpdateSettings(lobby.Id, chloe, regicide: true);
    }
}
