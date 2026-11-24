namespace HOLLauncher;

/**
 * The "Settings" window (L15), modal, kept thin: language (each named in its own language) and game folder ("Browse…",
 * "Launcher folder" puts the default back). "Save" checks the folder (LauncherSettings.GameDirFor) and gives Result,
 * the old settings with these fields changed (later fields kept); MainForm saves and applies them.
 * "Game launch" (L19): display mode and resolution ("Game setting" = no argument, the usual sizes, "Custom" with two
 * sizes bounded by LaunchArguments.MinSize / MaxSize) and the extra arguments; "Save" refuses an unclosed quote and
 * extra arguments the launch would leave out (LaunchArguments.Rejected) rather than drop them silently.
 */
public sealed class SettingsForm : Form
{
    private readonly ComboBox language = new() { DropDownStyle = ComboBoxStyle.DropDownList, Width = 200 };
    private readonly TextBox gameDir = new() { Dock = DockStyle.Fill, MinimumSize = new Size(320, 0) };
    private readonly ComboBox displayMode = new() { DropDownStyle = ComboBoxStyle.DropDownList, Width = 200 };
    private readonly ComboBox resolution = new() { DropDownStyle = ComboBoxStyle.DropDownList, Width = 200 };
    private readonly NumericUpDown width = NewSize();
    private readonly NumericUpDown height = NewSize();
    private readonly TextBox extraArguments = new() { Dock = DockStyle.Fill };
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
        FillLaunchOptions();

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
        var launch = NewLabel(Texts.Get("SettingsLaunch"));
        launch.Font = new Font(Font, FontStyle.Bold);
        launch.Margin = new Padding(3, 16, 3, 6);
        layout.Controls.Add(launch, 0, 4);
        layout.SetColumnSpan(launch, 3);
        layout.Controls.Add(NewLabel(Texts.Get("SettingsDisplayMode")), 0, 5);
        layout.Controls.Add(displayMode, 1, 5);
        layout.Controls.Add(NewLabel(Texts.Get("SettingsResolution")), 0, 6);
        layout.Controls.Add(resolution, 1, 6);
        var size = new FlowLayoutPanel { AutoSize = true, WrapContents = false, Margin = Padding.Empty };
        size.Controls.AddRange([width, NewLabel("×"), height]);
        layout.Controls.Add(size, 1, 7);
        layout.Controls.Add(NewLabel(Texts.Get("SettingsExtraArguments")), 0, 8);
        layout.Controls.Add(extraArguments, 1, 8);
        layout.SetColumnSpan(extraArguments, 2);
        var extraHint = NewLabel(Texts.Get("SettingsExtraArgumentsHint"));
        extraHint.MaximumSize = new Size(520, 0);
        layout.Controls.Add(extraHint, 0, 9);
        layout.SetColumnSpan(extraHint, 3);
        var buttons = new FlowLayoutPanel { AutoSize = true, FlowDirection = FlowDirection.RightToLeft, Dock = DockStyle.Fill };
        buttons.Controls.AddRange([cancel, save]);
        layout.Controls.Add(buttons, 0, 10);
        layout.SetColumnSpan(buttons, 3);
        Controls.Add(layout);

        browse.Click += (_, _) => Browse();
        launcherFolder.Click += (_, _) => gameDir.Text = Path.TrimEndingDirectorySeparator(Path.GetFullPath(launcherDir));
        save.Click += (_, _) => Save();
        resolution.SelectedIndexChanged += (_, _) => ShowResolution();
        ShowResolution();
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
        var extra = extraArguments.Text.Trim();
        if (LaunchArguments.HasUnclosedQuote(extra))
        {
            MessageBox.Show(this, Texts.Get("SettingsUnclosedQuote"), "Heroes of Liberty", MessageBoxButtons.OK, MessageBoxIcon.Warning);
            return;
        }
        var (resolutionWidth, resolutionHeight) = ChosenResolution();
        var chosen = settings with
        {
            Language = code,
            GameDir = dir,
            DisplayMode = ((DisplayItem)displayMode.SelectedItem!).Mode,
            ResolutionWidth = resolutionWidth,
            ResolutionHeight = resolutionHeight,
            ExtraArguments = extra == "" ? null : extra,
        };
        if (LaunchArguments.Rejected(chosen) is { Count: > 0 } rejected)
        {
            MessageBox.Show(this, Texts.Format("SettingsReservedArguments", string.Join(" ", rejected)), "Heroes of Liberty",
                MessageBoxButtons.OK, MessageBoxIcon.Warning);
            return;
        }
        Result = chosen;
        DialogResult = DialogResult.OK;
        Close();
    }

    /** The saved options shown: an unknown mode or an invalid size shows "Game setting", as the launch treats them. */
    private void FillLaunchOptions()
    {
        displayMode.Items.AddRange([
            new DisplayItem(null, Texts.Get("SettingsGameOwn")),
            new DisplayItem(LaunchArguments.Windowed, Texts.Get("SettingsWindowed")),
            new DisplayItem(LaunchArguments.Fullscreen, Texts.Get("SettingsFullscreen")),
        ]);
        displayMode.SelectedIndex = Math.Max(0, displayMode.Items.Cast<DisplayItem>().ToList().FindIndex(
            item => item.Mode is not null && string.Equals(item.Mode, settings.DisplayMode, StringComparison.OrdinalIgnoreCase)));

        resolution.Items.Add(new ResolutionItem(null, Texts.Get("SettingsGameOwn")));
        foreach (var (w, h) in LaunchArguments.CommonResolutions)
        {
            resolution.Items.Add(new ResolutionItem((w, h), $"{w} × {h}"));
        }
        resolution.Items.Add(new ResolutionItem(null, Texts.Get("SettingsResolutionCustom"), Custom: true));
        (int Width, int Height)? saved = settings is { ResolutionWidth: { } sw, ResolutionHeight: { } sh }
            && LaunchArguments.IsValidResolution(sw, sh) ? (sw, sh) : null;
        var common = resolution.Items.Cast<ResolutionItem>().ToList().FindIndex(item => saved is not null && item.Size == saved);
        resolution.SelectedIndex = saved is null ? 0 : common >= 0 ? common : resolution.Items.Count - 1;
        var (shownWidth, shownHeight) = saved ?? (1920, 1080);
        width.Value = shownWidth;
        height.Value = shownHeight;
        extraArguments.Text = settings.ExtraArguments ?? "";
    }

    /** The two sizes are typed only for "Custom"; otherwise they show the size chosen in the list. */
    private void ShowResolution()
    {
        var item = (ResolutionItem)resolution.SelectedItem!;
        width.Enabled = height.Enabled = item.Custom;
        if (item.Size is var (w, h))
        {
            width.Value = w;
            height.Value = h;
        }
    }

    private (int? Width, int? Height) ChosenResolution()
    {
        var item = (ResolutionItem)resolution.SelectedItem!;
        if (item.Custom)
        {
            return ((int)width.Value, (int)height.Value);
        }
        return item.Size is var (w, h) ? (w, h) : (null, null);
    }

    /** NumericUpDown keeps the size within the bounds LaunchArguments accepts. */
    private static NumericUpDown NewSize() => new()
    {
        Minimum = LaunchArguments.MinSize,
        Maximum = LaunchArguments.MaxSize,
        Value = 1920,
        Width = 90,
    };

    private static Label NewLabel(string text) => new()
    {
        Text = text,
        AutoSize = true,
        Anchor = AnchorStyles.Left,
        Margin = new Padding(3, 6, 3, 6),
    };

    private sealed record DisplayItem(string? Mode, string Text)
    {
        public override string ToString() => Text;
    }

    private sealed record ResolutionItem((int Width, int Height)? Size, string Text, bool Custom = false)
    {
        public override string ToString() => Text;
    }

    private sealed record LanguageItem(string Code)
    {
        public override string ToString() => Texts.LanguageName(Code);
    }
}
