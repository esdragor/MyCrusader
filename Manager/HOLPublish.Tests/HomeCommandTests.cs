using HOLManager.Versions;
using HOLUpdate;
using Microsoft.Extensions.Logging.Abstractions;

namespace HOLPublish.Tests;

/**
 * Launcher 5: "HOLPublish home ...", the host's tool for the launcher's home page (Tools/Manager/home.ps1 wraps it).
 * Strict: an announcement in every language or nothing written (the user's rule for the notes, applied here), and a file
 * it could not write back whole is never rewritten. What it writes is read back by the manager's own HomeStore.
 */
public sealed class HomeCommandTests : IDisposable
{
    private static readonly DateTime Now = new(2026, 11, 24, 18, 0, 0, 500, DateTimeKind.Utc);
    private static readonly byte[] Jpeg = [0xFF, 0xD8, 0xFF, 0xE0, 1, 2, 3, 4];
    private static readonly byte[] Png = [0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A, 5, 6];

    private readonly TempDir dir = new();

    public HomeCommandTests()
    {
        Directory.CreateDirectory(Versions);
    }

    public void Dispose() => dir.Dispose();

    private string Versions => Path.Combine(dir.Root, "Versions");
    private string Home => Path.Combine(Versions, HomeFiles.DirName);
    private string AnnouncementsFile => Path.Combine(Home, HomeFiles.AnnouncementsFile);

    private (int Code, string Output) Run(params string[] args)
    {
        var output = new StringWriter();
        var code = HomeCommand.Run(args.Take(1).Concat(["--versions", Versions]).Concat(args.Skip(1)).ToList(), output, () => Now);
        return (code, output.ToString().Trim());
    }

    /** A --from folder with every text (fr in title.txt / body.txt, en and ja beside), minus the files named in "without". */
    private string Texts(string name, params string[] without)
    {
        foreach (var (file, text) in new[]
                 {
                     ("title.txt", $"Titre {name}"), ("title.en.txt", $"Title {name}"), ("title.ja.txt", $"タイトル {name}"),
                     ("body.txt", "Ligne 1\r\nLigne 2\r\n"), ("body.en.txt", "Line 1\nLine 2"), ("body.ja.txt", "本文"),
                 })
        {
            if (!without.Contains(file))
            {
                dir.Write($"{name}/{file}", text);
            }
        }
        return Path.Combine(dir.Root, name);
    }

    private HomePage Served(string? language) => new HomeStore(Home, NullLogger<HomeStore>.Instance).Read(language);

    private static void AssertPass((int Code, string Output) result)
    {
        Assert.True(result.Code == 0, result.Output);
        Assert.StartsWith("HOLPublish : PASS ", result.Output.Split('\n')[^1].Trim());
    }

    private static void AssertFail((int Code, string Output) result, string reason)
    {
        Assert.Equal(1, result.Code);
        Assert.StartsWith("HOLPublish : FAIL (", result.Output);
        Assert.Contains(reason, result.Output);
    }

    [Fact]
    public void AddWritesEveryLanguageAndTheManagerServesIt()
    {
        AssertPass(Run("add", "--id", "map", "--from", Texts("map"), "--link", "https://example.org/map"));

        var english = Assert.Single(Served("en").Announcements);
        Assert.Equal(new HomeAnnouncement("map", "2026-11-24T18:00:00Z", "Title map", "Line 1\nLine 2", "https://example.org/map"), english);
        Assert.Equal("Titre map", Served(null).Announcements[0].Title);
        Assert.Equal("Ligne 1\nLigne 2", Served("fr").Announcements[0].Body);
        Assert.Equal("タイトル map", Served("ja").Announcements[0].Title);
        Assert.Equal([HomeFiles.AnnouncementsFile], Directory.GetFiles(Home).Select(Path.GetFileName));
        // Readable by the host: Japanese is not escaped.
        Assert.Contains("タイトル map", File.ReadAllText(AnnouncementsFile));
    }

    [Theory]
    [InlineData("title.txt")]
    [InlineData("title.en.txt")]
    [InlineData("title.ja.txt")]
    [InlineData("body.txt")]
    [InlineData("body.en.txt")]
    [InlineData("body.ja.txt")]
    public void AMissingLanguageIsRefusedAndNothingWritten(string missing)
    {
        var result = Run("add", "--id", "a", "--from", Texts("a", missing));
        AssertFail(result, "every language");
        Assert.Contains(missing, result.Output);
        Assert.False(Directory.Exists(Home));
    }

    [Fact]
    public void ABlankTranslationIsRefusedAndTheFileLeftAsItWas()
    {
        AssertPass(Run("add", "--id", "first", "--from", Texts("first")));
        var before = File.ReadAllText(AnnouncementsFile);
        var from = Texts("second");
        File.WriteAllText(Path.Combine(from, "body.ja.txt"), " \r\n ");
        File.Delete(Path.Combine(from, "title.en.txt"));
        var result = Run("add", "--id", "second", "--from", from);
        AssertFail(result, "body.ja.txt");
        Assert.Contains("title.en.txt", result.Output);
        Assert.Equal(before, File.ReadAllText(AnnouncementsFile));
    }

    [Fact]
    public void ATitleIsOneLine()
    {
        var from = Texts("a");
        File.WriteAllText(Path.Combine(from, "title.en.txt"), "Two\nlines");
        AssertFail(Run("add", "--id", "a", "--from", from), "one line");
        Assert.False(Directory.Exists(Home));
    }

    [Theory]
    [InlineData("--id", "bad id", "invalid id")]
    [InlineData("--link", "javascript:alert(1)", "--link")]
    [InlineData("--link", "C:\\x.html", "--link")]
    [InlineData("--date", "hier", "--date")]
    public void BadOptionsAreRefused(string option, string value, string reason)
    {
        var args = new Dictionary<string, string> { ["--id"] = "a", ["--from"] = Texts("a"), [option] = value };
        AssertFail(Run(["add", .. args.SelectMany(p => new[] { p.Key, p.Value })]), reason);
        Assert.False(Directory.Exists(Home));
    }

    [Fact]
    public void AnUnknownFolderOrVersionsFolderIsRefused()
    {
        AssertFail(Run("add", "--id", "a", "--from", Path.Combine(dir.Root, "none")), "folder not found");
        var output = new StringWriter();
        Assert.Equal(1, HomeCommand.Run(["list", "--versions", Path.Combine(dir.Root, "nowhere")], output));
        Assert.Contains("versions folder not found", output.ToString());
        AssertFail(Run("publish"), "unknown home command");
        AssertFail(Run("add", "--id"), "without a value");
    }

    [Fact]
    public void AddingTheSameIdReplacesItKeepingItsDate()
    {
        AssertPass(Run("add", "--id", "a", "--from", Texts("a"), "--date", "2026-11-01T10:00:00Z"));
        var result = Run("add", "--id", "a", "--from", Texts("fixed"));
        AssertPass(result);
        Assert.Contains("replaced", result.Output);
        var announcement = Assert.Single(Served("en").Announcements);
        Assert.Equal("Title fixed", announcement.Title);
        Assert.Equal("2026-11-01T10:00:00Z", announcement.Date);
        Assert.Null(announcement.Link);

        AssertPass(Run("add", "--id", "a", "--from", Texts("fixed"), "--date", "2026-11-20T08:30:00+01:00"));
        Assert.Equal("2026-11-20T07:30:00Z", Served("en").Announcements[0].Date);
    }

    [Fact]
    public void TheFileIsKeptNewestFirstAndRemoveTakesOne()
    {
        AssertPass(Run("add", "--id", "old", "--from", Texts("old"), "--date", "2026-10-01T00:00:00Z"));
        AssertPass(Run("add", "--id", "new", "--from", Texts("new")));
        AssertPass(Run("add", "--id", "mid", "--from", Texts("mid"), "--date", "2026-11-01T00:00:00Z"));
        var stored = HomeFiles.ReadAnnouncements(AnnouncementsFile);
        Assert.Equal(["new", "mid", "old"], stored.Select(a => a.Id));

        AssertPass(Run("remove", "--id", "mid"));
        Assert.Equal(["new", "old"], Served(null).Announcements.Select(a => a.Id));
        AssertFail(Run("remove", "--id", "mid"), "no announcement 'mid'");
        Assert.Empty(Directory.GetFiles(Home, "*.tmp"));
    }

    [Fact]
    public void AFileWithEntriesTheToolCannotKeepIsNeverRewritten()
    {
        Directory.CreateDirectory(Home);
        var text = """[ { "id": "hand written", "date": "2026-11-01T00:00:00Z", "title": "x" } ]""";
        File.WriteAllText(AnnouncementsFile, text);
        AssertFail(Run("add", "--id", "a", "--from", Texts("a")), "fix them by hand");
        Assert.Equal(text, File.ReadAllText(AnnouncementsFile));
        File.WriteAllText(AnnouncementsFile, "not json");
        AssertFail(Run("remove", "--id", "a"), "not valid JSON");
        Assert.Equal("not json", File.ReadAllText(AnnouncementsFile));
    }

    [Fact]
    public void LinksOnePerKindReplacedInPlaceAndRemoved()
    {
        AssertPass(Run("link", "--kind", "discord", "--url", "https://discord.gg/old"));
        AssertPass(Run("link", "--kind", "YouTube", "--url", "https://youtube.com/@hol"));
        AssertPass(Run("link", "--kind", "discord", "--url", "https://discord.gg/new"));
        Assert.Equal([new StoredLink("discord", "https://discord.gg/new"), new StoredLink("youtube", "https://youtube.com/@hol")],
            Served(null).Links);

        AssertFail(Run("link", "--kind", "twitch", "--url", "https://twitch.tv/x"), "unknown link kind");
        AssertFail(Run("link", "--kind", "website", "--url", "file:///C:/x"), "--url");
        AssertPass(Run("unlink", "--kind", "discord"));
        AssertFail(Run("unlink", "--kind", "discord"), "no discord link");
        Assert.Equal(["youtube"], Served(null).Links.Select(l => l.Kind));
    }

    [Fact]
    public void BackgroundIsCopiedAndReplacesTheOtherType()
    {
        var jpeg = dir.Write("art/key.JPEG", Jpeg);
        AssertPass(Run("background", "--image", jpeg));
        Assert.Equal(Jpeg, File.ReadAllBytes(Path.Combine(Home, "background.jpg")));
        Assert.True(Served(null).HasBackground);

        AssertPass(Run("background", "--image", dir.Write("art/key.png", Png)));
        Assert.Equal(["background.png"], Directory.GetFiles(Home).Select(Path.GetFileName));

        AssertPass(Run("background", "--remove"));
        Assert.Empty(Directory.GetFiles(Home));
        Assert.False(Served(null).HasBackground);
        AssertFail(Run("background", "--remove"), "no background");
    }

    [Fact]
    public void BackgroundRefusals()
    {
        AssertFail(Run("background", "--image", dir.Write("art/a.gif", "GIF89a")), "not '.gif'");
        AssertFail(Run("background", "--image", dir.Write("art/fake.jpg", "<html>")), "not a JPEG");
        AssertFail(Run("background", "--image", dir.Write("art/png.jpg", Png)), "not a JPEG");
        var big = new byte[HomeFiles.MaxBackgroundBytes + 1];
        Jpeg.CopyTo(big, 0);
        AssertFail(Run("background", "--image", dir.Write("art/big.jpg", big)), "at most");
        AssertFail(Run("background", "--image", Path.Combine(dir.Root, "none.jpg")), "image not found");
        AssertFail(Run("background"), "--image <file> or --remove");
        Assert.False(Directory.Exists(Home));
    }

    [Fact]
    public void ListShowsEverything()
    {
        AssertPass(Run("add", "--id", "a", "--from", Texts("a"), "--link", "https://example.org"));
        AssertPass(Run("link", "--kind", "website", "--url", "https://hol.example"));
        AssertPass(Run("background", "--image", dir.Write("art/key.jpg", Jpeg)));
        var (code, output) = Run("list");
        Assert.Equal(0, code);
        Assert.Contains("HOLPublish : announcement a 2026-11-24T18:00:00Z \"Titre a\" [fr/en/ja] https://example.org", output);
        Assert.Contains("HOLPublish : link website https://hol.example", output);
        Assert.Contains("HOLPublish : background ", output);
        Assert.EndsWith("HOLPublish : PASS 1 announcement(s), 1 link(s), a background", output);
    }

    [Fact]
    public void ProgramRoutesTheHomeCommand()
    {
        var output = new StringWriter();
        Assert.Equal(0, Program.Run(["home", "list", "--versions", Versions], output));
        Assert.Equal("HOLPublish : PASS 0 announcement(s), 0 link(s), no background", output.ToString().Trim());
    }
}
