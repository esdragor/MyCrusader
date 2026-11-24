namespace HOLLauncher;

/**
 * The launcher's window, kept thin: it reads the status in the background (StatusLoader) and shows the StatusView.
 * "Update the game" runs GameUpdater in the background with a progress bar and "Cancel" (ProgressView); "Update the
 * launcher" runs LauncherUpdater the same way, then starts the new exe with the same arguments and closes (L7);
 * "Play" (L8) asks PlayConfirmation if any (outdated game), starts the game (GameLauncher) with the launch options of
 * the settings (LaunchArguments, L19) and closes the launcher.
 * "Settings" (L15, while nothing runs) opens SettingsForm, saves the choice (LauncherSettings), applies the language
 * (Texts, ApplyTexts) and the game folder (GameFolder.Prepare, new HTTP clients), then reads the status again. The
 * window's fixed texts come from Texts (ApplyTexts); L16: every other text too (StatusView, ProgressView, the updaters'
 * messages, the message boxes), in Texts.Language.
 * "Pause" (L18), beside "Cancel" during either update, cancels the run but keeps the window in its paused state
 * (ProgressView.Paused): "Resume" runs the same update again, which fetches nothing already received (FileDownloader:
 * finished files skipped, a .part resumed with Range); "Cancel" while paused ends it like a cancel. Closing while
 * paused keeps the files too: "Update the game" resumes at the next start. "Files" (L17): MainForm.Files.cs.
 * Appearance (L22): the header band (HeaderBand), the news box (a RichTextBox filled by NewsFormat: version headings,
 * bullets) and the theme (Themes, Palette, ThemeApplier) of the settings, applied at start, after "Settings", and when
 * Windows' app theme changes while the setting follows it.
 */
public sealed partial class MainForm : Form
{
    // Both send the key of servers.json to its manager (L11).
    private HttpClient http;
    private HttpClient downloads;
    private readonly string launcherDir;
    private LauncherSettings settings;
    private readonly string? settingsProblem;
    private readonly Label installed = NewLabel();
    private readonly Label latest = NewLabel();
    private readonly Label state = NewLabel();
    private readonly Label notesTitle = NewLabel();
    private readonly RichTextBox news = new()
    {
        ReadOnly = true,
        BorderStyle = BorderStyle.None,
        ScrollBars = RichTextBoxScrollBars.Vertical,
        DetectUrls = false,
        ShortcutsEnabled = true,
        Dock = DockStyle.Fill,
    };
    // L22: the news box framed by a one-pixel border (Border) around a margin of the box's own colour (Surface).
    private readonly Panel newsFrame = new() { Dock = DockStyle.Fill, Padding = new Padding(1), Margin = new Padding(3, 2, 3, 6) };
    private readonly Panel newsMargin = new() { Dock = DockStyle.Fill, Padding = new Padding(10, 8, 4, 8) };
    private readonly HeaderBand header = new();
    private readonly Label launcher = NewLabel();
    private readonly ThemedButton play = NewButton();
    private readonly ThemedButton updateGame = NewButton();
    private readonly ThemedButton updateLauncher = NewButton();
    private readonly ThemedButton refresh = NewButton();
    private readonly ThemedButton settingsButton = NewButton();
    private readonly Label progressText = NewLabel();
    private readonly ProgressBar progressBar = new() { Dock = DockStyle.Fill, Minimum = 0, Maximum = 100, Height = 20 };
    private readonly ThemedButton pause = NewButton();
    private readonly ThemedButton cancel = NewButton();
    private readonly TableLayoutPanel progressPanel = new()
    {
        Dock = DockStyle.Fill, AutoSize = true, ColumnCount = 3, Visible = false, Margin = new Padding(0),
    };
    private StatusView current = StatusView.Loading();
    private ProgressView progressView = ProgressView.Starting();
    private CancellationTokenSource? updating;
    private bool autoUpdatePending;
    private bool autoLauncherUpdatePending;
    // Pause (L18): asked during the run; once paused, the run to start again on "Resume" and its cancel message.
    private bool pauseAsked;
    private ProgressView? pausedLine;
    private Func<Task>? resume;
    private string? pausedMessage;
    // L22: the theme in use, and what the news box shows (filled again only when either changes).
    private Palette palette = Palette.Light;
    private IReadOnlyList<NewsBlock>? shownNews;
    private Palette? shownPalette;

    public MainForm(LauncherOptions options, string launcherDir, SettingsLoad settingsLoad)
    {
        Options = options;
        this.launcherDir = launcherDir;
        settings = settingsLoad.Settings;
        settingsProblem = settingsLoad.Problem;
        http = VersionsClient.CreateHttpClient(options.GameDir);
        downloads = FileDownloader.CreateHttpClient(options.GameDir);
        autoUpdatePending = options.UpdateAtStart;
        autoLauncherUpdatePending = options.UpdateLauncherAtStart;
        // L22: larger than before for the header band; the minimum keeps a few lines of news (fr, ja, 100 % DPI).
        ClientSize = new Size(760, 600);
        MinimumSize = new Size(640, 600);
        StartPosition = FormStartPosition.CenterScreen;

        var buttons = new FlowLayoutPanel { Dock = DockStyle.Fill, AutoSize = true, WrapContents = true };
        buttons.Controls.AddRange([play, updateGame, updateLauncher, refresh, settingsButton]);
        AddFilesMenu(buttons); // L17: MainForm.Files.cs
        AddReportItem(); // L20: MainForm.Report.cs

        var layout = new TableLayoutPanel { Dock = DockStyle.Fill, ColumnCount = 1, Padding = new Padding(12) };
        layout.ColumnStyles.Add(new ColumnStyle(SizeType.Percent, 100));
        foreach (var control in new Control[] { installed, latest, state, notesTitle })
        {
            layout.RowStyles.Add(new RowStyle(SizeType.AutoSize));
            layout.Controls.Add(control);
        }
        layout.RowStyles.Add(new RowStyle(SizeType.Percent, 100));
        newsMargin.Controls.Add(news);
        newsFrame.Controls.Add(newsMargin);
        layout.Controls.Add(newsFrame);
        layout.RowStyles.Add(new RowStyle(SizeType.AutoSize));
        layout.Controls.Add(launcher);
        progressPanel.ColumnStyles.Add(new ColumnStyle(SizeType.Percent, 100));
        progressPanel.ColumnStyles.Add(new ColumnStyle(SizeType.AutoSize));
        progressPanel.ColumnStyles.Add(new ColumnStyle(SizeType.AutoSize));
        progressPanel.Controls.Add(progressText, 0, 0);
        progressPanel.SetColumnSpan(progressText, 3);
        progressPanel.Controls.Add(progressBar, 0, 1);
        progressPanel.Controls.Add(pause, 1, 1);
        progressPanel.Controls.Add(cancel, 2, 1);
        layout.RowStyles.Add(new RowStyle(SizeType.AutoSize));
        layout.Controls.Add(progressPanel);
        layout.RowStyles.Add(new RowStyle(SizeType.AutoSize));
        layout.Controls.Add(buttons);
        Controls.Add(layout);
        Controls.Add(header);
        layout.BringToFront(); // docked last: fills what the header band leaves
        play.Accent = true;

        play.Click += (_, _) => Play();
        updateGame.Click += async (_, _) => await UpdateGameAsync();
        updateLauncher.Click += async (_, _) => await UpdateLauncherAsync();
        refresh.Click += async (_, _) => await RefreshStatusAsync();
        cancel.Click += async (_, _) => await CancelUpdateAsync();
        pause.Click += async (_, _) => await PauseOrResumeAsync();
        settingsButton.Click += async (_, _) => await OpenSettingsAsync();
        Shown += async (_, _) =>
        {
            if (settingsProblem is not null)
            {
                MessageBox.Show(this, Texts.Format("SettingsUnreadable", settingsProblem), "Heroes of Liberty",
                    MessageBoxButtons.OK, MessageBoxIcon.Warning);
            }
            await OfferReportAfterCrashAsync(); // L20
            await RefreshStatusAsync();
        };
        FormClosing += (_, _) => updating?.Cancel(); // the downloaded part stays for the next try
        FormClosed += (_, _) =>
        {
            Microsoft.Win32.SystemEvents.UserPreferenceChanged -= OnUserPreferenceChanged;
            http.Dispose();
            downloads.Dispose();
        };
        Microsoft.Win32.SystemEvents.UserPreferenceChanged += OnUserPreferenceChanged;
        ApplyTexts();
        ApplyTheme();
        Display(StatusView.Loading());
    }

    public LauncherOptions Options { get; private set; }

    /** The window's fixed texts and font in Texts.Language (at start and after a language change). */
    private void ApplyTexts()
    {
        Text = Texts.Format("WindowTitle", LauncherInfo.Version);
        Font = new Font(Texts.FontFamily(Texts.Language), 10f);
        state.Font = new Font(Font, FontStyle.Bold);
        header.Subtitle = Texts.Get("HeaderSubtitle");
        notesTitle.Text = Texts.Get("NotesTitle");
        play.Text = Texts.Get("Play");
        updateLauncher.Text = Texts.Get("UpdateLauncher");
        refresh.Text = Texts.Get("Refresh");
        settingsButton.Text = Texts.Get("Settings");
        cancel.Text = Texts.Get("Cancel");
        pause.Text = progressView.PauseText();
        ApplyFilesTexts();
        ApplyReportTexts();
        ShowNews(current.News);
    }

    /** The theme of the settings (L22): colours of the window, its menu and the news box; the title bar too. */
    private void ApplyTheme()
    {
        palette = Themes.Current(settings);
        ThemeApplier.Apply(this, palette);
        ThemeApplier.Apply(filesMenu, palette);
        newsFrame.BackColor = palette.Border;
        newsMargin.BackColor = palette.Surface;
        launcher.ForeColor = palette.Muted;
        ShowNews(current.News);
    }

    /** Windows' app theme changed (Settings > Personalisation > Colours): followed when the setting says so. */
    private void OnUserPreferenceChanged(object? sender, Microsoft.Win32.UserPreferenceChangedEventArgs e)
    {
        if (e.Category != Microsoft.Win32.UserPreferenceCategory.General || IsDisposed || !IsHandleCreated)
        {
            return;
        }
        BeginInvoke(() =>
        {
            if (!IsDisposed && Themes.Current(settings) != palette)
            {
                ApplyTheme();
            }
        });
    }

    /** Fills the news box again only when the news, the palette or the language's font changed (keeps the scroll). */
    private void ShowNews(IReadOnlyList<NewsBlock> blocks)
    {
        var family = Texts.FontFamily(Texts.Language);
        if (ReferenceEquals(blocks, shownNews) && ReferenceEquals(palette, shownPalette) && news.Font.Name == family)
        {
            return;
        }
        shownNews = blocks;
        shownPalette = palette;
        news.Font = new Font(family, NewsFormat.BodySize);
        NewsFormat.Fill(news, NewsFormat.Paragraphs(blocks, palette), family);
    }

    /**
     * Only while nothing runs (Refresh enabled). The new game folder is made ready first (a failure keeps the old
     * settings); a settings file that cannot be written is said, the choice still applies until the launcher closes.
     */
    private async Task OpenSettingsAsync()
    {
        if (updating is not null || !current.RefreshEnabled)
        {
            return;
        }
        LauncherSettings chosen;
        using (var form = new SettingsForm(settings, launcherDir, Options.GameDir, palette))
        {
            if (form.ShowDialog(this) != DialogResult.OK || form.Result is null)
            {
                return;
            }
            chosen = form.Result;
        }
        var gameDir = LauncherOptions.GameDirFrom(chosen, launcherDir);
        var folderChanged = !GameFolder.Same(gameDir, Options.GameDir);
        if (folderChanged)
        {
            try
            {
                GameFolder.Prepare(Options.GameDir, gameDir);
            }
            catch (Exception e) when (e is IOException or UnauthorizedAccessException or ArgumentException or NotSupportedException)
            {
                MessageBox.Show(this, Texts.Format("SettingsBadFolder", e.Message), "Heroes of Liberty", MessageBoxButtons.OK, MessageBoxIcon.Warning);
                return;
            }
        }
        try
        {
            chosen.Save(launcherDir);
        }
        catch (Exception e) when (e is IOException or UnauthorizedAccessException)
        {
            MessageBox.Show(this, Texts.Format("SettingsNotSaved", e.Message), "Heroes of Liberty", MessageBoxButtons.OK, MessageBoxIcon.Warning);
        }
        settings = chosen;
        Texts.Use(Texts.Resolve(chosen.Language, System.Globalization.CultureInfo.CurrentUICulture));
        ApplyTexts();
        ApplyTheme();
        if (folderChanged)
        {
            Options = Options with { GameDir = gameDir };
            http.Dispose();
            downloads.Dispose();
            http = VersionsClient.CreateHttpClient(gameDir);
            downloads = FileDownloader.CreateHttpClient(gameDir);
        }
        await RefreshStatusAsync();
    }

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
            MessageBox.Show(this, Texts.Get("LauncherPathUnknown"), "Heroes of Liberty", MessageBoxButtons.OK, MessageBoxIcon.Warning);
            return;
        }
        using var source = new CancellationTokenSource();
        updating = source;
        pauseAsked = false;
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
        if (IsDisposed || EnterPauseIfAsked(result.Cancelled, result.Message, UpdateLauncherAsync))
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
                result = new LauncherUpdateResult(false, Texts.Format("LauncherRestartByHand", result.Message, e.Message));
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
            return new LauncherUpdateResult(false, Texts.Format("Error", e.Message));
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
        pauseAsked = false;
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
        if (IsDisposed || EnterPauseIfAsked(result.Cancelled, result.Message, UpdateGameAsync))
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
            return new UpdateResult(false, Texts.Format("Error", e.Message));
        }
        LatestResult latest;
        try
        {
            latest = await new VersionsClient(http).GetLatestAsync(server.Url, token);
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
        var result = GameLauncher.Start(Options.GameDir, LaunchArguments.For(settings));
        if (result.Success)
        {
            Close();
            return;
        }
        MessageBox.Show(this, result.Message, "Heroes of Liberty", MessageBoxButtons.OK, MessageBoxIcon.Warning);
    }

    private async Task CancelUpdateAsync()
    {
        if (resume is not null)
        {
            await EndPauseAsync();
            return;
        }
        updating?.Cancel();
        ShowProgress(progressView.Cancelling());
    }

    /** "Pause" stops the run (its token, like Cancel) and asks for the paused state at its end; "Resume" runs it again. */
    private async Task PauseOrResumeAsync()
    {
        if (resume is { } again)
        {
            resume = null;
            pausedMessage = null;
            await again();
            return;
        }
        if (updating is null)
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
        ShowProgress((pausedLine ?? progressView).Paused());
        return true;
    }

    /** "Cancel" while paused: the run's cancel message, as after a cancel; what was received stays for the next run. */
    private async Task EndPauseAsync()
    {
        var message = pausedMessage ?? "";
        resume = null;
        pausedMessage = null;
        progressPanel.Visible = false;
        MessageBox.Show(this, message, "Heroes of Liberty", MessageBoxButtons.OK, MessageBoxIcon.Warning);
        await RefreshStatusAsync();
    }

    private void ShowProgress(ProgressView view)
    {
        progressView = view;
        progressText.Text = view.Text;
        progressBar.Value = view.Percent;
        cancel.Enabled = view.CancelEnabled;
        pause.Enabled = view.PauseEnabled;
        pause.Text = view.PauseText();
    }

    private void Display(StatusView view)
    {
        current = view;
        installed.Text = view.InstalledText;
        latest.Text = view.LatestText;
        state.Text = view.StateText;
        ShowNews(view.News);
        notesTitle.Visible = view.NotesVisible;
        newsFrame.Visible = view.NotesVisible;
        launcher.Text = view.LauncherText;
        play.Enabled = view.PlayEnabled;
        updateGame.Enabled = view.UpdateGameEnabled;
        updateGame.Text = view.UpdateGameText;
        updateLauncher.Enabled = view.UpdateLauncherEnabled;
        refresh.Enabled = view.RefreshEnabled;
        settingsButton.Enabled = view.RefreshEnabled;
        filesButton.Enabled = view.RefreshEnabled;
    }

    private static Label NewLabel(string text = "") => new()
    {
        Text = text,
        AutoSize = true,
        Anchor = AnchorStyles.Left | AnchorStyles.Right, // wraps long lines in the table
        Margin = new Padding(3, 3, 3, 3),
    };

    /** L22: drawn in the theme's colours, a disabled one included (ThemedButton). */
    private static ThemedButton NewButton(string text = "") => new() { Text = text, AutoSize = true, Enabled = false };
}
