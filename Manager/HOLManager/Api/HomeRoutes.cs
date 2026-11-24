using HOLManager.Versions;
using Microsoft.Net.Http.Headers;

namespace HOLManager.Api;

/**
 * The launcher's home page (launcher 5; PROTOCOL.md, "Accueil du lanceur"): the host's announcements and community links
 * in the asked language, and the key art. Added beside the versions routes, behind the same access key (UseAccessKey
 * guards every route); protocolVersion does not change, the answer carries its own format.
 */
public static class HomeRoutes
{
    public const int HomeFormat = 1;
    public const string BackgroundRoute = "/launcher/home/background";

    public static void MapHomeRoutes(this WebApplication app)
    {
        app.MapGet("/launcher/home", (HomeStore store, string? lang) =>
        {
            var page = store.Read(lang);
            return Results.Json(new
            {
                format = HomeFormat,
                announcements = page.Announcements.Select(a => new { id = a.Id, date = a.Date, title = a.Title, body = a.Body, link = a.Link }),
                links = page.Links.Select(l => new { kind = l.Kind, url = l.Url }),
                background = page.HasBackground ? BackgroundRoute : null,
            });
        });

        // ETag and Last-Modified: Results.File answers If-None-Match / If-Modified-Since with 304 (the launcher's cache).
        app.MapGet(BackgroundRoute, (HomeStore store) => store.Background() is { } background
            ? Results.File(background.Path, background.ContentType, lastModified: background.LastWriteUtc,
                entityTag: EntityTagHeaderValue.Parse(background.ETag))
            : Results.Json(new { error = "no_background", message = $"No launcher background in {store.Dir}." }, statusCode: 404));
    }
}
