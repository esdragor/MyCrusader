using System.IO.Enumeration;
using HOLUpdate;

namespace HOLLauncher;

public enum DownloadKind
{
    /** GET /versions/{v}/files/{path}: the gzipped file, unpacked and checked against the list. */
    Full,
    /** GET /versions/{v}/patches/{installed}/{path}: an HDiffPatch patch, applied by PatchApplier (L6b). */
    Patch,
}

/**
 * One file to fetch: its entry in the target list, how, the route relative to the manager's address, and the route of
 * the full file (the same for Full; for Patch, the fallback when the patch is missing or does not apply).
 */
public sealed record PlannedDownload(FileEntry File, DownloadKind Kind, string RelativeUrl, string FullUrl);

/**
 * What an update does (L6a, L6b), pure: the installed list (ScanGame) against the latest version. A changed file comes
 * as a patch when the manager keeps patches from the installed version (PatchesFrom), else in full; an added file
 * always in full. FullBytes is the final size of the full files only: a patch's size is unknown before its answer, so
 * progress counts files and received bytes. Only files this launcher installed (the previous installed.json) are ever
 * deleted, and the launcher's own files (LauncherOwned) are left out of everything.
 */
public sealed class UpdatePlan
{
    /** The launcher's work folder inside the game folder (FileDownloader.TempDirFor): never part of the list. */
    public const string WorkDirName = ".holupdate";

    /** <game>/.holupdate/installed.json: the list this launcher installed last (VersionManifest), written at the end of a successful update. */
    public const string InstalledListName = "installed.json";

    /** The launcher itself, any name it is given (HOLLauncher.exe, HOLLauncher-2.exe...): replaced by L7 only. */
    public const string LauncherPattern = "HOLLauncher*.exe";

    /** The launcher's own download (LauncherUpdater: HOLLauncher.new.exe.part), not matched by LauncherPattern. */
    public const string LauncherPartPattern = LauncherPattern + LauncherUpdater.PartSuffix;

    /** HDiffPatch's patcher, shipped beside the launcher (L-Q2 bis): the launcher's, not the game's. */
    public const string HpatchzName = "hpatchz.exe";

    /**
     * The player's managers (GameServers.FileName): a local one is never overwritten nor deleted (the published package
     * may carry one aimed at 127.0.0.1); a missing one is installed from the published version.
     */
    public const string ServersName = GameServers.FileName;

    /**
     * Files of the launcher or the player, matched on the file name like ManifestBuilder's exclusions: never in the
     * game's list nor in installed.json, never replaced nor deleted; only a missing servers.json is fetched.
     */
    public static readonly IReadOnlyList<string> LauncherOwned = [LauncherPattern, LauncherPartPattern, HpatchzName, ServersName];

    /** ManifestBuilder counts every file under the game folder: the launcher also leaves out its work folder and its own files. */
    public static readonly IReadOnlyList<string> Excludes =
        [.. ManifestBuilder.DefaultExcludes, WorkDirName + "/*", .. LauncherOwned];

    private UpdatePlan(string version, string? installed, List<PlannedDownload> downloads, IReadOnlyList<FileEntry> toDelete,
        VersionManifest installedList)
    {
        Version = version;
        Installed = installed;
        Downloads = downloads;
        ToDelete = toDelete;
        InstalledList = installedList;
        FullBytes = downloads.Where(d => d.Kind == DownloadKind.Full).Sum(d => d.File.Size);
        PatchCount = downloads.Count(d => d.Kind == DownloadKind.Patch);
    }

    public string Version { get; }

    /** null on a first install. */
    public string? Installed { get; }

    public IReadOnlyList<PlannedDownload> Downloads { get; }

    /** Only files of the previous installed.json that the new version no longer has: unknown local files are kept. */
    public IReadOnlyList<FileEntry> ToDelete { get; }

    /** What installed.json says once this update is done: the target list minus the launcher's own files. */
    public VersionManifest InstalledList { get; }

    public long FullBytes { get; }
    public int PatchCount { get; }

    /**
     * previousList: the last installed.json (null: game installed from a zip, nothing is deleted). localServers: a
     * servers.json exists in the game folder (it is then left alone).
     */
    public static UpdatePlan Create(VersionManifest installed, LatestVersion latest, VersionManifest? previousList = null,
        bool localServers = false)
    {
        var target = new VersionManifest(latest.Version, latest.Manifest.Files.Where(f => !IsLauncherOwned(f.Path)));
        var diff = ManifestDiff.Compare(installed, target);
        var from = installed.Version == "" ? null : installed.Version;
        var patchable = from is not null && latest.PatchesFrom.Contains(from);
        var changed = diff.Changed.ToHashSet();
        var downloads = diff.ToDownload
            .Select(file => patchable && changed.Contains(file)
                ? new PlannedDownload(file, DownloadKind.Patch, PatchUrl(latest.Version, from!, file.Path), FileUrl(latest.Version, file.Path))
                : Full(latest.Version, file))
            .ToList();
        var servers = latest.Manifest.Files.FirstOrDefault(f => f.Path.Equals(ServersName, StringComparison.OrdinalIgnoreCase));
        if (servers is not null && !localServers)
        {
            downloads.Add(Full(latest.Version, servers));
        }
        var listed = new HashSet<string>((previousList?.Files ?? []).Select(f => f.Path), StringComparer.OrdinalIgnoreCase);
        var toDelete = diff.ToDelete.Where(f => listed.Contains(f.Path)).ToList();
        return new UpdatePlan(latest.Version, from, downloads, toDelete, target);
    }

    /** The plan for a game folder: its scan, its installed.json, its servers.json. */
    public static UpdatePlan ForGame(string gameDir, LatestVersion latest)
        => Create(ScanGame(gameDir), latest, ReadInstalledList(gameDir), File.Exists(Path.Combine(gameDir, ServersName)));

    /** The installed list (hashes every file: slow on the 332 MB exe); Empty when GameFiles.VersionFile is missing or empty. */
    public static VersionManifest ScanGame(string gameDir)
    {
        var version = InstalledVersion.Read(gameDir);
        return version is null ? VersionManifest.Empty : ManifestBuilder.Build(gameDir, version, Excludes);
    }

    public static string InstalledListPath(string gameDir) => Path.Combine(gameDir, WorkDirName, InstalledListName);

    /** The last installed.json; null when missing or unreadable (then nothing is deleted). */
    public static VersionManifest? ReadInstalledList(string gameDir)
    {
        var file = InstalledListPath(gameDir);
        try
        {
            return File.Exists(file) ? VersionManifest.Load(file) : null;
        }
        catch (InvalidDataException)
        {
            return null;
        }
    }

    /** LauncherOwned matched on the file name, ignoring case, like ManifestBuilder's exclusions. */
    public static bool IsLauncherOwned(string path)
    {
        var name = path[(path.LastIndexOf('/') + 1)..];
        return LauncherOwned.Any(pattern => FileSystemName.MatchesSimpleExpression(pattern, name, ignoreCase: true));
    }

    public static string FileUrl(string version, string path) => $"versions/{Escape(version)}/files/{EscapePath(path)}";

    public static string PatchUrl(string version, string from, string path)
        => $"versions/{Escape(version)}/patches/{Escape(from)}/{EscapePath(path)}";

    private static PlannedDownload Full(string version, FileEntry file)
    {
        var url = FileUrl(version, file.Path);
        return new PlannedDownload(file, DownloadKind.Full, url, url);
    }

    private static string EscapePath(string path) => string.Join('/', path.Split('/').Select(Escape));

    private static string Escape(string segment) => Uri.EscapeDataString(segment);
}
