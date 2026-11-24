using Microsoft.Win32;

namespace HOLLauncher;

/**
 * The launcher's colours (L22), in one place: a light and a dark palette, applied by ThemeApplier to both windows, the
 * buttons (ThemedButton), the news box and the "Files" menu. Every text colour is listed with its background in
 * TextPairs so that a test keeps each pair readable (contrast 4.5, WCAG AA), the disabled buttons' included: WinForms
 * draws a disabled button's text in its own grey, which nearly vanishes on a dark background, hence ThemedButton.
 * The header band (HeaderBand) keeps the same deep red and gold in both themes: the game's mark, not a surface.
 */
public sealed record Palette(
    bool IsDark,
    Color Window,
    Color Text,
    Color Muted,
    Color Surface,
    Color Border,
    Color Heading,
    Color HeaderBack,
    Color HeaderTitle,
    Color HeaderText,
    Color ButtonBack,
    Color ButtonText,
    Color ButtonBorder,
    Color ButtonHover,
    Color ButtonPressed,
    Color ButtonDisabledBack,
    Color ButtonDisabledText,
    Color AccentBack,
    Color AccentText,
    Color AccentHover,
    Color MenuSelected)
{
    public static readonly Palette Light = new(
        IsDark: false,
        Window: Rgb(0xF4F1EA),
        Text: Rgb(0x1F1B16),
        Muted: Rgb(0x5E564B),
        Surface: Rgb(0xFFFFFF),
        Border: Rgb(0xC9C1B3),
        Heading: Rgb(0x7A2620),
        HeaderBack: Rgb(0x4A1612),
        HeaderTitle: Rgb(0xE9C46A),
        HeaderText: Rgb(0xEFE3D0),
        ButtonBack: Rgb(0xE6E0D4),
        ButtonText: Rgb(0x1F1B16),
        ButtonBorder: Rgb(0xB3AA9A),
        ButtonHover: Rgb(0xDAD2C3),
        ButtonPressed: Rgb(0xCCC3B2),
        ButtonDisabledBack: Rgb(0xEDE9E1),
        ButtonDisabledText: Rgb(0x6B6459),
        AccentBack: Rgb(0x8A2A22),
        AccentText: Rgb(0xFFFFFF),
        AccentHover: Rgb(0x731F18),
        MenuSelected: Rgb(0xE6DCCB));

    public static readonly Palette Dark = new(
        IsDark: true,
        Window: Rgb(0x1C1A17),
        Text: Rgb(0xECE6DC),
        Muted: Rgb(0xA9A092),
        Surface: Rgb(0x25221E),
        Border: Rgb(0x4A443B),
        Heading: Rgb(0xE3B95F),
        HeaderBack: Rgb(0x3A100D),
        HeaderTitle: Rgb(0xE9C46A),
        HeaderText: Rgb(0xE6D8C2),
        ButtonBack: Rgb(0x35312B),
        ButtonText: Rgb(0xECE6DC),
        ButtonBorder: Rgb(0x5C554A),
        ButtonHover: Rgb(0x433E36),
        ButtonPressed: Rgb(0x524B42),
        ButtonDisabledBack: Rgb(0x2A2723),
        ButtonDisabledText: Rgb(0x9A9284),
        AccentBack: Rgb(0x9E3229),
        AccentText: Rgb(0xFFFFFF),
        AccentHover: Rgb(0xB23B31),
        MenuSelected: Rgb(0x433E36));

    /** Every text colour with the background it is drawn on (tests: contrast). */
    public IEnumerable<(string Name, Color Fore, Color Back)> TextPairs() =>
    [
        ("Text", Text, Window),
        ("TextOnSurface", Text, Surface),
        ("Muted", Muted, Window),
        ("MutedOnSurface", Muted, Surface),
        ("Heading", Heading, Surface),
        ("HeaderTitle", HeaderTitle, HeaderBack),
        ("HeaderText", HeaderText, HeaderBack),
        ("ButtonText", ButtonText, ButtonBack),
        ("ButtonTextHover", ButtonText, ButtonHover),
        ("ButtonTextPressed", ButtonText, ButtonPressed),
        ("ButtonDisabledText", ButtonDisabledText, ButtonDisabledBack),
        ("AccentText", AccentText, AccentBack),
        ("AccentTextHover", AccentText, AccentHover),
        ("MenuText", Text, Surface),
        ("MenuSelectedText", Text, MenuSelected),
    ];

    /** WCAG 2 contrast ratio, 1 (same) to 21 (black on white). */
    public static double Contrast(Color a, Color b)
    {
        var (la, lb) = (Luminance(a), Luminance(b));
        return (Math.Max(la, lb) + 0.05) / (Math.Min(la, lb) + 0.05);
    }

    /** WCAG 2 relative luminance of an sRGB colour, 0 (black) to 1 (white). */
    public static double Luminance(Color c)
    {
        static double Channel(byte value)
        {
            var v = value / 255.0;
            return v <= 0.03928 ? v / 12.92 : Math.Pow((v + 0.055) / 1.055, 2.4);
        }
        return 0.2126 * Channel(c.R) + 0.7152 * Channel(c.G) + 0.0722 * Channel(c.B);
    }

    private static Color Rgb(int rgb) => Color.FromArgb((rgb >> 16) & 0xFF, (rgb >> 8) & 0xFF, rgb & 0xFF);
}

/**
 * The theme setting (L22, LauncherSettings.Theme): "light" or "dark" forces it; none, "system" or an unknown word
 * follows Windows' app theme (Settings > Personalisation > Colours), read once when the window opens or the settings
 * change: HKCU\...\Themes\Personalize, AppsUseLightTheme 0 = dark, 1 or missing (older Windows) = light.
 */
public static class Themes
{
    public const string FollowSystem = "system";
    public const string Light = "light";
    public const string Dark = "dark";

    public static readonly IReadOnlyList<string> Choices = [FollowSystem, Light, Dark];

    private const string PersonalizeKey = @"Software\Microsoft\Windows\CurrentVersion\Themes\Personalize";

    public static Palette Resolve(string? setting, int? appsUseLightTheme) => ChoiceOf(setting) switch
    {
        Dark => Palette.Dark,
        Light => Palette.Light,
        _ => appsUseLightTheme == 0 ? Palette.Dark : Palette.Light,
    };

    /** The palette for these settings on this Windows. */
    public static Palette Current(LauncherSettings settings) => Resolve(settings.Theme, ReadAppsUseLightTheme());

    /** The choice the settings window shows for the stored setting. */
    public static string ChoiceOf(string? setting)
    {
        var lower = setting?.Trim().ToLowerInvariant();
        return lower is Light or Dark ? lower : FollowSystem;
    }

    /** What the settings file stores for a choice: following Windows is the default, not written. */
    public static string? SettingFor(string choice) => ChoiceOf(choice) is var c && c == FollowSystem ? null : c;

    public static string ChoiceText(string choice, string language) => Texts.Get(ChoiceOf(choice) switch
    {
        Light => "ThemeLight",
        Dark => "ThemeDark",
        _ => "ThemeSystem",
    }, language);

    /** Windows' AppsUseLightTheme (null: missing, not a number, or the registry refused). */
    public static int? ReadAppsUseLightTheme()
    {
        try
        {
            using var key = Registry.CurrentUser.OpenSubKey(PersonalizeKey);
            return key?.GetValue("AppsUseLightTheme") is int value ? value : null;
        }
        catch (Exception e) when (e is System.Security.SecurityException or UnauthorizedAccessException or IOException)
        {
            return null;
        }
    }
}
