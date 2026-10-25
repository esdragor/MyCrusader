using System.Text.Json;

namespace HOLLauncher;

/** One lobby manager of servers.json (M6-Q5): its address, its name (may be empty). */
public sealed record ManagerServer(string Name, string Url);

/**
 * Reads the game's servers.json like the game does (FHOLManagerClientRules::ParseServers, UHOLManagerClient): at the
 * game folder's root (FPaths::RootDir of a packaged game), { "servers": [ { "name", "url" } ] }, rows without url
 * skipped, the first one kept; no file or no row: the local manager. Unlike the game, an unreadable file is an error
 * (the launcher says so rather than guess).
 */
public static class GameServers
{
    public const string FileName = "servers.json";
    public const string DefaultUrl = "http://127.0.0.1:7700";
    public const string DefaultName = "Local";

    /** The first manager of <gameDir>/servers.json; InvalidDataException if the file cannot be read. */
    public static ManagerServer ReadFirst(string gameDir)
    {
        var file = Path.Combine(gameDir, FileName);
        if (!File.Exists(file))
        {
            return new ManagerServer(DefaultName, DefaultUrl);
        }
        try
        {
            using var document = JsonDocument.Parse(File.ReadAllText(file));
            if (document.RootElement.ValueKind != JsonValueKind.Object
                || !document.RootElement.TryGetProperty("servers", out var rows) || rows.ValueKind != JsonValueKind.Array)
            {
                throw new InvalidDataException($"{file}: no \"servers\" list");
            }
            foreach (var row in rows.EnumerateArray())
            {
                if (row.ValueKind == JsonValueKind.Object && ReadString(row, "url") is { Length: > 0 } url)
                {
                    return new ManagerServer(ReadString(row, "name") ?? "", url);
                }
            }
            return new ManagerServer(DefaultName, DefaultUrl);
        }
        catch (Exception e) when (e is JsonException or IOException or UnauthorizedAccessException)
        {
            throw new InvalidDataException($"{file} unreadable: {e.Message}", e);
        }
    }

    private static string? ReadString(JsonElement row, string name)
        => row.TryGetProperty(name, out var value) && value.ValueKind == JsonValueKind.String ? value.GetString() : null;
}
