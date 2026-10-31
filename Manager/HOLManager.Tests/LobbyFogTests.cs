using System.Text.Json;
using System.Text.Json.Nodes;
using HOLManager.Api;
using HOLManager.Lobbies;
using HOLManager.Matches;

namespace HOLManager.Tests;

/** F8 (D522, FogInProgress.md): the lobby's fog mode (Normal, Explored, Disabled) through the settings, the view and match.json. */
public class LobbyFogTests
{
    private const string Version = LobbyEntryTests.Version;

    private readonly MapLauncher launcher = new();
    private readonly LobbyStore store;
    private readonly Lobby lobby;
    private readonly string bob;
    private readonly string alice;

    public LobbyFogTests()
    {
        store = new LobbyStore(Version, new FakeClock(), launcher: launcher);
        var created = store.Create("A", "Bob", Version);
        lobby = created.Lobby;
        bob = created.Token;
        alice = store.Join(lobby.Id, "Alice", Version).Token;
    }

    [Fact]
    public void NewLobbyIsNormal()
    {
        Assert.Equal("Normal", lobby.Settings.Fog);
        Assert.Equal(["Normal", "Explored", "Disabled"], LobbyRules.FogModes);
    }

    [Fact]
    public void FogIsTheLeadersResetsEveryReadyAndRefusesAnUnknownMode()
    {
        LobbyEntryTests.AssertError(LobbyErrors.NotLeader, () => store.UpdateSettings(lobby.Id, alice, fog: "Explored"));
        LobbyEntryTests.AssertError(LobbyErrors.BadRequest, () => store.UpdateSettings(lobby.Id, bob, fog: "Dark"));
        store.UpdateMe(lobby.Id, bob, ready: true);
        store.UpdateMe(lobby.Id, alice, ready: true);
        store.UpdateSettings(lobby.Id, bob, fog: "normal");
        Assert.True(lobby.Slots[1].Player!.Ready);
        store.UpdateSettings(lobby.Id, bob, fog: "Explored");
        Assert.Equal("Explored", lobby.Settings.Fog);
        Assert.False(lobby.Slots[0].Player!.Ready || lobby.Slots[1].Player!.Ready);
        store.UpdateSettings(lobby.Id, bob, nomad: true);
        Assert.Equal("Explored", lobby.Settings.Fog);
    }

    [Fact]
    public void StartWritesTheFogMode()
    {
        store.UpdateSettings(lobby.Id, bob, fog: "Disabled");
        store.UpdateMe(lobby.Id, bob, ready: true);
        store.UpdateMe(lobby.Id, alice, ready: true);
        var config = store.Start(lobby.Id, bob).Config;
        Assert.Equal("Disabled", config.FogMode);
        Assert.Equal("Disabled", (string?)JsonNode.Parse(config.ToJson())!["fogMode"]);
        Assert.Equal("Normal", MatchConfig.FromLobby(new LobbyStore(Version, new FakeClock()).Create("B", "Cy", Version).Lobby, store.Maps[0], 1).FogMode);
    }

    [Fact]
    public void TheViewGivesTheFog()
    {
        store.UpdateSettings(lobby.Id, bob, fog: "Explored");
        var state = JsonNode.Parse(JsonSerializer.Serialize(store.Atomic(() => LobbyViews.State(store, lobby, bob))))!;
        Assert.Equal("Explored", (string?)state["settings"]!["fog"]);
    }
}
