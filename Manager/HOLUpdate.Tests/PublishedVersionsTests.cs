namespace HOLUpdate.Tests;

/**
 * L21: the publication order of a versions folder, shared by the publisher (patches, pruning) and the manager (notes
 * history): published.txt first, else manifest.json's last write; names never decide but to break a tie.
 */
public sealed class PublishedVersionsTests : IDisposable
{
    private readonly TempDir dir = new();

    public void Dispose() => dir.Dispose();

    private void Version(string name, DateTime? published)
    {
        new VersionManifest(name, []).Save(dir.Write(Path.Combine(name, PublishedVersions.ManifestFile), ""));
        if (published is { } date)
        {
            dir.Write(Path.Combine(name, PublishedVersions.DateFile), date.ToString("O"));
        }
    }

    [Fact]
    public void NewestFirstByPublicationDateNotByName()
    {
        Version("0.3.10", new DateTime(2026, 11, 3, 0, 0, 0, DateTimeKind.Utc));
        Version("0.3.9", new DateTime(2026, 11, 2, 0, 0, 0, DateTimeKind.Utc));
        Version("2026-10-30-r9", new DateTime(2026, 10, 30, 0, 0, 0, DateTimeKind.Utc));
        dir.Write(Path.Combine("launcher", "latest.txt"), "2");
        dir.Write(Path.Combine("0.3.11.tmp", "notes.txt"), "being written, no list yet");

        Assert.Equal(["0.3.10", "0.3.9", "2026-10-30-r9"], PublishedVersions.NewestFirst(dir.Root, _ => true));
        Assert.Equal(["0.3.10", "2026-10-30-r9"], PublishedVersions.NewestFirst(dir.Root, name => name != "0.3.9"));
        Assert.Equal(new DateTime(2026, 11, 2, 0, 0, 0, DateTimeKind.Utc), PublishedVersions.Date(Path.Combine(dir.Root, "0.3.9")));
    }

    [Fact]
    public void WithoutPublishedTxtTheListsLastWriteCounts()
    {
        Version("hand-copied", published: null);
        File.SetLastWriteTimeUtc(Path.Combine(dir.Root, "hand-copied", PublishedVersions.ManifestFile),
            new DateTime(2026, 11, 5, 0, 0, 0, DateTimeKind.Utc));
        Version("0.3.10", new DateTime(2026, 11, 3, 0, 0, 0, DateTimeKind.Utc));

        Assert.Equal(["hand-copied", "0.3.10"], PublishedVersions.NewestFirst(dir.Root, _ => true));
    }

    [Fact]
    public void MissingFolderHasNoVersion()
    {
        Assert.Empty(PublishedVersions.NewestFirst(Path.Combine(dir.Root, "nowhere"), _ => true));
    }
}
