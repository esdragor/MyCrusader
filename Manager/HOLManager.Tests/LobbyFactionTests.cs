using System.Net;
using System.Net.Http.Json;
using System.Text.Json.Nodes;
using HOLManager.Lobbies;
using HOLManager.Matches;
using Microsoft.AspNetCore.Mvc.Testing;
using Microsoft.Extensions.DependencyInjection;
using Microsoft.Extensions.DependencyInjection.Extensions;

namespace HOLManager.Tests;

/**
 * Templiers par cheat (EtapeDefini.md): a player sets his faction on his line (PUT /me), the leader an AI's (PUT /slots/{i}, state
 * "ai"). DawnOrder, BlackLegions and Templars are accepted; the others (greyed in the game) and unknown names are a bad_request. The
 * server does not check the cheat code of the client: Templars are accepted from anyone.
 */
public sealed class LobbyFactionTests : IClassFixture<WebApplicationFactory<Program>>, IDisposable
{
    private const string Version = LobbyEntryTests.Version;
    private readonly FakeClock clock = new();
    private readonly WebApplicationFactory<Program> app;
    private readonly HttpClient client;

    public LobbyFactionTests(WebApplicationFactory<Program> factory)
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

    private static async Task<JsonNode> Ok(HttpResponseMessage response)
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

    private Task<HttpResponseMessage> PutMe(string id, string token, object body) => Send(HttpMethod.Put, $"/lobbies/{id}/me", token, body);

    private Task<HttpResponseMessage> PutSlot(string id, string token, int index, object body) =>
        Send(HttpMethod.Put, $"/lobbies/{id}/slots/{index}", token, body);

    private async Task<JsonNode> Read(string id, string token) => await Ok(await Send(HttpMethod.Get, $"/lobbies/{id}", token));

    /** Bob creates the lobby (slot 0), Alice joins (slot 1). */
    private async Task<(string Id, string Bob, string Alice)> BobAndAlice()
    {
        var created = await Json(await client.PostAsJsonAsync("/lobbies", new { name = "Salon de Bob", playerName = "Bob", gameVersion = Version }));
        var id = (string)created["lobby"]!["id"]!;
        var joined = await Json(await client.PostAsJsonAsync($"/lobbies/{id}/join", new { playerName = "Alice", gameVersion = Version }));
        return (id, (string)created["token"]!, (string)joined["token"]!);
    }

    private static string? FactionOf(JsonNode state, int index) => (string?)state["slots"]![index]!["faction"];

    [Fact]
    public async Task APlayerSetsHisFactionAndEveryoneReadsIt()
    {
        var (id, bob, alice) = await BobAndAlice();
        Assert.Equal("DawnOrder", FactionOf(await Read(id, bob), 1));
        var state = await Ok(await PutMe(id, alice, new { faction = "BlackLegions" }));
        Assert.Equal("BlackLegions", FactionOf(state, 1));
        Assert.Equal("DawnOrder", FactionOf(state, 0));
        Assert.Equal("BlackLegions", FactionOf(await Read(id, bob), 1));
        // Templars: the server does not know the cheat code, it trusts the client.
        Assert.Equal("Templars", FactionOf(await Ok(await PutMe(id, alice, new { faction = "Templars" })), 1));
        Assert.Equal("DawnOrder", FactionOf(await Ok(await PutMe(id, alice, new { faction = "DawnOrder" })), 1));
    }

    [Fact]
    public async Task TheFactionNameIsMatchedWithoutCaseAndKeptCanonical()
    {
        var (id, _, alice) = await BobAndAlice();
        Assert.Equal("Templars", FactionOf(await Ok(await PutMe(id, alice, new { faction = "templars" })), 1));
    }

    [Theory]
    [InlineData("DragonChildren")]
    [InlineData("ShadowCircle")]
    [InlineData("FireHeirs")]
    [InlineData("Pirates")]
    [InlineData("")]
    public async Task AFactionOutsideTheThreeIsABadRequestAndChangesNothing(string faction)
    {
        var (id, bob, alice) = await BobAndAlice();
        await PutMe(id, alice, new { faction = "BlackLegions" });
        await AssertError(await PutMe(id, alice, new { faction }), HttpStatusCode.BadRequest, "bad_request");
        Assert.Equal("BlackLegions", FactionOf(await Read(id, bob), 1));
        // The leader's AI too.
        await AssertError(await PutSlot(id, bob, 2, new { state = "ai", faction }), HttpStatusCode.BadRequest, "bad_request");
        Assert.Equal("open", (string?)(await Read(id, bob))["slots"]![2]!["state"]);
    }

    [Fact]
    public async Task AbsentFieldLeavesTheFactionAndAChangeOfColourKeepsIt()
    {
        var (id, bob, alice) = await BobAndAlice();
        await PutMe(id, alice, new { faction = "Templars" });
        await Ok(await PutMe(id, alice, new { colorIndex = 5, team = 2 }));
        await Ok(await PutMe(id, alice, new { ready = true }));
        Assert.Equal("Templars", FactionOf(await Read(id, bob), 1));
    }

    [Fact]
    public async Task ChangingMyFactionResetsMyReadyLikeColourAndTeam()
    {
        var (id, bob, alice) = await BobAndAlice();
        await PutMe(id, alice, new { ready = true });
        await PutMe(id, alice, new { faction = "DawnOrder" });
        Assert.True((bool)(await Read(id, bob))["slots"]![1]!["ready"]!);
        await PutMe(id, alice, new { faction = "BlackLegions" });
        Assert.False((bool)(await Read(id, bob))["slots"]![1]!["ready"]!);
    }

    [Fact]
    public async Task ASpectatorHasNoFactionToSet()
    {
        var (id, _, alice) = await BobAndAlice();
        await Ok(await PutMe(id, alice, new { spectator = true }));
        await AssertError(await PutMe(id, alice, new { faction = "Templars" }), HttpStatusCode.BadRequest, "bad_request");
    }

    [Fact]
    public async Task ASpectatorTakingASlotAgainStartsWithTheDefaultFaction()
    {
        var (id, bob, alice) = await BobAndAlice();
        await PutMe(id, alice, new { faction = "BlackLegions" });
        await PutMe(id, alice, new { spectator = true });
        var state = await Ok(await PutMe(id, alice, new { spectator = false }));
        Assert.Equal("DawnOrder", FactionOf(state, 1));
        // A spectator may take a slot and choose a faction in the same request.
        await PutMe(id, alice, new { spectator = true });
        state = await Ok(await PutMe(id, alice, new { spectator = false, faction = "Templars" }));
        Assert.Equal("Templars", FactionOf(state, 1));
        Assert.Equal("DawnOrder", FactionOf(await Read(id, bob), 0));
    }

    [Fact]
    public async Task TheLeaderSetsAnAIFactionAndItKeepsItWhenItsColourChanges()
    {
        var (id, bob, _) = await BobAndAlice();
        var state = await Ok(await PutSlot(id, bob, 2, new { state = "ai", faction = "BlackLegions" }));
        Assert.Equal(("ai", "BlackLegions"), ((string?)state["slots"]![2]!["state"], FactionOf(state, 2)));
        state = await Ok(await PutSlot(id, bob, 2, new { state = "ai", colorIndex = 6 }));
        Assert.Equal("BlackLegions", FactionOf(state, 2));
        state = await Ok(await PutSlot(id, bob, 2, new { state = "ai", faction = "Templars" }));
        Assert.Equal("Templars", FactionOf(state, 2));
        // A new AI on another slot starts with the default one; the first AI keeps its own.
        state = await Ok(await PutSlot(id, bob, 3, new { state = "ai" }));
        Assert.Equal(("DawnOrder", "Templars"), (FactionOf(state, 3), FactionOf(state, 2)));
    }

    [Fact]
    public async Task ChangingAnAIFactionResetsEveryReadyButTheSameFactionDoesNot()
    {
        var (id, bob, alice) = await BobAndAlice();
        await PutSlot(id, bob, 2, new { state = "ai", faction = "BlackLegions" });
        await PutMe(id, alice, new { ready = true });
        await Ok(await PutSlot(id, bob, 2, new { state = "ai", faction = "BlackLegions" }));
        Assert.True((bool)(await Read(id, bob))["slots"]![1]!["ready"]!);
        await Ok(await PutSlot(id, bob, 2, new { state = "ai", faction = "DawnOrder" }));
        Assert.False((bool)(await Read(id, bob))["slots"]![1]!["ready"]!);
    }

    [Fact]
    public async Task OnlyTheLeaderSetsAnAIFaction()
    {
        var (id, bob, alice) = await BobAndAlice();
        await PutSlot(id, bob, 2, new { state = "ai" });
        await AssertError(await PutSlot(id, alice, 2, new { state = "ai", faction = "Templars" }), HttpStatusCode.Forbidden, "not_leader");
        Assert.Equal("DawnOrder", FactionOf(await Read(id, bob), 2));
    }

    [Fact]
    public async Task AnAIAndItsFactionGoWhenTheLeaderRemovesIt()
    {
        var (id, bob, _) = await BobAndAlice();
        await PutSlot(id, bob, 2, new { state = "ai", faction = "Templars" });
        await Ok(await PutSlot(id, bob, 2, new { state = "open" }));
        await PutSlot(id, bob, 2, new { state = "ai" });
        Assert.Equal("DawnOrder", FactionOf(await Read(id, bob), 2));
    }

    [Fact]
    public async Task OtherLinesKeepTheirFactionWhenAnAIIsAddedOrChanged()
    {
        var (id, bob, alice) = await BobAndAlice();
        await PutMe(id, bob, new { faction = "Templars" });
        await PutMe(id, alice, new { faction = "BlackLegions" });
        var state = await Ok(await PutSlot(id, bob, 2, new { state = "ai", faction = "BlackLegions" }));
        state = await Ok(await PutSlot(id, bob, 2, new { state = "ai", team = 3 }));
        Assert.Equal(["Templars", "BlackLegions", "BlackLegions"], new[] { 0, 1, 2 }.Select(i => FactionOf(state, i)));
    }
}

/** The lobby's factions at the store level: the launch configuration and the return to the lobby. */
public class LobbyFactionStoreTests
{
    private const string Version = LobbyEntryTests.Version;
    private readonly FakeClock clock = new();
    private readonly FakeLauncher launcher = new();
    private readonly LobbyStore store;

    public LobbyFactionStoreTests() => store = new LobbyStore(Version, clock, launcher: launcher);

    private static LobbyPlayer Player(Lobby lobby, string name) => lobby.Players.Single(p => p.Name == name);

    /** Bob (Templars, slot 0), Alice (BlackLegions, 1), Chloé (default, 2), Dan (default, 3), an AI on slot 4 (Templars). */
    private (Lobby Lobby, string Bob, string Alice, string Chloe) Ready()
    {
        var created = store.Create("Salon de Bob", "Bob", Version);
        var id = created.Lobby.Id;
        var alice = store.Join(id, "Alice", Version).Token;
        var chloe = store.Join(id, "Chloé", Version).Token;
        store.Join(id, "Dan", Version);
        store.SetSlotAI(id, created.Token, 4, faction: "Templars");
        store.UpdateMe(id, created.Token, faction: "Templars");
        store.UpdateMe(id, alice, faction: "BlackLegions");
        return (created.Lobby, created.Token, alice, chloe);
    }

    private void AllReady(Lobby lobby, params string[] tokens)
    {
        foreach (var token in tokens)
        {
            store.UpdateMe(lobby.Id, token, ready: true);
        }
    }

    [Fact]
    public void TheLaunchConfigurationCarriesEachSlotsFaction()
    {
        var (lobby, bob, alice, chloe) = Ready();
        var dan = lobby.Slots[3].Player!.Token;
        AllReady(lobby, bob, alice, chloe, dan);
        var slots = MatchConfig.FromLobby(lobby, store.Maps[0], seed: 7).Slots;
        Assert.Equal(["Templars", "BlackLegions", "DawnOrder", "DawnOrder", "Templars"], slots.Take(5).Select(s => s.Faction));
        // Through match.json, the name the game reads (EHOLFaction by name).
        var json = JsonNode.Parse(MatchConfig.FromLobby(lobby, store.Maps[0], seed: 7).ToJson())!;
        Assert.Equal(["Templars", "BlackLegions", "DawnOrder", "DawnOrder", "Templars"],
            json["slots"]!.AsArray().Take(5).Select(s => (string?)s!["faction"]));
    }

    [Fact]
    public void ThoseBackAfterAFinishedMatchTakeTheirFactionAgain()
    {
        var (lobby, bob, alice, chloe) = Ready();
        var dan = lobby.Slots[3].Player!.Token;
        AllReady(lobby, bob, alice, chloe, dan);
        var match = store.Start(lobby.Id, bob);
        store.ServerListening(match.Id, 7778);
        store.ServerFinished(match.Id, [0]);
        // Back in another order, each finds the faction he had; a stranger starts with the default one.
        store.Join(lobby.Id, "Chloé", Version);
        store.Join(lobby.Id, "alice", Version);
        store.Join(lobby.Id, "Bob", Version);
        store.Join(lobby.Id, "Eve", Version);
        Assert.Equal(["DawnOrder", "BlackLegions", "Templars", "DawnOrder"],
            new[] { "Chloé", "alice", "Bob", "Eve" }.Select(n => Player(lobby, n).Faction));
    }

    [Fact]
    public void PlayersKeepTheirFactionWhenTheServerStopsAndTheyKeepTheirLine()
    {
        var (lobby, bob, alice, chloe) = Ready();
        var dan = lobby.Slots[3].Player!.Token;
        AllReady(lobby, bob, alice, chloe, dan);
        var match = store.Start(lobby.Id, bob);
        store.ServerListening(match.Id, 7778);
        launcher.Alive.Remove(match.Id);
        store.Housekeep();
        Assert.Equal(["Templars", "BlackLegions"], new[] { "Bob", "Alice" }.Select(n => Player(lobby, n).Faction));
    }

    [Fact]
    public void ALobbyAIKeepsItsFactionAfterTheMatch()
    {
        var (lobby, bob, alice, chloe) = Ready();
        var dan = lobby.Slots[3].Player!.Token;
        AllReady(lobby, bob, alice, chloe, dan);
        var match = store.Start(lobby.Id, bob);
        store.ServerListening(match.Id, 7778);
        store.ServerFinished(match.Id, [0]);
        Assert.Equal("Templars", lobby.Slots[4].AI!.Faction);
    }

    [Fact]
    public void AMatchFactionIsOnlyGivenBackToTheNameThatHadIt()
    {
        var (lobby, bob, alice, chloe) = Ready();
        var dan = lobby.Slots[3].Player!.Token;
        AllReady(lobby, bob, alice, chloe, dan);
        var match = store.Start(lobby.Id, bob);
        store.ServerListening(match.Id, 7778);
        store.ServerFinished(match.Id, [0]);
        store.Join(lobby.Id, "Alice", Version);
        var eve = store.Join(lobby.Id, "Eve", Version).Token;
        Assert.Equal(("BlackLegions", "DawnOrder"), (Player(lobby, "Alice").Faction, Player(lobby, "Eve").Faction));
        // Not stuck: Eve changes hers like anyone.
        store.UpdateMe(lobby.Id, eve, faction: "Templars");
        Assert.Equal("Templars", Player(lobby, "Eve").Faction);
    }
}
