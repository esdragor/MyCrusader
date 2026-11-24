using System.Windows;
using System.Windows.Media;

namespace HOLLauncher;

/** What a message says it is: its badge's colour and glyph. */
public enum DialogKind
{
    Info,
    Warning,
    Error,
    Question,
}

/**
 * The launcher's dialogs (launcher 5) in place of MessageBox, whose buttons follow Windows' language and theme, not the
 * launcher's: the message (selectable, to copy a path or an error), "OK", or "Yes" / "No" (Escape is No; defaultNo puts
 * Enter on No, for the uninstall). Same custom frame and palette as the main window.
 */
public sealed partial class MessageWindow : Window
{
    private bool answer;

    private MessageWindow(Palette palette, string message, DialogKind kind, bool yesNo, bool defaultNo)
    {
        InitializeComponent();
        WpfTheme.Apply(this, palette);
        Title = MainWindow.GameName;
        TitleText.Text = MainWindow.GameName;
        FontFamily = new FontFamily(Texts.FontFamily(Texts.Language));
        MessageText.Text = message;
        YesButton.Visibility = NoButton.Visibility = yesNo ? Visibility.Visible : Visibility.Collapsed;
        OkButton.Visibility = yesNo ? Visibility.Collapsed : Visibility.Visible;
        var defaultButton = !yesNo ? OkButton : defaultNo ? NoButton : YesButton;
        defaultButton.IsDefault = true;
        (yesNo ? NoButton : OkButton).IsCancel = true;
        Badge.SetResourceReference(System.Windows.Shapes.Shape.FillProperty, kind switch
        {
            DialogKind.Error => "ServerUnreachableBrush",
            _ => "AccentBackBrush", // the white glyph is tested on it (AccentText)
        });
        Glyph.Data = Geometry.Parse(kind switch
        {
            DialogKind.Question => "M 11.6,11.6 A 3.4,3.4 0 1 1 16.6,14.6 C 15.6,15.2 15,16 15,17.4 M 15,21.6 V 21.8",
            DialogKind.Info => "M 15,13.6 V 21.6 M 15,9 V 9.2",
            _ => "M 15,8.6 V 17 M 15,21.4 V 21.6",
        });
        Loaded += (_, _) => defaultButton.Focus();
    }

    /** Shows the dialog over owner (centred on the screen without one); true when the answer is Yes (or OK). */
    public static bool Show(Window? owner, Palette palette, string message, DialogKind kind, bool yesNo = false, bool defaultNo = false)
    {
        var window = new MessageWindow(palette, message, kind, yesNo, defaultNo);
        if (owner is { IsLoaded: true })
        {
            window.Owner = owner;
        }
        else
        {
            window.WindowStartupLocation = WindowStartupLocation.CenterScreen;
            window.ShowInTaskbar = true;
        }
        window.ShowDialog();
        return window.answer;
    }

    private void OnYesClick(object sender, RoutedEventArgs e)
    {
        answer = true;
        Close();
    }

    private void OnNoClick(object sender, RoutedEventArgs e) => Close();

    private void OnCloseClick(object sender, RoutedEventArgs e) => Close();
}
