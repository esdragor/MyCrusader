using HOLUpdate;

namespace HOLLauncher.Tests;

/** Launcher 5: the cards of the information panel (release notes per version, announcements, community links). */
public class HomeCardsTests
{
    private static NewsBlock H(string text) => new(NewsKind.Heading, text);
    private static NewsBlock T(string text) => new(NewsKind.Text, text);
    private static NewsBlock B(string text) => new(NewsKind.Bullet, text);
    private static readonly NewsBlock Gap = new(NewsKind.Gap);

    [Fact]
    public void OneCardPerVersionWithoutTheGapsBetweenThem()
    {
        var cards = HomeCards.News([H("V 0.4.3"), B("Archers."), Gap, T("Fin."), Gap, H("V 0.4.2"), T("Un."), Gap,
            new NewsBlock(NewsKind.OlderMissing, "Plus ancien : rien.")], "0.4.3");

        Assert.Equal(2, cards.Cards.Count);
        Assert.Equal("V 0.4.3", cards.Cards[0].Title);
        Assert.Equal([B("Archers."), Gap, T("Fin.")], cards.Cards[0].Lines);
        Assert.Equal("V 0.4.2", cards.Cards[1].Title);
        Assert.Equal([T("Un.")], cards.Cards[1].Lines);
        Assert.Null(cards.Cards[0].Date);
        Assert.Equal("Plus ancien : rien.", cards.OlderMissing);
    }

    [Fact]
    public void NotesWithoutHeadingAreTheLatestVersionsCard()
    {
        var cards = HomeCards.News([T("Nouvelle carte."), B("Archers.")], "0.4.3");
        var card = Assert.Single(cards.Cards);
        Assert.Equal("V 0.4.3", card.Title);
        Assert.Equal([T("Nouvelle carte."), B("Archers.")], card.Lines);
        Assert.Null(cards.OlderMissing);
        Assert.Null(HomeCards.News([T("x")], null).Cards[0].Title);
    }

    [Fact]
    public void NoNotesNoCards()
    {
        Assert.Empty(HomeCards.News([], "0.4.3").Cards);
        Assert.Empty(HomeCards.News([H("V 0.4.3"), Gap], "0.4.3").Cards);
    }

    /** From the manager's notes to the cards: the real path of the window (ReleaseNotes.Blocks, then the cards). */
    [Fact]
    public void ThreeVersionsBehindGiveThreeCards()
    {
        ReleaseNote[] history = [new("0.4.3", "- Carte.\n- Archers."), new("0.4.2", "Ponts."), new("0.4.1", "Tours."), new("0.4.0", "x")];
        var latest = new LatestVersion("0.4.3", new VersionManifest("0.4.3", []), "", [], "", history);

        var cards = HomeCards.News(ReleaseNotes.Blocks("0.4.0", latest, "fr"), "0.4.3");

        Assert.Equal(["V 0.4.3", "V 0.4.2", "V 0.4.1"], cards.Cards.Select(c => c.Title));
        Assert.Equal([B("Carte."), B("Archers.")], cards.Cards[0].Lines);
    }

    [Theory]
    [InlineData("fr", "24 novembre 2026")]
    [InlineData("en", "November 24, 2026")]
    [InlineData("ja", "2026年11月24日")]
    public void AnnouncementDateInTheLanguage(string language, string expected)
    {
        Assert.Equal(expected, HomeCards.Date(new DateTime(2026, 11, 24, 23, 30, 0, DateTimeKind.Utc), language));
    }

    [Fact]
    public void AnnouncementsKeepTheirOrderAndOnlyOpenableLinks()
    {
        var cards = HomeCards.Announcements([
            new Announcement("a", new DateTime(2026, 11, 24), " Session samedi ", "Venez nombreux.\n", "https://example.org/a"),
            new Announcement("b", new DateTime(2026, 11, 20), "Maintenance", "Serveur coupé.", "javascript:alert(1)"),
            new Announcement("c", new DateTime(2026, 11, 19), " ", " "),
        ], "fr");

        Assert.Equal(2, cards.Count);
        Assert.Equal(new AnnouncementCard("24 novembre 2026", "Session samedi", "Venez nombreux.", new Uri("https://example.org/a")), cards[0]);
        Assert.Null(cards[1].Link);
    }

    [Fact]
    public void LinksGetTheirIconAndTooltipAndBadOnesAreDropped()
    {
        var links = HomeCards.Links([
            new CommunityLink("Discord", "https://discord.gg/abc"),
            new CommunityLink("youtube", "https://youtube.com/@hol"),
            new CommunityLink("website", "https://heroes.example.org/"),
            new CommunityLink("forum", "https://forum.example.org/hol"),
            new CommunityLink("discord", "file:///C:/Windows/System32/cmd.exe"),
            new CommunityLink("website", "not a url"),
        ], "fr");

        Assert.Equal([LinkIcon.Discord, LinkIcon.YouTube, LinkIcon.Website, LinkIcon.Other], links.Select(l => l.Icon));
        Assert.Equal(["Discord", "YouTube", "Site du jeu", "forum.example.org"], links.Select(l => l.Tooltip));
        Assert.Equal("Game website", HomeCards.Links([new CommunityLink("website", "https://x.org")], "en")[0].Tooltip);
    }
}
