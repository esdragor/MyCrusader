using System.IO.Compression;
using System.Security.Cryptography;
using HOLManager.Versions;
using HOLUpdate;

namespace HOLPublish.Tests;

/**
 * L4a: fake game folders published one after the other with the real hdiffz, and the versions folder read back by the
 * manager's own VersionStore (what the launcher will ask): layout, .gz, patches checked by hpatchz, latest.txt, refusals,
 * a failure in the middle that publishes nothing, and pruning.
 */
public sealed class PublishTests : IDisposable
{
    private const string Exe = "HeroesOfLiberty/Binaries/Win64/HeroesOfLiberty.exe";
    private const string Dll = "Engine/Binaries/ThirdParty/x.dll";
    private const string Removed = "Data/old.txt";
    private const string Added = "Data/new.txt";

    /** A few MB that gzip cannot shrink, like the game exe: only a patch makes a small change cheap. */
    private static readonly byte[] BigFile = RandomBytes(4 * 1024 * 1024 + 123, 11);

    private readonly TempDir dir = new();

    public void Dispose() => dir.Dispose();

    private string Versions => Path.Combine(dir.Root, "versions");

    private static byte[] RandomBytes(int size, int seed)
    {
        var bytes = new byte[size];
        new Random(seed).NextBytes(bytes);
        return bytes;
    }

    private static string Sha256(byte[] bytes) => Convert.ToHexString(SHA256.HashData(bytes)).ToLowerInvariant();

    private static byte[] Gunzip(string file)
    {
        using var input = new GZipStream(File.OpenRead(file), CompressionMode.Decompress);
        using var output = new MemoryStream();
        input.CopyTo(output);
        return output.ToArray();
    }

    /**
     * A player folder Windows\: edit 0 holds Data/old.txt, the others Data/new.txt instead; each edit flips a few bytes
     * more of the big exe; the dll never changes; the .pdb always does (and is never published).
     */
    private string Game(string version, int edit)
    {
        var game = Path.Combine(dir.Root, "games", version);
        var big = (byte[])BigFile.Clone();
        for (var i = 1; i <= edit; i++)
        {
            big[1_000_000 * i + 7] ^= 0x5A;
        }
        dir.Write(Path.Combine("games", version, "Version.txt"), version);
        dir.Write(Path.Combine("games", version, Exe), big);
        dir.Write(Path.Combine("games", version, Dll), "the same dll in every version");
        dir.Write(Path.Combine("games", version, edit == 0 ? Removed : Added), $"text of {version}");
        dir.Write(Path.Combine("games", version, "HeroesOfLiberty.pdb"), RandomBytes(2000, edit));
        return game;
    }

    private PublishResult Publish(string game, string? version = null, string? notes = null, int keep = 3, string? hdiffz = null)
    {
        string? notesFile = null;
        if (notes != null)
        {
            notesFile = dir.Write(Path.Combine("notes", Guid.NewGuid().ToString("N") + ".txt"), notes);
        }
        return Publisher.Publish(new PublishOptions(game, Versions, version, notesFile, keep, hdiffz ?? HDiffPatchTool.Hdiffz));
    }

    private string[] VersionDirs() =>
        Directory.GetDirectories(Versions).Select(d => Path.GetFileName(d)!).Order(StringComparer.Ordinal).ToArray();

    [Fact]
    public void FirstPublicationIsReadByTheManagersVersionStore()
    {
        var game = Game("2026-10-25-a", 0);

        var result = Publish(game, notes: "Archers plus rapides.\r\n");

        Assert.Equal("2026-10-25-a", result.Version);
        Assert.Equal(4, result.FileCount);
        Assert.Equal(0, result.PatchCount);
        Assert.Equal("2026-10-25-a", File.ReadAllText(Path.Combine(Versions, "latest.txt")).Trim());
        Assert.Equal(["2026-10-25-a"], VersionDirs());

        var latest = new VersionStore(Versions).Latest();
        Assert.Equal("2026-10-25-a", latest.Version);
        Assert.Equal("Archers plus rapides.", latest.Notes);
        Assert.Empty(latest.PatchesFrom);
        var expected = ManifestBuilder.Build(game);
        Assert.Equal(expected.Files, latest.Manifest.Files);
        Assert.DoesNotContain(latest.Manifest.Files, f => f.Path.EndsWith(".pdb"));
        Assert.Equal(["Data/old.txt", Dll, Exe, "Version.txt"], latest.Manifest.Files.Select(f => f.Path));

        long compressed = 0;
        foreach (var file in latest.Manifest.Files)
        {
            var gz = new VersionStore(Versions).FilePath("2026-10-25-a", file.Path);
            Assert.Equal(file.Sha256, Sha256(Gunzip(gz)));
            compressed += new FileInfo(gz).Length;
        }
        Assert.Equal(compressed, result.CompressedBytes);
    }

    [Fact]
    public void SecondPublicationPatchesTheChangedFileOnly()
    {
        var gameA = Game("2026-10-25-a", 0);
        var gameB = Game("2026-10-25-b", 1);
        Publish(gameA, notes: "a");

        var result = Publish(gameB);

        var store = new VersionStore(Versions);
        var latest = store.Latest();
        Assert.Equal("2026-10-25-b", latest.Version);
        Assert.Equal("", latest.Notes);
        Assert.False(File.Exists(Path.Combine(Versions, "2026-10-25-b", "notes.txt")));
        Assert.Equal(["2026-10-25-a"], latest.PatchesFrom);
        Assert.Equal(["2026-10-25-a", "2026-10-25-b"], VersionDirs());

        var patch = store.PatchPath("2026-10-25-b", "2026-10-25-a", Exe);
        Assert.True(new FileInfo(patch).Length < new FileInfo(store.FilePath("2026-10-25-b", Exe)).Length / 100);
        var rebuilt = Path.Combine(dir.Root, "rebuilt.exe");
        HDiffPatchTool.Apply(Path.Combine(gameA, Exe), patch, rebuilt);
        Assert.Equal(latest.Manifest.Files.Single(f => f.Path == Exe).Sha256, ManifestBuilder.Hash(rebuilt));

        Assert.Equal(404, Assert.Throws<VersionException>(() => store.PatchPath("2026-10-25-b", "2026-10-25-a", Dll)).Status);
        Assert.Equal(404, Assert.Throws<VersionException>(() => store.PatchPath("2026-10-25-b", "2026-10-25-a", Added)).Status);
        Assert.Equal(404, Assert.Throws<VersionException>(() => store.FilePath("2026-10-25-b", Removed)).Status);
        Assert.Equal(Sha256(File.ReadAllBytes(Path.Combine(gameB, Added))), Sha256(Gunzip(store.FilePath("2026-10-25-b", Added))));

        // Every patch kept is smaller than the full .gz, and counted in the result.
        var patches = Directory.GetFiles(Path.Combine(Versions, "2026-10-25-b", "patches"), "*.patch", SearchOption.AllDirectories);
        Assert.Contains(patch, patches);
        Assert.Equal(patches.Length, result.PatchCount);
        Assert.Equal(patches.Sum(p => new FileInfo(p).Length), result.PatchBytes);
        foreach (var p in patches)
        {
            var path = Path.GetRelativePath(Path.Combine(Versions, "2026-10-25-b", "patches", "2026-10-25-a"), p)
                .Replace('\\', '/')[..^".patch".Length];
            Assert.True(new FileInfo(p).Length < new FileInfo(store.FilePath("2026-10-25-b", path)).Length, path);
        }
        Assert.Empty(Directory.GetDirectories(Versions, "*.tmp"));
    }

    [Fact]
    public void AlreadyPublishedVersionIsRefused()
    {
        var game = Game("2026-10-25-a", 0);
        Publish(game);
        var manifest = File.ReadAllText(Path.Combine(Versions, "2026-10-25-a", "manifest.json"));

        var e = Assert.Throws<PublishException>(() => Publish(Game("2026-10-25-a", 1)));

        Assert.Contains("already", e.Message);
        Assert.Equal(manifest, File.ReadAllText(Path.Combine(Versions, "2026-10-25-a", "manifest.json")));
        Assert.Equal(["2026-10-25-a"], VersionDirs());
    }

    [Fact]
    public void VersionOptionOverridesVersionTxt()
    {
        Publish(Game("2026-10-25-a", 0), version: "essai-1");

        var latest = new VersionStore(Versions).Latest();
        Assert.Equal("essai-1", latest.Version);
        Assert.Equal("essai-1", latest.Manifest.Version);
    }

    [Theory]
    [InlineData("a b")]
    [InlineData("..")]
    [InlineData("../x")]
    [InlineData("launcher")]
    [InlineData("v.tmp")]
    public void InvalidVersionNameIsRefused(string name)
    {
        Assert.Throws<PublishException>(() => Publish(Game("g", 0), version: name));
        Assert.False(Directory.Exists(Versions) && Directory.EnumerateFileSystemEntries(Versions).Any());
    }

    [Fact]
    public void FailureInTheMiddlePublishesNothing()
    {
        Publish(Game("2026-10-25-a", 0));
        var latest = Path.Combine(Versions, "latest.txt");
        var latestWritten = File.GetLastWriteTimeUtc(latest);

        var e = Assert.Throws<PublishException>(() =>
            Publish(Game("2026-10-25-b", 1), hdiffz: Path.Combine(dir.Root, "nowhere", "hdiffz.exe")));

        Assert.Contains("hdiffz", e.Message);
        Assert.Equal(["2026-10-25-a"], VersionDirs());
        Assert.Equal("2026-10-25-a", File.ReadAllText(latest).Trim());
        Assert.Equal(latestWritten, File.GetLastWriteTimeUtc(latest));
        Assert.Equal(["latest.txt"], Directory.GetFiles(Versions).Select(f => Path.GetFileName(f)));
    }

    [Fact]
    public void LeftoverTmpOfTheSameVersionIsReplaced()
    {
        dir.Write(Path.Combine("versions", "2026-10-25-a.tmp", "files", "junk.gz"), "half written");

        Publish(Game("2026-10-25-a", 0));

        Assert.Equal(["2026-10-25-a"], VersionDirs());
        Assert.False(File.Exists(Path.Combine(Versions, "2026-10-25-a", "files", "junk.gz")));
    }

    /** Names against their order: pruning and patches follow the publication order, not the names. */
    [Fact]
    public void KeepOneOnThreePublicationsPrunesTheOldest()
    {
        Publish(Game("c", 0), keep: 1);
        Publish(Game("b", 1), keep: 1);

        var result = Publish(Game("a", 2), keep: 1);

        Assert.Equal(["c"], result.Pruned);
        Assert.Equal(["a", "b"], VersionDirs());
        Assert.Equal(["a", "b"], VersionFolder.Published(Versions));
        var latest = new VersionStore(Versions).Latest();
        Assert.Equal("a", latest.Version);
        Assert.Equal(["b"], latest.PatchesFrom);
        var patch = new VersionStore(Versions).PatchPath("a", "b", Exe);
        var rebuilt = Path.Combine(dir.Root, "rebuilt.exe");
        HDiffPatchTool.Apply(Path.Combine(dir.Root, "games", "b", Exe), patch, rebuilt);
        Assert.Equal(latest.Manifest.Files.Single(f => f.Path == Exe).Sha256, ManifestBuilder.Hash(rebuilt));
    }

    [Fact]
    public void KeepZeroMakesNoPatchAndKeepsOnlyTheNewVersion()
    {
        Publish(Game("2026-10-25-a", 0));

        var result = Publish(Game("2026-10-25-b", 1), keep: 0, hdiffz: Path.Combine(dir.Root, "nowhere", "hdiffz.exe"));

        Assert.Equal(0, result.PatchCount);
        Assert.Equal(["2026-10-25-a"], result.Pruned);
        Assert.Equal(["2026-10-25-b"], VersionDirs());
    }
}
