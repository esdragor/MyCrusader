namespace HOLUpdate.Tests;

/** Launcher 5: the home page's storage format shared by the manager (reads) and the host's tool (writes). */
public sealed class HomeFilesTests : IDisposable
{
    private readonly TempDir dir = new();

    public void Dispose() => dir.Dispose();

    [Theory]
    [InlineData("en", "English")]
    [InlineData("ja", "French")]
    [InlineData(null, "French")]
    [InlineData("de", "French")]
    public void PickFallsBackToFrench(string? language, string expected)
    {
        var texts = new Dictionary<string, string> { ["fr"] = "French", ["en"] = " English ", ["ja"] = "  " };
        Assert.Equal(expected, HomeFiles.Pick(texts, language));
    }

    [Fact]
    public void PickWithoutFrenchTakesAnyLanguageThenNothing()
    {
        Assert.Equal("English", HomeFiles.Pick(new Dictionary<string, string> { ["en"] = "English" }, "ja"));
        Assert.Equal("", HomeFiles.Pick(new Dictionary<string, string>(), "fr"));
    }

    [Fact]
    public void ImageTypeReadsTheFirstBytes()
    {
        Assert.Equal(HomeFiles.JpegType, HomeFiles.ImageType([0xFF, 0xD8, 0xFF, 0xDB]));
        Assert.Equal(HomeFiles.PngType, HomeFiles.ImageType([0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A]));
        Assert.Null(HomeFiles.ImageType("GIF89a"u8));
        Assert.Null(HomeFiles.ImageType([0x89, 0x50]));
        Assert.Null(HomeFiles.ImageType([]));
    }

    [Theory]
    [InlineData("2026-11-24T18:00:00Z", "2026-11-24T18:00:00Z")]
    [InlineData("2026-11-24T19:00:00+01:00", "2026-11-24T18:00:00Z")]
    [InlineData("2026-11-24 18:00", "2026-11-24T18:00:00Z")]
    [InlineData("2026-11-24", "2026-11-24T00:00:00Z")]
    public void DatesAreUtcAndADateWithoutOffsetIsUtc(string text, string expected)
    {
        Assert.True(HomeFiles.TryParseDate(text, out var utc));
        Assert.Equal(DateTimeKind.Utc, utc.Kind);
        Assert.Equal(expected, HomeFiles.FormatDate(utc));
    }

    [Theory]
    [InlineData("discord", "discord")]
    [InlineData(" YouTube ", "youtube")]
    [InlineData("twitch", "other")]
    [InlineData(null, "other")]
    public void UnknownKindsAreOther(string? kind, string expected) => Assert.Equal(expected, HomeFiles.NormalizeKind(kind));

    [Fact]
    public void WrittenAnnouncementsReadBackNewestFirst()
    {
        var file = Path.Combine(dir.Root, "home", HomeFiles.AnnouncementsFile);
        var texts = new Dictionary<string, string> { ["fr"] = "Bonjour", ["en"] = "Hello", ["ja"] = "こんにちは" };
        HomeFiles.WriteAnnouncements(file,
        [
            new StoredAnnouncement("a", new DateTime(2026, 11, 1, 0, 0, 0, DateTimeKind.Utc), texts, texts),
            new StoredAnnouncement("b", new DateTime(2026, 11, 2, 0, 0, 0, DateTimeKind.Utc), texts, texts, "https://x.org"),
        ]);
        var read = HomeFiles.ReadAnnouncements(file);
        Assert.Equal(["b", "a"], read.Select(a => a.Id));
        Assert.Equal("こんにちは", read[0].Title["ja"]);
        Assert.Equal("https://x.org", read[0].Link);
        Assert.False(File.Exists(file + ".tmp"));
        Assert.Empty(HomeFiles.ReadAnnouncements(Path.Combine(dir.Root, "none.json")));
    }

    [Fact]
    public void AHandWrittenFileMayHaveCommentsTrailingCommasAndPlainTitles()
    {
        var file = dir.Write("a.json", """
            // the host's notes
            [ { "id": "a", "date": "2026-11-24", "title": "Titre seul", "body": "Corps", }, ]
            """);
        var announcement = Assert.Single(HomeFiles.ReadAnnouncements(file));
        Assert.Equal("Titre seul", HomeFiles.Pick(announcement.Title, "en"));
        Assert.Equal("Corps", HomeFiles.Pick(announcement.Body, "ja"));
    }

    [Fact]
    public void FindBackgroundTakesTheNewest()
    {
        var home = dir.Root;
        Assert.Null(HomeFiles.FindBackground(home));
        var jpg = dir.Write("background.jpg", [1]);
        var png = dir.Write("background.png", [2]);
        File.SetLastWriteTimeUtc(jpg, DateTime.UtcNow.AddMinutes(-5));
        Assert.Equal(png, HomeFiles.FindBackground(home));
    }
}
