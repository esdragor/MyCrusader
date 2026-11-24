namespace HOLLauncher.Tests;

/** Launcher 5: after "Play" the launcher stays (minimized, watching the game) unless the settings say to close. */
public class GameSessionTests
{
    [Theory]
    [InlineData(null, AfterPlay.MinimizeAndWatch)]
    [InlineData(false, AfterPlay.MinimizeAndWatch)]
    [InlineData(true, AfterPlay.Close)]
    public void StaysOpenUnlessAskedToClose(bool? closeOnPlay, AfterPlay expected)
    {
        Assert.Equal(expected, GameSession.Decide(closeOnPlay));
    }

    [Fact]
    public void DefaultSettingsStayOpen()
    {
        Assert.Equal(AfterPlay.MinimizeAndWatch, GameSession.Decide(LauncherSettings.Default.CloseOnPlay));
    }

    [Theory]
    [InlineData("HeroesOfLiberty", true)]
    [InlineData("heroesofliberty", true)]
    [InlineData("HeroesOfLiberty-Win64-Shipping", true)]
    [InlineData("HeroesOfLiberty-Win64-Development", true)]
    [InlineData("HeroesOfLibertyServer", false)]
    [InlineData("HeroesOfLibertyServer-Win64-Shipping", false)]
    [InlineData("HOLManager", false)]
    [InlineData("HOLLauncher", false)]
    public void OnlyGameClientProcessesCount(string name, bool game)
    {
        Assert.Equal(game, GameSession.IsGameProcessName(name));
    }

    /** The started process exits, then the watch goes on while a game process is said to be left. */
    [Fact]
    public async Task WaitsForTheProcessThenForTheLeftOvers()
    {
        using var game = System.Diagnostics.Process.Start(new System.Diagnostics.ProcessStartInfo(
            Path.Combine(Environment.SystemDirectory, "cmd.exe"), "/c exit 0") { UseShellExecute = false, CreateNoWindow = true })!;
        var asked = 0;
        await GameSession.WaitForExitAsync(game, () => ++asked < 3, TimeSpan.FromMilliseconds(10));
        Assert.True(game.HasExited);
        Assert.Equal(3, asked);
    }

    [Fact]
    public async Task ClosingTheLauncherStopsTheWatch()
    {
        using var game = System.Diagnostics.Process.Start(new System.Diagnostics.ProcessStartInfo(
            Path.Combine(Environment.SystemDirectory, "cmd.exe"), "/c exit 0") { UseShellExecute = false, CreateNoWindow = true })!;
        using var cancel = new CancellationTokenSource(TimeSpan.FromMilliseconds(100));
        await Assert.ThrowsAnyAsync<OperationCanceledException>(
            () => GameSession.WaitForExitAsync(game, () => true, TimeSpan.FromMilliseconds(10), cancel.Token));
    }
}
