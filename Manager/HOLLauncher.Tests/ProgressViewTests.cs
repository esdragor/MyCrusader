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
}
