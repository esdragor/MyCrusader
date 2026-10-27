using System.Net;
using System.Net.Http.Json;
using System.Text.Json;
using System.Text.Json.Nodes;
using HOLManager.Api;
using HOLManager.Lobbies;
using HOLManager.Matches;
using Microsoft.AspNetCore.Mvc.Testing;

namespace HOLManager.Tests;

/** A launcher that keeps the map it was given (TC3: the file of the lobby's size). */
internal sealed class MapLauncher : IServerLauncher
{
    public MapInfo? Map { get; private set; }

    public int Launch(string matchId, MapInfo map, MatchConfig config, string? gameVersion = null)
    {
        Map = map;
        return 7778;
    }

    public int Running => 0;

    public bool HasExited(string matchId) => false;

    public void Stop(string matchId) { }
}

/** TC3 (map sizes, TC-Q3 to TC-Q6): the lobby's size and "Équipes ensemble", match.json version 3. */
public class LobbyMapSizeTests
{
    private const string Version = LobbyEntryTests.Version;

    private static void AssertError(string error, Action action) => LobbyEntryTests.AssertError(error, action);

    private readonly MapLauncher launcher = new();
    private readonly LobbyStore store;
    private readonly Lobby lobby;
    private readonly string bob;
    private readonly string alice;

    public LobbyMapSizeTests()
    {
        store = new LobbyStore(Version, new FakeClock(), launcher: launcher);
        var created = store.Create("A", "Bob", Version);
        lobby = created.Lobby;
        bob = created.Token;
        alice = store.Join(lobby.Id, "Alice", Version).Token;
    }

    private IEnumerable<SlotState> States => lobby.Slots.Select(s => s.State);

    /** Tools/Data/MapSizes.csv, found from the test's folder up to the project's. */
    private static string MapSizesCsv()
    {
        for (var dir = new DirectoryInfo(AppContext.BaseDirectory); dir != null; dir = dir.Parent)
        {
            var path = Path.Combine(dir.FullName, "Tools", "Data", "MapSizes.csv");
            if (File.Exists(path))
            {
                return path;
            }
        }
        throw new FileNotFoundException("Tools/Data/MapSizes.csv not found above " + AppContext.BaseDirectory);
    }

    [Fact]
    public void DefaultMapIsPlainInTheSizesOfMapSizesCsv()
    {
        var plain = Assert.Single(LobbyRules.DefaultMaps);
        Assert.Equal("Plain", plain.Name);
        var csv = File.ReadAllLines(MapSizesCsv()).Skip(1).Where(l => l.Trim() != "").Select(l => l.Split(','))
            .Select(c => (c[0], int.Parse(c[2]))).ToList();
        Assert.Equal(csv, plain.Sizes.Select(s => (s.Key, s.Places)));
        Assert.Equal([("Small", 2), ("Medium", 4), ("Normal", 6), ("Large", 8), ("Huge", 8)], csv);
        Assert.Equal(8, plain.MaxPlayers);
        Assert.Equal("/Game/Maps/Plain.Plain", plain.ObjectPath);
        Assert.Equal("/Game/Maps/Plain_Small", plain.File("Small").PackagePath);
        Assert.Equal(2, plain.Places("Small"));
    }

    [Fact]
    public void NewLobbyIsLargeTeamsTogetherEightSlotsOpen()
    {
        Assert.Equal(("Plain", "Large", true), (lobby.Settings.Map, lobby.Settings.Size, lobby.Settings.TeamsTogether));
        Assert.Equal(6, States.Count(s => s == SlotState.Open));
    }

    [Fact]
    public void SmallerSizeClosesTheSlotsPastItsPlacesAndMovesThePlayersBelow()
    {
        var carl = store.Join(lobby.Id, "Carl", Version).Token;
        store.Leave(lobby.Id, alice);
        store.UpdateMe(lobby.Id, carl, colorIndex: 6, team: 2, ready: true);
        store.UpdateSettings(lobby.Id, bob, size: "Small");
        Assert.Equal("Small", lobby.Settings.Size);
        Assert.Equal([SlotState.Player, SlotState.Player], States.Take(2));
        Assert.All(lobby.Slots.Skip(2), s => Assert.Equal(SlotState.Closed, s.State));
        var moved = lobby.Slots[1].Player!;
        Assert.Equal(("Carl", 1, 6, 2, false), (moved.Name, moved.SlotIndex, moved.ColorIndex, moved.Team, moved.Ready));
        AssertError(LobbyErrors.LobbyFull, () => store.Join(lobby.Id, "Dan", Version));
        // Carl's token still finds him on his new slot.
        store.UpdateMe(lobby.Id, carl, ready: true);
        Assert.True(lobby.Slots[1].Player!.Ready);
    }

    [Fact]
    public void SizeTooSmallForThePlayersIsRefused()
    {
        store.Join(lobby.Id, "Carl", Version);
        store.UpdateMe(lobby.Id, bob, ready: true);
        AssertError(LobbyErrors.SlotTaken, () => store.UpdateSettings(lobby.Id, bob, size: "Small", nomad: true));
        Assert.Equal(("Large", false, true), (lobby.Settings.Size, lobby.Settings.Nomad, lobby.Slots[0].Player!.Ready));
        Assert.Equal(SlotState.Open, lobby.Slots[3].State);
        store.UpdateSettings(lobby.Id, bob, size: "Medium");
        Assert.Equal([SlotState.Player, SlotState.Player, SlotState.Player, SlotState.Open], States.Take(4));
    }

    [Fact]
    public void BiggerSizeOpensTheSlotsItAddsOnly()
    {
        store.SetSlot(lobby.Id, bob, 2, open: false);
        store.UpdateSettings(lobby.Id, bob, size: "Medium");
        Assert.Equal([SlotState.Player, SlotState.Player, SlotState.Closed, SlotState.Open], States.Take(4));
        Assert.All(lobby.Slots.Skip(4), s => Assert.Equal(SlotState.Closed, s.State));
        store.UpdateSettings(lobby.Id, bob, size: "Huge");
        // Slot 2, closed by the leader, stays closed; 4 to 7, closed by the size, open again.
        Assert.Equal(SlotState.Closed, lobby.Slots[2].State);
        Assert.All(lobby.Slots.Skip(3), s => Assert.Equal(SlotState.Open, s.State));
    }

    [Fact]
    public void SlotPastTheSizeCannotOpen()
    {
        store.UpdateSettings(lobby.Id, bob, size: "Medium");
        AssertError(LobbyErrors.BadRequest, () => store.SetSlot(lobby.Id, bob, 4, open: true));
        store.SetSlot(lobby.Id, bob, 3, open: false);
        store.SetSlot(lobby.Id, bob, 3, open: true);
    }

    [Fact]
    public void SizeAndTeamsTogetherAreTheLeadersAndResetEveryReady()
    {
        AssertError(LobbyErrors.NotLeader, () => store.UpdateSettings(lobby.Id, alice, size: "Small"));
        AssertError(LobbyErrors.NotLeader, () => store.UpdateSettings(lobby.Id, alice, teamsTogether: false));
        AssertError(LobbyErrors.BadRequest, () => store.UpdateSettings(lobby.Id, bob, size: "Tiny"));
        store.UpdateMe(lobby.Id, bob, ready: true);
        store.UpdateMe(lobby.Id, alice, ready: true);
        store.UpdateSettings(lobby.Id, bob, size: "Large", teamsTogether: true);
        Assert.True(lobby.Slots[1].Player!.Ready);
        store.UpdateSettings(lobby.Id, bob, teamsTogether: false);
        Assert.False(lobby.Settings.TeamsTogether);
        Assert.False(lobby.Slots[0].Player!.Ready || lobby.Slots[1].Player!.Ready);
    }

    [Fact]
    public void StartWritesVersion3AndLaunchesTheFileOfTheSize()
    {
        store.UpdateSettings(lobby.Id, bob, size: "Small", teamsTogether: false);
        store.UpdateMe(lobby.Id, bob, ready: true);
        store.UpdateMe(lobby.Id, alice, ready: true);
        var config = store.Start(lobby.Id, bob).Config;
        Assert.Equal((3, "/Game/Maps/Plain.Plain", "Small", true, false),
            (config.Version, config.Map, config.MapSize, config.DrawPlaces, config.TeamsTogether));
        Assert.Equal("/Game/Maps/Plain_Small", launcher.Map!.PackagePath);
        var json = JsonNode.Parse(config.ToJson())!.AsObject();
        Assert.Equal(["version", "seed", "map", "mapSize", "bDrawPlaces", "bTeamsTogether", "slots", "mode", "bNomad", "bRegicide",
            "bCatastrophe"], json.Select(p => p.Key));
    }

    [Fact]
    public void ViewsGiveTheSizes()
    {
        store.UpdateSettings(lobby.Id, bob, size: "Medium", teamsTogether: false);
        var state = JsonNode.Parse(JsonSerializer.Serialize(store.Atomic(() => LobbyViews.State(store, lobby, bob))))!;
        Assert.Equal(("Medium", false), ((string?)state["settings"]!["size"], (bool)state["settings"]!["teamsTogether"]!));
        var sizes = state["maps"]![0]!["sizes"]!.AsArray();
        Assert.Equal(["Small", "Medium", "Normal", "Large", "Huge"], sizes.Select(s => (string?)s!["key"]));
        Assert.Equal([2, 4, 6, 8, 8], sizes.Select(s => (int)s!["places"]!));
        var summary = JsonNode.Parse(JsonSerializer.Serialize(store.Atomic(() => LobbyViews.Summary(lobby, store.Now))))!;
        Assert.Equal(("Plain", "Medium", 4), ((string?)summary["map"], (string?)summary["size"], (int)summary["maxPlayers"]!));
    }
}

/** TC3: the size and "Équipes ensemble" through PUT /lobbies/{id}/settings. */
public sealed class LobbyMapSizeRoutesTests(WebApplicationFactory<Program> factory) : IClassFixture<WebApplicationFactory<Program>>
{
    private const string Version = LobbyEntryTests.Version;

    [Fact]
    public async Task SettingsCarryTheSizeAndTeamsTogether()
    {
        var client = factory.WithWebHostBuilder(b => b.UseSetting("GameVersion", Version)).CreateClient();
        var created = JsonNode.Parse(await (await client.PostAsJsonAsync("/lobbies",
            new { name = "Salon", playerName = "Bob", gameVersion = Version })).Content.ReadAsStringAsync())!;
        var id = (string)created["lobby"]!["id"]!;
        var request = new HttpRequestMessage(HttpMethod.Put, $"/lobbies/{id}/settings")
        {
            Content = JsonContent.Create(new { size = "Small", teamsTogether = false }),
        };
        request.Headers.Add("X-Player-Token", (string)created["token"]!);
        var response = await client.SendAsync(request);
        Assert.Equal(HttpStatusCode.OK, response.StatusCode);
        var settings = JsonNode.Parse(await response.Content.ReadAsStringAsync())!["settings"]!;
        Assert.Equal(("Small", false), ((string?)settings["size"], (bool)settings["teamsTogether"]!));
    }
}
