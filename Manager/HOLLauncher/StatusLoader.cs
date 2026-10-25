namespace HOLLauncher;

/**
 * The reading done at start and on "Refresh": servers.json, then /versions/latest, then the installed version
 * (InstalledVersion), then the view.
 * An unreadable servers.json (InvalidDataException) becomes an error view instead of an exception.
 */
public static class StatusLoader
{
    public static async Task<StatusView> LoadAsync(string gameDir, VersionsClient client, CancellationToken cancel = default)
    {
        ManagerServer server;
        try
        {
            server = GameServers.ReadFirst(gameDir);
        }
        catch (InvalidDataException e)
        {
            return StatusView.Error(e.Message, InstalledVersion.Read(gameDir));
        }
        var latest = await client.GetLatestAsync(server.Url, cancel);
        return StatusView.From(LauncherStatus.Compute(InstalledVersion.Read(gameDir), latest));
    }
}
