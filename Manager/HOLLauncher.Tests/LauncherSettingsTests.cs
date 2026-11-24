namespace HOLLauncher.Tests;

public class LauncherSettingsTests
{
    [Fact]
    public void MissingFileGivesDefaults()
    {
        using var dir = new TempDir();
        var load = LauncherSettings.Load(dir.Root);
        Assert.Equal(LauncherSettings.Default, load.Settings);
        Assert.Null(load.Problem);
    }

    [Fact]
    public void SavedSettingsAreReadBack()
    {
        using var dir = new TempDir();
        var settings = new LauncherSettings(Language: "ja", GameDir: @"D:\Games\HOL");
        settings.Save(dir.Root);
        var load = LauncherSettings.Load(dir.Root);
        Assert.Equal(settings, load.Settings);
        Assert.Null(load.Problem);
        Assert.Equal([LauncherSettings.FileName], Directory.GetFiles(dir.Root).Select(Path.GetFileName));
    }

    [Fact]
    public void FileIsCamelCaseJsonWithoutEmptyFields()
    {
        using var dir = new TempDir();
        new LauncherSettings(Language: "en").Save(dir.Root);
        var text = File.ReadAllText(Path.Combine(dir.Root, LauncherSettings.FileName));
        Assert.Contains("\"language\": \"en\"", text);
        Assert.DoesNotContain("gameDir", text);
    }

    [Fact]
    public void UnknownAndMissingFieldsAreTolerated()
    {
        using var dir = new TempDir();
        dir.Write(LauncherSettings.FileName, "{ \"language\": \"en\", \"later\": 3 }");
        var load = LauncherSettings.Load(dir.Root);
        Assert.Equal(new LauncherSettings(Language: "en"), load.Settings);
        Assert.Null(load.Problem);
    }

    [Theory]
    [InlineData("{ \"language\": ")]
    [InlineData("[]")]
    [InlineData("null")]
    [InlineData("{ \"language\": 3 }")]
    public void UnreadableFileGivesDefaultsAndSaysSo(string content)
    {
        using var dir = new TempDir();
        dir.Write(LauncherSettings.FileName, content);
        var load = LauncherSettings.Load(dir.Root);
        Assert.Equal(LauncherSettings.Default, load.Settings);
        Assert.NotNull(load.Problem);
        Assert.Contains(LauncherSettings.FileName, load.Problem);
    }

    [Fact]
    public void SaveReplacesAnUnreadableFile()
    {
        using var dir = new TempDir();
        dir.Write(LauncherSettings.FileName, "garbage");
        new LauncherSettings(Language: "fr").Save(dir.Root);
        Assert.Equal(new LauncherSettings(Language: "fr"), LauncherSettings.Load(dir.Root).Settings);
    }

    [Fact]
    public void LaunchOptionsAreReadBack()
    {
        using var dir = new TempDir();
        var settings = new LauncherSettings(Language: "en", DisplayMode: "windowed", ResolutionWidth: 1600,
            ResolutionHeight: 900, ExtraArguments: "-nosound -Key=\"a b\"");
        settings.Save(dir.Root);
        Assert.Equal(settings, LauncherSettings.Load(dir.Root).Settings);
        var text = File.ReadAllText(Path.Combine(dir.Root, LauncherSettings.FileName));
        Assert.Contains("\"displayMode\": \"windowed\"", text);
        Assert.Contains("\"resolutionWidth\": 1600", text);
        Assert.Contains("\"resolutionHeight\": 900", text);
        Assert.Contains("\"extraArguments\": ", text);
    }

    [Fact]
    public void DefaultLaunchOptionsAreNotWritten()
    {
        using var dir = new TempDir();
        new LauncherSettings(Language: "fr").Save(dir.Root);
        var text = File.ReadAllText(Path.Combine(dir.Root, LauncherSettings.FileName));
        foreach (var name in new[] { "displayMode", "resolutionWidth", "resolutionHeight", "extraArguments" })
        {
            Assert.DoesNotContain(name, text);
        }
    }

    [Fact]
    public void LastCrashCheckIsReadBackInUtc()
    {
        using var dir = new TempDir();
        var time = new DateTime(2026, 11, 24, 9, 30, 15, 123, DateTimeKind.Utc);
        var settings = new LauncherSettings(Language: "en", LastCrashCheckUtc: time);
        settings.Save(dir.Root);
        var load = LauncherSettings.Load(dir.Root);
        Assert.Equal(settings, load.Settings);
        Assert.Equal(DateTimeKind.Utc, load.Settings.LastCrashCheckUtc!.Value.Kind);
        Assert.Contains("\"lastCrashCheckUtc\": \"2026-11-24T09:30:15.123Z\"",
            File.ReadAllText(Path.Combine(dir.Root, LauncherSettings.FileName)));
    }

    [Fact]
    public void NoCrashCheckIsNotWrittenAndAnL19FileStillLoads()
    {
        using var dir = new TempDir();
        new LauncherSettings(Language: "fr").Save(dir.Root);
        Assert.DoesNotContain("lastCrashCheckUtc", File.ReadAllText(Path.Combine(dir.Root, LauncherSettings.FileName)));
        // The file as L19 writes it: no crash check time.
        dir.Write(LauncherSettings.FileName, "{\n  \"language\": \"ja\",\n  \"displayMode\": \"windowed\"\n}");
        var load = LauncherSettings.Load(dir.Root);
        Assert.Null(load.Problem);
        Assert.Equal(new LauncherSettings(Language: "ja", DisplayMode: "windowed"), load.Settings);
    }

    [Fact]
    public void L15FileWithoutLaunchOptionsStillLoads()
    {
        using var dir = new TempDir();
        // The file as L15 writes it: no launch option.
        dir.Write(LauncherSettings.FileName, "{\n  \"language\": \"ja\",\n  \"gameDir\": \"D:\\\\HOL\"\n}");
        var load = LauncherSettings.Load(dir.Root);
        Assert.Null(load.Problem);
        Assert.Equal(new LauncherSettings(Language: "ja", GameDir: @"D:\HOL"), load.Settings);
        Assert.Empty(LaunchArguments.For(load.Settings));
    }

    /** L22: the theme is stored when forced ("light", "dark"); none (follow Windows) is not written. */
    [Fact]
    public void ThemeIsReadBackAndNoneIsNotWritten()
    {
        using var dir = new TempDir();
        var settings = new LauncherSettings(Language: "en", Theme: "dark");
        settings.Save(dir.Root);
        Assert.Equal(settings, LauncherSettings.Load(dir.Root).Settings);
        Assert.Contains("\"theme\": \"dark\"", File.ReadAllText(Path.Combine(dir.Root, LauncherSettings.FileName)));

        new LauncherSettings(Language: "en").Save(dir.Root);
        Assert.DoesNotContain("theme", File.ReadAllText(Path.Combine(dir.Root, LauncherSettings.FileName)));
    }

    /** Launcher 5: closing on "Play" is stored only when chosen; older files have none and stay open. */
    [Fact]
    public void CloseOnPlayIsReadBackAndAnOlderFileStaysOpen()
    {
        using var dir = new TempDir();
        var settings = new LauncherSettings(Language: "en", CloseOnPlay: true);
        settings.Save(dir.Root);
        Assert.Equal(settings, LauncherSettings.Load(dir.Root).Settings);
        Assert.Contains("\"closeOnPlay\": true", File.ReadAllText(Path.Combine(dir.Root, LauncherSettings.FileName)));

        dir.Write(LauncherSettings.FileName, "{ \"language\": \"fr\", \"theme\": \"dark\" }");
        var load = LauncherSettings.Load(dir.Root);
        Assert.Null(load.Problem);
        Assert.Null(load.Settings.CloseOnPlay);
        Assert.Equal(AfterPlay.MinimizeAndWatch, GameSession.Decide(load.Settings.CloseOnPlay));
    }

    [Fact]
    public void L21FileWithoutThemeStillLoadsAndFollowsWindows()
    {
        using var dir = new TempDir();
        // The file as L21 writes it: no theme.
        dir.Write(LauncherSettings.FileName,
            "{\n  \"language\": \"fr\",\n  \"lastCrashCheckUtc\": \"2026-11-24T09:30:15.123Z\"\n}");
        var load = LauncherSettings.Load(dir.Root);
        Assert.Null(load.Problem);
        Assert.Null(load.Settings.Theme);
        Assert.Same(Palette.Dark, Themes.Resolve(load.Settings.Theme, appsUseLightTheme: 0));
    }

    /** L23: the background fields are read back, and only written when not the default. */
    [Fact]
    public void BackgroundFieldsAreReadBack()
    {
        using var dir = new TempDir();
        var settings = new LauncherSettings(Language: "en", MinimizeToTray: true, AutoUpdate: "install", CheckEveryMinutes: 45,
            StartWithWindows: true, TrayHintShown: true);
        settings.Save(dir.Root);
        Assert.Equal(settings, LauncherSettings.Load(dir.Root).Settings);
        var text = File.ReadAllText(Path.Combine(dir.Root, LauncherSettings.FileName));
        Assert.Contains("\"minimizeToTray\": true", text);
        Assert.Contains("\"autoUpdate\": \"install\"", text);
        Assert.Contains("\"checkEveryMinutes\": 45", text);
        Assert.Contains("\"startWithWindows\": true", text);
        Assert.Contains("\"trayHintShown\": true", text);

        new LauncherSettings(Language: "en").Save(dir.Root);
        text = File.ReadAllText(Path.Combine(dir.Root, LauncherSettings.FileName));
        foreach (var name in new[] { "minimizeToTray", "autoUpdate", "checkEveryMinutes", "startWithWindows", "trayHintShown" })
        {
            Assert.DoesNotContain(name, text);
        }
    }

    /** A launcher 5 file (no background field): closing quits, updates are notified every 30 minutes, no logon start. */
    [Fact]
    public void Launcher5FileKeepsItsBehaviour()
    {
        using var dir = new TempDir();
        dir.Write(LauncherSettings.FileName, "{\n  \"language\": \"fr\",\n  \"theme\": \"dark\",\n  \"closeOnPlay\": true\n}");
        var load = LauncherSettings.Load(dir.Root);
        Assert.Null(load.Problem);
        Assert.Equal(new LauncherSettings(Language: "fr", Theme: "dark", CloseOnPlay: true), load.Settings);
        // Since 2026-11-24 (the user's choice) hiding in the notification area is the default, older files included.
        Assert.True(TrayMenu.CloseHides(load.Settings, gameRunning: false, quitting: false));
        Assert.Equal(AutoUpdateMode.Notify, BackgroundUpdatePolicy.ModeOf(load.Settings.AutoUpdate));
        Assert.Equal(TimeSpan.FromMinutes(30), BackgroundUpdatePolicy.Interval(load.Settings.CheckEveryMinutes));
        Assert.Null(load.Settings.StartWithWindows);
    }

    [Fact]
    public void ABadIntervalTypeMakesTheFileUnreadable()
    {
        using var dir = new TempDir();
        dir.Write(LauncherSettings.FileName, "{ \"checkEveryMinutes\": \"often\" }");
        Assert.NotNull(LauncherSettings.Load(dir.Root).Problem);
    }

    /** The settings window changes the theme with "with": the fields of later steps are kept. */
    [Fact]
    public void ThemeChangeKeepsTheOtherFields()
    {
        var settings = new LauncherSettings(Language: "ja", DisplayMode: "windowed", ExtraArguments: "-x");
        var chosen = settings with { Theme = Themes.SettingFor("light") };
        Assert.Equal(new LauncherSettings(Language: "ja", DisplayMode: "windowed", ExtraArguments: "-x", Theme: "light"), chosen);
    }

    [Fact]
    public void GameUpdatesNeverTouchTheSettings()
    {
        Assert.True(UpdatePlan.IsLauncherOwned(LauncherSettings.FileName));
    }

    [Theory]
    [InlineData("", null)]
    [InlineData("   ", null)]
    [InlineData(@"C:\Games\HOL", null)]
    [InlineData(@"C:\Games\HOL\", null)]
    [InlineData(@"c:\games\hol", null)]
    [InlineData(@"D:\HOL", @"D:\HOL")]
    [InlineData(@" D:\HOL\ ", @"D:\HOL")]
    public void GameDirChoice(string text, string? expected)
    {
        Assert.Equal(expected, LauncherSettings.GameDirFor(text, @"C:\Games\HOL\"));
    }

    [Theory]
    [InlineData(@"HOL")]
    [InlineData(@"..\HOL")]
    [InlineData(@"D:HOL")]
    public void RelativeGameDirIsRefused(string text)
    {
        Assert.Throws<ArgumentException>(() => LauncherSettings.GameDirFor(text, @"C:\Games\HOL"));
    }
}
