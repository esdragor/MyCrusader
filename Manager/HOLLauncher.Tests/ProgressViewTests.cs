namespace HOLLauncher.Tests;

public class ProgressViewTests
{
    [Fact]
    public void StartingAllowsCancel()
    {
        var view = ProgressView.Starting();
        Assert.Equal("Préparation de la mise à jour…", view.Text);
        Assert.Equal(0, view.Percent);
        Assert.True(view.CancelEnabled);
    }

    [Fact]
    public void FileBeingFetchedIsNumberedFromOne()
    {
        var view = ProgressView.From(new DownloadProgress(2, 5, "HeroesOfLiberty.exe", 13_002_342));
        Assert.Equal("Fichier 3/5 : HeroesOfLiberty.exe — 12,4 Mo", view.Text);
        Assert.Equal(40, view.Percent);
        Assert.True(view.CancelEnabled);
    }

    [Fact]
    public void LastFileDoneDoesNotGoPastTheCount()
    {
        var view = ProgressView.From(new DownloadProgress(5, 5, "Data/Map.pak", 1_048_576));
        Assert.Equal("Fichier 5/5 : Data/Map.pak — 1,0 Mo", view.Text);
        Assert.Equal(100, view.Percent);
    }

    [Fact]
    public void SmallAmountsAreShownInMegabytesToo()
    {
        Assert.EndsWith("— 0,0 Mo", ProgressView.From(new DownloadProgress(0, 1, "Version.txt", 2)).Text);
    }

    [Fact]
    public void NothingToFetchIsFull()
    {
        Assert.Equal(100, ProgressView.From(new DownloadProgress(0, 0, "", 0)).Percent);
    }

    [Fact]
    public void CancellingKeepsTheBarAndDisablesCancel()
    {
        var view = ProgressView.From(new DownloadProgress(1, 4, "a", 0)).Cancelling();
        Assert.Equal("Annulation…", view.Text);
        Assert.Equal(25, view.Percent);
        Assert.False(view.CancelEnabled);
    }

    [Fact]
    public void RunningUpdateCanBePaused()
    {
        var view = ProgressView.From(new DownloadProgress(1, 4, "a", 0));
        Assert.False(view.IsPaused);
        Assert.True(view.PauseEnabled);
        Assert.Equal("Pause", view.PauseText);
        Assert.True(ProgressView.Starting().PauseEnabled);
        Assert.True(ProgressView.FromLauncher(new LauncherProgress("2", 0, null)).PauseEnabled);
    }

    [Fact]
    public void PausingKeepsTheBarAndDisablesBothButtons()
    {
        var view = ProgressView.From(new DownloadProgress(1, 4, "a", 0)).Pausing();
        Assert.Equal("Mise en pause…", view.Text);
        Assert.Equal(25, view.Percent);
        Assert.False(view.CancelEnabled);
        Assert.False(view.PauseEnabled);
        Assert.False(view.IsPaused);
    }

    [Fact]
    public void PausedKeepsTheLineAndOffersResumeAndCancel()
    {
        // The line is the one shown when "Pause" was clicked (its own wording is L16's).
        var line = ProgressView.From(new DownloadProgress(2, 5, "HeroesOfLiberty.exe", 13_002_342));
        var view = line.Paused();
        Assert.Equal($"En pause ({line.Text}). Ce qui est déjà reçu est gardé : "
            + "« Reprendre » continue sans le télécharger de nouveau.", view.Text);
        Assert.Equal(40, view.Percent);
        Assert.True(view.IsPaused);
        Assert.True(view.CancelEnabled);
        Assert.True(view.PauseEnabled);
        Assert.Equal("Reprendre", view.PauseText);
    }

    [Fact]
    public void CancellingDisablesPauseToo()
    {
        Assert.False(ProgressView.From(new DownloadProgress(1, 4, "a", 0)).Cancelling().PauseEnabled);
    }

    [Fact]
    public void LauncherDownloadShowsMegabytesOfTheTotal()
    {
        var view = ProgressView.FromLauncher(new LauncherProgress("2", 13_002_342, 75_183_104));
        Assert.Equal("Lanceur version 2 : 12,4 / 71,7 Mo", view.Text);
        Assert.Equal(17, view.Percent);
        Assert.True(view.CancelEnabled);
    }

    [Fact]
    public void LauncherDownloadWithoutTotalStaysAtZero()
    {
        var view = ProgressView.FromLauncher(new LauncherProgress("2", 1_048_576, null));
        Assert.Equal("Lanceur version 2 : 1,0 Mo", view.Text);
        Assert.Equal(0, view.Percent);
    }
}
