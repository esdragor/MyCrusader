using System.Net.Http.Json;
using System.Text.Json.Nodes;
using HOLManager.Lobbies;
using HOLManager.Versions;
using Microsoft.AspNetCore.Mvc.Testing;
using Microsoft.Extensions.DependencyInjection;

namespace HOLManager.Tests;

/**
 * D481: one server per version. A new lobby takes the latest published version (latest.txt, read at its creation)
 * and keeps it; its matches start from Servers\<v>\HeroesOfLibertyServer.exe, else from ServerExe.
 */
public sealed class ServerPerVersionTests : IClassFixture<WebApplicationFactory<Program>>, IDisposable
{
    private const string Start = LobbyEntryTests.Version;
    private readonly WebApplicationFactory<Program> factory;
    private readonly string root = Directory.CreateTempSubdirectory("hol-versions-").FullName;
    private readonly FakeClock clock = new();
    private readonly FakeLauncher launcher = new();

    public ServerPerVersionTests(WebApplicationFactory<Program> factory) => this.factory = factory;

    public void Dispose() => ProcessServerLauncherTests.DeleteDir(root);

    private string VersionsDir => Path.Combine(root, "Versions");

    /** What publish.ps1 leaves: the version's folder, then latest.txt naming it. */
    private void Publish(string version)
    {
        Directory.CreateDirectory(Path.Combine(VersionsDir, version));
        File.WriteAllText(Path.Combine(VersionsDir, "latest.txt"), version + "\r\n");
    }

    private LobbyStore Store()
    {
        var versions = new VersionStore(VersionsDir);
        return new LobbyStore(Start, clock, launcher: launcher, latestVersion: versions.LatestName);
    }

    /** Two players ready in the lobby, the leader's token given back. */
    private string TwoReady(LobbyStore store, string lobbyName, string version)
    {
        var leader = store.Create(lobbyName, "Bob", version);
        var guest = store.Join(leader.Lobby.Id, "Alice", version);
        store.UpdateMe(leader.Lobby.Id, leader.Token, ready: true);
        store.UpdateMe(leader.Lobby.Id, guest.Token, ready: true);
        return leader.Token;
    }

    [Fact]
    public void LobbiesCreatedBeforeAndAfterAPublicationKeepTheirVersion()
    {
        var store = Store();
        Publish("v1");
        var old = store.Create("Old", "Bob", "v1").Lobby;
        Publish("v2");
        var fresh = store.Create("New", "Carl", "v2").Lobby;
        Assert.Equal("v1", old.GameVersion);
        Assert.Equal("v2", fresh.GameVersion);
        Assert.Equal("v2", store.GameVersion);
    }

    [Fact]
    public void AClientIsCheckedAgainstItsLobbysVersion()
    {
        var store = Store();
        Publish("v1");
        var old = store.Create("Old", "Bob", "v1").Lobby;
        Publish("v2");
        var fresh = store.Create("New", "Carl", "v2").Lobby;
        LobbyEntryTests.AssertError(LobbyErrors.VersionMismatch, () => store.Join(old.Id, "Alice", "v2"));
        store.Join(fresh.Id, "Alice", "v2");
        store.Join(old.Id, "Dan", "v1");
        // A new lobby is of the latest version only.
        LobbyEntryTests.AssertError(LobbyErrors.VersionMismatch, () => store.Create("Late", "Eve", "v1"));
    }

    [Fact]
    public void EachMatchStartsFromItsLobbysVersion()
    {
        var store = Store();
        Publish("v1");
        var oldToken = TwoReady(store, "Old", "v1");
        var oldId = store.List()[0].Id;
        Publish("v2");
        var freshToken = TwoReady(store, "New", "v2");
        var freshId = store.List()[1].Id;
        store.Start(freshId, freshToken);
        store.Start(oldId, oldToken);
        Assert.Equal(["v2", "v1"], launcher.Versions);
    }

    [Fact]
    public void WithoutLatestNothingChanges()
    {
        var store = Store();
        var lobby = store.Create("Dev", "Bob", Start).Lobby;
        Assert.Equal(Start, lobby.GameVersion);
        Assert.Equal(Start, store.GameVersion);
        var token = TwoReady(store, "Dev2", Start);
        store.Start(store.List()[1].Id, token);
        Assert.Equal([Start], launcher.Versions);
    }

    [Fact]
    public void TheServerOfTheVersionIsTakenWhenItIsThere()
    {
        var servers = Path.Combine(root, "Servers");
        var exe = Path.Combine(servers, "v2", "HeroesOfLibertyServer.exe");
        Directory.CreateDirectory(Path.GetDirectoryName(exe)!);
        File.WriteAllText(exe, "");
        var fallback = Path.Combine(root, "Server", "HeroesOfLibertyServer.exe");
        var settings = new ManagerSettings { ServersDir = servers, ServerExe = fallback };
        Assert.Equal(exe, settings.ResolveServerExe("v2"));
        // No Servers\v1\: the fallback, as before D481.
        Assert.Equal(fallback, settings.ResolveServerExe("v1"));
        Assert.Equal(fallback, settings.ResolveServerExe(null));
        // A name out of the rules never leaves the servers' folder.
        Assert.Equal(fallback, settings.ResolveServerExe(".."));
    }

    [Fact]
    public async Task TheRoutesFollowTheLatestVersion()
    {
        Publish("v1");
        using var app = factory.WithWebHostBuilder(b =>
        {
            b.UseSetting("GameVersion", Start);
            b.UseSetting("VersionsDir", VersionsDir);
            b.UseSetting("LogDir", "");
        });
        var client = app.CreateClient();
        Assert.Equal("v1", (string?)(await client.GetFromJsonAsync<JsonNode>("/status"))!["gameVersion"]);
        var created = await client.PostAsJsonAsync("/lobbies", new { name = "Old", playerName = "Bob", gameVersion = "v1" });
        created.EnsureSuccessStatusCode();
        Publish("v2");
        Assert.Equal("v2", (string?)(await client.GetFromJsonAsync<JsonNode>("/status"))!["gameVersion"]);
        var list = (await client.GetFromJsonAsync<JsonNode>("/lobbies"))!["lobbies"]!.AsArray();
        Assert.Equal("v1", (string?)list[0]!["gameVersion"]);
        var refused = await client.PostAsJsonAsync($"/lobbies/{(string?)list[0]!["id"]}/join", new { playerName = "Alice", gameVersion = "v2" });
        Assert.Equal(System.Net.HttpStatusCode.Conflict, refused.StatusCode);
        Assert.Equal(1, app.Services.GetRequiredService<LobbyStore>().Count);
    }
}
