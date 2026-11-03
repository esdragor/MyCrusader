using System.Text.RegularExpressions;

namespace HOLUpdate;

/** L14: how a version name is shown. A numbered one "0.Y.X" becomes "V 0.Y.X"; the others (old dated names, "dev") stay. */
public static partial class VersionName
{
    public static string Display(string name) => Numbered().IsMatch(name) ? $"V {name}" : name;

    [GeneratedRegex(@"^0\.\d+\.\d+$")]
    private static partial Regex Numbered();
}
