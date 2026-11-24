using HOLUpdate;

namespace HOLLauncher;

/**
 * What the window shows, computed from a LauncherStatus (pure, tested): the texts and which buttons are enabled.
 * "Update the game" runs GameUpdater (L6c); "Update the launcher" runs LauncherUpdater (L7); "Play" follows
 * PlayRules (L8): PlayConfirmation is asked before starting the game (null: none). UpdateGameText names that button:
 * "Install the game" when nothing is installed (L9). L16: every text comes from Texts in the given language (null: the
 * one in use), so a language change rebuilds the view (MainForm reads the status again).
 * Launcher 5 (the home window): Installed and Latest are the raw version names (null: none / unknown) for the footer
 * line and the release-note cards; Server says whether the last reading reached the manager (the footer's dot);
 * IsLoading is the reading itself (the primary button waits, PrimaryAction).
 */
/** Whether the last status reading reached the manager: Unknown while reading (and before the first one). */
public enum ServerLink
{
    Unknown,
    Reached,
    Unreachable,
}

public sealed record StatusView(
    string InstalledText,
    string LatestText,
    string StateText,
    IReadOnlyList<NewsBlock> News,
    string LauncherText,
    bool PlayEnabled,
    bool UpdateGameEnabled,
    bool UpdateLauncherEnabled,
    bool RefreshEnabled,
    string? PlayConfirmation = null,
    string UpdateGameText = "",
    string? Installed = null,
    string? Latest = null,
    ServerLink Server = ServerLink.Unknown,
    bool IsLoading = false)
{
    /** L22: the news as blocks for the box (NewsFormat); Notes is their plain text. */
    public string Notes => ReleaseNotes.PlainText(News);

    public bool NotesVisible => News.Count > 0;

    public static StatusView From(LauncherStatus status, string? language = null)
    {
        var lang = Texts.LanguageOr(language);
        var state = status.State switch
        {
            GameState.UpToDate => Texts.Get("GameUpToDate", lang),
            GameState.Outdated => Texts.FormatIn(lang, "GameOutdated", VersionName.Display(status.Latest!)),
            GameState.NotInstalled => Texts.FormatIn(lang, "GameNotInstalled", VersionName.Display(status.Latest!)),
            // L16: the refused key is a signal of its own (LatestOutcome.BadKey), never a compared text.
            _ when status.KeyRefused => Texts.Get("BadKey", lang),
            _ => Texts.FormatIn(lang, "ServerUnreachable", status.Message),
        };
        var play = PlayRules.Decide(status.State, status.Installed, language: lang);
        if (play.StateWarning != "")
        {
            state += " " + play.StateWarning;
        }
        var launcher = status.LauncherUpdateAvailable
            ? Texts.FormatIn(lang, "LauncherLineUpdate", status.LauncherCurrent, status.LauncherAvailable)
            : LauncherLine(status.LauncherCurrent, lang);
        return new StatusView(InstalledLine(status.Installed, lang), LatestLine(status.Latest, lang), state, status.News, launcher,
            PlayEnabled: play.Enabled,
            UpdateGameEnabled: status.State is GameState.Outdated or GameState.NotInstalled,
            UpdateLauncherEnabled: status.LauncherUpdateAvailable,
            RefreshEnabled: true,
            PlayConfirmation: play.Confirmation,
            UpdateGameText: Texts.Get(status.Installed is null ? "InstallGame" : "UpdateGame", lang),
            Installed: status.Installed,
            Latest: status.Latest,
            Server: status.State == GameState.Unreachable ? ServerLink.Unreachable : ServerLink.Reached);
    }

    /** While reading: everything disabled, "Refresh" included. */
    public static StatusView Loading(string launcherCurrent = LauncherInfo.Version, string? language = null)
    {
        var lang = Texts.LanguageOr(language);
        var pending = Texts.Get("VersionPending", lang);
        return new(Texts.FormatIn(lang, "InstalledVersion", pending), Texts.FormatIn(lang, "LatestVersion", pending),
            Texts.Get("StatusLoading", lang), [], LauncherLine(launcherCurrent, lang), false, false, false, false,
            UpdateGameText: Texts.Get("UpdateGame", lang), IsLoading: true);
    }

    /** During a game or launcher update: the texts stay, every button is disabled (the update has its own "Cancel"). */
    public static StatusView Updating(StatusView current)
        => current with { PlayEnabled = false, UpdateGameEnabled = false, UpdateLauncherEnabled = false, RefreshEnabled = false };

    /** servers.json unreadable (InvalidDataException): shown as the state, only "Refresh" enabled. */
    public static StatusView Error(string message, string? installed, string launcherCurrent = LauncherInfo.Version,
        string? language = null)
    {
        var lang = Texts.LanguageOr(language);
        return new(InstalledLine(installed, lang), LatestLine(null, lang), Texts.FormatIn(lang, "Error", message), [],
            LauncherLine(launcherCurrent, lang), false, false, false, true,
            UpdateGameText: Texts.Get(installed is null ? "InstallGame" : "UpdateGame", lang),
            Installed: installed, Server: ServerLink.Unreachable);
    }

    private static string InstalledLine(string? installed, string lang) => Texts.FormatIn(lang, "InstalledVersion",
        installed is null ? Texts.Get("VersionNotInstalled", lang) : VersionName.Display(installed));

    private static string LatestLine(string? latest, string lang) => Texts.FormatIn(lang, "LatestVersion",
        latest is null ? Texts.Get("VersionUnknown", lang) : VersionName.Display(latest));

    private static string LauncherLine(string current, string lang) => Texts.FormatIn(lang, "LauncherLine", current);
}
