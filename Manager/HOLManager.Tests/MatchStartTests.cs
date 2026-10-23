using System.Text.Json.Nodes;
using HOLManager.Lobbies;
using HOLManager.Matches;

namespace HOLManager.Tests;

/** R2.8: "Start" (leader, everyone ready) and match.json, FHOLMatchConfig version 2. */
public class MatchStartTests
{
    private const string Version = LobbyEntryTests.Version;
    private readonly FakeClock clock = new();
    private readonly LobbyStore store;

    public MatchStartTests() => store = new LobbyStore(Version, clock);

    internal static string ReferencePath => Path.Combine(AppContext.BaseDirectory, "Data", "match_v2_reference.json");

    /** The lobby of the reference file: Bob, slot 1 closed, Alice, Chloé; 2 against 1; Nomad. */
    private (Lobby Lobby, string Bob, string Alice, string Chloe) ReferenceLobby()
    {
        var created = store.Create("Salon de Bob", "Bob", Version);
        var id = created.Lobby.Id;
        store.SetSlot(id, created.Token, 1, open: false);
        var alice = store.Join(id, "Alice", Version).Token;
        var chloe = store.Join(id, "Chloé", Version).Token;
        store.UpdateSettings(id, created.Token, nomad: true);
        store.UpdateMe(id, created.Token, team: 1, ready: true);
        store.UpdateMe(id, alice, colorIndex: 3, team: 2, ready: true);
        store.UpdateMe(id, chloe, colorIndex: 5, team: 1, ready: true);
        return (created.Lobby, created.Token, alice, chloe);
    }

    [Fact]
    public void ReferenceLobbyGivesTheReferenceFile()
    {
        var (lobby, _, _, _) = ReferenceLobby();
        var json = MatchConfig.FromLobby(lobby, store.Maps[0], seed: 184467).ToJson();
        var expected = JsonNode.Parse(File.ReadAllText(ReferencePath));
        Assert.True(JsonNode.DeepEquals(expected, JsonNode.Parse(json)), json);
    }

    [Fact]
    public void FieldNamesAreTheOnesOfFJsonObjectConverter()
    {
        var (lobby, _, _, _) = ReferenceLobby();
        var json = JsonNode.Parse(MatchConfig.FromLobby(lobby, store.Maps[0], seed: 1).ToJson())!.AsObject();
        Assert.Equal(["version", "seed", "map", "slots", "mode", "bNomad", "bRegicide", "bCatastrophe"],
            json.Select(p => p.Key));
        Assert.Equal(["state", "faction", "colorIndex", "team", "playerName"],
            json["slots"]![0]!.AsObject().Select(p => p.Key));
        Assert.Equal(8, json["slots"]!.AsArray().Count);
    }

    [Fact]
    public void StartPutsTheLobbyInStartingWithAMatchId()
    {
        var (lobby, bob, _, _) = ReferenceLobby();
        var match = store.Start(lobby.Id, bob);
        Assert.Matches(@"^M20261023-120000-\d+$", match.Id);
        Assert.Equal(LobbyState.Starting, lobby.State);
        Assert.Same(match, lobby.Match);
        Assert.Equal(MatchConfig.CurrentVersion, match.Config.Version);
        Assert.Equal(["Bob", "", "Alice", "Chloé", "", "", "", ""], match.Config.Slots.Select(s => s.PlayerName));
        Assert.Equal("/Game/Maps/Plain.Plain", match.Config.Map);
        Assert.True(match.Config.Nomad);
    }

    [Fact]
    public void MatchIdsAreDistinct()
    {
        var a = ReferenceLobby();
        var b = ReferenceLobby();
        Assert.NotEqual(store.Start(a.Lobby.Id, a.Bob).Id, store.Start(b.Lobby.Id, b.Bob).Id);
    }

    [Fact]
    public void StartingLobbyRefusesEntryAndKeepsSilentPlayers()
    {
        var (lobby, bob, _, _) = ReferenceLobby();
        store.Start(lobby.Id, bob);
        clock.UtcNow += TimeSpan.FromSeconds(30);
        Assert.Equal(3, lobby.Players.Count());
        LobbyEntryTests.AssertError(LobbyErrors.LobbyInGame, () => store.Join(lobby.Id, "Dan", Version));
        LobbyEntryTests.AssertError(LobbyErrors.LobbyInGame, () => store.Start(lobby.Id, bob));
    }

    [Fact]
    public void OnlyTheLeaderStarts()
    {
        var (lobby, _, alice, _) = ReferenceLobby();
        LobbyEntryTests.AssertError(LobbyErrors.NotLeader, () => store.Start(lobby.Id, alice));
        Assert.Equal(LobbyState.Lobby, lobby.State);
    }

    [Fact]
    public void AloneIsNotEnough()
    {
        var created = store.Create("Seul", "Bob", Version);
        store.UpdateMe(created.Lobby.Id, created.Token, ready: true);
        LobbyEntryTests.AssertError(LobbyErrors.NotEnoughPlayers, () => store.Start(created.Lobby.Id, created.Token));
        Assert.Equal(LobbyState.Lobby, created.Lobby.State);
        Assert.Null(created.Lobby.Match);
    }

    [Fact]
    public void EveryoneMustBeReadyLeaderIncluded()
    {
        var (lobby, bob, alice, _) = ReferenceLobby();
        store.UpdateMe(lobby.Id, alice, ready: false);
        LobbyEntryTests.AssertError(LobbyErrors.NotAllReady, () => store.Start(lobby.Id, bob));
        store.UpdateMe(lobby.Id, alice, ready: true);
        store.UpdateMe(lobby.Id, bob, ready: false);
        LobbyEntryTests.AssertError(LobbyErrors.NotAllReady, () => store.Start(lobby.Id, bob));
        Assert.Equal(LobbyState.Lobby, lobby.State);
    }

    [Fact]
    public void ChecksComeInTheOrderOfTheProtocol()
    {
        // Alone and not ready: not_enough_players comes before not_all_ready.
        var created = store.Create("Seul", "Bob", Version);
        LobbyEntryTests.AssertError(LobbyErrors.NotEnoughPlayers, () => store.Start(created.Lobby.Id, created.Token));
        LobbyEntryTests.AssertError(LobbyErrors.LobbyNotFound, () => store.Start("L99", created.Token));
        LobbyEntryTests.AssertError(LobbyErrors.NotInLobby, () => store.Start(created.Lobby.Id, "nope"));
        // A non-leader who is not ready: not_leader first.
        var alice = store.Join(created.Lobby.Id, "Alice", Version).Token;
        LobbyEntryTests.AssertError(LobbyErrors.NotLeader, () => store.Start(created.Lobby.Id, alice));
    }
}
