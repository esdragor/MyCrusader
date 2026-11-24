namespace HOLLauncher;

/** A reading: the status (null: servers.json unreadable, no request made) and the view of it. */
public sealed record StatusReading(LauncherStatus? Status, StatusView View);

/**
 * The reading done at start and on "Refresh": servers.json, then /versions/latest, then the installed version
 * (InstalledVersion), then the view.
 * An unreadable servers.json (InvalidDataException) becomes an error view instead of an exception.
 * L23: ReadAsync also gives the status itself, for the background check (BackgroundUpdatePolicy).
 */
public static class StatusLoader
{
    /** L16: the view's texts in the given language (null: the one in use, read once before the request). */
    public static async Task<StatusView> LoadAsync(string gameDir, VersionsClient client, CancellationToken cancel = default,
        string? language = null)
        => (await ReadAsync(gameDir, client, cancel, language)).View;

    public static async Task<StatusReading> ReadAsync(string gameDir, VersionsClient client, CancellationToken cancel = default,
        string? language = null)
    {
        var lang = Texts.LanguageOr(language);
        ManagerServer server;
        try
        {
            server = GameServers.ReadFirst(gameDir);
        }
        catch (InvalidDataException e)
        {
            return new StatusReading(null, StatusView.Error(e.Message, InstalledVersion.Read(gameDir), language: lang));
        }
        // The notes in the view's language (a language change in the settings reads the status again).
        var latest = await client.GetLatestAsync(server.Url, cancel, lang);
        var status = LauncherStatus.Compute(InstalledVersion.Read(gameDir), latest, language: lang);
        return new StatusReading(status, StatusView.From(status, lang));
    }
}
