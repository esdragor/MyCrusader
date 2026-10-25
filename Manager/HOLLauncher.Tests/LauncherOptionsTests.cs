namespace HOLLauncher.Tests;

public class LauncherOptionsTests
{
    [Fact]
    public void GameDirDefaultsToLauncherFolder()
    {
        var options = LauncherOptions.Parse([], @"C:\Games\HOL\");
        Assert.Equal(Path.GetFullPath(@"C:\Games\HOL\"), options.GameDir);
    }

    [Fact]
    public void GameDirOption()
    {
        var options = LauncherOptions.Parse(["--game-dir", @"D:\HOL"], @"C:\Games\HOL");
        Assert.Equal(@"D:\HOL", options.GameDir);
    }

    [Fact]
    public void GameDirWithoutValueIsRefused()
    {
        Assert.Throws<ArgumentException>(() => LauncherOptions.Parse(["--game-dir"], @"C:\Games\HOL"));
    }

    [Fact]
    public void UnknownArgumentIsRefused()
    {
        var e = Assert.Throws<ArgumentException>(() => LauncherOptions.Parse(["--fast"], @"C:\Games\HOL"));
        Assert.Contains("--fast", e.Message);
    }

    [Fact]
    public void UpdateIsOffByDefault()
    {
        Assert.False(LauncherOptions.Parse([], @"C:\Games\HOL").UpdateAtStart);
    }

    [Fact]
    public void UpdateOption()
    {
        var options = LauncherOptions.Parse(["--update", "--game-dir", @"D:\HOL"], @"C:\Games\HOL");
        Assert.True(options.UpdateAtStart);
        Assert.Equal(@"D:\HOL", options.GameDir);
    }

    [Fact]
    public void UpdateLauncherOption()
    {
        Assert.False(LauncherOptions.Parse([], @"C:\Games\HOL").UpdateLauncherAtStart);
        Assert.True(LauncherOptions.Parse(["--update-launcher"], @"C:\Games\HOL").UpdateLauncherAtStart);
    }
}
