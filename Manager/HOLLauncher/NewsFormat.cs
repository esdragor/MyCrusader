namespace HOLLauncher;

/**
 * One styled paragraph of the "What's new" box (L22); Bullet: the box draws a bullet and indents it. Size in points.
 * Family: its own font (null: the language's, Texts.FontFamily).
 */
public sealed record NewsParagraph(string Text, float Size, FontStyle Style, Color Color, bool Bullet, string? Family = null);

/**
 * L22: how the "What's new" box shows the notes' blocks (ReleaseNotes.Blocks). Paragraphs is pure (tested): version
 * headings bold, larger and in the palette's heading colour, bullet lines with a real bullet, blank lines a little
 * shorter than a text line, the "older notes are gone" line in italics and muted. A heading is a version's name, Latin
 * in every language: Segoe UI, whose bold Yu Gothic UI (Japanese) would barely show. Fill writes them into a read-only
 * RichTextBox (no WebView2: not sure to be on the players' PCs, and a dependency for a few styles). No links.
 */
public static class NewsFormat
{
    public const float BodySize = 10f;
    public const float HeadingSize = 12.5f;
    public const float GapSize = 7f;
    public const string HeadingFamily = "Segoe UI";

    /** Pixels between the box's left edge and a bullet, then between the bullet and its text. */
    private const int BulletLeft = 6;
    private const int BulletText = 14;

    public static IReadOnlyList<NewsParagraph> Paragraphs(IEnumerable<NewsBlock> blocks, Palette palette)
        => blocks.Select(b => b.Kind switch
        {
            NewsKind.Heading => new NewsParagraph(b.Text, HeadingSize, FontStyle.Bold, palette.Heading, false, HeadingFamily),
            NewsKind.Bullet => new NewsParagraph(b.Text, BodySize, FontStyle.Regular, palette.Text, true),
            NewsKind.Gap => new NewsParagraph("", GapSize, FontStyle.Regular, palette.Text, false),
            NewsKind.OlderMissing => new NewsParagraph(b.Text, BodySize, FontStyle.Italic, palette.Muted, false),
            _ => new NewsParagraph(b.Text, BodySize, FontStyle.Regular, palette.Text, false),
        }).ToList();

    /** The box's text once filled: RichTextBox separates paragraphs with "\n". */
    public static string BoxText(IEnumerable<NewsParagraph> paragraphs) => string.Join("\n", paragraphs.Select(p => p.Text));

    /**
     * Replaces the box's content, paragraph by paragraph: each one's format is set at the end of the text, then its
     * text typed there (the paragraph format applies to the paragraph the caret is in). Scrolled back to the top.
     */
    public static void Fill(RichTextBox box, IReadOnlyList<NewsParagraph> paragraphs, string fontFamily)
    {
        var fonts = new Dictionary<(string, float, FontStyle), Font>();
        try
        {
            box.SuspendLayout();
            box.Clear();
            for (var i = 0; i < paragraphs.Count; i++)
            {
                var p = paragraphs[i];
                var key = (p.Family ?? fontFamily, p.Size, p.Style);
                if (!fonts.TryGetValue(key, out var font))
                {
                    font = new Font(key.Item1, p.Size, p.Style);
                    fonts[key] = font;
                }
                box.Select(box.TextLength, 0);
                box.SelectionFont = font;
                box.SelectionColor = p.Color;
                // BulletIndent is read when SelectionBullet is set: first.
                box.BulletIndent = p.Bullet ? BulletText : 0;
                box.SelectionBullet = p.Bullet;
                box.SelectionIndent = p.Bullet ? BulletLeft : 0;
                box.SelectedText = i < paragraphs.Count - 1 ? p.Text + "\n" : p.Text;
            }
            box.Select(0, 0);
            box.ScrollToCaret();
        }
        finally
        {
            box.ResumeLayout();
            // The box copies the format into its own text: the fonts are no longer needed.
            foreach (var font in fonts.Values)
            {
                font.Dispose();
            }
        }
    }
}
