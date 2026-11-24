using System.Net;
using System.Net.Http.Headers;
using System.Text;

namespace HOLLauncher.Tests;

/**
 * Launcher 5: HomeClient reads GET /launcher/home into the window's HomeContent and keeps the key art in
 * <launcher>/HOLLauncher.home/ (ETag, 304, offline). Never throws: every failure is HomeContent.Empty.
 */
public sealed class HomeClientTests : IDisposable
{
    private const string Manager = "http://host:7700";
    private static readonly byte[] Jpeg = [0xFF, 0xD8, 0xFF, 0xE0, 1, 2, 3];
    private static readonly byte[] Png = [0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A, 4, 5];

    private static readonly string Page = """
        {
          "format": 1,
          "announcements": [
            { "id": "old", "date": "2026-11-01T10:00:00Z", "title": "Old", "body": "Before", "link": null },
            { "id": "map", "date": "2026-11-24T18:00:00Z", "title": "New map", "body": "Line 1\nLine 2", "link": "https://example.org/map" }
          ],
          "links": [ { "kind": "discord", "url": "https://discord.gg/x" }, { "kind": "youtube", "url": "https://youtube.com/@hol" } ],
          "background": null
        }
        """;

    private static readonly string PageWithBackground = Page.Replace("\"background\": null", "\"background\": \"/launcher/home/background\"");

    private readonly TempDir launcher = new();

    public void Dispose() => launcher.Dispose();

    private string CacheDir => Path.Combine(launcher.Root, HomeClient.CacheDirName);

    private HomeClient Client(FakeHandler handler) => new(new HttpClient(handler), CacheDir);

    private static HttpResponseMessage Json(string body, HttpStatusCode code = HttpStatusCode.OK) =>
        new(code) { Content = new StringContent(body, Encoding.UTF8, "application/json") };

    private static HttpResponseMessage Image(byte[] bytes, string type, string? tag)
    {
        var response = new HttpResponseMessage(HttpStatusCode.OK) { Content = new ByteArrayContent(bytes) };
        response.Content.Headers.ContentType = new MediaTypeHeaderValue(type);
        if (tag != null)
        {
            response.Headers.ETag = new EntityTagHeaderValue(tag);
        }
        return response;
    }

    /** The home page, and the background answered by the given function (which sees the request's If-None-Match). */
    private static FakeHandler Manager_(string page, Func<HttpRequestMessage, HttpResponseMessage> background) =>
        new(request => request.RequestUri!.AbsolutePath == "/launcher/home/background" ? background(request) : Json(page));

    private static void AssertEmpty(HomeContent content, string? background = null)
    {
        Assert.Empty(content.Announcements);
        Assert.Empty(content.Links);
        Assert.Equal(background, content.BackgroundFile);
    }

    [Fact]
    public async Task ReadsTheAnnouncementsNewestFirstAndTheLinks()
    {
        var handler = new FakeHandler(_ => Json(Page));
        var content = await Client(handler).GetAsync(Manager + "/", "en");

        Assert.Equal(new Uri("http://host:7700/launcher/home?lang=en"), Assert.Single(handler.Requests));
        Assert.Equal(
            [
                new Announcement("map", new DateTime(2026, 11, 24, 18, 0, 0, DateTimeKind.Utc), "New map", "Line 1\nLine 2", "https://example.org/map"),
                new Announcement("old", new DateTime(2026, 11, 1, 10, 0, 0, DateTimeKind.Utc), "Old", "Before"),
            ],
            content.Announcements);
        Assert.Equal(DateTimeKind.Utc, content.Announcements[0].DateUtc.Kind);
        Assert.Equal([new CommunityLink("discord", "https://discord.gg/x"), new CommunityLink("youtube", "https://youtube.com/@hol")], content.Links);
        Assert.Null(content.BackgroundFile);
        Assert.False(Directory.Exists(CacheDir));
    }

    [Fact]
    public async Task WithoutLanguageNoQueryAndABasePathIsKept()
    {
        var handler = new FakeHandler(_ => Json(Page));
        await Client(handler).GetAsync("http://host:7700/hol", null);
        Assert.Equal(new Uri("http://host:7700/hol/launcher/home"), Assert.Single(handler.Requests));
    }

    [Fact]
    public async Task BadRowsAreLeftOutOneByOne()
    {
        var page = """
            {
              "format": 1,
              "announcements": [
                { "id": "ok", "date": "2026-11-24T18:00:00Z", "title": "Ok", "link": "javascript:alert(1)" },
                { "date": "2026-11-24T18:00:00Z", "title": "No id" },
                { "id": "nodate", "title": "No date" },
                { "id": "notitle", "date": "2026-11-24T18:00:00Z", "title": " " },
                12, "text"
              ],
              "links": [
                { "kind": "twitch", "url": "https://twitch.tv/hol" },
                { "kind": "discord", "url": "file:///C:/Windows/notepad.exe" },
                { "kind": "website" },
                { "url": "http://hol.example" }
              ],
              "later": { "field": "ignored" }
            }
            """;
        var content = await Client(new FakeHandler(_ => Json(page))).GetAsync(Manager, "fr");
        var announcement = Assert.Single(content.Announcements);
        Assert.Equal(new Announcement("ok", new DateTime(2026, 11, 24, 18, 0, 0, DateTimeKind.Utc), "Ok", ""), announcement);
        Assert.Equal([new CommunityLink("other", "https://twitch.tv/hol"), new CommunityLink("other", "http://hol.example")], content.Links);
    }

    [Theory]
    // A manager older than launcher 5: no route.
    [InlineData(HttpStatusCode.NotFound, "")]
    [InlineData(HttpStatusCode.Unauthorized, "{\"error\":\"bad_key\",\"message\":\"Missing or wrong X-HOL-Key\"}")]
    [InlineData(HttpStatusCode.InternalServerError, "oops")]
    [InlineData(HttpStatusCode.OK, "not json")]
    [InlineData(HttpStatusCode.OK, "[]")]
    [InlineData(HttpStatusCode.OK, "{ \"announcements\": [] }")]
    [InlineData(HttpStatusCode.OK, "{ \"format\": 2, \"announcements\": [] }")]
    [InlineData(HttpStatusCode.OK, "{ \"format\": \"1\" }")]
    public async Task AnythingElseIsEmpty(HttpStatusCode code, string body)
    {
        AssertEmpty(await Client(new FakeHandler(_ => Json(body, code))).GetAsync(Manager, "fr"));
    }

    [Fact]
    public async Task ABadAddressOrNoAnswerIsEmptyNeverAThrow()
    {
        var handler = new FakeHandler(_ => Json(Page));
        AssertEmpty(await Client(handler).GetAsync("not an address", "fr"));
        AssertEmpty(await Client(handler).GetAsync("ftp://host", "fr"));
        Assert.Empty(handler.Requests);
        AssertEmpty(await Client(FakeHandler.Throwing(new HttpRequestException("refused"))).GetAsync(Manager, "fr"));
        AssertEmpty(await Client(FakeHandler.Throwing(new TaskCanceledException("timeout"))).GetAsync(Manager, "fr"));
    }

    [Fact]
    public async Task TheBackgroundIsCachedThenAskedWithItsTag()
    {
        var asked = new List<string?>();
        var handler = Manager_(PageWithBackground, request =>
        {
            var tag = request.Headers.IfNoneMatch.SingleOrDefault()?.ToString();
            asked.Add(tag);
            return tag == "\"v1\"" ? new HttpResponseMessage(HttpStatusCode.NotModified) : Image(Jpeg, "image/jpeg", "\"v1\"");
        });
        var client = Client(handler);

        var first = await client.GetAsync(Manager, "fr");
        var file = Path.Combine(CacheDir, "background.jpg");
        Assert.Equal(file, first.BackgroundFile);
        Assert.Equal(Jpeg, File.ReadAllBytes(file));
        Assert.Equal("\"v1\"", File.ReadAllText(Path.Combine(CacheDir, HomeClient.ETagFile)));
        Assert.Equal(2, first.Announcements.Count);

        var written = File.GetLastWriteTimeUtc(file);
        var second = await client.GetAsync(Manager, "fr");
        Assert.Equal(file, second.BackgroundFile);
        Assert.Equal([null, "\"v1\""], asked);
        Assert.Equal(written, File.GetLastWriteTimeUtc(file));
        Assert.Contains(new Uri("http://host:7700/launcher/home/background"), handler.Requests);
    }

    [Fact]
    public async Task ANewPictureReplacesTheOldOneEvenOfAnotherType()
    {
        await Client(Manager_(PageWithBackground, _ => Image(Jpeg, "image/jpeg", "\"v1\""))).GetAsync(Manager, "fr");
        var content = await Client(Manager_(PageWithBackground, _ => Image(Png, "image/png", "\"v2\""))).GetAsync(Manager, "fr");

        Assert.Equal(Path.Combine(CacheDir, "background.png"), content.BackgroundFile);
        Assert.Equal(["background.etag", "background.png"], Directory.GetFiles(CacheDir).Select(Path.GetFileName).Order());
        Assert.Equal("\"v2\"", File.ReadAllText(Path.Combine(CacheDir, HomeClient.ETagFile)));
    }

    [Fact]
    public async Task OfflineTheLastPictureStays()
    {
        await Client(Manager_(PageWithBackground, _ => Image(Jpeg, "image/jpeg", "\"v1\""))).GetAsync(Manager, "fr");
        var offline = await Client(FakeHandler.Throwing(new HttpRequestException("no route to host"))).GetAsync(Manager, "fr");
        AssertEmpty(offline, Path.Combine(CacheDir, "background.jpg"));
        Assert.Equal(Jpeg, File.ReadAllBytes(offline.BackgroundFile!));
    }

    [Theory]
    [InlineData("error")]
    [InlineData("throw")]
    [InlineData("not an image")]
    [InlineData("too big")]
    public async Task AFailedPictureDownloadKeepsTheLastOne(string failure)
    {
        await Client(Manager_(PageWithBackground, _ => Image(Jpeg, "image/jpeg", "\"v1\""))).GetAsync(Manager, "fr");
        var content = await Client(Manager_(PageWithBackground, _ => failure switch
        {
            "error" => Json("{\"error\":\"no_background\",\"message\":\"x\"}", HttpStatusCode.NotFound),
            "throw" => throw new HttpRequestException("cut"),
            "not an image" => Image(Encoding.UTF8.GetBytes("<html>"), "image/jpeg", "\"v2\""),
            _ => Image([.. Png, .. new byte[HOLUpdate.HomeFiles.MaxBackgroundBytes]], "image/png", "\"v2\""),
        })).GetAsync(Manager, "fr");

        Assert.Equal(Path.Combine(CacheDir, "background.jpg"), content.BackgroundFile);
        Assert.Equal(2, content.Announcements.Count);
        Assert.Equal(Jpeg, File.ReadAllBytes(content.BackgroundFile!));
        Assert.Equal("\"v1\"", File.ReadAllText(Path.Combine(CacheDir, HomeClient.ETagFile)));
        Assert.False(File.Exists(Path.Combine(CacheDir, "background.png")));
    }

    [Fact]
    public async Task AFirstFailedPictureGivesNone()
    {
        var content = await Client(Manager_(PageWithBackground, _ => throw new HttpRequestException("cut"))).GetAsync(Manager, "fr");
        Assert.Null(content.BackgroundFile);
        Assert.Equal(2, content.Announcements.Count);
    }

    [Fact]
    public async Task TheManagerWithoutPictureRemovesTheCachedOne()
    {
        await Client(Manager_(PageWithBackground, _ => Image(Jpeg, "image/jpeg", "\"v1\""))).GetAsync(Manager, "fr");
        var content = await Client(Manager_(Page, _ => throw new InvalidOperationException("not asked"))).GetAsync(Manager, "fr");
        Assert.Null(content.BackgroundFile);
        Assert.Empty(Directory.GetFiles(CacheDir));
    }

    [Fact]
    public async Task ThePictureWithoutTagIsCachedAndAskedPlainNextTime()
    {
        var asked = new List<int>();
        var handler = Manager_(PageWithBackground, request =>
        {
            asked.Add(request.Headers.IfNoneMatch.Count);
            return Image(Jpeg, "image/jpeg", tag: null);
        });
        var client = Client(handler);
        Assert.NotNull((await client.GetAsync(Manager, "fr")).BackgroundFile);
        Assert.NotNull((await client.GetAsync(Manager, "fr")).BackgroundFile);
        Assert.Equal([0, 0], asked);
        Assert.False(File.Exists(Path.Combine(CacheDir, HomeClient.ETagFile)));
    }

    [Fact]
    public async Task TheKitsKeyIsSentWithBothRequests()
    {
        var keys = new List<string?>();
        var spy = new FakeHandler(request =>
        {
            keys.Add(request.Headers.TryGetValues(AccessKeyHandler.Header, out var v) ? v.Single() : null);
            return request.RequestUri!.AbsolutePath.EndsWith("/background") ? Image(Jpeg, "image/jpeg", "\"v1\"") : Json(PageWithBackground);
        });
        var http = new HttpClient(new AccessKeyHandler(() => new ManagerServer("Bob", Manager, "k3y")) { InnerHandler = spy });
        await new HomeClient(http, CacheDir).GetAsync(Manager, "fr");
        Assert.Equal(["k3y", "k3y"], keys);
    }

    [Fact]
    public void TheLaunchersCacheIsBesideItsExe()
    {
        using var game = new TempDir();
        Assert.Equal(Path.Combine(launcher.Root, "HOLLauncher.home"), HomeClient.ForLauncher(game.Root, launcher.Root).CacheDir);
    }
}
