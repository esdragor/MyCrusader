namespace HOLLauncher;

public enum GameState
{
    NotInstalled,
    UpToDate,
    /** Installed version differs from the latest one (not "older": names are not ordered). */
    Outdated,
    /**
     * No latest version known: server unreachable or answer refused (Message says which, KeyRefused when the manager
     * refused the access key). Installed may be null.
     */
    Unreachable,
}

/**
 * What the window shows (L5b): the game's state, both versions, the news, and whether a newer launcher exists. Notes:
 * the news since the installed version (ReleaseNotes, L21). KeyRefused (L16): the manager answered 401 bad_key, a
 * signal of its own so that the views say it in the player's language (Message stays the technical detail, never
 * compared).
 */
public sealed record LauncherStatus(
    GameState State,
    string? Installed,
    string? Latest,
    string Notes,
    string LauncherCurrent,
    string LauncherAvailable,
    string Message,
    bool KeyRefused = false)
{
    /** A launcher is published and it is not this one. */
    public bool LauncherUpdateAvailable => LauncherAvailable != "" && LauncherAvailable != LauncherCurrent;

    public static LauncherStatus Compute(string? installed, LatestResult latest, string launcherCurrent = LauncherInfo.Version)
    {
        if (latest.Outcome != LatestOutcome.Ok || latest.Latest is null)
        {
            return new LauncherStatus(GameState.Unreachable, installed, null, "", launcherCurrent, "", latest.Message,
                KeyRefused: latest.Outcome == LatestOutcome.BadKey);
        }
        var available = latest.Latest;
        var state = installed is null ? GameState.NotInstalled
            : installed == available.Version ? GameState.UpToDate
            : GameState.Outdated;
        return new LauncherStatus(state, installed, available.Version, ReleaseNotes.Text(installed, available), launcherCurrent,
            available.LauncherVersion, latest.Message);
    }
}
