using System.Net;
using System.Net.Http.Headers;
using System.Text.Json.Nodes;
using HOLUpdate;
using Microsoft.AspNetCore.Mvc.Testing;

namespace HOLManager.Tests;

/**
 * Launcher 5: GET /launcher/home and /launcher/home/background on a real versions folder, <versions>/home/ as the host's
 * tool writes it (announcements.json, links.json, background.jpg / .png). Each part stands alone: a bad file empties its
 * part, never the page.
 */
public sealed class HomeRoutesTests : IClassFixture<WebApplicationFactory<Program>>, IDisposable
{
    internal static readonly byte[] Jpeg = [0xFF, 0xD8, 0xFF, 0xE0, 1, 2, 3, 4, 5, 6, 7, 8];
    internal static readonly byte[] Png = [0x89, 0x50, 0x4E, 0x47, 0x0D, 0x0A, 0x1A, 0x0A, 9, 9, 9];

    private readonly string root = Path.Combine(Path.GetTempPath(), $"HOL home {Guid.NewGuid():N}");
    private readonly HttpClient client;

    public HomeRoutesTests(WebApplicationFactory<Program> factory)
    {
        Directory.CreateDirectory(Home);
        client = factory.WithWebHostBuilder(b => b.UseSetting("VersionsDir", Path.Combine(root, "Versions"))).CreateClient();
    }

    public void Dispose() => ProcessServerLauncherTests.DeleteDir(root);

    private string Home => Path.Combine(root, "Versions", HomeFiles.DirName);

    private void Write(string name, string text) => File.WriteAllText(Path.Combine(Home, name), text);

    private static string Texts(string fr, string? en = null, string? ja = null) =>
        new JsonObject { ["fr"] = fr, ["en"] = en, ["ja"] = ja }.ToJsonString();

    private static string Row(string id, string date, string title, string body = "\"\"", string? link = null) =>
        $$"""{ "id": "{{id}}", "date": "{{date}}", "title": {{title}}, "body": {{body}}{{(link == null ? "" : $", \"link\": \"{link}\"")}} }""";

    private async Task<JsonNode> GetHome(string query = "")
    {
        var response = await client.GetAsync("/launcher/home" + query);
        Assert.Equal(HttpStatusCode.OK, response.StatusCode);
        return JsonNode.Parse(await response.Content.ReadAsStringAsync())!;
    }

    private static string?[] Field(JsonNode body, string array, string name) =>
        body[array]!.AsArray().Select(n => (string?)n![name]).ToArray();

    [Fact]
    public async Task NothingPublishedGivesAnEmptyPage()
    {
        Directory.Delete(Home);
        var body = await GetHome();
        Assert.Equal(1, (int?)body["format"]);
        Assert.Empty(body["announcements"]!.AsArray());
        Assert.Empty(body["links"]!.AsArray());
        Assert.Null(body["background"]);
        Assert.True(body.AsObject().ContainsKey("background"));
    }

    [Theory]
    [InlineData("", "Nouvelle carte")]
    [InlineData("?lang=fr", "Nouvelle carte")]
    [InlineData("?lang=en", "New map")]
    [InlineData("?lang=ja", "新しいマップ")]
    // Not a language of the files, or not two lowercase letters: French.
    [InlineData("?lang=de", "Nouvelle carte")]
    [InlineData("?lang=EN", "Nouvelle carte")]
    [InlineData("?lang=..%2Fx", "Nouvelle carte")]
    public async Task AnnouncementsComeInTheAskedLanguage(string query, string title)
    {
        Write(HomeFiles.AnnouncementsFile, $"[ {Row("map", "2026-11-24T18:00:00Z", Texts("Nouvelle carte", "New map", "新しいマップ"),
            Texts("Corps", "Body", "本文"), "https://example.org/map")} ]");
        var body = await GetHome(query);
        var announcement = Assert.Single(body["announcements"]!.AsArray())!;
        Assert.Equal("map", (string?)announcement["id"]);
        Assert.Equal("2026-11-24T18:00:00Z", (string?)announcement["date"]);
        Assert.Equal(title, (string?)announcement["title"]);
        Assert.Equal("https://example.org/map", (string?)announcement["link"]);
    }

    [Fact]
    public async Task AMissingTranslationFallsBackToFrenchFieldByField()
    {
        Write(HomeFiles.AnnouncementsFile, $"[ {Row("a", "2026-11-24T18:00:00Z", Texts("Titre", "Title"), Texts("Texte", null, "  "))} ]");
        var announcement = (await GetHome("?lang=en"))["announcements"]![0]!;
        Assert.Equal("Title", (string?)announcement["title"]);
        Assert.Equal("Texte", (string?)announcement["body"]);
        Assert.Null(announcement["link"]);
        var japanese = (await GetHome("?lang=ja"))["announcements"]![0]!;
        Assert.Equal("Titre", (string?)japanese["title"]);
        Assert.Equal("Texte", (string?)japanese["body"]);
    }

    [Fact]
    public async Task NewestFirstAndAtMostTwenty()
    {
        var rows = Enumerable.Range(0, 25)
            .Select(i => Row($"a{i:00}", new DateTime(2026, 11, 1, 12, 0, 0, DateTimeKind.Utc).AddDays(i).ToString("O"), Texts($"T{i}")));
        Write(HomeFiles.AnnouncementsFile, "[" + string.Join(",", rows.Reverse().Skip(3).Concat(rows.Reverse().Take(3)).Reverse()) + "]");
        var ids = Field(await GetHome(), "announcements", "id");
        Assert.Equal(20, ids.Length);
        Assert.Equal(Enumerable.Range(5, 20).Reverse().Select(i => $"a{i:00}"), ids);
    }

    [Fact]
    public async Task ADateWithAnOffsetIsGivenInUtc()
    {
        Write(HomeFiles.AnnouncementsFile, $"[ {Row("a", "2026-11-24T19:30:00+01:00", Texts("T"))} ]");
        Assert.Equal("2026-11-24T18:30:00Z", (string?)(await GetHome())["announcements"]![0]!["date"]);
    }

    [Fact]
    public async Task BadEntriesAreLeftOutTheOthersServed()
    {
        Write(HomeFiles.AnnouncementsFile, "[ "
            + string.Join(", ",
                Row("ok", "2026-11-24T18:00:00Z", Texts("Bon"), link: "javascript:alert(1)"),
                Row("bad id!", "2026-11-24T18:00:00Z", Texts("Id")),
                Row("nodate", "hier", Texts("Date")),
                Row("notitle", "2026-11-24T18:00:00Z", Texts(" ")),
                "42", "null")
            + " ]");
        var announcement = Assert.Single((await GetHome())["announcements"]!.AsArray())!;
        Assert.Equal("ok", (string?)announcement["id"]);
        // A link that is not http(s) is dropped, the announcement kept.
        Assert.Null(announcement["link"]);
    }

    [Theory]
    [InlineData("not json {")]
    [InlineData("{ \"id\": \"a\" }")]
    [InlineData("")]
    public async Task AnUnreadableAnnouncementsFileEmptiesOnlyThatPart(string text)
    {
        Write(HomeFiles.AnnouncementsFile, text);
        Write(HomeFiles.LinksFile, """[ { "kind": "discord", "url": "https://discord.gg/x" } ]""");
        var body = await GetHome();
        Assert.Empty(body["announcements"]!.AsArray());
        Assert.Equal(["discord"], Field(body, "links", "kind"));
    }

    [Fact]
    public async Task AnUnreadableLinksFileEmptiesOnlyThatPart()
    {
        Write(HomeFiles.AnnouncementsFile, $"[ {Row("a", "2026-11-24T18:00:00Z", Texts("T"))} ]");
        Write(HomeFiles.LinksFile, "[ broken");
        var body = await GetHome();
        Assert.Empty(body["links"]!.AsArray());
        Assert.Single(body["announcements"]!.AsArray());
    }

    [Fact]
    public async Task LinksKeepTheirOrderOnlyHttpAndUnknownKindsAreOther()
    {
        Write(HomeFiles.LinksFile, """
            [
              { "kind": "YouTube", "url": "https://youtube.com/@hol" },
              { "kind": "discord", "url": "file:///C:/Windows/notepad.exe" },
              { "kind": "twitch", "url": "http://twitch.tv/hol" },
              { "kind": "website" },
              { "kind": "website", "url": "https://hol.example" },
            ]
            """);
        var body = await GetHome();
        Assert.Equal(["youtube", "other", "website"], Field(body, "links", "kind"));
        Assert.Equal(["https://youtube.com/@hol", "http://twitch.tv/hol", "https://hol.example"], Field(body, "links", "url"));
    }

    [Fact]
    public async Task BackgroundIsServedWithItsTypeAndCacheHeaders()
    {
        File.WriteAllBytes(Path.Combine(Home, "background.jpg"), Jpeg);
        Assert.Equal("/launcher/home/background", (string?)(await GetHome())["background"]);

        var response = await client.GetAsync("/launcher/home/background");
        Assert.Equal(HttpStatusCode.OK, response.StatusCode);
        Assert.Equal("image/jpeg", response.Content.Headers.ContentType?.MediaType);
        Assert.Equal(Jpeg, await response.Content.ReadAsByteArrayAsync());
        var tag = response.Headers.ETag;
        Assert.NotNull(tag);
        Assert.NotNull(response.Content.Headers.LastModified);

        var again = new HttpRequestMessage(HttpMethod.Get, "/launcher/home/background");
        again.Headers.IfNoneMatch.Add(tag!);
        var notModified = await client.SendAsync(again);
        Assert.Equal(HttpStatusCode.NotModified, notModified.StatusCode);
        Assert.Empty(await notModified.Content.ReadAsByteArrayAsync());

        var stale = new HttpRequestMessage(HttpMethod.Get, "/launcher/home/background");
        stale.Headers.IfNoneMatch.Add(new EntityTagHeaderValue("\"old\""));
        Assert.Equal(HttpStatusCode.OK, (await client.SendAsync(stale)).StatusCode);
    }

    [Fact]
    public async Task ANewImageGetsANewTag()
    {
        var file = Path.Combine(Home, "background.jpg");
        File.WriteAllBytes(file, Jpeg);
        var first = (await client.GetAsync("/launcher/home/background")).Headers.ETag;
        File.Delete(file);
        File.WriteAllBytes(Path.Combine(Home, "background.png"), Png);
        var response = await client.GetAsync("/launcher/home/background");
        Assert.Equal("image/png", response.Content.Headers.ContentType?.MediaType);
        Assert.NotEqual(first, response.Headers.ETag);
    }

    [Fact]
    public async Task NoBackgroundIs404()
    {
        var response = await client.GetAsync("/launcher/home/background");
        Assert.Equal(HttpStatusCode.NotFound, response.StatusCode);
        var body = JsonNode.Parse(await response.Content.ReadAsStringAsync())!;
        Assert.Equal("no_background", (string?)body["error"]);
        Assert.False(string.IsNullOrEmpty((string?)body["message"]));
    }

    [Fact]
    public async Task AFileThatIsNotAnImageIsNotServed()
    {
        File.WriteAllText(Path.Combine(Home, "background.jpg"), "<html>not an image</html>");
        Assert.Null((await GetHome())["background"]);
        Assert.Equal(HttpStatusCode.NotFound, (await client.GetAsync("/launcher/home/background")).StatusCode);
    }
}
