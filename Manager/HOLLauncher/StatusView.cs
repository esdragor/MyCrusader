namespace HOLLauncher;

/**
 * What the window shows, computed from a LauncherStatus (pure, tested): the French texts and which buttons are
 * enabled. The two update buttons only signal that an update exists (they do nothing yet: L6/L7); "Play" is shown but
 * disabled until L8.
 */
public sealed record StatusView(
    string InstalledText,
    string LatestText,
    string StateText,
    string Notes,
    string LauncherText,
    bool PlayEnabled,
    bool UpdateGameEnabled,
    bool UpdateLauncherEnabled,
    bool RefreshEnabled)
{
    public bool NotesVisible => Notes != "";

    public static StatusView From(LauncherStatus status)
    {
        var state = status.State switch
        {
            GameState.UpToDate => "Le jeu est à jour.",
            GameState.Outdated => $"Une mise à jour du jeu est disponible ({status.Latest}).",
            GameState.NotInstalled => $"Le jeu n'est pas installé ; la version {status.Latest} est disponible.",
            _ => $"Serveur injoignable : {status.Message}",
        };
        var launcher = LauncherLine(status.LauncherCurrent);
        if (status.LauncherUpdateAvailable)
        {
            launcher += $" (version {status.LauncherAvailable} disponible)";
        }
        return new StatusView(InstalledLine(status.Installed), LatestLine(status.Latest), state, status.Notes, launcher,
            PlayEnabled: false,
            UpdateGameEnabled: status.State is GameState.Outdated or GameState.NotInstalled,
            UpdateLauncherEnabled: status.LauncherUpdateAvailable,
            RefreshEnabled: true);
    }

    /** While reading: everything disabled, "Refresh" included. */
    public static StatusView Loading(string launcherCurrent = LauncherInfo.Version)
        => new("Version installée : …", "Dernière version : …", "Lecture en cours…", "", LauncherLine(launcherCurrent),
            false, false, false, false);

    /** servers.json unreadable (InvalidDataException): shown as the state, only "Refresh" enabled. */
    public static StatusView Error(string message, string? installed, string launcherCurrent = LauncherInfo.Version)
        => new(InstalledLine(installed), LatestLine(null), $"Erreur : {message}", "", LauncherLine(launcherCurrent),
            false, false, false, true);

    private static string InstalledLine(string? installed) => $"Version installée : {installed ?? "pas installé"}";

    private static string LatestLine(string? latest) => $"Dernière version : {latest ?? "inconnue"}";

    private static string LauncherLine(string current) => $"Lanceur : version {current}";
}
