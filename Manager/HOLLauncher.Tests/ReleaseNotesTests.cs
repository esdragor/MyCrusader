using HOLUpdate;

namespace HOLLauncher.Tests;

/**
 * L21: which notes the "Nouveautés" box shows. Outdated: every version newer than the installed one, newest first, each
 * headed by its name; the installed one pruned by -Keep: all the kept ones and a line saying older notes are gone.
 * Up to date, not installed, or a manager without history: the latest's notes alone, as before L21.
 */
public class ReleaseNotesTests
{
    /** Three versions published (newest first, as the manager sends them). */
    private static readonly ReleaseNote[] Three =
        [new("0.3.11", "Nouvelle carte."), new("0.3.10", "Archers plus rapides.\nTours moins chères."), new("0.3.9", "Un.")];

    private static LatestVersion Latest(IReadOnlyList<ReleaseNote>? history, string notes = "Nouvelle carte.")
        => new("0.3.11", new VersionManifest("0.3.11", []), notes, [], "", history);

    [Fact]
    public void InstalledFirstOfThreeShowsTheTwoNewerNotesNewestFirst()
    {
        var shown = ReleaseNotes.Select("0.3.9", Latest(Three));

        Assert.Equal(["0.3.11", "0.3.10"], shown.Notes.Select(n => n.Version));
        Assert.True(shown.Headed);
        Assert.False(shown.OlderMissing);
        Assert.Equal("V 0.3.11\r\nNouvelle carte.\r\n\r\nV 0.3.10\r\nArchers plus rapides.\r\nTours moins chères.",
            ReleaseNotes.Text("0.3.9", Latest(Three)));
    }

    [Fact]
    public void InstalledPrunedShowsAllKeptNotesAndSaysOlderAreGone()
    {
        var shown = ReleaseNotes.Select("0.3.2", Latest(Three));

        Assert.Equal(["0.3.11", "0.3.10", "0.3.9"], shown.Notes.Select(n => n.Version));
        Assert.True(shown.OlderMissing);
        Assert.EndsWith("V 0.3.9\r\nUn.\r\n\r\n" + Texts.Get("NotesOlderMissing"), ReleaseNotes.Text("0.3.2", Latest(Three)));
    }

    [Fact]
    public void UpToDateShowsTheLatestNotesAlone()
    {
        Assert.Equal("Nouvelle carte.", ReleaseNotes.Text("0.3.11", Latest(Three)));
    }

    [Fact]
    public void NotInstalledShowsTheLatestNotesAlone()
    {
        var shown = ReleaseNotes.Select(null, Latest(Three));
        Assert.False(shown.Headed);
        Assert.Equal("Nouvelle carte.", ReleaseNotes.Text(null, Latest(Three)));
    }

    [Fact]
    public void OldManagerWithoutHistoryShowsTheLatestNotesAlone()
    {
        Assert.Equal("Nouvelle carte.", ReleaseNotes.Text("0.3.9", Latest(history: null)));
        Assert.Equal("Nouvelle carte.", ReleaseNotes.Text("0.3.9", Latest(history: [])));
    }

    [Fact]
    public void VersionsWithoutNotesAreLeftOut()
    {
        ReleaseNote[] history = [new("0.3.11", ""), new("0.3.10", "Archers."), new("0.3.9", "Un.")];

        Assert.Equal("V 0.3.10\r\nArchers.", ReleaseNotes.Text("0.3.9", Latest(history, notes: "")));
        Assert.Equal("", ReleaseNotes.Text("0.3.10", Latest(history, notes: "")));
    }

    [Fact]
    public void NoNotesAtAllShowsNothingEvenWhenOlderAreGone()
    {
        ReleaseNote[] history = [new("0.3.11", ""), new("0.3.10", " ")];
        Assert.Equal("", ReleaseNotes.Text("0.3.2", Latest(history, notes: "")));
    }
}
