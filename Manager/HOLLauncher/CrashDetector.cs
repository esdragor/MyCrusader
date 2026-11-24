using System.Diagnostics;
using System.Text;

namespace HOLLauncher;

/** CrashDetector.Check: whether the game seems to have crashed since the last check, and what showed it (for tests). */
public sealed record CrashCheck(bool Crashed, string? Evidence = null);

/**
 * Finds out at the launcher's start whether the game crashed since the launcher last looked (L20), from what Unreal
 * already writes (no code in the game, no C++):
 * - a crash folder, <Saved>\Crashes\UECC-Windows-..., written by the crashing game (CrashContext.runtime-xml, the
 *   minidump, a copy of the log). An ensure writes one too while the game goes on (<CrashType>Ensure, seen in the
 *   editor's Saved\Crashes): not a crash; a stall likewise. A folder without a readable context counts as a crash.
 * - the last run's log, <Saved>\Logs\HeroesOfLiberty.log (GameLogs: Unreal renames the previous one to
 *   HeroesOfLiberty-backup-<date>.log at start), whose last line is not "Log file closed, <date>", the line Unreal
 *   writes last on a clean exit (seen in real logs: "[2026.10.13-13.43.34:731][175]Log file closed, 10/13/26
 *   15:43:34"; a crash ends on the callstack). This also covers a crash that wrote no folder (the package has no
 *   CrashReportClient) and a game killed from the Task Manager (reported as a crash: the player can say No). Not
 *   looked at while the game runs (its log is still open).
 * Only what was written after lastCheckUtc counts; the first start (no time stored) flags nothing: older crashes are
 * not the player's news. Never throws: a file that cannot be read is not a crash.
 */
public static class CrashDetector
{
    public const string CleanExitLine = "Log file closed";
    public const string LogFileName = "HeroesOfLiberty.log";
    private const string Project = "HeroesOfLiberty";
    private const int TailBytes = 4096;

    public static CrashCheck Check(string gameDir, DateTime? lastCheckUtc, bool gameRunning)
        => Check(gameDir, lastCheckUtc, gameRunning, Environment.GetFolderPath(Environment.SpecialFolder.LocalApplicationData));

    /** localAppData: where a Shipping package keeps its Saved folder (GameLogs); tests give their own. */
    public static CrashCheck Check(string gameDir, DateTime? lastCheckUtc, bool gameRunning, string localAppData)
    {
        if (lastCheckUtc is not { } since)
        {
            return new CrashCheck(false);
        }
        try
        {
            if (CrashesDir(gameDir, localAppData) is { } crashes)
            {
                foreach (var folder in new DirectoryInfo(crashes).EnumerateDirectories())
                {
                    if (folder.LastWriteTimeUtc > since && IsCrash(folder.FullName))
                    {
                        return new CrashCheck(true, folder.FullName);
                    }
                }
            }
            if (!gameRunning && GameLogs.FindDir(gameDir, localAppData) is { } logs)
            {
                var log = new FileInfo(Path.Combine(logs, LogFileName));
                if (log.Exists && log.LastWriteTimeUtc > since && LastLine(log.FullName) is { } last
                    && !last.Contains(CleanExitLine, StringComparison.Ordinal))
                {
                    return new CrashCheck(true, log.FullName);
                }
            }
        }
        catch (Exception e) when (e is IOException or UnauthorizedAccessException or System.Security.SecurityException)
        {
            // Unreadable: nothing to offer, the launcher starts as usual.
        }
        return new CrashCheck(false);
    }

    /**
     * The game's crash folders' parent: <game>\HeroesOfLiberty\Saved\Crashes (Development package), else the same
     * under localAppData (Shipping, like GameLogs); null when neither exists. Looked for apart from the logs: a
     * Shipping game writes no log but still its crashes.
     */
    public static string? CrashesDir(string gameDir, string localAppData)
        => new[] { Path.Combine(gameDir, Project, "Saved", "Crashes"), Path.Combine(localAppData, Project, "Saved", "Crashes") }
            .FirstOrDefault(Directory.Exists);

    /** The game's process (the root HeroesOfLiberty.exe or the one it starts, same name) is running. */
    public static bool GameIsRunning()
    {
        var processes = Process.GetProcessesByName(Path.GetFileNameWithoutExtension(GameLauncher.ExeName));
        foreach (var process in processes)
        {
            process.Dispose();
        }
        return processes.Length > 0;
    }

    private static bool IsCrash(string folder)
    {
        var context = Path.Combine(folder, "CrashContext.runtime-xml");
        if (!File.Exists(context))
        {
            return true;
        }
        try
        {
            var text = File.ReadAllText(context);
            return !text.Contains("<CrashType>Ensure</CrashType>", StringComparison.Ordinal)
                && !text.Contains("<CrashType>Stall</CrashType>", StringComparison.Ordinal)
                && !text.Contains("<IsEnsure>true</IsEnsure>", StringComparison.Ordinal);
        }
        catch (Exception e) when (e is IOException or UnauthorizedAccessException)
        {
            return true;
        }
    }

    /** The log's last non-blank line, read from its end only (a log may weigh tens of MB); null for an empty log. */
    private static string? LastLine(string file)
    {
        using var stream = new FileStream(file, FileMode.Open, FileAccess.Read, FileShare.ReadWrite | FileShare.Delete);
        var length = (int)Math.Min(stream.Length, TailBytes);
        stream.Seek(-length, SeekOrigin.End);
        var bytes = new byte[length];
        stream.ReadExactly(bytes);
        var lines = Encoding.UTF8.GetString(bytes).Split('\n').Select(l => l.Trim()).Where(l => l != "").ToList();
        return lines.Count == 0 ? null : lines[^1];
    }
}
