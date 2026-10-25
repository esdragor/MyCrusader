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
        game.Write("HeroesOfLiberty/Version.txt", "2026-10-25-b\r\n");
        Assert.Equal("2026-10-25-b", InstalledVersion.Read(game.Root));
    }

    [Fact]
    public void EmptyVersionFileMeansNotInstalled()
    {
        using var game = new TempDir();
        game.Write("HeroesOfLiberty/Version.txt", "  \n");
        Assert.Null(InstalledVersion.Read(game.Root));
    }

    /** The package's place (Tools/Package, HOLVersion.h): a Version.txt at the root (L6-L9 trials) is not read. */
    [Fact]
    public void VersionAtTheRootIsNotRead()
    {
        using var game = new TempDir();
        game.Write("Version.txt", "v1");
        game.Write("HeroesOfLiberty.exe", "exe");
        Assert.Null(InstalledVersion.Read(game.Root));
    }
}
