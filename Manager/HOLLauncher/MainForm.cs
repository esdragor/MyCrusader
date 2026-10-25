namespace HOLLauncher;

/**
 * The launcher's window, kept thin: it reads the status in the background (StatusLoader) and shows the StatusView.
 * "Update the game" runs GameUpdater in the background with a progress bar and "Cancel" (ProgressView); "Update the
 * launcher" runs LauncherUpdater the same way, then starts the new exe with the same arguments and closes (L7);
 * "Play" (L8) asks PlayConfirmation if any (outdated game), starts the game (GameLauncher) and closes the launcher.
 */
public sealed class MainForm : Form
{
    // Both send the key of servers.json to its manager (L11).
    private readonly HttpClient http;
    private readonly HttpClient downloads;
    private readonly Label installed = NewLabel();
    private readonly Label latest = NewLabel();
    private readonly Label state = NewLabel();
    private readonly Label notesTitle = NewLabel("Nouveautés :");
    private readonly TextBox notes = new()
    {
        Multiline = true,
        ReadOnly = true,
        ScrollBars = ScrollBars.Vertical,
        Dock = DockStyle.Fill,
    };
    private readonly Label launcher = NewLabel();
    private readonly Button play = NewButton("Jouer");
    private readonly Button updateGame = NewButton(StatusView.UpdateGameLabel);
    private readonly Button updateLauncher = NewButton("Mettre à jour le lanceur");
    private readonly Button refresh = NewButton("Actualiser");
    private readonly Label progressText = NewLabel();
    private readonly ProgressBar progressBar = new() { Dock = DockStyle.Fill, Minimum = 0, Maximum = 100, Height = 20 };
    private readonly Button cancel = NewButton("Annuler");
    private readonly TableLayoutPanel progressPanel = new()
    {
        Dock = DockStyle.Fill, AutoSize = true, ColumnCount = 2, Visible = false, Margin = new Padding(0),
    };
    private StatusView current = StatusView.Loading();
    private ProgressView progressView = ProgressView.Starting();
    private CancellationTokenSource? updating;
    private bool autoUpdatePending;
    private bool autoLauncherUpdatePending;

    public MainForm(LauncherOptions options)
    {
        Options = options;
        http = VersionsClient.CreateHttpClient(options.GameDir);
        downloads = FileDownloader.CreateHttpClient(options.GameDir);
        autoUpdatePending = options.UpdateAtStart;
        autoLauncherUpdatePending = options.UpdateLauncherAtStart;
        Text = $"Heroes of Liberty - lanceur {LauncherInfo.Version}";
        ClientSize = new Size(640, 400);
        MinimumSize = new Size(480, 320);
        StartPosition = FormStartPosition.CenterScreen;
        Font = new Font("Segoe UI", 10f);

        var buttons = new FlowLayoutPanel { Dock = DockStyle.Fill, AutoSize = true, WrapContents = true };
        buttons.Controls.AddRange([play, updateGame, updateLauncher, refresh]);

        var layout = new TableLayoutPanel { Dock = DockStyle.Fill, ColumnCount = 1, Padding = new Padding(12) };
        layout.ColumnStyles.Add(new ColumnStyle(SizeType.Percent, 100));
        foreach (var control in new Control[] { installed, latest, state, notesTitle })
        {
            layout.RowStyles.Add(new RowStyle(SizeType.AutoSize));
            layout.Controls.Add(control);
        }
        layout.RowStyles.Add(new RowStyle(SizeType.Percent, 100));
        layout.Controls.Add(notes);
        layout.RowStyles.Add(new RowStyle(SizeType.AutoSize));
        layout.Controls.Add(launcher);
        progressPanel.ColumnStyles.Add(new ColumnStyle(SizeType.Percent, 100));
        progressPanel.ColumnStyles.Add(new ColumnStyle(SizeType.AutoSize));
        progressPanel.Controls.Add(progressText, 0, 0);
        progressPanel.SetColumnSpan(progressText, 2);
        progressPanel.Controls.Add(progressBar, 0, 1);
        progressPanel.Controls.Add(cancel, 1, 1);
        layout.RowStyles.Add(new RowStyle(SizeType.AutoSize));
        layout.Controls.Add(progressPanel);
        layout.RowStyles.Add(new RowStyle(SizeType.AutoSize));
        layout.Controls.Add(buttons);
        Controls.Add(layout);

        play.Click += (_, _) => Play();
        updateGame.Click += async (_, _) => await UpdateGameAsync();
        updateLauncher.Click += async (_, _) => await UpdateLauncherAsync();
        refresh.Click += async (_, _) => await RefreshStatusAsync();
        cancel.Click += (_, _) => CancelUpdate();
        Shown += async (_, _) => await RefreshStatusAsync();
        FormClosing += (_, _) => updating?.Cancel(); // the downloaded part stays for the next try
        FormClosed += (_, _) =>
        {
            http.Dispose();
            downloads.Dispose();
        };
        Display(StatusView.Loading());
    }

    public LauncherOptions Options { get; }

    private async Task RefreshStatusAsync()
    {
        Display(StatusView.Loading());
        StatusView view;
        try
        {
            view = await Task.Run(() => StatusLoader.LoadAsync(Options.GameDir, new VersionsClient(http)));
        }
        catch (Exception e) when (e is IOException or UnauthorizedAccessException)
        {
            // Version.txt or servers.json unreadable on disk: shown, the user can fix it and refresh.
            view = StatusView.Error(e.Message, null);
        }
        if (!IsDisposed)
        {
            Display(view);
        }
        if (autoUpdatePending && !IsDisposed)
        {
            // --update: once, right after the first reading, if an update is there.
            autoUpdatePending = false;
            if (view.UpdateGameEnabled)
            {
                await UpdateGameAsync();
            }
        }
        if (autoLauncherUpdatePending && !IsDisposed)
        {
            // --update-launcher: likewise, once.
            autoLauncherUpdatePending = false;
            if (current.UpdateLauncherEnabled)
            {
                await UpdateLauncherAsync();
            }
        }
    }

    /**
     * Downloads the new launcher in the background (bar, "Cancel"), swaps it in, then starts it with the same
     * arguments and closes; a failure is shown and the status read again.
     */
    private async Task UpdateLauncherAsync()
    {
        if (updating is not null)
        {
            return;
        }
        var exePath = Environment.ProcessPath;
        if (exePath is null)
        {
            MessageBox.Show(this, "Erreur : chemin du lanceur inconnu.", "Heroes of Liberty", MessageBoxButtons.OK, MessageBoxIcon.Warning);
            return;
        }
        using var source = new CancellationTokenSource();
        updating = source;
        Display(StatusView.Updating(current));
        ShowProgress(ProgressView.Starting());
        progressPanel.Visible = true;
        var progress = new Progress<LauncherProgress>(p =>
        {
            if (!source.IsCancellationRequested && !IsDisposed)
            {
                ShowProgress(ProgressView.FromLauncher(p));
            }
        });
        LauncherUpdateResult result;
        try
        {
            result = await Task.Run(() => RunLauncherUpdateAsync(exePath, progress, source.Token));
        }
        finally
        {
            updating = null;
        }
        if (IsDisposed)
        {
            return;
        }
        progressPanel.Visible = false;
        if (result.Success)
        {
            try
            {
                var start = new System.Diagnostics.ProcessStartInfo(exePath) { UseShellExecute = false };
                foreach (var argument in LauncherUpdater.RestartArguments(Environment.GetCommandLineArgs().Skip(1)))
                {
                    start.ArgumentList.Add(argument);
                }
                System.Diagnostics.Process.Start(start)?.Dispose();
                Close();
                return;
            }
            catch (Exception e) when (e is System.ComponentModel.Win32Exception or IOException)
            {
                result = new LauncherUpdateResult(false, $"{result.Message} Relancez-le à la main ({e.Message}).");
            }
        }
        MessageBox.Show(this, result.Message, "Heroes of Liberty", MessageBoxButtons.OK, MessageBoxIcon.Warning);
        await RefreshStatusAsync();
    }

    private async Task<LauncherUpdateResult> RunLauncherUpdateAsync(string exePath, IProgress<LauncherProgress> progress,
        CancellationToken token)
    {
        ManagerServer server;
        try
        {
            server = GameServers.ReadFirst(Options.GameDir);
        }
        catch (Exception e) when (e is InvalidDataException or IOException or UnauthorizedAccessException)
        {
            return new LauncherUpdateResult(false, $"Erreur : {e.Message}");
        }
        return await new LauncherUpdater(downloads, server.Url, exePath).UpdateAsync(progress, token);
    }

    /** Reads servers.json and /versions/latest again (the status may be old), then runs the update in the background. */
    private async Task UpdateGameAsync()
    {
        if (updating is not null)
        {
            return;
        }
        using var source = new CancellationTokenSource();
        updating = source;
        Display(StatusView.Updating(current));
        ShowProgress(ProgressView.Starting());
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
            result = await Task.Run(() => RunUpdateAsync(progress, source.Token));
        }
        finally
        {
            updating = null;
        }
        if (IsDisposed)
        {
            return;
        }
        progressPanel.Visible = false;
        MessageBox.Show(this, result.Message, "Heroes of Liberty", MessageBoxButtons.OK,
            result.Success ? MessageBoxIcon.Information : MessageBoxIcon.Warning);
        await RefreshStatusAsync();
    }

    private async Task<UpdateResult> RunUpdateAsync(IProgress<DownloadProgress> progress, CancellationToken token)
    {
        ManagerServer server;
        try
        {
            server = GameServers.ReadFirst(Options.GameDir);
        }
        catch (Exception e) when (e is InvalidDataException or IOException or UnauthorizedAccessException)
        {
            return new UpdateResult(false, $"Erreur : {e.Message}");
        }
        LatestResult latest;
        try
        {
            latest = await new VersionsClient(http).GetLatestAsync(server.Url, token);
        }
        catch (OperationCanceledException)
        {
            return new UpdateResult(false, "Mise à jour annulée.");
        }
        if (latest.Latest is null)
        {
            return new UpdateResult(false, $"Serveur injoignable : {latest.Message}");
        }
        return await new GameUpdater(downloads).UpdateAsync(Options.GameDir, server.Url, latest.Latest, progress, token);
    }

    /** Asks the confirmation if any, starts the game, then closes (decision L8); a failure is shown, the launcher stays. */
    private void Play()
    {
        if (updating is not null || !current.PlayEnabled)
        {
            return;
        }
        if (current.PlayConfirmation is { } question
            && MessageBox.Show(this, question, "Heroes of Liberty", MessageBoxButtons.YesNo, MessageBoxIcon.Warning) != DialogResult.Yes)
        {
            return;
        }
        var result = GameLauncher.Start(Options.GameDir);
        if (result.Success)
        {
            Close();
            return;
        }
        MessageBox.Show(this, result.Message, "Heroes of Liberty", MessageBoxButtons.OK, MessageBoxIcon.Warning);
    }

    private void CancelUpdate()
    {
        updating?.Cancel();
        ShowProgress(progressView.Cancelling());
    }

    private void ShowProgress(ProgressView view)
    {
        progressView = view;
        progressText.Text = view.Text;
        progressBar.Value = view.Percent;
        cancel.Enabled = view.CancelEnabled;
    }

    private void Display(StatusView view)
    {
        current = view;
        installed.Text = view.InstalledText;
        latest.Text = view.LatestText;
        state.Text = view.StateText;
        notes.Text = view.Notes;
        notesTitle.Visible = view.NotesVisible;
        notes.Visible = view.NotesVisible;
        launcher.Text = view.LauncherText;
        play.Enabled = view.PlayEnabled;
        updateGame.Enabled = view.UpdateGameEnabled;
        updateGame.Text = view.UpdateGameText;
        updateLauncher.Enabled = view.UpdateLauncherEnabled;
        refresh.Enabled = view.RefreshEnabled;
    }

    private static Label NewLabel(string text = "") => new()
    {
        Text = text,
        AutoSize = true,
        Anchor = AnchorStyles.Left | AnchorStyles.Right, // wraps long lines in the table
        Margin = new Padding(3, 4, 3, 4),
    };

    private static Button NewButton(string text) => new() { Text = text, AutoSize = true, Enabled = false };
}
