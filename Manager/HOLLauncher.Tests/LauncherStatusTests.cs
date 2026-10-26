using HOLUpdate;

namespace HOLLauncher.Tests;

public class LauncherStatusTests
{
    private static LatestResult Ok(string version, string notes = "News.", string launcher = "")
        => new(LatestOutcome.Ok, new LatestVersion(version, new VersionManifest(version, []), notes, [], launcher), "");

    [Fact]
    public void NotInstalled()
    {
        var status = LauncherStatus.Compute(null, Ok("v2"));
        Assert.Equal(GameState.NotInstalled, status.State);
        Assert.Null(status.Installed);
        Assert.Equal("v2", status.Latest);
        Assert.Equal("News.", status.Notes);
    }

    [Fact]
    public void UpToDate()
    {
        var status = LauncherStatus.Compute("v2", Ok("v2"));
        Assert.Equal(GameState.UpToDate, status.State);
        Assert.Equal("v2", status.Installed);
    }

    [Fact]
    public void OutdatedWhenVersionsDiffer()
    {
        Assert.Equal(GameState.Outdated, LauncherStatus.Compute("v1", Ok("v2")).State);
        // Names are not ordered: a "newer" installed version is still not the latest one.
        Assert.Equal(GameState.Outdated, LauncherStatus.Compute("v3", Ok("v2")).State);
    }

    [Theory]
    [InlineData(LatestOutcome.Unreachable)]
    [InlineData(LatestOutcome.Refused)]
    public void UnreachableKeepsInstalledVersionAndMessage(LatestOutcome outcome)
    {
        var status = LauncherStatus.Compute("v1", new LatestResult(outcome, null, "connection refused"));
        Assert.Equal(GameState.Unreachable, status.State);
        Assert.Equal("v1", status.Installed);
        Assert.Null(status.Latest);
        Assert.Equal("", status.Notes);
        Assert.Equal("connection refused", status.Message);
        Assert.False(status.LauncherUpdateAvailable);
    }

    [Fact]
    public void LauncherUpdateAvailableWhenAnotherLauncherIsPublished()
    {
        var status = LauncherStatus.Compute("v2", Ok("v2", launcher: "2"), launcherCurrent: "1");
        Assert.Equal("1", status.LauncherCurrent);
        Assert.Equal("2", status.LauncherAvailable);
        Assert.True(status.LauncherUpdateAvailable);
    }

    [Fact]
    public void NoLauncherUpdateWhenSameOrNonePublished()
    {
        Assert.False(LauncherStatus.Compute("v2", Ok("v2", launcher: "1"), launcherCurrent: "1").LauncherUpdateAvailable);
        Assert.False(LauncherStatus.Compute("v2", Ok("v2", launcher: ""), launcherCurrent: "1").LauncherUpdateAvailable);
    }

    [Fact]
    public void DefaultLauncherVersionIsTheConstant()
    {
        Assert.Equal(LauncherInfo.Version, LauncherStatus.Compute("v2", Ok("v2")).LauncherCurrent);
    }
}
