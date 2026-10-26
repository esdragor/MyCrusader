using HOLUpdate;

namespace HOLLauncher;

/** The launcher being fetched: its version (X-Launcher-Version), bytes in the file so far (resumed part included), its size when known. */
public sealed record LauncherProgress(string Version, long BytesReceived, long? TotalBytes);

/** How a launcher update ended; Version is the new launcher's on success. Messages in French, shown as they are. */
public sealed record LauncherUpdateResult(bool Success, string Message, string? Version = null);

/**
 * The launcher's own update (L7, L-Q6: on its button). GET /launcher/latest is fetched beside the running exe into
 * HOLLauncher.new.exe.part (resumed with Range by FileDownloader), checked against X-Launcher-Sha256 (a mismatch
 * deletes it), renamed HOLLauncher.new.exe, then swapped in: the running exe (Windows lets a running exe be renamed)
 * becomes HOLLauncher.old.exe and the new one takes its name; a failure in the middle puts the old one back. The
 * window then starts the new exe; HOLLauncher.old.exe is deleted at the next start (DeleteOld). All these names match
 * UpdatePlan.LauncherOwned: a game update never lists, replaces nor deletes them. exePath is injectable (tests).
 */
public sealed class LauncherUpdater(HttpClient http, string managerUrl, string exePath)
{
    public const string Route = "launcher/latest";
    public const string NewName = "HOLLauncher.new.exe";
    public const string OldName = "HOLLauncher.old.exe";
    public const string PartSuffix = ".part";
    public const string UpdateLauncherOption = "--update-launcher";

    public async Task<LauncherUpdateResult> UpdateAsync(IProgress<LauncherProgress>? progress = null, CancellationToken cancel = default)
    {
        var dir = DirOf(exePath);
        var newPath = Path.Combine(dir, NewName);
        var part = newPath + PartSuffix;
        string? version = null;
        string? sha256 = null;
        try
        {
            cancel.ThrowIfCancellationRequested();
            long kept = 0;
            long? total = null;
            var received = progress is null ? null : new Relay(bytes => progress.Report(new LauncherProgress(version!, kept + bytes, total)));
            await new FileDownloader(http, managerUrl, dir).FetchRouteAsync(Route, part, (response, start) =>
            {
                version = response.Headers.TryGetValues("X-Launcher-Version", out var v) ? v.FirstOrDefault() : null;
                sha256 = response.Headers.TryGetValues("X-Launcher-Sha256", out var s) ? s.FirstOrDefault()?.ToLowerInvariant() : null;
                if (string.IsNullOrEmpty(version) || string.IsNullOrEmpty(sha256))
                {
                    throw new UpdateDownloadException("réponse sans en-tête X-Launcher-Version ou X-Launcher-Sha256");
                }
                kept = start;
                total = response.Content.Headers.ContentLength is { } length ? start + length : null;
                progress?.Report(new LauncherProgress(version, start, total));
            }, received, cancel);
            var hash = ManifestBuilder.Hash(part);
            if (hash != sha256)
            {
                File.Delete(part);
                return new LauncherUpdateResult(false,
                    $"Le lanceur téléchargé est abîmé (SHA-256 {hash} au lieu de {sha256}) ; il a été supprimé, réessayez.");
            }
            File.Move(part, newPath, overwrite: true);
            SwapIn(exePath, newPath);
            return new LauncherUpdateResult(true, $"Le lanceur est passé à la version {version}.", version);
        }
        catch (OperationCanceledException)
        {
            return new LauncherUpdateResult(false, "Mise à jour du lanceur annulée ; elle reprendra où elle s'est arrêtée.");
        }
        catch (UpdateDownloadException e)
        {
            return new LauncherUpdateResult(false, $"Téléchargement du lanceur impossible : {e.Message}");
        }
        catch (Exception e) when (e is IOException or UnauthorizedAccessException)
        {
            return new LauncherUpdateResult(false, $"Mise à jour du lanceur interrompue : {e.Message} ; le lanceur actuel est gardé.");
        }
    }

    /**
     * exePath → HOLLauncher.old.exe (an old one left behind is deleted first), then newPath → exePath. If the second
     * move fails, the first is undone and the error thrown.
     */
    public static void SwapIn(string exePath, string newPath)
    {
        var old = Path.Combine(DirOf(exePath), OldName);
        File.Delete(old);
        File.Move(exePath, old);
        try
        {
            File.Move(newPath, exePath);
        }
        catch
        {
            File.Move(old, exePath);
            throw;
        }
    }

    /** At start: deletes HOLLauncher.old.exe beside exePath. false (silently) while still locked: tried again next start. */
    public static bool DeleteOld(string exePath)
    {
        try
        {
            File.Delete(Path.Combine(DirOf(exePath), OldName));
            return true;
        }
        catch (Exception e) when (e is IOException or UnauthorizedAccessException)
        {
            return false;
        }
    }

    /** DeleteOld retried for a few seconds, in the background at start: the old launcher that started this one is closing. */
    public static async Task<bool> DeleteOldAsync(string exePath, int attempts = 20, int delayMs = 250)
    {
        for (var i = 1; i < attempts; i++)
        {
            if (DeleteOld(exePath))
            {
                return true;
            }
            await Task.Delay(delayMs);
        }
        return DeleteOld(exePath);
    }

    /** The new launcher gets the same arguments, without --update-launcher (it is the new version already). */
    public static IReadOnlyList<string> RestartArguments(IEnumerable<string> args)
        => args.Where(a => a != UpdateLauncherOption).ToList();

    private static string DirOf(string exePath) => Path.GetDirectoryName(Path.GetFullPath(exePath))!;

    private sealed class Relay(Action<long> report) : IProgress<long>
    {
        public void Report(long value) => report(value);
    }
}
