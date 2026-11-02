using HOLManager.Api;
using HOLManager.Lobbies;

namespace HOLManager.Tests;

/**
 * SP1: spectators of the lobby (Manager/PROTOCOL.md, « Spectateur de salon »): present without a slot, a colour, a
 * team nor a "Prêt"; the leader may be one; no limit to their number; they follow the match and come back as spectators.
 */
public class LobbySpectatorTests
{
    private const string Version = LobbyEntryTests.Version;
    private readonly FakeClock clock = new();
    private readonly FakeLauncher launcher = new();
    private readonly LobbyStore store;

    public LobbySpectatorTests() => store = new LobbyStore(Version, clock, launcher: launcher);

    private static void AssertError(string error, Action action) => LobbyEntryTests.AssertError(error, action);

    /** Bob (leader) and Alice at the lobby, not ready. */
    private (Lobby Lobby, string Bob, string Alice) Two()
    {
        var created = store.Create("Salon de Bob", "Bob", Version);
        var alice = store.Join(created.Lobby.Id, "Alice", Version);
        return (created.Lobby, created.Token, alice.Token);
    }

    [Fact]
    public void JoinAsSpectatorTakesNoSlot()
    {
        var (lobby, _, _) = Two();
        var carol = store.Join(lobby.Id, "Carol", Version, spectator: true);
        Assert.Equal(-1, carol.Slot);
        Assert.Equal(["Carol"], lobby.Spectators.Select(s => s.Name));
        Assert.Equal(2, lobby.Players.Count());
        Assert.Equal(2, lobby.Slots.Count(s => s.State == SlotState.Player));
        Assert.Equal(6, lobby.Slots.Count(s => s.State == SlotState.Open));
    }

    [Fact]
    public void AFullLobbyStillLetsSpectatorsIn_WithoutLimit()
    {
        var (lobby, bob, _) = Two();
        for (var i = 2; i < LobbyRules.SlotCount; i++)
        {
            store.Join(lobby.Id, $"P{i}", Version);
        }
        AssertError(LobbyErrors.LobbyFull, () => store.Join(lobby.Id, "Late", Version));
        for (var i = 0; i < 20; i++)
        {
            store.Join(lobby.Id, $"Spec{i}", Version, spectator: true);
        }
        Assert.Equal(20, lobby.Spectators.Count);
        Assert.Equal(8, lobby.Players.Count());
        Assert.Equal("Bob", lobby.Leader?.Name);
        Assert.NotNull(bob);
    }

    [Fact]
    public void ANameIsUniqueAmongPlayersAndSpectators()
    {
        var (lobby, _, _) = Two();
        store.Join(lobby.Id, "Carol", Version, spectator: true);
        AssertError(LobbyErrors.NameTaken, () => store.Join(lobby.Id, "carol", Version, spectator: true));
        AssertError(LobbyErrors.NameTaken, () => store.Join(lobby.Id, "Carol", Version));
        AssertError(LobbyErrors.NameTaken, () => store.Join(lobby.Id, "alice", Version, spectator: true));
    }

    [Fact]
    public void ASpectatorIsPresentOnlyWhileHeReads()
    {
        var (lobby, bob, _) = Two();
        var carol = store.Join(lobby.Id, "Carol", Version, spectator: true);
        clock.UtcNow += LobbyRules.SilenceLimit - TimeSpan.FromSeconds(1);
        store.Read(lobby.Id, carol.Token);
        store.Read(lobby.Id, bob);
        clock.UtcNow += TimeSpan.FromSeconds(2);
        store.Read(lobby.Id, bob);
        Assert.Equal(["Carol"], lobby.Spectators.Select(s => s.Name));
        clock.UtcNow += TimeSpan.FromSeconds(3);
        store.Read(lobby.Id, bob);
        clock.UtcNow += TimeSpan.FromSeconds(3);
        store.Read(lobby.Id, bob);
        Assert.Empty(lobby.Spectators);
        AssertError(LobbyErrors.NotInLobby, () => store.Read(lobby.Id, carol.Token));
    }

    [Fact]
    public void ThePlayerWhoBecomesSpectatorFreesHisSlotAndHisColour()
    {
        var (lobby, _, alice) = Two();
        Assert.Equal(1, lobby.Slots[1].Player!.ColorIndex);
        store.UpdateMe(lobby.Id, alice, ready: true);
        store.UpdateMe(lobby.Id, alice, spectator: true);
        Assert.Equal(SlotState.Open, lobby.Slots[1].State);
        Assert.Equal(["Alice"], lobby.Spectators.Select(s => s.Name));
        Assert.False(lobby.Spectators[0].Ready);
        // Her colour is free for the next one, who takes the first slot and the first free colour (1).
        var chloe = store.Join(lobby.Id, "Chloé", Version);
        Assert.Equal((1, 1), (chloe.Slot, lobby.Slots[1].Player!.ColorIndex));
    }

    [Fact]
    public void ASpectatorBecomesAPlayerAgainOnTheFirstOpenSlot_ColourBackIfFree()
    {
        var (lobby, _, alice) = Two();
        store.UpdateMe(lobby.Id, alice, colorIndex: 5, team: 2);
        store.UpdateMe(lobby.Id, alice, spectator: true);
        store.UpdateMe(lobby.Id, alice, spectator: false);
        var me = lobby.Slots[1].Player!;
        Assert.Equal(("Alice", 1, 5, 0, false), (me.Name, me.SlotIndex, me.ColorIndex, me.Team, me.Ready));
        Assert.Empty(lobby.Spectators);

        // Her colour taken meanwhile: the first free one.
        store.UpdateMe(lobby.Id, alice, spectator: true);
        var chloe = store.Join(lobby.Id, "Chloé", Version).Token;
        store.UpdateMe(lobby.Id, chloe, colorIndex: 5);
        store.UpdateMe(lobby.Id, alice, spectator: false);
        Assert.Equal(1, lobby.Players.Single(p => p.Name == "Alice").ColorIndex);
    }

    [Fact]
    public void ASpectatorCannotTakeASlotInAFullLobby_AndChangesNothing()
    {
        var (lobby, _, _) = Two();
        var carol = store.Join(lobby.Id, "Carol", Version, spectator: true);
        for (var i = 2; i < LobbyRules.SlotCount; i++)
        {
            store.Join(lobby.Id, $"P{i}", Version);
        }
        AssertError(LobbyErrors.LobbyFull, () => store.UpdateMe(lobby.Id, carol.Token, spectator: false));
        Assert.Equal(["Carol"], lobby.Spectators.Select(s => s.Name));
    }

    [Fact]
    public void ASpectatorSetsNoColourTeamOrReady()
    {
        var (lobby, _, alice) = Two();
        store.UpdateMe(lobby.Id, alice, spectator: true);
        AssertError(LobbyErrors.BadRequest, () => store.UpdateMe(lobby.Id, alice, colorIndex: 4));
        AssertError(LobbyErrors.BadRequest, () => store.UpdateMe(lobby.Id, alice, team: 1));
        AssertError(LobbyErrors.BadRequest, () => store.UpdateMe(lobby.Id, alice, ready: true));
        // Becoming a player in the same request lets him set them.
        store.UpdateMe(lobby.Id, alice, colorIndex: 4, ready: true, spectator: false);
        var me = lobby.Players.Single(p => p.Name == "Alice");
        Assert.Equal((4, true), (me.ColorIndex, me.Ready));
    }

    [Fact]
    public void ALeaderWhoBecomesSpectatorStaysLeader_AndStillSetsAndStarts()
    {
        var (lobby, bob, alice) = Two();
        store.UpdateMe(lobby.Id, bob, spectator: true);
        Assert.Equal("Bob", lobby.Leader?.Name);
        Assert.Equal("Bob", lobby.LeaderName);
        // Only one player and no AI: not enough to start; with an AI it starts without Bob playing.
        AssertError(LobbyErrors.NotEnoughPlayers, () => store.Start(lobby.Id, bob));
        store.UpdateSettings(lobby.Id, bob, nomad: true);
        store.SetSlotAI(lobby.Id, bob, 3);
        store.UpdateMe(lobby.Id, alice, ready: true);
        AssertError(LobbyErrors.NotLeader, () => store.Start(lobby.Id, alice));
        var match = store.Start(lobby.Id, bob);
        Assert.Equal(["Alice"], match.Config.Slots.Where(s => s.State == "Human").Select(s => s.PlayerName));
    }

    [Fact]
    public void WhenTheLeaderLeavesTheEarliestPresentLeads_ASpectatorToo()
    {
        var (lobby, bob, _) = Two();
        store.Join(lobby.Id, "Carol", Version, spectator: true);
        store.Leave(lobby.Id, bob);
        // Alice entered before Carol.
        Assert.Equal("Alice", lobby.Leader?.Name);

        var (second, bob2, alice2) = SecondLobby();
        store.UpdateMe(second.Id, alice2, spectator: true);
        store.Leave(second.Id, bob2);
        Assert.Equal("Alice", second.Leader?.Name);
        Assert.Equal(LobbyNotices.LeaderChanged, second.Notice);
    }

    private (Lobby, string, string) SecondLobby()
    {
        var created = store.Create("Autre", "Bob", Version);
        var alice = store.Join(created.Lobby.Id, "Alice", Version);
        return (created.Lobby, created.Token, alice.Token);
    }

    [Fact]
    public void ALobbyOfSpectatorsAloneIsKept_AndDeletedWhenTheLastLeaves()
    {
        var (lobby, bob, alice) = Two();
        var carol = store.Join(lobby.Id, "Carol", Version, spectator: true);
        store.Leave(lobby.Id, bob);
        store.Leave(lobby.Id, alice);
        Assert.Same(lobby, store.Find(lobby.Id));
        Assert.Empty(lobby.Players);
        Assert.Equal("Carol", lobby.Leader?.Name);
        store.Leave(lobby.Id, carol.Token);
        Assert.Null(store.Find(lobby.Id));
    }

    [Fact]
    public void SpectatorsDoNotBlockLaunchNorCountAsPlayers()
    {
        var (lobby, bob, alice) = Two();
        store.Join(lobby.Id, "Carol", Version, spectator: true);
        store.UpdateMe(lobby.Id, bob, ready: true);
        store.UpdateMe(lobby.Id, alice, ready: true);
        var match = store.Start(lobby.Id, bob);
        Assert.Equal(["Alice", "Bob"], match.Config.Slots.Where(s => s.State == "Human").Select(s => s.PlayerName).Order());
    }

    [Fact]
    public void AfterTheFinishedMatchTheSpectatorsAreOut_ThenJoinAgainAsSpectators()
    {
        var (lobby, bob, alice) = Two();
        store.Join(lobby.Id, "Carol", Version, spectator: true);
        store.UpdateMe(lobby.Id, bob, ready: true);
        store.UpdateMe(lobby.Id, alice, ready: true);
        var match = store.Start(lobby.Id, bob);
        store.ServerListening(match.Id, 7778);
        store.ServerFinished(match.Id, [0]);
        Assert.Empty(lobby.Spectators);
        Assert.Empty(lobby.Players);
        var back = store.Join(lobby.Id, "Carol", Version, spectator: true);
        Assert.Equal(-1, back.Slot);
        Assert.Equal(["Carol"], lobby.Spectators.Select(s => s.Name));
    }

    [Fact]
    public void ASpectatorLeaderGetsTheLeaderGraceFromTheEnd()
    {
        var (lobby, bob, alice) = Two();
        store.UpdateMe(lobby.Id, bob, spectator: true);
        store.SetSlotAI(lobby.Id, bob, 2);
        store.UpdateMe(lobby.Id, alice, ready: true);
        var match = store.Start(lobby.Id, bob);
        store.ServerListening(match.Id, 7778);
        store.ServerFinished(match.Id, [1]);
        Assert.Equal("Bob", lobby.AwayLeader);
        Assert.Equal(clock.UtcNow + LobbyRules.LeaderReturnGrace, lobby.AwayLeaderUntil);
        // Alice is back first; after the grace she leads.
        var back = store.Join(lobby.Id, "Alice", Version);
        AssertError(LobbyErrors.NotLeader, () => store.UpdateSettings(lobby.Id, back.Token, nomad: true));
        for (var i = 0; i < 9; i++)
        {
            clock.UtcNow += TimeSpan.FromSeconds(4);
            store.Read(lobby.Id, back.Token);
        }
        Assert.Equal("Alice", lobby.LeaderName);
    }

    [Fact]
    public void AServerStoppedKeepsTheSpectatorsLineAndReservation()
    {
        var (lobby, bob, alice) = Two();
        var carol = store.Join(lobby.Id, "Carol", Version, spectator: true);
        store.UpdateMe(lobby.Id, bob, ready: true);
        store.UpdateMe(lobby.Id, alice, ready: true);
        var match = store.Start(lobby.Id, bob);
        store.ServerListening(match.Id, 7778);
        launcher.Alive.Clear();
        store.Housekeep();
        Assert.Equal(LobbyState.Lobby, lobby.State);
        Assert.Equal(["Carol"], lobby.Spectators.Select(s => s.Name));
        // Silent for more than 5 s but reserved for 2 min.
        clock.UtcNow += TimeSpan.FromSeconds(30);
        store.Housekeep();
        Assert.Equal(["Carol"], lobby.Spectators.Select(s => s.Name));
        var again = store.Join(lobby.Id, "Carol", Version, spectator: true);
        Assert.NotEqual(carol.Token, again.Token);
        Assert.Single(lobby.Spectators);
    }

    [Fact]
    public void TheViewsShowTheSpectatorsAndWhoYouAre()
    {
        var (lobby, bob, _) = Two();
        var carol = store.Join(lobby.Id, "Carol", Version, spectator: true);
        var json = System.Text.Json.JsonSerializer.Serialize(store.Atomic(() => LobbyViews.State(store, lobby, carol.Token)));
        Assert.Contains("\"you\":\"Carol\"", json);
        Assert.Contains("\"spectators\":[{\"playerName\":\"Carol\"}]", json);
        var summary = System.Text.Json.JsonSerializer.Serialize(store.Atomic(() => LobbyViews.Summary(lobby, clock.UtcNow)));
        Assert.Contains("\"players\":2", summary);
        Assert.Contains("\"spectators\":1", summary);
        Assert.Contains("\"canSpectate\":true", summary);
        Assert.NotNull(bob);
    }
}
