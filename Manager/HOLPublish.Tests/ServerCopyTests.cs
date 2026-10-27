namespace HOLPublish.Tests;

/**
 * R8a B2 (D481): a publication also copies the dedicated server WindowsServer\ into <servers>\<v>\ (without .pdb),
 * before latest.txt; a failure leaves no server folder; pruning Versions\ never touches the servers (the manager owns them).
 */
public sealed class ServerCopyTests : IDisposable
{
    private readonly TempDir dir = new();

    public void Dispose() => dir.Dispose();

    private string Versions => Path.Combine(dir.Root, "versions");
    private string Servers => Path.Combine(dir.Root, "servers");

    private string Game(string version)
    {
        dir.Write(Path.Combine("games", version, "Version.txt"), version);
        dir.Write(Path.Combine("games", version, "HeroesOfLiberty.exe"), $"game {version}");
        return Path.Combine(dir.Root, "games", version);
    }

    private string Server(string version, bool withExe = true)
    {
        var root = Path.Combine("servers-src", version);
        if (withExe)
        {
            dir.Write(Path.Combine(root, "HeroesOfLibertyServer.exe"), $"server {version}");
        }
        dir.Write(Path.Combine(root, "HeroesOfLiberty", "Binaries", "Win64", "x.dll"), "dll");
        dir.Write(Path.Combine(root, "HeroesOfLiberty", "Binaries", "Win64", "HeroesOfLibertyServer.pdb"), "symbols");
        return Path.Combine(dir.Root, root);
    }

    private PublishResult Publish(string version, string? server, int keep = 3, string? hdiffz = null) =>
        Publisher.Publish(new PublishOptions(Game(version), Versions, Keep: keep, Hdiffz: hdiffz ?? HDiffPatchTool.Hdiffz,
            ServerDir: server, ServersDir: server != null ? Servers : null));

    private string[] ServerDirs() =>
        Directory.GetDirectories(Servers).Select(d => Path.GetFileName(d)!).Order(StringComparer.Ordinal).ToArray();

    [Fact]
    public void TheServerIsCopiedWithoutPdb()
    {
        Publish("v1", Server("v1"));

        var copy = Path.Combine(Servers, "v1");
        Assert.Equal("server v1", File.ReadAllText(Path.Combine(copy, "HeroesOfLibertyServer.exe")));
        Assert.True(File.Exists(Path.Combine(copy, "HeroesOfLiberty", "Binaries", "Win64", "x.dll")));
        Assert.Empty(Directory.GetFiles(copy, "*.pdb", SearchOption.AllDirectories));
        Assert.Equal(["v1"], ServerDirs());
        Assert.Equal("v1", File.ReadAllText(Path.Combine(Versions, "latest.txt")).Trim());
    }

    [Fact]
    public void AServerWithoutItsExeIsRefusedAndNothingIsPublished()
    {
        var e = Assert.Throws<PublishException>(() => Publish("v1", Server("v1", withExe: false)));

        Assert.Contains("HeroesOfLibertyServer.exe", e.Message);
        Assert.False(File.Exists(Path.Combine(Versions, "latest.txt")));
        Assert.False(Directory.Exists(Path.Combine(Versions, "v1")));
        Assert.False(Directory.Exists(Servers) && ServerDirs().Length > 0);
    }

    [Fact]
    public void AServerFolderAlreadyThereIsRefused()
    {
        dir.Write(Path.Combine("servers", "v1", "HeroesOfLibertyServer.exe"), "old");

        Assert.Throws<PublishException>(() => Publish("v1", Server("v1")));

        Assert.Equal("old", File.ReadAllText(Path.Combine(Servers, "v1", "HeroesOfLibertyServer.exe")));
        Assert.False(File.Exists(Path.Combine(Versions, "latest.txt")));
    }

    [Fact]
    public void AFailureAfterTheCopyLeavesNoServerFolder()
    {
        Publish("v1", Server("v1"));

        // The second publication fails on hdiffz (a patch from v1 is attempted): its server must not stay.
        Assert.Throws<PublishException>(() => Publish("v2", Server("v2"), hdiffz: Path.Combine(dir.Root, "nowhere", "hdiffz.exe")));

        Assert.Equal(["v1"], ServerDirs());
        Assert.Equal("v1", File.ReadAllText(Path.Combine(Versions, "latest.txt")).Trim());
    }

    [Fact]
    public void PruningTheVersionsKeepsTheServers()
    {
        Publish("v1", Server("v1"), keep: 0);
        Publish("v2", Server("v2"), keep: 0);
        Publish("v3", Server("v3"), keep: 0);

        Assert.Equal(["v3"], Directory.GetDirectories(Versions).Select(d => Path.GetFileName(d)!));
        Assert.Equal(["v1", "v2", "v3"], ServerDirs());
    }

    [Fact]
    public void WithoutServerOptionNoServersFolderIsWritten()
    {
        Publish("v1", null);

        Assert.False(Directory.Exists(Servers));
    }
}
