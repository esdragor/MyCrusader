namespace HOLLauncher;

/**
 * What the window gives to read the home content: its HTTP client (it sends the access key of servers.json, L11), the
 * manager to ask (the first one of servers.json), the game folder (a cache may live there) and the language the
 * texts are wanted in.
 */
public sealed record HomeRequest(HttpClient Http, ManagerServer Server, string GameDir, string Language);

/**
 * Where the home window gets its HomeContent (announcements, community links, background), read in the background
 * after each status reading. The manager side (HomeClient, GET /launcher/home) implements it; the window only shows
 * what it gives, and treats a failure as HomeContent.Empty (panels hidden, vector art).
 */
public interface IHomeSource
{
    Task<HomeContent> LoadAsync(HomeRequest request, CancellationToken cancel);
}

/** No home content yet (a manager without the route, or before HomeClient is wired): always HomeContent.Empty. */
public sealed class NoHomeSource : IHomeSource
{
    public static readonly NoHomeSource Instance = new();

    public Task<HomeContent> LoadAsync(HomeRequest request, CancellationToken cancel) => Task.FromResult(HomeContent.Empty);
}
