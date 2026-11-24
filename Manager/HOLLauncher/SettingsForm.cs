namespace HOLLauncher;

/**
 * The "Settings" window (L15), modal, kept thin: language (each named in its own language) and game folder ("Browse…",
 * "Launcher folder" puts the default back). "Save" checks the folder (LauncherSettings.GameDirFor) and gives Result,
 * the old settings with these fields changed (later fields kept); MainForm saves and applies them.
 */
public sealed class SettingsForm : Form
{
    private readonly ComboBox language = new() { DropDownStyle = ComboBoxStyle.DropDownList, Width = 200 };
    private readonly TextBox gameDir = new() { Dock = DockStyle.Fill, MinimumSize = new Size(320, 0) };
    private readonly LauncherSettings settings;
    private readonly string launcherDir;

    public SettingsForm(LauncherSettings settings, string launcherDir, string currentGameDir)
    {
        this.settings = settings;
        this.launcherDir = launcherDir;
        Text = Texts.Get("Settings");
        Font = new Font(Texts.FontFamily(Texts.Language), 10f);
        FormBorderStyle = FormBorderStyle.FixedDialog;
        MaximizeBox = false;
        MinimizeBox = false;
        ShowInTaskbar = false;
        StartPosition = FormStartPosition.CenterParent;
        AutoSize = true;
        AutoSizeMode = AutoSizeMode.GrowAndShrink;

        foreach (var code in Texts.Languages)
        {
            language.Items.Add(new LanguageItem(code));
        }
        language.SelectedIndex = Math.Max(0, Texts.Languages.ToList().IndexOf(Texts.Language));
        gameDir.Text = currentGameDir;

        var browse = new Button { Text = Texts.Get("SettingsBrowse"), AutoSize = true };
        var launcherFolder = new Button { Text = Texts.Get("SettingsLauncherDir"), AutoSize = true };
        var save = new Button { Text = Texts.Get("SettingsSave"), AutoSize = true };
        var cancel = new Button { Text = Texts.Get("Cancel"), AutoSize = true, DialogResult = DialogResult.Cancel };
        AcceptButton = save;
        CancelButton = cancel;

        var layout = new TableLayoutPanel { AutoSize = true, ColumnCount = 3, Padding = new Padding(12), Dock = DockStyle.Fill };
        layout.Controls.Add(NewLabel(Texts.Get("SettingsLanguage")), 0, 0);
        layout.Controls.Add(language, 1, 0);
        layout.Controls.Add(NewLabel(Texts.Get("SettingsGameDir")), 0, 1);
        layout.Controls.Add(gameDir, 1, 1);
        layout.Controls.Add(browse, 2, 1);
        layout.Controls.Add(launcherFolder, 1, 2);
        var hint = NewLabel(Texts.Get("SettingsGameDirHint"));
        hint.MaximumSize = new Size(520, 0);
        layout.Controls.Add(hint, 0, 3);
        layout.SetColumnSpan(hint, 3);
        var buttons = new FlowLayoutPanel { AutoSize = true, FlowDirection = FlowDirection.RightToLeft, Dock = DockStyle.Fill };
        buttons.Controls.AddRange([cancel, save]);
        layout.Controls.Add(buttons, 0, 4);
        layout.SetColumnSpan(buttons, 3);
        Controls.Add(layout);

        browse.Click += (_, _) => Browse();
        launcherFolder.Click += (_, _) => gameDir.Text = Path.TrimEndingDirectorySeparator(Path.GetFullPath(launcherDir));
        save.Click += (_, _) => Save();
    }

    /** The settings chosen ("Save"); null if the window was cancelled. */
    public LauncherSettings? Result { get; private set; }

    private void Browse()
    {
        using var dialog = new FolderBrowserDialog
        {
            Description = Texts.Get("SettingsBrowseDescription"),
            UseDescriptionForTitle = true,
            ShowNewFolderButton = true,
            InitialDirectory = Directory.Exists(gameDir.Text) ? gameDir.Text : launcherDir,
        };
        if (dialog.ShowDialog(this) == DialogResult.OK)
        {
            gameDir.Text = dialog.SelectedPath;
        }
    }

    private void Save()
    {
        string? dir;
        try
        {
            dir = LauncherSettings.GameDirFor(gameDir.Text, launcherDir);
        }
        catch (ArgumentException e)
        {
            MessageBox.Show(this, Texts.Format("SettingsBadFolder", e.Message), "Heroes of Liberty", MessageBoxButtons.OK, MessageBoxIcon.Warning);
            return;
        }
        var code = ((LanguageItem)language.SelectedItem!).Code;
        Result = settings with { Language = code, GameDir = dir };
        DialogResult = DialogResult.OK;
        Close();
    }

    private static Label NewLabel(string text) => new()
    {
        Text = text,
        AutoSize = true,
        Anchor = AnchorStyles.Left,
        Margin = new Padding(3, 6, 3, 6),
    };

    private sealed record LanguageItem(string Code)
    {
        public override string ToString() => Texts.LanguageName(Code);
    }
}
