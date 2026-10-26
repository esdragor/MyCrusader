namespace HOLLauncher.Tests;

public class GameServersTests
{
    [Fact]
    public void NoFileGivesLocalManager()
    {
        using var game = new TempDir();
        Assert.Equal(new ManagerServer("Local", "http://127.0.0.1:7700"), GameServers.ReadFirst(game.Root));
    }

    [Fact]
    public void FirstServerIsKept()
    {
        using var game = new TempDir();
        game.Write("servers.json", """
            { "servers": [ { "name": "Home", "url": "http://100.64.0.1:7700" }, { "name": "Other", "url": "http://10.0.0.2:7700" } ] }
            """);
        Assert.Equal(new ManagerServer("Home", "http://100.64.0.1:7700"), GameServers.ReadFirst(game.Root));
    }

    [Fact]
    public void RowsWithoutUrlAreSkippedAndNameIsOptional()
    {
        using var game = new TempDir();
        game.Write("servers.json", """
            { "servers": [ { "name": "Broken" }, { "name": "Empty", "url": "" }, 3, { "url": "http://host:7700" } ] }
            """);
        Assert.Equal(new ManagerServer("", "http://host:7700"), GameServers.ReadFirst(game.Root));
    }

    [Fact]
    public void NoServerGivesLocalManager()
    {
        using var game = new TempDir();
        game.Write("servers.json", """{ "servers": [] }""");
        Assert.Equal(new ManagerServer("Local", "http://127.0.0.1:7700"), GameServers.ReadFirst(game.Root));
    }

    [Theory]
    [InlineData("not json")]
    [InlineData("""{ "other": [] }""")]
    [InlineData("""{ "servers": "http://host" }""")]
    [InlineData("[]")]
    public void UnreadableFileIsAnError(string text)
    {
        using var game = new TempDir();
        var file = game.Write("servers.json", text);
        var e = Assert.Throws<InvalidDataException>(() => GameServers.ReadFirst(game.Root));
        Assert.Contains(file, e.Message);
    }
}
