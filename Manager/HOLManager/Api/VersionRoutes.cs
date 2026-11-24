using HOLManager.Versions;

namespace HOLManager.Api;

/**
 * The routes that serve the game versions to the launcher (L2; PROTOCOL.md, Versions du jeu). Files are sent as stored,
 * with Range so that a cut download resumes; errors use the body of PROTOCOL.md ({ error, message }).
 */
public static class VersionRoutes
{
    /** Format of the /versions/latest answer, apart from ManagerSettings.ProtocolVersion (routes added, nothing changed). */
    public const int LatestFormat = 1;

    public const string LauncherVersionHeader = "X-Launcher-Version";
    public const string LauncherSha256Header = "X-Launcher-Sha256";

    public static void MapVersionRoutes(this WebApplication app)
    {
        // ?lang=<language> (launcher 3): notes and history in that language when translated, else French.
        app.MapGet("/versions/latest", (VersionStore store, string? lang) => Answer(() =>
        {
            var latest = store.Latest(lang);
            return Results.Json(new
            {
                format = LatestFormat,
                version = latest.Version,
                manifest = new { format = latest.Manifest.Format, version = latest.Manifest.Version, files = latest.Manifest.Files },
                notes = latest.Notes,
                patchesFrom = latest.PatchesFrom,
                launcherVersion = latest.LauncherVersion,
                // L21: a field added, format stays 1 (an older launcher ignores it and shows "notes" only).
                history = latest.History.Select(h => new { version = h.Version, published = h.Published, notes = h.Notes }),
            });
        }));

        app.MapGet("/versions/{version}/files/{**path}", (string version, string path, VersionStore store) => Answer(() =>
            Results.File(store.FilePath(version, path), "application/gzip", enableRangeProcessing: true)));

        app.MapGet("/versions/{version}/patches/{from}/{**path}", (string version, string from, string path, VersionStore store) =>
            Answer(() => Results.File(store.PatchPath(version, from, path), "application/octet-stream", enableRangeProcessing: true)));

        app.MapGet("/launcher/latest", (HttpResponse response, VersionStore store) => Answer(() =>
        {
            var launcher = store.Launcher();
            response.Headers[LauncherVersionHeader] = launcher.Version;
            response.Headers[LauncherSha256Header] = launcher.Sha256;
            return Results.File(launcher.Path, "application/octet-stream", VersionStore.LauncherExe, enableRangeProcessing: true);
        }));
    }

    private static IResult Answer(Func<IResult> answer)
    {
        try
        {
            return answer();
        }
        catch (VersionException e)
        {
            return Results.Json(new { error = e.Error, message = e.Message }, statusCode: e.Status);
        }
    }
}
