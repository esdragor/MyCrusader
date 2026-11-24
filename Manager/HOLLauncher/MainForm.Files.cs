using System.ComponentModel;
using System.Diagnostics;
using HOLUpdate;

namespace HOLLauncher;

/**
 * The "Files" button of the window (L17), kept apart from MainForm.cs: a menu with "Verify / repair the files"
 * (GameRepairer, run like an update: bar, "Pause" / "Resume" (L18: a resumed check hashes the files again, the files
 * already fetched are not), "Cancel"), "Open the game folder", "Open the logs folder" (GameLogs) and
 * "Uninstall the game" (confirmation, then GameUninstaller). Enabled like "Settings": only while nothing runs.
 */
public sealed partial class MainForm
{
    private readonly Button filesButton = NewButton();
    private readonly ContextMenuStrip filesMenu = new();
    private readonly ToolStripMenuItem verifyItem = new();
    private readonly ToolStripMenuItem openGameItem = new();
    private readonly ToolStripMenuItem openLogsItem = new();
    private readonly ToolStripMenuItem uninstallItem = new();

    private void AddFilesMenu(FlowLayoutPanel buttons)
    {
        filesMenu.Items.AddRange([verifyItem, openGameItem, openLogsItem, new ToolStripSeparator(), uninstallItem]);
        buttons.Controls.Add(filesButton);
        filesButton.Click += (_, _) => filesMenu.Show(filesButton, new Point(0, filesButton.Height));
        verifyItem.Click += async (_, _) => await RepairGameAsync();
        openGameItem.Click += (_, _) => OpenGameFolder();
        openLogsItem.Click += (_, _) => OpenLogsFolder();
        uninstallItem.Click += async (_, _) => await UninstallGameAsync();
        FormClosed += (_, _) => filesMenu.Dispose();
    }

    private void ApplyFilesTexts()
    {
        filesButton.Text = Texts.Get("Files") + " ▾";
        filesMenu.Font = Font;
        verifyItem.Text = Texts.Get("FilesVerify");
        openGameItem.Text = Texts.Get("FilesOpenGame");
        openLogsItem.Text = Texts.Get("FilesOpenLogs");
        uninstallItem.Text = Texts.Get("FilesUninstall");
    }

    /**
     * As UpdateGameAsync: the status may be old, so servers.json and /versions/latest are read again first. Only
     * reached from the menu while nothing runs, or from "Resume" (the window then still shows StatusView.Updating).
     */
    private async Task RepairGameAsync()
    {
        if (updating is not null)
        {
            return;
        }
        using var source = new CancellationTokenSource();
        updating = source;
        pauseAsked = false;
        Display(StatusView.Updating(current));
        ShowProgress(new ProgressView(Texts.Get("RepairChecking"), 0, true));
        progressPanel.Visible = true;
        var progress = new Progress<DownloadProgress>(p =>
        {
            if (!source.IsCancellationRequested && !IsDisposed)
            {
                ShowProgress(ProgressView.From(p));
            }
        });
        UpdateResult result;
        try
        {
            result = await Task.Run(() => RunRepairAsync(progress, source.Token));
        }
        finally
        {
            updating = null;
        }
        if (IsDisposed || EnterPauseIfAsked(result.Cancelled, result.Message, RepairGameAsync))
        {
            return;
        }
        progressPanel.Visible = false;
        MessageBox.Show(this, result.Message, "Heroes of Liberty", MessageBoxButtons.OK,
            result.Success ? MessageBoxIcon.Information : MessageBoxIcon.Warning);
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
        ManagerServer server;
        try
        {
            server = GameServers.ReadFirst(Options.GameDir);
        }
        catch (Exception e) when (e is InvalidDataException or IOException or UnauthorizedAccessException)
        {
            return ("", null, Texts.Format("FilesServersError", e.Message));
        }
        var latest = await new VersionsClient(http).GetLatestAsync(server.Url, token);
        if (token.IsCancellationRequested)
        {
            return (server.Url, null, Texts.Get("RepairCancelled"));
        }
        return latest.Latest is null
            ? (server.Url, null, Texts.Format("RepairServerUnreachable", latest.Message))
            : (server.Url, latest.Latest, "");
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

    /**
     * Asks first (No by default). Without installed.json the manager's latest list is fetched to know the game's
     * files (GameUninstaller); unreachable: GameUninstaller says nothing was deleted.
     */
    private async Task UninstallGameAsync()
    {
        if (updating is not null || !current.RefreshEnabled)
        {
            return;
        }
        if (MessageBox.Show(this, Texts.Format("UninstallConfirm", Options.GameDir), "Heroes of Liberty",
                MessageBoxButtons.YesNo, MessageBoxIcon.Warning, MessageBoxDefaultButton.Button2) != DialogResult.Yes)
        {
            return;
        }
        Display(StatusView.Updating(current));
        var gameDir = Options.GameDir;
        var result = await Task.Run(async () =>
        {
            VersionManifest? serverList = null;
            if (GameUninstaller.NeedsServerList(gameDir))
            {
                serverList = (await ReadLatestAsync(CancellationToken.None)).Latest?.Manifest;
            }
            return GameUninstaller.Uninstall(gameDir, serverList);
        });
        if (IsDisposed)
        {
            return;
        }
        MessageBox.Show(this, result.Message, "Heroes of Liberty", MessageBoxButtons.OK,
            result.Success ? MessageBoxIcon.Information : MessageBoxIcon.Warning);
        await RefreshStatusAsync();
    }

    private void Warn(string message)
        => MessageBox.Show(this, message, "Heroes of Liberty", MessageBoxButtons.OK, MessageBoxIcon.Warning);
}
