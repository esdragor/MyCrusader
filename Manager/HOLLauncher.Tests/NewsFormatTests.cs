using System.Drawing;
using HOLUpdate;

namespace HOLLauncher.Tests;

/**
 * L22: the "What's new" box is a RichTextBox. ReleaseNotes gives blocks (version headings, text lines, bullet lines,
 * gaps, the "older notes" line) and NewsFormat turns them into styled paragraphs; the structure is tested, not pixels.
 */
public class NewsFormatTests
{
    private static LatestVersion Latest(IReadOnlyList<ReleaseNote>? history, string notes = "Nouvelle carte.")
        => new("0.3.11", new VersionManifest("0.3.11", []), notes, [], "", history);

    private static NewsBlock H(string text) => new(NewsKind.Heading, text);
    private static NewsBlock T(string text) => new(NewsKind.Text, text);
    private static NewsBlock B(string text) => new(NewsKind.Bullet, text);
    private static readonly NewsBlock Gap = new(NewsKind.Gap);

    [Fact]
    public void OutdatedGivesOneHeadingPerVersionWithAGapBetween()
    {
        ReleaseNote[] history =
            [new("0.3.11", "Nouvelle carte."), new("0.3.10", "Archers plus rapides.\nTours moins chères."), new("0.3.9", "Un.")];

        Assert.Equal([H("V 0.3.11"), T("Nouvelle carte."), Gap, H("V 0.3.10"), T("Archers plus rapides."), T("Tours moins chères.")],
            ReleaseNotes.Blocks("0.3.9", Latest(history), "fr"));
    }

    [Fact]
    public void DashStarAndBulletLinesBecomeBullets()
    {
        var notes = "Equilibrage :\n- Archers plus rapides.\n  * Tours moins chères.\n• Murs plus solides.\n-5 % de bois n'est pas une puce.";

        Assert.Equal([T("Equilibrage :"), B("Archers plus rapides."), B("Tours moins chères."), B("Murs plus solides."),
                T("-5 % de bois n'est pas une puce.")],
            ReleaseNotes.Blocks(null, Latest(null, notes), "fr"));
    }

    [Fact]
    public void BlankLinesInsideANoteBecomeOneGapAndOuterOnesGo()
    {
        Assert.Equal([T("Un."), Gap, T("Deux.")], ReleaseNotes.Blocks(null, Latest(null, "\n\nUn.\r\n\r\n\r\nDeux.  \n\n"), "fr"));
    }

    [Fact]
    public void OlderMissingIsTheLastBlockAfterAGap()
    {
        ReleaseNote[] history = [new("0.3.11", "Nouvelle carte."), new("0.3.10", "Archers.")];

        var blocks = ReleaseNotes.Blocks("0.3.2", Latest(history), "en");

        Assert.Equal(Gap, blocks[^2]);
        Assert.Equal(new NewsBlock(NewsKind.OlderMissing, Texts.Get("NotesOlderMissing", "en")), blocks[^1]);
    }

    [Fact]
    public void NoNotesGiveNoBlocks()
    {
        Assert.Empty(ReleaseNotes.Blocks(null, Latest(null, notes: " "), "fr"));
    }

    [Fact]
    public void PlainTextWritesBulletsWithADash()
    {
        Assert.Equal("V 0.3.11\r\n- Archers.\r\n\r\nFin.", ReleaseNotes.PlainText([H("V 0.3.11"), B("Archers."), Gap, T("Fin.")]));
    }

    [Theory]
    [InlineData(false)]
    [InlineData(true)]
    public void HeadingsAreBoldAndLargerBulletsAreBulletsOlderMissingIsItalicAndMuted(bool dark)
    {
        var palette = dark ? Palette.Dark : Palette.Light;
        NewsBlock[] blocks = [H("V 0.4.3"), B("Archers."), T("Fin."), Gap, new(NewsKind.OlderMissing, "Plus rien.")];

        var paragraphs = NewsFormat.Paragraphs(blocks, palette);

        Assert.Equal(["V 0.4.3", "Archers.", "Fin.", "", "Plus rien."], paragraphs.Select(p => p.Text));
        var (heading, bullet, text, gap, older) = (paragraphs[0], paragraphs[1], paragraphs[2], paragraphs[3], paragraphs[4]);
        Assert.Equal(FontStyle.Bold, heading.Style);
        Assert.True(heading.Size > text.Size);
        Assert.Equal(palette.Heading, heading.Color);
        Assert.Equal("Segoe UI", heading.Family);
        Assert.False(heading.Bullet);
        Assert.Null(text.Family);
        Assert.True(bullet.Bullet);
        Assert.Equal(FontStyle.Regular, bullet.Style);
        Assert.Equal(palette.Text, bullet.Color);
        Assert.False(text.Bullet);
        Assert.Equal(palette.Text, text.Color);
        Assert.True(gap.Size <= text.Size);
        Assert.Equal(FontStyle.Italic, older.Style);
        Assert.Equal(palette.Muted, older.Color);
    }

    /** The box's text once filled: one paragraph per block (bullets are drawn by the box, not typed). */
    [Fact]
    public void BoxTextIsOneLinePerParagraph()
    {
        var paragraphs = NewsFormat.Paragraphs([H("V 0.4.3"), B("Archers."), Gap, T("Fin.")], Palette.Light);
        Assert.Equal("V 0.4.3\nArchers.\n\nFin.", NewsFormat.BoxText(paragraphs));
    }

    /** The status keeps the blocks (the window formats them) and their plain text (the tests, the report). */
    [Fact]
    public void StatusCarriesTheBlocks()
    {
        ReleaseNote[] history = [new("0.3.11", "- Nouvelle carte."), new("0.3.10", "Archers.")];
        var latest = new LatestResult(LatestOutcome.Ok, Latest(history), "");

        var view = StatusView.From(LauncherStatus.Compute("0.3.10", latest, "1", "fr"), "fr");

        Assert.Equal([H("V 0.3.11"), B("Nouvelle carte.")], view.News);
        Assert.Equal("V 0.3.11\r\n- Nouvelle carte.", view.Notes);
        Assert.True(view.NotesVisible);
        Assert.Empty(StatusView.Loading(language: "fr").News);
    }
}
