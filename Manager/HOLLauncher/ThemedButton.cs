namespace HOLLauncher;

/**
 * A button drawn in the palette's colours (L22). WinForms draws a disabled button's text in its own grey whatever the
 * colours, which nearly vanishes on a dark background: this one paints itself (background, border, text) from Colors,
 * where a disabled button has colours of its own, kept readable (ThemeTests). Accent: the main action ("Play").
 * Without a palette it is a plain WinForms button. The size stays WinForms' (AutoSize from the text and Padding).
 */
public sealed class ThemedButton : Button
{
    private Palette? palette;
    private bool hover;
    private bool pressed;

    public ThemedButton()
    {
        Padding = new Padding(8, 3, 8, 3);
        SetStyle(ControlStyles.OptimizedDoubleBuffer | ControlStyles.AllPaintingInWmPaint | ControlStyles.UserPaint, true);
    }

    public Palette? Palette
    {
        get => palette;
        set
        {
            palette = value;
            FlatStyle = value is null ? FlatStyle.Standard : FlatStyle.Flat;
            if (value is not null)
            {
                FlatAppearance.BorderSize = 1;
            }
            Invalidate();
        }
    }

    /** The main action of its window, drawn in the accent colours while enabled. */
    public bool Accent { get; set; }

    /** Background, text and border for a state (pure, tested): disabled wins over hover and pressed. */
    public static (Color Back, Color Fore, Color Border) Colors(Palette p, bool enabled, bool hover, bool pressed, bool accent)
    {
        if (!enabled)
        {
            return (p.ButtonDisabledBack, p.ButtonDisabledText, p.Border);
        }
        if (accent)
        {
            return (hover || pressed ? p.AccentHover : p.AccentBack, p.AccentText, p.AccentHover);
        }
        return (pressed ? p.ButtonPressed : hover ? p.ButtonHover : p.ButtonBack, p.ButtonText, p.ButtonBorder);
    }

    protected override void OnPaint(PaintEventArgs e)
    {
        if (palette is not { } p)
        {
            base.OnPaint(e);
            return;
        }
        var (back, fore, border) = Colors(p, Enabled, hover, pressed, Accent);
        var g = e.Graphics;
        var bounds = ClientRectangle;
        using (var brush = new SolidBrush(back))
        {
            g.FillRectangle(brush, bounds);
        }
        using (var pen = new Pen(border))
        {
            g.DrawRectangle(pen, bounds.X, bounds.Y, bounds.Width - 1, bounds.Height - 1);
        }
        if (Focused && ShowFocusCues && Enabled)
        {
            // Two pixels in the heading colour: the dotted focus rectangle is hard to see on a flat dark button.
            using var focus = new Pen(Accent ? p.AccentText : p.Heading, 2);
            g.DrawRectangle(focus, bounds.X + 2, bounds.Y + 2, bounds.Width - 5, bounds.Height - 5);
        }
        var flags = TextFormatFlags.HorizontalCenter | TextFormatFlags.VerticalCenter | TextFormatFlags.SingleLine
            | TextFormatFlags.EndEllipsis | (ShowKeyboardCues ? 0 : TextFormatFlags.HidePrefix);
        TextRenderer.DrawText(g, Text, Font, bounds, fore, back, flags);
    }

    protected override void OnMouseEnter(EventArgs e)
    {
        hover = true;
        Invalidate();
        base.OnMouseEnter(e);
    }

    protected override void OnMouseLeave(EventArgs e)
    {
        hover = false;
        pressed = false;
        Invalidate();
        base.OnMouseLeave(e);
    }

    protected override void OnMouseDown(MouseEventArgs e)
    {
        pressed = e.Button == MouseButtons.Left;
        Invalidate();
        base.OnMouseDown(e);
    }

    protected override void OnMouseUp(MouseEventArgs e)
    {
        pressed = false;
        Invalidate();
        base.OnMouseUp(e);
    }

    protected override void OnEnabledChanged(EventArgs e)
    {
        hover = pressed = false;
        Invalidate();
        base.OnEnabledChanged(e);
    }
}
