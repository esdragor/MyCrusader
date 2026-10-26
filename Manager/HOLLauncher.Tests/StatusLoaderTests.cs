using System.Net;

namespace HOLLauncher.Tests;

public class StatusLoaderTests
{
    private const string Latest = """
        { "format": 1, "version": "v2", "manifest": { "format": 1, "version": "v2", "files": [] },
          "notes": "Nouveautés.", "patchesFrom": [], "launcherVersion": "1" }
        """;

    [Fact]
    public async Task ReadsServersThenLatestThenInstalled()
    {
        using var dir = new TempDir();
        dir.Write("servers.json", """{ "servers": [ { "name": "Home", "url": "http://home:7700" } ] }""");
        dir.Write("HeroesOfLiberty/Version.txt", "v1");
        var handler = FakeHandler.Json(HttpStatusCode.OK, Latest);

        var view = await StatusLoader.LoadAsync(dir.Root, new VersionsClient(new HttpClient(handler)));

        Assert.Equal(new Uri("http://home:7700/versions/latest"), Assert.Single(handler.Requests));
        Assert.Equal("Version installée : v1", view.InstalledText);
        Assert.Equal("Dernière version : v2", view.LatestText);
        Assert.True(view.UpdateGameEnabled);
        Assert.Equal("Nouveautés.", view.Notes);
    }

    [Fact]
    public async Task UnreadableServersJsonBecomesErrorView()
    {
        using var dir = new TempDir();
        dir.Write("servers.json", "{ not json");
        var handler = FakeHandler.Json(HttpStatusCode.OK, Latest);

        var view = await StatusLoader.LoadAsync(dir.Root, new VersionsClient(new HttpClient(handler)));

        Assert.Empty(handler.Requests);
        Assert.StartsWith("Erreur : ", view.StateText);
        Assert.True(view.RefreshEnabled);
    }

    [Fact]
    public async Task NoServerShowsUnreachable()
    {
        using var dir = new TempDir();
        var handler = FakeHandler.Throwing(new HttpRequestException("connection refused"));

        var view = await StatusLoader.LoadAsync(dir.Root, new VersionsClient(new HttpClient(handler)));

        Assert.StartsWith("Serveur injoignable : ", view.StateText);
        Assert.Equal("Version installée : pas installé", view.InstalledText);
    }
}
