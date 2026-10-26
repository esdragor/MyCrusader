namespace HOLPublish.Tests;

/** L4a: the options of HOLPublish and its one-line verdict. */
public class CommandLineTests : IDisposable
{
    private readonly TempDir dir = new();

    public void Dispose() => dir.Dispose();

    [Fact]
    public void AllOptionsAreRead()
    {
        var options = CommandLine.Parse(["--game", @"C:\G\Windows", "--versions", @"D:\V", "--version", "2026-10-25-b",
            "--notes", "n.txt", "--keep", "5", "--hdiffz", @"C:\T\hdiffz.exe"]);
        Assert.Equal(new PublishOptions(@"C:\G\Windows", @"D:\V", "2026-10-25-b", "n.txt", 5, @"C:\T\hdiffz.exe"), options);
    }

    [Fact]
    public void DefaultsKeepThreeAndNoVersionNotesNorHdiffz()
    {
        Assert.Equal(new PublishOptions("g", "v"), CommandLine.Parse(["--versions", "v", "--game", "g"]));
        Assert.Equal(3, new PublishOptions("g", "v").Keep);
    }

    [Theory]
    [InlineData("--versions", "v")]
    [InlineData("--game", "g")]
    [InlineData("--game", "g", "--versions", "v", "--keep", "x")]
    [InlineData("--game", "g", "--versions", "v", "--keep", "-1")]
    [InlineData("--game", "g", "--versions", "v", "--color", "red")]
    [InlineData("--game", "g", "--versions")]
    public void BadCommandLinesAreRefused(params string[] args)
    {
        Assert.Throws<ArgumentException>(() => CommandLine.Parse(args));
    }

    [Fact]
    public void RunPrintsPassAndReturnsZero()
    {
        dir.Write(@"game\Version.txt", "2026-10-25-a");
        dir.Write(@"game\HeroesOfLiberty.exe", new byte[3 * 1024 * 1024]);
        var output = new StringWriter();
        var code = Program.Run(["--game", Path.Combine(dir.Root, "game"), "--versions", Path.Combine(dir.Root, "versions"),
            "--hdiffz", HDiffPatchTool.Hdiffz], output);
        Assert.Equal(0, code);
        Assert.Matches(@"^HOLPublish : PASS 2026-10-25-a \(2 fichiers, \d+,\d Mo compressés, 0 correctifs, 0,0 Mo\)\r?\n$",
            output.ToString());
    }

    [Fact]
    public void RunPrintsFailAndReturnsOne()
    {
        var output = new StringWriter();
        var code = Program.Run(["--game", Path.Combine(dir.Root, "missing"), "--versions", Path.Combine(dir.Root, "v")], output);
        Assert.Equal(1, code);
        Assert.StartsWith("HOLPublish : FAIL (", output.ToString());
        Assert.False(Directory.Exists(Path.Combine(dir.Root, "v", "missing")));
    }

    [Fact]
    public void RunWithBadOptionsPrintsFail()
    {
        var output = new StringWriter();
        Assert.Equal(1, Program.Run(["--keep"], output));
        Assert.StartsWith("HOLPublish : FAIL (", output.ToString());
    }
}
