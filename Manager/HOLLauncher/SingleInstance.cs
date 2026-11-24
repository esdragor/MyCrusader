using System.Runtime.InteropServices;
using System.Security.Cryptography;
using System.Text;

namespace HOLLauncher;

/**
 * One launcher per game folder (L23). With the launcher hidden in the notification area, a second start (the desktop
 * shortcut, the Start menu) must bring the running one to the front instead of opening a second launcher that would
 * update the same files.
 * One named kernel event per game folder (Local\: this Windows session) is both the lock and the signal: the first start
 * creates it and keeps it open while it runs (Claim), a later start finds it, sets it (Signal) and exits; the first one
 * waits on it in the thread pool (Listen) and shows its window. The game folder is the one this start resolved
 * (--game-dir, else the settings', else the launcher's folder), its form and case ignored.
 * The second start's own options are not forwarded (ShowsFirst): --update / --update-launcher are ignored (the window
 * it brings to the front offers them on its big button), and a second --tray start (the Run key at logon while the
 * launcher already runs) does nothing at all, not even show the window.
 * The launcher's own update closes this one and starts the new exe: the claim is given back first (Release), so that the
 * new launcher, started while this one is still closing, becomes the first.
 */
public sealed class SingleInstance : IDisposable
{
    private const string Prefix = @"Local\HeroesOfLibertyLauncher.";

    private EventWaitHandle? handle;
    private RegisteredWaitHandle? registration;
    private Action? listener;

    private SingleInstance(EventWaitHandle handle) => this.handle = handle;

    public string Name { get; private init; } = "";

    /** The kernel object's name for a game folder (same folder, same name, whatever its form). */
    public static string NameFor(string gameDir)
    {
        var normal = Path.TrimEndingDirectorySeparator(Path.GetFullPath(gameDir)).ToUpperInvariant();
        var hash = SHA256.HashData(Encoding.UTF8.GetBytes(normal));
        return Prefix + Convert.ToHexString(hash, 0, 12);
    }

    /** Whether a second start asks the first to show its window (false: it just exits). */
    public static bool ShowsFirst(LauncherOptions options) => !options.StartInTray;

    /** This start becomes the first one for name; null when another launcher already holds it. */
    public static SingleInstance? Claim(string name)
    {
        var created = new EventWaitHandle(false, EventResetMode.AutoReset, name, out var createdNew);
        if (createdNew)
        {
            return new SingleInstance(created) { Name = name };
        }
        created.Dispose();
        return null;
    }

    /**
     * Asks the first launcher to show its window; false when none holds name any more (it closed meanwhile: the caller
     * claims it again). The first launcher may then take the foreground (Windows lets the foreground process give it).
     */
    public static bool Signal(string name)
    {
        if (!EventWaitHandle.TryOpenExisting(name, out var existing))
        {
            return false;
        }
        using (existing)
        {
            AllowSetForegroundWindow(AsfwAny);
            existing.Set();
        }
        return true;
    }

    /** onSignal runs on a thread-pool thread at each Signal (the caller goes back to its own thread). */
    public void Listen(Action onSignal)
    {
        listener = onSignal;
        if (handle is null || registration is not null)
        {
            return;
        }
        registration = ThreadPool.RegisterWaitForSingleObject(handle, (_, _) => onSignal(), null, Timeout.Infinite,
            executeOnlyOnce: false);
    }

    /** Gives the claim back (the launcher's own update, before the new exe starts); Dispose does it too. */
    public void Release()
    {
        registration?.Unregister(null);
        registration = null;
        handle?.Dispose();
        handle = null;
    }

    /**
     * Claims the name again after Release (the new launcher could not be started), listening as before; false when
     * another launcher took it meanwhile.
     */
    public bool Reclaim()
    {
        if (handle is not null)
        {
            return true;
        }
        var created = new EventWaitHandle(false, EventResetMode.AutoReset, Name, out var createdNew);
        if (!createdNew)
        {
            created.Dispose();
            return false;
        }
        handle = created;
        if (listener is { } onSignal)
        {
            Listen(onSignal);
        }
        return true;
    }

    public void Dispose() => Release();

    private const int AsfwAny = -1;

    [DllImport("user32.dll")]
    [return: MarshalAs(UnmanagedType.Bool)]
    private static extern bool AllowSetForegroundWindow(int processId);
}
