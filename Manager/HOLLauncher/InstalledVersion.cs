using HOLUpdate;

namespace HOLLauncher;

/**
 * The installed game's version: the text of <gameDir>/Version.txt (ManifestBuilder.VersionFile), trimmed; null if not
 * installed (no file, or an empty one).
 */
public static class InstalledVersion
{
    public static string? Read(string gameDir)
    {
        var file = Path.Combine(gameDir, ManifestBuilder.VersionFile);
        if (!File.Exists(file))
        {
            return null;
        }
        var version = File.ReadAllText(file).Trim();
        return version == "" ? null : version;
    }
}
