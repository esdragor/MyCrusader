using System.Text.Json;
using HOLUpdate;

namespace HOLLauncher;

/**
 * The answer of GET /versions/latest (PROTOCOL.md, "Versions du jeu"). History (L21): the kept versions' notes, newest
 * first; null when the manager is older than L21 (no "history" field), the launcher then shows Notes alone.
 */
public sealed record LatestVersion(
    string Version, VersionManifest Manifest, string Notes, IReadOnlyList<string> PatchesFrom, string LauncherVersion,
    IReadOnlyList<ReleaseNote>? History = null);

/** One version's notes in the history of /versions/latest (L21). */
public sealed record ReleaseNote(string Version, string Notes);

public enum LatestOutcome
{
    Ok,
    /** No answer: bad address, no connection, timeout. */
    Unreachable,
    /** An answer, but an error (404 no_version...) or a body this launcher does not accept. */
    Refused,
    /** 401 bad_key (L11): the kit's access key is not the manager's; the views say it in the player's language (L16). */
    BadKey,
}

/** Latest is set only when Outcome is Ok; Message says why otherwise (technical detail in English, L16). */
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

    /** The launcher's, sending the key of <gameDir>/servers.json (L11). */
    public static HttpClient CreateHttpClient(string gameDir) => AccessKeyHandler.CreateHttpClient(gameDir, DefaultTimeout);

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
        if (AccessKeyHandler.IsBadKey(code, ReadErrorCode(body)))
        {
            return new LatestResult(LatestOutcome.BadKey, null, "HTTP 401 bad_key");
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
            patchesFrom, ReadString(root, "launcherVersion") ?? "", ReadHistory(root));
    }

    /**
     * L21: "history", null without it. Only news: a bad row is skipped instead of refusing the answer (the update itself
     * does not depend on it).
     */
    private static IReadOnlyList<ReleaseNote>? ReadHistory(JsonElement root)
    {
        if (!root.TryGetProperty("history", out var rows) || rows.ValueKind != JsonValueKind.Array)
        {
            return null;
        }
        return rows.EnumerateArray()
            .Where(r => r.ValueKind == JsonValueKind.Object && !string.IsNullOrEmpty(ReadString(r, "version")))
            .Select(r => new ReleaseNote(ReadString(r, "version")!, ReadString(r, "notes") ?? ""))
            .ToList();
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

    /** The error code of the manager's error body, or null. */
    private static string? ReadErrorCode(string body)
    {
        try
        {
            using var document = JsonDocument.Parse(body);
            return document.RootElement.ValueKind == JsonValueKind.Object ? ReadString(document.RootElement, "error") : null;
        }
        catch (JsonException)
        {
            return null;
        }
    }

    private static string? ReadString(JsonElement row, string name)
        => row.TryGetProperty(name, out var value) && value.ValueKind == JsonValueKind.String ? value.GetString() : null;

    private static LatestResult Unreachable(string message) => new(LatestOutcome.Unreachable, null, message);

    private static LatestResult Refused(string message) => new(LatestOutcome.Refused, null, message);
}
