using System.Text.Json;
using HOLUpdate;

namespace HOLLauncher;

/** The answer of GET /versions/latest (PROTOCOL.md, "Versions du jeu"). */
public sealed record LatestVersion(
    string Version, VersionManifest Manifest, string Notes, IReadOnlyList<string> PatchesFrom, string LauncherVersion);

public enum LatestOutcome
{
    Ok,
    /** No answer: bad address, no connection, timeout. */
    Unreachable,
    /** An answer, but an error (404 no_version...) or a body this launcher does not accept. */
    Refused,
}

/** Latest is set only when Outcome is Ok; Message says why otherwise. */
public sealed record LatestResult(LatestOutcome Outcome, LatestVersion? Latest, string Message);

/**
 * Client of GET /versions/latest. Never throws to the window: every failure becomes a LatestResult. The HttpClient is
 * given (tests: a fake HttpMessageHandler); CreateHttpClient gives the launcher's, with a short timeout.
 */
public sealed class VersionsClient
{
    public const int CurrentFormat = 1;
    public static readonly TimeSpan DefaultTimeout = TimeSpan.FromSeconds(5);

    private readonly HttpClient http;

    public VersionsClient(HttpClient http)
    {
        this.http = http;
    }

    public static HttpClient CreateHttpClient() => new() { Timeout = DefaultTimeout };

    public async Task<LatestResult> GetLatestAsync(string managerUrl, CancellationToken cancel = default)
    {
        if (!Uri.TryCreate(managerUrl.TrimEnd('/') + "/versions/latest", UriKind.Absolute, out var uri)
            || uri.Scheme is not ("http" or "https"))
        {
            return Unreachable($"bad manager address '{managerUrl}'");
        }
        string body;
        int code;
        try
        {
            using var response = await http.GetAsync(uri, cancel);
            code = (int)response.StatusCode;
            body = await response.Content.ReadAsStringAsync(cancel);
        }
        catch (HttpRequestException e)
        {
            return Unreachable(e.Message);
        }
        catch (OperationCanceledException)
        {
            return Unreachable(cancel.IsCancellationRequested ? "canceled" : $"no answer within {http.Timeout.TotalSeconds:0.#} s");
        }
        if (code < 200 || code >= 300)
        {
            return Refused($"HTTP {code}{ReadError(body)}");
        }
        try
        {
            return new LatestResult(LatestOutcome.Ok, Parse(body), "");
        }
        catch (Exception e) when (e is JsonException or InvalidDataException or InvalidOperationException)
        {
            return Refused(e.Message);
        }
    }

    private static LatestVersion Parse(string body)
    {
        using var document = JsonDocument.Parse(body);
        var root = document.RootElement;
        if (root.ValueKind != JsonValueKind.Object)
        {
            throw new InvalidDataException("answer is not a JSON object");
        }
        if (!root.TryGetProperty("format", out var format) || !format.TryGetInt32(out var number)
            || number < 1 || number > CurrentFormat)
        {
            throw new InvalidDataException($"answer format unknown (this launcher reads up to {CurrentFormat})");
        }
        var version = ReadString(root, "version");
        if (string.IsNullOrEmpty(version))
        {
            throw new InvalidDataException("answer without version");
        }
        if (!root.TryGetProperty("manifest", out var manifest) || manifest.ValueKind != JsonValueKind.Object)
        {
            throw new InvalidDataException("answer without file list");
        }
        var patchesFrom = new List<string>();
        if (root.TryGetProperty("patchesFrom", out var rows) && rows.ValueKind == JsonValueKind.Array)
        {
            patchesFrom.AddRange(rows.EnumerateArray().Where(r => r.ValueKind == JsonValueKind.String).Select(r => r.GetString()!));
        }
        return new LatestVersion(version, VersionManifest.FromJson(manifest.GetRawText()), ReadString(root, "notes") ?? "",
            patchesFrom, ReadString(root, "launcherVersion") ?? "");
    }

    /** ": <error> (<message>)" from the manager's error body, or nothing if the body is not one. */
    private static string ReadError(string body)
    {
        try
        {
            using var document = JsonDocument.Parse(body);
            if (document.RootElement.ValueKind == JsonValueKind.Object && ReadString(document.RootElement, "error") is { } error)
            {
                return $": {error} ({ReadString(document.RootElement, "message")})";
            }
        }
        catch (JsonException)
        {
        }
        return "";
    }

    private static string? ReadString(JsonElement row, string name)
        => row.TryGetProperty(name, out var value) && value.ValueKind == JsonValueKind.String ? value.GetString() : null;

    private static LatestResult Unreachable(string message) => new(LatestOutcome.Unreachable, null, message);

    private static LatestResult Refused(string message) => new(LatestOutcome.Refused, null, message);
}
