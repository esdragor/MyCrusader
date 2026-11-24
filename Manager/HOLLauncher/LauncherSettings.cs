using System.Text.Json;
using System.Text.Json.Serialization;

namespace HOLLauncher;

/** LauncherSettings.Load: the settings, and why the file was set aside if it was unreadable (null: read or absent). */
public sealed record SettingsLoad(LauncherSettings Settings, string? Problem);

/**
 * The player's launcher settings (L15), HOLLauncher.settings.json beside the launcher's exe, camelCase JSON. A missing
 * file or field gives the default; an unknown field is ignored (a later launcher may add some); an unreadable file
 * gives the defaults and says so (Problem), and the next Save replaces it. A game update never lists, replaces nor
 * deletes it (UpdatePlan.LauncherOwned).
 * Language: "fr", "en" or "ja" (Texts.Resolve; null: Windows' language). GameDir: the game folder (null: the launcher's
 * folder; --game-dir wins over it, LauncherOptions).
 * Later steps add their own fields (L19 launch options, L23 automatic update): with "with", the window keeps them.
 */
public sealed record LauncherSettings(string? Language = null, string? GameDir = null)
{
    public const string FileName = "HOLLauncher.settings.json";

    public static readonly LauncherSettings Default = new();

    private static readonly JsonSerializerOptions Json = new()
    {
        PropertyNamingPolicy = JsonNamingPolicy.CamelCase,
        PropertyNameCaseInsensitive = true,
        DefaultIgnoreCondition = JsonIgnoreCondition.WhenWritingNull,
        WriteIndented = true,
    };

    public static SettingsLoad Load(string launcherDir)
    {
        var file = Path.Combine(launcherDir, FileName);
        if (!File.Exists(file))
        {
            return new SettingsLoad(Default, null);
        }
        try
        {
            using var document = JsonDocument.Parse(File.ReadAllText(file));
            if (document.RootElement.ValueKind != JsonValueKind.Object)
            {
                return new SettingsLoad(Default, $"{file}: not a JSON object");
            }
            return new SettingsLoad(document.RootElement.Deserialize<LauncherSettings>(Json) ?? Default, null);
        }
        catch (Exception e) when (e is JsonException or IOException or UnauthorizedAccessException)
        {
            return new SettingsLoad(Default, $"{file}: {e.Message}");
        }
    }

    /** Written beside, then moved over the old file (never half written). IOException / UnauthorizedAccessException on failure. */
    public void Save(string launcherDir)
    {
        var file = Path.Combine(launcherDir, FileName);
        var part = file + ".part";
        File.WriteAllText(part, JsonSerializer.Serialize(this, Json));
        File.Move(part, file, overwrite: true);
    }

    /**
     * The GameDir to store for what the player typed: blank or the launcher's folder = null (the default), else the full
     * path without its trailing separator. A path that is not fully qualified (relative, "D:HOL") is refused
     * (ArgumentException): it would depend on the folder the launcher is started from.
     */
    public static string? GameDirFor(string text, string launcherDir)
    {
        var trimmed = text.Trim();
        if (trimmed == "")
        {
            return null;
        }
        if (!Path.IsPathFullyQualified(trimmed))
        {
            throw new ArgumentException($"'{trimmed}' is not a full path (example: D:\\Games\\Heroes of Liberty)");
        }
        var full = Path.TrimEndingDirectorySeparator(Path.GetFullPath(trimmed));
        return GameFolder.Same(full, launcherDir) ? null : full;
    }
}
