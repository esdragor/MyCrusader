namespace HOLLauncher;

/** The end of an update, for the window: success, or an error message for the player. */
public sealed record UpdateResult(bool Success, string Message);

/**
 * A whole game update (L6b): plan (scan, installed.json, servers.json), downloads into <game>/.holupdate/<v>/, patches
 * applied with hpatchz (a patch that fails: the full file), then UpdateApplier. Never throws to the window: every
 * failure becomes an UpdateResult with a message; what was downloaded stays in the work folder for the next try.
 */
public sealed class GameUpdater(HttpClient http, string? hpatchz = null)
{
    private readonly PatchApplier patcher = new(hpatchz ?? PatchApplier.DefaultHpatchz);

    public async Task<UpdateResult> UpdateAsync(string gameDir, string managerUrl, LatestVersion latest,
        IProgress<DownloadProgress>? progress = null, CancellationToken cancel = default)
    {
        try
        {
            var plan = await Task.Run(() => UpdatePlan.ForGame(gameDir, latest), cancel);
            var tempDir = FileDownloader.TempDirFor(gameDir, plan.Version);
            var downloader = new FileDownloader(http, managerUrl, tempDir);
            var results = await downloader.DownloadAllAsync(plan, progress, cancel);
            var ready = new List<ReadyFile>();
            foreach (var result in results)
            {
                ready.Add(result.Kind == DownloadKind.Full
                    ? new ReadyFile(result.Planned.File, result.LocalPath)
                    : await PatchOrFullAsync(gameDir, tempDir, downloader, result, cancel));
            }
            await Task.Run(() => UpdateApplier.Apply(gameDir, plan, ready), cancel);
            return new UpdateResult(true, $"Le jeu est à jour (version {plan.Version}).");
        }
        catch (OperationCanceledException)
        {
            return new UpdateResult(false, "Mise à jour annulée ; elle reprendra où elle s'est arrêtée.");
        }
        catch (GameLockedException e)
        {
            return new UpdateResult(false, e.Message);
        }
        catch (UpdateDownloadException e)
        {
            return new UpdateResult(false, $"Téléchargement impossible : {e.Message}");
        }
        catch (Exception e) when (e is IOException or UnauthorizedAccessException or InvalidDataException)
        {
            return new UpdateResult(false, $"Mise à jour interrompue : {e.Message} ; relancez-la.");
        }
    }

    private async Task<ReadyFile> PatchOrFullAsync(
        string gameDir, string tempDir, FileDownloader downloader, DownloadResult patch, CancellationToken cancel)
    {
        var file = patch.Planned.File;
        var relative = file.Path.Replace('/', Path.DirectorySeparatorChar);
        var output = Path.Combine(tempDir, "files", relative);
        var failure = await patcher.ApplyAsync(Path.Combine(gameDir, relative), patch.LocalPath, output, file, cancel);
        File.Delete(patch.LocalPath);
        if (failure is null)
        {
            return new ReadyFile(file, output);
        }
        var full = await downloader.DownloadAsync(
            patch.Planned with { Kind = DownloadKind.Full, RelativeUrl = patch.Planned.FullUrl }, cancel: cancel);
        return new ReadyFile(file, full.LocalPath);
    }
}
