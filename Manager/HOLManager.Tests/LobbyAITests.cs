using System.Net;
using System.Net.Http.Json;
using System.Text.Json.Nodes;
using HOLManager.Lobbies;
using Microsoft.AspNetCore.Mvc.Testing;
using Microsoft.Extensions.DependencyInjection;
using Microsoft.Extensions.DependencyInjection.Extensions;

namespace HOLManager.Tests;

/**
 * IA0.6 L1 (Q4, Q5, Q6): the leader puts an AI on a slot of the lobby (PUT /lobbies/{id}/slots/{index}, state "ai"),
 * sets its colour and team; its level is Normal. Through HTTP, the lobby's state read back by GET /lobbies/{id}.
 */
public sealed class LobbyAITests : IClassFixture<WebApplicationFactory<Program>>, IDisposable
{
    private const string Version = LobbyEntryTests.Version;
    private readonly FakeClock clock = new();
    private readonly WebApplicationFactory<Program> app;
    private readonly HttpClient client;

    public LobbyAITests(WebApplicationFactory<Program> factory)
    {
        app = factory.WithWebHostBuilder(b =>
        {
            b.UseSetting("GameVersion", Version);
            b.ConfigureServices(s => s.Replace(ServiceDescriptor.Singleton<IClock>(clock)));
        });
        client = app.CreateClient();
    }

    public void Dispose() => app.Dispose();

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

    private async Task<JsonNode> Read(string id, string token) => await Ok(await Send(HttpMethod.Get, $"/lobbies/{id}", token));

    /** Bob creates the lobby (slot 0, colour 0), Alice joins (slot 1, colour 1). */
    private async Task<(string Id, string Bob, string Alice)> BobAndAlice()
    {
        var created = await Json(await client.PostAsJsonAsync("/lobbies", new { name = "Salon de Bob", playerName = "Bob", gameVersion = Version }));
        var id = (string)created["lobby"]!["id"]!;
        var joined = await Json(await client.PostAsJsonAsync($"/lobbies/{id}/join", new { playerName = "Alice", gameVersion = Version }));
        return (id, (string)created["token"]!, (string)joined["token"]!);
    }

    private static JsonNode Slot(JsonNode state, int index) => state["slots"]![index]!;

    private static (string?, string?, int?, int?, string?) AI(JsonNode slot) =>
        ((string?)slot["state"], (string?)slot["level"], (int?)slot["colorIndex"], (int?)slot["team"], (string?)slot["faction"]);

    [Fact]
    public async Task LeaderPutsAnAIOnAnOpenSlotWithTheFirstFreeColour()
    {
        var (id, bob, _) = await BobAndAlice();
        var state = await Ok(await PutSlot(id, bob, 2, new { state = "ai" }));
        // Normal, the first colour nobody has (Bob 0, Alice 1), no team; its name « IA Normal 3 » is the client's.
        Assert.Equal(("ai", "Normal", 2, 0, "DawnOrder"), AI(Slot(state, 2)));
        Assert.Null(Slot(state, 2)["playerName"]);
        Assert.Equal(("ai", "Normal", 2, 0, "DawnOrder"), AI(Slot(await Read(id, bob), 2)));
        // Not a player: two players, the AI's slot is no longer open to a join.
        var list = await Json(await client.GetAsync("/lobbies"));
        Assert.Equal(2, (int)list["lobbies"]![0]!["players"]!);
    }

    [Fact]
    public async Task LeaderPutsAnAIOnAClosedSlotWithItsColourAndTeam()
    {
        var (id, bob, _) = await BobAndAlice();
        await Ok(await PutSlot(id, bob, 5, new { state = "closed" }));
        var state = await Ok(await PutSlot(id, bob, 5, new { state = "ai", colorIndex = 6, team = 2 }));
        Assert.Equal(("ai", "Normal", 6, 2, "DawnOrder"), AI(Slot(state, 5)));
    }

    [Fact]
    public async Task OnlyTheLeaderPutsAnAI()
    {
        var (id, _, alice) = await BobAndAlice();
        await AssertError(await PutSlot(id, alice, 2, new { state = "ai" }), HttpStatusCode.Forbidden, "not_leader");
        Assert.Equal("open", (string?)Slot(await Read(id, alice), 2)["state"]);
    }

    [Fact]
    public async Task AnAIIsRefusedOnAPlayersSlotOrPastThePlaces()
    {
        var (id, bob, _) = await BobAndAlice();
        await AssertError(await PutSlot(id, bob, 1, new { state = "ai" }), HttpStatusCode.Conflict, "slot_taken");
        await Ok(await Send(HttpMethod.Put, $"/lobbies/{id}/settings", bob, new { size = "Medium" }));
        await AssertError(await PutSlot(id, bob, 4, new { state = "ai" }), HttpStatusCode.BadRequest, "bad_request");
        await AssertError(await PutSlot(id, bob, 2, new { state = "ai", colorIndex = 9 }), HttpStatusCode.BadRequest, "bad_request");
        await AssertError(await PutSlot(id, bob, 2, new { state = "ai", team = 5 }), HttpStatusCode.BadRequest, "bad_request");
        Assert.Equal("open", (string?)Slot(await Read(id, bob), 2)["state"]);
    }

    [Fact]
    public async Task ATakenColourIsRefusedBothWays()
    {
        var (id, bob, alice) = await BobAndAlice();
        // A player's colour for the AI.
        await AssertError(await PutSlot(id, bob, 2, new { state = "ai", colorIndex = 1 }), HttpStatusCode.Conflict, "color_taken");
        Assert.Equal("open", (string?)Slot(await Read(id, bob), 2)["state"]);
        await Ok(await PutSlot(id, bob, 2, new { state = "ai", colorIndex = 4 }));
        // Another AI's colour.
        await AssertError(await PutSlot(id, bob, 3, new { state = "ai", colorIndex = 4 }), HttpStatusCode.Conflict, "color_taken");
        // The AI's colour for a player.
        await AssertError(await PutMe(id, alice, new { colorIndex = 4 }), HttpStatusCode.Conflict, "color_taken");
        // "Aléatoire" is never taken (E2-2).
        await Ok(await PutSlot(id, bob, 3, new { state = "ai", colorIndex = LobbyRules.RandomColor }));
        await Ok(await PutMe(id, alice, new { colorIndex = LobbyRules.RandomColor }));
        // A new player takes the first colour neither a player nor an AI has: Carl 1 (freed by Alice), Dan 2 (4 is the AI's).
        var carl = await Json(await client.PostAsJsonAsync($"/lobbies/{id}/join", new { playerName = "Carl", gameVersion = Version }));
        var carlSlot = (int)carl["slot"]!;
        Assert.Equal(1, (int)Slot(carl["lobby"]!, carlSlot)["colorIndex"]!);
        var dan = await Json(await client.PostAsJsonAsync($"/lobbies/{id}/join", new { playerName = "Dan", gameVersion = Version }));
        Assert.Equal(2, (int)Slot(dan["lobby"]!, (int)dan["slot"]!)["colorIndex"]!);
    }

    [Fact]
    public async Task LeaderChangesTheAIsColourAndTeam()
    {
        var (id, bob, _) = await BobAndAlice();
        await Ok(await PutSlot(id, bob, 2, new { state = "ai" }));
        var state = await Ok(await PutSlot(id, bob, 2, new { state = "ai", team = 3 }));
        Assert.Equal(("ai", "Normal", 2, 3, "DawnOrder"), AI(Slot(state, 2)));
        state = await Ok(await PutSlot(id, bob, 2, new { state = "ai", colorIndex = 5 }));
        Assert.Equal(("ai", "Normal", 5, 3, "DawnOrder"), AI(Slot(state, 2)));
        // Its own colour again is no conflict.
        await Ok(await PutSlot(id, bob, 2, new { state = "ai", colorIndex = 5 }));
    }

    [Fact]
    public async Task AddingOrRemovingAnAIResetsEveryReady()
    {
        var (id, bob, alice) = await BobAndAlice();
        async Task BothReady()
        {
            await Ok(await PutMe(id, bob, new { ready = true }));
            await Ok(await PutMe(id, alice, new { ready = true }));
        }
        bool?[] Ready(JsonNode state) => [(bool?)Slot(state, 0)["ready"], (bool?)Slot(state, 1)["ready"]];

        await BothReady();
        Assert.Equal([false, false], Ready(await Ok(await PutSlot(id, bob, 2, new { state = "ai" }))));
        await BothReady();
        // The same AI again changes nothing.
        Assert.Equal([true, true], Ready(await Ok(await PutSlot(id, bob, 2, new { state = "ai" }))));
        Assert.Equal([false, false], Ready(await Ok(await PutSlot(id, bob, 2, new { state = "open" }))));
        await Ok(await PutSlot(id, bob, 3, new { state = "ai" }));
        await BothReady();
        Assert.Equal([false, false], Ready(await Ok(await PutSlot(id, bob, 3, new { state = "closed" }))));
    }

    [Fact]
    public async Task AnAIGoesBackToOpenOrClosed()
    {
        var (id, bob, _) = await BobAndAlice();
        await Ok(await PutSlot(id, bob, 2, new { state = "ai", colorIndex = 5, team = 1 }));
        await Ok(await PutSlot(id, bob, 3, new { state = "ai" }));
        var state = await Ok(await PutSlot(id, bob, 2, new { state = "open" }));
        Assert.Equal(new JsonObject { ["index"] = 2, ["state"] = "open" }.ToJsonString(), Slot(state, 2).ToJsonString());
        state = await Ok(await PutSlot(id, bob, 3, new { state = "closed" }));
        Assert.Equal(new JsonObject { ["index"] = 3, ["state"] = "closed" }.ToJsonString(), Slot(state, 3).ToJsonString());
        // Its colour is free again, and a new AI starts anew.
        state = await Ok(await PutSlot(id, bob, 2, new { state = "ai" }));
        Assert.Equal(("ai", "Normal", 2, 0, "DawnOrder"), AI(Slot(state, 2)));
    }

    [Fact]
    public async Task ASmallerSizeMovesAnAIPastItsPlacesBelowAsAPlayer()
    {
        // As the solo screen (FHOLMapSizeRules::FitSlotsToSize) and a player: the AI onto the first free slot below,
        // with its colour and team.
        var (id, bob, alice) = await BobAndAlice();
        await Send(HttpMethod.Post, $"/lobbies/{id}/leave", alice);
        var state = await Ok(await PutSlot(id, bob, 5, new { state = "ai", colorIndex = 4, team = 2 }));
        await Ok(await PutMe(id, bob, new { ready = true }));
        state = await Ok(await Send(HttpMethod.Put, $"/lobbies/{id}/settings", bob, new { size = "Small" }));
        Assert.Equal(["player", "ai", "closed", "closed", "closed", "closed", "closed", "closed"],
            state["slots"]!.AsArray().Select(s => (string?)s!["state"]));
        Assert.Equal(("ai", "Normal", 4, 2, "DawnOrder"), AI(Slot(state, 1)));
        Assert.False((bool?)Slot(state, 0)["ready"]);
        // Back to Large, slot 5 reopens empty, the AI stays on slot 1.
        state = await Ok(await Send(HttpMethod.Put, $"/lobbies/{id}/settings", bob, new { size = "Large" }));
        Assert.Equal(("open", "ai"), ((string?)Slot(state, 5)["state"], (string?)Slot(state, 1)["state"]));
    }

    [Fact]
    public async Task ASizeTooSmallForThePlayersAndAIsIsRefused()
    {
        var (id, bob, _) = await BobAndAlice();
        await Ok(await PutSlot(id, bob, 2, new { state = "ai" }));
        // Bob, Alice and an AI below the places of Small (2): refused, as for three players.
        await AssertError(await Send(HttpMethod.Put, $"/lobbies/{id}/settings", bob, new { size = "Small" }), HttpStatusCode.Conflict, "slot_taken");
        var state = await Read(id, bob);
        Assert.Equal(("Large", "ai"), ((string?)state["settings"]!["size"], (string?)Slot(state, 2)["state"]));
        // Medium (4) holds them, the AI stays where it is.
        state = await Ok(await Send(HttpMethod.Put, $"/lobbies/{id}/settings", bob, new { size = "Medium" }));
        Assert.Equal(["player", "player", "ai", "open", "closed"], state["slots"]!.AsArray().Take(5).Select(s => (string?)s!["state"]));
    }

    [Fact]
    public async Task APlayerPastThePlacesDoesNotLandOnAnAI()
    {
        var (id, bob, alice) = await BobAndAlice();
        var carl = await Json(await client.PostAsJsonAsync($"/lobbies/{id}/join", new { playerName = "Carl", gameVersion = Version }));
        Assert.Equal(2, (int)carl["slot"]!);
        await Send(HttpMethod.Post, $"/lobbies/{id}/leave", alice);
        await Ok(await PutSlot(id, bob, 1, new { state = "ai", colorIndex = 6 }));
        // Small (2): Carl (slot 2) would need slot 1, the AI's: refused, as for three players.
        await AssertError(await Send(HttpMethod.Put, $"/lobbies/{id}/settings", bob, new { size = "Small" }), HttpStatusCode.Conflict, "slot_taken");
        var state = await Read(id, bob);
        Assert.Equal(("player", "ai", "player"), ((string?)Slot(state, 0)["state"], (string?)Slot(state, 1)["state"], (string?)Slot(state, 2)["state"]));
    }
}
