using System.Net;

namespace HOLLauncher.Tests;

/** L11: the key of servers.json, sent in X-HOL-Key to its manager only; a 401 bad_key is said in French. */
public sealed class AccessKeyTests : IDisposable
{
    private const string Key = "k3y-Of_The-Kit";
    private const string BadKeyBody = "{\"error\":\"bad_key\",\"message\":\"Missing or wrong X-HOL-Key\"}";

    private readonly TempDir game = new();

    public void Dispose() => game.Dispose();

    private void WriteServers(string json) => File.WriteAllText(Path.Combine(game.Root, GameServers.FileName), json);

    [Fact]
    public void ServersJsonKeyIsRead()
    {
        WriteServers($"{{\"servers\":[{{\"name\":\"Bob\",\"url\":\"http://90.55.168.58:7700\",\"key\":\"{Key}\"}}]}}");
        Assert.Equal(new ManagerServer("Bob", "http://90.55.168.58:7700", Key), GameServers.ReadFirst(game.Root));
    }

    [Theory]
    [InlineData("{\"servers\":[{\"name\":\"Bob\",\"url\":\"http://h:7700\"}]}")]
    [InlineData("{\"servers\":[{\"name\":\"Bob\",\"url\":\"http://h:7700\",\"key\":\"\"}]}")]
    [InlineData("{\"servers\":[{\"name\":\"Bob\",\"url\":\"http://h:7700\",\"key\":12}]}")]
    public void ServersJsonWithoutKeyGivesNone(string json)
    {
        WriteServers(json);
        Assert.Null(GameServers.ReadFirst(game.Root).Key);
    }

    [Fact]
    public void NoServersJsonGivesNoKey() => Assert.Null(GameServers.ReadFirst(game.Root).Key);

    private (HttpClient Client, List<string?> Keys) Client(Func<ManagerServer?> server)
    {
        var keys = new List<string?>();
        var fake = new FakeHandler(request =>
        {
            keys.Add(request.Headers.TryGetValues(AccessKeyHandler.Header, out var v) ? v.Single() : null);
            return new HttpResponseMessage(HttpStatusCode.OK) { Content = new StringContent("{}") };
        });
        return (new HttpClient(new AccessKeyHandler(server) { InnerHandler = fake }), keys);
    }

    [Fact]
    public async Task HandlerSendsTheKeyToItsManagerOnly()
    {
        var (client, keys) = Client(() => new ManagerServer("Bob", "http://90.55.168.58:7700/", Key));
        await client.GetAsync("http://90.55.168.58:7700/versions/latest");
        await client.GetAsync("http://90.55.168.58:7700/versions/v2/files/a.gz");
        await client.GetAsync("http://elsewhere:7700/versions/latest");
        await client.GetAsync("http://90.55.168.58:7701/versions/latest");
        Assert.Equal([Key, Key, null, null], keys);
    }

    [Fact]
    public async Task HandlerSendsNothingWithoutKeyOrServer()
    {
        var (client, keys) = Client(() => new ManagerServer("Bob", "http://h:7700", null));
        await client.GetAsync("http://h:7700/versions/latest");
        var (none, noKeys) = Client(() => null);
        await none.GetAsync("http://h:7700/versions/latest");
        Assert.Equal([null], keys);
        Assert.Equal([null], noKeys);
    }

    [Fact]
    public async Task HandlerReadsTheServersJsonOfTheGame()
    {
        WriteServers($"{{\"servers\":[{{\"url\":\"http://h:7700\",\"key\":\"{Key}\"}}]}}");
        var (client, keys) = Client(AccessKeyHandler.FromGame(game.Root));
        await client.GetAsync("http://h:7700/status");
        WriteServers("{ broken");
        await client.GetAsync("http://h:7700/status");
        Assert.Equal([Key, null], keys);
    }

    [Fact]
    public void LauncherClientsSendTheKey()
    {
        WriteServers($"{{\"servers\":[{{\"url\":\"http://h:7700\",\"key\":\"{Key}\"}}]}}");
        using var versions = VersionsClient.CreateHttpClient(game.Root);
        using var downloads = FileDownloader.CreateHttpClient(game.Root);
        Assert.Equal(VersionsClient.DefaultTimeout, versions.Timeout);
        Assert.Equal(Timeout.InfiniteTimeSpan, downloads.Timeout);
    }

    [Fact]
    public async Task BadKeyIsSaidInTheStatus()
    {
        var client = new VersionsClient(new HttpClient(FakeHandler.Json(HttpStatusCode.Unauthorized, BadKeyBody)));
        var result = await client.GetLatestAsync("http://h:7700");
        Assert.Equal(LatestOutcome.Refused, result.Outcome);
        Assert.Equal(AccessKeyHandler.BadKeyMessage, result.Message);
        var view = StatusView.From(LauncherStatus.Compute("v1", result));
        Assert.StartsWith("Clé d'accès refusée : le kit n'est pas le bon. Redemandez-le.", view.StateText);
        Assert.DoesNotContain("injoignable", view.StateText);
    }

    [Fact]
    public async Task BadKeyIsSaidByTheDownloads()
    {
        var downloader = new FileDownloader(new HttpClient(FakeHandler.Json(HttpStatusCode.Unauthorized, BadKeyBody)),
            "http://h:7700", game.Root);
        var e = await Assert.ThrowsAsync<UpdateDownloadException>(
            () => downloader.FetchRouteAsync("launcher/latest", Path.Combine(game.Root, "l.part")));
        Assert.Equal(AccessKeyHandler.BadKeyMessage, e.Message);
    }
}
