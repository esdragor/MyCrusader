namespace HOLLauncher.Tests;

public class InstalledVersionTests
{
    [Fact]
    public void NoVersionFileMeansNotInstalled()
    {
        using var game = new TempDir();
        Assert.Null(InstalledVersion.Read(game.Root));
    }

    [Fact]
    public void MissingGameFolderMeansNotInstalled()
    {
        using var game = new TempDir();
        Assert.Null(InstalledVersion.Read(Path.Combine(game.Root, "nothing")));
    }

    [Fact]
    public void VersionIsTrimmed()
    {
        using var game = new TempDir();
        game.Write("Version.txt", "2026-10-25-b\r\n");
        Assert.Equal("2026-10-25-b", InstalledVersion.Read(game.Root));
    }

    [Fact]
    public void EmptyVersionFileMeansNotInstalled()
    {
        using var game = new TempDir();
        game.Write("Version.txt", "  \n");
        Assert.Null(InstalledVersion.Read(game.Root));
    }
}
