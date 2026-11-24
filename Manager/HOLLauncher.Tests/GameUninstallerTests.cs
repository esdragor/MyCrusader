using System.Security.Cryptography;
using System.Text;
using HOLUpdate;

namespace HOLLauncher.Tests;

/** L17: "Uninstall" deletes the game's files only; the launcher's and the player's files stay. */
public class GameUninstallerTests
{
    private static FileEntry Entry(string path, string content)
    {
        var bytes = Encoding.UTF8.GetBytes(content);
        return new FileEntry(path, bytes.Length, Convert.ToHexString(SHA256.HashData(bytes)).ToLowerInvariant());
    }

    private static readonly (string Path, string Content)[] GameFiles =
    [
        ("HeroesOfLiberty/Version.txt", "v2"),
        ("HeroesOfLiberty.exe", "exe"),
        ("HeroesOfLiberty/Content/Paks/Game.pak", "pak"),
        ("Engine/Binaries/ThirdParty/Lib.dll", "dll"),
    ];

    private static readonly string[] Kept =
    [
        "HOLLauncher.exe", "hpatchz.exe", "servers.json", LauncherSettings.FileName, "Stray.txt",
        "HeroesOfLiberty/Saved/Logs/HeroesOfLiberty.log", "HeroesOfLiberty/Saved/Config/Game.ini",
    ];

    /** A game installed by the launcher (installed.json, a leftover work folder) unless fromZip. */
    private static TempDir Installed(bool fromZip = false)
    {
        var game = new TempDir();
        foreach (var (path, content) in GameFiles)
        {
            game.Write(path, content);
        }
        foreach (var path in Kept)
        {
            game.Write(path, path);
        }
        if (!fromZip)
        {
            new VersionManifest("v2", GameFiles.Select(f => Entry(f.Path, f.Content))).Save(Path.Combine(
                Directory.CreateDirectory(Path.Combine(game.Root, UpdatePlan.WorkDirName)).FullName, UpdatePlan.InstalledListName));
            game.Write(".holupdate/v3/files/HeroesOfLiberty.exe.gz.part", "half a download");
        }
        return game;
    }

    private static bool Exists(TempDir game, string path) => File.Exists(Path.Combine(game.Root, path));

    private static bool FolderExists(TempDir game, string path) => Directory.Exists(Path.Combine(game.Root, path));

    [Fact]
    public void GameFilesAreDeletedAndLauncherAndPlayerFilesKept()
    {
        using var game = Installed();

        var result = GameUninstaller.Uninstall(game.Root);

        Assert.True(result.Success, result.Message);
        Assert.Equal(Texts.Format("UninstallDone", 4), result.Message);
        Assert.All(GameFiles, f => Assert.False(Exists(game, f.Path), f.Path));
        Assert.All(Kept, path => Assert.True(Exists(game, path), path));
        Assert.False(FolderExists(game, UpdatePlan.WorkDirName));
        Assert.Null(InstalledVersion.Read(game.Root));
    }

    [Fact]
    public void EmptiedFoldersAreRemovedOthersStay()
    {
        using var game = Installed();
        Directory.CreateDirectory(Path.Combine(game.Root, "MyEmptyFolder"));

        var result = GameUninstaller.Uninstall(game.Root);

        Assert.True(result.Success, result.Message);
        Assert.False(FolderExists(game, "Engine"));
        Assert.False(FolderExists(game, "HeroesOfLiberty/Content"));
        Assert.True(FolderExists(game, "HeroesOfLiberty/Saved"));
        Assert.True(FolderExists(game, "MyEmptyFolder"));
        Assert.True(Directory.Exists(game.Root));
    }

    [Fact]
    public void LauncherFilesAreKeptEvenWhenTheListNamesThem()
    {
        using var game = Installed();
        new VersionManifest("v2", [.. GameFiles.Select(f => Entry(f.Path, f.Content)),
                Entry("HOLLauncher.exe", "x"), Entry("hpatchz.exe", "x"), Entry("servers.json", "x"), Entry(LauncherSettings.FileName, "x")])
            .Save(UpdatePlan.InstalledListPath(game.Root));

        var result = GameUninstaller.Uninstall(game.Root);

        Assert.True(result.Success, result.Message);
        Assert.All(Kept, path => Assert.True(Exists(game, path), path));
    }

    [Fact]
    public void ZipInstallUsesTheGivenListAndTheVersionFile()
    {
        using var game = Installed(fromZip: true);
        // The server's list of another version: its paths are still the game's.
        var latest = new VersionManifest("v3", [Entry("HeroesOfLiberty.exe", "new exe"), Entry("Engine/Binaries/ThirdParty/Lib.dll", "new"),
            Entry("HeroesOfLiberty/Content/Paks/Game.pak", "new"), Entry("servers.json", "published")]);

        var result = GameUninstaller.Uninstall(game.Root, latest);

        Assert.True(result.Success, result.Message);
        Assert.All(GameFiles, f => Assert.False(Exists(game, f.Path), f.Path));
        Assert.All(Kept, path => Assert.True(Exists(game, path), path));
    }

    [Fact]
    public void ZipInstallWithoutAnyListDeletesNothing()
    {
        using var game = Installed(fromZip: true);

        var result = GameUninstaller.Uninstall(game.Root);

        Assert.False(result.Success);
        Assert.Equal(Texts.Get("UninstallNoList"), result.Message);
        Assert.All(GameFiles, f => Assert.True(Exists(game, f.Path), f.Path));
        Assert.True(GameUninstaller.NeedsServerList(game.Root));
    }

    [Fact]
    public void NothingInstalledIsSaid()
    {
        using var game = new TempDir();
        game.Write("HOLLauncher.exe", "kit");

        var result = GameUninstaller.Uninstall(game.Root);

        Assert.False(result.Success);
        Assert.Equal(Texts.Format("UninstallNothing", game.Root), result.Message);
        Assert.True(Exists(game, "HOLLauncher.exe"));
    }

    [Fact]
    public void FileInUseDeletesNothing()
    {
        using var game = Installed();
        UninstallResult result;
        using (new FileStream(Path.Combine(game.Root, "HeroesOfLiberty.exe"), FileMode.Open, FileAccess.Read, FileShare.Read))
        {
            result = GameUninstaller.Uninstall(game.Root);
        }

        Assert.False(result.Success);
        Assert.Equal(Texts.Format("UninstallLocked", "HeroesOfLiberty.exe"), result.Message);
        Assert.All(GameFiles, f => Assert.True(Exists(game, f.Path), f.Path));
        Assert.True(File.Exists(UpdatePlan.InstalledListPath(game.Root)));
    }

    [Fact]
    public void ReadOnlyAndAlreadyMissingFilesAreFine()
    {
        using var game = Installed();
        File.Delete(Path.Combine(game.Root, "Engine", "Binaries", "ThirdParty", "Lib.dll"));
        File.SetAttributes(Path.Combine(game.Root, "HeroesOfLiberty.exe"), FileAttributes.ReadOnly);

        var result = GameUninstaller.Uninstall(game.Root);

        Assert.True(result.Success, result.Message);
        Assert.Equal(Texts.Format("UninstallDone", 3), result.Message);
        Assert.False(Exists(game, "HeroesOfLiberty.exe"));
        Assert.False(FolderExists(game, "Engine"));
    }
}
