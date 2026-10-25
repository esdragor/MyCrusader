using System.Text.Json;
using System.Text.Json.Serialization;

namespace HOLUpdate;

/**
 * The list of a version's files (L1): the text of Version.txt, the list format, and the entries sorted by path in
 * ordinal order so that the same folder always gives the same JSON. Read from the network in L6: FromJson rejects
 * paths that could leave the game folder and hashes that are not 64 lowercase hex digits.
 */
public sealed class VersionManifest
{
    public const int CurrentFormat = 1;

    private static readonly JsonSerializerOptions JsonOptions = new(JsonSerializerDefaults.Web) { WriteIndented = true };

    public VersionManifest(string version, IEnumerable<FileEntry> files, int format = CurrentFormat)
    {
        Version = version;
        Format = format;
        Files = files.OrderBy(f => f.Path, StringComparer.Ordinal).ToList();
    }

    /** Nothing installed yet (first install): everything is to download. */
    public static VersionManifest Empty { get; } = new("", []);

    public string Version { get; }
    public int Format { get; }
    public IReadOnlyList<FileEntry> Files { get; }

    public string ToJson() => JsonSerializer.Serialize(new Dto(Format, Version, Files.ToList()), JsonOptions);

    public static VersionManifest FromJson(string json)
    {
        Dto? dto;
        try
        {
            dto = JsonSerializer.Deserialize<Dto>(json, JsonOptions);
        }
        catch (JsonException e)
        {
            throw new InvalidDataException($"Unreadable version list: {e.Message}", e);
        }
        if (dto is null || dto.Version is null || dto.Files is null)
        {
            throw new InvalidDataException("Version list without version or files.");
        }
        if (dto.Format < 1 || dto.Format > CurrentFormat)
        {
            throw new InvalidDataException($"Version list format {dto.Format} unknown (this program reads up to {CurrentFormat}).");
        }
        foreach (var file in dto.Files)
        {
            Validate(file);
        }
        return new VersionManifest(dto.Version, dto.Files, dto.Format);
    }

    public void Save(string file) => File.WriteAllText(file, ToJson());

    public static VersionManifest Load(string file) => FromJson(File.ReadAllText(file));

    private static void Validate(FileEntry? file)
    {
        if (file is null || string.IsNullOrEmpty(file.Path) || file.Path.StartsWith('/') || file.Path.Contains('\\')
            || file.Path.Contains(':') || file.Path.Split('/').Any(part => part is "" or "." or ".."))
        {
            throw new InvalidDataException($"Unsafe path in version list: '{file?.Path}'.");
        }
        if (file.Size < 0)
        {
            throw new InvalidDataException($"Negative size for '{file.Path}'.");
        }
        if (file.Sha256 is not { Length: 64 } || !file.Sha256.All(c => c is >= '0' and <= '9' or >= 'a' and <= 'f'))
        {
            throw new InvalidDataException($"Bad SHA-256 for '{file.Path}': '{file.Sha256}'.");
        }
    }

    private sealed record Dto(
        [property: JsonPropertyOrder(0)] int Format,
        [property: JsonPropertyOrder(1)] string? Version,
        [property: JsonPropertyOrder(2)] List<FileEntry>? Files);
}
