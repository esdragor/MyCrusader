using System.Runtime.CompilerServices;
using System.Runtime.InteropServices;

namespace HOLLauncher;

/**
 * Applies a palette (L22) to a window and everything in it: the window's background and text (inherited by the
 * panels and labels), ThemedButton, the inputs (text boxes, lists, numbers, the news box), the "Files" menu
 * (MenuRenderer). In the dark theme Windows' own parts follow too where it allows: the title bar
 * (DwmSetWindowAttribute, Windows 10 20H1 and later) and the scroll bars and drop-down lists (the DarkMode_Explorer /
 * DarkMode_CFD visual styles); a refusal (older Windows) leaves them light, nothing else. The progress bar keeps
 * Windows' look in both themes.
 */
public static class ThemeApplier
{
    private const int DwmUseImmersiveDarkMode = 20;

    /** The Windows parts (title bar, visual styles) to apply when a control gets its handle, and again if it is recreated. */
    private static readonly ConditionalWeakTable<Control, Palette> Native = new();

    public static void Apply(Control root, Palette palette)
    {
        if (root is Form form)
        {
            form.BackColor = palette.Window;
            form.ForeColor = palette.Text;
            WhenCreated(form, palette);
        }
        ApplyTo(root, palette);
    }

    public static void Apply(ToolStripDropDown menu, Palette palette)
    {
        menu.Renderer = new MenuRenderer(palette);
        menu.BackColor = palette.Surface;
        menu.ForeColor = palette.Text;
        foreach (ToolStripItem item in menu.Items)
        {
            item.ForeColor = palette.Text;
        }
    }

    private static void ApplyTo(Control control, Palette palette)
    {
        switch (control)
        {
            case ThemedButton button:
                button.Palette = palette;
                break;
            case HeaderBand header:
                header.Palette = palette;
                break;
            case RichTextBox or TextBox or NumericUpDown:
                control.BackColor = palette.Surface;
                control.ForeColor = palette.Text;
                WhenCreated(control, palette);
                break;
            case ComboBox combo:
                combo.BackColor = palette.Surface;
                combo.ForeColor = palette.Text;
                                WhenCreated(control, palette);
                break;
        }
        foreach (Control child in control.Controls)
        {
            ApplyTo(child, palette);
        }
    }

    /** Now if the handle exists, and each time it is created (a hidden control gets it when first shown). */
    private static void WhenCreated(Control control, Palette palette)
    {
        Native.AddOrUpdate(control, palette);
        control.HandleCreated -= OnHandleCreated;
        control.HandleCreated += OnHandleCreated;
        if (control.IsHandleCreated)
        {
            ApplyNative(control, palette);
        }
    }

    private static void OnHandleCreated(object? sender, EventArgs e)
    {
        if (sender is Control control && Native.TryGetValue(control, out var palette))
        {
            ApplyNative(control, palette);
        }
    }

    /**
     * The title bar (Windows 10 20H1 and later) or the control's visual style (scroll bars, drop-down button); refused
     * or unknown on this Windows: it keeps Windows' own look, nothing else.
     */
    private static void ApplyNative(Control control, Palette palette)
    {
        try
        {
            if (control is Form)
            {
                var dark = palette.IsDark ? 1 : 0;
                _ = DwmSetWindowAttribute(control.Handle, DwmUseImmersiveDarkMode, ref dark, sizeof(int));
            }
            else
            {
                var style = !palette.IsDark ? "Explorer" : control is ComboBox ? "DarkMode_CFD" : "DarkMode_Explorer";
                _ = SetWindowTheme(control.Handle, style, null);
            }
        }
        catch (Exception e) when (e is DllNotFoundException or EntryPointNotFoundException)
        {
            // Without dwmapi / uxtheme the window keeps Windows' classic look.
        }
    }

    [DllImport("dwmapi.dll")]
    private static extern int DwmSetWindowAttribute(IntPtr hwnd, int attribute, ref int value, int size);

    [DllImport("uxtheme.dll", CharSet = CharSet.Unicode)]
    private static extern int SetWindowTheme(IntPtr hwnd, string? subAppName, string? subIdList);

    /** The "Files" menu in the palette: background, hover, separators, border, text (enabled or not). */
    private sealed class MenuRenderer(Palette palette) : ToolStripProfessionalRenderer(new MenuColors(palette))
    {
        protected override void OnRenderItemText(ToolStripItemTextRenderEventArgs e)
        {
            e.TextColor = e.Item.Enabled ? palette.Text : palette.Muted;
            base.OnRenderItemText(e);
        }

        protected override void OnRenderArrow(ToolStripArrowRenderEventArgs e)
        {
            e.ArrowColor = palette.Text;
            base.OnRenderArrow(e);
        }
    }

    private sealed class MenuColors(Palette palette) : ProfessionalColorTable
    {
        public override Color ToolStripDropDownBackground => palette.Surface;
        public override Color ImageMarginGradientBegin => palette.Surface;
        public override Color ImageMarginGradientMiddle => palette.Surface;
        public override Color ImageMarginGradientEnd => palette.Surface;
        public override Color MenuBorder => palette.Border;
        public override Color MenuItemBorder => palette.Border;
        public override Color MenuItemSelected => palette.MenuSelected;
        public override Color MenuItemSelectedGradientBegin => palette.MenuSelected;
        public override Color MenuItemSelectedGradientEnd => palette.MenuSelected;
        public override Color MenuItemPressedGradientBegin => palette.MenuSelected;
        public override Color MenuItemPressedGradientEnd => palette.MenuSelected;
        public override Color SeparatorDark => palette.Border;
        public override Color SeparatorLight => palette.Surface;
    }
}
