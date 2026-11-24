namespace HOLLauncher.Tests;

/** L17: where the packaged game writes its logs ("Open the logs folder"). */
public class GameLogsTests
{
    [Fact]
    public void LogsBesideTheGameComeFirst()
    {
        using var game = new TempDir();
        using var local = new TempDir();
        game.Write("HeroesOfLiberty/Saved/Logs/HeroesOfLiberty.log", "log");
        local.Write("HeroesOfLiberty/Saved/Logs/HeroesOfLiberty.log", "log");

        Assert.Equal(Path.Combine(game.Root, "HeroesOfLiberty", "Saved", "Logs"), GameLogs.FindDir(game.Root, local.Root));
    }

    [Fact]
    public void ShippingLogsInLocalAppDataAreFound()
    {
        using var game = new TempDir();
        using var local = new TempDir();
        local.Write("HeroesOfLiberty/Saved/Logs/HeroesOfLiberty.log", "log");

        Assert.Equal(Path.Combine(local.Root, "HeroesOfLiberty", "Saved", "Logs"), GameLogs.FindDir(game.Root, local.Root));
    }

    [Fact]
    public void NoLogsYetGivesNullAndTheExpectedPlace()
    {
        using var game = new TempDir();
        using var local = new TempDir();

        Assert.Null(GameLogs.FindDir(game.Root, local.Root));
        Assert.Equal(Path.Combine(game.Root, "HeroesOfLiberty", "Saved", "Logs"), GameLogs.ExpectedDir(game.Root));
    }
}
