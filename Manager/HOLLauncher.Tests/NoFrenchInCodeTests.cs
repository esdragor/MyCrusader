using System.Text;
using System.Text.RegularExpressions;

namespace HOLLauncher.Tests;

/**
 * L16: every text the player sees comes from Texts (Strings*.resx), so the launcher's code holds no French (nor
 * Japanese) text of its own. The sources of Manager/HOLLauncher are read, comments left out: no letter beyond ASCII
 * (accents, Japanese) and none of the usual French words of the launcher's messages. Allowed: the languages named in
 * their own language (Texts.LanguageName, shown as such in every language).
 */
public class NoFrenchInCodeTests
{
    /** Exact literals allowed in code, with why. */
    private static readonly string[] Allowed =
    [
        "\"Français\"", // Texts.LanguageName: French named in French in every language.
        "\"日本語\"", // Texts.LanguageName: Japanese named in Japanese in every language.
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

    /** Manager/HOLLauncher, found from the test assembly's folder upward (bin/<config>/<tfm> under the tests). */
    private static string LauncherSourceDir()
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
