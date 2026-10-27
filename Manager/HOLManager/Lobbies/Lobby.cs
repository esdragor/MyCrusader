namespace HOLManager.Lobbies;

/** Time source of the lobby rules, replaced in the tests (R2.6: 5 s without reading). */
public interface IClock
{
    DateTimeOffset UtcNow { get; }
}

public sealed class SystemClock : IClock
{
    public DateTimeOffset UtcNow => DateTimeOffset.UtcNow;
}

/** A refused request: Error is the stable code of Manager/PROTOCOL.md. */
public sealed class LobbyException(string error, string message) : Exception(message)
{
    public string Error { get; } = error;
}

/** Error codes of Manager/PROTOCOL.md used by the lobby rules. */
public static class LobbyErrors
{
    public const string BadRequest = "bad_request";
    public const string InvalidName = "invalid_name";
    public const string VersionMismatch = "version_mismatch";
    public const string LobbyNotFound = "lobby_not_found";
    public const string NotInLobby = "not_in_lobby";
    public const string LobbyFull = "lobby_full";
    public const string NameTaken = "name_taken";
    public const string LobbyInGame = "lobby_in_game";
    public const string ColorTaken = "color_taken";
    public const string NotLeader = "not_leader";
    public const string SlotTaken = "slot_taken";
    public const string NoMatch = "no_match";
    public const string NotAllReady = "not_all_ready";
    public const string NotEnoughPlayers = "not_enough_players";
    public const string MaxServers = "max_servers";
    public const string ServerStartFailed = "server_start_failed";
    public const string NotLocal = "not_local";
    public const string MatchNotFound = "match_not_found";
    public const string BadKey = "bad_key";
}

/** Notices of the lobby state (PROTOCOL.md, notice): shown once, cleared after 10 s. */
public static class LobbyNotices
{
    public const string LeaderChanged = "leader_changed";
    public const string ServerStopped = "server_stopped";
}

public enum LobbyState
{
    Lobby,
    Starting,
    InGame,
}

public enum SlotState
{
    Open,
    Closed,
    Player,
}

public sealed class LobbyPlayer
{
    public required string Name { get; init; }
    /** Changed when the player comes back by name after a match (R2.11). */
    public string Token { get; internal set; } = "";
    public required int SlotIndex { get; init; }
    public int ColorIndex { get; internal set; }
    public int Team { get; internal set; }
    public string Faction { get; internal set; } = LobbyRules.DefaultFaction;
    public bool Ready { get; internal set; }

    /** Order of arrival: the earliest one still here is the leader. */
    public long JoinOrder { get; internal set; }
    public DateTimeOffset LastSeen { get; internal set; }

    /** After a match (R2.11): not removed by silence and name reserved until then; null otherwise. */
    public DateTimeOffset? ReservedUntil { get; internal set; }

    /** False from the return to the lobby until the player's first call (or join by name) after it. */
    public bool Back { get; internal set; } = true;
}

public sealed class LobbySlot(int index)
{
    public int Index { get; } = index;
    public bool Closed { get; internal set; }
    public LobbyPlayer? Player { get; internal set; }
    public SlotState State => Player != null ? SlotState.Player : Closed ? SlotState.Closed : SlotState.Open;
}

public sealed class LobbySettings
{
    public string Map { get; internal set; } = "";
    public bool Nomad { get; internal set; }
    public bool Regicide { get; internal set; }
    public bool Catastrophe { get; internal set; }
}

public sealed class Lobby
{
    public required string Id { get; init; }
    public required string Name { get; init; }
    public LobbyState State { get; internal set; } = LobbyState.Lobby;
    public required LobbySettings Settings { get; init; }
    public IReadOnlyList<LobbySlot> Slots { get; } = Enumerable.Range(0, LobbyRules.SlotCount).Select(i => new LobbySlot(i)).ToArray();

    /** Players in order of arrival. */
    public IEnumerable<LobbyPlayer> Players => Slots.Select(s => s.Player).OfType<LobbyPlayer>().OrderBy(p => p.JoinOrder);

    /** The earliest arrival among the players back (the match's leader keeps his order); none back: the earliest. */
    public LobbyPlayer? Leader => Players.FirstOrDefault(p => p.Back) ?? Players.FirstOrDefault();

    /**
     * After "finished" (M6-Q14): the match's leader, not back yet, still leads by name and nobody else sets or
     * starts; null otherwise. Kept, without blocking anybody, while nobody is back.
     */
    public string? AwayLeader { get; internal set; }

    /** The away leader left the match's server, or it stopped: he leads until then only (LeaderReturnGrace). */
    public DateTimeOffset? AwayLeaderUntil { get; internal set; }

    /** The leader's name of PROTOCOL.md: the away leader's, else the leading player's. */
    public string? LeaderName => AwayLeader ?? Leader?.Name;

    /**
     * The colors of the last finished match's players, by name (E2-1): one of them coming back by join takes his
     * color again if no player in the lobby has it.
     */
    public Dictionary<string, int> LastMatchColors { get; internal set; } = new(LobbyRules.NameComparer);

    /** The match being started or played (state starting or in_game), null at the lobby. */
    public LobbyMatch? Match { get; internal set; }

    public string? Notice { get; internal set; }
    public DateTimeOffset NoticeUntil { get; internal set; }
}

/**
 * A match started from a lobby, followed until its server stops (R2.10). Port is its server's (0 without a server
 * launcher, store tests).
 */
public sealed class LobbyMatch(string id, HOLManager.Matches.MatchConfig config, int port, DateTimeOffset launchedAt)
{
    public string Id { get; } = id;
    public HOLManager.Matches.MatchConfig Config { get; } = config;
    public int Port { get; internal set; } = port;
    public DateTimeOffset LaunchedAt { get; } = launchedAt;

    /** "listening" received: the lobby is in_game. */
    public bool Listening { get; internal set; }

    /** "started" received: the minutes count from here. */
    public DateTimeOffset? StartedAt { get; internal set; }

    /** "finished" received: the lobby went back to the lobby; the server is still followed until it stops. */
    public bool Finished { get; internal set; }

    public IReadOnlyList<int> Winners { get; internal set; } = [];

    /** Names of the players who left the server before "finished" (M6-Q14: such a leader leads no more after it). */
    public HashSet<string> LeftBeforeEnd { get; } = new(LobbyRules.NameComparer);

    /** R7.6: the match's players gone from its server and not back, by their name in the match, with when they left. */
    public Dictionary<string, DateTimeOffset> AwaySince { get; } = new(LobbyRules.NameComparer);

    /** R7.6: the names of AwaySince still within the server's window (GET /lobbies, « Revenir »). */
    public IEnumerable<string> Away(DateTimeOffset now) =>
        AwaySince.Where(p => now - p.Value < LobbyRules.DisconnectWindow).Select(p => p.Key).Order(StringComparer.Ordinal);

    /** "empty" received: the server is stopped if it still runs EmptyStopDelay later. */
    public DateTimeOffset? EmptyAt { get; internal set; }

    /** Whole minutes since "started", 0 before it. */
    public int Minutes(DateTimeOffset now) => StartedAt is { } start ? (int)Math.Floor((now - start).TotalMinutes) : 0;
}

/** What entering a lobby gives back (PROTOCOL.md, POST /lobbies and /join). */
public sealed record JoinResult(Lobby Lobby, string Token, int Slot);
