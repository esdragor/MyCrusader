using System.Windows;
using System.Windows.Controls;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using System.Windows.Shapes;

namespace HOLLauncher;

/**
 * The right of the home window (launcher 5): the information panel's tabs, "Updates" (the release notes as one card per
 * version, HomeCards.News) and "Announcements" (HomeContent, hidden when there is none), the community links under it
 * (round icon buttons, hidden when none), and the host's background picture in place of the vector art. The cards are
 * built here from the pure HomeCards; their colours are resource references, so a theme change recolours them.
 * The background file is read into memory and closed at once (BitmapCacheOption.OnLoad from a stream disposed here):
 * HomeClient replaces it when a newer picture arrives, the window must never hold it open.
 */
public sealed partial class MainWindow
{
    private HomeContent home = HomeContent.Empty;
    private IReadOnlyList<NewsBlock>? shownNews;
    private string? shownBackground;

    /** Reads the home content beside the status; a failure is an empty home (the panels hide, the art stays). */
    private async Task LoadHomeAsync()
    {
        var gameDir = Options.GameDir;
        var language = Texts.Language;
        HomeContent content;
        if (server is not { } manager)
        {
            content = HomeContent.Empty;
        }
        else
        {
            try
            {
                content = await Task.Run(() => homeSource.LoadAsync(new HomeRequest(manager, gameDir, language), closing.Token));
            }
            catch (OperationCanceledException)
            {
                return; // the launcher closes
            }
            catch (Exception e) when (e is HttpRequestException or IOException or InvalidOperationException or UnauthorizedAccessException)
            {
                content = HomeContent.Empty;
            }
        }
        if (!closed && GameFolder.Same(gameDir, Options.GameDir) && language == Texts.Language)
        {
            home = content;
            ShowHome();
        }
    }

    /** The announcements tab, the links and the background for the home content in hand. */
    private void ShowHome()
    {
        var announcements = HomeCards.Announcements(home.Announcements);
        AnnouncementsTab.Visibility = announcements.Count > 0 ? Visibility.Visible : Visibility.Collapsed;
        if (announcements.Count == 0 && AnnouncementsTab.IsChecked == true)
        {
            UpdatesTab.IsChecked = true;
        }
        ShowLinks(HomeCards.Links(home.Links));
        ShowBackground(home.BackgroundFile);
        ShowCards();
    }

    /** The notes' cards again only when the notes changed (keeps the scroll); ApplyTexts forces it. */
    private void ShowNews()
    {
        if (ReferenceEquals(shownNews, current.News))
        {
            return;
        }
        shownNews = current.News;
        if (UpdatesTab.IsChecked == true)
        {
            ShowCards();
        }
    }

    private void OnTabChecked(object sender, RoutedEventArgs e)
    {
        if (Cards is not null)
        {
            ShowCards();
        }
    }

    private void ShowCards()
    {
        Cards.Children.Clear();
        if (AnnouncementsTab.IsChecked == true)
        {
            foreach (var card in HomeCards.Announcements(home.Announcements))
            {
                Cards.Children.Add(AnnouncementCardElement(card));
            }
        }
        else
        {
            var news = HomeCards.News(current.News, current.Latest);
            foreach (var card in news.Cards)
            {
                Cards.Children.Add(NewsCardElement(card));
            }
            if (news.Cards.Count == 0)
            {
                Cards.Children.Add(Text(current.IsLoading ? Texts.Get("PrimaryLoading") : Texts.Get("NotesNone"), "TextBrush", 14,
                    margin: new Thickness(2, 8, 0, 0)));
            }
            if (news.OlderMissing is { } older)
            {
                var line = Text(older, "TextBrush", 13, margin: new Thickness(2, 2, 0, 4));
                line.FontStyle = FontStyles.Italic;
                Cards.Children.Add(line);
            }
        }
        CardsScroll.ScrollToTop();
    }

    private UIElement NewsCardElement(NewsCard card)
    {
        var stack = new StackPanel();
        if (card.Title is { } title)
        {
            var heading = Text(title, "HeadingBrush", 17, margin: new Thickness(0, 0, 0, 6));
            heading.FontWeight = FontWeights.SemiBold;
            // A version's name is Latin in every language: Segoe UI, whose bold Yu Gothic UI would barely show.
            heading.FontFamily = new FontFamily("Segoe UI");
            stack.Children.Add(heading);
        }
        if (card.Date is { } date)
        {
            stack.Children.Add(Text(date, "MutedBrush", 12, margin: new Thickness(0, -4, 0, 6)));
        }
        foreach (var line in card.Lines)
        {
            stack.Children.Add(line.Kind switch
            {
                NewsKind.Gap => new Border { Height = 6 },
                NewsKind.Bullet => Bullet(line.Text),
                _ => Text(line.Text, "TextBrush", 14, margin: new Thickness(0, 1, 0, 1)),
            });
        }
        return Card(stack);
    }

    private UIElement AnnouncementCardElement(AnnouncementCard card)
    {
        var stack = new StackPanel();
        stack.Children.Add(Text(card.Date, "MutedBrush", 12, margin: new Thickness(0, 0, 0, 2)));
        if (card.Title != "")
        {
            var title = Text(card.Title, "TextBrush", 16.5, margin: new Thickness(0, 0, 0, 6));
            title.FontWeight = FontWeights.SemiBold;
            stack.Children.Add(title);
        }
        if (card.Body != "")
        {
            stack.Children.Add(Text(card.Body, "TextBrush", 14));
        }
        if (card.Link is { } link)
        {
            var more = new Button
            {
                Content = Texts.Get("AnnouncementMore"),
                Style = (Style)FindResource("LinkButton"),
                Margin = new Thickness(0, 10, 0, 0),
                ToolTip = link.AbsoluteUri,
            };
            more.Click += (_, _) => LinkRules.Open(link.AbsoluteUri);
            stack.Children.Add(more);
        }
        return Card(stack);
    }

    private static Border Card(UIElement content)
    {
        var card = new Border { CornerRadius = new CornerRadius(8), Padding = new Thickness(16, 14, 16, 14), Margin = new Thickness(0, 0, 0, 10), Child = content };
        card.SetResourceReference(Border.BackgroundProperty, "CardBrush");
        return card;
    }

    private static UIElement Bullet(string text)
    {
        var grid = new Grid { Margin = new Thickness(0, 2, 0, 2) };
        grid.ColumnDefinitions.Add(new ColumnDefinition { Width = new GridLength(18) });
        grid.ColumnDefinitions.Add(new ColumnDefinition { Width = new GridLength(1, GridUnitType.Star) });
        var dot = new Ellipse { Width = 5, Height = 5, VerticalAlignment = VerticalAlignment.Top, HorizontalAlignment = HorizontalAlignment.Left, Margin = new Thickness(3, 8, 0, 0) };
        dot.SetResourceReference(Shape.FillProperty, "HeadingBrush");
        var line = Text(text, "TextBrush", 14);
        Grid.SetColumn(line, 1);
        grid.Children.Add(dot);
        grid.Children.Add(line);
        return grid;
    }

    private static TextBlock Text(string text, string brush, double size, Thickness margin = default)
    {
        var block = new TextBlock { Text = text, FontSize = size, TextWrapping = TextWrapping.Wrap, Margin = margin };
        block.SetResourceReference(TextBlock.ForegroundProperty, brush);
        return block;
    }

    private void ShowLinks(IReadOnlyList<CommunityLinkButton> links)
    {
        LinksRow.Children.Clear();
        foreach (var link in links)
        {
            var (icon, filled) = link.Icon switch
            {
                LinkIcon.Discord => ("IconChat", true),
                LinkIcon.YouTube => ("IconVideo", true),
                LinkIcon.Website => ("IconGlobe", false),
                _ => ("IconExternal", false),
            };
            var path = new System.Windows.Shapes.Path
            {
                Data = (Geometry)FindResource(icon),
                Style = (Style)FindResource(filled ? "IconFill" : "IconStroke"),
                Width = 24,
                Height = 24,
            };
            var button = new Button { Style = (Style)FindResource("RoundLinkButton"), Content = path, ToolTip = link.Tooltip };
            var url = link.Url.AbsoluteUri;
            button.Click += (_, _) => LinkRules.Open(url);
            LinksRow.Children.Add(button);
        }
        LinksRow.Visibility = links.Count > 0 ? Visibility.Visible : Visibility.Collapsed;
    }

    /** The host's picture when readable (in memory, the file closed), else the vector art. */
    private void ShowBackground(string? file)
    {
        // The same name may hold a newer picture (HomeClient replaces it): the file's time is part of the key.
        var key = file is null ? null : $"{file}|{SafeWriteTime(file):O}";
        if (key == shownBackground)
        {
            return;
        }
        shownBackground = key;
        BitmapImage? image = null;
        if (file is not null)
        {
            try
            {
                using var stream = File.OpenRead(file);
                image = new BitmapImage();
                image.BeginInit();
                image.CacheOption = BitmapCacheOption.OnLoad;
                image.CreateOptions = BitmapCreateOptions.IgnoreImageCache | BitmapCreateOptions.IgnoreColorProfile;
                image.StreamSource = stream;
                image.EndInit();
                image.Freeze();
            }
            catch (Exception e) when (e is IOException or UnauthorizedAccessException or NotSupportedException
                or ArgumentException or InvalidOperationException)
            {
                image = null;
            }
        }
        BackgroundImage.Source = image;
        BackgroundImage.Visibility = image is null ? Visibility.Collapsed : Visibility.Visible;
        Art.Visibility = image is null ? Visibility.Visible : Visibility.Collapsed;
    }

    private static DateTime SafeWriteTime(string file)
    {
        try
        {
            return File.GetLastWriteTimeUtc(file);
        }
        catch (Exception e) when (e is IOException or UnauthorizedAccessException or ArgumentException or NotSupportedException)
        {
            return DateTime.MinValue;
        }
    }
}
