using System.Net;
using System.Text.Json;
using HOLManager.Lobbies;

namespace HOLManager.Api;

/** The routes of Manager/PROTOCOL.md (players R2.7, game server R2.10). Every answer is built under the store's lock. */
public static class LobbyRoutes
{
    public const string TokenHeader = "X-Player-Token";

    private sealed record CreateRequest(string? Name, string? PlayerName, string? GameVersion);

    private sealed record JoinRequest(string? PlayerName, string? GameVersion);

    private sealed record MeRequest(int? ColorIndex, int? Team, bool? Ready);

    private sealed record SettingsRequest(string? Map, bool? Nomad, bool? Regicide, bool? Catastrophe, string? Size = null,
        bool? TeamsTogether = null);

    private sealed record SlotRequest(string? State, int? ColorIndex = null, int? Team = null);

    /** HTTP status of each error code (PROTOCOL.md, Erreurs). */
    private static readonly Dictionary<string, int> Statuses = new()
    {
        [LobbyErrors.BadRequest] = 400,
        [LobbyErrors.InvalidName] = 400,
        [LobbyErrors.VersionMismatch] = 409,
        ["protocol_mismatch"] = 409,
        [LobbyErrors.LobbyNotFound] = 404,
        [LobbyErrors.NotInLobby] = 403,
        [LobbyErrors.LobbyFull] = 409,
        [LobbyErrors.NameTaken] = 409,
        [LobbyErrors.LobbyInGame] = 409,
        [LobbyErrors.ColorTaken] = 409,
        [LobbyErrors.NotLeader] = 403,
        [LobbyErrors.SlotTaken] = 409,
        [LobbyErrors.NotAllReady] = 409,
        [LobbyErrors.NotEnoughPlayers] = 409,
        [LobbyErrors.MaxServers] = 503,
        [LobbyErrors.ServerStartFailed] = 500,
        [LobbyErrors.NoMatch] = 409,
        [LobbyErrors.NotLocal] = 403,
        [LobbyErrors.MatchNotFound] = 404,
        [LobbyErrors.BadKey] = 401,
    };

    public static int StatusOf(string error) => Statuses.GetValueOrDefault(error, 500);

    /** Turns a refused request into the error body of PROTOCOL.md. */
    public static void UseLobbyErrors(this WebApplication app) =>
        app.Use(async (context, next) =>
        {
            try
            {
                await next(context);
            }
            catch (LobbyException e)
            {
                context.Response.StatusCode = StatusOf(e.Error);
                await context.Response.WriteAsJsonAsync(new { error = e.Error, message = e.Message });
            }
        });

    public static void MapLobbyRoutes(this WebApplication app)
    {
        app.MapGet("/lobbies", (LobbyStore store) =>
            store.Atomic(() => new { lobbies = store.List().Select(l => LobbyViews.Summary(l, store.Now)).ToList() }));

        app.MapPost("/lobbies", async (HttpRequest request, LobbyStore store) =>
        {
            var body = await Body<CreateRequest>(request);
            return Results.Json(store.Atomic(() => JoinView(store, store.Create(body.Name, body.PlayerName, body.GameVersion))),
                statusCode: StatusCodes.Status201Created);
        });

        app.MapPost("/lobbies/{lobbyId}/join", async (string lobbyId, HttpRequest request, LobbyStore store) =>
        {
            var body = await Body<JoinRequest>(request);
            return store.Atomic(() => JoinView(store, store.Join(lobbyId, body.PlayerName, body.GameVersion)));
        });

        app.MapGet("/lobbies/{lobbyId}", (string lobbyId, HttpRequest request, LobbyStore store) =>
        {
            var token = Token(request);
            return store.Atomic(() => LobbyViews.State(store, store.Read(lobbyId, token), token));
        });

        app.MapPut("/lobbies/{lobbyId}/me", async (string lobbyId, HttpRequest request, LobbyStore store) =>
        {
            var body = await Body<MeRequest>(request);
            var token = Token(request);
            return store.Atomic(() =>
                LobbyViews.State(store, store.UpdateMe(lobbyId, token, body.ColorIndex, body.Team, body.Ready), token));
        });

        app.MapPut("/lobbies/{lobbyId}/settings", async (string lobbyId, HttpRequest request, LobbyStore store) =>
        {
            var body = await Body<SettingsRequest>(request);
            var token = Token(request);
            return store.Atomic(() => LobbyViews.State(store,
                store.UpdateSettings(lobbyId, token, body.Map, body.Nomad, body.Regicide, body.Catastrophe, body.Size,
                    body.TeamsTogether), token));
        });

        app.MapPut("/lobbies/{lobbyId}/slots/{index}", async (string lobbyId, string index, HttpRequest request, LobbyStore store) =>
        {
            var body = await Body<SlotRequest>(request);
            var token = Token(request);
            var slotIndex = int.TryParse(index, out var i) ? i : -1;
            bool? open = body.State switch { "open" => true, "closed" => false, _ => null };
            // IA0.6 (Q4): "ai" puts an AI on the slot, or changes its colour and team.
            return store.Atomic(() => LobbyViews.State(store, body.State == "ai"
                ? store.SetSlotAI(lobbyId, token, slotIndex, body.ColorIndex, body.Team)
                : store.SetSlot(lobbyId, token, slotIndex, open), token));
        });

        app.MapPost("/lobbies/{lobbyId}/leave", (string lobbyId, HttpRequest request, LobbyStore store) =>
        {
            store.Leave(lobbyId, Token(request));
            return Results.NoContent();
        });

        app.MapPost("/lobbies/{lobbyId}/start", (string lobbyId, HttpRequest request, LobbyStore store) =>
            Results.Json(new { matchId = store.Start(lobbyId, Token(request)).Id }, statusCode: StatusCodes.Status202Accepted));

        app.MapGet("/lobbies/{lobbyId}/watch", (string lobbyId, LobbyStore store) => store.Atomic(() =>
        {
            var lobby = store.Read(lobbyId, null);
            return lobby.State == LobbyState.InGame
                ? LobbyViews.Watch(lobby, store.Now)
                : throw new LobbyException(LobbyErrors.NoMatch, $"Lobby {lobbyId} is not in a match.");
        }));
    }

    private static readonly JsonSerializerOptions WebJson = new(JsonSerializerDefaults.Web);

    private sealed record ListeningRequest(int? Port);

    private sealed record FinishedRequest(List<int>? Winners);

    private sealed record LeftRequest(string? PlayerName);

    /** The routes the game server calls (PROTOCOL.md, Routes du serveur Unreal): 127.0.0.1 only, answer 204. */
    public static void MapServerRoutes(this WebApplication app)
    {
        app.MapPost("/matches/{matchId}/listening", async (string matchId, HttpRequest request, LobbyStore store) =>
        {
            var body = await LocalBody<ListeningRequest>(request);
            store.ServerListening(matchId, body?.Port);
            return Results.NoContent();
        });
        app.MapPost("/matches/{matchId}/started", async (string matchId, HttpRequest request, LobbyStore store) =>
        {
            await LocalBody<object>(request);
            store.ServerStarted(matchId);
            return Results.NoContent();
        });
        app.MapPost("/matches/{matchId}/finished", async (string matchId, HttpRequest request, LobbyStore store) =>
        {
            var body = await LocalBody<FinishedRequest>(request);
            store.ServerFinished(matchId, body?.Winners);
            return Results.NoContent();
        });
        app.MapPost("/matches/{matchId}/left", async (string matchId, HttpRequest request, LobbyStore store) =>
        {
            var body = await LocalBody<LeftRequest>(request);
            store.ServerLeft(matchId, body?.PlayerName);
            return Results.NoContent();
        });
        app.MapPost("/matches/{matchId}/back", async (string matchId, HttpRequest request, LobbyStore store) =>
        {
            var body = await LocalBody<LeftRequest>(request);
            store.ServerBack(matchId, body?.PlayerName);
            return Results.NoContent();
        });
        app.MapPost("/matches/{matchId}/empty", async (string matchId, HttpRequest request, LobbyStore store) =>
        {
            await LocalBody<object>(request);
            store.ServerEmpty(matchId);
            return Results.NoContent();
        });
    }

    /** not_local first, then illegal JSON; an empty body is allowed (null). */
    private static async Task<T?> LocalBody<T>(HttpRequest request) where T : class
    {
        var remote = request.HttpContext.Connection.RemoteIpAddress;
        if (remote == null || !IPAddress.IsLoopback(remote.IsIPv4MappedToIPv6 ? remote.MapToIPv4() : remote))
        {
            throw new LobbyException(LobbyErrors.NotLocal, $"Game server routes are for 127.0.0.1, not {remote}.");
        }
        var text = await new StreamReader(request.Body).ReadToEndAsync();
        try
        {
            return string.IsNullOrWhiteSpace(text) ? null : JsonSerializer.Deserialize<T>(text, WebJson);
        }
        catch (JsonException e)
        {
            throw new LobbyException(LobbyErrors.BadRequest, $"Illegal JSON: {e.Message}");
        }
    }

    private static string? Token(HttpRequest request) =>
        request.Headers.TryGetValue(TokenHeader, out var token) ? token.ToString() : null;

    /** Illegal JSON is a bad_request, checked before the rules. */
    private static async Task<T> Body<T>(HttpRequest request) where T : class
    {
        try
        {
            return await request.ReadFromJsonAsync<T>()
                ?? throw new LobbyException(LobbyErrors.BadRequest, "Empty body.");
        }
        catch (Exception e) when (e is JsonException or InvalidOperationException)
        {
            throw new LobbyException(LobbyErrors.BadRequest, $"Illegal JSON: {e.Message}");
        }
    }

    private static object JoinView(LobbyStore store, JoinResult result) =>
        new { token = result.Token, slot = result.Slot, lobby = LobbyViews.State(store, result.Lobby, result.Token) };
}
