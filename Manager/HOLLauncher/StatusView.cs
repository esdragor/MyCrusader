namespace HOLLauncher;

/**
 * What the window shows, computed from a LauncherStatus (pure, tested): the French texts and which buttons are
 * enabled. "Update the game" runs GameUpdater (L6c); "Update the launcher" runs LauncherUpdater (L7); "Play" follows
 * PlayRules (L8): PlayConfirmation is asked before starting the game (null: none). UpdateGameText names that button:
 * "Installer le jeu" when nothing is installed (L9).
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
    bool RefreshEnabled,
    string? PlayConfirmation = null,
    string UpdateGameText = StatusView.UpdateGameLabel)
{
    public const string UpdateGameLabel = "Mettre à jour le jeu";

    /** The same button when nothing is installed (L9: a friend's kit folder): the update downloads everything. */
    public const string InstallGameLabel = "Installer le jeu";

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
        var play = PlayRules.Decide(status.State, status.Installed);
        if (play.StateWarning != "")
        {
            state += " " + play.StateWarning;
        }
        var launcher = LauncherLine(status.LauncherCurrent);
        if (status.LauncherUpdateAvailable)
        {
            launcher += $" (version {status.LauncherAvailable} disponible)";
        }
        return new StatusView(InstalledLine(status.Installed), LatestLine(status.Latest), state, status.Notes, launcher,
            PlayEnabled: play.Enabled,
            UpdateGameEnabled: status.State is GameState.Outdated or GameState.NotInstalled,
            UpdateLauncherEnabled: status.LauncherUpdateAvailable,
            RefreshEnabled: true,
            PlayConfirmation: play.Confirmation,
            UpdateGameText: status.Installed is null ? InstallGameLabel : UpdateGameLabel);
    }

    /** While reading: everything disabled, "Refresh" included. */
    public static StatusView Loading(string launcherCurrent = LauncherInfo.Version)
        => new("Version installée : …", "Dernière version : …", "Lecture en cours…", "", LauncherLine(launcherCurrent),
            false, false, false, false);

    /** During a game or launcher update: the texts stay, every button is disabled (the update has its own "Cancel"). */
    public static StatusView Updating(StatusView current)
        => current with { PlayEnabled = false, UpdateGameEnabled = false, UpdateLauncherEnabled = false, RefreshEnabled = false };

    /** servers.json unreadable (InvalidDataException): shown as the state, only "Refresh" enabled. */
    public static StatusView Error(string message, string? installed, string launcherCurrent = LauncherInfo.Version)
        => new(InstalledLine(installed), LatestLine(null), $"Erreur : {message}", "", LauncherLine(launcherCurrent),
            false, false, false, true);

    private static string InstalledLine(string? installed) => $"Version installée : {installed ?? "pas installé"}";

    private static string LatestLine(string? latest) => $"Dernière version : {latest ?? "inconnue"}";

    private static string LauncherLine(string current) => $"Lanceur : version {current}";
}
