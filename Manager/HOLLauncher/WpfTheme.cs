using System.Windows;
using System.Windows.Media;

namespace HOLLauncher;

/**
 * The palette as WPF brushes (launcher 5): one ResourceDictionary per palette, merged into each window's resources and
 * swapped when the theme changes (Apply); the XAML only names the keys (DynamicResource), so a swap recolours a window
 * that is open. The brushes are frozen (shared, cheap). Keys are the palette's names plus "Brush".
 */
public static class WpfTheme
{
    /** The keys every window may use, each with its colour for a palette (tests: all present, all from the palette). */
    public static IReadOnlyList<(string Key, System.Drawing.Color Color, double Opacity)> Entries(Palette p) =>
    [
        ("WindowBrush", p.Window, 1),
        ("TextBrush", p.Text, 1),
        ("MutedBrush", p.Muted, 1),
        ("SurfaceBrush", p.Surface, 1),
        ("BorderBrush", p.Border, 1),
        ("HeadingBrush", p.Heading, 1),
        ("ButtonBackBrush", p.ButtonBack, 1),
        ("ButtonTextBrush", p.ButtonText, 1),
        ("ButtonBorderBrush", p.ButtonBorder, 1),
        ("ButtonHoverBrush", p.ButtonHover, 1),
        ("ButtonPressedBrush", p.ButtonPressed, 1),
        ("ButtonDisabledBackBrush", p.ButtonDisabledBack, 1),
        ("ButtonDisabledTextBrush", p.ButtonDisabledText, 1),
        ("AccentBackBrush", p.AccentBack, 1),
        ("AccentTextBrush", p.AccentText, 1),
        ("AccentHoverBrush", p.AccentHover, 1),
        ("MenuSelectedBrush", p.MenuSelected, 1),
        ("ChromeBackBrush", p.ChromeBack, 1),
        ("ChromeTextBrush", p.ChromeText, 1),
        ("ChromeHoverBrush", p.ChromeHover, 1),
        ("CardBrush", p.Card, 1),
        ("PanelBrush", p.Surface, p.PanelOpacity),
        ("ArtTextBrush", Palette.ArtText, 1),
        ("ArtMutedBrush", Palette.ArtMuted, 1),
        ("GoldBrush", Palette.Gold, 1),
        ("CloseHoverBrush", Palette.CloseHover, 1),
        ("ServerReachedBrush", Palette.ServerReached, 1),
        ("ServerUnreachableBrush", Palette.ServerUnreachable, 1),
        ("ServerUnknownBrush", Palette.ServerUnknown, 1),
    ];

    public static ResourceDictionary Resources(Palette palette)
    {
        var dictionary = new ResourceDictionary();
        foreach (var (key, color, opacity) in Entries(palette))
        {
            dictionary[key] = Brush(color, opacity);
        }
        return dictionary;
    }

    /** Replaces the palette's dictionary among the window's merged ones (the first merged one is the palette's). */
    public static void Apply(FrameworkElement element, Palette palette)
    {
        var merged = element.Resources.MergedDictionaries;
        var fresh = Resources(palette);
        if (merged.Count > 0 && merged[0].Contains("WindowBrush"))
        {
            merged[0] = fresh;
        }
        else
        {
            merged.Insert(0, fresh);
        }
    }

    public static SolidColorBrush Brush(System.Drawing.Color color, double opacity = 1)
    {
        var brush = new SolidColorBrush(ToWpf(color)) { Opacity = opacity };
        brush.Freeze();
        return brush;
    }

    public static Color ToWpf(System.Drawing.Color color) => Color.FromArgb(color.A, color.R, color.G, color.B);
}
