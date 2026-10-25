using System.Security.Cryptography;
using System.Text;

namespace HOLUpdate.Tests;

/** L1: the list of a player folder (relative path, size, SHA-256), built from real files. */
public class ManifestBuilderTests : IDisposable
{
    /** Get-FileHash -Algorithm SHA256 of a file holding the three bytes "abc" (lowercased). */
    private const string AbcSha256 = "ba7816bf8f01cfea414140de5dae2223b00361a396177a9cb410ff61f20015ad";

    private readonly TempDir dir = new();

    public void Dispose() => dir.Dispose();

    [Fact]
    public void EmptyFolderGivesEmptyList()
    {
        var manifest = ManifestBuilder.Build(dir.Root, "2026-10-25-3713f76");
        Assert.Equal("2026-10-25-3713f76", manifest.Version);
        Assert.Equal(VersionManifest.CurrentFormat, manifest.Format);
        Assert.Empty(manifest.Files);
    }

    [Fact]
    public void HashMatchesGetFileHash()
    {
        dir.Write("abc.txt", "abc");
        var entry = Assert.Single(ManifestBuilder.Build(dir.Root, "v").Files);
        Assert.Equal(new FileEntry("abc.txt", 3, AbcSha256), entry);
        Assert.Equal(AbcSha256, ManifestBuilder.Hash(Path.Combine(dir.Root, "abc.txt")));
    }

    [Fact]
    public void LargeFileIsHashedAcrossBuffers()
    {
        var content = new byte[3 * 1024 * 1024 + 17];
        new Random(42).NextBytes(content);
        var file = dir.Write("big.bin", content);
        var expected = Convert.ToHexString(SHA256.HashData(content)).ToLowerInvariant();
        Assert.Equal(expected, ManifestBuilder.Hash(file));
        Assert.Equal(content.Length, Assert.Single(ManifestBuilder.Build(dir.Root, "v").Files).Size);
    }

    [Fact]
    public void SubfoldersArePathsWithForwardSlashes()
    {
        dir.Write(@"HeroesOfLiberty\Content\Paks\pakchunk0.pak", "pak");
        dir.Write(@"Engine\Binaries\ThirdParty\x.dll", "dll");
        dir.Write("Version.txt", "v");
        var paths = ManifestBuilder.Build(dir.Root, "v").Files.Select(f => f.Path);
        Assert.Equal(["Engine/Binaries/ThirdParty/x.dll", "HeroesOfLiberty/Content/Paks/pakchunk0.pak", "Version.txt"], paths);
    }

    [Fact]
    public void SameContentSameHashOneByteChangedOtherHash()
    {
        var bytes = Encoding.UTF8.GetBytes("HeroesOfLiberty.exe, version one");
        var a = dir.Write("a.bin", bytes);
        var b = dir.Write("sub/b.bin", bytes);
        Assert.Equal(ManifestBuilder.Hash(a), ManifestBuilder.Hash(b));
        bytes[5] ^= 1;
        File.WriteAllBytes(b, bytes);
        Assert.NotEqual(ManifestBuilder.Hash(a), ManifestBuilder.Hash(b));
        Assert.Equal(new FileInfo(a).Length, new FileInfo(b).Length);
    }

    [Fact]
    public void PdbIsNeverListed()
    {
        dir.Write("HeroesOfLiberty.exe", "exe");
        dir.Write("HeroesOfLiberty.pdb", "symbols");
        dir.Write("Engine/Plugins/Some.PDB", "symbols");
        var entry = Assert.Single(ManifestBuilder.Build(dir.Root, "v").Files);
        Assert.Equal("HeroesOfLiberty.exe", entry.Path);
    }

    [Fact]
    public void ExclusionsAcceptFolderPatterns()
    {
        dir.Write("HeroesOfLiberty.exe", "exe");
        dir.Write("Saved/Logs/HeroesOfLiberty.log", "log");
        dir.Write("Manifest_DebugFiles_Win64.txt", "debug");
        var manifest = ManifestBuilder.Build(dir.Root, "v", [.. ManifestBuilder.DefaultExcludes, "Saved/*", "Manifest_Debug*"]);
        Assert.Equal(["HeroesOfLiberty.exe"], manifest.Files.Select(f => f.Path));
    }

    [Fact]
    public void OrderIsOrdinalWhateverTheCreationOrder()
    {
        foreach (var name in new[] { "b.txt", "Z/1.txt", "_x.txt", "a.txt", "C.txt", "a/z.txt", "a.b/c.txt" }) // no "B.txt": same file as "b.txt" on Windows
        {
            dir.Write(name, name);
        }
        var paths = ManifestBuilder.Build(dir.Root, "v").Files.Select(f => f.Path).ToList();
        Assert.Equal(["C.txt", "Z/1.txt", "_x.txt", "a.b/c.txt", "a.txt", "a/z.txt", "b.txt"], paths);
        Assert.Equal(ManifestBuilder.Build(dir.Root, "v").ToJson(), ManifestBuilder.Build(dir.Root, "v").ToJson());
    }

    [Fact]
    public void VersionIsReadFromVersionTxtWhenNotGiven()
    {
        dir.Write(ManifestBuilder.VersionFile, "2026-10-25-3713f76\r\n");
        var manifest = ManifestBuilder.Build(dir.Root);
        Assert.Equal("2026-10-25-3713f76", manifest.Version);
        Assert.Equal(ManifestBuilder.VersionFile, Assert.Single(manifest.Files).Path);
    }

    [Fact]
    public void MissingVersionTxtIsAnError()
    {
        Assert.Throws<FileNotFoundException>(() => ManifestBuilder.Build(dir.Root));
    }
}
