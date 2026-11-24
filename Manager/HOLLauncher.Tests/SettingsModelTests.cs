namespace HOLLauncher.Tests;

/** Launcher 5: the settings window's logic, apart from the WPF window (fields shown, "Save"). */
public class SettingsModelTests
{
    private static readonly string LauncherDir = Path.Combine(Path.GetTempPath(), "HOLLauncherModel");

    [Fact]
    public void StoredSettingsComeBackUnchangedThroughTheFields()
    {
        var stored = new LauncherSettings(Language: "ja", GameDir: @"D:\Games\HOL", DisplayMode: "fullscreen",
            ResolutionWidth: 2560, ResolutionHeight: 1440, ExtraArguments: "-nosound", LastCrashCheckUtc: new DateTime(2026, 11, 1),
            Theme: "dark", CloseOnPlay: true);

        var fields = SettingsModel.Fields(stored, @"D:\Games\HOL", "ja");
        var outcome = SettingsModel.Build(stored, fields, LauncherDir, "ja");

        Assert.Null(outcome.Problem);
        Assert.Equal(stored, outcome.Result);
        Assert.True(fields.CloseOnPlay);
    }

    /** A file written before launcher 5 (no closeOnPlay, no theme): saving it unchanged writes nothing new. */
    [Fact]
    public void AnOldFileRoundTripsWithoutNewFields()
    {
        using var dir = new TempDir();
        dir.Write(LauncherSettings.FileName, "{ \"language\": \"fr\", \"displayMode\": \"windowed\", \"resolutionWidth\": 1000, \"resolutionHeight\": 700 }");
        var stored = LauncherSettings.Load(dir.Root).Settings;

        var fields = SettingsModel.Fields(stored, dir.Root, "fr");
        var result = SettingsModel.Build(stored, fields, dir.Root, "fr").Result!;

        Assert.False(fields.CloseOnPlay);
        Assert.Equal(SettingsModel.ResolutionChoices("fr").Count - 1, fields.ResolutionIndex);
        Assert.Equal(("1000", "700"), (fields.Width, fields.Height));
        Assert.Equal(stored, result);
        Assert.DoesNotContain("closeOnPlay", result.ToJson());
        Assert.DoesNotContain("theme", result.ToJson());
    }

    [Fact]
    public void DefaultsShowTheGamesOwnAndTheUsualSize()
    {
        var fields = SettingsModel.Fields(LauncherSettings.Default, LauncherDir, "en");
        Assert.Equal(new SettingsFields(1, 0, LauncherDir, 0, 0, "1920", "1080", "", false), fields);
    }

    [Fact]
    public void ACommonSizeIsSelectedInTheList()
    {
        var fields = SettingsModel.Fields(new LauncherSettings(ResolutionWidth: 1920, ResolutionHeight: 1080), LauncherDir, "fr");
        Assert.Equal((1920, 1080), SettingsModel.ResolutionChoices("fr")[fields.ResolutionIndex].Size);
    }

    [Fact]
    public void CloseOnPlayIsStoredTrueOrNotAtAll()
    {
        var fields = SettingsModel.Fields(LauncherSettings.Default, LauncherDir, "fr");
        Assert.True(SettingsModel.Build(LauncherSettings.Default, fields with { CloseOnPlay = true }, LauncherDir, "fr").Result!.CloseOnPlay);
        Assert.Null(SettingsModel.Build(new LauncherSettings(CloseOnPlay: true), fields, LauncherDir, "fr").Result!.CloseOnPlay);
    }

    [Fact]
    public void TheChosenFieldsAreApplied()
    {
        var fields = SettingsModel.Fields(LauncherSettings.Default, LauncherDir, "fr") with
        {
            LanguageIndex = 1, ThemeIndex = 1, GameDir = @"E:\HOL\", DisplayIndex = 1,
            ResolutionIndex = SettingsModel.ResolutionChoices("fr").Count - 1, Width = " 1280 ", Height = "720",
            ExtraArguments = "  -nosound ",
        };

        var result = SettingsModel.Build(LauncherSettings.Default, fields, LauncherDir, "fr").Result!;

        Assert.Equal(new LauncherSettings(Language: "en", GameDir: @"E:\HOL", DisplayMode: "windowed", ResolutionWidth: 1280,
            ResolutionHeight: 720, ExtraArguments: "-nosound", Theme: "light"), result);
    }

    [Theory]
    [InlineData("relative\\dir", "", "1920", "Dossier du jeu impossible")]
    [InlineData("", "-Key=\"a", "1920", "guillemet")]
    [InlineData("", "-ResX=10", "1920", "-ResX=10")]
    [InlineData("", "", "12", "entre 320 et 16384")]
    [InlineData("", "", "abc", "entre 320 et 16384")]
    [InlineData("", "", "-1920", "entre 320 et 16384")]
    public void ProblemsKeepTheWindowOpen(string gameDir, string extra, string width, string said)
    {
        var fields = SettingsModel.Fields(LauncherSettings.Default, LauncherDir, "fr") with
        {
            GameDir = gameDir, ExtraArguments = extra, Width = width,
            ResolutionIndex = SettingsModel.ResolutionChoices("fr").Count - 1,
        };

        var outcome = SettingsModel.Build(LauncherSettings.Default, fields, LauncherDir, "fr");

        Assert.Null(outcome.Result);
        Assert.Contains(said, outcome.Problem);
    }

    [Fact]
    public void TypedSizesOnlyCountForCustom()
    {
        var fields = SettingsModel.Fields(LauncherSettings.Default, LauncherDir, "fr") with { Width = "abc", ResolutionIndex = 0 };
        var result = SettingsModel.Build(LauncherSettings.Default, fields, LauncherDir, "fr").Result!;
        Assert.Null(result.ResolutionWidth);
    }

    [Fact]
    public void ListsAreInTheLanguage()
    {
        Assert.Equal(["Français", "English", "日本語"], SettingsModel.Languages().Select(l => l.ToString()));
        Assert.Equal("ゲームの設定", SettingsModel.DisplayChoices("ja")[0].Text);
        Assert.Equal("1920 × 1080", SettingsModel.ResolutionChoices("en").First(r => r.Size == (1920, 1080)).Text);
        Assert.Equal(3, SettingsModel.ThemeChoices("en").Count);
    }
}
