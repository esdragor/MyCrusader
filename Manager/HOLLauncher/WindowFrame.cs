using System.Runtime.InteropServices;
using System.Windows;
using System.Windows.Input;
using System.Windows.Interop;
using System.Windows.Media;

namespace HOLLauncher;

/**
 * The launcher's own window frame (launcher 5, WindowChrome: no Windows title bar). A window without Windows' frame that
 * is maximized would spill a few pixels off the screen on every side and over the taskbar: WM_GETMINMAXINFO is
 * answered with the monitor's work area instead, so a maximized launcher fills exactly the screen minus the taskbar.
 * Also the commands of the title bar's buttons (minimize, maximize / restore, close).
 */
public static class WindowFrame
{
    private const int WmGetMinMaxInfo = 0x0024;
    private const uint MonitorDefaultToNearest = 2;

    /** Hooks the window's messages once its handle exists. */
    public static void Attach(Window window)
    {
        window.SourceInitialized += (_, _) =>
        {
            if (PresentationSource.FromVisual(window) is HwndSource source)
            {
                source.AddHook((IntPtr hwnd, int msg, IntPtr wParam, IntPtr lParam, ref bool handled) =>
                {
                    if (msg == WmGetMinMaxInfo)
                    {
                        FitToWorkArea(window, hwnd, lParam);
                    }
                    return IntPtr.Zero;
                });
            }
        };
    }

    public static void Minimize(Window window) => SystemCommands.MinimizeWindow(window);

    public static void ToggleMaximize(Window window)
    {
        if (window.WindowState == WindowState.Maximized)
        {
            SystemCommands.RestoreWindow(window);
        }
        else
        {
            SystemCommands.MaximizeWindow(window);
        }
    }

    public static void Close(Window window) => SystemCommands.CloseWindow(window);

    /** A double click on the title bar maximizes or restores, as Windows' own title bar does. */
    public static void OnTitleBarMouseDown(Window window, MouseButtonEventArgs e)
    {
        if (e.ClickCount == 2 && window.ResizeMode is ResizeMode.CanResize or ResizeMode.CanResizeWithGrip)
        {
            ToggleMaximize(window);
            e.Handled = true;
        }
    }

    /**
     * Keeps the window's own minimum size (in device pixels) and sets the maximized size and position to the work area of
     * the monitor the window is on.
     */
    private static void FitToWorkArea(Window window, IntPtr hwnd, IntPtr lParam)
    {
        var info = Marshal.PtrToStructure<MinMaxInfo>(lParam);
        var monitor = MonitorFromWindow(hwnd, MonitorDefaultToNearest);
        var monitorInfo = new MonitorInfo { Size = Marshal.SizeOf<MonitorInfo>() };
        if (monitor != IntPtr.Zero && GetMonitorInfo(monitor, ref monitorInfo))
        {
            var work = monitorInfo.Work;
            var whole = monitorInfo.Monitor;
            info.MaxPosition = new Point32(work.Left - whole.Left, work.Top - whole.Top);
            info.MaxSize = new Point32(work.Right - work.Left, work.Bottom - work.Top);
            var dpi = VisualTreeHelper.GetDpi(window);
            info.MinTrackSize = new Point32((int)Math.Ceiling(window.MinWidth * dpi.DpiScaleX),
                (int)Math.Ceiling(window.MinHeight * dpi.DpiScaleY));
            Marshal.StructureToPtr(info, lParam, false);
        }
    }

    [StructLayout(LayoutKind.Sequential)]
    private struct Point32(int x, int y)
    {
        public int X = x;
        public int Y = y;
    }

    [StructLayout(LayoutKind.Sequential)]
    private struct MinMaxInfo
    {
        public Point32 Reserved;
        public Point32 MaxSize;
        public Point32 MaxPosition;
        public Point32 MinTrackSize;
        public Point32 MaxTrackSize;
    }

    [StructLayout(LayoutKind.Sequential)]
    private struct Rect32
    {
        public int Left;
        public int Top;
        public int Right;
        public int Bottom;
    }

    [StructLayout(LayoutKind.Sequential)]
    private struct MonitorInfo
    {
        public int Size;
        public Rect32 Monitor;
        public Rect32 Work;
        public uint Flags;
    }

    [DllImport("user32.dll")]
    private static extern IntPtr MonitorFromWindow(IntPtr hwnd, uint flags);

    [DllImport("user32.dll", CharSet = CharSet.Unicode)]
    [return: MarshalAs(UnmanagedType.Bool)]
    private static extern bool GetMonitorInfo(IntPtr monitor, ref MonitorInfo info);
}
