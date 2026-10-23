using HOLManager.Lobbies;

namespace HOLManager.Tests;

/** R2.11: back to the lobby after a match (choice 6 of PROTOCOL.md, M6-Q7). */
public class ReturnToLobbyTests
{
    private const string Version = LobbyEntryTests.Version;
    private readonly FakeClock clock = new();
    private readonly FakeLauncher launcher = new();
    private readonly LobbyStore store;

    public ReturnToLobbyTests() => store = new LobbyStore(Version, clock, launcher: launcher);

    /** Bob (leader), Alice (color 3, team 2), Chloé; Nomad on; match played and finished. */
    private (Lobby Lobby, string Bob, string Alice, string Chloe) Finished(bool closeOthers = false)
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
        store.ServerFinished(match.Id, [0]);
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
}
