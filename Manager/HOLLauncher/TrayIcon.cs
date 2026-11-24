using System.Runtime.InteropServices;
using System.Windows;
using System.Windows.Media;
using System.Windows.Media.Imaging;
using Forms = System.Windows.Forms;

namespace HOLLauncher;

/**
 * The launcher's icon in the notification area (L23), shown while the launcher runs and removed when it closes
 * (Dispose: Windows would otherwise keep a dead icon until the mouse passes over it). WinForms' NotifyIcon does the
 * Shell_NotifyIcon work (the icon comes back after Explorer restarts, the notifications); the menu itself is the
 * window's WPF ContextMenu in the launcher's theme (MenuRequested), not WinForms'.
 * A left click or a double click asks for the window (OpenRequested), a right click for the menu. Notify shows a
 * notification (a toast on Windows 10 / 11); a click on it runs its action.
 * The picture is a placeholder (there is no logo yet, Placeholders.txt): the title bar's shield emblem drawn in code,
 * red with a gold star, at the size Windows uses for small icons.
 */
public sealed class TrayIcon : IDisposable
{
    /** The placeholder's colours: the accent red of the light theme and the logo's gold (Palette). */
    private static readonly Color ShieldColor = Media(Palette.Light.AccentBack);
    private static readonly Color StarColor = Media(Palette.Gold);

    private readonly Forms.NotifyIcon icon;
    private readonly System.Drawing.Icon picture;
    private Action? notificationClick;
    private bool disposed;

    public TrayIcon(Geometry emblem, string tooltip)
    {
        picture = ToIcon(Draw(emblem, Forms.SystemInformation.SmallIconSize.Width));
        icon = new Forms.NotifyIcon { Icon = picture, Text = Cut(tooltip) };
        icon.MouseClick += (_, e) =>
        {
            if (e.Button == Forms.MouseButtons.Left)
            {
                OpenRequested?.Invoke();
            }
            else if (e.Button == Forms.MouseButtons.Right)
            {
                MenuRequested?.Invoke();
            }
        };
        icon.MouseDoubleClick += (_, e) =>
        {
            if (e.Button == Forms.MouseButtons.Left)
            {
                OpenRequested?.Invoke();
            }
        };
        icon.BalloonTipClicked += (_, _) =>
        {
            var action = notificationClick;
            notificationClick = null;
            action?.Invoke();
        };
        icon.BalloonTipClosed += (_, _) => notificationClick = null;
        icon.Visible = true;
    }

    public event Action? OpenRequested;

    public event Action? MenuRequested;

    /** The text shown when the mouse rests on the icon (TrayMenu.Tooltip already cut it; cut again for safety). */
    public string Tooltip
    {
        set
        {
            var text = Cut(value);
            if (!disposed && icon.Text != text)
            {
                icon.Text = text;
            }
        }
    }

    /** A notification from the icon; onClick runs if the player clicks it (null: nothing). */
    public void Notify(string title, string text, Action? onClick)
    {
        if (disposed)
        {
            return;
        }
        notificationClick = onClick;
        icon.ShowBalloonTip(10000, title, text, Forms.ToolTipIcon.None);
    }

    public void Dispose()
    {
        if (disposed)
        {
            return;
        }
        disposed = true;
        icon.Visible = false;
        icon.Dispose();
        picture.Dispose();
    }

    /** The placeholder picture, size x size pixels (also rendered by the visual check). */
    public static BitmapSource Draw(Geometry emblem, int size)
    {
        // The emblem (Styles.xaml, 16 x 16) is the shield with the star cut out: drawn as its two figures.
        var path = PathGeometry.CreateFromGeometry(emblem);
        var shield = new PathGeometry([path.Figures[0].Clone()]);
        var star = new PathGeometry(path.Figures.Skip(1).Select(f => f.Clone()));
        var visual = new DrawingVisual();
        using (var context = visual.RenderOpen())
        {
            context.PushTransform(new ScaleTransform(size / 16.0, size / 16.0));
            context.DrawGeometry(new SolidColorBrush(ShieldColor), new Pen(new SolidColorBrush(StarColor), 1.0), shield);
            context.DrawGeometry(new SolidColorBrush(StarColor), null, star);
        }
        var bitmap = new RenderTargetBitmap(size, size, 96, 96, PixelFormats.Pbgra32);
        bitmap.Render(visual);
        bitmap.Freeze();
        return bitmap;
    }

    /** A Windows icon of the picture (straight alpha, as GDI+ wants it), owning its own handle. */
    private static System.Drawing.Icon ToIcon(BitmapSource source)
    {
        var straight = new FormatConvertedBitmap(source, PixelFormats.Bgra32, null, 0);
        var (width, height) = (straight.PixelWidth, straight.PixelHeight);
        var pixels = new byte[width * height * 4];
        straight.CopyPixels(pixels, width * 4, 0);
        using var bitmap = new System.Drawing.Bitmap(width, height, System.Drawing.Imaging.PixelFormat.Format32bppArgb);
        var data = bitmap.LockBits(new System.Drawing.Rectangle(0, 0, width, height), System.Drawing.Imaging.ImageLockMode.WriteOnly,
            System.Drawing.Imaging.PixelFormat.Format32bppArgb);
        try
        {
            for (var row = 0; row < height; row++)
            {
                Marshal.Copy(pixels, row * width * 4, data.Scan0 + row * data.Stride, width * 4);
            }
        }
        finally
        {
            bitmap.UnlockBits(data);
        }
        var handle = bitmap.GetHicon();
        try
        {
            // FromHandle does not own the handle; the clone does (CopyImage), so the original is destroyed here.
            using var borrowed = System.Drawing.Icon.FromHandle(handle);
            return (System.Drawing.Icon)borrowed.Clone();
        }
        finally
        {
            DestroyIcon(handle);
        }
    }

    private static Color Media(System.Drawing.Color c) => Color.FromRgb(c.R, c.G, c.B);

    private static string Cut(string text) => text.Length <= TrayMenu.MaxTooltip ? text : text[..TrayMenu.MaxTooltip];

    [DllImport("user32.dll")]
    [return: MarshalAs(UnmanagedType.Bool)]
    private static extern bool DestroyIcon(IntPtr handle);
}
