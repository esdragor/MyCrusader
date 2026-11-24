namespace HOLLauncher;

/**
 * The launcher's home page content (launcher 5, the redesign), read from the manager's GET /launcher/home?lang=<lang>
 * in one request (PROTOCOL.md, "Accueil du lanceur"): everything here is already in the asked language (the manager
 * picks it; the host writes every text in fr, en and ja, the same rule as the release notes). This file is the contract
 * between the manager side (HomeClient, the route, the host's tool) and the window (it only displays it): change it in
 * both or neither. An older manager without the route gives Empty: the window then hides these panels.
 */
public sealed record HomeContent(
    IReadOnlyList<Announcement> Announcements,
    NextSession? Session,
    IReadOnlyList<CommunityLink> Links,
    string? BackgroundFile,
    LobbiesSummary Lobbies)
{
    public static readonly HomeContent Empty = new([], null, [], null, LobbiesSummary.None);
}

/** One announcement written by the host (newest first in the list). Link: an optional http(s) address to open. */
public sealed record Announcement(string Id, DateTime DateUtc, string Title, string Body, string? Link = null);

/** The next game session set by the host ("Saturday 21:00"); null when none is planned or it is long past. */
public sealed record NextSession(DateTime StartUtc, string Title);

/** A community link shown as an icon button: Kind is "discord", "youtube", "website" or "other"; Url is http(s). */
public sealed record CommunityLink(string Kind, string Url);

/** One lobby of the manager as the home page lists it (GET /lobbies, PROTOCOL.md). State: "lobby", "starting", "in_game". */
public sealed record LobbyLine(string Name, string Map, int Players, int MaxPlayers, string State, int? Minutes);

/** The manager's lobbies right now: how many are open, how many players in them, and the list. */
public sealed record LobbiesSummary(int Open, int Players, IReadOnlyList<LobbyLine> List)
{
    public static readonly LobbiesSummary None = new(0, 0, []);
}
