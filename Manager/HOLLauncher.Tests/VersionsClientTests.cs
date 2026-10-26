using System.Net;

namespace HOLLauncher.Tests;

public class VersionsClientTests
{
    private const string Hash = "3f5a000000000000000000000000000000000000000000000000000000000000";

    private static readonly string Latest = $$"""
        {
          "format": 1,
          "version": "2026-10-25-b",
          "manifest": { "format": 1, "version": "2026-10-25-b",
                        "files": [ { "path": "HeroesOfLiberty.exe", "size": 348127232, "sha256": "{{Hash}}" } ] },
          "notes": "Archers plus rapides.",
          "patchesFrom": [ "2026-10-24", "2026-10-25-a" ],
          "launcherVersion": "2"
        }
        """;

    private static Task<LatestResult> Get(FakeHandler handler, string url = "http://host:7700")
        => new VersionsClient(new HttpClient(handler)).GetLatestAsync(url);

    [Fact]
    public async Task ReadsLatestVersion()
    {
        var handler = FakeHandler.Json(HttpStatusCode.OK, Latest);
        var result = await Get(handler);

        Assert.Equal(LatestOutcome.Ok, result.Outcome);
        var latest = result.Latest!;
        Assert.Equal("2026-10-25-b", latest.Version);
        Assert.Equal("Archers plus rapides.", latest.Notes);
        Assert.Equal(["2026-10-24", "2026-10-25-a"], latest.PatchesFrom);
        Assert.Equal("2", latest.LauncherVersion);
        var file = Assert.Single(latest.Manifest.Files);
        Assert.Equal(new HOLUpdate.FileEntry("HeroesOfLiberty.exe", 348127232, Hash), file);
        Assert.Equal(new Uri("http://host:7700/versions/latest"), Assert.Single(handler.Requests));
    }

    [Fact]
    public async Task TrailingSlashOfManagerUrlIsIgnored()
    {
        var handler = FakeHandler.Json(HttpStatusCode.OK, Latest);
        await Get(handler, "http://host:7700/");
        Assert.Equal(new Uri("http://host:7700/versions/latest"), Assert.Single(handler.Requests));
    }

    [Fact]
    public async Task MissingOptionalFieldsAreEmpty()
    {
        var body = """{ "format": 1, "version": "v1", "manifest": { "format": 1, "version": "v1", "files": [] } }""";
        var result = await Get(FakeHandler.Json(HttpStatusCode.OK, body));
        Assert.Equal(LatestOutcome.Ok, result.Outcome);
        Assert.Equal("", result.Latest!.Notes);
        Assert.Empty(result.Latest.PatchesFrom);
        Assert.Equal("", result.Latest.LauncherVersion);
    }

    [Fact]
    public async Task ManagerErrorIsRefusedWithItsMessage()
    {
        var result = await Get(FakeHandler.Json(HttpStatusCode.NotFound,
            """{ "error": "no_version", "message": "nothing published" }"""));
        Assert.Equal(LatestOutcome.Refused, result.Outcome);
        Assert.Null(result.Latest);
        Assert.Contains("no_version", result.Message);
        Assert.Contains("nothing published", result.Message);
    }

    [Fact]
    public async Task ErrorWithoutJsonBodyIsRefused()
    {
        var result = await Get(FakeHandler.Json(HttpStatusCode.InternalServerError, "<html>oops</html>"));
        Assert.Equal(LatestOutcome.Refused, result.Outcome);
        Assert.Contains("500", result.Message);
    }

    [Theory]
    [InlineData("not json")]
    [InlineData("""{ "format": 2, "version": "v1", "manifest": { "format": 1, "version": "v1", "files": [] } }""")]
    [InlineData("""{ "format": 1, "manifest": { "format": 1, "version": "v1", "files": [] } }""")]
    [InlineData("""{ "format": 1, "version": "v1" }""")]
    [InlineData("""{ "format": 1, "version": "v1", "manifest": { "format": 1, "version": "v1", "files": [ { "path": "../x", "size": 1, "sha256": "00" } ] } }""")]
    public async Task BadBodyIsRefused(string body)
    {
        var result = await Get(FakeHandler.Json(HttpStatusCode.OK, body));
        Assert.Equal(LatestOutcome.Refused, result.Outcome);
        Assert.Null(result.Latest);
        Assert.NotEmpty(result.Message);
    }

    [Fact]
    public async Task NoConnectionIsUnreachable()
    {
        var result = await Get(FakeHandler.Throwing(new HttpRequestException("connection refused")));
        Assert.Equal(LatestOutcome.Unreachable, result.Outcome);
        Assert.Contains("connection refused", result.Message);
    }

    [Fact]
    public async Task CanceledRequestIsUnreachable()
    {
        var result = await Get(FakeHandler.Throwing(new TaskCanceledException("timeout")));
        Assert.Equal(LatestOutcome.Unreachable, result.Outcome);
    }

    [Fact]
    public async Task SlowServerHitsTheClientTimeout()
    {
        var http = new HttpClient(new SlowHandler()) { Timeout = TimeSpan.FromMilliseconds(100) };
        var result = await new VersionsClient(http).GetLatestAsync("http://host:7700");
        Assert.Equal(LatestOutcome.Unreachable, result.Outcome);
    }

    [Fact]
    public async Task BadManagerUrlIsUnreachable()
    {
        var result = await Get(FakeHandler.Json(HttpStatusCode.OK, Latest), "not a url");
        Assert.Equal(LatestOutcome.Unreachable, result.Outcome);
        Assert.Contains("not a url", result.Message);
    }

    [Fact]
    public void LauncherClientHasShortTimeout()
    {
        using var http = VersionsClient.CreateHttpClient();
        Assert.True(http.Timeout <= TimeSpan.FromSeconds(10));
    }

    private sealed class SlowHandler : HttpMessageHandler
    {
        protected override async Task<HttpResponseMessage> SendAsync(HttpRequestMessage request, CancellationToken cancellationToken)
        {
            await Task.Delay(TimeSpan.FromSeconds(30), cancellationToken);
            return new HttpResponseMessage(HttpStatusCode.OK);
        }
    }
}
