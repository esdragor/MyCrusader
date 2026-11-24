using HOLUpdate;

namespace HOLLauncher;

/**
 * "Verify / repair the files" (L17): every file of the game folder is hashed (ManifestBuilder, whatever installed.json
 * says) and compared with the list of a version; what is damaged or missing is fetched again in full, then put in place
 * like an update (UpdateApplier: in-use check first, Version.txt last, installed.json rewritten). Reused machinery:
 * UpdatePlan.Create with no patches offered (a patch needs an intact old file), FileDownloader, UpdateApplier.
 * The reference list: the latest version's (GET /versions/latest) when it is the installed one; else installed.json
 * when it names the installed version and the manager still serves that version (one small probe request, no manager
 * route lists an older version); else the latest version's, and the player is told the game moved to it. The
 * launcher's and the player's files are never touched (UpdatePlan.Excludes, LauncherOwned). Never throws to the window.
 */
public sealed class GameRepairer(HttpClient http, Func<string, long>? freeSpace = null)
{
    public async Task<UpdateResult> RepairAsync(string gameDir, string managerUrl, LatestVersion latest,
        IProgress<DownloadProgress>? progress = null, CancellationToken cancel = default)
    {
        try
        {
            cancel.ThrowIfCancellationRequested();
            var installed = InstalledVersion.Read(gameDir);
            if (installed is null)
            {
                return new UpdateResult(false, Texts.Get("RepairNotInstalled"));
            }
            var previous = UpdatePlan.ReadInstalledList(gameDir);
            var (version, reference) = await ReferenceAsync(managerUrl, installed, previous, latest, cancel);
            // The scan carries the installed version: UpdatePlan names it Installed, and offers no patch (PatchesFrom empty).
            var scanned = await Task.Run(() => ManifestBuilder.Build(gameDir, installed, UpdatePlan.Excludes), cancel);
            var plan = UpdatePlan.Create(scanned, new LatestVersion(version, reference, "", [], ""), previous,
                File.Exists(Path.Combine(gameDir, UpdatePlan.ServersName)));
            CheckFreeSpace(gameDir, plan);
            var downloader = new FileDownloader(http, managerUrl, FileDownloader.TempDirFor(gameDir, plan.Version));
            var results = await downloader.DownloadAllAsync(plan, progress, cancel);
            var ready = results.Select(r => new ReadyFile(r.Planned.File, r.LocalPath)).ToList();
            await Task.Run(() => UpdateApplier.Apply(gameDir, plan, ready), cancel);
            var fixedCount = plan.Downloads.Count + plan.ToDelete.Count;
            if (version != installed)
            {
                return new UpdateResult(true, Texts.Format("RepairToLatest", VersionName.Display(installed),
                    VersionName.Display(version), plan.Downloads.Count));
            }
            return new UpdateResult(true, fixedCount == 0
                ? Texts.Format("RepairIntact", VersionName.Display(version))
                : Texts.Format("RepairDone", fixedCount, VersionName.Display(version)));
        }
        catch (OperationCanceledException)
        {
            return new UpdateResult(false, Texts.Get("RepairCancelled"), Cancelled: true);
        }
        catch (NotEnoughSpaceException e)
        {
            return new UpdateResult(false, e.Message);
        }
        catch (GameLockedException e)
        {
            return new UpdateResult(false, e.Message);
        }
        catch (UpdateDownloadException e)
        {
            return new UpdateResult(false, Texts.Format("RepairDownloadFailed", e.Message));
        }
        catch (Exception e) when (e is IOException or UnauthorizedAccessException or InvalidDataException)
        {
            return new UpdateResult(false, Texts.Format("RepairInterrupted", e.Message));
        }
    }

    /** The version to repair to and its list (see the class comment). */
    private async Task<(string Version, VersionManifest List)> ReferenceAsync(string managerUrl, string installed,
        VersionManifest? previous, LatestVersion latest, CancellationToken cancel)
    {
        if (installed == latest.Version)
        {
            return (latest.Version, latest.Manifest);
        }
        if (previous is not null && previous.Version == installed && previous.Files.Count > 0
            && await ServesAsync(managerUrl, installed, previous, cancel))
        {
            return (installed, previous);
        }
        return (latest.Version, latest.Manifest);
    }

    /**
     * Whether the manager still serves the version: the answer's headers to the smallest file of its list
     * (Version.txt), the body is not read. Any failure counts as no: the latest version is then repaired instead.
     */
    private async Task<bool> ServesAsync(string managerUrl, string version, VersionManifest list, CancellationToken cancel)
    {
        var probe = list.Files.OrderBy(f => f.Size).ThenBy(f => f.Path, StringComparer.Ordinal).First();
        var uri = new Uri(new Uri(managerUrl.TrimEnd('/') + "/"), UpdatePlan.FileUrl(version, probe.Path));
        try
        {
            using var response = await http.GetAsync(uri, HttpCompletionOption.ResponseHeadersRead, cancel);
            return response.IsSuccessStatusCode;
        }
        catch (HttpRequestException)
        {
            return false;
        }
    }

    /** As GameUpdater: every file to fetch at its final size plus GameUpdater.DiskMargin, checked before any download. */
    private void CheckFreeSpace(string gameDir, UpdatePlan plan)
    {
        if (plan.Downloads.Count == 0)
        {
            return;
        }
        var needed = plan.Downloads.Sum(d => d.File.Size) + GameUpdater.DiskMargin;
        var free = (freeSpace ?? FreeSpace)(gameDir);
        if (free < needed)
        {
            throw new NotEnoughSpaceException(Texts.Format("RepairNoSpace", Gigabytes(needed), gameDir, Gigabytes(free)));
        }
    }

    private static long FreeSpace(string folder)
    {
        try
        {
            return new DriveInfo(Path.GetPathRoot(Path.GetFullPath(folder))!).AvailableFreeSpace;
        }
        catch (Exception e) when (e is ArgumentException or IOException or UnauthorizedAccessException)
        {
            return long.MaxValue;
        }
    }

    private static string Gigabytes(long bytes)
        => (bytes / (1024.0 * 1024 * 1024)).ToString("0.0", System.Globalization.CultureInfo.GetCultureInfo(Texts.Language));
}
