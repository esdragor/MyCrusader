namespace HOLLauncher;

/** The "autoUpdate" setting (L23): no background check, a notification, or the game update started by itself. */
public enum AutoUpdateMode
{
    Off,
    Notify,
    Install,
}

/** What the background check does for the game. */
public enum BackgroundAction
{
    None,
    /** A notification says that a version is available (click: the window). */
    Notify,
    /** The game update starts in the background (it keeps "Pause" / "Cancel" in the window). */
    Install,
}

/**
 * What the background checks already did during this run of the launcher, so that one version is notified once and
 * installed by itself once (a failed, paused or cancelled install is not started again by the timer: the failure was
 * notified, the player decides). Kept in memory only: a new start of the launcher (a logon) may remind it once more.
 */
public sealed record BackgroundMemory(string? NotifiedGame = null, string? InstallTried = null, string? NotifiedLauncher = null)
{
    public static readonly BackgroundMemory Empty = new();
}

/**
 * The background check's decision: the game's action and its version (Version, the latest one), the launcher version to
 * notify (LauncherVersion, null: none), and the memory to keep for the next check.
 */
public sealed record BackgroundDecision(BackgroundAction Game, string? Version, string? LauncherVersion, BackgroundMemory Memory)
{
    public bool NothingToSay => Game == BackgroundAction.None && LauncherVersion is null;
}

/**
 * The background update check (L23, pure, tested): every CheckEveryMinutes (CheckSchedule) while the launcher runs,
 * window hidden or not, /versions/latest is read and this decides. The table, first rule that applies:
 * - the game is running (started from the launcher or not): nothing at all, not even a notification (it would pop up
 *   over the game); the next check after the game says it.
 * - autoUpdate "off": nothing (a check the player asks for, manual, still notifies, never installs).
 * - no update (up to date, server unreachable, no status): nothing for the game.
 * - "install", an installed game is outdated, no run (update, repair, pause...) under way, this version not tried yet:
 *   Install. A game not installed at all is only notified (a whole download is the player's choice).
 * - a run is under way: nothing (it may be this very update).
 * - the window is shown (visible, not minimized): nothing, the window itself shows the new state; the notification
 *   comes once it is hidden.
 * - this version not notified yet (or manual): Notify.
 * The launcher's own update is only ever notified (same once-only and window-shown rules, never while playing): it is
 * never installed nor restarted silently (the player may be in a menu of it).
 */
public static class BackgroundUpdatePolicy
{
    public const string Off = "off";
    public const string Notify = "notify";
    public const string Install = "install";

    public const int DefaultMinutes = 30;
    public const int MinMinutes = 5;
    public const int MaxMinutes = 1440;

    /** The setting's mode; null or an unknown value (a hand-edited file): Notify, the default. */
    public static AutoUpdateMode ModeOf(string? setting) => setting?.Trim().ToLowerInvariant() switch
    {
        Off => AutoUpdateMode.Off,
        Install => AutoUpdateMode.Install,
        _ => AutoUpdateMode.Notify,
    };

    /** The value stored for a mode: the default (Notify) is not written. */
    public static string? SettingFor(AutoUpdateMode mode) => mode switch
    {
        AutoUpdateMode.Off => Off,
        AutoUpdateMode.Install => Install,
        _ => null,
    };

    /** The time between two checks: the setting within 5..1440 minutes, 30 when none. */
    public static TimeSpan Interval(int? minutes) => TimeSpan.FromMinutes(Math.Clamp(minutes ?? DefaultMinutes, MinMinutes, MaxMinutes));

    /**
     * status: the reading (null: none could be made, servers.json unreadable). gameRunning: any game client process;
     * running: a run of the window (with or without a bar, paused included); windowShown: visible and not minimized;
     * manual: the player asked ("Check for updates" of the tray menu), the memory does not hold the notification back.
     */
    public static BackgroundDecision Decide(LauncherSettings settings, LauncherStatus? status, bool gameRunning, bool running,
        bool windowShown, BackgroundMemory memory, bool manual = false)
    {
        var mode = ModeOf(settings.AutoUpdate);
        var none = new BackgroundDecision(BackgroundAction.None, null, null, memory);
        if (gameRunning || status is null || (mode == AutoUpdateMode.Off && !manual))
        {
            return none;
        }
        var seen = manual ? BackgroundMemory.Empty : memory;
        var game = BackgroundAction.None;
        string? version = null;
        if (status.State is GameState.Outdated or GameState.NotInstalled && status.Latest is { } latest)
        {
            if (mode == AutoUpdateMode.Install && status.State == GameState.Outdated && !running && seen.InstallTried != latest)
            {
                game = BackgroundAction.Install;
                memory = memory with { InstallTried = latest, NotifiedGame = latest };
            }
            else if (!running && (manual || !windowShown) && seen.NotifiedGame != latest)
            {
                game = BackgroundAction.Notify;
                memory = memory with { NotifiedGame = latest };
            }
            version = game == BackgroundAction.None ? null : latest;
        }
        string? launcher = null;
        if (status.LauncherUpdateAvailable && (manual || !windowShown) && seen.NotifiedLauncher != status.LauncherAvailable)
        {
            launcher = status.LauncherAvailable;
            memory = memory with { NotifiedLauncher = launcher };
        }
        return new BackgroundDecision(game, version, launcher, memory);
    }
}

/**
 * When the next background check is due (L23), against an injected clock (tests). Every reading of the status counts
 * (the window's own at start, "Refresh", after the settings): the next check comes a full interval later. A clock that
 * went back (time zone or manual change; the clock is UTC, so rarely) makes the check due at once rather than never.
 * Wall-clock time, not a countdown: after a sleep or hibernation longer than the interval, the check is due at once.
 */
public sealed class CheckSchedule(Func<DateTime> utcNow)
{
    private DateTime? last;

    public DateTime? Last => last;

    public void Checked() => last = utcNow();

    public bool IsDue(int? everyMinutes)
    {
        if (last is not { } previous)
        {
            return true;
        }
        var now = utcNow();
        return now < previous || now - previous >= BackgroundUpdatePolicy.Interval(everyMinutes);
    }
}
