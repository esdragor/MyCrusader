using HOLUpdate;

namespace HOLLauncher.Tests;

public class StatusViewTests
{
    private static LauncherStatus Status(string? installed, string latest, string notes = "", string launcher = "")
        => LauncherStatus.Compute(installed,
            new LatestResult(LatestOutcome.Ok, new LatestVersion(latest, new VersionManifest(latest, []), notes, [], launcher), ""),
            "1");

    [Fact]
    public void UpToDate()
    {
        var view = StatusView.From(Status("v2", "v2", launcher: "1"));
        Assert.Equal("Version installée : v2", view.InstalledText);
        Assert.Equal("Dernière version : v2", view.LatestText);
        Assert.Equal("Le jeu est à jour.", view.StateText);
        Assert.Equal("Lanceur : version 1", view.LauncherText);
        Assert.False(view.UpdateGameEnabled);
        Assert.False(view.UpdateLauncherEnabled);
        Assert.True(view.RefreshEnabled);
    }

    [Fact]
    public void OutdatedEnablesGameUpdate()
    {
        var view = StatusView.From(Status("v1", "v2"));
        Assert.Equal("Version installée : v1", view.InstalledText);
        Assert.Equal("Une mise à jour du jeu est disponible (v2).", view.StateText);
        Assert.True(view.UpdateGameEnabled);
    }

    [Fact]
    public void NotInstalledEnablesGameUpdate()
    {
        var view = StatusView.From(Status(null, "v2"));
        Assert.Equal("Version installée : pas installé", view.InstalledText);
        Assert.Equal("Le jeu n'est pas installé ; la version v2 est disponible.", view.StateText);
        Assert.True(view.UpdateGameEnabled);
    }

    [Fact]
    public void UnreachableShowsReason()
    {
        var status = LauncherStatus.Compute("v1", new LatestResult(LatestOutcome.Unreachable, null, "connection refused"), "1");
        var view = StatusView.From(status);
        Assert.Equal("Version installée : v1", view.InstalledText);
        Assert.Equal("Dernière version : inconnue", view.LatestText);
        Assert.Equal("Serveur injoignable : connection refused", view.StateText);
        Assert.False(view.UpdateGameEnabled);
        Assert.False(view.UpdateLauncherEnabled);
        Assert.True(view.RefreshEnabled);
        Assert.False(view.NotesVisible);
    }

    [Fact]
    public void NotesShownOnlyWhenPresent()
    {
        Assert.False(StatusView.From(Status("v2", "v2")).NotesVisible);
        var view = StatusView.From(Status("v2", "v2", notes: "Archers plus rapides."));
        Assert.True(view.NotesVisible);
        Assert.Equal("Archers plus rapides.", view.Notes);
    }

    [Fact]
    public void NewerLauncherEnablesLauncherUpdate()
    {
        var view = StatusView.From(Status("v2", "v2", launcher: "2"));
        Assert.True(view.UpdateLauncherEnabled);
        Assert.Equal("Lanceur : version 1 (version 2 disponible)", view.LauncherText);
    }

    [Fact]
    public void PlayDisabledUntilL8()
    {
        Assert.False(StatusView.From(Status("v2", "v2")).PlayEnabled);
        Assert.False(StatusView.From(Status("v1", "v2")).PlayEnabled);
    }

    [Fact]
    public void LoadingDisablesEverything()
    {
        var view = StatusView.Loading("1");
        Assert.Equal("Lecture en cours…", view.StateText);
        Assert.False(view.PlayEnabled || view.UpdateGameEnabled || view.UpdateLauncherEnabled || view.RefreshEnabled);
        Assert.Equal("Lanceur : version 1", view.LauncherText);
        Assert.False(view.NotesVisible);
    }

    [Fact]
    public void ErrorKeepsRefresh()
    {
        var view = StatusView.Error("servers.json: bad JSON", "v1", "1");
        Assert.Equal("Erreur : servers.json: bad JSON", view.StateText);
        Assert.Equal("Version installée : v1", view.InstalledText);
        Assert.Equal("Dernière version : inconnue", view.LatestText);
        Assert.True(view.RefreshEnabled);
        Assert.False(view.UpdateGameEnabled || view.UpdateLauncherEnabled || view.PlayEnabled);
    }

    [Fact]
    public void UpdatingDisablesEveryButtonAndKeepsTheTexts()
    {
        var view = StatusView.Updating(StatusView.From(Status("v1", "v2", launcher: "2")));
        Assert.Equal("Version installée : v1", view.InstalledText);
        Assert.Equal("Une mise à jour du jeu est disponible (v2).", view.StateText);
        Assert.False(view.PlayEnabled || view.UpdateGameEnabled || view.UpdateLauncherEnabled || view.RefreshEnabled);
    }
}
