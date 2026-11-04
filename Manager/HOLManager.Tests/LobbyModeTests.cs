using System.Text.Json;
using System.Text.Json.Nodes;
using HOLManager.Api;
using HOLManager.Lobbies;
using HOLManager.Matches;

namespace HOLManager.Tests;

/** D5 (DiplomacyInProgress.md): the lobby's match mode (Standard, Diplomacy) through the settings, the view and match.json. */
public class LobbyModeTests
{
    private const string Version = LobbyEntryTests.Version;

    private readonly MapLauncher launcher = new();
    private readonly LobbyStore store;
    private readonly Lobby lobby;
    private readonly string bob;
    private readonly string alice;

    public LobbyModeTests()
    {
        store = new LobbyStore(Version, new FakeClock(), launcher: launcher);
        var created = store.Create("A", "Bob", Version);
        lobby = created.Lobby;
        bob = created.Token;
        alice = store.Join(lobby.Id, "Alice", Version).Token;
    }

    [Fact]
    public void NewLobbyIsStandard()
    {
        Assert.Equal("Standard", lobby.Settings.Mode);
        Assert.Equal(["Standard", "Diplomacy"], LobbyRules.Modes);
    }

    [Fact]
    public void ModeIsTheLeadersResetsEveryReadyAndRefusesAnUnknownMode()
    {
        LobbyEntryTests.AssertError(LobbyErrors.NotLeader, () => store.UpdateSettings(lobby.Id, alice, mode: "Diplomacy"));
        LobbyEntryTests.AssertError(LobbyErrors.BadRequest, () => store.UpdateSettings(lobby.Id, bob, mode: "Battlefield"));
        store.UpdateMe(lobby.Id, bob, ready: true);
        store.UpdateMe(lobby.Id, alice, ready: true);
        store.UpdateSettings(lobby.Id, bob, mode: "standard");
        Assert.True(lobby.Slots[1].Player!.Ready);
        store.UpdateSettings(lobby.Id, bob, mode: "Diplomacy");
        Assert.Equal("Diplomacy", lobby.Settings.Mode);
        Assert.False(lobby.Slots[0].Player!.Ready || lobby.Slots[1].Player!.Ready);
        store.UpdateSettings(lobby.Id, bob, nomad: true, fog: "Explored");
        Assert.Equal("Diplomacy", lobby.Settings.Mode);
        Assert.Equal("Explored", lobby.Settings.Fog);
    }

    [Fact]
    public void StartWritesTheMode()
    {
        store.UpdateSettings(lobby.Id, bob, mode: "Diplomacy");
        store.UpdateMe(lobby.Id, bob, ready: true);
        store.UpdateMe(lobby.Id, alice, ready: true);
        var config = store.Start(lobby.Id, bob).Config;
        Assert.Equal("Diplomacy", config.Mode);
        Assert.Equal("Diplomacy", (string?)JsonNode.Parse(config.ToJson())!["mode"]);
        Assert.Equal("Standard", MatchConfig.FromLobby(new LobbyStore(Version, new FakeClock()).Create("B", "Cy", Version).Lobby, store.Maps[0], 1).Mode);
    }

    [Fact]
    public void TheViewGivesTheMode()
    {
        store.UpdateSettings(lobby.Id, bob, mode: "Diplomacy");
        var state = JsonNode.Parse(JsonSerializer.Serialize(store.Atomic(() => LobbyViews.State(store, lobby, bob))))!;
        Assert.Equal("Diplomacy", (string?)state["settings"]!["mode"]);
    }
}
