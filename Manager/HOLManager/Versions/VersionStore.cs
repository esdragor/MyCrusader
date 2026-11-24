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

/**
 * What /versions/latest tells the launcher (L2). History (L21): the notes of every kept version, newest first from this
 * one down, so that a launcher several versions behind shows what changed since its own.
 */
public sealed record LatestVersion(string Version, VersionManifest Manifest, string Notes, IReadOnlyList<string> PatchesFrom,
    string LauncherVersion, IReadOnlyList<VersionNotes> History);

/** One kept version's notes, as listed in the history of /versions/latest (L21). */
public sealed record VersionNotes(string Version, DateTime Published, string Notes);

/** The launcher exe to serve, its version and SHA-256 (L2, L7). */
public sealed record LauncherFile(string Version, string Path, string Sha256);

/**
 * Reads the versions folder written by the publication script (L4), laid out as in LauncherInProgress.md: latest.txt,
 * <v>/manifest.json, <v>/notes.txt, <v>/published.txt (order of the notes history, L21), <v>/files/<path>.gz,
 * <v>/patches/<from>/<path>.patch, launcher/latest.txt and launcher/<lv>/HOLLauncher.exe. Nothing is cached but the
 * launcher's hash: a version can be published while running.
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

    /**
     * language (launcher 3, ?lang=): the notes in that language when the version has <v>/notes.<language>.txt, else
     * notes.txt (French, the language they are written in). Only two lowercase letters are looked for (no path).
     */
    public LatestVersion Latest(string? language = null)
    {
        var version = ReadName(Path.Combine(Root, "latest.txt"));
        if (version == null || !Directory.Exists(Path.Combine(Root, version)))
        {
            throw new VersionException(404, "no_version", $"No version published in {Root}.");
        }
        var dir = Path.Combine(Root, version);
        var manifest = LoadManifest(version);
        var notes = ReadNotes(dir, language);
        var patchesDir = Path.Combine(dir, "patches");
        var patchesFrom = Directory.Exists(patchesDir)
            ? Directory.GetDirectories(patchesDir).Select(d => Path.GetFileName(d)).Where(IsValidName)
                .Order(StringComparer.Ordinal).ToList()
            : [];
        var launcher = ReadName(Path.Combine(Root, "launcher", "latest.txt")) ?? "";
        return new LatestVersion(version, manifest, notes, patchesFrom, launcher, History(version, notes, language));
    }

    /**
     * L21: the kept versions in publication order (HOLUpdate.PublishedVersions, as the publisher prunes them), newest
     * first from latest down: a version published after it (latest.txt set back by hand) is not news for anyone. A
     * version pruned while being read is left out rather than failing the launcher's whole answer.
     */
    private IReadOnlyList<VersionNotes> History(string latest, string latestNotes, string? language)
    {
        var kept = PublishedVersions.NewestFirst(Root,
            name => IsValidName(name) && !name.EndsWith(".tmp", StringComparison.OrdinalIgnoreCase)).ToList();
        var from = kept.IndexOf(latest);
        if (from < 0)
        {
            return [new VersionNotes(latest, PublishedVersions.Date(Path.Combine(Root, latest)), latestNotes)];
        }
        var history = new List<VersionNotes>();
        foreach (var version in kept.Skip(from))
        {
            var dir = Path.Combine(Root, version);
            try
            {
                history.Add(new VersionNotes(version, PublishedVersions.Date(dir), version == latest ? latestNotes : ReadNotes(dir, language)));
            }
            catch (Exception e) when (e is IOException or UnauthorizedAccessException)
            {
            }
        }
        return history;
    }

    /** <v>/notes.<language>.txt if there, else <v>/notes.txt, trimmed; empty without either. */
    private static string ReadNotes(string dir, string? language = null)
    {
        if (IsLanguage(language) && Path.Combine(dir, $"notes.{language}.txt") is var translated && File.Exists(translated))
        {
            return File.ReadAllText(translated).Trim();
        }
        var file = Path.Combine(dir, "notes.txt");
        return File.Exists(file) ? File.ReadAllText(file).Trim() : "";
    }

    /** "en", "ja"...: two lowercase ASCII letters, so that a ?lang= never names another file. */
    public static bool IsLanguage(string? language) => language is { Length: 2 } && language.All(c => c is >= 'a' and <= 'z');

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
