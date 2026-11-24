using System.Diagnostics;
using System.Windows;
using HOLUpdate;

namespace HOLLauncher;

/**
 * What the home window runs (as the L6c-L18 window did): the game update and the repair (GameRepairer) with the bar
 * under the big button, which then reads "Pause" (L18: the run is cancelled but the window stays paused, "Resume" runs
 * it again and fetches nothing already received) beside "Cancel"; the launcher's own update (L7: then the new exe is
 * started with the same arguments and this one closes); the uninstall (confirmation, No by default). Closing while
 * paused keeps the files: "Update" resumes at the next start.
 * "Play" (L8, launcher 5): PlayConfirmation first (outdated game), then the game with the settings' launch options
 * (LaunchArguments, L19); then either the launcher closes (closeOnPlay) or it minimizes and watches the game
 * (GameSession): "Running" until it exits, then the window comes back, the crash check of that run (CrashDetector, from
 * its start: the report is offered at once) and a new reading.
 */
public sealed partial class MainWindow
{
    private CancellationTokenSource? updating;
    private ProgressView? progressView;
    private bool busy;
    private bool gameRunning;
    private bool autoUpdatePending;
    private bool autoLauncherUpdatePending;
    // Pause (L18): asked during the run; once paused, the run to start again on "Resume" and its cancel message.
    private bool pauseAsked;
    private ProgressView? pausedLine;
    private Func<Task>? resume;
    private string? pausedMessage;

    private void ShowProgress(ProgressView view)
    {
        progressView = view;
        Render();
    }

    private void HideProgress()
    {
        progressView = null;
        Render();
    }

    /** A run with the bar: the status buttons off, the bar on, the work in the background, the pause handled at its end. */
    private async Task<T?> RunWithBarAsync<T>(ProgressView first, Func<CancellationToken, Task<T>> work,
        Func<T, (bool Cancelled, string Message)> outcome, Func<Task> again) where T : class
    {
        if (updating is not null)
        {
            return null;
        }
        using var source = new CancellationTokenSource();
        updating = source;
        pauseAsked = false;
        Display(StatusView.Updating(current));
        ShowProgress(first);
        T result;
        try
        {
            result = await Task.Run(() => work(source.Token));
        }
        finally
        {
            updating = null;
        }
        var (cancelled, message) = outcome(result);
        if (closed || EnterPauseIfAsked(cancelled, message, again))
        {
            return null;
        }
        HideProgress();
        return result;
    }

    /** Reads servers.json and /versions/latest again (the status may be old), then runs the update in the background. */
    private async Task UpdateGameAsync()
    {
        if (gameRunning)
        {
            return;
        }
        var progress = NewProgress<DownloadProgress>(ProgressView.From);
        var result = await RunWithBarAsync(ProgressView.Starting(), token => RunUpdateAsync(progress, token),
            r => (r.Cancelled, r.Message), UpdateGameAsync);
        if (result is null)
        {
            return;
        }
        if (result.Success)
        {
            Inform(result.Message);
        }
        else
        {
            Warn(result.Message);
        }
        await RefreshStatusAsync();
    }

    private async Task<UpdateResult> RunUpdateAsync(IProgress<DownloadProgress> progress, CancellationToken token)
    {
        ManagerServer manager;
        try
        {
            manager = GameServers.ReadFirst(Options.GameDir);
        }
        catch (Exception e) when (e is InvalidDataException or IOException or UnauthorizedAccessException)
        {
            return new UpdateResult(false, Texts.Format("Error", e.Message));
        }
        LatestResult latest;
        try
        {
            latest = await new VersionsClient(http).GetLatestAsync(manager.Url, token);
        }
        catch (OperationCanceledException)
        {
            return new UpdateResult(false, Texts.Get("UpdateCancelled"), Cancelled: true);
        }
        if (latest.Latest is null)
        {
            return new UpdateResult(false, latest.Outcome == LatestOutcome.BadKey
                ? Texts.Get("BadKey")
                : Texts.Format("ServerUnreachable", latest.Message));
        }
        return await new GameUpdater(downloads).UpdateAsync(Options.GameDir, manager.Url, latest.Latest, progress, token);
    }

    /**
     * "Verify / repair" (L17), like the update: the status may be old, so servers.json and /versions/latest are read
     * again first. A resumed check hashes the files again; the files already fetched are not fetched again.
     */
    private async Task RepairGameAsync()
    {
        if (gameRunning)
        {
            return;
        }
        var progress = NewProgress<DownloadProgress>(ProgressView.From);
        var result = await RunWithBarAsync(new ProgressView(Texts.Get("RepairChecking"), 0, true),
            token => RunRepairAsync(progress, token), r => (r.Cancelled, r.Message), RepairGameAsync);
        if (result is null)
        {
            return;
        }
        if (result.Success)
        {
            Inform(result.Message);
        }
        else
        {
            Warn(result.Message);
        }
        await RefreshStatusAsync();
    }

    private async Task<UpdateResult> RunRepairAsync(IProgress<DownloadProgress> progress, CancellationToken token)
    {
        var (url, latest, problem) = await ReadLatestAsync(token);
        if (latest is null)
        {
            return new UpdateResult(false, problem, Cancelled: token.IsCancellationRequested);
        }
        return await new GameRepairer(downloads).RepairAsync(Options.GameDir, url, latest, progress, token);
    }

    /** The first manager of servers.json and its latest version; latest null with the message for the player otherwise. */
    private async Task<(string Url, LatestVersion? Latest, string Problem)> ReadLatestAsync(CancellationToken token)
    {
        ManagerServer manager;
        try
        {
            manager = GameServers.ReadFirst(Options.GameDir);
        }
        catch (Exception e) when (e is InvalidDataException or IOException or UnauthorizedAccessException)
        {
            return ("", null, Texts.Format("FilesServersError", e.Message));
        }
        var latest = await new VersionsClient(http).GetLatestAsync(manager.Url, token);
        if (token.IsCancellationRequested)
        {
            return (manager.Url, null, Texts.Get("RepairCancelled"));
        }
        return latest.Latest is null
            ? (manager.Url, null, Texts.Format("RepairServerUnreachable", latest.Message))
            : (manager.Url, latest.Latest, "");
    }

    /**
     * Downloads the new launcher in the background (bar, "Pause", "Cancel"), swaps it in, then starts it with the same
     * arguments and closes; a failure is shown and the status read again.
     */
    private async Task UpdateLauncherAsync()
    {
        if (gameRunning || updating is not null)
        {
            return;
        }
        var exePath = Environment.ProcessPath;
        if (exePath is null)
        {
            Warn(Texts.Get("LauncherPathUnknown"));
            return;
        }
        var progress = NewProgress<LauncherProgress>(ProgressView.FromLauncher);
        var result = await RunWithBarAsync(ProgressView.Starting(), token => RunLauncherUpdateAsync(exePath, progress, token),
            r => (r.Cancelled, r.Message), UpdateLauncherAsync);
        if (result is null)
        {
            return;
        }
        if (result.Success)
        {
            try
            {
                var start = new ProcessStartInfo(exePath) { UseShellExecute = false };
                foreach (var argument in LauncherUpdater.RestartArguments(Environment.GetCommandLineArgs().Skip(1)))
                {
                    start.ArgumentList.Add(argument);
                }
                Process.Start(start)?.Dispose();
                Close();
                return;
            }
            catch (Exception e) when (e is System.ComponentModel.Win32Exception or IOException)
            {
                result = new LauncherUpdateResult(false, Texts.Format("LauncherRestartByHand", result.Message, e.Message));
            }
        }
        Warn(result.Message);
        await RefreshStatusAsync();
    }

    private async Task<LauncherUpdateResult> RunLauncherUpdateAsync(string exePath, IProgress<LauncherProgress> progress,
        CancellationToken token)
    {
        ManagerServer manager;
        try
        {
            manager = GameServers.ReadFirst(Options.GameDir);
        }
        catch (Exception e) when (e is InvalidDataException or IOException or UnauthorizedAccessException)
        {
            return new LauncherUpdateResult(false, Texts.Format("Error", e.Message));
        }
        return await new LauncherUpdater(downloads, manager.Url, exePath).UpdateAsync(progress, token);
    }

    /** The run's progress, shown unless a stop is under way (its line stays "Cancelling" / "Pausing"). */
    private Progress<T> NewProgress<T>(Func<T, string?, ProgressView> view) => new(p =>
    {
        if (updating is { IsCancellationRequested: false } && !closed)
        {
            ShowProgress(view(p, null));
        }
    });

    private async Task CancelUpdateAsync()
    {
        if (resume is not null)
        {
            await EndPauseAsync();
            return;
        }
        if (updating is null || progressView is null)
        {
            return;
        }
        updating.Cancel();
        ShowProgress(progressView.Cancelling());
    }

    /** "Pause" stops the run (its token, like Cancel) and asks for the paused state at its end; "Resume" runs it again. */
    private async Task PauseOrResumeAsync()
    {
        if (resume is { } again)
        {
            resume = null;
            pausedMessage = null;
            progressView = null;
            await again();
            return;
        }
        if (updating is null || progressView is null)
        {
            return;
        }
        pauseAsked = true;
        pausedLine = progressView;
        updating.Cancel();
        ShowProgress(progressView.Pausing());
    }

    /**
     * At the end of a run: if "Pause" stopped it, the window stays on the paused line with "Resume" and "Cancel"
     * (true). A run that ended otherwise (done, or failed before the click took) is shown as usual (false).
     */
    private bool EnterPauseIfAsked(bool cancelled, string message, Func<Task> again)
    {
        var asked = pauseAsked;
        pauseAsked = false;
        if (!asked || !cancelled)
        {
            return false;
        }
        resume = again;
        pausedMessage = message;
        ShowProgress((pausedLine ?? progressView ?? ProgressView.Starting()).Paused());
        return true;
    }

    /** "Cancel" while paused: the run's cancel message, as after a cancel; what was received stays for the next run. */
    private async Task EndPauseAsync()
    {
        var message = pausedMessage ?? "";
        resume = null;
        pausedMessage = null;
        HideProgress();
        Warn(message);
        await RefreshStatusAsync();
    }

    /**
     * Asks first (No by default). Without installed.json the manager's latest list is fetched to know the game's
     * files (GameUninstaller); unreachable: GameUninstaller says nothing was deleted.
     */
    private async Task UninstallGameAsync()
    {
        if (Running || gameRunning || !current.RefreshEnabled)
        {
            return;
        }
        if (!Ask(Texts.Format("UninstallConfirm", Options.GameDir), defaultNo: true))
        {
            return;
        }
        busy = true;
        Display(StatusView.Updating(current));
        var gameDir = Options.GameDir;
        UninstallResult result;
        try
        {
            result = await Task.Run(async () =>
            {
                VersionManifest? serverList = null;
                if (GameUninstaller.NeedsServerList(gameDir))
                {
                    serverList = (await ReadLatestAsync(CancellationToken.None)).Latest?.Manifest;
                }
                return GameUninstaller.Uninstall(gameDir, serverList);
            });
        }
        finally
        {
            busy = false;
        }
        if (closed)
        {
            return;
        }
        if (result.Success)
        {
            Inform(result.Message);
        }
        else
        {
            Warn(result.Message);
        }
        await RefreshStatusAsync();
    }

    /** Asks the confirmation if any, starts the game, then closes or watches it; a failure is shown, the launcher stays. */
    private void Play()
    {
        if (Running || gameRunning || !current.PlayEnabled)
        {
            return;
        }
        if (current.PlayConfirmation is { } question && !Ask(question))
        {
            return;
        }
        var startedUtc = DateTime.UtcNow;
        var result = GameLauncher.Start(Options.GameDir, LaunchArguments.For(settings));
        if (!result.Success)
        {
            Warn(result.Message);
            return;
        }
        if (GameSession.Decide(settings.CloseOnPlay) == AfterPlay.Close)
        {
            result.Game?.Dispose();
            Close();
            return;
        }
        if (result.Game is { } game)
        {
            _ = WatchGameAsync(game, startedUtc);
        }
    }

    /** "Running" until the game exits; then back to the window, the crash check of that run, a new reading. */
    private async Task WatchGameAsync(Process game, DateTime startedUtc)
    {
        gameRunning = true;
        var before = WindowState;
        Render();
        WindowState = WindowState.Minimized;
        try
        {
            await GameSession.WaitForExitAsync(game, GameSession.IsGameRunning, TimeSpan.FromSeconds(2), closing.Token);
        }
        catch (OperationCanceledException)
        {
            return; // the launcher closes
        }
        finally
        {
            game.Dispose();
        }
        gameRunning = false;
        if (closed)
        {
            return;
        }
        if (WindowState == WindowState.Minimized)
        {
            WindowState = before == WindowState.Maximized ? WindowState.Maximized : WindowState.Normal;
        }
        Activate();
        Render();
        var gameDir = Options.GameDir;
        var check = await Task.Run(() => CrashDetector.Check(gameDir, startedUtc, gameRunning: false));
        StoreCrashCheck(DateTime.UtcNow);
        if (check.Crashed && !closed && Ask(Texts.Get("ReportCrashDetected")))
        {
            await SendReportAsync();
        }
        if (!closed)
        {
            await RefreshStatusAsync();
        }
    }
}
