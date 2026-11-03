namespace HOLPublish.Tests;

/**
 * L14: the version number of a publication, "0.Y.X" (Y = steps validated by the user, X = update of the current step,
 * from 0), kept in <versions folder>\release.json beside latest.txt: the file holds the NEXT version to publish; a
 * publication that succeeded moves X up, "next-stage" moves Y up and X back to 0. Driven like the other commands, by
 * Program.Run, which publish.ps1 reads.
 */
public sealed class ReleaseTests : IDisposable
{
    private readonly TempDir dir = new();

    public void Dispose() => dir.Dispose();

    private string File => Path.Combine(dir.Root, "versions", "release.json");

    private (int Code, string Output) Run(params string[] args)
    {
        var output = new StringWriter();
        var code = Program.Run(args, output);
        return (code, output.ToString().Trim());
    }

    private void Set(int stage, int update) => Run("release", "set", "--file", File, "--stage", $"{stage}", "--update", $"{update}");

    [Fact]
    public void SetThenNameGivesTheNextVersion()
    {
        Assert.Equal(0, Run("release", "set", "--file", File, "--stage", "3", "--update", "0").Code);
        var (code, output) = Run("release", "name", "--file", File);
        Assert.Equal(0, code);
        Assert.Equal("HOLPublish : RELEASE 0.3.0", output);
    }

    [Fact]
    public void DoneMovesTheUpdateUpWhenTheVersionPublishedWasTheNext()
    {
        Set(3, 0);
        Assert.Equal(0, Run("release", "done", "--file", File, "--version", "0.3.0").Code);
        Assert.Equal("HOLPublish : RELEASE 0.3.1", Run("release", "name", "--file", File).Output);
        Run("release", "done", "--file", File, "--version", "0.3.1");
        Assert.Equal("HOLPublish : RELEASE 0.3.2", Run("release", "name", "--file", File).Output);
    }

    [Theory]
    [InlineData("2026-10-30-r9")]
    [InlineData("0.3.7")]
    [InlineData("dev")]
    public void DoneLeavesTheNumberAloneForAnotherVersion(string published)
    {
        Set(3, 0);
        var (code, output) = Run("release", "done", "--file", File, "--version", published);
        Assert.Equal(0, code);
        Assert.Contains("unchanged", output);
        Assert.Equal("HOLPublish : RELEASE 0.3.0", Run("release", "name", "--file", File).Output);
    }

    [Fact]
    public void NextStageMovesTheStageUpAndTheUpdateBackToZero()
    {
        Set(3, 4);
        var (code, output) = Run("release", "next-stage", "--file", File);
        Assert.Equal(0, code);
        Assert.Equal("HOLPublish : RELEASE 0.4.0", output);
        Assert.Equal("HOLPublish : RELEASE 0.4.0", Run("release", "name", "--file", File).Output);
    }

    [Fact]
    public void WithoutAFileTheNameIsRefusedAndTellsHowToCreateIt()
    {
        var (code, output) = Run("release", "name", "--file", File);
        Assert.Equal(1, code);
        Assert.StartsWith("HOLPublish : FAIL", output);
        Assert.Contains("release set", output);
    }

    [Theory]
    [InlineData("{}")]
    [InlineData("not json")]
    [InlineData("{\"stage\":-1,\"update\":0}")]
    [InlineData("{\"stage\":3}")]
    public void ABadFileIsRefusedAndNeverRewritten(string content)
    {
        var path = dir.Write(Path.Combine("versions", "release.json"), content);
        foreach (var verb in new[] { "name", "next-stage" })
        {
            var (code, output) = Run("release", verb, "--file", path);
            Assert.Equal(1, code);
            Assert.StartsWith("HOLPublish : FAIL", output);
        }
        Assert.Equal(content, System.IO.File.ReadAllText(path));
    }

    [Fact]
    public void TheNameIsAlwaysAValidVersionNameWithoutSpace()
    {
        Set(12, 105);
        var name = Run("release", "name", "--file", File).Output["HOLPublish : RELEASE ".Length..];
        Assert.Equal("0.12.105", name);
        Assert.True(VersionFolder.IsPublishable(name));
    }

    [Theory]
    [InlineData("0.3.0", "V 0.3.0")]
    [InlineData("0.12.105", "V 0.12.105")]
    [InlineData("2026-10-30-r9", "2026-10-30-r9")]
    [InlineData("dev", "dev")]
    public void DisplayAddsTheVOnlyToTheNumberedVersions(string name, string shown) =>
        Assert.Equal(shown, ReleaseNumber.Display(name));
}
