using System.Globalization;
using System.Text.Encodings.Web;
using System.Text.Json;
using System.Text.Json.Nodes;
using System.Text.RegularExpressions;

namespace HOLUpdate;

/**
 * One announcement as the host stores it (<versions>/home/announcements.json): every text in each language of the
 * launcher (fr, en, ja), keyed by language. The manager picks one language per request; the host's tool writes them.
 */
public sealed record StoredAnnouncement(
    string Id, DateTime DateUtc, IReadOnlyDictionary<string, string> Title, IReadOnlyDictionary<string, string> Body,
    string? Link = null);

/** One community link as stored in <versions>/home/links.json; Kind is one of HomeFiles.LinkKinds. */
public sealed record StoredLink(string Kind, string Url);

/**
 * The launcher home page's files on the host (PROTOCOL.md, "Accueil du lanceur"), in <versions>/home/ beside latest.txt:
 * announcements.json, links.json and an optional background.jpg / background.png. Shared here so that the format has
 * one source: the manager reads it (HomeStore, tolerant: a bad entry is skipped, never the whole page), the host's tool
 * writes it (HOLPublish home, strict: every language or nothing). "home" is therefore never a version name.
 */
public static partial class HomeFiles
{
    public const string DirName = "home";
    public const string AnnouncementsFile = "announcements.json";
    public const string LinksFile = "links.json";
    public const string BackgroundName = "background";

    /** The launcher's languages; French first, the language every text falls back to. */
    public static readonly IReadOnlyList<string> Languages = ["fr", "en", "ja"];

    public const string FallbackLanguage = "fr";

    /** The icons the launcher knows; any other kind read from links.json is shown as "other". */
    public static readonly IReadOnlyList<string> LinkKinds = ["discord", "youtube", "website", "other"];

    /** Key art cap: the launcher downloads it over Tailscale and keeps it in memory. */
    public const long MaxBackgroundBytes = 4 * 1024 * 1024;

    /** The manager serves at most this many announcements, newest first (older ones stay in the file). */
    public const int MaxServed = 20;

    public const string JpegType = "image/jpeg";
    public const string PngType = "image/png";

    private static readonly JsonSerializerOptions Indented = new()
    {
        WriteIndented = true,
        // Japanese and accented French stay readable in the file: the host may open it.
        Encoder = JavaScriptEncoder.UnsafeRelaxedJsonEscaping,
    };

    [GeneratedRegex("^[A-Za-z0-9._-]{1,64}$")]
    private static partial Regex IdPattern();

    /** Announcement ids: [A-Za-z0-9._-], 1 to 64 characters (typed on the command line, shown in "list"). */
    public static bool IsValidId(string? id) => id != null && IdPattern().IsMatch(id);

    /** An absolute http or https address: the only links the launcher opens. */
    public static bool IsHttpUrl(string? url) =>
        url != null && Uri.TryCreate(url, UriKind.Absolute, out var uri) && uri.Scheme is "http" or "https";

    /** The kind as the launcher shows it: a known kind in lower case, anything else "other". */
    public static string NormalizeKind(string? kind)
    {
        var lower = kind?.Trim().ToLowerInvariant() ?? "";
        return LinkKinds.Contains(lower) ? lower : "other";
    }

    /** "image/jpeg" or "image/png" from the first bytes (the extension can lie), else null. */
    public static string? ImageType(ReadOnlySpan<byte> head)
    {
        if (head.Length >= 3 && head[0] == 0xFF && head[1] == 0xD8 && head[2] == 0xFF)
        {
            return JpegType;
        }
        if (head.Length >= 8 && head[..8].SequenceEqual(new byte[] { 0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A }))
        {
            return PngType;
        }
        return null;
    }

    /** ".jpg" / ".png" for a type of ImageType. */
    public static string ExtensionOf(string imageType) => imageType == PngType ? ".png" : ".jpg";

    /** The background file in this home folder: background.jpg or background.png, the newest if both; null without. */
    public static string? FindBackground(string homeDir) =>
        new[] { ".jpg", ".png" }
            .Select(extension => Path.Combine(homeDir, BackgroundName + extension))
            .Where(File.Exists)
            .OrderByDescending(File.GetLastWriteTimeUtc)
            .FirstOrDefault();

    /** The text in that language, else French, else the first language that has one; "" without any. */
    public static string Pick(IReadOnlyDictionary<string, string> texts, string? language)
    {
        foreach (var candidate in new[] { language, FallbackLanguage }.Concat(Languages).Concat(texts.Keys))
        {
            if (candidate != null && texts.TryGetValue(candidate, out var text) && !string.IsNullOrWhiteSpace(text))
            {
                return text.Trim();
            }
        }
        return "";
    }

    /**
     * The announcements of the file, as stored (no order); none when it is missing. InvalidDataException when it is not
     * a JSON array (the reader then shows none); a bad entry is left out and said in skipped (so is a link dropped).
     */
    public static IReadOnlyList<StoredAnnouncement> ReadAnnouncements(string file, ICollection<string>? skipped = null)
    {
        var announcements = new List<StoredAnnouncement>();
        foreach (var (row, index) in ReadArray(file).Select((row, index) => (row, index)))
        {
            if (ParseAnnouncement(row, out var why) is { } announcement)
            {
                announcements.Add(announcement);
                if (why != "")
                {
                    skipped?.Add($"entry {index}: {why}");
                }
            }
            else
            {
                skipped?.Add($"entry {index} left out ({why})");
            }
        }
        return announcements;
    }

    /** The links of the file, kinds normalized; none when it is missing; same failures as ReadAnnouncements. */
    public static IReadOnlyList<StoredLink> ReadLinks(string file, ICollection<string>? skipped = null)
    {
        var links = new List<StoredLink>();
        foreach (var (row, index) in ReadArray(file).Select((row, index) => (row, index)))
        {
            var url = row is JsonObject o ? ReadString(o, "url") : null;
            if (!IsHttpUrl(url))
            {
                skipped?.Add($"entry {index} left out (url is not an http(s) address)");
                continue;
            }
            links.Add(new StoredLink(NormalizeKind(ReadString((JsonObject)row!, "kind")), url!));
        }
        return links;
    }

    /** Newest first, written beside then moved over the file: the manager reads the old list or the new one. */
    public static void WriteAnnouncements(string file, IEnumerable<StoredAnnouncement> announcements)
    {
        var rows = new JsonArray();
        foreach (var a in NewestFirst(announcements))
        {
            var row = new JsonObject
            {
                ["id"] = a.Id,
                ["date"] = FormatDate(a.DateUtc),
                ["title"] = Texts(a.Title),
                ["body"] = Texts(a.Body),
            };
            if (a.Link != null)
            {
                row["link"] = a.Link;
            }
            rows.Add(row);
        }
        WriteAtomically(file, rows.ToJsonString(Indented));
    }

    public static void WriteLinks(string file, IEnumerable<StoredLink> links)
    {
        var rows = new JsonArray();
        foreach (var link in links)
        {
            rows.Add(new JsonObject { ["kind"] = link.Kind, ["url"] = link.Url });
        }
        WriteAtomically(file, rows.ToJsonString(Indented));
    }

    /** Newest first, then by id (a stable order for two announcements of the same minute). */
    public static IEnumerable<StoredAnnouncement> NewestFirst(IEnumerable<StoredAnnouncement> announcements) =>
        announcements.OrderByDescending(a => a.DateUtc).ThenBy(a => a.Id, StringComparer.Ordinal);

    /** ISO 8601 UTC to the second, "2026-11-24T18:00:00Z" (what the route and the file both carry). */
    public static string FormatDate(DateTime utc) =>
        utc.ToUniversalTime().ToString("yyyy-MM-dd'T'HH:mm:ss'Z'", CultureInfo.InvariantCulture);

    /** An ISO 8601 date; one without offset is UTC. */
    public static bool TryParseDate(string? text, out DateTime utc)
    {
        if (text != null && DateTime.TryParse(text.Trim(), CultureInfo.InvariantCulture,
                DateTimeStyles.AssumeUniversal | DateTimeStyles.AdjustToUniversal, out utc))
        {
            utc = DateTime.SpecifyKind(utc, DateTimeKind.Utc);
            return true;
        }
        utc = default;
        return false;
    }

    /** Written to <file>.tmp, then moved over the file; the folder is created. */
    public static void WriteAtomically(string file, string text)
    {
        Directory.CreateDirectory(Path.GetDirectoryName(Path.GetFullPath(file))!);
        var temporary = file + ".tmp";
        File.WriteAllText(temporary, text + Environment.NewLine);
        File.Move(temporary, file, overwrite: true);
    }

    private static JsonArray ReadArray(string file)
    {
        if (!File.Exists(file))
        {
            return [];
        }
        JsonNode? root;
        try
        {
            root = JsonNode.Parse(File.ReadAllText(file), documentOptions: new JsonDocumentOptions
            {
                // Hand-edited files: a trailing comma or a comment is not worth losing the page.
                AllowTrailingCommas = true,
                CommentHandling = JsonCommentHandling.Skip,
            });
        }
        catch (JsonException e)
        {
            throw new InvalidDataException($"{file} is not valid JSON: {e.Message}");
        }
        return root as JsonArray ?? throw new InvalidDataException($"{file} is not a JSON array");
    }

    private static StoredAnnouncement? ParseAnnouncement(JsonNode? row, out string why)
    {
        if (row is not JsonObject o)
        {
            why = "not an object";
            return null;
        }
        var id = ReadString(o, "id");
        if (!IsValidId(id))
        {
            why = "id missing or not [A-Za-z0-9._-]";
            return null;
        }
        if (!TryParseDate(ReadString(o, "date"), out var date))
        {
            why = "date missing or not ISO 8601";
            return null;
        }
        var title = ReadTexts(o["title"]);
        if (Pick(title, null) == "")
        {
            why = "no title";
            return null;
        }
        var link = ReadString(o, "link");
        why = link == null || IsHttpUrl(link) ? "" : "link dropped (not an http(s) address)";
        return new StoredAnnouncement(id!, date, title, ReadTexts(o["body"]), IsHttpUrl(link) ? link : null);
    }

    /** { "fr": "...", "en": "..." }; a plain string is taken as French (a file written by hand). */
    private static IReadOnlyDictionary<string, string> ReadTexts(JsonNode? node)
    {
        var texts = new Dictionary<string, string>(StringComparer.Ordinal);
        if (node is JsonValue value && value.TryGetValue<string>(out var single))
        {
            texts[FallbackLanguage] = single;
        }
        else if (node is JsonObject o)
        {
            foreach (var (language, text) in o)
            {
                if (text is JsonValue v && v.TryGetValue<string>(out var s))
                {
                    texts[language] = s;
                }
            }
        }
        return texts;
    }

    private static JsonObject Texts(IReadOnlyDictionary<string, string> texts)
    {
        var o = new JsonObject();
        foreach (var language in Languages.Where(texts.ContainsKey).Concat(texts.Keys.Where(k => !Languages.Contains(k))))
        {
            o[language] = texts[language];
        }
        return o;
    }

    private static string? ReadString(JsonObject o, string name) =>
        o[name] is JsonValue value && value.TryGetValue<string>(out var text) ? text : null;
}
