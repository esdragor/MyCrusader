using System.ComponentModel;
using System.Diagnostics;

namespace HOLLauncher;

public sealed record LaunchResult(bool Success, string Message);

/**
 * Starts the game (L8): <game dir>\HeroesOfLiberty.exe, the exe of the package's Windows\ folder (Tools\Package), with
 * the game folder as working folder. No argument by default: the window settings and the player's name are the game's
 * own (saved by the game; -PlayerName= of the playtest .bat only serves two players on one PC). The game then reads
 * servers.json at FPaths::RootDir(), the folder of this exe: the same file GameServers reads.
 * The player's launch options (L19) are the arguments, built by LaunchArguments.For: Unreal tokens passed as they are
 * (ProcessStartInfo.Arguments, never ArgumentList, which would re-quote -Key="a b" into a token Unreal cannot read).
 */
public static class GameLauncher
{
    public const string ExeName = "HeroesOfLiberty.exe";

    /** <gameDir>\HeroesOfLiberty.exe if it exists, else null. */
    public static string? FindExe(string gameDir)
    {
        var exe = Path.Combine(gameDir, ExeName);
        return File.Exists(exe) ? exe : null;
    }

    /**
     * Starts the game without waiting for it; never throws (a missing exe or a refused start is the message, L16: in the
     * given language, null: the one in use). starter replaces Process.Start (tests: what the game would be given).
     */
    public static LaunchResult Start(string gameDir, IReadOnlyList<string>? arguments = null,
        Func<ProcessStartInfo, Process?>? starter = null, string? language = null)
    {
        var lang = Texts.LanguageOr(language);
        var exe = FindExe(gameDir);
        if (exe is null)
        {
            return new LaunchResult(false, Texts.FormatIn(lang, "GameNotFound", Path.Combine(gameDir, ExeName)));
        }
        var start = new ProcessStartInfo(exe)
        {
            UseShellExecute = false,
            WorkingDirectory = gameDir,
            Arguments = LaunchArguments.CommandLine(arguments ?? []),
        };
        try
        {
            (starter ?? Process.Start)(start)?.Dispose();
            return new LaunchResult(true, Texts.Get("GameStarted", lang));
        }
        catch (Exception e) when (e is Win32Exception or IOException or InvalidOperationException)
        {
            return new LaunchResult(false, Texts.FormatIn(lang, "GameStartFailed", e.Message));
        }
    }
}
