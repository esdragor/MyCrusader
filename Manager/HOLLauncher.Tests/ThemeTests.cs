using System.Drawing;

namespace HOLLauncher.Tests;

/**
 * L22: the light and dark themes. The setting "system" (or none, or an unknown word) follows Windows' app theme
 * (AppsUseLightTheme: 0 dark, 1 light, missing light); "light" and "dark" force it. Every text colour of both palettes
 * stays readable on its background (contrast ratio of WCAG AA, 4.5), disabled buttons included.
 */
public class ThemeTests
{
    [Theory]
    [InlineData(null, 0, true)]
    [InlineData(null, 1, false)]
    [InlineData(null, null, false)]
    [InlineData("system", 0, true)]
    [InlineData("system", null, false)]
    [InlineData("dark", 1, true)]
    [InlineData("dark", null, true)]
    [InlineData("light", 0, false)]
    [InlineData("DARK", 1, true)]
    [InlineData("purple", 0, true)]
    [InlineData("purple", 1, false)]
    public void ThemeIsTheSettingElseWindows(string? setting, int? appsUseLightTheme, bool dark)
    {
        var palette = Themes.Resolve(setting, appsUseLightTheme);
        Assert.Equal(dark, palette.IsDark);
        Assert.Same(dark ? Palette.Dark : Palette.Light, palette);
    }

    [Theory]
    [InlineData(null, "system")]
    [InlineData("system", "system")]
    [InlineData("Light", "light")]
    [InlineData("dark", "dark")]
    [InlineData("purple", "system")]
    public void ChoiceShownForTheSetting(string? setting, string choice)
    {
        Assert.Equal(choice, Themes.ChoiceOf(setting));
    }

    [Theory]
    [InlineData("system", null)]
    [InlineData("light", "light")]
    [InlineData("dark", "dark")]
    public void SettingStoredForTheChoice(string choice, string? setting)
    {
        Assert.Equal(setting, Themes.SettingFor(choice));
    }

    [Fact]
    public void EveryChoiceHasItsTextInEveryLanguage()
    {
        Assert.Equal(["system", "light", "dark"], Themes.Choices);
        foreach (var language in Texts.Languages)
        {
            Assert.Equal(3, Themes.Choices.Select(c => Themes.ChoiceText(c, language)).Distinct().Count());
        }
    }

    [Theory]
    [InlineData(false)]
    [InlineData(true)]
    public void EveryTextColourIsReadableOnItsBackground(bool dark)
    {
        var palette = dark ? Palette.Dark : Palette.Light;
        var pairs = palette.TextPairs().ToList();
        Assert.Contains(pairs, p => p.Name == "ButtonDisabledText");
        Assert.Contains(pairs, p => p.Name == "Muted");
        var poor = pairs.Where(p => Palette.Contrast(p.Fore, p.Back) < 4.5)
            .Select(p => $"{p.Name}: {Palette.Contrast(p.Fore, p.Back):0.00}").ToList();
        Assert.True(poor.Count == 0, "contrast below 4.5: " + string.Join(", ", poor));
    }

    [Fact]
    public void DarkIsDarkAndLightIsLight()
    {
        Assert.True(Palette.Dark.IsDark);
        Assert.False(Palette.Light.IsDark);
        Assert.True(Palette.Luminance(Palette.Dark.Window) < 0.05);
        Assert.True(Palette.Luminance(Palette.Light.Window) > 0.7);
    }

    [Fact]
    public void ContrastRatioIsWcags()
    {
        Assert.Equal(21, Palette.Contrast(Color.Black, Color.White), 2);
        Assert.Equal(21, Palette.Contrast(Color.White, Color.Black), 2);
        Assert.Equal(1, Palette.Contrast(Color.Gray, Color.Gray), 2);
        Assert.Equal(4.48, Palette.Contrast(Color.FromArgb(0x77, 0x77, 0x77), Color.White), 2);
    }

    /** A disabled button keeps its own readable colours (WinForms would grey its text on any background). */
    [Theory]
    [InlineData(false)]
    [InlineData(true)]
    public void ButtonColoursFollowItsState(bool dark)
    {
        var p = dark ? Palette.Dark : Palette.Light;
        Assert.Equal((p.ButtonBack, p.ButtonText, p.ButtonBorder), ThemedButton.Colors(p, enabled: true, hover: false, pressed: false, accent: false));
        Assert.Equal(p.ButtonHover, ThemedButton.Colors(p, true, hover: true, pressed: false, accent: false).Back);
        Assert.Equal(p.ButtonPressed, ThemedButton.Colors(p, true, hover: true, pressed: true, accent: false).Back);
        Assert.Equal((p.AccentBack, p.AccentText), Pick(ThemedButton.Colors(p, true, false, false, accent: true)));
        Assert.Equal(p.AccentHover, ThemedButton.Colors(p, true, hover: true, pressed: false, accent: true).Back);
        var disabled = ThemedButton.Colors(p, enabled: false, hover: true, pressed: true, accent: true);
        Assert.Equal((p.ButtonDisabledBack, p.ButtonDisabledText), Pick(disabled));
    }

    private static (Color, Color) Pick((Color Back, Color Fore, Color Border) colors) => (colors.Back, colors.Fore);
}
