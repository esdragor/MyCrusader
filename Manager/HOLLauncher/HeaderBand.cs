namespace HOLLauncher;

/**
 * The band across the top of the launcher (L22): the game's name in a serif font, gold on deep red, a line under it
 * (Subtitle, in the player's language) and a gold rule along the bottom. The game has no logo yet: the name drawn as
 * text stands for it (a placeholder, Placeholders.txt). Its height follows the fonts (Japanese, DPI): FitHeight.
 */
public sealed class HeaderBand : Control
{
    /** The game's name: a proper name, the same in every language. */
    public const string Title = "Heroes of Liberty";

    private const string TitleFamily = "Georgia";
    private const int PadX = 16;
    private const int PadY = 8;
    private const int Rule = 2;

    private Palette palette = Palette.Light;
    private readonly Font titleFont = new(TitleFamily, 18f, FontStyle.Bold);
    private string subtitle = "";

    public HeaderBand()
    {
        SetStyle(ControlStyles.OptimizedDoubleBuffer | ControlStyles.AllPaintingInWmPaint | ControlStyles.UserPaint
            | ControlStyles.ResizeRedraw, true);
        Dock = DockStyle.Top;
        TabStop = false;
        FitHeight();
    }

    public Palette Palette
    {
        get => palette;
        set
        {
            palette = value;
            Invalidate();
        }
    }

    /** The line under the name ("Game launcher"), in the font of the window (Font, set by the language). */
    public string Subtitle
    {
        get => subtitle;
        set
        {
            subtitle = value;
            FitHeight();
            Invalidate();
        }
    }

    protected override void OnFontChanged(EventArgs e)
    {
        base.OnFontChanged(e);
        FitHeight();
    }

    protected override void OnPaint(PaintEventArgs e)
    {
        var g = e.Graphics;
        var bounds = ClientRectangle;
        using (var back = new System.Drawing.Drawing2D.LinearGradientBrush(bounds, Lighter(palette.HeaderBack), palette.HeaderBack,
                   System.Drawing.Drawing2D.LinearGradientMode.Vertical))
        {
            g.FillRectangle(back, bounds);
        }
        using (var rule = new SolidBrush(palette.HeaderTitle))
        {
            g.FillRectangle(rule, 0, bounds.Height - Rule, bounds.Width, Rule);
        }
        var titleSize = TextRenderer.MeasureText(g, Title, titleFont, Size.Empty, TextFormatFlags.NoPadding);
        TextRenderer.DrawText(g, Title, titleFont, new Point(PadX, PadY), palette.HeaderTitle, TextFormatFlags.NoPadding);
        if (subtitle != "")
        {
            var top = PadY + titleSize.Height + 2;
            var area = new Rectangle(PadX + 2, top, Math.Max(0, bounds.Width - 2 * PadX), Math.Max(0, bounds.Height - top - Rule));
            TextRenderer.DrawText(g, subtitle, Font, area, palette.HeaderText,
                TextFormatFlags.NoPadding | TextFormatFlags.SingleLine | TextFormatFlags.EndEllipsis | TextFormatFlags.NoPrefix);
        }
    }

    protected override void Dispose(bool disposing)
    {
        if (disposing)
        {
            titleFont.Dispose();
        }
        base.Dispose(disposing);
    }

    private void FitHeight()
    {
        var title = TextRenderer.MeasureText(Title, titleFont, Size.Empty, TextFormatFlags.NoPadding).Height;
        var sub = subtitle == "" ? 0 : TextRenderer.MeasureText(subtitle, Font, Size.Empty, TextFormatFlags.NoPadding).Height + 2;
        Height = PadY + title + sub + PadY + Rule;
    }

    private static Color Lighter(Color c) => Color.FromArgb(Math.Min(255, c.R + 28), Math.Min(255, c.G + 12), Math.Min(255, c.B + 10));
}
