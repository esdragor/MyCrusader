namespace HOLLauncher.Tests;

public class GameFolderTests
{
    [Fact]
    public void NewFolderIsCreatedWithTheKitServers()
    {
        using var dir = new TempDir();
        var old = Path.Combine(dir.Root, "old");
        dir.Write("old/servers.json", "{ \"servers\": [ { \"url\": \"http://x:7700\", \"key\": \"k\" } ] }");
        var target = Path.Combine(dir.Root, "new", "deeper");
        GameFolder.Prepare(old, target);
        Assert.Equal("k", GameServers.ReadFirst(target).Key);
    }

    [Fact]
    public void ServersOfTheNewFolderAreKept()
    {
        using var dir = new TempDir();
        dir.Write("old/servers.json", "{ \"servers\": [ { \"url\": \"http://x:7700\", \"key\": \"old\" } ] }");
        dir.Write("new/servers.json", "{ \"servers\": [ { \"url\": \"http://y:7700\", \"key\": \"new\" } ] }");
        GameFolder.Prepare(Path.Combine(dir.Root, "old"), Path.Combine(dir.Root, "new"));
        Assert.Equal("new", GameServers.ReadFirst(Path.Combine(dir.Root, "new")).Key);
    }

    [Fact]
    public void NoServersToCopyIsFine()
    {
        using var dir = new TempDir();
        var target = Path.Combine(dir.Root, "new");
        GameFolder.Prepare(Path.Combine(dir.Root, "old"), target);
        Assert.True(Directory.Exists(target));
        Assert.False(File.Exists(Path.Combine(target, GameServers.FileName)));
    }

    [Theory]
    [InlineData(@"C:\Games\HOL", @"C:\Games\HOL\", true)]
    [InlineData(@"C:\Games\HOL", @"c:\games\hol", true)]
    [InlineData(@"C:\Games\HOL", @"C:\Games\HOL2", false)]
    public void SameFolder(string a, string b, bool same)
    {
        Assert.Equal(same, GameFolder.Same(a, b));
    }
}
