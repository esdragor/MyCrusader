using HOLUpdate;

namespace HOLLauncher.Tests;

/** Launcher 5: the line under the big button, "V 0.4.3 · Lanceur 5 · Serveur" and its dot. */
public class FooterLineTests
{
    private static readonly ManagerServer Server = new("Salle", "http://hol.example.org:7700");

    private static StatusView View(string? installed, LatestOutcome outcome)
        => StatusView.From(LauncherStatus.Compute(installed, outcome == LatestOutcome.Ok
            ? new LatestResult(outcome, new LatestVersion("0.4.3", new VersionManifest("0.4.3", []), "", [], "5"), "")
            : new LatestResult(outcome, null, "refused"), "5", "fr"), "fr");

    [Fact]
    public void ReachedIsGreenWithTheServerInTheTooltip()
    {
        var line = FooterLine.From(View("0.4.3", LatestOutcome.Ok), Server, "5", "fr");
        Assert.Equal("V 0.4.3 · Lanceur 5 · Serveur", line.Text);
        Assert.Equal(ServerLink.Reached, line.Dot);
        Assert.Equal("Serveur joint : Salle (http://hol.example.org:7700)", line.Tooltip);
    }

    [Fact]
    public void UnreachableIsRed()
    {
        var line = FooterLine.From(View("0.4.3", LatestOutcome.Unreachable), new ManagerServer("", "http://127.0.0.1:7700"), "5", "en");
        Assert.Equal(ServerLink.Unreachable, line.Dot);
        Assert.Equal("Server unreachable: http://127.0.0.1:7700", line.Tooltip);
    }

    [Fact]
    public void ReadingIsUnknownAndNothingInstalledIsSaid()
    {
        var line = FooterLine.From(StatusView.Loading(language: "fr"), Server, "5", "fr");
        Assert.Equal(ServerLink.Unknown, line.Dot);
        Assert.Equal("Jeu non installé · Lanceur 5 · Serveur", line.Text);
        Assert.Equal("Serveur : Salle (http://hol.example.org:7700)", line.Tooltip);
    }

    [Fact]
    public void UnreadableServersJsonIsNamed()
    {
        var line = FooterLine.From(StatusView.Error("bad json", "0.4.3", language: "en"), null, "5", "en");
        Assert.Equal(ServerLink.Unreachable, line.Dot);
        Assert.Equal("Server unreachable: servers.json", line.Tooltip);
        Assert.Equal("V 0.4.3 · Launcher 5 · Server", line.Text);
    }

    [Fact]
    public void JapaneseLine()
    {
        Assert.Equal("V 0.4.3 · ランチャー 5 · サーバー", FooterLine.From(View("0.4.3", LatestOutcome.Ok), Server, "5", "ja").Text);
    }
}
