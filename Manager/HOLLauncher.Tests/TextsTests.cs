using System.Globalization;
using System.Text.RegularExpressions;

namespace HOLLauncher.Tests;

public class TextsTests
{
    [Theory]
    [InlineData("en", "fr-FR", "en")]
    [InlineData("ja", "en-US", "ja")]
    [InlineData(null, "fr-FR", "fr")]
    [InlineData(null, "ja-JP", "ja")]
    [InlineData(null, "en-GB", "en")]
    [InlineData(null, "de-DE", "en")]
    [InlineData("xx", "fr-CA", "fr")]
    [InlineData("", "de-DE", "en")]
    public void LanguageIsTheSettingElseWindowsElseEnglish(string? setting, string system, string expected)
    {
        Assert.Equal(expected, Texts.Resolve(setting, CultureInfo.GetCultureInfo(system)));
    }

    [Fact]
    public void TextsFollowTheLanguage()
    {
        Assert.Equal("Jouer", Texts.Get("Play", "fr"));
        Assert.Equal("Play", Texts.Get("Play", "en"));
        Assert.Equal("プレイ", Texts.Get("Play", "ja"));
        Assert.Equal("Heroes of Liberty - launcher 7", Texts.Format("WindowTitle", "en", "7"));
    }

    [Fact]
    public void UnknownTextOrLanguageIsAnError()
    {
        Assert.Throws<KeyNotFoundException>(() => Texts.Get("NoSuchText", "fr"));
        Assert.Throws<ArgumentException>(() => Texts.Get("Play", "de"));
        Assert.Throws<ArgumentException>(() => Texts.Use("de"));
    }

    [Fact]
    public void LanguagesAreNamedInTheirOwnLanguage()
    {
        Assert.Equal(["fr", "en", "ja"], Texts.Languages);
        Assert.Equal("Français", Texts.LanguageName("fr"));
        Assert.Equal("English", Texts.LanguageName("en"));
        Assert.Equal("日本語", Texts.LanguageName("ja"));
    }

    /** Every French text has its English and Japanese one, not empty, with the same {n} slots. */
    [Fact]
    public void EveryTextExistsInEveryLanguage()
    {
        var french = Texts.All("fr");
        Assert.NotEmpty(french);
        foreach (var language in new[] { "en", "ja" })
        {
            var other = Texts.All(language);
            Assert.Equal(french.Keys.Order(), other.Keys.Order());
            foreach (var (name, text) in french)
            {
                Assert.False(string.IsNullOrWhiteSpace(other[name]), $"{language}: {name} empty");
                Assert.Equal(Slots(text), Slots(other[name]));
            }
        }
    }

    private static string[] Slots(string text) => Regex.Matches(text, @"\{\d+\}").Select(m => m.Value).Order().ToArray();
}
