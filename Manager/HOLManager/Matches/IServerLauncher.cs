using HOLManager.Lobbies;

namespace HOLManager.Matches;

/** Starts the dedicated server of a match (R2.9); replaced in the tests. */
public interface IServerLauncher
{
    /** Servers started and still running. */
    int Running { get; }

    /**
     * Writes match.json and starts the server of the match on a free port, which it returns. Throws LobbyException
     * max_servers (MaxServers already running) or server_start_failed.
     */
    int Launch(string matchId, MapInfo map, MatchConfig config);

    /** True once the match's server has stopped (or for a match it never started). */
    bool HasExited(string matchId);

    /** Stops the match's server if it still runs. */
    void Stop(string matchId);
}
