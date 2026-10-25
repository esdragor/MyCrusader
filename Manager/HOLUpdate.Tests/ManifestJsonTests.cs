namespace HOLUpdate.Tests;

/** L1: the list written to and read from JSON (served by the manager in L2, kept by the launcher next to the game). */
public class ManifestJsonTests : IDisposable
{
    private readonly TempDir dir = new();

    public void Dispose() => dir.Dispose();

    private VersionManifest BuildSample()
    {
        dir.Write("Version.txt", "2026-10-25-3713f76");
        dir.Write("HeroesOfLiberty.exe", "exe");
        dir.Write("HeroesOfLiberty/Content/Paks/HeroesOfLiberty-Windows.ucas", "ucas");
        dir.Write("Engine/Binaries/ThirdParty/x.dll", "dll");
        return ManifestBuilder.Build(dir.Root);
    }

    [Fact]
    public void RoundTripIsIdentical()
    {
        var manifest = BuildSample();
        var json = manifest.ToJson();
        var back = VersionManifest.FromJson(json);
        Assert.Equal((manifest.Version, manifest.Format), (back.Version, back.Format));
        Assert.Equal(manifest.Files, back.Files);
        Assert.Equal(json, back.ToJson());
    }

    [Fact]
    public void SaveAndLoadThroughAFile()
    {
        var manifest = BuildSample();
        using var other = new TempDir();
        var file = Path.Combine(other.Root, "manifest.json");
        manifest.Save(file);
        var back = VersionManifest.Load(file);
        Assert.Equal(manifest.Files, back.Files);
        Assert.Equal(manifest.ToJson(), File.ReadAllText(file));
    }

    [Fact]
    public void JsonNamesAreCamelCase()
    {
        var json = BuildSample().ToJson();
        foreach (var name in new[] { "\"format\"", "\"version\"", "\"files\"", "\"path\"", "\"size\"", "\"sha256\"" })
        {
            Assert.Contains(name, json);
        }
    }

    [Theory]
    [InlineData("../evil.dll")]
    [InlineData("a/../../evil.dll")]
    [InlineData("/abs.dll")]
    [InlineData("C:/abs.dll")]
    [InlineData(@"a\b.dll")]
    [InlineData("")]
    public void UnsafePathIsRejected(string path)
    {
        var json = $$"""{"format":1,"version":"v","files":[{"path":{{System.Text.Json.JsonSerializer.Serialize(path)}},"size":1,"sha256":"{{new string('a', 64)}}"}]}""";
        Assert.Throws<InvalidDataException>(() => VersionManifest.FromJson(json));
    }

    [Theory]
    [InlineData("ABC")]
    [InlineData("zz")]
    public void BadHashIsRejected(string hash)
    {
        var json = $$"""{"format":1,"version":"v","files":[{"path":"a.dll","size":1,"sha256":"{{hash}}"}]}""";
        Assert.Throws<InvalidDataException>(() => VersionManifest.FromJson(json));
    }

    [Fact]
    public void NewerFormatIsRejected()
    {
        var json = $$"""{"format":{{VersionManifest.CurrentFormat + 1}},"version":"v","files":[]}""";
        Assert.Throws<InvalidDataException>(() => VersionManifest.FromJson(json));
    }
}
