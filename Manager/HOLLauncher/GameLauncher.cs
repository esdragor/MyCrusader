using System.ComponentModel;
using System.Diagnostics;

namespace HOLLauncher;

public sealed record LaunchResult(bool Success, string Message);

/**
 * Starts the game (L8): <game dir>\HeroesOfLiberty.exe, the exe of the package's Windows\ folder (Tools\Package), with
 * the game folder as working folder. No argument by default: the window settings and the player's name are the game's
 * own (saved by the game; -PlayerName= of the playtest .bat only serves two players on one PC). The game then reads
 * servers.json at FPaths::RootDir(), the folder of this exe: the same file GameServers reads.
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

    /** Starts the game without waiting for it; never throws (a missing exe or a refused start is the message). */
    public static LaunchResult Start(string gameDir, IReadOnlyList<string>? arguments = null)
    {
        var exe = FindExe(gameDir);
        if (exe is null)
        {
            return new LaunchResult(false, $"Le jeu est introuvable : {Path.Combine(gameDir, ExeName)} n'existe pas.");
        }
        var start = new ProcessStartInfo(exe) { UseShellExecute = false, WorkingDirectory = gameDir };
        foreach (var argument in arguments ?? [])
        {
            start.ArgumentList.Add(argument);
        }
        try
        {
            Process.Start(start)?.Dispose();
            return new LaunchResult(true, "Jeu lancé.");
        }
        catch (Exception e) when (e is Win32Exception or IOException or InvalidOperationException)
        {
            return new LaunchResult(false, $"Le jeu n'a pas pu être lancé : {e.Message}");
        }
    }
}
