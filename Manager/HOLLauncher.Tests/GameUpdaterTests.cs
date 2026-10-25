using System.IO.Compression;
using System.Net;
using System.Security.Cryptography;
using System.Text;
using HOLUpdate;

namespace HOLLauncher.Tests;

public class GameUpdaterTests
{
    private const string Manager = "http://host:7700";

    private static readonly byte[] OldExe = Content("exe", 1, 20000);
    private static readonly byte[] NewExe = Content("exe", 2, 20000);

    /** Mostly shared bytes with a seed-dependent part, so hdiffz gives a real, small patch. */
    private static byte[] Content(string name, int seed, int size)
    {
        var bytes = new byte[size];
        new Random(42).NextBytes(bytes);
        var tail = Encoding.UTF8.GetBytes($"{name} version {seed}");
        tail.CopyTo(bytes, size / 2);
        return bytes;
    }

    private static byte[] Text(string text) => Encoding.UTF8.GetBytes(text);

    private static FileEntry Entry(string path, byte[] content)
        => new(path, content.Length, Convert.ToHexString(SHA256.HashData(content)).ToLowerInvariant());

    private static byte[] Gzip(byte[] content)
    {
        using var memory = new MemoryStream();
        using (var gzip = new GZipStream(memory, CompressionLevel.Optimal))
        {
            gzip.Write(content);
        }
        return memory.ToArray();
    }

    /** A published v2 on a fake manager: full files gzipped, patches from v1; keeps the requested paths. */
    private sealed class Published
    {
        public Dictionary<string, byte[]> Files { get; } = new(StringComparer.Ordinal)
        {
            ["HeroesOfLiberty/Version.txt"] = Text("v2"),
            ["HeroesOfLiberty.exe"] = NewExe,
            ["Data/New.pak"] = Text("new pak"),
            ["servers.json"] = Text("""{ "managers": [ { "url": "http://127.0.0.1:7700" } ] }"""),
            ["HOLLauncher.exe"] = Text("launcher v2"),
        };

        public Dictionary<string, byte[]> Patches { get; } = [];
        public List<string> Requests { get; } = [];

        public LatestVersion Latest => new("v2", new VersionManifest("v2", Files.Select(f => Entry(f.Key, f.Value))), "", ["v1"], "");

        public HttpClient Client() => new(new FakeHandler(Answer));

        private HttpResponseMessage Answer(HttpRequestMessage request)
        {
            var path = Uri.UnescapeDataString(request.RequestUri!.AbsolutePath);
            Requests.Add(path);
            byte[]? body = null;
            if (path.StartsWith("/versions/v2/files/") && Files.TryGetValue(path["/versions/v2/files/".Length..], out var file))
            {
                body = Gzip(file);
            }
            else if (path.StartsWith("/versions/v2/patches/v1/"))
            {
                Patches.TryGetValue(path["/versions/v2/patches/v1/".Length..], out body);
            }
            if (body is null)
            {
                var error = path.Contains("/patches/") ? "patch_not_found" : "file_not_found";
                return new HttpResponseMessage(HttpStatusCode.NotFound)
                {
                    Content = new StringContent($$"""{ "error": "{{error}}", "message": "no" }""", Encoding.UTF8, "application/json"),
                };
            }
            return new HttpResponseMessage(HttpStatusCode.OK) { Content = new ByteArrayContent(body) };
        }
    }

    /** An installed v1, from a launcher update (installed.json) unless fromZip. */
    private static TempDir InstalledGame(bool fromZip = false)
    {
        var game = new TempDir();
        game.Write("HeroesOfLiberty/Version.txt", "v1");
        game.Write("HeroesOfLiberty.exe", OldExe);
        game.Write("Data/Old.pak", "old pak");
        game.Write("Stray.txt", "player's file");
        game.Write("HOLLauncher.exe", "launcher v1");
        game.Write("hpatchz.exe", "patcher");
        game.Write("servers.json", """{ "managers": [ { "url": "http://friend:7700" } ] }""");
        if (!fromZip)
        {
            var list = new VersionManifest("v1",
                [Entry("HeroesOfLiberty/Version.txt", Text("v1")), Entry("HeroesOfLiberty.exe", OldExe), Entry("Data/Old.pak", Text("old pak"))]);
            Directory.CreateDirectory(Path.Combine(game.Root, UpdatePlan.WorkDirName));
            list.Save(UpdatePlan.InstalledListPath(game.Root));
        }
        return game;
    }

    private static Published WithPatch()
    {
        var published = new Published();
        published.Patches["HeroesOfLiberty.exe"] = HDiffPatchTool.Diff(OldExe, NewExe);
        return published;
    }

    private static Task<UpdateResult> Update(TempDir game, Published published, string? hpatchz = null,
        Func<string, long>? freeSpace = null)
        => new GameUpdater(published.Client(), hpatchz ?? HDiffPatchTool.Hpatchz, freeSpace ?? (_ => long.MaxValue))
            .UpdateAsync(game.Root, Manager, published.Latest);

    /** A friend's first install (L9): only the kit (launcher, patcher, servers.json aimed at the user), no Version.txt. */
    private static TempDir KitOnly()
    {
        var game = new TempDir();
        game.Write("HOLLauncher.exe", "kit launcher");
        game.Write("hpatchz.exe", "patcher");
        game.Write("servers.json", """{ "servers": [ { "name": "Ami", "url": "http://friend:7700" } ] }""");
        return game;
    }

    /** The bytes a full install of Published needs: the final sizes of what it downloads (servers.json is the kit's). */
    private static long FinalBytes(Published published)
        => published.Files.Where(f => !UpdatePlan.IsLauncherOwned(f.Key)).Sum(f => (long)f.Value.Length);

    private static string Read(TempDir game, string path) => File.ReadAllText(Path.Combine(game.Root, path));

    private static bool Exists(TempDir game, string path) => File.Exists(Path.Combine(game.Root, path));

    [Fact]
    public async Task PatchIsAppliedAndFilesAreReplaced()
    {
        using var game = InstalledGame();
        var published = WithPatch();

        var result = await Update(game, published);

        Assert.True(result.Success, result.Message);
        Assert.Equal(NewExe, File.ReadAllBytes(Path.Combine(game.Root, "HeroesOfLiberty.exe")));
        Assert.Contains("/versions/v2/patches/v1/HeroesOfLiberty.exe", published.Requests);
        Assert.DoesNotContain("/versions/v2/files/HeroesOfLiberty.exe", published.Requests);
        Assert.Equal("v2", Read(game, "HeroesOfLiberty/Version.txt"));
        Assert.False(Directory.Exists(FileDownloader.TempDirFor(game.Root, "v2")));
    }

    [Fact]
    public async Task CorruptPatchFallsBackToFullFile()
    {
        using var game = InstalledGame();
        var published = new Published();
        published.Patches["HeroesOfLiberty.exe"] = Text("not a patch at all");

        var result = await Update(game, published);

        Assert.True(result.Success, result.Message);
        Assert.Equal(NewExe, File.ReadAllBytes(Path.Combine(game.Root, "HeroesOfLiberty.exe")));
        Assert.Contains("/versions/v2/files/HeroesOfLiberty.exe", published.Requests);
    }

    [Fact]
    public async Task FullFileAfterAFailedPatchReportsItsProgress()
    {
        using var game = InstalledGame();
        var published = new Published();
        published.Patches["HeroesOfLiberty.exe"] = Text("not a patch at all");
        var reports = new List<DownloadProgress>();

        var result = await new GameUpdater(published.Client(), HDiffPatchTool.Hpatchz)
            .UpdateAsync(game.Root, Manager, published.Latest, new ListProgress(reports));

        Assert.True(result.Success, result.Message);
        // The end of the downloads is the first report with every file done; the full file comes after it.
        var downloadsDone = reports.FindIndex(r => r.FilesDone == r.FileCount);
        var fallback = reports.Skip(downloadsDone + 1).ToList();
        Assert.NotEmpty(fallback);
        Assert.All(fallback, r => Assert.Equal(("HeroesOfLiberty.exe", 3, 3), (r.CurrentPath, r.FilesDone, r.FileCount)));
        Assert.Equal(reports[downloadsDone].BytesReceived + Gzip(NewExe).Length, fallback[^1].BytesReceived);
    }

    /** Collects reports as they come (Progress<T> would post them later). */
    private sealed class ListProgress(List<DownloadProgress> reports) : IProgress<DownloadProgress>
    {
        public void Report(DownloadProgress value) => reports.Add(value);
    }

    [Fact]
    public async Task MissingHpatchzFallsBackToFullFile()
    {
        using var game = InstalledGame();
        var published = WithPatch();

        var result = await Update(game, published, hpatchz: Path.Combine(game.Root, "nowhere", "hpatchz.exe"));

        Assert.True(result.Success, result.Message);
        Assert.Equal(NewExe, File.ReadAllBytes(Path.Combine(game.Root, "HeroesOfLiberty.exe")));
        Assert.Contains("/versions/v2/files/HeroesOfLiberty.exe", published.Requests);
    }

    [Fact]
    public async Task AddedFileIsInstalled()
    {
        using var game = InstalledGame();

        var result = await Update(game, WithPatch());

        Assert.True(result.Success, result.Message);
        Assert.Equal("new pak", Read(game, "Data/New.pak"));
    }

    [Fact]
    public async Task RemovedFileListedInInstalledJsonIsDeleted()
    {
        using var game = InstalledGame();

        var result = await Update(game, WithPatch());

        Assert.True(result.Success, result.Message);
        Assert.False(Exists(game, "Data/Old.pak"));
    }

    [Fact]
    public async Task WithoutInstalledJsonNothingIsDeleted()
    {
        using var game = InstalledGame(fromZip: true);

        var result = await Update(game, WithPatch());

        Assert.True(result.Success, result.Message);
        Assert.Equal("old pak", Read(game, "Data/Old.pak"));
    }

    [Fact]
    public async Task UnknownLauncherAndServersFilesAreKept()
    {
        using var game = InstalledGame();
        var published = WithPatch();

        var result = await Update(game, published);

        Assert.True(result.Success, result.Message);
        Assert.Equal("player's file", Read(game, "Stray.txt"));
        Assert.Equal("launcher v1", Read(game, "HOLLauncher.exe"));
        Assert.Equal("patcher", Read(game, "hpatchz.exe"));
        Assert.Contains("friend", Read(game, "servers.json"));
        Assert.DoesNotContain("/versions/v2/files/servers.json", published.Requests);
        Assert.DoesNotContain("/versions/v2/files/HOLLauncher.exe", published.Requests);
    }

    [Fact]
    public async Task MissingServersJsonIsInstalled()
    {
        using var game = InstalledGame();
        File.Delete(Path.Combine(game.Root, "servers.json"));

        var result = await Update(game, WithPatch());

        Assert.True(result.Success, result.Message);
        Assert.Contains("127.0.0.1", Read(game, "servers.json"));
    }

    [Fact]
    public async Task InstalledJsonIsWrittenWithoutLauncherFiles()
    {
        using var game = InstalledGame();

        var result = await Update(game, WithPatch());

        Assert.True(result.Success, result.Message);
        var list = VersionManifest.Load(UpdatePlan.InstalledListPath(game.Root));
        Assert.Equal("v2", list.Version);
        Assert.Equal(["Data/New.pak", "HeroesOfLiberty.exe", "HeroesOfLiberty/Version.txt"], list.Files.Select(f => f.Path));
    }

    [Fact]
    public async Task FailureInTheMiddleKeepsOldVersionTxt()
    {
        using var game = InstalledGame();
        game.Write("Sub", "a file where the new version wants a folder");
        var published = WithPatch();
        published.Files["Sub/C.txt"] = Text("c");

        var result = await Update(game, published);

        Assert.False(result.Success);
        Assert.Equal("v1", Read(game, "HeroesOfLiberty/Version.txt"));
        Assert.Equal("v1", VersionManifest.Load(UpdatePlan.InstalledListPath(game.Root)).Version);
    }

    [Fact]
    public async Task LockedFileGivesErrorAndReplacesNothing()
    {
        using var game = InstalledGame();
        var published = WithPatch();
        UpdateResult result;
        using (new FileStream(Path.Combine(game.Root, "HeroesOfLiberty.exe"), FileMode.Open, FileAccess.Read, FileShare.Read))
        {
            result = await Update(game, published);
        }

        Assert.False(result.Success);
        Assert.Contains("fermez le jeu", result.Message, StringComparison.OrdinalIgnoreCase);
        Assert.Equal(OldExe, File.ReadAllBytes(Path.Combine(game.Root, "HeroesOfLiberty.exe")));
        Assert.False(Exists(game, "Data/New.pak"));
        Assert.True(Exists(game, "Data/Old.pak"));
        Assert.Equal("v1", Read(game, "HeroesOfLiberty/Version.txt"));
    }

    [Fact]
    public async Task LauncherUpdateFilesAreNeverDeleted()
    {
        using var game = InstalledGame();
        string[] names = ["HOLLauncher.new.exe", "HOLLauncher.new.exe.part", "HOLLauncher.old.exe"];
        foreach (var name in names)
        {
            game.Write(name, name);
        }
        // Worst case: an installed.json that lists them (hand-edited, older launcher).
        var list = VersionManifest.Load(UpdatePlan.InstalledListPath(game.Root));
        new VersionManifest("v1", [.. list.Files, .. names.Select(n => Entry(n, Text(n)))]).Save(UpdatePlan.InstalledListPath(game.Root));

        var result = await Update(game, WithPatch());

        Assert.True(result.Success, result.Message);
        Assert.All(names, name => Assert.Equal(name, Read(game, name)));
    }

    [Fact]
    public async Task FirstInstallInKitFolderDownloadsEverythingInFull()
    {
        using var game = KitOnly();
        var published = WithPatch();

        var result = await Update(game, published);

        Assert.True(result.Success, result.Message);
        Assert.Equal("v2", Read(game, "HeroesOfLiberty/Version.txt"));
        Assert.Equal(NewExe, File.ReadAllBytes(Path.Combine(game.Root, "HeroesOfLiberty.exe")));
        Assert.Equal("new pak", Read(game, "Data/New.pak"));
        Assert.DoesNotContain(published.Requests, r => r.Contains("/patches/"));
        Assert.Equal(["/versions/v2/files/Data/New.pak", "/versions/v2/files/HeroesOfLiberty.exe", "/versions/v2/files/HeroesOfLiberty/Version.txt"],
            published.Requests.Order(StringComparer.Ordinal));
        Assert.Equal("kit launcher", Read(game, "HOLLauncher.exe"));
        Assert.Equal("patcher", Read(game, "hpatchz.exe"));
        Assert.Contains("friend", Read(game, "servers.json"));
        var list = VersionManifest.Load(UpdatePlan.InstalledListPath(game.Root));
        Assert.Equal("v2", list.Version);
        Assert.Equal(["Data/New.pak", "HeroesOfLiberty.exe", "HeroesOfLiberty/Version.txt"], list.Files.Select(f => f.Path));
        Assert.False(Directory.Exists(FileDownloader.TempDirFor(game.Root, "v2")));
    }

    [Fact]
    public async Task NotEnoughDiskSpaceDownloadsNothing()
    {
        using var game = KitOnly();
        var published = WithPatch();
        string? asked = null;

        var result = await Update(game, published, freeSpace: dir =>
        {
            asked = dir;
            return FinalBytes(published) + GameUpdater.DiskMargin - 1;
        });

        Assert.False(result.Success);
        Assert.StartsWith("Espace disque insuffisant", result.Message);
        Assert.Equal(game.Root, asked);
        Assert.Empty(published.Requests);
        Assert.False(Exists(game, "HeroesOfLiberty/Version.txt"));
        Assert.False(Directory.Exists(Path.Combine(game.Root, UpdatePlan.WorkDirName)));
    }

    [Fact]
    public async Task JustEnoughDiskSpaceInstalls()
    {
        using var game = KitOnly();
        var published = WithPatch();

        var result = await Update(game, published, freeSpace: _ => FinalBytes(published) + GameUpdater.DiskMargin);

        Assert.True(result.Success, result.Message);
        Assert.Equal("v2", Read(game, "HeroesOfLiberty/Version.txt"));
    }
}
