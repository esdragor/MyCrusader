using System.IO.Compression;
using System.Net;
using System.Security.Cryptography;
using System.Text;
using HOLUpdate;

namespace HOLLauncher.Tests;

/** L17: "Verify / repair the files": a damaged or missing file is fetched again in full, nothing else is touched. */
public class GameRepairerTests
{
    private const string Manager = "http://host:7700";

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

    private static Dictionary<string, byte[]> Version(string version) => new(StringComparer.Ordinal)
    {
        ["HeroesOfLiberty/Version.txt"] = Text(version),
        ["HeroesOfLiberty.exe"] = Text($"exe {version}"),
        ["Data/Game.pak"] = Text($"pak {version}"),
        ["servers.json"] = Text("""{ "managers": [ { "url": "http://127.0.0.1:7700" } ] }"""),
        ["HOLLauncher.exe"] = Text($"launcher {version}"),
    };

    /** A manager keeping some versions (latest: the last one given); keeps the requested paths. */
    private sealed class Server(params string[] versions)
    {
        public Dictionary<string, Dictionary<string, byte[]>> Versions { get; } =
            versions.ToDictionary(v => v, Version);

        public List<string> Requests { get; } = [];

        public LatestVersion Latest
        {
            get
            {
                var version = versions[^1];
                return new LatestVersion(version, Manifest(version), "", [], "");
            }
        }

        public VersionManifest Manifest(string version)
            => new(version, Versions[version].Select(f => Entry(f.Key, f.Value)));

        public HttpClient Client() => new(new FakeHandler(Answer));

        private HttpResponseMessage Answer(HttpRequestMessage request)
        {
            var path = Uri.UnescapeDataString(request.RequestUri!.AbsolutePath);
            Requests.Add(path);
            var parts = path.Split('/', 5); // "", "versions", v, "files", path
            if (parts.Length == 5 && parts[1] == "versions" && parts[3] == "files")
            {
                if (!Versions.TryGetValue(parts[2], out var files))
                {
                    return Error("version_not_found");
                }
                if (files.TryGetValue(parts[4], out var file))
                {
                    return new HttpResponseMessage(HttpStatusCode.OK) { Content = new ByteArrayContent(Gzip(file)) };
                }
            }
            return Error("file_not_found");
        }

        private static HttpResponseMessage Error(string error) => new(HttpStatusCode.NotFound)
        {
            Content = new StringContent($$"""{ "error": "{{error}}", "message": "no" }""", Encoding.UTF8, "application/json"),
        };
    }

    /** The version installed by the launcher (installed.json) unless fromZip, with the launcher's and the player's files. */
    private static TempDir Installed(Server server, string version, bool fromZip = false)
    {
        var game = new TempDir();
        foreach (var (path, content) in Version(version).Where(f => !UpdatePlan.IsLauncherOwned(f.Key)))
        {
            game.Write(path, content);
        }
        game.Write("HOLLauncher.exe", "my launcher");
        game.Write("hpatchz.exe", "patcher");
        game.Write("servers.json", """{ "managers": [ { "url": "http://friend:7700" } ] }""");
        game.Write(LauncherSettings.FileName, """{ "language": "fr" }""");
        game.Write("Stray.txt", "player's file");
        game.Write("HeroesOfLiberty/Saved/Config/Game.ini", "player's settings");
        if (!fromZip)
        {
            Directory.CreateDirectory(Path.Combine(game.Root, UpdatePlan.WorkDirName));
            new VersionManifest(version, Version(version).Where(f => !UpdatePlan.IsLauncherOwned(f.Key)).Select(f => Entry(f.Key, f.Value)))
                .Save(UpdatePlan.InstalledListPath(game.Root));
        }
        return game;
    }

    private static Task<UpdateResult> Repair(TempDir game, Server server, Func<string, long>? freeSpace = null,
        CancellationToken cancel = default)
        => new GameRepairer(server.Client(), freeSpace ?? (_ => long.MaxValue))
            .RepairAsync(game.Root, Manager, server.Latest, cancel: cancel);

    private static string Read(TempDir game, string path) => File.ReadAllText(Path.Combine(game.Root, path));

    [Fact]
    public async Task DamagedFileIsFetchedAgainAndOnlyIt()
    {
        var server = new Server("v2");
        using var game = Installed(server, "v2");
        game.Write("Data/Game.pak", "pak v2 damaged on purpose");

        var result = await Repair(game, server);

        Assert.True(result.Success, result.Message);
        Assert.Equal(Texts.Format("RepairDone", 1, VersionName.Display("v2")), result.Message);
        Assert.Equal("pak v2", Read(game, "Data/Game.pak"));
        Assert.Equal(["/versions/v2/files/Data/Game.pak"], server.Requests);
        Assert.False(Directory.Exists(FileDownloader.TempDirFor(game.Root, "v2")));
    }

    [Fact]
    public async Task MissingFilesAreRestored()
    {
        var server = new Server("v2");
        using var game = Installed(server, "v2");
        File.Delete(Path.Combine(game.Root, "HeroesOfLiberty.exe"));
        File.Delete(Path.Combine(game.Root, "Data", "Game.pak"));

        var result = await Repair(game, server);

        Assert.True(result.Success, result.Message);
        Assert.Equal(Texts.Format("RepairDone", 2, VersionName.Display("v2")), result.Message);
        Assert.Equal("exe v2", Read(game, "HeroesOfLiberty.exe"));
        Assert.Equal("pak v2", Read(game, "Data/Game.pak"));
    }

    [Fact]
    public async Task IntactGameFetchesNothing()
    {
        var server = new Server("v2");
        using var game = Installed(server, "v2");

        var result = await Repair(game, server);

        Assert.True(result.Success, result.Message);
        Assert.Equal(Texts.Format("RepairIntact", VersionName.Display("v2")), result.Message);
        Assert.Empty(server.Requests);
    }

    [Fact]
    public async Task LauncherAndPlayerFilesAreLeftAlone()
    {
        var server = new Server("v2");
        using var game = Installed(server, "v2");
        game.Write("Data/Game.pak", "damaged");

        var result = await Repair(game, server);

        Assert.True(result.Success, result.Message);
        Assert.Equal("my launcher", Read(game, "HOLLauncher.exe"));
        Assert.Equal("patcher", Read(game, "hpatchz.exe"));
        Assert.Contains("friend", Read(game, "servers.json"));
        Assert.Contains("fr", Read(game, LauncherSettings.FileName));
        Assert.Equal("player's file", Read(game, "Stray.txt"));
        Assert.Equal("player's settings", Read(game, "HeroesOfLiberty/Saved/Config/Game.ini"));
        Assert.DoesNotContain(server.Requests, r => r.EndsWith("HOLLauncher.exe") || r.EndsWith("servers.json"));
    }

    [Fact]
    public async Task OlderVersionStillOnTheServerIsRepairedInPlace()
    {
        var server = new Server("v1", "v2");
        using var game = Installed(server, "v1");
        game.Write("HeroesOfLiberty.exe", "damaged");

        var result = await Repair(game, server);

        Assert.True(result.Success, result.Message);
        Assert.Equal(Texts.Format("RepairDone", 1, VersionName.Display("v1")), result.Message);
        Assert.Equal("exe v1", Read(game, "HeroesOfLiberty.exe"));
        Assert.Equal("v1", Read(game, "HeroesOfLiberty/Version.txt"));
        Assert.Contains("/versions/v1/files/HeroesOfLiberty.exe", server.Requests);
        Assert.DoesNotContain(server.Requests, r => r.StartsWith("/versions/v2/"));
    }

    [Fact]
    public async Task VersionGoneFromTheServerIsRepairedToTheLatest()
    {
        var server = new Server("v2");
        using var game = Installed(server, "v1");
        game.Write("Data/Game.pak", "damaged");

        var result = await Repair(game, server);

        Assert.True(result.Success, result.Message);
        Assert.Equal(Texts.Format("RepairToLatest", VersionName.Display("v1"), VersionName.Display("v2"), 3), result.Message);
        Assert.Equal("v2", Read(game, "HeroesOfLiberty/Version.txt"));
        Assert.Equal("exe v2", Read(game, "HeroesOfLiberty.exe"));
        Assert.Equal("pak v2", Read(game, "Data/Game.pak"));
        Assert.Equal("v2", VersionManifest.Load(UpdatePlan.InstalledListPath(game.Root)).Version);
    }

    [Fact]
    public async Task OlderZipInstallWithoutListIsRepairedToTheLatest()
    {
        var server = new Server("v1", "v2");
        using var game = Installed(server, "v1", fromZip: true);

        var result = await Repair(game, server);

        Assert.True(result.Success, result.Message);
        Assert.Equal("v2", Read(game, "HeroesOfLiberty/Version.txt"));
        Assert.Equal("player's file", Read(game, "Stray.txt"));
    }

    [Fact]
    public async Task ZipInstallOfTheLatestGetsItsInstalledList()
    {
        var server = new Server("v2");
        using var game = Installed(server, "v2", fromZip: true);

        var result = await Repair(game, server);

        Assert.True(result.Success, result.Message);
        var list = VersionManifest.Load(UpdatePlan.InstalledListPath(game.Root));
        Assert.Equal(["Data/Game.pak", "HeroesOfLiberty.exe", "HeroesOfLiberty/Version.txt"], list.Files.Select(f => f.Path));
    }

    [Fact]
    public async Task NothingInstalledIsRefused()
    {
        var server = new Server("v2");
        using var game = new TempDir();

        var result = await Repair(game, server);

        Assert.False(result.Success);
        Assert.Equal(Texts.Get("RepairNotInstalled"), result.Message);
        Assert.Empty(server.Requests);
    }

    [Fact]
    public async Task LockedDamagedFileReplacesNothing()
    {
        var server = new Server("v2");
        using var game = Installed(server, "v2");
        game.Write("Data/Game.pak", "damaged");
        game.Write("HeroesOfLiberty.exe", "damaged too");
        UpdateResult result;
        using (new FileStream(Path.Combine(game.Root, "HeroesOfLiberty.exe"), FileMode.Open, FileAccess.Read, FileShare.Read))
        {
            result = await Repair(game, server);
        }

        Assert.False(result.Success);
        Assert.Equal("damaged", Read(game, "Data/Game.pak"));
    }

    [Fact]
    public async Task NotEnoughSpaceFetchesNothing()
    {
        var server = new Server("v2");
        using var game = Installed(server, "v2");
        File.Delete(Path.Combine(game.Root, "HeroesOfLiberty.exe"));

        var result = await Repair(game, server, freeSpace: _ => GameUpdater.DiskMargin);

        Assert.False(result.Success);
        Assert.Empty(server.Requests);
        Assert.False(File.Exists(Path.Combine(game.Root, "HeroesOfLiberty.exe")));
    }

    [Fact]
    public async Task CancelledRepairReplacesNothing()
    {
        var server = new Server("v2");
        using var game = Installed(server, "v2");
        game.Write("Data/Game.pak", "damaged");
        using var cancel = new CancellationTokenSource();
        cancel.Cancel();

        var result = await Repair(game, server, cancel: cancel.Token);

        Assert.False(result.Success);
        Assert.Equal(Texts.Get("RepairCancelled"), result.Message);
        Assert.Equal("damaged", Read(game, "Data/Game.pak"));
    }
}
