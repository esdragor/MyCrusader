using HOLManager.Lobbies;
using HOLManager.Matches;
using HOLManager.Versions;

namespace HOLManager.Tests;

/**
 * R8a B2 (D481): Servers\<v>\ of an old version is deleted once v is not the latest, no live lobby nor followed match
 * uses it, and that has lasted ServerCleaner.RetireDelay; never the latest, never a .tmp nor a name out of the rules,
 * never a file; a delete that fails (exe still locked) is retried at the next pass.
 */
public sealed class ServerCleanupTests : IDisposable
{
    private readonly string root = Directory.CreateTempSubdirectory("hol-servers-").FullName;
    private readonly FakeClock clock = new();
    private readonly FakeLauncher launcher = new();

    public void Dispose() => ProcessServerLauncherTests.DeleteDir(root);

    private string Servers => Path.Combine(root, "Servers");
    private string VersionsDir => Path.Combine(root, "Versions");

    private string Server(string version)
    {
        var exe = Path.Combine(Servers, version, "HeroesOfLibertyServer.exe");
        Directory.CreateDirectory(Path.GetDirectoryName(exe)!);
        File.WriteAllText(exe, version);
        return exe;
    }

    private void Publish(string version)
    {
        Directory.CreateDirectory(Path.Combine(VersionsDir, version));
        File.WriteAllText(Path.Combine(VersionsDir, "latest.txt"), version + "\r\n");
    }

    private LobbyStore Store() =>
        new(LobbyEntryTests.Version, clock, launcher: launcher, latestVersion: new VersionStore(VersionsDir).LatestName);

    private ServerCleaner Cleaner(LobbyStore store) => new(Servers, store, clock);

    private bool Has(string version) => Directory.Exists(Path.Combine(Servers, version));

    private void Later() => clock.UtcNow += ServerCleaner.RetireDelay;

    [Fact]
    public void AnOldVersionUnusedIsDeletedAfterTheDelayAndTheLatestNever()
    {
        Server("v1");
        Server("v2");
        Publish("v2");
        var cleaner = Cleaner(Store());

        cleaner.Clean();
        Assert.True(Has("v1"));
        Later();
        cleaner.Clean();

        Assert.False(Has("v1"));
        Assert.True(Has("v2"));
    }

    [Fact]
    public void AVersionUsedByALobbyOrAMatchIsKeptUntilItsEnd()
    {
        Server("v1");
        Server("v2");
        var store = Store();
        Publish("v1");
        var leader = store.Create("Old", "Bob", "v1");
        var guest = store.Join(leader.Lobby.Id, "Alice", "v1");
        store.UpdateMe(leader.Lobby.Id, leader.Token, ready: true);
        store.UpdateMe(leader.Lobby.Id, guest.Token, ready: true);
        var match = store.Start(leader.Lobby.Id, leader.Token);
        Publish("v2");
        var cleaner = Cleaner(store);

        Assert.Equal(["v1"], store.VersionsInUse());
        cleaner.Clean();
        Later();
        cleaner.Clean();
        Assert.True(Has("v1"));

        // Everyone leaves the lobby during the match: only the followed match keeps v1.
        store.Leave(leader.Lobby.Id, guest.Token);
        store.Leave(leader.Lobby.Id, leader.Token);
        Later();
        cleaner.Clean();
        Assert.True(Has("v1"));

        // The server exits: the match is no longer followed, v1 goes after the delay.
        launcher.Alive.Remove(match.Id);
        store.Housekeep();
        Assert.Empty(store.VersionsInUse());
        cleaner.Clean();
        Assert.True(Has("v1"));
        Later();
        cleaner.Clean();
        Assert.False(Has("v1"));
        Assert.True(Has("v2"));
    }

    [Fact]
    public void OnlyValidVersionFoldersAreTouched()
    {
        Server("v2");
        Publish("v2");
        Directory.CreateDirectory(Path.Combine(Servers, "v3.tmp"));
        Directory.CreateDirectory(Path.Combine(Servers, "bad name"));
        File.WriteAllText(Path.Combine(Servers, "notes.txt"), "kept");
        var cleaner = Cleaner(Store());

        cleaner.Clean();
        Later();
        cleaner.Clean();

        Assert.True(Has("v2"));
        Assert.True(Has("v3.tmp"));
        Assert.True(Has("bad name"));
        Assert.True(File.Exists(Path.Combine(Servers, "notes.txt")));
    }

    [Fact]
    public void ALockedServerIsRetriedAtTheNextPass()
    {
        var exe = Server("v1");
        Server("v2");
        Publish("v2");
        var cleaner = Cleaner(Store());
        cleaner.Clean();
        Later();

        using (new FileStream(exe, FileMode.Open, FileAccess.Read, FileShare.None))
        {
            cleaner.Clean();
            Assert.True(File.Exists(exe));
        }
        cleaner.Clean();

        Assert.False(Has("v1"));
    }

    [Fact]
    public void NoServersFolderIsNotAnError()
    {
        Publish("v2");
        Cleaner(Store()).Clean();
        Assert.False(Directory.Exists(Servers));
    }
}
