using HOLUpdate;

namespace HOLLauncher;

/**
 * The small line under the big button (pure, tested): "V 0.4.3 · Lanceur 5 · Serveur" then a dot, green when the last
 * reading reached the manager, red when not, grey while reading. Tooltip: the manager's name and address (servers.json),
 * with what the dot says. server: null when servers.json is unreadable (the state line already says why).
 */
public sealed record FooterLine(string Text, ServerLink Dot, string Tooltip)
{
    private const string Separator = " · ";

    public static FooterLine From(StatusView view, ManagerServer? server, string launcherVersion = LauncherInfo.Version,
        string? language = null)
    {
        var lang = Texts.LanguageOr(language);
        var game = view.Installed is null ? Texts.Get("FooterNotInstalled", lang) : VersionName.Display(view.Installed);
        var text = game + Separator + Texts.FormatIn(lang, "FooterLauncher", launcherVersion) + Separator
            + Texts.Get("FooterServer", lang);
        var where = server is null ? GameServers.FileName
            : server.Name.Trim() == "" ? server.Url
            : $"{server.Name.Trim()} ({server.Url})";
        var tooltip = view.Server switch
        {
            ServerLink.Reached => Texts.FormatIn(lang, "FooterServerOk", where),
            ServerLink.Unreachable => Texts.FormatIn(lang, "FooterServerDown", where),
            _ => Texts.FormatIn(lang, "FooterServerPending", where),
        };
        return new FooterLine(text, view.Server, tooltip);
    }
}
