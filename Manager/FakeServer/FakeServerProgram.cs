using System.Runtime.InteropServices;

namespace HOLManager.FakeServer;

/**
 * Stand-in for HeroesOfLibertyServer.exe in the manager's tests: writes its process id (fake.pid) and its raw command
 * line (commandline.txt) next to the match.json of -MatchConfig=, then waits (60 s at most) until killed. A
 * fake_lifetime_ms.txt file in the folder above the match's folder shortens the wait (a server that stops by itself).
 */
public static class FakeServerProgram
{
    public const string CommandLineFile = "commandline.txt";
    public const string PidFile = "fake.pid";
    public const string LifetimeFile = "fake_lifetime_ms.txt";
    private const int MaxLifetimeMs = 60_000;

    [DllImport("kernel32.dll", CharSet = CharSet.Unicode)]
    private static extern IntPtr GetCommandLineW();

    public static int Main(string[] args)
    {
        var config = args.FirstOrDefault(a => a.StartsWith("-MatchConfig=", StringComparison.OrdinalIgnoreCase));
        if (config == null)
        {
            return 1;
        }
        var matchDir = Path.GetDirectoryName(config["-MatchConfig=".Length..]) ?? ".";
        File.WriteAllText(Path.Combine(matchDir, PidFile), Environment.ProcessId.ToString());
        // The raw command line (Environment.CommandLine may be rebuilt from the parsed arguments), written last and
        // whole (renamed into place): the tests wait for it.
        var temp = Path.Combine(matchDir, CommandLineFile + ".tmp");
        File.WriteAllText(temp, Marshal.PtrToStringUni(GetCommandLineW()));
        File.Move(temp, Path.Combine(matchDir, CommandLineFile));

        var lifetime = MaxLifetimeMs;
        var lifetimeFile = Path.Combine(Path.GetDirectoryName(matchDir) ?? ".", LifetimeFile);
        if (File.Exists(lifetimeFile) && int.TryParse(File.ReadAllText(lifetimeFile).Trim(), out var ms))
        {
            lifetime = Math.Clamp(ms, 0, MaxLifetimeMs);
        }
        Thread.Sleep(lifetime);
        return 0;
    }
}
