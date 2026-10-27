using System.Net;
using System.Net.Http.Json;
using System.Text;
using System.Text.Json.Nodes;
using HOLManager.FakeServer;
using HOLManager.Lobbies;
using Microsoft.AspNetCore.Builder;
using Microsoft.AspNetCore.Hosting;
using Microsoft.AspNetCore.Http;
using Microsoft.AspNetCore.Mvc.Testing;
using Microsoft.Extensions.DependencyInjection;
using Microsoft.Extensions.DependencyInjection.Extensions;

namespace HOLManager.Tests;

/** R2.7: the lobby routes of Manager/PROTOCOL.md, through HTTP. */
public sealed class LobbyRoutesTests : IClassFixture<WebApplicationFactory<Program>>, IDisposable
{
    private const string Version = LobbyEntryTests.Version;
    private readonly FakeClock clock = new();
    private readonly WebApplicationFactory<Program> app;
    private readonly HttpClient client;

    /** "Start" launches the fake server (R2.9) here, on high ports, one server at most. */
    private readonly string matchesDir = Path.Combine(Path.GetTempPath(), $"HOL routes {Guid.NewGuid():N}");

    public LobbyRoutesTests(WebApplicationFactory<Program> factory)
    {
        app = factory.WithWebHostBuilder(b =>
        {
            b.UseSetting("GameVersion", Version);
            b.UseSetting("ServerExe", ProcessServerLauncherTests.FakeExe);
            b.UseSetting("MatchesDir", matchesDir);
            b.UseSetting("FirstServerPort", "47877");
            b.UseSetting("MaxServers", "1");
            b.ConfigureServices(s =>
            {
                s.Replace(ServiceDescriptor.Singleton<IClock>(clock));
                s.AddSingleton<IStartupFilter>(new RemoteAddressFilter());
            });
        });
        client = app.CreateClient();
    }

    /** Disposing the app disposes the launcher, which stops the fake servers. */
    public void Dispose()
    {
        app.Dispose();
        ProcessServerLauncherTests.DeleteDir(matchesDir);
    }

    private LobbyStore Store => app.Services.GetRequiredService<LobbyStore>();

    private static async Task<JsonNode> Json(HttpResponseMessage response) =>
        JsonNode.Parse(await response.Content.ReadAsStringAsync())!;

    private static async Task AssertError(HttpResponseMessage response, HttpStatusCode status, string error)
    {
        Assert.Equal(status, response.StatusCode);
        var body = await Json(response);
        Assert.Equal(error, (string?)body["error"]);
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

    private async Task<(string Id, string Token)> CreateLobby(string name = "Salon de Bob", string player = "Bob")
    {
        var response = await client.PostAsJsonAsync("/lobbies", new { name, playerName = player, gameVersion = Version });
        Assert.Equal(HttpStatusCode.Created, response.StatusCode);
        var body = await Json(response);
        return ((string)body["lobby"]!["id"]!, (string)body["token"]!);
    }

    private async Task<string> JoinLobby(string id, string player)
    {
        var response = await client.PostAsJsonAsync($"/lobbies/{id}/join", new { playerName = player, gameVersion = Version });
        Assert.Equal(HttpStatusCode.OK, response.StatusCode);
        return (string)(await Json(response))["token"]!;
    }

    [Fact]
    public async Task CreateGivesTokenSlotAndState()
    {
        var response = await client.PostAsJsonAsync("/lobbies", new { name = "Salon de Bob", playerName = "Bob", gameVersion = Version });
        Assert.Equal(HttpStatusCode.Created, response.StatusCode);
        var body = await Json(response);
        Assert.Matches("^[0-9a-f]{32}$", (string)body["token"]!);
        Assert.Equal(0, (int)body["slot"]!);
        var lobby = body["lobby"]!;
        Assert.Equal("Salon de Bob", (string?)lobby["name"]);
        Assert.Equal("lobby", (string?)lobby["state"]);
        Assert.Equal("Bob", (string?)lobby["leader"]);
        Assert.Equal("Bob", (string?)lobby["you"]);
        Assert.Null(lobby["minutes"]);
        Assert.Null(lobby["match"]);
        Assert.Null(lobby["notice"]);
        Assert.True(lobby.AsObject().ContainsKey("match") && lobby.AsObject().ContainsKey("minutes"));
        Assert.Equal("Plain", (string?)lobby["settings"]!["map"]);
        Assert.False((bool)lobby["settings"]!["nomad"]!);
        Assert.Equal("Plain", (string?)lobby["maps"]![0]!["name"]);
        Assert.Equal(8, (int)lobby["maps"]![0]!["maxPlayers"]!);
        var slots = lobby["slots"]!.AsArray();
        Assert.Equal(8, slots.Count);
        var bob = slots[0]!;
        Assert.Equal((0, "player", "Bob", 0, 0, "DawnOrder", false),
            ((int)bob["index"]!, (string?)bob["state"], (string?)bob["playerName"], (int)bob["colorIndex"]!, (int)bob["team"]!,
                (string?)bob["faction"], (bool)bob["ready"]!));
        Assert.Equal("open", (string?)slots[1]!["state"]);
        Assert.False(slots[1]!.AsObject().ContainsKey("playerName"));
    }

    [Fact]
    public async Task CreateWithBadNameOrVersionIsRefused()
    {
        await AssertError(await client.PostAsJsonAsync("/lobbies", new { name = "A", playerName = "B<b>", gameVersion = Version }),
            HttpStatusCode.BadRequest, "invalid_name");
        await AssertError(await client.PostAsJsonAsync("/lobbies", new { name = "A", playerName = "Bob", gameVersion = "old" }),
            HttpStatusCode.Conflict, "version_mismatch");
        await AssertError(await client.PostAsJsonAsync("/lobbies", new { name = "A", gameVersion = Version }),
            HttpStatusCode.BadRequest, "bad_request");
        await AssertError(await client.PostAsync("/lobbies", new StringContent("{ not json", Encoding.UTF8, "application/json")),
            HttpStatusCode.BadRequest, "bad_request");
        Assert.Equal(0, Store.Count);
    }

    [Fact]
    public async Task ListShowsTheLobbies()
    {
        var (id, _) = await CreateLobby();
        await JoinLobby(id, "Alice");
        var body = await Json(await client.GetAsync("/lobbies"));
        var lobby = body["lobbies"]!.AsArray().Single()!;
        Assert.Equal(id, (string?)lobby["id"]);
        Assert.Equal("Salon de Bob", (string?)lobby["name"]);
        Assert.Equal((2, 8, "lobby"), ((int)lobby["players"]!, (int)lobby["maxPlayers"]!, (string?)lobby["state"]));
        Assert.Null(lobby["minutes"]);
        Assert.True((bool)lobby["canJoin"]!);
        Assert.False((bool)lobby["canWatch"]!);
    }

    [Fact]
    public async Task ListCountsClosedSlotsOutOfMaxPlayers()
    {
        var (id, bob) = await CreateLobby();
        Assert.Equal(HttpStatusCode.OK, (await Send(HttpMethod.Put, $"/lobbies/{id}/slots/7", bob, new { state = "closed" })).StatusCode);
        var lobby = (await Json(await client.GetAsync("/lobbies")))["lobbies"]![0]!;
        Assert.Equal(7, (int)lobby["maxPlayers"]!);
    }

    [Fact]
    public async Task ListIsEmptyAtFirst()
    {
        var body = await Json(await client.GetAsync("/lobbies"));
        Assert.Empty(body["lobbies"]!.AsArray());
    }

    [Fact]
    public async Task JoinGivesTheNextSlot()
    {
        var (id, _) = await CreateLobby();
        var response = await client.PostAsJsonAsync($"/lobbies/{id}/join", new { playerName = "Alice", gameVersion = Version });
        Assert.Equal(HttpStatusCode.OK, response.StatusCode);
        var body = await Json(response);
        Assert.Equal(1, (int)body["slot"]!);
        Assert.Equal("Alice", (string?)body["lobby"]!["you"]);
        Assert.Equal("Bob", (string?)body["lobby"]!["leader"]);
    }

    [Fact]
    public async Task JoinErrors()
    {
        var (id, _) = await CreateLobby();
        await AssertError(await client.PostAsJsonAsync($"/lobbies/{id}/join", new { playerName = "BOB", gameVersion = Version }),
            HttpStatusCode.Conflict, "name_taken");
        await AssertError(await client.PostAsJsonAsync("/lobbies/L99/join", new { playerName = "Alice", gameVersion = Version }),
            HttpStatusCode.NotFound, "lobby_not_found");
        for (var i = 1; i < 8; i++)
        {
            await JoinLobby(id, $"P{i}");
        }
        await AssertError(await client.PostAsJsonAsync($"/lobbies/{id}/join", new { playerName = "Alice", gameVersion = Version }),
            HttpStatusCode.Conflict, "lobby_full");
    }

    [Fact]
    public async Task ReadGivesTheStateAndCountsAsPresence()
    {
        var (id, bob) = await CreateLobby();
        var alice = await JoinLobby(id, "Alice");
        clock.UtcNow += TimeSpan.FromSeconds(3);
        var response = await Send(HttpMethod.Get, $"/lobbies/{id}", alice);
        Assert.Equal(HttpStatusCode.OK, response.StatusCode);
        Assert.Equal("Alice", (string?)(await Json(response))["you"]);
        clock.UtcNow += TimeSpan.FromSeconds(3);
        var state = await Json(await Send(HttpMethod.Get, $"/lobbies/{id}", alice));
        Assert.Equal("Alice", (string?)state["leader"]);
        Assert.Equal("leader_changed", (string?)state["notice"]);
        Assert.Equal("open", (string?)state["slots"]![0]!["state"]);
        await AssertError(await Send(HttpMethod.Get, $"/lobbies/{id}", bob), HttpStatusCode.Forbidden, "not_in_lobby");
    }

    [Fact]
    public async Task ReadWithoutTokenHasNoYou()
    {
        var (id, _) = await CreateLobby();
        var response = await client.GetAsync($"/lobbies/{id}");
        Assert.Equal(HttpStatusCode.OK, response.StatusCode);
        var body = await Json(response);
        Assert.True(body.AsObject().ContainsKey("you"));
        Assert.Null(body["you"]);
        await AssertError(await client.GetAsync("/lobbies/L99"), HttpStatusCode.NotFound, "lobby_not_found");
        await AssertError(await Send(HttpMethod.Get, $"/lobbies/{id}", "0123"), HttpStatusCode.Forbidden, "not_in_lobby");
    }

    [Fact]
    public async Task MyLineChangesColorTeamAndReady()
    {
        var (id, bob) = await CreateLobby();
        var response = await Send(HttpMethod.Put, $"/lobbies/{id}/me", bob, new { colorIndex = 4, team = 2, ready = true });
        Assert.Equal(HttpStatusCode.OK, response.StatusCode);
        var line = (await Json(response))["slots"]![0]!;
        Assert.Equal((4, 2, true), ((int)line["colorIndex"]!, (int)line["team"]!, (bool)line["ready"]!));
    }

    [Fact]
    public async Task MyLineErrors()
    {
        var (id, bob) = await CreateLobby();
        await JoinLobby(id, "Alice");
        await AssertError(await Send(HttpMethod.Put, $"/lobbies/{id}/me", bob, new { colorIndex = 1 }), HttpStatusCode.Conflict, "color_taken");
        await AssertError(await Send(HttpMethod.Put, $"/lobbies/{id}/me", bob, new { team = 5 }), HttpStatusCode.BadRequest, "bad_request");
        await AssertError(await Send(HttpMethod.Put, $"/lobbies/{id}/me", bob, new { team = "x" }), HttpStatusCode.BadRequest, "bad_request");
        await AssertError(await Send(HttpMethod.Put, $"/lobbies/{id}/me", null, new { team = 1 }), HttpStatusCode.Forbidden, "not_in_lobby");
    }

    [Fact]
    public async Task SettingsByTheLeader()
    {
        var (id, bob) = await CreateLobby();
        var response = await Send(HttpMethod.Put, $"/lobbies/{id}/settings", bob, new { nomad = true });
        Assert.Equal(HttpStatusCode.OK, response.StatusCode);
        Assert.True((bool)(await Json(response))["settings"]!["nomad"]!);
    }

    [Fact]
    public async Task SettingsErrors()
    {
        var (id, bob) = await CreateLobby();
        var alice = await JoinLobby(id, "Alice");
        await AssertError(await Send(HttpMethod.Put, $"/lobbies/{id}/settings", alice, new { nomad = true }), HttpStatusCode.Forbidden, "not_leader");
        await AssertError(await Send(HttpMethod.Put, $"/lobbies/{id}/settings", bob, new { map = "Moon" }), HttpStatusCode.BadRequest, "bad_request");
    }

    [Fact]
    public async Task SlotOpenAndClose()
    {
        var (id, bob) = await CreateLobby();
        var response = await Send(HttpMethod.Put, $"/lobbies/{id}/slots/3", bob, new { state = "closed" });
        Assert.Equal(HttpStatusCode.OK, response.StatusCode);
        Assert.Equal("closed", (string?)(await Json(response))["slots"]![3]!["state"]);
        response = await Send(HttpMethod.Put, $"/lobbies/{id}/slots/3", bob, new { state = "open" });
        Assert.Equal("open", (string?)(await Json(response))["slots"]![3]!["state"]);
    }

    [Fact]
    public async Task SlotErrors()
    {
        var (id, bob) = await CreateLobby();
        var alice = await JoinLobby(id, "Alice");
        await AssertError(await Send(HttpMethod.Put, $"/lobbies/{id}/slots/1", bob, new { state = "closed" }), HttpStatusCode.Conflict, "slot_taken");
        await AssertError(await Send(HttpMethod.Put, $"/lobbies/{id}/slots/3", alice, new { state = "closed" }), HttpStatusCode.Forbidden, "not_leader");
        await AssertError(await Send(HttpMethod.Put, $"/lobbies/{id}/slots/8", bob, new { state = "closed" }), HttpStatusCode.BadRequest, "bad_request");
        await AssertError(await Send(HttpMethod.Put, $"/lobbies/{id}/slots/x", bob, new { state = "closed" }), HttpStatusCode.BadRequest, "bad_request");
        await AssertError(await Send(HttpMethod.Put, $"/lobbies/{id}/slots/3", bob, new { state = "ajar" }), HttpStatusCode.BadRequest, "bad_request");
        await AssertError(await Send(HttpMethod.Put, $"/lobbies/{id}/slots/3", bob, new { }), HttpStatusCode.BadRequest, "bad_request");
    }

    [Fact]
    public async Task LeaveRemovesAndDeletesTheEmptyLobby()
    {
        var (id, bob) = await CreateLobby();
        Assert.Equal(HttpStatusCode.NoContent, (await Send(HttpMethod.Post, $"/lobbies/{id}/leave", bob)).StatusCode);
        Assert.Equal(0, Store.Count);
        await AssertError(await Send(HttpMethod.Post, $"/lobbies/{id}/leave", bob), HttpStatusCode.NotFound, "lobby_not_found");
    }

    [Fact]
    public async Task LeaveWithUnknownTokenIsRefused()
    {
        var (id, _) = await CreateLobby();
        await AssertError(await Send(HttpMethod.Post, $"/lobbies/{id}/leave", "nope"), HttpStatusCode.Forbidden, "not_in_lobby");
        Assert.Equal(1, Store.Count);
    }

    [Fact]
    public async Task StartAnswers202WithMatchIdAndLobbyIsStarting()
    {
        var (id, bob) = await CreateLobby();
        var alice = await JoinLobby(id, "Alice");
        await AssertError(await Send(HttpMethod.Post, $"/lobbies/{id}/start", alice), HttpStatusCode.Forbidden, "not_leader");
        await AssertError(await Send(HttpMethod.Post, $"/lobbies/{id}/start", bob), HttpStatusCode.Conflict, "not_all_ready");
        await Send(HttpMethod.Put, $"/lobbies/{id}/me", bob, new { ready = true });
        await Send(HttpMethod.Put, $"/lobbies/{id}/me", alice, new { ready = true });
        var response = await Send(HttpMethod.Post, $"/lobbies/{id}/start", bob);
        Assert.Equal(HttpStatusCode.Accepted, response.StatusCode);
        var matchId = (string)(await Json(response))["matchId"]!;
        Assert.Matches(@"^M\d{8}-\d{6}-\d+$", matchId);
        var state = await Json(await Send(HttpMethod.Get, $"/lobbies/{id}", bob));
        Assert.Equal("starting", (string?)state["state"]);
        Assert.Null(state["match"]);
        var list = await Json(await client.GetAsync("/lobbies"));
        Assert.Equal("starting", (string?)list["lobbies"]![0]!["state"]);
        // The fake server was started with the match.json of the lobby.
        var commandLine = ProcessServerLauncherTests.WaitForFile(Path.Combine(matchesDir, matchId, FakeServerProgram.CommandLineFile));
        Assert.Contains($"-MatchId={matchId} ", commandLine);
        Assert.Equal(Store.Atomic(() => Store.Find(id)!.Match!.Config.ToJson()),
            ProcessServerLauncherTests.WaitForFile(Path.Combine(matchesDir, matchId, "match.json")));
        Assert.Equal(1, (int)(await Json(await client.GetAsync("/status")))["servers"]!);
    }

    private async Task<(string Id, string Bob)> ReadyLobby(string name)
    {
        var (id, bob) = await CreateLobby(name);
        var alice = await JoinLobby(id, "Alice");
        await Send(HttpMethod.Put, $"/lobbies/{id}/me", bob, new { ready = true });
        await Send(HttpMethod.Put, $"/lobbies/{id}/me", alice, new { ready = true });
        return (id, bob);
    }

    [Fact]
    public async Task StartBeyondMaxServersIs503AndTheLobbyStays()
    {
        var first = await ReadyLobby("A");
        var second = await ReadyLobby("B");
        Assert.Equal(HttpStatusCode.Accepted, (await Send(HttpMethod.Post, $"/lobbies/{first.Id}/start", first.Bob)).StatusCode);
        await AssertError(await Send(HttpMethod.Post, $"/lobbies/{second.Id}/start", second.Bob), HttpStatusCode.ServiceUnavailable, "max_servers");
        var state = await Json(await Send(HttpMethod.Get, $"/lobbies/{second.Id}", second.Bob));
        Assert.Equal("lobby", (string?)state["state"]);
    }

    [Fact]
    public async Task StartWithoutServerExeIs500AndTheLobbyStays()
    {
        using var noExe = app.WithWebHostBuilder(b => b.UseSetting("ServerExe", Path.Combine(matchesDir, "nope.exe")));
        var http = noExe.CreateClient();
        var created = await (await http.PostAsJsonAsync("/lobbies", new { name = "A", playerName = "Bob", gameVersion = Version })).Content.ReadFromJsonAsync<JsonNode>();
        var (id, bob) = ((string)created!["lobby"]!["id"]!, (string)created["token"]!);
        var joined = await (await http.PostAsJsonAsync($"/lobbies/{id}/join", new { playerName = "Alice", gameVersion = Version })).Content.ReadFromJsonAsync<JsonNode>();
        var alice = (string)joined!["token"]!;
        foreach (var token in new[] { bob, alice })
        {
            var ready = new HttpRequestMessage(HttpMethod.Put, $"/lobbies/{id}/me") { Content = JsonContent.Create(new { ready = true }) };
            ready.Headers.Add("X-Player-Token", token);
            Assert.Equal(HttpStatusCode.OK, (await http.SendAsync(ready)).StatusCode);
        }
        var start = new HttpRequestMessage(HttpMethod.Post, $"/lobbies/{id}/start");
        start.Headers.Add("X-Player-Token", bob);
        await AssertError(await http.SendAsync(start), HttpStatusCode.InternalServerError, "server_start_failed");
        Assert.Equal("lobby", (string?)(await http.GetFromJsonAsync<JsonNode>($"/lobbies/{id}"))!["state"]);
    }

    [Fact]
    public async Task StartAloneIsRefused()
    {
        var (id, bob) = await CreateLobby();
        await AssertError(await Send(HttpMethod.Post, $"/lobbies/{id}/start", bob), HttpStatusCode.Conflict, "not_enough_players");
        await AssertError(await Send(HttpMethod.Post, $"/lobbies/{id}/start", "nope"), HttpStatusCode.Forbidden, "not_in_lobby");
        await AssertError(await Send(HttpMethod.Post, "/lobbies/L99/start", bob), HttpStatusCode.NotFound, "lobby_not_found");
    }

    /** The test server has no remote address: 127.0.0.1 unless the X-Test-Remote header gives another one. */
    private sealed class RemoteAddressFilter : IStartupFilter
    {
        public Action<IApplicationBuilder> Configure(Action<IApplicationBuilder> next) => app =>
        {
            app.Use(async (context, nextMiddleware) =>
            {
                context.Connection.RemoteIpAddress = context.Request.Headers.TryGetValue("X-Test-Remote", out var remote)
                    ? IPAddress.Parse(remote.ToString())
                    : IPAddress.Loopback;
                await nextMiddleware(context);
            });
            next(app);
        };
    }

    private Task<HttpResponseMessage> Server(string matchId, string route, object? body = null, string? remote = null)
    {
        var request = new HttpRequestMessage(HttpMethod.Post, $"/matches/{matchId}/{route}")
        {
            Content = JsonContent.Create(body ?? new { }),
        };
        if (remote != null)
        {
            request.Headers.Add("X-Test-Remote", remote);
        }
        return client.SendAsync(request);
    }

    private async Task<(string Id, string Bob, string MatchId)> StartedLobby()
    {
        var (id, bob) = await ReadyLobby("Salon de Bob");
        var response = await Send(HttpMethod.Post, $"/lobbies/{id}/start", bob);
        Assert.Equal(HttpStatusCode.Accepted, response.StatusCode);
        return (id, bob, (string)(await Json(response))["matchId"]!);
    }

    [Fact]
    public async Task ServerRoutesFollowTheMatch()
    {
        var (id, bob, matchId) = await StartedLobby();
        Assert.Equal(HttpStatusCode.NoContent, (await Server(matchId, "listening", new { port = 47900 })).StatusCode);
        var state = await Json(await Send(HttpMethod.Get, $"/lobbies/{id}", bob));
        Assert.Equal("in_game", (string?)state["state"]);
        Assert.Equal(0, (int)state["minutes"]!);
        var match = state["match"]!;
        Assert.Equal((matchId, 47900, "Bob", Version),
            ((string?)match["matchId"], (int)match["port"]!, (string?)match["playerName"], (string?)match["gameVersion"]));
        var entry = (await Json(await client.GetAsync("/lobbies")))["lobbies"]![0]!;
        Assert.Equal(("in_game", 0, false, true),
            ((string?)entry["state"], (int)entry["minutes"]!, (bool)entry["canJoin"]!, (bool)entry["canWatch"]!));

        Assert.Equal(HttpStatusCode.NoContent, (await Server(matchId, "started")).StatusCode);
        clock.UtcNow += TimeSpan.FromMinutes(12);
        var watch = await client.GetAsync($"/lobbies/{id}/watch");
        Assert.Equal(HttpStatusCode.OK, watch.StatusCode);
        var view = await Json(watch);
        Assert.Equal((matchId, 47900, 12), ((string?)view["matchId"], (int)view["port"]!, (int)view["minutes"]!));
        Assert.Equal(12, (int)(await Json(await client.GetAsync("/lobbies")))["lobbies"]![0]!["minutes"]!);

        Assert.Equal(HttpStatusCode.NoContent, (await Server(matchId, "finished", new { winners = new[] { 0 } })).StatusCode);
        // D2: the match's players are out of the lobby; it stays, read without a token.
        await AssertError(await Send(HttpMethod.Get, $"/lobbies/{id}", bob), HttpStatusCode.Forbidden, "not_in_lobby");
        state = await Json(await client.GetAsync($"/lobbies/{id}"));
        Assert.All(state["slots"]!.AsArray(), slot => Assert.NotEqual("player", (string?)slot!["state"]));
        Assert.Equal("lobby", (string?)state["state"]);
        Assert.Null(state["match"]);
        Assert.Null(state["minutes"]);
        Assert.Null(state["notice"]);
        await AssertError(await client.GetAsync($"/lobbies/{id}/watch"), HttpStatusCode.Conflict, "no_match");
        // M6-Q14: the match's leader still leads while away, by name.
        Assert.Equal("Bob", (string?)state["leader"]);
        Assert.Equal(HttpStatusCode.NoContent, (await Server(matchId, "left", new { playerName = "Bob" })).StatusCode);

        // The fake server lingers after "empty": stopped 10 s later.
        Assert.Equal(HttpStatusCode.NoContent, (await Server(matchId, "empty")).StatusCode);
        Assert.Equal(1, (int)(await Json(await client.GetAsync("/status")))["servers"]!);
        clock.UtcNow += TimeSpan.FromSeconds(10);
        Assert.Equal(0, (int)(await Json(await client.GetAsync("/status")))["servers"]!);
        await AssertError(await Server(matchId, "empty"), HttpStatusCode.NotFound, "match_not_found");
    }

    [Fact]
    public async Task TheLeaderBackOnTheServerStillLeadsAfterTheMatch()
    {
        // M6-Q15: left then back within R7's window, he counts as present at "finished".
        var (id, _, matchId) = await StartedLobby();
        Assert.Equal(HttpStatusCode.NoContent, (await Server(matchId, "listening", new { port = 47900 })).StatusCode);
        Assert.Equal(HttpStatusCode.NoContent, (await Server(matchId, "started")).StatusCode);
        Assert.Equal(HttpStatusCode.NoContent, (await Server(matchId, "left", new { playerName = "Bob" })).StatusCode);
        Assert.Equal(HttpStatusCode.NoContent, (await Server(matchId, "back", new { playerName = "Bob" })).StatusCode);
        Assert.Equal(HttpStatusCode.NoContent, (await Server(matchId, "finished", new { winners = new[] { 0 } })).StatusCode);
        await JoinLobby(id, "Alice");
        Assert.Equal("Bob", (string?)(await Json(await client.GetAsync($"/lobbies/{id}")))["leader"]);
        await JoinLobby(id, "Bob");
        Assert.Equal("Bob", (string?)(await Json(await client.GetAsync($"/lobbies/{id}")))["leader"]);
    }

    [Fact]
    public async Task TheListNamesThePlayersAwayFromTheMatchWithinTheWindow()
    {
        // R7.6: « Revenir » for a player gone from the match's server (left, not back) within R7's 180 s.
        var (_, _, matchId) = await StartedLobby();
        async Task<List<string?>> Away() =>
            (await Json(await client.GetAsync("/lobbies")))["lobbies"]![0]!["away"]!.AsArray().Select(n => (string?)n).ToList();
        Assert.Empty(await Away());
        Assert.Equal(HttpStatusCode.NoContent, (await Server(matchId, "listening", new { port = 47900 })).StatusCode);
        Assert.Equal(HttpStatusCode.NoContent, (await Server(matchId, "started")).StatusCode);
        Assert.Empty(await Away());
        Assert.Equal(HttpStatusCode.NoContent, (await Server(matchId, "left", new { playerName = "bob" })).StatusCode);
        Assert.Equal(["Bob"], await Away());
        Assert.Equal(HttpStatusCode.NoContent, (await Server(matchId, "back", new { playerName = "Bob" })).StatusCode);
        Assert.Empty(await Away());
        Assert.Equal(HttpStatusCode.NoContent, (await Server(matchId, "left", new { playerName = "Bob" })).StatusCode);
        clock.UtcNow += LobbyRules.DisconnectWindow - TimeSpan.FromSeconds(1);
        Assert.Equal(["Bob"], await Away());
        // Window over: the server has defeated his slot, nothing to come back to.
        clock.UtcNow += TimeSpan.FromSeconds(1);
        Assert.Empty(await Away());
        // A name absent from the match changes nothing; after "finished", nobody is away.
        Assert.Equal(HttpStatusCode.NoContent, (await Server(matchId, "left", new { playerName = "Chloé" })).StatusCode);
        Assert.Empty(await Away());
        Assert.Equal(HttpStatusCode.NoContent, (await Server(matchId, "left", new { playerName = "Bob" })).StatusCode);
        Assert.Equal(HttpStatusCode.NoContent, (await Server(matchId, "finished", new { winners = new[] { 0 } })).StatusCode);
        Assert.Empty(await Away());
    }

    [Fact]
    public async Task ServerRoutesErrors()
    {
        var (_, _, matchId) = await StartedLobby();
        await AssertError(await Server(matchId, "listening", new { port = 47900 }, remote: "100.64.0.7"), HttpStatusCode.Forbidden, "not_local");
        await AssertError(await Server("M99", "listening", new { port = 47900 }, remote: "100.64.0.7"), HttpStatusCode.Forbidden, "not_local");
        await AssertError(await Server("M99", "listening", new { port = 47900 }), HttpStatusCode.NotFound, "match_not_found");
        await AssertError(await Server("M99", "started"), HttpStatusCode.NotFound, "match_not_found");
        await AssertError(await Server("M99", "finished"), HttpStatusCode.NotFound, "match_not_found");
        await AssertError(await Server("M99", "empty"), HttpStatusCode.NotFound, "match_not_found");
        await AssertError(await Server("M99", "left", new { playerName = "Bob" }), HttpStatusCode.NotFound, "match_not_found");
        await AssertError(await Server(matchId, "left", new { playerName = "Bob" }, remote: "100.64.0.7"), HttpStatusCode.Forbidden, "not_local");
        await AssertError(await Server(matchId, "left"), HttpStatusCode.BadRequest, "bad_request");
        await AssertError(await Server("M99", "back", new { playerName = "Bob" }), HttpStatusCode.NotFound, "match_not_found");
        await AssertError(await Server(matchId, "back", new { playerName = "Bob" }, remote: "100.64.0.7"), HttpStatusCode.Forbidden, "not_local");
        await AssertError(await Server(matchId, "back"), HttpStatusCode.BadRequest, "bad_request");
        await AssertError(await Server("M99", "back"), HttpStatusCode.BadRequest, "bad_request");
        await AssertError(await client.PostAsync($"/matches/{matchId}/listening", new StringContent("{ not json", Encoding.UTF8, "application/json")),
            HttpStatusCode.BadRequest, "bad_request");
        await AssertError(await Server(matchId, "listening", new { port = "x" }), HttpStatusCode.BadRequest, "bad_request");
        // A body-less call is accepted.
        Assert.Equal(HttpStatusCode.NoContent, (await client.PostAsync($"/matches/{matchId}/started", null)).StatusCode);
    }

    [Fact]
    public async Task ServerGoneShowsTheNoticeToThePlayers()
    {
        var (id, bob, matchId) = await StartedLobby();
        Assert.Equal(HttpStatusCode.NoContent, (await Server(matchId, "listening", new { port = 47900 })).StatusCode);
        var pid = ProcessServerLauncherTests.WaitForPid(Path.Combine(matchesDir, matchId));
        // Time moves before the kill: the housekeeping timer (every second) may see the exit before the GET, and
        // the notice and reservations it sets must not be aged by a later clock move.
        clock.UtcNow += TimeSpan.FromMinutes(5);
        using (var process = System.Diagnostics.Process.GetProcessById(pid))
        {
            process.Kill();
            Assert.True(process.WaitForExit(10_000), $"Fake server {pid} did not stop.");
        }
        var state = await Json(await Send(HttpMethod.Get, $"/lobbies/{id}", bob));
        Assert.Equal("lobby", (string?)state["state"]);
        Assert.Equal("server_stopped", (string?)state["notice"]);
        Assert.Equal("player", (string?)state["slots"]![1]!["state"]);
    }

    [Fact]
    public async Task WatchWithoutMatchIsRefused()
    {
        var (id, _) = await CreateLobby();
        await AssertError(await client.GetAsync($"/lobbies/{id}/watch"), HttpStatusCode.Conflict, "no_match");
        await AssertError(await client.GetAsync("/lobbies/L99/watch"), HttpStatusCode.NotFound, "lobby_not_found");
        var (startedId, _, _) = await StartedLobby();
        await AssertError(await client.GetAsync($"/lobbies/{startedId}/watch"), HttpStatusCode.Conflict, "no_match");
    }
}
