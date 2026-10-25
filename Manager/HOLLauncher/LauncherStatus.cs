namespace HOLLauncher;

public enum GameState
{
    NotInstalled,
    UpToDate,
    /** Installed version differs from the latest one (not "older": names are not ordered). */
    Outdated,
    /** No latest version known: server unreachable or answer refused (Message says which). Installed may be null. */
    Unreachable,
}

/** What the window shows (L5b): the game's state, both versions, the news, and whether a newer launcher exists. */
public sealed record LauncherStatus(
    GameState State,
    string? Installed,
    string? Latest,
    string Notes,
    string LauncherCurrent,
    string LauncherAvailable,
    string Message)
{
    /** A launcher is published and it is not this one. */
    public bool LauncherUpdateAvailable => LauncherAvailable != "" && LauncherAvailable != LauncherCurrent;

    public static LauncherStatus Compute(string? installed, LatestResult latest, string launcherCurrent = LauncherInfo.Version)
    {
        if (latest.Outcome != LatestOutcome.Ok || latest.Latest is null)
        {
            return new LauncherStatus(GameState.Unreachable, installed, null, "", launcherCurrent, "", latest.Message);
        }
        var available = latest.Latest;
        var state = installed is null ? GameState.NotInstalled
            : installed == available.Version ? GameState.UpToDate
            : GameState.Outdated;
        return new LauncherStatus(state, installed, available.Version, available.Notes, launcherCurrent,
            available.LauncherVersion, latest.Message);
    }
}
