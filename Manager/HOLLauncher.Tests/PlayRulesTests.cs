namespace HOLLauncher.Tests;

public class PlayRulesTests
{
    [Fact]
    public void UpToDatePlaysWithoutWarning()
    {
        var play = PlayRules.Decide(GameState.UpToDate, "v2");
        Assert.True(play.Enabled);
        Assert.Null(play.Confirmation);
        Assert.Equal("", play.StateWarning);
    }

    [Fact]
    public void OutdatedPlaysAfterConfirmation()
    {
        var play = PlayRules.Decide(GameState.Outdated, "v1");
        Assert.True(play.Enabled);
        Assert.Equal("Le jeu n'est pas à jour : le serveur refusera cette version. Jouer quand même (en solo) ?",
            play.Confirmation);
        Assert.Equal("", play.StateWarning);
    }

    [Fact]
    public void UnreachableButInstalledPlaysSoloWithAShortWarning()
    {
        var play = PlayRules.Decide(GameState.Unreachable, "v1");
        Assert.True(play.Enabled);
        Assert.Null(play.Confirmation);
        Assert.Equal("Vous pouvez jouer en solo.", play.StateWarning);
    }

    [Fact]
    public void NotInstalledCannotPlay()
    {
        Assert.False(PlayRules.Decide(GameState.NotInstalled, null).Enabled);
        Assert.False(PlayRules.Decide(GameState.Unreachable, null).Enabled);
        Assert.Equal("", PlayRules.Decide(GameState.Unreachable, null).StateWarning);
    }

    [Fact]
    public void NoPlayDuringAnUpdate()
    {
        foreach (var state in Enum.GetValues<GameState>())
        {
            Assert.False(PlayRules.Decide(state, "v1", updating: true).Enabled);
        }
    }
}
