using System.Net;
using System.Net.Http.Headers;
using System.Text.Json;
using HOLUpdate;

namespace HOLLauncher;

/**
 * Client of GET /launcher/home (launcher 5; PROTOCOL.md, "Accueil du lanceur"), giving the window its HomeContent. Never
 * throws: an older manager (404), a refusal (401 bad_key...), a body this launcher does not read or a network failure
 * give HomeContent.Empty, and the window hides its panels. Bad rows are left out one by one (only news).
 * The key art is kept in <launcher folder>/HOLLauncher.home/ (CacheDirName, a launcher-owned folder a game update never
 * touches, UpdatePlan.LauncherOwned): background.jpg or .png and background.etag. It is asked again with If-None-Match
 * at each read (304: the cached file); when the manager cannot be reached, the last file is still given so that the
 * home keeps its picture offline. The manager saying it has none removes it.
 */
public sealed class HomeClient
{
    public const int CurrentFormat = 1;

    /** Beside the launcher's exe, like LauncherSettings.FileName; listed in UpdatePlan.LauncherOwned. */
    public const string CacheDirName = "HOLLauncher.home";

    public const string ETagFile = "background.etag";

    /** The key art takes longer than /versions/latest's 5 s on a slow Tailscale link (4 MB at most, HomeFiles). */
    public static readonly TimeSpan DefaultTimeout = TimeSpan.FromSeconds(20);

    private readonly HttpClient http;

    public HomeClient(HttpClient http, string cacheDir)
    {
        this.http = http;
        CacheDir = cacheDir;
    }

    public string CacheDir { get; }

    /** The launcher's: the key of <gameDir>/servers.json (L11), the cache beside the launcher's exe. */
    public static HomeClient ForLauncher(string gameDir, string launcherDir) =>
        new(AccessKeyHandler.CreateHttpClient(gameDir, DefaultTimeout), Path.Combine(launcherDir, CacheDirName));

    /** language: "fr", "en", "ja" (the window's); the manager falls back to French field by field. */
    public async Task<HomeContent> GetAsync(string managerUrl, string? language, CancellationToken cancel = default)
    {
        var baseUrl = managerUrl.TrimEnd('/') + "/";
        var query = string.IsNullOrEmpty(language) ? "" : "?lang=" + Uri.EscapeDataString(language);
        if (!Uri.TryCreate(baseUrl, UriKind.Absolute, out var baseUri) || baseUri.Scheme is not ("http" or "https"))
        {
            return HomeContent.Empty;
        }
        string body;
        try
        {
            using var response = await http.GetAsync(new Uri(baseUri, "launcher/home" + query), cancel);
            if (!response.IsSuccessStatusCode)
            {
                return HomeContent.Empty;
            }
            body = await response.Content.ReadAsStringAsync(cancel);
        }
        catch (Exception e) when (e is HttpRequestException or OperationCanceledException)
        {
            // Unreachable (or closing): the panels hide, the last picture stays.
            return HomeContent.Empty with { BackgroundFile = CachedBackground() };
        }
        HomeContent content;
        string? background;
        try
        {
            (content, background) = Parse(body);
        }
        catch (Exception e) when (e is JsonException or InvalidDataException or InvalidOperationException)
        {
            return HomeContent.Empty;
        }
        if (background == null || !Uri.TryCreate(baseUri, background.TrimStart('/'), out var backgroundUri)
            || backgroundUri.Scheme is not ("http" or "https"))
        {
            ClearCache();
            return content;
        }
        return content with { BackgroundFile = await BackgroundAsync(backgroundUri, cancel) };
    }

    /** The cached key art (the newest of background.jpg / .png), null without. */
    public string? CachedBackground()
    {
        try
        {
            return Directory.Exists(CacheDir) ? HomeFiles.FindBackground(CacheDir) : null;
        }
        catch (Exception e) when (e is IOException or UnauthorizedAccessException)
        {
            return null;
        }
    }

    private static (HomeContent Content, string? Background) Parse(string body)
    {
        using var document = JsonDocument.Parse(body);
        var root = document.RootElement;
        if (root.ValueKind != JsonValueKind.Object)
        {
            throw new InvalidDataException("answer is not a JSON object");
        }
        if (!root.TryGetProperty("format", out var format) || format.ValueKind != JsonValueKind.Number
            || !format.TryGetInt32(out var number) || number < 1 || number > CurrentFormat)
        {
            throw new InvalidDataException($"answer format unknown (this launcher reads up to {CurrentFormat})");
        }
        var announcements = Rows(root, "announcements")
            .Select(ReadAnnouncement)
            .OfType<Announcement>()
            .OrderByDescending(a => a.DateUtc)
            .Take(HomeFiles.MaxServed)
            .ToList();
        var links = Rows(root, "links")
            .Select(r => (Kind: ReadString(r, "kind"), Url: ReadString(r, "url")))
            .Where(r => HomeFiles.IsHttpUrl(r.Url))
            .Select(r => new CommunityLink(HomeFiles.NormalizeKind(r.Kind), r.Url!))
            .ToList();
        return (new HomeContent(announcements, links, null), ReadString(root, "background"));
    }

    private static Announcement? ReadAnnouncement(JsonElement row)
    {
        var id = ReadString(row, "id");
        var title = ReadString(row, "title")?.Trim();
        if (string.IsNullOrEmpty(id) || string.IsNullOrEmpty(title) || !HomeFiles.TryParseDate(ReadString(row, "date"), out var date))
        {
            return null;
        }
        var link = ReadString(row, "link");
        return new Announcement(id, date, title, ReadString(row, "body") ?? "", HomeFiles.IsHttpUrl(link) ? link : null);
    }

    private static IEnumerable<JsonElement> Rows(JsonElement root, string name) =>
        root.TryGetProperty(name, out var rows) && rows.ValueKind == JsonValueKind.Array
            ? rows.EnumerateArray().Where(r => r.ValueKind == JsonValueKind.Object).ToList()
            : [];

    private static string? ReadString(JsonElement row, string name) =>
        row.TryGetProperty(name, out var value) && value.ValueKind == JsonValueKind.String ? value.GetString() : null;

    /**
     * The key art, asked with the cached file's ETag: 304 keeps it, 200 replaces it (written beside then moved, the
     * old one left if the window still holds it), anything else (network, error, not an image) keeps the last file.
     */
    private async Task<string?> BackgroundAsync(Uri uri, CancellationToken cancel)
    {
        var cached = CachedBackground();
        var tagFile = Path.Combine(CacheDir, ETagFile);
        try
        {
            using var request = new HttpRequestMessage(HttpMethod.Get, uri);
            if (cached != null && File.Exists(tagFile)
                && EntityTagHeaderValue.TryParse(File.ReadAllText(tagFile).Trim(), out var tag))
            {
                request.Headers.IfNoneMatch.Add(tag);
            }
            using var response = await http.SendAsync(request, HttpCompletionOption.ResponseHeadersRead, cancel);
            if (response.StatusCode == HttpStatusCode.NotModified || !response.IsSuccessStatusCode)
            {
                return cached;
            }
            if (response.Content.Headers.ContentLength > HomeFiles.MaxBackgroundBytes)
            {
                return cached;
            }
            var bytes = await ReadCapped(response, cancel);
            if (bytes == null || HomeFiles.ImageType(bytes) is not { } type)
            {
                return cached;
            }
            Directory.CreateDirectory(CacheDir);
            var target = Path.Combine(CacheDir, HomeFiles.BackgroundName + HomeFiles.ExtensionOf(type));
            var part = target + ".part";
            await File.WriteAllBytesAsync(part, bytes, cancel);
            File.Move(part, target, overwrite: true);
            // The tag only once the file is in place: a failed move never pairs an old picture with a new tag.
            if (response.Headers.ETag is { } newTag)
            {
                File.WriteAllText(tagFile, newTag.ToString());
            }
            else
            {
                File.Delete(tagFile);
            }
            DeleteQuietly(Path.Combine(CacheDir, HomeFiles.BackgroundName + (type == HomeFiles.PngType ? ".jpg" : ".png")));
            return target;
        }
        catch (Exception e) when (e is HttpRequestException or OperationCanceledException or IOException or UnauthorizedAccessException)
        {
            return cached;
        }
    }

    /** The body, null past MaxBackgroundBytes (a manager without Content-Length is not trusted with memory either). */
    private static async Task<byte[]?> ReadCapped(HttpResponseMessage response, CancellationToken cancel)
    {
        await using var stream = await response.Content.ReadAsStreamAsync(cancel);
        using var memory = new MemoryStream();
        var buffer = new byte[81920];
        int read;
        while ((read = await stream.ReadAsync(buffer, cancel)) > 0)
        {
            memory.Write(buffer, 0, read);
            if (memory.Length > HomeFiles.MaxBackgroundBytes)
            {
                return null;
            }
        }
        return memory.ToArray();
    }

    /** The manager has no key art (any more): the cached one goes too. */
    private void ClearCache()
    {
        foreach (var name in new[] { HomeFiles.BackgroundName + ".jpg", HomeFiles.BackgroundName + ".png", ETagFile })
        {
            DeleteQuietly(Path.Combine(CacheDir, name));
        }
    }

    private static void DeleteQuietly(string file)
    {
        try
        {
            File.Delete(file);
        }
        catch (Exception e) when (e is IOException or UnauthorizedAccessException)
        {
            // Held by the window: the next read tries again.
        }
    }
}
