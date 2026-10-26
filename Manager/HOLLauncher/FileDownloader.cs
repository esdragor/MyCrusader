using System.IO.Compression;
using System.Net;
using System.Net.Http.Headers;
using System.Text.Json;
using HOLUpdate;

namespace HOLLauncher;

/** Where an update stands: files finished out of all, the file being fetched, bytes received so far (all files). */
public sealed record DownloadProgress(int FilesDone, int FileCount, string CurrentPath, long BytesReceived);

/**
 * A fetched file, ready for L6b. Kind is what was really fetched (a missing patch falls back to Full). LocalPath: for
 * Full, the unpacked file, already checked (size and SHA-256); for Patch, the whole .patch, not applied yet.
 */
public sealed record DownloadResult(PlannedDownload Planned, DownloadKind Kind, string LocalPath);

/** A download that failed: HTTP error, dropped connection (the .part is kept for resuming), or a file that does not match its list. */
public sealed class UpdateDownloadException(string message, Exception? inner = null) : Exception(message, inner);

/**
 * Fetches the files of an UpdatePlan into a work folder (L6a), laid out as:
 *   files/<path>            unpacked full file, checked against the list
 *   files/<path>.gz.part    gzip being received
 *   patches/<path>.patch    whole patch (L6b applies it)
 *   patches/<path>.patch.part
 * A .part left by a cut is resumed with Range (206: appended; 200: restarted from zero). A full file already present
 * and matching its list is not fetched again. The HttpClient is given (tests: a fake handler); CreateHttpClient gives
 * one without a global timeout, since the exe weighs 124 MB gzipped; answers are read as streams.
 */
public sealed class FileDownloader
{
    private const int BufferSize = 1 << 16;

    private readonly HttpClient http;
    private readonly string managerUrl;
    private readonly string tempDir;

    public FileDownloader(HttpClient http, string managerUrl, string tempDir)
    {
        this.http = http;
        this.managerUrl = managerUrl.TrimEnd('/') + "/";
        this.tempDir = Path.GetFullPath(tempDir);
    }

    public static HttpClient CreateHttpClient() => new() { Timeout = Timeout.InfiniteTimeSpan };

    /** The launcher's, sending the key of <gameDir>/servers.json (L11). */
    public static HttpClient CreateHttpClient(string gameDir) => AccessKeyHandler.CreateHttpClient(gameDir, Timeout.InfiniteTimeSpan);

    /** <game>/.holupdate/<version>: left out of the game's list by UpdatePlan.Excludes. */
    public static string TempDirFor(string gameDir, string version) => Path.Combine(gameDir, UpdatePlan.WorkDirName, version);

    public async Task<IReadOnlyList<DownloadResult>> DownloadAllAsync(
        UpdatePlan plan, IProgress<DownloadProgress>? progress = null, CancellationToken cancel = default)
    {
        var results = new List<DownloadResult>();
        long before = 0;
        foreach (var planned in plan.Downloads)
        {
            var done = results.Count;
            var offset = before;
            var fileProgress = progress is null ? null : new Relay(received =>
                progress.Report(new DownloadProgress(done, plan.Downloads.Count, planned.File.Path, offset + received)));
            var counter = new Counter(fileProgress);
            results.Add(await DownloadAsync(planned, counter, cancel));
            before += counter.Received;
            progress?.Report(new DownloadProgress(results.Count, plan.Downloads.Count, planned.File.Path, before));
        }
        return results;
    }

    public Task<DownloadResult> DownloadAsync(
        PlannedDownload planned, IProgress<DownloadProgress>? progress = null, CancellationToken cancel = default)
    {
        var counter = new Counter(progress is null ? null
            : new Relay(received => progress.Report(new DownloadProgress(0, 1, planned.File.Path, received))));
        return DownloadAsync(planned, counter, cancel);
    }

    private async Task<DownloadResult> DownloadAsync(PlannedDownload planned, Counter counter, CancellationToken cancel)
    {
        cancel.ThrowIfCancellationRequested();
        if (planned.Kind == DownloadKind.Patch)
        {
            var patch = LocalPath("patches", planned.File.Path) + ".patch";
            if (await FetchAsync(planned.RelativeUrl, patch + ".part", counter, allowPatchNotFound: true, cancel))
            {
                File.Move(patch + ".part", patch, overwrite: true);
                return new DownloadResult(planned, DownloadKind.Patch, patch);
            }
            // L-Q2 bis: no patch from the installed version for this file, the full file instead.
            planned = planned with { Kind = DownloadKind.Full, RelativeUrl = planned.FullUrl };
        }
        var final = LocalPath("files", planned.File.Path);
        if (File.Exists(final) && Matches(final, planned.File))
        {
            return new DownloadResult(planned, DownloadKind.Full, final);
        }
        var gzPart = final + ".gz.part";
        await FetchAsync(planned.RelativeUrl, gzPart, counter, allowPatchNotFound: false, cancel);
        await UnpackAndCheckAsync(gzPart, final, planned.File, cancel);
        return new DownloadResult(planned, DownloadKind.Full, final);
    }

    /**
     * Fetches any route of the manager into part, resuming it like the game's files (L7: the launcher itself). answer
     * sees the successful answer (its headers) and the bytes kept from part (0 when restarting) before the body;
     * received reports the bytes received by this call. Errors: UpdateDownloadException, the .part kept on a cut.
     */
    public Task FetchRouteAsync(string relativeUrl, string part, Action<HttpResponseMessage, long>? answer = null,
        IProgress<long>? received = null, CancellationToken cancel = default)
        => FetchAsync(relativeUrl, part, new Counter(received is null ? null : new Relay(received.Report)),
            allowPatchNotFound: false, cancel, answer);

    /** Fetches a route into part, resuming it. false only for a 404 patch_not_found when allowed. */
    private async Task<bool> FetchAsync(string relativeUrl, string part, Counter counter, bool allowPatchNotFound,
        CancellationToken cancel, Action<HttpResponseMessage, long>? answer = null)
    {
        Directory.CreateDirectory(Path.GetDirectoryName(part)!);
        var existing = File.Exists(part) ? new FileInfo(part).Length : 0;
        using var request = new HttpRequestMessage(HttpMethod.Get, new Uri(new Uri(managerUrl), relativeUrl));
        if (existing > 0)
        {
            request.Headers.Range = new RangeHeaderValue(existing, null);
        }
        HttpResponseMessage response;
        try
        {
            response = await http.SendAsync(request, HttpCompletionOption.ResponseHeadersRead, cancel);
        }
        catch (HttpRequestException e)
        {
            throw new UpdateDownloadException($"{relativeUrl}: no answer ({e.Message})", e);
        }
        using (response)
        {
            if (response.StatusCode == HttpStatusCode.RequestedRangeNotSatisfiable && existing > 0)
            {
                // The .part is not a prefix of this file any more (or already whole): start over.
                File.Delete(part);
                return await FetchAsync(relativeUrl, part, counter, allowPatchNotFound, cancel, answer);
            }
            if (!response.IsSuccessStatusCode)
            {
                var body = await response.Content.ReadAsStringAsync(cancel);
                var error = ReadError(body);
                if (allowPatchNotFound && response.StatusCode == HttpStatusCode.NotFound && error == "patch_not_found")
                {
                    return false;
                }
                if (AccessKeyHandler.IsBadKey((int)response.StatusCode, error))
                {
                    throw new UpdateDownloadException(AccessKeyHandler.BadKeyMessage);
                }
                throw new UpdateDownloadException($"{relativeUrl}: HTTP {(int)response.StatusCode}{(error is null ? "" : $" {error}")}");
            }
            var append = response.StatusCode == HttpStatusCode.PartialContent && existing > 0;
            answer?.Invoke(response, append ? existing : 0);
            try
            {
                await using var source = await response.Content.ReadAsStreamAsync(cancel);
                await using var target = new FileStream(part, append ? FileMode.Append : FileMode.Create, FileAccess.Write,
                    FileShare.None, BufferSize, useAsync: true);
                var buffer = new byte[BufferSize];
                int read;
                while ((read = await source.ReadAsync(buffer, cancel)) > 0)
                {
                    await target.WriteAsync(buffer.AsMemory(0, read), cancel);
                    counter.Add(read);
                }
            }
            catch (Exception e) when (e is IOException or HttpRequestException && !cancel.IsCancellationRequested)
            {
                throw new UpdateDownloadException($"{relativeUrl}: interrupted ({e.Message}), will resume", e);
            }
        }
        return true;
    }

    private static async Task UnpackAndCheckAsync(string gz, string final, FileEntry file, CancellationToken cancel)
    {
        var unpacked = final + ".tmp";
        try
        {
            await using (var source = new GZipStream(File.OpenRead(gz), CompressionMode.Decompress))
            await using (var target = File.Create(unpacked))
            {
                await source.CopyToAsync(target, cancel);
            }
        }
        catch (InvalidDataException e)
        {
            Delete(gz, unpacked);
            throw new UpdateDownloadException($"{file.Path}: bad gzip ({e.Message})", e);
        }
        var size = new FileInfo(unpacked).Length;
        if (size != file.Size)
        {
            Delete(gz, unpacked);
            throw new UpdateDownloadException($"{file.Path}: size {size} instead of {file.Size}");
        }
        var hash = ManifestBuilder.Hash(unpacked);
        if (hash != file.Sha256)
        {
            Delete(gz, unpacked);
            throw new UpdateDownloadException($"{file.Path}: SHA-256 {hash} instead of {file.Sha256}");
        }
        File.Move(unpacked, final, overwrite: true);
        File.Delete(gz);
    }

    private static bool Matches(string local, FileEntry file)
        => new FileInfo(local).Length == file.Size && ManifestBuilder.Hash(local) == file.Sha256;

    private string LocalPath(string kind, string path)
        => Path.Combine(tempDir, kind, path.Replace('/', Path.DirectorySeparatorChar));

    private static void Delete(params string[] files)
    {
        foreach (var file in files)
        {
            File.Delete(file);
        }
    }

    private static string? ReadError(string body)
    {
        try
        {
            using var document = JsonDocument.Parse(body);
            return document.RootElement.ValueKind == JsonValueKind.Object
                && document.RootElement.TryGetProperty("error", out var error) && error.ValueKind == JsonValueKind.String
                ? error.GetString() : null;
        }
        catch (JsonException)
        {
            return null;
        }
    }

    /** Bytes received for one file, reported as they come. */
    private sealed class Counter(Relay? relay)
    {
        public long Received { get; private set; }

        public void Add(int bytes)
        {
            Received += bytes;
            relay?.Report(Received);
        }
    }

    private sealed class Relay(Action<long> report)
    {
        public void Report(long received) => report(received);
    }
}
