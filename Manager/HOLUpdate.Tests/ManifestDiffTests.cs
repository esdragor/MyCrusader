namespace HOLUpdate.Tests;

/** L1: what the launcher downloads and deletes between the installed list and the target one (both built from disk). */
public class ManifestDiffTests : IDisposable
{
    private readonly TempDir installedDir = new();
    private readonly TempDir targetDir = new();

    public void Dispose()
    {
        installedDir.Dispose();
        targetDir.Dispose();
    }

    private ManifestDiff Compare() =>
        ManifestDiff.Compare(ManifestBuilder.Build(installedDir.Root, "old"), ManifestBuilder.Build(targetDir.Root, "new"));

    private static IEnumerable<string> Paths(IEnumerable<FileEntry> entries) => entries.Select(f => f.Path);

    [Fact]
    public void AddedChangedRemovedUnchanged()
    {
        installedDir.Write("Same.dll", "same");
        installedDir.Write("HeroesOfLiberty.exe", "exe v1");
        installedDir.Write("Old/Gone.txt", "gone");
        targetDir.Write("Same.dll", "same");
        targetDir.Write("HeroesOfLiberty.exe", "exe v2, longer");
        targetDir.Write("New/Added.txt", "added");

        var diff = Compare();
        Assert.Equal(["New/Added.txt"], Paths(diff.Added));
        Assert.Equal(["HeroesOfLiberty.exe"], Paths(diff.Changed));
        Assert.Equal(["HeroesOfLiberty.exe", "New/Added.txt"], Paths(diff.ToDownload));
        Assert.Equal(["Old/Gone.txt"], Paths(diff.ToDelete));
        Assert.Equal(["Same.dll"], Paths(diff.Unchanged));
        Assert.Equal("exe v2, longer".Length + "added".Length, diff.DownloadBytes);
    }

    [Fact]
    public void SameSizeOtherContentIsChanged()
    {
        installedDir.Write("Version.txt", "2026-10-24-aaaaaaa");
        targetDir.Write("Version.txt", "2026-10-25-bbbbbbb");
        var diff = Compare();
        var changed = Assert.Single(diff.Changed);
        Assert.Equal(18, changed.Size);
        Assert.Empty(diff.ToDelete);
        Assert.Empty(diff.Unchanged);
    }

    [Fact]
    public void IdenticalFoldersDownloadNothing()
    {
        foreach (var d in new[] { installedDir, targetDir })
        {
            d.Write("a.dll", "a");
            d.Write("sub/b.dll", "b");
        }
        var diff = Compare();
        Assert.Empty(diff.ToDownload);
        Assert.Empty(diff.ToDelete);
        Assert.Equal(["a.dll", "sub/b.dll"], Paths(diff.Unchanged));
        Assert.Equal(0, diff.DownloadBytes);
    }

    [Fact]
    public void FirstInstallDownloadsEverything()
    {
        targetDir.Write("HeroesOfLiberty.exe", "exe");
        targetDir.Write("Engine/x.dll", "dll!");
        var target = ManifestBuilder.Build(targetDir.Root, "new");
        foreach (var installed in new[] { VersionManifest.Empty, ManifestBuilder.Build(installedDir.Root, "") })
        {
            var diff = ManifestDiff.Compare(installed, target);
            Assert.Equal(target.Files, diff.ToDownload);
            Assert.Equal(target.Files, diff.Added);
            Assert.Empty(diff.ToDelete);
            Assert.Equal(7, diff.DownloadBytes);
        }
    }

    [Fact]
    public void OtherCaseOfTheSamePathIsTheSameFile()
    {
        installedDir.Write("Engine/Binaries/X.dll", "x");
        installedDir.Write("Data/Table.csv", "v1");
        targetDir.Write("engine/binaries/x.DLL", "x");
        targetDir.Write("data/table.CSV", "v2");

        var diff = Compare();
        Assert.Equal(["engine/binaries/x.DLL"], Paths(diff.Unchanged));
        Assert.Equal(["data/table.CSV"], Paths(diff.Changed));
        Assert.Empty(diff.Added);
        Assert.Empty(diff.ToDelete);
    }
}
