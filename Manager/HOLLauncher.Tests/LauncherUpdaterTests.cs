using System.Net;
using System.Net.Http.Headers;
using System.Security.Cryptography;
using System.Text;

namespace HOLLauncher.Tests;

public class LauncherUpdaterTests
{
    private const string Manager = "http://host:7700";
    private static readonly byte[] OldLauncher = Encoding.UTF8.GetBytes("old launcher exe");
    private static readonly byte[] NewLauncher = Enumerable.Range(0, 5000).Select(i => (byte)(i * 7)).ToArray();

    private static string Sha(byte[] content) => Convert.ToHexString(SHA256.HashData(content)).ToLowerInvariant();

    /** Serves GET /launcher/latest like HOLManager: Range honored with 206, the two headers on 200 and 206. */
    private sealed class Server
    {
        public byte[]? Body { get; set; } = NewLauncher;
        public string? Sha256 { get; set; } = Sha(NewLauncher);
        public List<(string Path, string? Range)> Requests { get; } = [];

        public HttpClient Client() => new(new FakeHandler(Answer));

        private HttpResponseMessage Answer(HttpRequestMessage request)
        {
            var range = request.Headers.Range?.Ranges.FirstOrDefault();
            Requests.Add((request.RequestUri!.AbsolutePath, range is null ? null : $"{range.From}-"));
            if (Body is null)
            {
                return new HttpResponseMessage(HttpStatusCode.NotFound)
                {
                    Content = new StringContent("""{ "error": "launcher_not_found", "message": "no" }""", Encoding.UTF8, "application/json"),
                };
            }
            var from = range?.From is { } start ? (int)start : 0;
            var response = new HttpResponseMessage(from > 0 ? HttpStatusCode.PartialContent : HttpStatusCode.OK)
            {
                Content = new ByteArrayContent(Body[from..]),
            };
            response.Headers.Add("X-Launcher-Version", "2");
            if (Sha256 is not null)
            {
                response.Headers.Add("X-Launcher-Sha256", Sha256);
            }
            return response;
        }
    }

    private static (TempDir Dir, string Exe) RunningLauncher(string name = "HOLLauncher.exe")
    {
        var dir = new TempDir();
        return (dir, dir.Write(name, OldLauncher));
    }

    private static string In(TempDir dir, string name) => Path.Combine(dir.Root, name);

    [Fact]
    public async Task NewLauncherIsDownloadedCheckedAndSwappedIn()
    {
        var (dir, exe) = RunningLauncher();
        using var _ = dir;
        var server = new Server();

        var result = await new LauncherUpdater(server.Client(), Manager, exe).UpdateAsync();

        Assert.True(result.Success, result.Message);
        Assert.Equal("2", result.Version);
        Assert.Equal("/launcher/latest", server.Requests.Single().Path);
        Assert.Equal(NewLauncher, File.ReadAllBytes(exe));
        Assert.Equal(OldLauncher, File.ReadAllBytes(In(dir, LauncherUpdater.OldName)));
        Assert.False(File.Exists(In(dir, LauncherUpdater.NewName)));
        Assert.False(File.Exists(In(dir, LauncherUpdater.NewName + ".part")));
    }

    [Fact]
    public async Task ExeKeepsItsOwnName()
    {
        var (dir, exe) = RunningLauncher("HOLLauncher-2.exe");
        using var _ = dir;

        var result = await new LauncherUpdater(new Server().Client(), Manager, exe).UpdateAsync();

        Assert.True(result.Success, result.Message);
        Assert.Equal(NewLauncher, File.ReadAllBytes(exe));
        Assert.False(File.Exists(In(dir, "HOLLauncher.exe")));
    }

    [Fact]
    public async Task WrongSha256DeletesTheDownloadAndKeepsTheLauncher()
    {
        var (dir, exe) = RunningLauncher();
        using var _ = dir;
        var server = new Server { Sha256 = Sha(OldLauncher) };

        var result = await new LauncherUpdater(server.Client(), Manager, exe).UpdateAsync();

        Assert.False(result.Success);
        Assert.Contains("SHA-256", result.Message);
        Assert.Equal(OldLauncher, File.ReadAllBytes(exe));
        Assert.Equal(["HOLLauncher.exe"], Directory.GetFiles(dir.Root).Select(Path.GetFileName));
    }

    [Fact]
    public async Task MissingSha256HeaderIsRefused()
    {
        var (dir, exe) = RunningLauncher();
        using var _ = dir;
        var server = new Server { Sha256 = null };

        var result = await new LauncherUpdater(server.Client(), Manager, exe).UpdateAsync();

        Assert.False(result.Success);
        Assert.Contains("X-Launcher-Sha256", result.Message);
        Assert.Equal(OldLauncher, File.ReadAllBytes(exe));
        Assert.Equal(["HOLLauncher.exe"], Directory.GetFiles(dir.Root).Select(Path.GetFileName));
    }

    [Fact]
    public async Task PartIsResumedWithRange()
    {
        var (dir, exe) = RunningLauncher();
        using var _ = dir;
        dir.Write(LauncherUpdater.NewName + ".part", NewLauncher[..1200]);
        var server = new Server();
        var reports = new List<LauncherProgress>();

        var result = await new LauncherUpdater(server.Client(), Manager, exe).UpdateAsync(new ListProgress(reports));

        Assert.True(result.Success, result.Message);
        Assert.Equal("1200-", server.Requests.Single().Range);
        Assert.Equal(NewLauncher, File.ReadAllBytes(exe));
        Assert.Equal(new LauncherProgress("2", NewLauncher.Length, NewLauncher.Length), reports[^1]);
        Assert.All(reports, r => Assert.True(r.BytesReceived >= 1200));
    }

    private sealed class ListProgress(List<LauncherProgress> reports) : IProgress<LauncherProgress>
    {
        public void Report(LauncherProgress value) => reports.Add(value);
    }

    [Fact]
    public async Task NoPublishedLauncherGivesAMessage()
    {
        var (dir, exe) = RunningLauncher();
        using var _ = dir;

        var result = await new LauncherUpdater(new Server { Body = null }.Client(), Manager, exe).UpdateAsync();

        Assert.False(result.Success);
        Assert.Contains("launcher_not_found", result.Message);
        Assert.Equal(OldLauncher, File.ReadAllBytes(exe));
    }

    [Fact]
    public async Task CancelKeepsTheLauncher()
    {
        var (dir, exe) = RunningLauncher();
        using var _ = dir;
        using var cancel = new CancellationTokenSource();
        cancel.Cancel();

        var result = await new LauncherUpdater(new Server().Client(), Manager, exe).UpdateAsync(cancel: cancel.Token);

        Assert.False(result.Success);
        Assert.Contains("annulée", result.Message);
        Assert.Equal(OldLauncher, File.ReadAllBytes(exe));
    }

    [Fact]
    public void FailedSwapPutsTheOldLauncherBack()
    {
        var (dir, exe) = RunningLauncher();
        using var _ = dir;

        Assert.ThrowsAny<IOException>(() => LauncherUpdater.SwapIn(exe, In(dir, LauncherUpdater.NewName)));

        Assert.Equal(OldLauncher, File.ReadAllBytes(exe));
        Assert.False(File.Exists(In(dir, LauncherUpdater.OldName)));
    }

    [Fact]
    public void SwapReplacesAnOldExeLeftBehind()
    {
        var (dir, exe) = RunningLauncher();
        using var _ = dir;
        dir.Write(LauncherUpdater.OldName, "older still");
        dir.Write(LauncherUpdater.NewName, NewLauncher);

        LauncherUpdater.SwapIn(exe, In(dir, LauncherUpdater.NewName));

        Assert.Equal(NewLauncher, File.ReadAllBytes(exe));
        Assert.Equal(OldLauncher, File.ReadAllBytes(In(dir, LauncherUpdater.OldName)));
    }

    [Fact]
    public void OldExeIsDeletedAtStart()
    {
        var (dir, exe) = RunningLauncher();
        using var _ = dir;
        dir.Write(LauncherUpdater.OldName, OldLauncher);

        Assert.True(LauncherUpdater.DeleteOld(exe));

        Assert.False(File.Exists(In(dir, LauncherUpdater.OldName)));
        Assert.True(File.Exists(exe));
    }

    [Fact]
    public void LockedOldExeIsLeftSilently()
    {
        var (dir, exe) = RunningLauncher();
        using var _ = dir;
        var old = dir.Write(LauncherUpdater.OldName, OldLauncher);

        using (new FileStream(old, FileMode.Open, FileAccess.Read, FileShare.None))
        {
            Assert.False(LauncherUpdater.DeleteOld(exe));
        }

        Assert.True(File.Exists(old));
        Assert.True(LauncherUpdater.DeleteOld(exe)); // next start
    }

    [Fact]
    public void NoOldExeIsFine()
    {
        var (dir, exe) = RunningLauncher();
        using var _ = dir;

        Assert.True(LauncherUpdater.DeleteOld(exe));
    }

    [Fact]
    public void RestartArgumentsDropUpdateLauncher()
    {
        Assert.Equal(["--game-dir", @"D:\HOL", "--update"],
            LauncherUpdater.RestartArguments(["--game-dir", @"D:\HOL", "--update-launcher", "--update"]));
    }
}
