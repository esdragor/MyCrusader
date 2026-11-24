namespace HOLLauncher;

/**
 * The reading done at start and on "Refresh": servers.json, then /versions/latest, then the installed version
 * (InstalledVersion), then the view.
 * An unreadable servers.json (InvalidDataException) becomes an error view instead of an exception.
 */
public static class StatusLoader
{
    /** L16: the view's texts in the given language (null: the one in use, read once before the request). */
    public static async Task<StatusView> LoadAsync(string gameDir, VersionsClient client, CancellationToken cancel = default,
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
            return StatusView.Error(e.Message, InstalledVersion.Read(gameDir), language: lang);
        }
        // The notes in the view's language (a language change in the settings reads the status again).
        var latest = await client.GetLatestAsync(server.Url, cancel, lang);
        return StatusView.From(LauncherStatus.Compute(InstalledVersion.Read(gameDir), latest), lang);
    }
}
