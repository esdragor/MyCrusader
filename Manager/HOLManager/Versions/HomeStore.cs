using System.Collections.Concurrent;
using HOLUpdate;

namespace HOLManager.Versions;

/** One announcement in the asked language, as GET /launcher/home sends it. */
public sealed record HomeAnnouncement(string Id, string Date, string Title, string Body, string? Link);

/** What GET /launcher/home tells the launcher: the parts in one language, and whether a key art is there. */
public sealed record HomePage(IReadOnlyList<HomeAnnouncement> Announcements, IReadOnlyList<StoredLink> Links, bool HasBackground);

/** The key art to serve: its file, type, and what the launcher's cache compares (ETag, Last-Modified). */
public sealed record HomeBackground(string Path, string ContentType, DateTime LastWriteUtc, string ETag);

/**
 * The launcher's home page (PROTOCOL.md, "Accueil du lanceur"), read from <versions>/home/ (HOLUpdate.HomeFiles) at each
 * request, like the versions: the host's tool may change it while the manager runs. Each part stands alone: a file
 * unreadable or invalid gives that part empty (logged), never an error for the whole page. A warning is logged once per
 * state of a file (size, write time), not at each launcher's refresh.
 */
public sealed class HomeStore(string dir, ILogger<HomeStore> logger)
{
    private readonly ConcurrentDictionary<string, (long Size, DateTime Written)> warned = new();

    public string Dir { get; } = dir;

    /** language: two lowercase letters (VersionStore.IsLanguage), anything else French. */
    public HomePage Read(string? language)
    {
        var lang = VersionStore.IsLanguage(language) ? language : HomeFiles.FallbackLanguage;
        var announcements = HomeFiles.NewestFirst(ReadPart(HomeFiles.AnnouncementsFile, HomeFiles.ReadAnnouncements))
            .Take(HomeFiles.MaxServed)
            .Select(a => new HomeAnnouncement(a.Id, HomeFiles.FormatDate(a.DateUtc), HomeFiles.Pick(a.Title, lang),
                HomeFiles.Pick(a.Body, lang), a.Link))
            .ToList();
        var links = ReadPart(HomeFiles.LinksFile, HomeFiles.ReadLinks);
        return new HomePage(announcements, links, Background() != null);
    }

    /** The key art, null without one (or one whose first bytes are neither JPEG nor PNG). */
    public HomeBackground? Background()
    {
        try
        {
            if (HomeFiles.FindBackground(Dir) is not { } file)
            {
                return null;
            }
            var info = new FileInfo(file);
            var head = new byte[8];
            int read;
            using (var stream = File.OpenRead(file))
            {
                read = stream.ReadAtLeast(head, head.Length, throwOnEndOfStream: false);
            }
            if (HomeFiles.ImageType(head.AsSpan(0, read)) is not { } type)
            {
                Warn(file, "neither a JPEG nor a PNG image: not served");
                return null;
            }
            // Size and write time: a new image gives a new tag without hashing 4 MB at each request.
            var tag = $"\"{info.LastWriteTimeUtc.Ticks:x}-{info.Length:x}\"";
            return new HomeBackground(file, type, info.LastWriteTimeUtc, tag);
        }
        catch (Exception e) when (e is IOException or UnauthorizedAccessException)
        {
            logger.LogWarning("Launcher home: background unreadable in {Dir}: {Message}", Dir, e.Message);
            return null;
        }
    }

    private IReadOnlyList<T> ReadPart<T>(string name, Func<string, ICollection<string>, IReadOnlyList<T>> read)
    {
        var file = Path.Combine(Dir, name);
        var skipped = new List<string>();
        try
        {
            var rows = read(file, skipped);
            if (skipped.Count > 0)
            {
                Warn(file, string.Join("; ", skipped));
            }
            return rows;
        }
        catch (Exception e) when (e is InvalidDataException or IOException or UnauthorizedAccessException)
        {
            Warn(file, $"{e.Message}; shown empty");
            return [];
        }
    }

    private void Warn(string file, string message)
    {
        (long, DateTime) state;
        try
        {
            var info = new FileInfo(file);
            state = (info.Exists ? info.Length : -1, info.Exists ? info.LastWriteTimeUtc : default);
        }
        catch (Exception e) when (e is IOException or UnauthorizedAccessException)
        {
            state = (-1, default);
        }
        if (warned.TryGetValue(file, out var last) && last == state)
        {
            return;
        }
        warned[file] = state;
        logger.LogWarning("Launcher home: {File}: {Message}", file, message);
    }
}
