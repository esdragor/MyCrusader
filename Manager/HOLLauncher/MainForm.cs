namespace HOLLauncher;

/**
 * The launcher's window, kept thin: it reads the status in the background (StatusLoader) and shows the StatusView.
 * "Update the game" runs GameUpdater in the background with a progress bar and "Cancel" (ProgressView); "Update the
 * launcher" (L7) and "Play" (L8) do nothing yet.
 */
public sealed class MainForm : Form
{
    private readonly HttpClient http = VersionsClient.CreateHttpClient();
    private readonly HttpClient downloads = FileDownloader.CreateHttpClient();
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
    private readonly Button updateGame = NewButton("Mettre à jour le jeu");
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

    public MainForm(LauncherOptions options)
    {
        Options = options;
        autoUpdatePending = options.UpdateAtStart;
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

        updateGame.Click += async (_, _) => await UpdateGameAsync();
        updateLauncher.Click += (_, _) => Coming("La mise à jour du lanceur est à venir (L7).");
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
        updateLauncher.Enabled = view.UpdateLauncherEnabled;
        refresh.Enabled = view.RefreshEnabled;
    }

    private void Coming(string message)
        => MessageBox.Show(this, message, "Heroes of Liberty", MessageBoxButtons.OK, MessageBoxIcon.Information);

    private static Label NewLabel(string text = "") => new()
    {
        Text = text,
        AutoSize = true,
        Anchor = AnchorStyles.Left | AnchorStyles.Right, // wraps long lines in the table
        Margin = new Padding(3, 4, 3, 4),
    };

    private static Button NewButton(string text) => new() { Text = text, AutoSize = true, Enabled = false };
}
