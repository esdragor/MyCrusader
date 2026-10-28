using HOLManager.Lobbies;
using HOLManager.Versions;

namespace HOLManager.Matches;

/**
 * D481 (R8a B2): deletes <serversDir>\<v>\ of an old version once v is not the latest (store.GameVersion), no live
 * lobby nor followed match uses it (store.VersionsInUse), and that has lasted RetireDelay (a publication renames its
 * server folder a moment before latest.txt names it). Only folders with a valid version name, never "*.tmp" (a
 * publication writing), never anything outside serversDir. A failed delete (exe still locked) is logged once and
 * retried at the next pass. Run by HousekeepingService, outside the store's lock (the delete may take seconds).
 */
public sealed class ServerCleaner(string serversDir, LobbyStore store, IClock clock, ILogger? logger = null)
{
    public static readonly TimeSpan RetireDelay = TimeSpan.FromMinutes(1);

    private readonly Dictionary<string, DateTimeOffset> unusedSince = new(StringComparer.OrdinalIgnoreCase);
    private readonly HashSet<string> failed = new(StringComparer.OrdinalIgnoreCase);

    /** The folders deleted by this pass. */
    public IReadOnlyList<string> Clean()
    {
        var deleted = new List<string>();
        string[] folders;
        try
        {
            folders = Directory.Exists(serversDir) ? Directory.GetDirectories(serversDir) : [];
        }
        catch (Exception e) when (e is IOException or UnauthorizedAccessException)
        {
            logger?.LogWarning("Servers folder {Dir} not read: {Error}", serversDir, e.Message);
            return deleted;
        }
        var latest = store.GameVersion;
        var inUse = new HashSet<string>(store.VersionsInUse(), StringComparer.OrdinalIgnoreCase);
        var now = clock.UtcNow;
        var seen = new HashSet<string>(StringComparer.OrdinalIgnoreCase);
        foreach (var folder in folders)
        {
            var version = Path.GetFileName(folder);
            if (!VersionStore.IsValidName(version) || version.EndsWith(".tmp", StringComparison.OrdinalIgnoreCase)
                || string.Equals(version, latest, StringComparison.OrdinalIgnoreCase) || inUse.Contains(version))
            {
                continue;
            }
            seen.Add(version);
            if (!unusedSince.TryGetValue(version, out var since))
            {
                unusedSince[version] = now;
                continue;
            }
            if (now - since < RetireDelay)
            {
                continue;
            }
            try
            {
                Directory.Delete(folder, recursive: true);
                deleted.Add(version);
                failed.Remove(version);
                logger?.LogInformation("Server of version {Version} deleted ({Folder}): not the latest, no lobby nor match on it.",
                    version, folder);
            }
            catch (Exception e) when (e is IOException or UnauthorizedAccessException)
            {
                if (failed.Add(version))
                {
                    logger?.LogWarning("Server of version {Version} not deleted yet ({Error}); retried at each pass.",
                        version, e.Message);
                }
            }
        }
        // A version back in use, become the latest again, or gone starts its delay anew.
        foreach (var version in unusedSince.Keys.Where(v => !seen.Contains(v) || deleted.Contains(v)).ToList())
        {
            unusedSince.Remove(version);
        }
        return deleted;
    }
}
