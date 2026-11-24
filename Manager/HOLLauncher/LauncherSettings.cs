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
 * Launch options (L19, LaunchArguments): DisplayMode "windowed" or "fullscreen" (null: the game's own),
 * ResolutionWidth / ResolutionHeight in pixels (null: the game's own), ExtraArguments the player's free extra arguments
 * (advanced, null: none). An L15 file has none of them: the game starts without argument, as before.
 * LastCrashCheckUtc (L20, CrashDetector): when the launcher last looked for a crash of the game, UTC (null: never, the
 * first start only stores the time). Older files have none.
 * Theme (L22, Themes): "light" or "dark" (null: follow Windows' app theme). Older files have none: they follow Windows.
 * CloseOnPlay (launcher 5, GameSession): true closes the launcher once the game started (L8's behaviour); null or
 * false (the default) keeps it open, minimized, watching the game. Older files have none: they stay open.
 * Background (L23, written only when not the default; older files have none and keep launcher 5's behaviour):
 * MinimizeToTray true: the window's close button hides the launcher in the notification area (TrayRules; null: false,
 * closing quits). AutoUpdate "off", "notify" or "install" (BackgroundUpdatePolicy; null or unknown: "notify").
 * CheckEveryMinutes: how often the background check reads /versions/latest (null: 30; bounded 5..1440). StartWithWindows
 * true: the launcher starts hidden in the notification area at logon (StartWithWindows, the HKCU Run key; null: false).
 * TrayHintShown: the one-time "the launcher went to the notification area" message was shown (null: not yet).
 * Later steps add their own fields: with "with", the window keeps them.
 */
public sealed record LauncherSettings(
    string? Language = null,
    string? GameDir = null,
    string? DisplayMode = null,
    int? ResolutionWidth = null,
    int? ResolutionHeight = null,
    string? ExtraArguments = null,
    DateTime? LastCrashCheckUtc = null,
    string? Theme = null,
    bool? CloseOnPlay = null,
    bool? MinimizeToTray = null,
    string? AutoUpdate = null,
    int? CheckEveryMinutes = null,
    bool? StartWithWindows = null,
    bool? TrayHintShown = null)
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
        File.WriteAllText(part, ToJson());
        File.Move(part, file, overwrite: true);
    }

    /** The file's text (also shown in the report, ReportBuilder, L20). */
    public string ToJson() => JsonSerializer.Serialize(this, Json);

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
