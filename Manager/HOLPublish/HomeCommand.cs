using System.Globalization;
using HOLUpdate;

namespace HOLPublish;

/**
 * Launcher 5: the host's tool for the launcher's home page (PROTOCOL.md, "Accueil du lanceur"), driven by
 * Tools/Manager/home.ps1. Writes <versions>/home/ (HOLUpdate.HomeFiles) while the manager runs: each file is written
 * beside then moved over the old one, so the manager reads the old page or the new one. Strict where the manager is
 * tolerant: an announcement is written in every language of the launcher or not at all (the user's rule for the
 * release notes, 2026-11-24, applied to announcements), and a file holding entries the tool could not keep is never
 * rewritten (fix it by hand first). Lines start with "HOLPublish : "; the last is PASS or FAIL; exit code 0 / 1.
 */
public static class HomeCommand
{
    public const string Usage =
        "HOLPublish home list|add|remove|link|unlink|background --versions <versions folder> "
        + "[--id <id>] [--from <folder with title.txt, title.en.txt, title.ja.txt, body.txt, body.en.txt, body.ja.txt>] "
        + "[--link <http(s) address>] [--date <ISO 8601, default now>] [--kind discord|youtube|website|other] [--url <http(s)>] "
        + "[--image <file.jpg|png>] [--remove]";

    /** Flags without a value; every other option takes one. */
    private static readonly HashSet<string> Flags = ["--remove"];

    /** The file of one text in one language inside --from: French is <part>.txt, the others <part>.<language>.txt. */
    public static string TextFile(string folder, string part, string language) =>
        Path.Combine(folder, language == HomeFiles.FallbackLanguage ? $"{part}.txt" : $"{part}.{language}.txt");

    public static int Run(IReadOnlyList<string> args, TextWriter output, Func<DateTime>? utcNow = null)
    {
        try
        {
            var verb = args.Count > 0 ? args[0] : "";
            var options = Parse(args.Skip(1).ToList());
            var versions = Required(options, "--versions");
            if (!Directory.Exists(versions))
            {
                throw new ArgumentException($"versions folder not found: {Path.GetFullPath(versions)}");
            }
            var home = Path.Combine(Path.GetFullPath(versions), HomeFiles.DirName);
            var result = verb switch
            {
                "list" => List(home, output),
                "add" => Add(home, options, (utcNow ?? (() => DateTime.UtcNow))()),
                "remove" => Remove(home, Required(options, "--id")),
                "link" => Link(home, Required(options, "--kind"), Required(options, "--url")),
                "unlink" => Unlink(home, Required(options, "--kind")),
                "background" => Background(home, options),
                _ => throw new ArgumentException($"unknown home command '{verb}'. Usage: {Usage}"),
            };
            output.WriteLine($"HOLPublish : PASS {result}");
            return 0;
        }
        catch (Exception e) when (e is ArgumentException or InvalidDataException or IOException or UnauthorizedAccessException)
        {
            output.WriteLine($"HOLPublish : FAIL ({e.Message})");
            return 1;
        }
    }

    private static Dictionary<string, string> Parse(IReadOnlyList<string> args)
    {
        var options = new Dictionary<string, string>();
        for (var i = 0; i < args.Count; i++)
        {
            var name = args[i];
            if (!name.StartsWith("--", StringComparison.Ordinal))
            {
                throw new ArgumentException($"unexpected '{name}'. Usage: {Usage}");
            }
            if (Flags.Contains(name))
            {
                options[name] = "";
                continue;
            }
            if (i + 1 >= args.Count)
            {
                throw new ArgumentException($"{name} without a value. Usage: {Usage}");
            }
            options[name] = args[++i];
        }
        return options;
    }

    private static string Required(Dictionary<string, string> options, string name) =>
        options.TryGetValue(name, out var value) && value.Trim() != ""
            ? value
            : throw new ArgumentException($"{name} is required. Usage: {Usage}");

    private static string List(string home, TextWriter output)
    {
        var announcements = HomeFiles.NewestFirst(ReadAnnouncements(home)).ToList();
        foreach (var a in announcements)
        {
            var languages = string.Join("/", HomeFiles.Languages.Where(l =>
                !string.IsNullOrWhiteSpace(a.Title.GetValueOrDefault(l)) && !string.IsNullOrWhiteSpace(a.Body.GetValueOrDefault(l))));
            output.WriteLine($"HOLPublish : announcement {a.Id} {HomeFiles.FormatDate(a.DateUtc)} \"{HomeFiles.Pick(a.Title, null)}\" "
                + $"[{languages}]{(a.Link != null ? " " + a.Link : "")}");
        }
        var links = ReadLinks(home);
        foreach (var link in links)
        {
            output.WriteLine($"HOLPublish : link {link.Kind} {link.Url}");
        }
        var background = HomeFiles.FindBackground(home);
        if (background != null)
        {
            output.WriteLine($"HOLPublish : background {background} ({new FileInfo(background).Length / 1024} Ko)");
        }
        var served = announcements.Count > HomeFiles.MaxServed ? $" ({HomeFiles.MaxServed} newest served)" : "";
        return $"{announcements.Count} announcement(s){served}, {links.Count} link(s), "
            + (background != null ? "a background" : "no background");
    }

    private static string Add(string home, Dictionary<string, string> options, DateTime now)
    {
        var id = Required(options, "--id");
        if (!HomeFiles.IsValidId(id))
        {
            throw new ArgumentException($"invalid id '{id}' (letters, digits, '.', '_', '-'; 64 at most)");
        }
        var from = Required(options, "--from");
        if (!Directory.Exists(from))
        {
            throw new ArgumentException($"folder not found: {Path.GetFullPath(from)}");
        }
        string? link = null;
        if (options.TryGetValue("--link", out var givenLink))
        {
            link = givenLink.Trim();
            if (!HomeFiles.IsHttpUrl(link))
            {
                throw new ArgumentException($"--link must be an http(s) address, not '{givenLink}'");
            }
        }
        DateTime? date = null;
        if (options.TryGetValue("--date", out var givenDate))
        {
            date = HomeFiles.TryParseDate(givenDate, out var parsed)
                ? parsed
                : throw new ArgumentException($"--date must be an ISO 8601 date (2026-11-24T18:00:00Z), not '{givenDate}'");
        }
        var (title, body) = ReadTexts(from);
        var announcements = ReadAnnouncements(home).ToList();
        var old = announcements.FirstOrDefault(a => a.Id == id);
        // Replacing keeps the first date unless another is given: fixing a typo does not move the announcement up.
        var added = new StoredAnnouncement(id, date ?? old?.DateUtc ?? Truncate(now), title, body, link);
        announcements.RemoveAll(a => a.Id == id);
        announcements.Add(added);
        HomeFiles.WriteAnnouncements(Path.Combine(home, HomeFiles.AnnouncementsFile), announcements);
        return $"{(old != null ? "replaced" : "added")} announcement {id} ({HomeFiles.FormatDate(added.DateUtc)}, "
            + $"{announcements.Count} in {HomeFiles.AnnouncementsFile})";
    }

    /**
     * Every text in every language, read before anything is written; the refusal names every missing or empty file.
     * A title is one line (the launcher shows it as a heading); a body keeps its lines ("\n").
     */
    private static (Dictionary<string, string> Title, Dictionary<string, string> Body) ReadTexts(string from)
    {
        var title = new Dictionary<string, string>();
        var body = new Dictionary<string, string>();
        var missing = new List<string>();
        foreach (var language in HomeFiles.Languages)
        {
            foreach (var (part, texts) in new[] { ("title", title), ("body", body) })
            {
                var file = TextFile(from, part, language);
                var text = File.Exists(file) ? File.ReadAllText(file).Replace("\r\n", "\n").Trim() : "";
                if (text == "")
                {
                    missing.Add(Path.GetFileName(file));
                    continue;
                }
                if (part == "title" && text.Contains('\n'))
                {
                    throw new ArgumentException($"{file}: a title is one line");
                }
                texts[language] = text;
            }
        }
        if (missing.Count > 0)
        {
            throw new ArgumentException("an announcement is written in every language of the launcher (fr, en, ja); "
                + $"missing or empty in {Path.GetFullPath(from)}: {string.Join(", ", missing)}");
        }
        return (title, body);
    }

    private static string Remove(string home, string id)
    {
        var announcements = ReadAnnouncements(home).ToList();
        if (announcements.RemoveAll(a => a.Id == id) == 0)
        {
            throw new ArgumentException($"no announcement '{id}' (home list shows them)");
        }
        HomeFiles.WriteAnnouncements(Path.Combine(home, HomeFiles.AnnouncementsFile), announcements);
        return $"removed announcement {id} ({announcements.Count} left)";
    }

    /** One link per kind: setting a kind again replaces its address, in place. */
    private static string Link(string home, string kind, string url)
    {
        var normalized = Kind(kind);
        url = url.Trim();
        if (!HomeFiles.IsHttpUrl(url))
        {
            throw new ArgumentException($"--url must be an http(s) address, not '{url}'");
        }
        var links = ReadLinks(home).ToList();
        var index = links.FindIndex(l => l.Kind == normalized);
        if (index >= 0)
        {
            links[index] = new StoredLink(normalized, url);
        }
        else
        {
            links.Add(new StoredLink(normalized, url));
        }
        HomeFiles.WriteLinks(Path.Combine(home, HomeFiles.LinksFile), links);
        return $"{(index >= 0 ? "replaced" : "added")} link {normalized} {url}";
    }

    private static string Unlink(string home, string kind)
    {
        var normalized = Kind(kind);
        var links = ReadLinks(home).ToList();
        if (links.RemoveAll(l => l.Kind == normalized) == 0)
        {
            throw new ArgumentException($"no {normalized} link (home list shows them)");
        }
        HomeFiles.WriteLinks(Path.Combine(home, HomeFiles.LinksFile), links);
        return $"removed link {normalized}";
    }

    /** --image: a JPEG or PNG (checked on its bytes) of MaxBackgroundBytes at most, copied; --remove: deleted. */
    private static string Background(string home, Dictionary<string, string> options)
    {
        var remove = options.ContainsKey("--remove");
        options.TryGetValue("--image", out var image);
        if (remove == (image != null))
        {
            throw new ArgumentException($"background wants --image <file> or --remove. Usage: {Usage}");
        }
        if (remove)
        {
            var existing = HomeFiles.FindBackground(home) ?? throw new ArgumentException("no background to remove");
            DeleteBackgrounds(home, keep: null);
            return $"removed background {Path.GetFileName(existing)}";
        }
        if (!File.Exists(image))
        {
            throw new ArgumentException($"image not found: {Path.GetFullPath(image!)}");
        }
        var extension = Path.GetExtension(image).ToLowerInvariant();
        if (extension is not (".jpg" or ".jpeg" or ".png"))
        {
            throw new ArgumentException($"the background is a .jpg or a .png, not '{extension}'");
        }
        var size = new FileInfo(image).Length;
        if (size > HomeFiles.MaxBackgroundBytes)
        {
            throw new ArgumentException($"{image} is {Mb(size)} Mo; {Mb(HomeFiles.MaxBackgroundBytes)} Mo at most (the launcher downloads it)");
        }
        var bytes = File.ReadAllBytes(image);
        var type = HomeFiles.ImageType(bytes);
        if (type == null || HomeFiles.ExtensionOf(type) != (extension == ".png" ? ".png" : ".jpg"))
        {
            throw new ArgumentException($"{image} is not a {(extension == ".png" ? "PNG" : "JPEG")} image");
        }
        Directory.CreateDirectory(home);
        var target = Path.Combine(home, HomeFiles.BackgroundName + HomeFiles.ExtensionOf(type));
        var temporary = target + ".tmp";
        File.WriteAllBytes(temporary, bytes);
        File.Move(temporary, target, overwrite: true);
        DeleteBackgrounds(home, keep: target);
        return $"background {Path.GetFileName(target)} ({size / 1024} Ko)";
    }

    private static void DeleteBackgrounds(string home, string? keep)
    {
        foreach (var file in new[] { ".jpg", ".png" }.Select(e => Path.Combine(home, HomeFiles.BackgroundName + e)))
        {
            if (file != keep && File.Exists(file))
            {
                File.Delete(file);
            }
        }
    }

    private static string Kind(string kind)
    {
        var lower = kind.Trim().ToLowerInvariant();
        return HomeFiles.LinkKinds.Contains(lower)
            ? lower
            : throw new ArgumentException($"unknown link kind '{kind}' ({string.Join(", ", HomeFiles.LinkKinds)})");
    }

    /** The file's announcements; refused when the file holds any the tool could not write back (they would be lost). */
    private static IReadOnlyList<StoredAnnouncement> ReadAnnouncements(string home)
    {
        var file = Path.Combine(home, HomeFiles.AnnouncementsFile);
        var skipped = new List<string>();
        var announcements = HomeFiles.ReadAnnouncements(file, skipped);
        Refuse(file, skipped);
        return announcements;
    }

    private static IReadOnlyList<StoredLink> ReadLinks(string home)
    {
        var file = Path.Combine(home, HomeFiles.LinksFile);
        var skipped = new List<string>();
        var links = HomeFiles.ReadLinks(file, skipped);
        Refuse(file, skipped);
        return links;
    }

    private static void Refuse(string file, List<string> skipped)
    {
        if (skipped.Count > 0)
        {
            throw new InvalidDataException($"{file} has entries this tool cannot keep, fix them by hand first: {string.Join("; ", skipped)}");
        }
    }

    private static DateTime Truncate(DateTime utc) =>
        new(utc.Ticks - utc.Ticks % TimeSpan.TicksPerSecond, DateTimeKind.Utc);

    private static string Mb(long bytes) =>
        (bytes / (1024.0 * 1024.0)).ToString("0.0", CultureInfo.InvariantCulture).Replace('.', ',');
}
