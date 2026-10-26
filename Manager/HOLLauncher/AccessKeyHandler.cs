namespace HOLLauncher;

/**
 * L11: adds the key of servers.json (X-HOL-Key) to every request sent to its manager, and to it only (same scheme,
 * host and port), so that a manager with a key serves the friends' kit (PROTOCOL.md, Clé d'accès). The server is asked
 * for each request (servers.json read again: "Refresh" sees a new kit); none or no key: nothing added.
 */
public sealed class AccessKeyHandler(Func<ManagerServer?> server) : DelegatingHandler
{
    public const string Header = "X-HOL-Key";

    /** What the player sees when the manager answers 401 bad_key. */
    public const string BadKeyMessage = "Clé d'accès refusée : le kit n'est pas le bon. Redemandez-le.";

    /** The first server of <gameDir>/servers.json; an unreadable file gives none (the window says so elsewhere). */
    public static Func<ManagerServer?> FromGame(string gameDir) => () =>
    {
        try
        {
            return GameServers.ReadFirst(gameDir);
        }
        catch (Exception e) when (e is InvalidDataException or IOException or UnauthorizedAccessException)
        {
            return null;
        }
    };

    /** An HttpClient sending the key of <gameDir>/servers.json to its manager. */
    public static HttpClient CreateHttpClient(string gameDir, TimeSpan timeout) =>
        new(new AccessKeyHandler(FromGame(gameDir)) { InnerHandler = new HttpClientHandler() }) { Timeout = timeout };

    protected override Task<HttpResponseMessage> SendAsync(HttpRequestMessage request, CancellationToken cancellationToken)
    {
        if (server() is { Key: { Length: > 0 } key } manager && request.RequestUri is { } uri
            && Uri.TryCreate(manager.Url, UriKind.Absolute, out var managerUri)
            && Uri.Compare(uri, managerUri, UriComponents.SchemeAndServer, UriFormat.Unescaped, StringComparison.OrdinalIgnoreCase) == 0)
        {
            request.Headers.Remove(Header);
            request.Headers.Add(Header, key);
        }
        return base.SendAsync(request, cancellationToken);
    }

    /** True for the manager's 401 with error bad_key. */
    public static bool IsBadKey(int code, string? error) => code == 401 && error == "bad_key";
}
