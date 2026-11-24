using System.Diagnostics;

namespace HOLLauncher;

/** What the launcher does once the game started: closes, or stays (minimized) and watches it until it exits. */
public enum AfterPlay
{
    Close,
    MinimizeAndWatch,
}

/**
 * The game started from "Play" (launcher 5), pure parts tested. By default the launcher no longer closes: it minimizes,
 * its big button says "Running" until the game exits, then it comes back, looks for a crash of that run (CrashDetector,
 * from the start time: offered at once instead of at the next start) and reads the status again. closeOnPlay (the
 * settings) brings back the old behaviour (L8: close).
 * The packaged game's root HeroesOfLiberty.exe is Unreal's bootstrap: it starts
 * HeroesOfLiberty\Binaries\Win64\HeroesOfLiberty-Win64-Shipping.exe and waits for it, so watching the process
 * GameLauncher.Start returns is enough; IsGameRunning is still asked after its exit in case the bootstrap ever returns
 * first (and covers a game started twice).
 */
public static class GameSession
{
    private const string Project = "HeroesOfLiberty";

    public static AfterPlay Decide(bool? closeOnPlay) => closeOnPlay == true ? AfterPlay.Close : AfterPlay.MinimizeAndWatch;

    /**
     * A process of the game client: the bootstrap (HeroesOfLiberty) or a client binary (HeroesOfLiberty-Win64-Shipping,
     * -Development...). Not a dedicated server (HeroesOfLibertyServer...): the host may run one on the same PC.
     */
    public static bool IsGameProcessName(string processName)
        => string.Equals(processName, Project, StringComparison.OrdinalIgnoreCase)
            || processName.StartsWith(Project + "-Win64-", StringComparison.OrdinalIgnoreCase);

    /** Whether any game client process runs on this PC. */
    public static bool IsGameRunning()
    {
        var running = false;
        foreach (var process in Process.GetProcesses())
        {
            try
            {
                running |= IsGameProcessName(process.ProcessName);
            }
            catch (InvalidOperationException)
            {
                // Exited while listed.
            }
            finally
            {
                process.Dispose();
            }
        }
        return running;
    }

    /**
     * Waits for the started process to exit, then while stillRunning says a game process is left, every poll. Ends
     * early (OperationCanceledException) when cancel fires (the launcher closes).
     */
    public static async Task WaitForExitAsync(Process game, Func<bool> stillRunning, TimeSpan poll, CancellationToken cancel = default)
    {
        await game.WaitForExitAsync(cancel);
        while (stillRunning())
        {
            await Task.Delay(poll, cancel);
        }
    }
}
