namespace HOLLauncher;

/**
 * The launcher's home page content (launcher 5, the redesign), read from the manager's GET /launcher/home?lang=<lang>
 * in one request (PROTOCOL.md, "Accueil du lanceur"): everything here is already in the asked language (the manager
 * picks it; the host writes every text in fr, en and ja, the same rule as the release notes). This file is the contract
 * between the manager side (HomeClient, the route, the host's tool) and the window (it only displays it): change it in
 * both or neither. An older manager without the route gives Empty: the window then hides these panels.
 * Not here on purpose (the user's choice, 2026-11-24): no next-session countdown, no live lobbies / players panel.
 */
public sealed record HomeContent(
    IReadOnlyList<Announcement> Announcements,
    IReadOnlyList<CommunityLink> Links,
    string? BackgroundFile)
{
    public static readonly HomeContent Empty = new([], [], null);
}

/** One announcement written by the host (newest first in the list). Link: an optional http(s) address to open. */
public sealed record Announcement(string Id, DateTime DateUtc, string Title, string Body, string? Link = null);

/** A community link shown as an icon button: Kind is "discord", "youtube", "website" or "other"; Url is http(s). */
public sealed record CommunityLink(string Kind, string Url);
