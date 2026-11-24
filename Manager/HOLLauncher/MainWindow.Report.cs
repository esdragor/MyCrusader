using System.ComponentModel;
using System.Diagnostics;
using System.Windows.Input;

namespace HOLLauncher;

/**
 * "Send a report" (L20) and the two folders (L17), from the ⚙ menu (the report also from the title bar's "?"): the
 * player picks where to save the zip (the Desktop by default), ReportBuilder writes it in the background, then the
 * Explorer opens with the file selected; the player gives it to the developer (nothing is uploaded).
 * At start, CrashDetector looks for a crash of the game since the last look and offers the same report; the time of
 * this look is stored in the settings (LastCrashCheckUtc) so that a crash is offered once. Launcher 5: the same look
 * runs when a game started from the launcher exits (MainWindow.Runs.cs).
 */
public sealed partial class MainWindow
{
    /**
     * The look for a crash, once at start. The time is stored even when nothing is found or the player says No; not
     * when the settings file was unreadable (it is left as it is for the player to see, as before L20): the look then
     * starts again from "first start" at the next start.
     */
    private async Task OfferReportAfterCrashAsync()
    {
        var gameDir = Options.GameDir;
        var since = settings.LastCrashCheckUtc;
        var now = DateTime.UtcNow;
        var check = await Task.Run(() => CrashDetector.Check(gameDir, since, CrashDetector.GameIsRunning()));
        StoreCrashCheck(now);
        if (check.Crashed && !closed && Ask(Texts.Get("ReportCrashDetected")))
        {
            await SendReportAsync();
        }
    }

    /** Stores the time of a crash look (not over an unreadable settings file, see above). */
    private void StoreCrashCheck(DateTime nowUtc)
    {
        if (settingsProblem is not null)
        {
            return;
        }
        settings = settings with { LastCrashCheckUtc = nowUtc };
        try
        {
            settings.Save(launcherDir);
        }
        catch (Exception e) when (e is IOException or UnauthorizedAccessException)
        {
            // A read-only launcher folder: the same crash may be offered again, no more.
        }
    }

    private async Task SendReportAsync()
    {
        if (Running)
        {
            return;
        }
        var dialog = new Microsoft.Win32.SaveFileDialog
        {
            Title = Texts.Get("ReportSaveTitle"),
            Filter = Texts.Get("ReportFilter"),
            DefaultExt = "zip",
            AddExtension = true,
            OverwritePrompt = true,
            InitialDirectory = Environment.GetFolderPath(Environment.SpecialFolder.DesktopDirectory),
            FileName = ReportBuilder.FileNameFor(DateTime.Now),
        };
        if (dialog.ShowDialog(this) != true)
        {
            return;
        }
        var file = dialog.FileName;
        var input = new ReportInput(Options.GameDir, launcherDir, settings, DateTime.UtcNow,
            [settingsProblem ?? "", current.StateText, current.LauncherText],
            Environment.GetFolderPath(Environment.SpecialFolder.LocalApplicationData));
        busy = true;
        Render();
        Mouse.OverrideCursor = Cursors.Wait;
        try
        {
            await Task.Run(() => ReportBuilder.Build(file, input));
        }
        catch (Exception e) when (e is IOException or UnauthorizedAccessException or NotSupportedException)
        {
            Mouse.OverrideCursor = null;
            Warn(Texts.Format("ReportFailed", e.Message));
            return;
        }
        finally
        {
            busy = false;
            Mouse.OverrideCursor = null;
            Render();
        }
        try
        {
            Process.Start(new ProcessStartInfo("explorer.exe", $"/select,\"{file}\"") { UseShellExecute = true })?.Dispose();
        }
        catch (Exception e) when (e is Win32Exception or IOException or InvalidOperationException)
        {
            // The message below still names the file.
        }
        Inform(Texts.Format("ReportCreated", file));
    }

    private void OpenGameFolder()
    {
        if (!Directory.Exists(Options.GameDir))
        {
            Warn(Texts.Format("GameDirMissing", Options.GameDir));
            return;
        }
        OpenFolder(Options.GameDir);
    }

    private void OpenLogsFolder()
    {
        var logs = GameLogs.FindDir(Options.GameDir);
        if (logs is null)
        {
            Warn(Texts.Format("LogsMissing", GameLogs.ExpectedDir(Options.GameDir)));
            return;
        }
        OpenFolder(logs);
    }

    /** In the Explorer; a refusal is said. */
    private void OpenFolder(string folder)
    {
        try
        {
            Process.Start(new ProcessStartInfo(folder) { UseShellExecute = true })?.Dispose();
        }
        catch (Exception e) when (e is Win32Exception or IOException or InvalidOperationException)
        {
            Warn(Texts.Format("FolderOpenFailed", folder, e.Message));
        }
    }
}
