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
