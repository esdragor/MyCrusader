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

    /** Launcher 5: every brush the XAML names (DynamicResource ...Brush) is made from the palette, in both themes. */
    [Fact]
    public void EveryBrushTheWindowsUseComesFromThePalette()
    {
        var keys = Directory.GetFiles(NoFrenchInCodeTests.LauncherSourceDir(), "*.xaml")
            .SelectMany(f => System.Text.RegularExpressions.Regex.Matches(File.ReadAllText(f), @"DynamicResource (\w+Brush)\}"))
            .Select(m => m.Groups[1].Value)
            .Concat(Directory.GetFiles(NoFrenchInCodeTests.LauncherSourceDir(), "*.cs")
                .SelectMany(f => System.Text.RegularExpressions.Regex.Matches(File.ReadAllText(f), @"""(\w+Brush)""")).Select(m => m.Groups[1].Value))
            .ToHashSet();
        Assert.Contains("PanelBrush", keys);
        Assert.Contains("CardBrush", keys);
        foreach (var palette in new[] { Palette.Light, Palette.Dark })
        {
            var entries = WpfTheme.Entries(palette).Select(e => e.Key).ToHashSet();
            Assert.Empty(keys.Except(entries));
        }
        Assert.Contains(WpfTheme.Entries(Palette.Dark), e => e.Key == "PanelBrush" && e.Color == Palette.Dark.Surface && e.Opacity < 1);
        Assert.Contains(WpfTheme.Entries(Palette.Light), e => e.Key == "TextBrush" && e.Color == Palette.Light.Text);
    }

    /** The panel over the key art is tested against the worst art behind it (black under light, white under dark). */
    [Fact]
    public void PanelIsTestedOverTheWorstArt()
    {
        Assert.Equal(Color.FromArgb(0, 0, 0), Palette.Blend(Color.White, Color.Black, 0));
        Assert.Equal(Color.FromArgb(128, 128, 128), Palette.Blend(Color.White, Color.Black, 0.5));
        Assert.True(Palette.Luminance(Palette.Light.PanelWorst) < Palette.Luminance(Palette.Light.Surface));
        Assert.True(Palette.Luminance(Palette.Dark.PanelWorst) > Palette.Luminance(Palette.Dark.Surface));
        Assert.Contains(Palette.Dark.TextPairs(), p => p.Name == "TextOnPanel" && p.Back == Palette.Dark.PanelWorst);
    }
}
