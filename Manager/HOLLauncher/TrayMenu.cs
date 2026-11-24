namespace HOLLauncher;

/** The commands of the notification-area icon's menu (L23). */
public enum TrayCommand
{
    Open,
    Play,
    CheckForUpdates,
    Quit,
}

/** One line of the tray menu; Bold: the default command (a double click on the icon does it). */
public sealed record TrayEntry(TrayCommand Command, string Text, bool Enabled, bool SeparatorBefore = false, bool Bold = false);

/**
 * The notification-area icon (L23, pure, tested): its menu for the window's state, its tooltip, and what the window's
 * close button does.
 * Menu: "Open" (always, the default), "Play" only when the game can be played and is not running (enabled while nothing
 * runs; an outdated game asks first, in the window), "Check for updates" (not while the status is being read nor during
 * a run), "Quit" (always: really closes the launcher, a run under way is stopped as when the window closes).
 * Close button: with minimizeToTray the window hides in the notification area instead of quitting, except while the game
 * started from the launcher runs ("Running": closing quits, as in launcher 5) and when the launcher itself quits (tray
 * "Quit", Windows' logoff, the launcher's own update, closeOnPlay).
 */
public static class TrayMenu
{
    /** Windows cuts a notification icon's tooltip at 127 characters (NOTIFYICONDATA.szTip, 128 with the null). */
    public const int MaxTooltip = 127;

    /** The horizontal ellipsis (U+2026) that ends a cut tooltip. */
    public const char Ellipsis = (char)0x2026;

    public static IReadOnlyList<TrayEntry> For(StatusView view, bool running, bool gameRunning, string? language = null)
    {
        var lang = Texts.LanguageOr(language);
        var entries = new List<TrayEntry> { new(TrayCommand.Open, Texts.Get("TrayOpen", lang), true, Bold: true) };
        if (view.PlayEnabled && !gameRunning)
        {
            entries.Add(new TrayEntry(TrayCommand.Play, Texts.Get("Play", lang), !running));
        }
        entries.Add(new TrayEntry(TrayCommand.CheckForUpdates, Texts.Get("TrayCheckForUpdates", lang),
            !running && !view.IsLoading && !gameRunning, SeparatorBefore: true));
        entries.Add(new TrayEntry(TrayCommand.Quit, Texts.Get("TrayQuit", lang), true, SeparatorBefore: true));
        return entries;
    }

    /** Whether a command can run now (the menu and its handler agree). */
    public static bool Enabled(TrayCommand command, StatusView view, bool running, bool gameRunning)
        => For(view, running, gameRunning, "en").FirstOrDefault(e => e.Command == command)?.Enabled ?? false;

    /** The game's name, then what the launcher is doing (a run's line, "Running", or the state), cut to Windows' limit. */
    public static string Tooltip(string gameName, string? line)
    {
        var text = string.IsNullOrWhiteSpace(line) ? gameName : gameName + "\n" + line.Trim();
        return text.Length <= MaxTooltip ? text : text[..(MaxTooltip - 1)] + Ellipsis;
    }

    /**
     * The close button hides the window in the notification area (true) or quits (false). Hiding is the default (the
     * user's choice, 2026-11-24, as in other game launchers): only an explicit false quits.
     */
    public static bool CloseHides(LauncherSettings settings, bool gameRunning, bool quitting)
        => settings.MinimizeToTray != false && !gameRunning && !quitting;
}
