using System.Collections;
using System.Globalization;
using System.Resources;

namespace HOLLauncher;

/**
 * The launcher's texts in French, English and Japanese (L15): Strings.resx (French, the neutral language),
 * Strings.en.resx, Strings.ja.resx, compiled into satellite assemblies (bundled in the single-file exe). Language is the
 * one in use, set once at start and again when the settings change (Use). Every text exists in the three languages with
 * the same {n} slots (TextsTests). A new text is added to the three files at once.
 */
public static class Texts
{
    public static readonly IReadOnlyList<string> Languages = ["fr", "en", "ja"];

    private static readonly ResourceManager Resources = new("HOLLauncher.Strings", typeof(Texts).Assembly);

    public static string Language { get; private set; } = "fr";

    public static void Use(string language) => Language = Checked(language);

    /** The setting if it is a language of the launcher, else Windows' language if it is one, else English. */
    public static string Resolve(string? setting, CultureInfo system)
    {
        if (setting is not null && Languages.Contains(setting))
        {
            return setting;
        }
        var windows = system.TwoLetterISOLanguageName;
        return Languages.Contains(windows) ? windows : "en";
    }

    public static string Get(string name) => Get(name, Language);

    public static string Get(string name, string language)
        => Resources.GetString(name, Culture(language)) ?? throw new KeyNotFoundException($"no launcher text '{name}'");

    public static string Format(string name, params object[] args) => FormatIn(Language, name, args);

    /**
     * Named apart from Format on purpose: a Format(name, language, params args) overload would take a single string
     * argument for the language (the launcher crashed at start with Format("WindowTitle", "1")).
     */
    public static string FormatIn(string language, string name, params object[] args)
        => string.Format(Culture(language), Get(name, language), args);

    /**
     * The given language (checked), else the one in use (L16). The views and messages take an optional language so that
     * tests name theirs instead of depending on the global Language, which another test may be changing in parallel.
     */
    public static string LanguageOr(string? language) => language is null ? Language : Checked(language);

    /** A number in the language's own format (L16): 12,4 in French, 12.4 in English and Japanese. */
    public static string Number(double value, string format, string language) => value.ToString(format, Culture(language));

    /** Each language in its own language, as the settings list shows it. */
    public static string LanguageName(string language) => Checked(language) switch
    {
        "fr" => "Français",
        "en" => "English",
        _ => "日本語",
    };

    /** Segoe UI has no Japanese letters of its own: Yu Gothic UI is Windows' Japanese interface font. */
    public static string FontFamily(string language) => language == "ja" ? "Yu Gothic UI" : "Segoe UI";

    /** Every text of one language, read from its own file only (no fallback to French): tests. */
    public static IReadOnlyDictionary<string, string> All(string language)
    {
        var set = Resources.GetResourceSet(Culture(language), createIfNotExists: true, tryParents: false)
            ?? throw new MissingSatelliteAssemblyException($"no launcher texts for '{language}'");
        return set.Cast<DictionaryEntry>().ToDictionary(e => (string)e.Key, e => (string)e.Value!);
    }

    private static CultureInfo Culture(string language) => CultureInfo.GetCultureInfo(Checked(language));

    private static string Checked(string language) => Languages.Contains(language)
        ? language
        : throw new ArgumentException($"unknown launcher language '{language}' (fr, en, ja)");
}
