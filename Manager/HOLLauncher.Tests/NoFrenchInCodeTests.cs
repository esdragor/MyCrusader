using System.Text;
using System.Text.RegularExpressions;

namespace HOLLauncher.Tests;

/**
 * L16: every text the player sees comes from Texts (Strings*.resx), so the launcher's code holds no French (nor
 * Japanese) text of its own. The sources of Manager/HOLLauncher are read, comments left out: no letter beyond ASCII
 * (accents, Japanese) and none of the usual French words of the launcher's messages. Allowed: the languages named in
 * their own language (Texts.LanguageName, shown as such in every language).
 * Launcher 5: the windows' XAML too (comments left out): no letter beyond ASCII, no literal player text in a Text,
 * Content, Header, ToolTip or Title attribute (they bind with {local:Tr Name}) nor between tags (a Geometry's path
 * data excepted).
 */
public class NoFrenchInCodeTests
{
    /** Exact literals allowed in code, with why. */
    private static readonly string[] Allowed =
    [
        "\"Français\"", // Texts.LanguageName: French named in French in every language.
        "\"日本語\"", // Texts.LanguageName: Japanese named in Japanese in every language.
        "\"×\"", // SettingsWindow (L19): the multiplication sign between width and height, the same in every language.
        "} × {", // SettingsModel (L19): a resolution "1920 × 1080", likewise.
        "\" · \"", // FooterLine (launcher 5): the separator of the line under the big button, the same in every language.
    ];

    /** French words of the old messages that are plain ASCII ("Serveur injoignable", "Lanceur : version"...). */
    private static readonly Regex FrenchWords = new(
        @"\b(jeu|lanceur|erreur|fichier|serveur|injoignable|disponible|relancez|introuvable|dossier|disque|libres|inconnue|annulation|version\s+install)\b",
        RegexOptions.IgnoreCase);

    [Fact]
    public void NoFrenchTextLeftInCode()
    {
        var sources = Directory.GetFiles(LauncherSourceDir(), "*.cs");
        Assert.Contains(sources, f => Path.GetFileName(f) == "Texts.cs");
        var found = new List<string>();
        foreach (var file in sources)
        {
            var lines = WithoutComments(File.ReadAllText(file)).Split('\n');
            for (var i = 0; i < lines.Length; i++)
            {
                if (FrenchIn(lines[i]) is { } text)
                {
                    found.Add($"{Path.GetFileName(file)}:{i + 1}: {text}");
                }
            }
        }
        Assert.True(found.Count == 0, "French text in code (use Texts):\n" + string.Join("\n", found));
    }

    /** The scanner itself: comments are skipped, text in code is caught, the allowed literals are not. */
    [Fact]
    public void ScannerSeesTextButNotComments()
    {
        Assert.Null(FrenchIn(WithoutComments("var a = 1; // Le jeu est à jour")));
        Assert.Null(FrenchIn(WithoutComments("/** Lanceur\n déjà */ var b = \"http://x\";").Replace("\n", " ")));
        Assert.NotNull(FrenchIn("var c = \"Mise à jour\";"));
        Assert.NotNull(FrenchIn("var d = $\"Serveur injoignable : {e}\";"));
        Assert.NotNull(FrenchIn("var e = \"ランチャー\";"));
        Assert.Null(FrenchIn("\"fr\" => \"Français\","));
        Assert.Equal("var f = \"//not a comment\"; ", WithoutComments("var f = \"//not a comment\"; // a comment"));
        Assert.Equal("var g = '\"'; x", WithoutComments("var g = '\"'; /* \"é */x"));
    }

    /** The line if it holds French (or any non-ASCII letter) once the allowed literals are taken out, else null. */
    private static string? FrenchIn(string line)
    {
        var rest = Allowed.Aggregate(line, (text, allowed) => text.Replace(allowed, ""));
        return rest.Any(c => c > 127) || FrenchWords.IsMatch(rest) ? line.Trim() : null;
    }

    /**
     * The source without its comments (line and block comments), line breaks kept so that line numbers stay right. String and
     * char literals are skipped over so that "//" inside a text is not a comment; a quote inside an interpolation hole
     * only makes the scanner see a bit of text as code, which is scanned all the same.
     */
    private static string WithoutComments(string source)
    {
        var code = new StringBuilder(source.Length);
        var i = 0;
        while (i < source.Length)
        {
            var c = source[i];
            var next = i + 1 < source.Length ? source[i + 1] : '\0';
            if (c == '/' && next == '/')
            {
                while (i < source.Length && source[i] != '\n')
                {
                    i++;
                }
            }
            else if (c == '/' && next == '*')
            {
                var end = source.IndexOf("*/", i + 2, StringComparison.Ordinal);
                var stop = end < 0 ? source.Length : end + 2;
                code.Append('\n', source.AsSpan(i, stop - i).Count('\n'));
                i = stop;
            }
            else if (c is '"' or '\'')
            {
                var verbatim = c == '"' && i > 0 && (source[i - 1] == '@' || (source[i - 1] == '$' && i > 1 && source[i - 2] == '@'));
                var start = i++;
                while (i < source.Length && source[i] != c && (verbatim || source[i] != '\n'))
                {
                    i += !verbatim && source[i] == '\\' ? 2 : 1;
                }
                i = Math.Min(i + 1, source.Length);
                code.Append(source, start, i - start);
            }
            else
            {
                code.Append(c);
                i++;
            }
        }
        return code.ToString();
    }

    private static readonly Regex XamlTextAttribute = new(@"\b(Text|Content|Header|ToolTip|Title)=""([^""{][^""]*)""");
    private static readonly Regex XamlElementText = new(@"<(?!Geometry\b)([\w.:]+)[^<>]*>([^<]*[A-Za-z][^<]*)</\1>");

    [Fact]
    public void NoPlayerTextInXaml()
    {
        var files = Directory.GetFiles(LauncherSourceDir(), "*.xaml");
        Assert.Contains(files, f => Path.GetFileName(f) == "MainWindow.xaml");
        var found = new List<string>();
        foreach (var file in files)
        {
            foreach (var problem in XamlTextIn(File.ReadAllText(file)))
            {
                found.Add($"{Path.GetFileName(file)}: {problem}");
            }
        }
        Assert.True(found.Count == 0, "Player text in XAML (use {local:Tr Name}):\n" + string.Join("\n", found));
    }

    [Fact]
    public void XamlScannerSeesTextButNotCommentsBindingsOrPaths()
    {
        Assert.Empty(XamlTextIn("<!-- Jouer à la fête --><TextBlock Text=\"{local:Tr Play}\" />"));
        Assert.Empty(XamlTextIn("<Geometry x:Key=\"IconClose\">M 3,3 L 12,12</Geometry><TextBlock x:Name=\"A\" Text=\"\" />"));
        Assert.NotEmpty(XamlTextIn("<TextBlock Text=\"Play\" />"));
        Assert.NotEmpty(XamlTextIn("<Button ToolTip=\"Close\" />"));
        Assert.NotEmpty(XamlTextIn("<TextBlock>Jouer</TextBlock>"));
        Assert.NotEmpty(XamlTextIn("<Path Data=\"M 0,0\" Tag=\"é\" />"));
    }

    /** What looks like player text in a XAML file, comments left out. */
    private static IEnumerable<string> XamlTextIn(string xaml)
    {
        var code = Regex.Replace(xaml, "<!--.*?-->", "", RegexOptions.Singleline);
        foreach (Match m in XamlTextAttribute.Matches(code))
        {
            yield return m.Value;
        }
        foreach (Match m in XamlElementText.Matches(code))
        {
            yield return m.Value;
        }
        foreach (var line in code.Split('\n').Where(l => l.Any(c => c > 127)))
        {
            yield return line.Trim();
        }
    }

    /** Manager/HOLLauncher, found from the test assembly's folder upward (bin/<config>/<tfm> under the tests). */
    internal static string LauncherSourceDir()
    {
        for (var dir = new DirectoryInfo(AppContext.BaseDirectory); dir is not null; dir = dir.Parent)
        {
            var launcher = Path.Combine(dir.FullName, "HOLLauncher");
            if (File.Exists(Path.Combine(launcher, "HOLLauncher.csproj")))
            {
                return launcher;
            }
        }
        throw new DirectoryNotFoundException($"no HOLLauncher/HOLLauncher.csproj above {AppContext.BaseDirectory}");
    }
}
