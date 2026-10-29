using System.Net;
using System.Net.Http.Json;
using System.Text.Json.Nodes;
using HOLManager.Lobbies;
using HOLManager.Matches;
using Microsoft.AspNetCore.Mvc.Testing;
using Microsoft.Extensions.DependencyInjection;
using Microsoft.Extensions.DependencyInjection.Extensions;

namespace HOLManager.Tests;

/** A launcher that keeps the match.json it was given. */
internal sealed class ConfigLauncher : IServerLauncher
{
    public MatchConfig? Config { get; private set; }

    public int Launch(string matchId, MapInfo map, MatchConfig config, string? gameVersion = null)
    {
        Config = config;
        return 7778;
    }

    public int Running => 0;

    public bool HasExited(string matchId) => false;

    public void Stop(string matchId) { }
}

/**
 * IA0.6 L2: "Lancer" with AIs (Q3 = A: an AI counts in the two players a match needs; only the humans are "Prêt"),
 * and the AI slots of match.json ("AI", colour, team, "difficulty": "Normal"). Through HTTP, the match.json read from
 * the launcher the store started.
 */
public sealed class LobbyAIStartTests : IClassFixture<WebApplicationFactory<Program>>, IDisposable
{
    private const string Version = LobbyEntryTests.Version;
    private readonly FakeClock clock = new();
    private readonly ConfigLauncher launcher = new();
    private readonly WebApplicationFactory<Program> app;
    private readonly HttpClient client;

    public LobbyAIStartTests(WebApplicationFactory<Program> factory)
    {
        app = factory.WithWebHostBuilder(b =>
        {
            b.UseSetting("GameVersion", Version);
            b.ConfigureServices(s =>
            {
                s.Replace(ServiceDescriptor.Singleton<IClock>(clock));
                s.Replace(ServiceDescriptor.Singleton<IServerLauncher>(launcher));
            });
        });
        client = app.CreateClient();
    }

    public void Dispose() => app.Dispose();

    internal static string AIReferencePath => Path.Combine(AppContext.BaseDirectory, "Data", "match_v3_ai_reference.json");

    private static async Task<JsonNode> Json(HttpResponseMessage response) =>
        JsonNode.Parse(await response.Content.ReadAsStringAsync())!;

    private async Task<JsonNode> Ok(HttpResponseMessage response)
    {
        Assert.True(response.StatusCode == HttpStatusCode.OK, $"{response.StatusCode}: {await response.Content.ReadAsStringAsync()}");
        return await Json(response);
    }

    private static async Task AssertError(HttpResponseMessage response, HttpStatusCode status, string error)
    {
        var body = await Json(response);
        Assert.Equal((status, error), (response.StatusCode, (string?)body["error"]));
        Assert.False(string.IsNullOrEmpty((string?)body["message"]));
    }

    private Task<HttpResponseMessage> Send(HttpMethod method, string url, string? token, object? body = null)
    {
        var request = new HttpRequestMessage(method, url);
        if (token != null)
        {
            request.Headers.Add("X-Player-Token", token);
        }
        if (body != null)
        {
            request.Content = JsonContent.Create(body);
        }
        return client.SendAsync(request);
    }

    private Task<HttpResponseMessage> PutSlot(string id, string token, int index, object body) =>
        Send(HttpMethod.Put, $"/lobbies/{id}/slots/{index}", token, body);

    private Task<HttpResponseMessage> PutMe(string id, string token, object body) => Send(HttpMethod.Put, $"/lobbies/{id}/me", token, body);

    private Task<HttpResponseMessage> Start(string id, string token) => Send(HttpMethod.Post, $"/lobbies/{id}/start", token);

    /** Bob creates the lobby (slot 0, colour 0), alone. */
    private async Task<(string Id, string Bob)> Bob()
    {
        var created = await Json(await client.PostAsJsonAsync("/lobbies", new { name = "Salon de Bob", playerName = "Bob", gameVersion = Version }));
        return ((string)created["lobby"]!["id"]!, (string)created["token"]!);
    }

    private async Task<string> Join(string id, string name) =>
        (string)(await Json(await client.PostAsJsonAsync($"/lobbies/{id}/join", new { playerName = name, gameVersion = Version })))["token"]!;

    /** match.json as the server receives it. */
    private JsonArray Slots()
    {
        Assert.NotNull(launcher.Config);
        return JsonNode.Parse(launcher.Config!.ToJson())!["slots"]!.AsArray();
    }

    private static (string?, int?, int?, string?, string?, string?) Fields(JsonNode slot) =>
        ((string?)slot["state"], (int?)slot["colorIndex"], (int?)slot["team"], (string?)slot["faction"], (string?)slot["playerName"],
            (string?)slot["difficulty"]);

    [Fact]
    public async Task TheLeaderAloneWithAnAIStarts()
    {
        // Q3 = A: an AI counts in the two players; it has no "Prêt", only Bob must be ready.
        var (id, bob) = await Bob();
        await Ok(await PutSlot(id, bob, 1, new { state = "ai", colorIndex = 4, team = 2 }));
        await Ok(await PutMe(id, bob, new { team = 1 }));
        await Ok(await PutMe(id, bob, new { ready = true }));
        var response = await Start(id, bob);
        Assert.True(response.StatusCode == HttpStatusCode.Accepted, $"{response.StatusCode}: {await response.Content.ReadAsStringAsync()}");
        var slots = Slots();
        Assert.Equal(("Human", 0, 1, "DawnOrder", "Bob", null), Fields(slots[0]!));
        Assert.Equal(("AI", 4, 2, "DawnOrder", "", "Normal"), Fields(slots[1]!));
        Assert.Equal(("Closed", 2, 0, "DawnOrder", "", null), Fields(slots[2]!));
        // FJsonObjectConverter's names; "difficulty" (FHOLMatchSlot::Difficulty) on an AI slot only.
        Assert.Equal(["state", "faction", "colorIndex", "team", "playerName", "difficulty"], slots[1]!.AsObject().Select(p => p.Key));
        Assert.Equal(["state", "faction", "colorIndex", "team", "playerName"], slots[0]!.AsObject().Select(p => p.Key));
        Assert.Equal("starting", (string?)(await Ok(await Send(HttpMethod.Get, $"/lobbies/{id}", bob)))["state"]);
    }

    [Fact]
    public async Task AnAIOnRandomGetsAColourNobodyElseHas()
    {
        // E2-2: Bob and two AIs on "Aléatoire", Alice 1, an AI 5: five distinct real colours.
        var (id, bob) = await Bob();
        var alice = await Join(id, "Alice");
        await Ok(await PutMe(id, bob, new { colorIndex = LobbyRules.RandomColor }));
        await Ok(await PutSlot(id, bob, 2, new { state = "ai", colorIndex = LobbyRules.RandomColor }));
        await Ok(await PutSlot(id, bob, 3, new { state = "ai", colorIndex = 5 }));
        await Ok(await PutSlot(id, bob, 4, new { state = "ai", colorIndex = LobbyRules.RandomColor }));
        await Ok(await PutMe(id, bob, new { ready = true }));
        await Ok(await PutMe(id, alice, new { ready = true }));
        Assert.Equal(HttpStatusCode.Accepted, (await Start(id, bob)).StatusCode);
        var slots = Slots();
        var colors = slots.Take(5).Select(s => (int)s!["colorIndex"]!).ToList();
        Assert.Equal(["Human", "Human", "AI", "AI", "AI"], slots.Take(5).Select(s => (string?)s!["state"]));
        Assert.Equal(1, colors[1]);
        Assert.Equal(5, colors[3]);
        Assert.All(colors, c => Assert.InRange(c, 0, LobbyRules.ColorCount - 1));
        Assert.Equal(5, colors.Distinct().Count());
        // The lobby keeps "Aléatoire".
        Assert.Equal(LobbyRules.RandomColor, (int?)(await Ok(await Send(HttpMethod.Get, $"/lobbies/{id}", bob)))["slots"]![2]!["colorIndex"]);
    }

    [Fact]
    public async Task AHumanNotReadyStillBlocks()
    {
        var (id, bob) = await Bob();
        var alice = await Join(id, "Alice");
        await Ok(await PutSlot(id, bob, 2, new { state = "ai" }));
        await Ok(await PutMe(id, bob, new { ready = true }));
        await AssertError(await Start(id, bob), HttpStatusCode.Conflict, "not_all_ready");
        // The leader with an AI, not ready himself.
        await Ok(await PutMe(id, alice, new { ready = true }));
        await Ok(await PutMe(id, bob, new { ready = false }));
        await AssertError(await Start(id, bob), HttpStatusCode.Conflict, "not_all_ready");
        Assert.Null(launcher.Config);
    }

    [Fact]
    public async Task TheLeaderAloneWithoutAnAIIsStillRefused()
    {
        var (id, bob) = await Bob();
        await Ok(await PutMe(id, bob, new { ready = true }));
        await AssertError(await Start(id, bob), HttpStatusCode.Conflict, "not_enough_players");
        // An AI put then taken away.
        await Ok(await PutSlot(id, bob, 1, new { state = "ai" }));
        await Ok(await PutSlot(id, bob, 1, new { state = "open" }));
        await Ok(await PutMe(id, bob, new { ready = true }));
        await AssertError(await Start(id, bob), HttpStatusCode.Conflict, "not_enough_players");
        Assert.Null(launcher.Config);
    }

    /** The lobby of the AI reference file: Bob and Alice (team 1) against two AIs (team 2), slot 1 and 3. */
    private static (LobbyStore Store, Lobby Lobby, string Bob) ReferenceAILobby()
    {
        var store = new LobbyStore(Version, new FakeClock());
        var created = store.Create("Salon de Bob", "Bob", Version);
        var id = created.Lobby.Id;
        store.SetSlotAI(id, created.Token, 1, colorIndex: 6, team: 2);
        var alice = store.Join(id, "Alice", Version).Token;
        store.SetSlotAI(id, created.Token, 3, colorIndex: 4, team: 2);
        store.UpdateMe(id, created.Token, team: 1, ready: true);
        store.UpdateMe(id, alice, colorIndex: 3, team: 1, ready: true);
        return (store, created.Lobby, created.Token);
    }

    [Fact]
    public void ReferenceAILobbyGivesTheAIReferenceFile()
    {
        // The contract with the game: HOLMapSizeLobbyTests.cpp (HeroesOfLiberty.MapSize.Lobby.ManagerAIReference) reads it.
        var (store, lobby, _) = ReferenceAILobby();
        var json = MatchConfig.FromLobby(lobby, store.Maps[0], seed: 184467).ToJson();
        var expected = JsonNode.Parse(File.ReadAllText(AIReferencePath));
        Assert.True(JsonNode.DeepEquals(expected, JsonNode.Parse(json)), json);
    }

    [Fact]
    public void AIsStayAtTheLobbyAfterTheMatch()
    {
        // L1: the match's players leave the lobby at "finished", its AIs stay as they were set.
        var (store, lobby, bob) = ReferenceAILobby();
        var match = store.Start(lobby.Id, bob);
        store.ServerFinished(match.Id, [0]);
        Assert.Equal(LobbyState.Lobby, lobby.State);
        Assert.Empty(lobby.Players);
        Assert.Equal([SlotState.Open, SlotState.AI, SlotState.Open, SlotState.AI], lobby.Slots.Take(4).Select(s => s.State));
        Assert.Equal((6, 2, "Normal"), (lobby.Slots[1].AI!.ColorIndex, lobby.Slots[1].AI!.Team, lobby.Slots[1].AI!.Level));
    }
}
