namespace HOLLauncher;

/**
 * The launcher's window, kept thin: it reads the status in the background (StatusLoader) and shows the StatusView.
 * The update buttons and "Play" do nothing yet (L6/L7, L8).
 */
public sealed class MainForm : Form
{
    private readonly HttpClient http = VersionsClient.CreateHttpClient();
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

    public MainForm(LauncherOptions options)
    {
        Options = options;
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
        layout.RowStyles.Add(new RowStyle(SizeType.AutoSize));
        layout.Controls.Add(buttons);
        Controls.Add(layout);

        updateGame.Click += (_, _) => Coming("La mise à jour du jeu est à venir (L6/L7).");
        updateLauncher.Click += (_, _) => Coming("La mise à jour du lanceur est à venir (L6/L7).");
        refresh.Click += async (_, _) => await RefreshStatusAsync();
        Shown += async (_, _) => await RefreshStatusAsync();
        FormClosed += (_, _) => http.Dispose();
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
    }

    private void Display(StatusView view)
    {
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
