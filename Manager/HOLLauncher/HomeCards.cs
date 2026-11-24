using System.Globalization;
using HOLUpdate;

namespace HOLLauncher;

/**
 * One card of the "Updates" tab: a version's name (null: unknown), its date (null: the manager sends none yet) and its
 * lines (Text, Bullet, or Gap for a blank line inside the notes).
 */
public sealed record NewsCard(string? Title, string? Date, IReadOnlyList<NewsBlock> Lines);

/** The "Updates" tab: one card per version, newest first; OlderMissing: the closing line, null when none. */
public sealed record NewsCards(IReadOnlyList<NewsCard> Cards, string? OlderMissing);

/** One card of the "Announcements" tab, ready to show: the date in the language, the link only if it may be opened. */
public sealed record AnnouncementCard(string Date, string Title, string Body, Uri? Link);

/** A community link's icon (vector, drawn by the window). */
public enum LinkIcon
{
    Discord,
    YouTube,
    Website,
    Other,
}

/** A community link ready to show: its icon, the address to open, the tooltip. */
public sealed record CommunityLinkButton(LinkIcon Icon, Uri Url, string Tooltip);

/**
 * The cards of the home window's information panel (pure, tested). The release notes come as ReleaseNotes.Blocks
 * (StatusView.News): a heading starts a version's card; notes without heading (up to date, not installed: the latest
 * version alone) make one card named after latest. The blank lines between two versions are the cards' gaps, not
 * lines; the "older notes are gone" line closes the tab. Announcements and links come from HomeContent: what cannot be
 * opened safely is left out (LinkRules).
 */
public static class HomeCards
{
    public static NewsCards News(IReadOnlyList<NewsBlock> blocks, string? latest)
    {
        var cards = new List<NewsCard>();
        string? older = null;
        string? title = latest is null ? null : VersionName.Display(latest);
        var lines = new List<NewsBlock>();
        void Close()
        {
            while (lines.Count > 0 && lines[^1].Kind == NewsKind.Gap)
            {
                lines.RemoveAt(lines.Count - 1);
            }
            if (lines.Count > 0)
            {
                cards.Add(new NewsCard(title, null, lines.ToList()));
            }
            lines.Clear();
        }
        foreach (var block in blocks)
        {
            switch (block.Kind)
            {
                case NewsKind.Heading:
                    Close();
                    title = block.Text;
                    break;
                case NewsKind.OlderMissing:
                    older = block.Text;
                    break;
                case NewsKind.Gap when lines.Count == 0:
                    break;
                default:
                    lines.Add(block);
                    break;
            }
        }
        Close();
        return new NewsCards(cards, older);
    }

    /** The announcements in the given order (the manager's: newest first), their date in the language's own way. */
    public static IReadOnlyList<AnnouncementCard> Announcements(IEnumerable<Announcement> announcements, string? language = null)
    {
        var lang = Texts.LanguageOr(language);
        return announcements
            .Where(a => !string.IsNullOrWhiteSpace(a.Title) || !string.IsNullOrWhiteSpace(a.Body))
            .Select(a => new AnnouncementCard(Date(a.DateUtc, lang), a.Title.Trim(), a.Body.Trim(), LinkRules.Openable(a.Link)))
            .ToList();
    }

    /** The day of an announcement (its UTC date: a day, not a moment), "24 novembre 2026", "November 24, 2026"... */
    public static string Date(DateTime dateUtc, string language)
        => dateUtc.ToString(Texts.Get("AnnouncementDate", language), CultureInfo.GetCultureInfo(language));

    /** The links that may be opened, in the given order; an unknown kind is "other". */
    public static IReadOnlyList<CommunityLinkButton> Links(IEnumerable<CommunityLink> links, string? language = null)
    {
        var lang = Texts.LanguageOr(language);
        var buttons = new List<CommunityLinkButton>();
        foreach (var link in links)
        {
            if (LinkRules.Openable(link.Url) is not { } url)
            {
                continue;
            }
            var icon = link.Kind?.Trim().ToLowerInvariant() switch
            {
                "discord" => LinkIcon.Discord,
                "youtube" => LinkIcon.YouTube,
                "website" => LinkIcon.Website,
                _ => LinkIcon.Other,
            };
            var tooltip = icon switch
            {
                LinkIcon.Discord => "Discord",
                LinkIcon.YouTube => "YouTube",
                LinkIcon.Website => Texts.Get("LinkWebsite", lang),
                _ => url.Host,
            };
            buttons.Add(new CommunityLinkButton(icon, url, tooltip));
        }
        return buttons;
    }
}
