using HOLManager.Lobbies;

namespace HOLManager.Tests;

public sealed class FakeClock : IClock
{
    public DateTimeOffset UtcNow { get; set; } = new(2026, 10, 23, 12, 0, 0, TimeSpan.Zero);
}

/** R2.4: lobbies and entering (Manager/PROTOCOL.md). */
public class LobbyEntryTests
{
    internal const string Version = "2026-10-23-test";
    private readonly LobbyStore store = new(Version, new FakeClock());

    internal static void AssertError(string error, Action action) =>
        Assert.Equal(error, Assert.Throws<LobbyException>(action).Error);

    [Fact]
    public void CreateEntersTheCreatorAsLeader()
    {
        var result = store.Create("Salon de Bob", "Bob", Version);
        var lobby = result.Lobby;
        Assert.Equal("Salon de Bob", lobby.Name);
        Assert.Equal(LobbyState.Lobby, lobby.State);
        Assert.Equal(0, result.Slot);
        Assert.Equal("Bob", lobby.Leader?.Name);
        Assert.Equal(result.Token, lobby.Leader?.Token);
        Assert.Equal(SlotState.Player, lobby.Slots[0].State);
        Assert.All(lobby.Slots.Skip(1), s => Assert.Equal(SlotState.Open, s.State));
        Assert.Equal(8, lobby.Slots.Count);
        var bob = lobby.Slots[0].Player!;
        Assert.Equal((0, 0, false, "DawnOrder"), (bob.ColorIndex, bob.Team, bob.Ready, bob.Faction));
        Assert.Equal(("Plain", false, false, false),
            (lobby.Settings.Map, lobby.Settings.Nomad, lobby.Settings.Regicide, lobby.Settings.Catastrophe));
        Assert.Same(lobby, store.Find(lobby.Id));
        Assert.Equal(1, store.Count);
    }

    [Fact]
    public void LobbiesGetDistinctIds()
    {
        var a = store.Create("A", "Bob", Version).Lobby;
        var b = store.Create("A", "Bob", Version).Lobby;
        Assert.NotEqual(a.Id, b.Id);
        Assert.StartsWith("L", a.Id);
        Assert.Equal(2, store.Count);
    }

    [Fact]
    public void TokensAre32HexAndDistinct()
    {
        var lobby = store.Create("A", "Bob", Version);
        var alice = store.Join(lobby.Lobby.Id, "Alice", Version);
        Assert.Matches("^[0-9a-f]{32}$", lobby.Token);
        Assert.Matches("^[0-9a-f]{32}$", alice.Token);
        Assert.NotEqual(lobby.Token, alice.Token);
    }

    [Fact]
    public void JoinGivesNextOpenSlotAndFirstFreeColorAndKeepsTheLeader()
    {
        var id = store.Create("A", "Bob", Version).Lobby.Id;
        var alice = store.Join(id, "Alice", Version);
        Assert.Equal(1, alice.Slot);
        var player = alice.Lobby.Slots[1].Player!;
        Assert.Equal(("Alice", 1, 0, false), (player.Name, player.ColorIndex, player.Team, player.Ready));
        Assert.Equal("Bob", alice.Lobby.Leader?.Name);
    }

    [Fact]
    public void JoinSkipsClosedSlotsAndTakesFirstFreeColor()
    {
        var bob = store.Create("A", "Bob", Version);
        bob.Lobby.Slots[1].Closed = true;
        bob.Lobby.Slots[0].Player!.ColorIndex = 1;
        var alice = store.Join(bob.Lobby.Id, "Alice", Version);
        Assert.Equal(2, alice.Slot);
        Assert.Equal(0, alice.Lobby.Slots[2].Player!.ColorIndex);
    }

    [Fact]
    public void NamesAreTrimmed()
    {
        var bob = store.Create("  Salon  ", " Bob ", Version);
        Assert.Equal("Salon", bob.Lobby.Name);
        Assert.Equal("Bob", bob.Lobby.Leader?.Name);
    }

    [Fact]
    public void NameTakenIgnoresCase()
    {
        var id = store.Create("A", "Chloé", Version).Lobby.Id;
        AssertError(LobbyErrors.NameTaken, () => store.Join(id, "CHLOÉ", Version));
        AssertError(LobbyErrors.NameTaken, () => store.Join(id, " chloé", Version));
    }

    [Fact]
    public void EightPlayersAtMost()
    {
        var id = store.Create("A", "P0", Version).Lobby.Id;
        for (var i = 1; i < 8; i++)
        {
            Assert.Equal(i, store.Join(id, $"P{i}", Version).Slot);
        }
        AssertError(LobbyErrors.LobbyFull, () => store.Join(id, "P8", Version));
    }

    [Fact]
    public void ClosedSlotsCountForFull()
    {
        var lobby = store.Create("A", "P0", Version).Lobby;
        for (var i = 2; i < 8; i++)
        {
            lobby.Slots[i].Closed = true;
        }
        store.Join(lobby.Id, "P1", Version);
        AssertError(LobbyErrors.LobbyFull, () => store.Join(lobby.Id, "P2", Version));
    }

    [Fact]
    public void OtherVersionIsRefusedOnJoinWithBothVersions()
    {
        var id = store.Create("A", "Bob", Version).Lobby.Id;
        var e = Assert.Throws<LobbyException>(() => store.Join(id, "Alice", "2026-10-22-old"));
        Assert.Equal(LobbyErrors.VersionMismatch, e.Error);
        Assert.Contains(Version, e.Message);
        Assert.Contains("2026-10-22-old", e.Message);
    }

    [Fact]
    public void OtherVersionIsRefusedOnCreate()
    {
        AssertError(LobbyErrors.VersionMismatch, () => store.Create("A", "Bob", "dev"));
        Assert.Equal(0, store.Count);
    }

    [Fact]
    public void DevServesDevOnly()
    {
        var dev = new LobbyStore("dev", new FakeClock());
        dev.Create("A", "Bob", "dev");
        AssertError(LobbyErrors.VersionMismatch, () => dev.Create("A", "Bob", Version));
    }

    [Fact]
    public void UnknownLobbyIsNotFound()
    {
        AssertError(LobbyErrors.LobbyNotFound, () => store.Join("L99", "Alice", Version));
        Assert.Null(store.Find("L99"));
    }

    [Theory]
    [InlineData(LobbyState.Starting)]
    [InlineData(LobbyState.InGame)]
    public void NoEntryDuringAMatch(LobbyState state)
    {
        var lobby = store.Create("A", "Bob", Version).Lobby;
        lobby.State = state;
        AssertError(LobbyErrors.LobbyInGame, () => store.Join(lobby.Id, "Alice", Version));
    }

    [Theory]
    [InlineData("A")]
    [InlineData("Abcdefghijklmnop")]
    [InlineData("Chloé")]
    [InlineData("Jean-Luc_2")]
    [InlineData("Bob Smith")]
    [InlineData("ゆき")]
    public void ValidPlayerNames(string name)
    {
        var id = store.Create("A", "Host", Version).Lobby.Id;
        store.Join(id, name, Version);
        store.Create("B", name, Version);
    }

    [Theory]
    [InlineData("")]
    [InlineData("   ")]
    [InlineData("Abcdefghijklmnopq")]
    [InlineData("Bob!")]
    [InlineData("O'Neil")]
    [InlineData("a/b")]
    public void InvalidPlayerNames(string name)
    {
        var id = store.Create("A", "Host", Version).Lobby.Id;
        AssertError(LobbyErrors.InvalidName, () => store.Join(id, name, Version));
        AssertError(LobbyErrors.InvalidName, () => store.Create("B", name, Version));
        Assert.Equal(1, store.Count);
    }

    [Theory]
    [InlineData("Salon de Bob")]
    [InlineData("Bob's 2v2")]
    [InlineData("Abcdefghijklmnopqrstuvwx")]
    public void ValidLobbyNames(string name) => store.Create(name, "Bob", Version);

    [Theory]
    [InlineData("")]
    [InlineData("Abcdefghijklmnopqrstuvwxy")]
    [InlineData("Salon #1")]
    public void InvalidLobbyNames(string name)
    {
        AssertError(LobbyErrors.InvalidName, () => store.Create(name, "Bob", Version));
        Assert.Equal(0, store.Count);
    }

    [Fact]
    public void MissingFieldsAreBadRequests()
    {
        AssertError(LobbyErrors.BadRequest, () => store.Create(null, "Bob", Version));
        AssertError(LobbyErrors.BadRequest, () => store.Create("A", null, Version));
        AssertError(LobbyErrors.BadRequest, () => store.Create("A", "Bob", null));
        var id = store.Create("A", "Bob", Version).Lobby.Id;
        AssertError(LobbyErrors.BadRequest, () => store.Join(id, null, Version));
        AssertError(LobbyErrors.BadRequest, () => store.Join(id, "Alice", null));
        Assert.Equal(1, store.Count);
    }

    [Fact]
    public void JoinTimeIsTheClocks()
    {
        var clock = new FakeClock();
        var timed = new LobbyStore(Version, clock);
        var bob = timed.Create("A", "Bob", Version);
        Assert.Equal(clock.UtcNow, bob.Lobby.Leader?.LastSeen);
    }
}
