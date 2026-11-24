using System.ComponentModel;
using System.Diagnostics;

namespace HOLLauncher;

/**
 * "Send a report" (L20), an item of the "Files" menu (MainForm.Files.cs, enabled only while nothing runs): the player
 * picks where to save the zip (the Desktop by default), ReportBuilder writes it in the background, then the Explorer
 * opens with the file selected; the player gives it to the developer (nothing is uploaded).
 * At start (Shown), CrashDetector looks for a crash of the game since the last start and offers the same report; the
 * time of this look is stored in the settings (LastCrashCheckUtc) so that a crash is offered once.
 */
public sealed partial class MainForm
{
    private readonly ToolStripMenuItem reportItem = new();
    private bool reporting;

    /** After "Open the logs folder": the report is made of those logs. */
    private void AddReportItem()
    {
        filesMenu.Items.Insert(filesMenu.Items.IndexOf(openLogsItem) + 1, reportItem);
        reportItem.Click += async (_, _) => await SendReportAsync();
    }

    private void ApplyReportTexts() => reportItem.Text = Texts.Get("ReportSend");

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
        if (settingsProblem is null)
        {
            settings = settings with { LastCrashCheckUtc = now };
            try
            {
                settings.Save(launcherDir);
            }
            catch (Exception e) when (e is IOException or UnauthorizedAccessException)
            {
                // A read-only launcher folder: the same crash may be offered again, no more.
            }
        }
        if (check.Crashed && !IsDisposed
            && MessageBox.Show(this, Texts.Get("ReportCrashDetected"), "Heroes of Liberty", MessageBoxButtons.YesNo,
                MessageBoxIcon.Question) == DialogResult.Yes)
        {
            await SendReportAsync();
        }
    }

    private async Task SendReportAsync()
    {
        if (updating is not null || reporting)
        {
            return;
        }
        string file;
        using (var dialog = new SaveFileDialog
        {
            Title = Texts.Get("ReportSaveTitle"),
            Filter = Texts.Get("ReportFilter"),
            DefaultExt = "zip",
            AddExtension = true,
            OverwritePrompt = true,
            InitialDirectory = Environment.GetFolderPath(Environment.SpecialFolder.DesktopDirectory),
            FileName = ReportBuilder.FileNameFor(DateTime.Now),
        })
        {
            if (dialog.ShowDialog(this) != DialogResult.OK)
            {
                return;
            }
            file = dialog.FileName;
        }
        var input = new ReportInput(Options.GameDir, launcherDir, settings, DateTime.UtcNow,
            [settingsProblem ?? "", current.StateText, current.LauncherText],
            Environment.GetFolderPath(Environment.SpecialFolder.LocalApplicationData));
        reporting = true;
        UseWaitCursor = true;
        try
        {
            await Task.Run(() => ReportBuilder.Build(file, input));
        }
        catch (Exception e) when (e is IOException or UnauthorizedAccessException or NotSupportedException)
        {
            Warn(Texts.Format("ReportFailed", e.Message));
            return;
        }
        finally
        {
            reporting = false;
            UseWaitCursor = false;
        }
        try
        {
            Process.Start(new ProcessStartInfo("explorer.exe", $"/select,\"{file}\"") { UseShellExecute = true })?.Dispose();
        }
        catch (Exception e) when (e is Win32Exception or IOException or InvalidOperationException)
        {
            // The message below still names the file.
        }
        MessageBox.Show(this, Texts.Format("ReportCreated", file), "Heroes of Liberty", MessageBoxButtons.OK,
            MessageBoxIcon.Information);
    }
}
