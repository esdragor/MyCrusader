using System.Collections.Concurrent;
using System.Text.RegularExpressions;
using HOLUpdate;

namespace HOLManager.Versions;

/** A request on the versions refused with a PROTOCOL.md error code (section Versions du jeu). */
public sealed class VersionException(int status, string error, string message) : Exception(message)
{
    public int Status { get; } = status;
    public string Error { get; } = error;
}

/** What /versions/latest tells the launcher (L2). */
public sealed record LatestVersion(string Version, VersionManifest Manifest, string Notes, IReadOnlyList<string> PatchesFrom,
    string LauncherVersion);

/** The launcher exe to serve, its version and SHA-256 (L2, L7). */
public sealed record LauncherFile(string Version, string Path, string Sha256);

/**
 * Reads the versions folder written by the publication script (L4), laid out as in LauncherInProgress.md: latest.txt,
 * <v>/manifest.json, <v>/notes.txt, <v>/files/<path>.gz, <v>/patches/<from>/<path>.patch, launcher/latest.txt and
 * launcher/<lv>/HOLLauncher.exe. Nothing is cached but the launcher's hash: a version can be published while running.
 * A file is served only if its path is an entry of the version's list, and only if it resolves under the version's folder.
 */
public sealed partial class VersionStore(string root)
{
    public const string LauncherExe = "HOLLauncher.exe";

    private readonly ConcurrentDictionary<(string Path, long Size, DateTime Written), string> launcherHashes = new();

    public string Root { get; } = root;

    [GeneratedRegex("^[A-Za-z0-9._-]+$")]
    private static partial Regex NamePattern();

    /** Version names: [A-Za-z0-9._-]+, but not "." nor "..". */
    public static bool IsValidName(string? name) =>
        name != null && NamePattern().IsMatch(name) && name.Trim('.').Length > 0;

    /** D481: the name in latest.txt if that version's folder is there, else null (nothing published). */
    public string? LatestName() =>
        ReadName(Path.Combine(Root, "latest.txt")) is { } version && Directory.Exists(Path.Combine(Root, version)) ? version : null;

    public LatestVersion Latest()
    {
        var version = ReadName(Path.Combine(Root, "latest.txt"));
        if (version == null || !Directory.Exists(Path.Combine(Root, version)))
        {
            throw new VersionException(404, "no_version", $"No version published in {Root}.");
        }
        var dir = Path.Combine(Root, version);
        var manifest = LoadManifest(version);
        var notesFile = Path.Combine(dir, "notes.txt");
        var notes = File.Exists(notesFile) ? File.ReadAllText(notesFile).Trim() : "";
        var patchesDir = Path.Combine(dir, "patches");
        var patchesFrom = Directory.Exists(patchesDir)
            ? Directory.GetDirectories(patchesDir).Select(d => Path.GetFileName(d)).Where(IsValidName)
                .Order(StringComparer.Ordinal).ToList()
            : [];
        var launcher = ReadName(Path.Combine(Root, "launcher", "latest.txt")) ?? "";
        return new LatestVersion(version, manifest, notes, patchesFrom, launcher);
    }

    /** <v>/files/<path>.gz, the file compressed at publication. */
    public string FilePath(string version, string path)
    {
        var dir = VersionDir(version);
        var file = Entry(version, path, dir);
        return Under(dir, Path.Combine(dir, "files", file.Path + ".gz"), "file_not_found", path);
    }

    /** <v>/patches/<from>/<path>.patch; 404 when there is none (the launcher then takes the full file). */
    public string PatchPath(string version, string from, string path)
    {
        CheckName(from);
        var dir = VersionDir(version);
        var file = Entry(version, path, dir);
        var patch = Path.Combine(dir, "patches", from, file.Path + ".patch");
        if (!File.Exists(patch))
        {
            throw new VersionException(404, "patch_not_found", $"No patch of '{path}' from {from} to {version}.");
        }
        return Under(dir, patch, "patch_not_found", path);
    }

    public LauncherFile Launcher()
    {
        var version = ReadName(Path.Combine(Root, "launcher", "latest.txt"));
        var exe = version == null ? null : Path.Combine(Root, "launcher", version, LauncherExe);
        if (exe == null || !File.Exists(exe))
        {
            throw new VersionException(404, "launcher_not_found", $"No launcher published in {Root}.");
        }
        var info = new FileInfo(exe);
        var hash = launcherHashes.GetOrAdd((exe, info.Length, info.LastWriteTimeUtc), key => ManifestBuilder.Hash(key.Path));
        return new LauncherFile(version!, exe, hash);
    }

    private static void CheckName(string name)
    {
        if (!IsValidName(name))
        {
            throw new VersionException(400, "invalid_version", $"Invalid version name '{name}' (letters, digits, '.', '_', '-').");
        }
    }

    private string VersionDir(string version)
    {
        CheckName(version);
        var dir = Path.Combine(Root, version);
        if (!File.Exists(Path.Combine(dir, "manifest.json")))
        {
            throw new VersionException(404, "version_not_found", $"Version {version} is not published.");
        }
        return dir;
    }

    private VersionManifest LoadManifest(string version)
    {
        var file = Path.Combine(Root, version, "manifest.json");
        try
        {
            return VersionManifest.Load(file);
        }
        catch (Exception e) when (e is InvalidDataException or IOException)
        {
            throw new VersionException(500, "bad_manifest", $"List of version {version} unreadable ({file}): {e.Message}");
        }
    }

    /** The list's entry of that exact path (ordinal: the launcher asks the paths of the list). */
    private FileEntry Entry(string version, string path, string dir) =>
        LoadManifest(version).Files.FirstOrDefault(f => f.Path == path)
        ?? throw new VersionException(404, "file_not_found", $"'{path}' is not a file of version {version}.");

    /** Second guard after the list's own checks: the resolved file stays under the version's folder and exists. */
    private static string Under(string dir, string file, string error, string path)
    {
        var full = Path.GetFullPath(file);
        var prefix = Path.GetFullPath(dir) + Path.DirectorySeparatorChar;
        if (!full.StartsWith(prefix, StringComparison.OrdinalIgnoreCase) || !File.Exists(full))
        {
            throw new VersionException(404, error, $"'{path}' is not served.");
        }
        return full;
    }

    /** First line of a name file (latest.txt), null if missing, empty or not a valid name. */
    private static string? ReadName(string file)
    {
        if (!File.Exists(file))
        {
            return null;
        }
        var name = File.ReadAllText(file).Trim();
        return IsValidName(name) ? name : null;
    }
}
