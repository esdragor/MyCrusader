using System.Text.Json;
using System.Text.Json.Nodes;
using System.Text.RegularExpressions;
using HOLManager.Api;
using HOLManager.Lobbies;
using HOLManager.Matches;

namespace HOLManager.Tests;

/** B2 (BiomeInProgress.md, D651-D656): the lobby's biome (the ambience: ground, light, sky, mist, generic decor) through the settings, the view and match.json. */
public class LobbyBiomeTests
{
    private const string Version = LobbyEntryTests.Version;

    private readonly MapLauncher launcher = new();
    private readonly LobbyStore store;
    private readonly Lobby lobby;
    private readonly string bob;
    private readonly string alice;

    public LobbyBiomeTests()
    {
        store = new LobbyStore(Version, new FakeClock(), launcher: launcher);
        var created = store.Create("A", "Bob", Version);
        lobby = created.Lobby;
        bob = created.Token;
        alice = store.Join(lobby.Id, "Alice", Version).Token;
    }

    /** The keys of Script/Biomes/HOLBiomeRules.as (the one table of the five biomes), found from the test's folder up to the project's. */
    private static IReadOnlyList<string> ScriptBiomeKeys()
    {
        for (var dir = new DirectoryInfo(AppContext.BaseDirectory); dir != null; dir = dir.Parent)
        {
            var path = Path.Combine(dir.FullName, "Script", "Biomes", "HOLBiomeRules.as");
            if (File.Exists(path))
            {
                return Regex.Matches(File.ReadAllText(path), "B\\.Key = \"([a-z_]+)\"").Select(m => m.Groups[1].Value).ToList();
            }
        }
        throw new FileNotFoundException("Script/Biomes/HOLBiomeRules.as not found above " + AppContext.BaseDirectory);
    }

    [Fact]
    public void NewLobbyKeepsTheMapsOwnBiome()
    {
        Assert.Equal("default", lobby.Settings.Biome);
        Assert.Equal("default", LobbyRules.DefaultBiome);
    }

    [Fact]
    public void TheListIsTheScriptTablePlusTheMapsOwn()
    {
        // B-Q3: five biomes; B-Q4: any biome on any map; the first choice is the map's own (BiomeInProgress.md).
        var keys = ScriptBiomeKeys();
        Assert.Equal(5, keys.Count);
        Assert.Equal("default", LobbyRules.Biomes[0]);
        Assert.Equal(keys.Order(StringComparer.Ordinal), LobbyRules.Biomes.Skip(1).Order(StringComparer.Ordinal));
    }

    [Fact]
    public void BiomeIsTheLeadersResetsEveryReadyAndRefusesAnUnknownBiome()
    {
        LobbyEntryTests.AssertError(LobbyErrors.NotLeader, () => store.UpdateSettings(lobby.Id, alice, biome: "snow"));
        LobbyEntryTests.AssertError(LobbyErrors.BadRequest, () => store.UpdateSettings(lobby.Id, bob, biome: "lava"));
        LobbyEntryTests.AssertError(LobbyErrors.BadRequest, () => store.UpdateSettings(lobby.Id, bob, biome: ""));
        store.UpdateMe(lobby.Id, bob, ready: true);
        store.UpdateMe(lobby.Id, alice, ready: true);
        store.UpdateSettings(lobby.Id, bob, biome: "DEFAULT");
        Assert.True(lobby.Slots[1].Player!.Ready);
        store.UpdateSettings(lobby.Id, bob, biome: "Snow");
        Assert.Equal("snow", lobby.Settings.Biome);
        Assert.False(lobby.Slots[0].Player!.Ready || lobby.Slots[1].Player!.Ready);
        store.UpdateSettings(lobby.Id, bob, nomad: true);
        Assert.Equal("snow", lobby.Settings.Biome);
    }

    [Fact]
    public void AnyBiomeOnAnyMapAndSizeAndTheBiomeSurvivesAMapChange()
    {
        // B-Q4: no list per map; changing the map or the size does not touch the biome.
        store.UpdateSettings(lobby.Id, bob, biome: "dark_forest");
        foreach (var map in store.Maps)
        {
            store.UpdateSettings(lobby.Id, bob, map: map.Name);
            Assert.Equal("dark_forest", lobby.Settings.Biome);
        }
    }

    [Fact]
    public void StartWritesTheBiome()
    {
        store.UpdateSettings(lobby.Id, bob, biome: "autumn");
        store.UpdateMe(lobby.Id, bob, ready: true);
        store.UpdateMe(lobby.Id, alice, ready: true);
        var config = store.Start(lobby.Id, bob).Config;
        Assert.Equal("autumn", config.Biome);
        Assert.Equal("autumn", (string?)JsonNode.Parse(config.ToJson())!["biome"]);
        Assert.Equal("default", MatchConfig.FromLobby(new LobbyStore(Version, new FakeClock()).Create("B", "Cy", Version).Lobby, store.Maps[0], 1).Biome);
    }

    [Fact]
    public void TheViewGivesTheBiome()
    {
        store.UpdateSettings(lobby.Id, bob, biome: "snow");
        var state = JsonNode.Parse(JsonSerializer.Serialize(store.Atomic(() => LobbyViews.State(store, lobby, bob))))!;
        Assert.Equal("snow", (string?)state["settings"]!["biome"]);
    }
}
