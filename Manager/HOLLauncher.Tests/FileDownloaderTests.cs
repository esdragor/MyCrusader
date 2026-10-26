using System.IO.Compression;
using System.Net;
using System.Net.Http.Headers;
using System.Security.Cryptography;
using System.Text;
using HOLUpdate;

namespace HOLLauncher.Tests;

public class FileDownloaderTests
{
    private const string Manager = "http://host:7700";

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

    private static PlannedDownload Full(FileEntry file)
        => new(file, DownloadKind.Full, UpdatePlan.FileUrl("v2", file.Path), UpdatePlan.FileUrl("v2", file.Path));

    private static PlannedDownload Patch(FileEntry file)
        => new(file, DownloadKind.Patch, UpdatePlan.PatchUrl("v2", "v1", file.Path), UpdatePlan.FileUrl("v2", file.Path));

    /** Serves bodies by path, honors Range with 206, and can cut a body after some bytes (once). */
    private sealed class Server
    {
        public Dictionary<string, byte[]> Bodies { get; } = [];
        public bool IgnoreRange { get; set; }
        public int? CutAfter { get; set; }
        public List<(string Path, string? Range)> Requests { get; } = [];

        public FakeHandler Handler => new(Answer);

        private HttpResponseMessage Answer(HttpRequestMessage request)
        {
            var path = request.RequestUri!.AbsolutePath;
            var range = request.Headers.Range?.Ranges.FirstOrDefault();
            Requests.Add((path, range is null ? null : $"{range.From}-"));
            if (!Bodies.TryGetValue(path, out var body))
            {
                var error = path.Contains("/patches/") ? "patch_not_found" : "file_not_found";
                return new HttpResponseMessage(HttpStatusCode.NotFound)
                {
                    Content = new StringContent($$"""{ "error": "{{error}}", "message": "no" }""", Encoding.UTF8, "application/json"),
                };
            }
            var from = !IgnoreRange && range?.From is { } start ? (int)start : 0;
            var slice = body[from..];
            Stream stream = new MemoryStream(slice);
            if (CutAfter is { } cut)
            {
                CutAfter = null;
                stream = new CuttingStream(slice, cut);
            }
            var response = new HttpResponseMessage(from > 0 ? HttpStatusCode.PartialContent : HttpStatusCode.OK)
            {
                Content = new StreamContent(stream),
            };
            if (from > 0)
            {
                response.Content.Headers.ContentRange = new ContentRangeHeaderValue(from, body.Length - 1, body.Length);
            }
            return response;
        }
    }

    /** Gives the first `cut` bytes, then fails like a dropped connection. */
    private sealed class CuttingStream(byte[] data, int cut) : MemoryStream(data)
    {
        public override int Read(byte[] buffer, int offset, int count)
        {
            if (Position >= cut)
            {
                throw new IOException("connection reset");
            }
            return base.Read(buffer, offset, (int)Math.Min(count, cut - Position));
        }

        public override Task<int> ReadAsync(byte[] buffer, int offset, int count, CancellationToken cancellationToken)
            => Task.FromResult(Read(buffer, offset, count));

        public override ValueTask<int> ReadAsync(Memory<byte> buffer, CancellationToken cancellationToken = default)
        {
            var array = new byte[buffer.Length];
            var read = Read(array, 0, array.Length);
            array.AsMemory(0, read).CopyTo(buffer);
            return ValueTask.FromResult(read);
        }
    }

    private static FileDownloader Downloader(Server server, string temp) => new(new HttpClient(server.Handler), Manager, temp);

    [Fact]
    public async Task FullFileIsUnpackedAndChecked()
    {
        using var temp = new TempDir();
        var content = Encoding.UTF8.GetBytes(new string('x', 5000) + "end");
        var file = Entry("Data/A b.txt", content);
        var server = new Server();
        server.Bodies["/versions/v2/files/Data/A%20b.txt"] = Gzip(content);

        var result = await Downloader(server, temp.Root).DownloadAsync(Full(file));

        Assert.Equal(DownloadKind.Full, result.Kind);
        Assert.Equal(Path.Combine(temp.Root, "files", "Data", "A b.txt"), result.LocalPath);
        Assert.Equal(content, File.ReadAllBytes(result.LocalPath));
        Assert.Empty(Directory.GetFiles(temp.Root, "*.part", SearchOption.AllDirectories));
        Assert.Empty(Directory.GetFiles(temp.Root, "*.gz", SearchOption.AllDirectories));
    }

    [Fact]
    public async Task WrongHashDeletesFileAndFails()
    {
        using var temp = new TempDir();
        var file = Entry("A.txt", Encoding.UTF8.GetBytes("good"));
        var server = new Server();
        server.Bodies["/versions/v2/files/A.txt"] = Gzip(Encoding.UTF8.GetBytes("evil"));

        var error = await Assert.ThrowsAsync<UpdateDownloadException>(() => Downloader(server, temp.Root).DownloadAsync(Full(file)));

        Assert.Contains("SHA-256", error.Message);
        Assert.Contains("A.txt", error.Message);
        Assert.Empty(Directory.GetFiles(temp.Root, "*", SearchOption.AllDirectories));
    }

    [Fact]
    public async Task WrongSizeDeletesFileAndFails()
    {
        using var temp = new TempDir();
        var file = Entry("A.txt", Encoding.UTF8.GetBytes("good")) with { Size = 99 };
        var server = new Server();
        server.Bodies["/versions/v2/files/A.txt"] = Gzip(Encoding.UTF8.GetBytes("good"));

        var error = await Assert.ThrowsAsync<UpdateDownloadException>(() => Downloader(server, temp.Root).DownloadAsync(Full(file)));

        Assert.Contains("size", error.Message);
        Assert.Empty(Directory.GetFiles(temp.Root, "*", SearchOption.AllDirectories));
    }

    [Fact]
    public async Task CutDownloadResumesWithRange()
    {
        using var temp = new TempDir();
        var content = RandomNumberGenerator.GetBytes(200_000);
        var gz = Gzip(content);
        var file = Entry("Game.exe", content);
        var server = new Server { CutAfter = 70_000 };
        server.Bodies["/versions/v2/files/Game.exe"] = gz;
        var downloader = Downloader(server, temp.Root);

        await Assert.ThrowsAsync<UpdateDownloadException>(() => downloader.DownloadAsync(Full(file)));
        var part = Assert.Single(Directory.GetFiles(temp.Root, "*.part", SearchOption.AllDirectories));
        Assert.Equal(70_000, new FileInfo(part).Length);

        var result = await downloader.DownloadAsync(Full(file));

        Assert.Equal(content, File.ReadAllBytes(result.LocalPath));
        Assert.Equal(("/versions/v2/files/Game.exe", (string?)"70000-"), server.Requests[^1]);
    }

    [Fact]
    public async Task ServerIgnoringRangeRestartsFromZero()
    {
        using var temp = new TempDir();
        var content = RandomNumberGenerator.GetBytes(50_000);
        var file = Entry("Game.exe", content);
        var server = new Server { IgnoreRange = true };
        server.Bodies["/versions/v2/files/Game.exe"] = Gzip(content);
        temp.Write("files/Game.exe.gz.part", new byte[1234]);

        var result = await Downloader(server, temp.Root).DownloadAsync(Full(file));

        Assert.Equal(content, File.ReadAllBytes(result.LocalPath));
        Assert.Equal("1234-", server.Requests[^1].Range);
    }

    [Fact]
    public async Task PatchIsKeptAsIs()
    {
        using var temp = new TempDir();
        var patch = Encoding.UTF8.GetBytes("HDIFF-patch-bytes");
        var file = Entry("Game.exe", Encoding.UTF8.GetBytes("new exe"));
        var server = new Server();
        server.Bodies["/versions/v2/patches/v1/Game.exe"] = patch;

        var result = await Downloader(server, temp.Root).DownloadAsync(Patch(file));

        Assert.Equal(DownloadKind.Patch, result.Kind);
        Assert.Equal(Path.Combine(temp.Root, "patches", "Game.exe.patch"), result.LocalPath);
        Assert.Equal(patch, File.ReadAllBytes(result.LocalPath));
    }

    [Fact]
    public async Task MissingPatchFallsBackToFullFile()
    {
        using var temp = new TempDir();
        var content = Encoding.UTF8.GetBytes("new exe");
        var file = Entry("Game.exe", content);
        var server = new Server();
        server.Bodies["/versions/v2/files/Game.exe"] = Gzip(content);

        var result = await Downloader(server, temp.Root).DownloadAsync(Patch(file));

        Assert.Equal(DownloadKind.Full, result.Kind);
        Assert.Equal(content, File.ReadAllBytes(result.LocalPath));
        Assert.Equal(["/versions/v2/patches/v1/Game.exe", "/versions/v2/files/Game.exe"], server.Requests.Select(r => r.Path));
    }

    [Fact]
    public async Task MissingFullFileFailsClearly()
    {
        using var temp = new TempDir();
        var file = Entry("A.txt", Encoding.UTF8.GetBytes("x"));

        var error = await Assert.ThrowsAsync<UpdateDownloadException>(
            () => Downloader(new Server(), temp.Root).DownloadAsync(Full(file)));

        Assert.Contains("404", error.Message);
        Assert.Contains("file_not_found", error.Message);
    }

    [Fact]
    public async Task AlreadyVerifiedFileIsNotDownloadedAgain()
    {
        using var temp = new TempDir();
        var content = Encoding.UTF8.GetBytes("done");
        var file = Entry("A.txt", content);
        temp.Write("files/A.txt", content);
        var server = new Server();

        var result = await Downloader(server, temp.Root).DownloadAsync(Full(file));

        Assert.Equal(content, File.ReadAllBytes(result.LocalPath));
        Assert.Empty(server.Requests);
    }

    [Fact]
    public async Task ProgressReportsReceivedBytesAndFiles()
    {
        using var temp = new TempDir();
        var a = Encoding.UTF8.GetBytes("aaaa");
        var b = RandomNumberGenerator.GetBytes(300_000);
        var server = new Server();
        server.Bodies["/versions/v2/files/A.txt"] = Gzip(a);
        server.Bodies["/versions/v2/files/B.bin"] = Gzip(b);
        var plan = UpdatePlan.Create(VersionManifest.Empty,
            new LatestVersion("v2", new VersionManifest("v2", [Entry("A.txt", a), Entry("B.bin", b)]), "", [], ""));
        var reports = new List<DownloadProgress>();

        var results = await Downloader(server, temp.Root).DownloadAllAsync(plan, new SyncProgress<DownloadProgress>(reports.Add));

        Assert.Equal(2, results.Count);
        var last = reports[^1];
        Assert.Equal(2, last.FileCount);
        Assert.Equal(2, last.FilesDone);
        Assert.Equal(Gzip(a).Length + Gzip(b).Length, last.BytesReceived);
        Assert.Contains(reports, r => r.CurrentPath == "B.bin" && r.FilesDone == 1);
    }

    [Fact]
    public async Task CancelStopsDownload()
    {
        using var temp = new TempDir();
        var content = Encoding.UTF8.GetBytes("x");
        var server = new Server();
        server.Bodies["/versions/v2/files/A.txt"] = Gzip(content);
        using var cancel = new CancellationTokenSource();
        cancel.Cancel();

        await Assert.ThrowsAnyAsync<OperationCanceledException>(
            () => Downloader(server, temp.Root).DownloadAsync(Full(Entry("A.txt", content)), null, cancel.Token));
    }

    [Fact]
    public void UpdateFolderIsUnderGameFolder()
    {
        Assert.Equal(Path.Combine("C:\\Game", ".holupdate", "v2"), FileDownloader.TempDirFor("C:\\Game", "v2"));
    }

    [Fact]
    public void DownloadClientHasNoGlobalTimeout()
    {
        Assert.Equal(Timeout.InfiniteTimeSpan, FileDownloader.CreateHttpClient().Timeout);
    }

    /** Progress<T> posts to the thread pool; this one reports at once, in order. */
    private sealed class SyncProgress<T>(Action<T> report) : IProgress<T>
    {
        public void Report(T value) => report(value);
    }
}
