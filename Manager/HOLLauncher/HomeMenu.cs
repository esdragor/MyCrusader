namespace HOLLauncher;

/** The commands of the ⚙ part of the big button (and of the top bar's icons for three of them). */
public enum MenuCommand
{
    PlayAnyway,
    Settings,
    Repair,
    OpenGameFolder,
    OpenLogs,
    SendReport,
    Uninstall,
    UpdateLauncher,
    Refresh,
}

/** One menu line; SeparatorBefore draws a line above it (groups: game, files, launcher). */
public sealed record MenuEntry(MenuCommand Command, string Text, bool Enabled, bool SeparatorBefore = false);

/**
 * The ⚙ menu for the window's state (pure, tested). What changes the game's files (repair, uninstall, the launcher's
 * own update, playing an outdated game) only while nothing runs and the game is closed; settings, report and refresh
 * while nothing runs (the game may be open: they do not touch its files); the two folders always (harmless).
 * "Update the launcher" is listed only when a newer one is published, "Play anyway" only when the big button says
 * "Update" for a game that can still be played (PlayRules: asked first). running: a run with or without a bar.
 */
public static class HomeMenu
{
    public static IReadOnlyList<MenuEntry> For(StatusView view, bool running, bool gameRunning, string? language = null)
    {
        var lang = Texts.LanguageOr(language);
        var idle = !running && view.RefreshEnabled;
        var filesFree = idle && !gameRunning;
        var entries = new List<MenuEntry>();
        if (view.PlayEnabled && view.UpdateGameEnabled && view.Installed is not null && !gameRunning)
        {
            entries.Add(new MenuEntry(MenuCommand.PlayAnyway, Texts.Get("PlayAnyway", lang), filesFree));
        }
        entries.Add(new MenuEntry(MenuCommand.Settings, Texts.Get("Settings", lang), idle, entries.Count > 0));
        entries.Add(new MenuEntry(MenuCommand.Repair, Texts.Get("FilesVerify", lang), filesFree, true));
        entries.Add(new MenuEntry(MenuCommand.OpenGameFolder, Texts.Get("FilesOpenGame", lang), true));
        entries.Add(new MenuEntry(MenuCommand.OpenLogs, Texts.Get("FilesOpenLogs", lang), true));
        entries.Add(new MenuEntry(MenuCommand.SendReport, Texts.Get("ReportSend", lang), !running));
        entries.Add(new MenuEntry(MenuCommand.Uninstall, Texts.Get("FilesUninstall", lang), filesFree));
        if (view.UpdateLauncherEnabled)
        {
            entries.Add(new MenuEntry(MenuCommand.UpdateLauncher, Texts.Get("UpdateLauncher", lang), filesFree, true));
        }
        entries.Add(new MenuEntry(MenuCommand.Refresh, Texts.Get("Refresh", lang), idle, !view.UpdateLauncherEnabled));
        return entries;
    }

    /** Whether a command can run now (the top bar's icons and the menu agree). */
    public static bool Enabled(MenuCommand command, StatusView view, bool running, bool gameRunning)
        => For(view, running, gameRunning, "en").FirstOrDefault(e => e.Command == command)?.Enabled ?? false;
}
