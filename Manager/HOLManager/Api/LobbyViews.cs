using HOLManager.Lobbies;

namespace HOLManager.Api;

/** JSON shapes of PROTOCOL.md, copied from the live lobby: call them under the store's lock only. */
public static class LobbyViews
{
    public static string StateName(LobbyState state) => state switch
    {
        LobbyState.Starting => "starting",
        LobbyState.InGame => "in_game",
        _ => "lobby",
    };

    /** An entry of GET /lobbies. */
    public static object Summary(Lobby lobby, DateTimeOffset now) => new
    {
        id = lobby.Id,
        name = lobby.Name,
        // D481: the lobby's game version; a client of another version is refused (version_mismatch).
        gameVersion = lobby.GameVersion,
        players = lobby.Players.Count(),
        maxPlayers = lobby.Slots.Count(s => s.State != SlotState.Closed),
        state = StateName(lobby.State),
        minutes = Minutes(lobby, now),
        canJoin = lobby.State == LobbyState.Lobby && lobby.Slots.Any(s => s.State == SlotState.Open),
        canWatch = lobby.State == LobbyState.InGame,
        // R7.6: in game, the match's players gone from its server within the window (« Revenir »); [] otherwise.
        away = lobby.State == LobbyState.InGame && lobby.Match is { } match ? match.Away(now).ToList() : [],
    };

    /** GET /lobbies/{id}, also the answer of every change. */
    public static object State(LobbyStore store, Lobby lobby, string? token)
    {
        var you = token == null ? null : lobby.Players.FirstOrDefault(p => p.Token == token)?.Name;
        return new
        {
        id = lobby.Id,
        name = lobby.Name,
        state = StateName(lobby.State),
        minutes = Minutes(lobby, store.Now),
        leader = lobby.LeaderName,
        you,
        settings = new
        {
            map = lobby.Settings.Map,
            nomad = lobby.Settings.Nomad,
            regicide = lobby.Settings.Regicide,
            catastrophe = lobby.Settings.Catastrophe,
        },
        maps = store.Maps.Select(m => new { name = m.Name, maxPlayers = m.MaxPlayers }).ToList(),
        slots = lobby.Slots.Select(Slot).ToList(),
        // In game: what the client opens (PROTOCOL.md, Démarrage de la partie); playerName null without a token.
        match = lobby.State == LobbyState.InGame && lobby.Match is { } m
            ? new { matchId = m.Id, port = m.Port, playerName = you, gameVersion = lobby.GameVersion }
            : null,
        notice = lobby.Notice,
        };
    }

    /** GET /lobbies/{id}/watch of a lobby in game. */
    public static object Watch(Lobby lobby, DateTimeOffset now) =>
        new { matchId = lobby.Match!.Id, port = lobby.Match.Port, minutes = Minutes(lobby, now) };

    /** Whole minutes since "started" in game (0 before it), null otherwise. */
    private static int? Minutes(Lobby lobby, DateTimeOffset now) =>
        lobby.State == LobbyState.InGame ? lobby.Match?.Minutes(now) : null;

    private static object Slot(LobbySlot slot) => slot.Player is { } p
        ? (object)new { index = slot.Index, state = "player", playerName = p.Name, colorIndex = p.ColorIndex, team = p.Team, faction = p.Faction, ready = p.Ready }
        : new { index = slot.Index, state = slot.State == SlotState.Closed ? "closed" : "open" };
}
