using System.Diagnostics;
using System.Net;
using System.Net.Sockets;
using HOLManager.FakeServer;
using HOLManager.Lobbies;
using HOLManager.Matches;

namespace HOLManager.Tests;

/** R2.9: the dedicated server started for a match, here the fake server (Manager/FakeServer). */
public sealed class ProcessServerLauncherTests : IDisposable
{
    /** High ports, away from real game servers (7777+) and from LobbyRoutesTests. */
    private const int BasePort = 47777;

    private static readonly MapInfo Plain = LobbyRules.DefaultMaps[0];

    /** A folder with a space in its path: the quotes of the command line are tested on it. */
    private readonly string matchesDir = Path.Combine(Path.GetTempPath(), $"HOL launcher {Guid.NewGuid():N}");
    private readonly List<ProcessServerLauncher> launchers = [];

    internal static string FakeExe => Path.Combine(AppContext.BaseDirectory, "FakeServer.exe");

    public void Dispose()
    {
        launchers.ForEach(l => l.Dispose());
        DeleteDir(matchesDir);
    }

    /** Deletes a test folder, retried while a file in it is still locked for a moment (10 s at most). */
    internal static void DeleteDir(string dir)
    {
        var watch = Stopwatch.StartNew();
        while (Directory.Exists(dir))
        {
            try
            {
                Directory.Delete(dir, recursive: true);
            }
            catch (Exception e) when (e is IOException or UnauthorizedAccessException && watch.Elapsed < TimeSpan.FromSeconds(10))
            {
                Thread.Sleep(50);
            }
        }
    }

    private ProcessServerLauncher Launcher(int maxServers = 2, string? exe = null)
    {
        var launcher = new ProcessServerLauncher(new ManagerSettings
        {
            ServerExe = exe ?? FakeExe,
            MatchesDir = matchesDir,
            FirstServerPort = BasePort,
            MaxServers = maxServers,
            ManagerUrl = "http://127.0.0.1:7700",
        });
        launchers.Add(launcher);
        return launcher;
    }

    private static MatchConfig Config(int seed = 184467) => new()
    {
        Seed = seed,
        Map = Plain.ObjectPath,
        Slots = Enumerable.Range(0, LobbyRules.SlotCount).Select(i => new MatchSlot { ColorIndex = i }).ToList(),
    };

    /**
     * Waits for a file the fake server writes and reads it (10 s at most). A new file can be briefly locked by
     * another process (antivirus, indexer): the read is retried until the deadline.
     */
    internal static string WaitForFile(string path)
    {
        var watch = Stopwatch.StartNew();
        while (true)
        {
            try
            {
                return File.ReadAllText(path);
            }
            catch (IOException) when (watch.Elapsed < TimeSpan.FromSeconds(10))
            {
                // Not written yet, or locked for a moment.
            }
            Thread.Sleep(20);
        }
    }

    /** The fake server's process id, once its command line is written (10 s at most). */
    internal static int WaitForPid(string matchDir)
    {
        WaitForFile(Path.Combine(matchDir, FakeServerProgram.CommandLineFile));
        return int.Parse(WaitForFile(Path.Combine(matchDir, FakeServerProgram.PidFile)));
    }

    private string MatchDir(string matchId) => Path.Combine(matchesDir, matchId);

    [Fact]
    public void ArgumentsAreTheOnesOfTheProtocol()
    {
        var args = ProcessServerLauncher.BuildArguments(Plain, 7778, @"C:\a b\Saved\Matches\M20261023-153012-3",
            "http://127.0.0.1:7700", "M20261023-153012-3");
        Assert.Equal("/Game/Maps/Plain -Port=7778 -MatchConfig=\"C:\\a b\\Saved\\Matches\\M20261023-153012-3\\match.json\""
            + " -ManagerUrl=http://127.0.0.1:7700 -MatchId=M20261023-153012-3 -log -unattended -useperfthreads"
            + " -abslog=\"C:\\a b\\Saved\\Matches\\M20261023-153012-3\\server.log\"", args);
    }

    [Fact]
    public void LaunchStartsTheServerWithTheseArguments()
    {
        var launcher = Launcher();
        var port = launcher.Launch("M1", Plain, Config());
        var commandLine = WaitForFile(Path.Combine(MatchDir("M1"), FakeServerProgram.CommandLineFile));
        Assert.Contains(' ', matchesDir);
        Assert.Equal($"\"{FakeExe}\" " + ProcessServerLauncher.BuildArguments(Plain, port, MatchDir("M1"),
            "http://127.0.0.1:7700", "M1"), commandLine);
        Assert.Contains($"-MatchConfig=\"{MatchDir("M1")}\\match.json\"", commandLine);
        Assert.Equal(1, launcher.Running);
    }

    [Fact]
    public void LaunchWritesTheMatchJsonOfTheLobby()
    {
        var config = Config(seed: 42);
        Launcher().Launch("M1", Plain, config);
        Assert.Equal(config.ToJson(), WaitForFile(Path.Combine(MatchDir("M1"), "match.json")));
    }

    [Fact]
    public void PortsBusyInUdpOrGivenToARunningServerAreSkipped()
    {
        using var busy = new UdpClient(new IPEndPoint(IPAddress.Any, BasePort));
        var launcher = Launcher();
        Assert.Equal(BasePort + 1, launcher.Launch("M1", Plain, Config()));
        // The fake server does not listen: only the launcher knows the port is given.
        Assert.Equal(BasePort + 2, launcher.Launch("M2", Plain, Config()));
    }

    [Fact]
    public void MaxServersRefusesAnotherOne()
    {
        var launcher = Launcher(maxServers: 1);
        launcher.Launch("M1", Plain, Config());
        LobbyEntryTests.AssertError(LobbyErrors.MaxServers, () => launcher.Launch("M2", Plain, Config()));
        Assert.Equal(1, launcher.Running);
        Assert.False(Directory.Exists(MatchDir("M2")));
    }

    [Fact]
    public void MissingExeIsServerStartFailed()
    {
        var launcher = Launcher(exe: Path.Combine(matchesDir, "nope", "HeroesOfLibertyServer.exe"));
        LobbyEntryTests.AssertError(LobbyErrors.ServerStartFailed, () => launcher.Launch("M1", Plain, Config()));
        Assert.Equal(0, launcher.Running);
    }

    [Fact]
    public void DisposeStopsTheServers()
    {
        var launcher = Launcher();
        launcher.Launch("M1", Plain, Config());
        launcher.Launch("M2", Plain, Config());
        var pids = new[] { "M1", "M2" }.Select(id => WaitForPid(MatchDir(id))).ToList();
        launcher.Dispose();
        foreach (var pid in pids)
        {
            Assert.False(IsAlive(pid), $"Fake server {pid} still runs.");
        }
        Assert.Equal(0, launcher.Running);
    }

    [Fact]
    public void AServerThatStopsLeavesRunning()
    {
        Directory.CreateDirectory(matchesDir);
        File.WriteAllText(Path.Combine(matchesDir, FakeServerProgram.LifetimeFile), "0");
        var launcher = Launcher();
        launcher.Launch("M1", Plain, Config());
        var watch = Stopwatch.StartNew();
        while (launcher.Running > 0)
        {
            Assert.True(watch.Elapsed < TimeSpan.FromSeconds(10), "The stopped server is still counted.");
            Thread.Sleep(20);
        }
    }

    [Fact]
    public void HasExitedFollowsTheProcess()
    {
        var launcher = Launcher();
        launcher.Launch("M1", Plain, Config());
        Assert.False(launcher.HasExited("M1"));
        Assert.True(launcher.HasExited("M99"));
        var pid = WaitForPid(MatchDir("M1"));
        launcher.Stop("M1");
        Assert.False(IsAlive(pid), $"Fake server {pid} still runs.");
        Assert.True(launcher.HasExited("M1"));
        Assert.Equal(0, launcher.Running);
        launcher.Stop("M1");
        launcher.Stop("M99");
    }

    [Fact]
    public void HasExitedWhenTheServerStopsByItself()
    {
        Directory.CreateDirectory(matchesDir);
        File.WriteAllText(Path.Combine(matchesDir, FakeServerProgram.LifetimeFile), "0");
        var launcher = Launcher();
        launcher.Launch("M1", Plain, Config());
        WaitUntil(() => launcher.HasExited("M1"), "The stopped server is not seen.");
    }

    /** R2.10 with the fake server: a server that disappears sends its lobby back with server_stopped. */
    [Fact]
    public void AServerGoneInGameSendsTheLobbyBack()
    {
        Directory.CreateDirectory(matchesDir);
        File.WriteAllText(Path.Combine(matchesDir, FakeServerProgram.LifetimeFile), "1500");
        var store = new LobbyStore(LobbyEntryTests.Version, new SystemClock(), launcher: Launcher());
        var created = store.Create("A", "Bob", LobbyEntryTests.Version);
        var id = created.Lobby.Id;
        var alice = store.Join(id, "Alice", LobbyEntryTests.Version).Token;
        store.UpdateMe(id, created.Token, ready: true);
        store.UpdateMe(id, alice, ready: true);
        var matchId = store.Start(id, created.Token).Id;
        store.ServerListening(matchId, null);
        Assert.Equal(LobbyState.InGame, store.Find(id)!.State);
        WaitUntil(() => store.Atomic(() => store.Find(id)!.State == LobbyState.Lobby), "The lobby did not come back.");
        Assert.Equal(LobbyNotices.ServerStopped, store.Find(id)!.Notice);
        Assert.Equal(2, store.Read(id, alice).Players.Count());
    }

    private static void WaitUntil(Func<bool> condition, string message)
    {
        var watch = Stopwatch.StartNew();
        while (!condition())
        {
            Assert.True(watch.Elapsed < TimeSpan.FromSeconds(10), message);
            Thread.Sleep(20);
        }
    }

    internal static bool IsAlive(int pid)
    {
        try
        {
            using var process = Process.GetProcessById(pid);
            return !process.HasExited;
        }
        catch (ArgumentException)
        {
            return false;
        }
    }
}
