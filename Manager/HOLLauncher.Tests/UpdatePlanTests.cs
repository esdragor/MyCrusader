using HOLUpdate;

namespace HOLLauncher.Tests;

public class UpdatePlanTests
{
    private static readonly string HashA = new('a', 64);
    private static readonly string HashB = new('b', 64);
    private static readonly string HashC = new('c', 64);

    private static LatestVersion Latest(string version, IEnumerable<FileEntry> files, params string[] patchesFrom)
        => new(version, new VersionManifest(version, files), "", patchesFrom, "");

    [Fact]
    public void ChangedFileIsPatchedWhenInstalledVersionHasPatches()
    {
        var installed = new VersionManifest("v1", [new FileEntry("Game.exe", 10, HashA)]);
        var plan = UpdatePlan.Create(installed, Latest("v2", [new FileEntry("Game.exe", 12, HashB)], "v0", "v1"));

        var download = Assert.Single(plan.Downloads);
        Assert.Equal(DownloadKind.Patch, download.Kind);
        Assert.Equal("versions/v2/patches/v1/Game.exe", download.RelativeUrl);
        Assert.Equal(new FileEntry("Game.exe", 12, HashB), download.File);
        Assert.Equal("v2", plan.Version);
        Assert.Equal("v1", plan.Installed);
    }

    [Fact]
    public void ChangedFileIsFullWithoutPatchesFromInstalledVersion()
    {
        var installed = new VersionManifest("v1", [new FileEntry("Game.exe", 10, HashA)]);
        var plan = UpdatePlan.Create(installed, Latest("v2", [new FileEntry("Game.exe", 12, HashB)], "v0"));

        var download = Assert.Single(plan.Downloads);
        Assert.Equal(DownloadKind.Full, download.Kind);
        Assert.Equal("versions/v2/files/Game.exe", download.RelativeUrl);
    }

    [Fact]
    public void AddedFileIsAlwaysFull()
    {
        var installed = new VersionManifest("v1", [new FileEntry("Game.exe", 10, HashA)]);
        var plan = UpdatePlan.Create(installed,
            Latest("v2", [new FileEntry("Game.exe", 10, HashA), new FileEntry("Data/New.pak", 5, HashC)], "v1"));

        var download = Assert.Single(plan.Downloads);
        Assert.Equal(DownloadKind.Full, download.Kind);
        Assert.Equal("Data/New.pak", download.File.Path);
    }

    [Fact]
    public void RemovedFilesAreToDeleteAndUnchangedAreSkipped()
    {
        var installed = new VersionManifest("v1",
            [new FileEntry("Game.exe", 10, HashA), new FileEntry("Old.txt", 3, HashB)]);
        var plan = UpdatePlan.Create(installed, Latest("v2", [new FileEntry("Game.exe", 10, HashA)]), previousList: installed);

        Assert.Empty(plan.Downloads);
        Assert.Equal(["Old.txt"], plan.ToDelete.Select(f => f.Path));
    }

    [Fact]
    public void FirstInstallDownloadsEverythingFull()
    {
        var plan = UpdatePlan.Create(VersionManifest.Empty,
            Latest("v2", [new FileEntry("A.txt", 3, HashA), new FileEntry("B.txt", 4, HashB)], ""));

        Assert.All(plan.Downloads, d => Assert.Equal(DownloadKind.Full, d.Kind));
        Assert.Equal(2, plan.Downloads.Count);
        Assert.Null(plan.Installed);
    }

    [Fact]
    public void KnownBytesCountOnlyFullFiles()
    {
        var installed = new VersionManifest("v1", [new FileEntry("Game.exe", 10, HashA)]);
        var plan = UpdatePlan.Create(installed,
            Latest("v2", [new FileEntry("Game.exe", 100, HashB), new FileEntry("New.txt", 7, HashC)], "v1"));

        Assert.Equal(7, plan.FullBytes);
        Assert.Equal(1, plan.PatchCount);
    }

    [Fact]
    public void PathSegmentsAreEscaped()
    {
        var plan = UpdatePlan.Create(VersionManifest.Empty,
            Latest("v2", [new FileEntry("Content/Maps 1/a#b%c?.pak", 1, HashA)]));

        Assert.Equal("versions/v2/files/Content/Maps%201/a%23b%25c%3F.pak", Assert.Single(plan.Downloads).RelativeUrl);
    }

    [Fact]
    public void ScanGameIgnoresUpdateFolderAndSymbols()
    {
        using var game = new TempDir();
        game.Write("Version.txt", "v1");
        game.Write("Game.exe", "exe");
        game.Write("Game.pdb", "symbols");
        game.Write(".holupdate/v2/files/Game.exe", "new");

        var manifest = UpdatePlan.ScanGame(game.Root);

        Assert.Equal("v1", manifest.Version);
        Assert.Equal(["Game.exe", "Version.txt"], manifest.Files.Select(f => f.Path));
    }

    [Fact]
    public void ScanGameWithoutVersionIsEmpty()
    {
        using var game = new TempDir();
        game.Write("Stray.txt", "x");

        Assert.Same(VersionManifest.Empty, UpdatePlan.ScanGame(game.Root));
    }

    [Fact]
    public void NothingIsDeletedWithoutPreviousListAndOnlyListedFilesWithOne()
    {
        var installed = new VersionManifest("v1",
            [new FileEntry("Game.exe", 10, HashA), new FileEntry("Old.txt", 3, HashB), new FileEntry("Mine.txt", 3, HashC)]);
        var latest = Latest("v2", [new FileEntry("Game.exe", 10, HashA)]);
        var previous = new VersionManifest("v1", [new FileEntry("Game.exe", 10, HashA), new FileEntry("old.TXT", 3, HashB)]);

        Assert.Empty(UpdatePlan.Create(installed, latest).ToDelete);
        Assert.Equal(["Old.txt"], UpdatePlan.Create(installed, latest, previous).ToDelete.Select(f => f.Path));
    }

    [Fact]
    public void LauncherFilesAreLeftOutAndServersOnlyWhenMissing()
    {
        var latest = Latest("v2", [new FileEntry("Game.exe", 10, HashA), new FileEntry("HOLLauncher.exe", 4, HashB),
            new FileEntry("hpatchz.exe", 4, HashB), new FileEntry("servers.json", 5, HashC)]);
        var installed = new VersionManifest("v1", [new FileEntry("Game.exe", 10, HashA)]);

        Assert.Empty(UpdatePlan.Create(installed, latest, localServers: true).Downloads);
        var missing = UpdatePlan.Create(installed, latest, localServers: false);
        Assert.Equal(["servers.json"], missing.Downloads.Select(d => d.File.Path));
        Assert.Equal(["Game.exe"], missing.InstalledList.Files.Select(f => f.Path));
    }

    [Fact]
    public void ScanGameIgnoresLauncherFiles()
    {
        using var game = new TempDir();
        game.Write("Version.txt", "v1");
        game.Write("HOLLauncher.exe", "l");
        game.Write("HOLLauncher-old.exe", "l");
        game.Write("hpatchz.exe", "p");
        game.Write("servers.json", "{}");

        Assert.Equal(["Version.txt"], UpdatePlan.ScanGame(game.Root).Files.Select(f => f.Path));
    }

    [Fact]
    public void PatchCarriesTheFullFileUrl()
    {
        var installed = new VersionManifest("v1", [new FileEntry("Game.exe", 10, HashA)]);
        var plan = UpdatePlan.Create(installed, Latest("v2", [new FileEntry("Game.exe", 12, HashB)], "v1"));

        Assert.Equal("versions/v2/files/Game.exe", Assert.Single(plan.Downloads).FullUrl);
    }
}
