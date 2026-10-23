using HOLManager.Lobbies;

namespace HOLManager.Tests;

/** R2.6: leaving and silence (Manager/PROTOCOL.md). */
public class LobbyLeaveTests
{
    private const string Version = LobbyEntryTests.Version;

    private static void AssertError(string error, Action action) => LobbyEntryTests.AssertError(error, action);

    private readonly FakeClock clock = new();
    private readonly LobbyStore store;
    private readonly Lobby lobby;
    private readonly string bob;
    private readonly string alice;

    public LobbyLeaveTests()
    {
        store = new(Version, clock);
        var created = store.Create("A", "Bob", Version);
        lobby = created.Lobby;
        bob = created.Token;
        alice = store.Join(lobby.Id, "Alice", Version).Token;
    }

    private void Wait(double seconds) => clock.UtcNow += TimeSpan.FromSeconds(seconds);

    private LobbyPlayer? Player(string name) => lobby.Players.FirstOrDefault(p => p.Name == name);

    [Fact]
    public void LeaveRemovesThePlayerAndFreesSlotAndColor()
    {
        store.Leave(lobby.Id, alice);
        Assert.Null(Player("Alice"));
        Assert.Equal(SlotState.Open, lobby.Slots[1].State);
        var chloe = store.Join(lobby.Id, "Chloé", Version);
        Assert.Equal(1, chloe.Slot);
        Assert.Equal(1, lobby.Slots[1].Player!.ColorIndex);
    }

    [Fact]
    public void LeaveWithUnknownTokenOrLobbyIsRefused()
    {
        AssertError(LobbyErrors.NotInLobby, () => store.Leave(lobby.Id, "nope"));
        AssertError(LobbyErrors.NotInLobby, () => store.Leave(lobby.Id, null));
        AssertError(LobbyErrors.LobbyNotFound, () => store.Leave("L99", alice));
        store.Leave(lobby.Id, alice);
        AssertError(LobbyErrors.NotInLobby, () => store.Leave(lobby.Id, alice));
        Assert.Single(lobby.Players);
    }

    [Fact]
    public void LeaderLeavingPassesToTheEarliestArrival()
    {
        store.Join(lobby.Id, "Chloé", Version);
        store.Leave(lobby.Id, bob);
        Assert.Equal("Alice", lobby.Leader?.Name);
        Assert.Equal(LobbyNotices.LeaderChanged, lobby.Notice);
        // A later arrival in a lower slot does not become leader.
        var dan = store.Join(lobby.Id, "Dan", Version);
        Assert.Equal(0, dan.Slot);
        store.Leave(lobby.Id, alice);
        Assert.Equal("Chloé", lobby.Leader?.Name);
    }

    [Fact]
    public void OtherPlayerLeavingIsNoLeaderNotice()
    {
        store.Leave(lobby.Id, alice);
        Assert.Null(lobby.Notice);
    }

    [Fact]
    public void LeaderNoticeClearsAfter10Seconds()
    {
        store.Leave(lobby.Id, bob);
        Wait(4);
        Assert.Equal(LobbyNotices.LeaderChanged, store.Read(lobby.Id, alice).Notice);
        Wait(4);
        Assert.Equal(LobbyNotices.LeaderChanged, store.Read(lobby.Id, alice).Notice);
        Wait(2);
        Assert.Null(store.Read(lobby.Id, alice).Notice);
    }

    [Fact]
    public void LastPlayerLeavingDeletesTheLobby()
    {
        store.Leave(lobby.Id, alice);
        store.Leave(lobby.Id, bob);
        Assert.Null(store.Find(lobby.Id));
        Assert.Equal(0, store.Count);
        AssertError(LobbyErrors.LobbyNotFound, () => store.Join(lobby.Id, "Chloé", Version));
    }

    [Fact]
    public void LobbyInMatchIsKeptWhenEmpty()
    {
        lobby.State = LobbyState.InGame;
        store.Leave(lobby.Id, alice);
        store.Leave(lobby.Id, bob);
        Assert.Same(lobby, store.Find(lobby.Id));
    }

    [Fact]
    public void ReadReturnsTheLobbyWithOrWithoutToken()
    {
        Assert.Same(lobby, store.Read(lobby.Id, alice));
        Assert.Same(lobby, store.Read(lobby.Id, null));
        AssertError(LobbyErrors.NotInLobby, () => store.Read(lobby.Id, "nope"));
        AssertError(LobbyErrors.LobbyNotFound, () => store.Read("L99", alice));
    }

    [Fact]
    public void FiveSecondsWithoutReadingRemoves()
    {
        Wait(4.9);
        store.Read(lobby.Id, bob);
        Assert.NotNull(Player("Alice"));
        Wait(0.1);
        store.Read(lobby.Id, bob);
        Assert.Null(Player("Alice"));
        Assert.Equal(SlotState.Open, lobby.Slots[1].State);
        AssertError(LobbyErrors.NotInLobby, () => store.Read(lobby.Id, alice));
    }

    [Fact]
    public void ReadingKeepsThePlayer()
    {
        for (var i = 0; i < 10; i++)
        {
            Wait(3);
            store.Read(lobby.Id, bob);
            store.Read(lobby.Id, alice);
        }
        Assert.Equal(2, lobby.Players.Count());
    }

    [Fact]
    public void ReadingWithoutTokenIsNoPresence()
    {
        Wait(3);
        store.Read(lobby.Id, bob);
        Wait(3);
        store.Read(lobby.Id, null);
        Assert.Equal(new[] { "Bob" }, lobby.Players.Select(p => p.Name));
    }

    [Fact]
    public void SilentLeaderPassesTheLead()
    {
        Wait(3);
        store.Read(lobby.Id, alice);
        Wait(3);
        Assert.Equal("Alice", store.Read(lobby.Id, alice).Leader?.Name);
        Assert.Equal(LobbyNotices.LeaderChanged, lobby.Notice);
    }

    [Fact]
    public void SilentLobbyIsDeletedAtTheNextCall()
    {
        Wait(5);
        Assert.Equal(0, store.Count);
        Assert.Null(store.Find(lobby.Id));
    }

    [Fact]
    public void SilentPlayerIsRemovedBeforeAnyCall()
    {
        Wait(3);
        store.Read(lobby.Id, bob);
        Wait(3);
        // Alice's own call comes too late: she is already out.
        AssertError(LobbyErrors.NotInLobby, () => store.UpdateMe(lobby.Id, alice, ready: true));
        // Her name and color are free again.
        var again = store.Join(lobby.Id, "Alice", Version);
        Assert.Equal((1, 1), (again.Slot, lobby.Slots[1].Player!.ColorIndex));
    }

    [Fact]
    public void NoSilenceRuleDuringTheMatch()
    {
        lobby.State = LobbyState.Starting;
        Wait(60);
        Assert.Equal(2, store.Find(lobby.Id)?.Players.Count());
        lobby.State = LobbyState.InGame;
        Wait(600);
        Assert.Equal(2, store.Find(lobby.Id)?.Players.Count());
    }

    [Fact]
    public void LineChangesCountAsPresence()
    {
        Wait(4);
        store.Read(lobby.Id, bob);
        store.UpdateMe(lobby.Id, alice, team: 2);
        Wait(4);
        store.Read(lobby.Id, bob);
        Assert.NotNull(Player("Alice"));
    }
}
