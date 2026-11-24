namespace HOLLauncher;

/** What the big button of the home window does (launcher 5); the window maps each kind to its handler. */
public enum PrimaryKind
{
    /** The status is being read: disabled. */
    Loading,
    /** Nothing installed: runs the game update, which installs. */
    Install,
    /** An update exists: runs it (playing the outdated game stays in the menu, PlayAnyway). */
    Update,
    Play,
    /** An update, repair or launcher update runs: pauses it (disabled while a stop is under way). */
    Pause,
    /** The run is paused: runs it again. */
    Resume,
    /** The game started from the launcher still runs: disabled until it exits. */
    Running,
    /** Something without a progress bar runs (uninstall, a report being written): disabled. */
    Busy,
    /** Nothing can be done (server unreachable without a game, servers.json unreadable): disabled. */
    None,
}

/** The big button: what it does, its label, whether it can be clicked. */
public sealed record PrimaryAction(PrimaryKind Kind, string Label, bool Enabled)
{
    /**
     * The button for the window's state (pure, tested), in priority order: the running game, then a run with a
     * progress bar (paused or not), then a run without one, then the reading, then the status itself: install, update,
     * play. A status where neither is possible keeps the label it would have (install or play), disabled.
     * progress: the run's line (null: no run with a bar); busy: something else runs.
     */
    public static PrimaryAction From(StatusView view, ProgressView? progress, bool gameRunning, bool busy,
        string? language = null)
    {
        var lang = Texts.LanguageOr(language);
        if (gameRunning)
        {
            return new PrimaryAction(PrimaryKind.Running, Texts.Get("PrimaryRunning", lang), false);
        }
        if (progress is not null)
        {
            return progress.IsPaused
                ? new PrimaryAction(PrimaryKind.Resume, Texts.Get("Resume", lang), true)
                : new PrimaryAction(PrimaryKind.Pause, Texts.Get("Pause", lang), progress.PauseEnabled);
        }
        if (busy)
        {
            return new PrimaryAction(PrimaryKind.Busy, Texts.Get("PrimaryBusy", lang), false);
        }
        if (view.IsLoading)
        {
            return new PrimaryAction(PrimaryKind.Loading, Texts.Get("PrimaryLoading", lang), false);
        }
        if (view.UpdateGameEnabled)
        {
            return view.Installed is null
                ? new PrimaryAction(PrimaryKind.Install, Texts.Get("InstallGame", lang), true)
                : new PrimaryAction(PrimaryKind.Update, Texts.Get("PrimaryUpdate", lang), true);
        }
        if (view.PlayEnabled)
        {
            return new PrimaryAction(PrimaryKind.Play, Texts.Get("Play", lang), true);
        }
        return new PrimaryAction(PrimaryKind.None, Texts.Get(view.Installed is null ? "InstallGame" : "Play", lang), false);
    }
}
