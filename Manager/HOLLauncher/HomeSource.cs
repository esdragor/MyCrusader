namespace HOLLauncher;

/**
 * What the window gives to read the home content: the manager to ask (the first one of servers.json), the game folder
 * (its servers.json holds the access key, L11) and the language the texts are wanted in.
 */
public sealed record HomeRequest(ManagerServer Server, string GameDir, string Language);

/**
 * Where the home window gets its HomeContent (announcements, community links, background), read in the background
 * with each status reading (start, "Refresh", a language or game folder change). The window only shows what it gives
 * and treats a failure as HomeContent.Empty (panels hidden, vector art). The seam lets the renders and tests give a
 * sample without a manager.
 */
public interface IHomeSource
{
    Task<HomeContent> LoadAsync(HomeRequest request, CancellationToken cancel);
}

/**
 * The launcher's: HomeClient (GET /launcher/home, its cache beside the launcher's exe). One client per game folder,
 * kept: its HttpClient carries that folder's access key, and a new one per reading would leak sockets.
 */
public sealed class ManagerHomeSource(string launcherDir) : IHomeSource
{
    private readonly object gate = new();
    private string? clientDir;
    private HomeClient? client;

    public Task<HomeContent> LoadAsync(HomeRequest request, CancellationToken cancel)
    {
        HomeClient home;
        lock (gate)
        {
            if (client is null || !GameFolder.Same(clientDir!, request.GameDir))
            {
                client = HomeClient.ForLauncher(request.GameDir, launcherDir);
                clientDir = request.GameDir;
            }
            home = client;
        }
        return home.GetAsync(request.Server.Url, request.Language, cancel);
    }
}

/** No home content (the renders' "nothing published" case): always HomeContent.Empty. */
public sealed class NoHomeSource : IHomeSource
{
    public static readonly NoHomeSource Instance = new();

    public Task<HomeContent> LoadAsync(HomeRequest request, CancellationToken cancel) => Task.FromResult(HomeContent.Empty);
}
