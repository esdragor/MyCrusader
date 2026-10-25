using System.ComponentModel;
using System.Diagnostics;
using System.Net;
using System.Net.Sockets;
using HOLManager.Lobbies;

namespace HOLManager.Matches;

/**
 * R2.9: starts HeroesOfLibertyServer.exe for a match with the command line of Manager/PROTOCOL.md, on the first port
 * free in UDP from FirstServerPort. Dispose stops the servers still running. The store follows each server through
 * HasExited and Stop (R2.10). With an access key (L11), the server gets it in -HOLAccessKey=.
 */
public sealed class ProcessServerLauncher(ManagerSettings settings, string? accessKey = null) : IServerLauncher, IDisposable
{
    /** Ports tried from FirstServerPort before giving up. */
    private const int PortsTried = 100;

    private readonly object gate = new();
    private readonly List<(Process Process, int Port, string MatchId)> servers = [];

    public string MatchesDir { get; } = Path.GetFullPath(settings.MatchesDir, AppContext.BaseDirectory);

    public int Running
    {
        get
        {
            lock (gate)
            {
                Prune();
                return servers.Count;
            }
        }
    }

    public int Launch(string matchId, MapInfo map, MatchConfig config)
    {
        lock (gate)
        {
            Prune();
            if (servers.Count >= settings.MaxServers)
            {
                throw new LobbyException(LobbyErrors.MaxServers, $"{servers.Count} game servers already run (at most {settings.MaxServers}).");
            }
            var exe = Path.GetFullPath(settings.ServerExe, AppContext.BaseDirectory);
            if (!File.Exists(exe))
            {
                throw new LobbyException(LobbyErrors.ServerStartFailed, $"No server executable: {exe}.");
            }
            var port = FreePort();
            var matchDir = Path.Combine(MatchesDir, matchId);
            try
            {
                Directory.CreateDirectory(matchDir);
                File.WriteAllText(Path.Combine(matchDir, "match.json"), config.ToJson());
                var info = new ProcessStartInfo(exe)
                {
                    // Built by hand, not with ArgumentList: the quotes go after the "=".
                    Arguments = BuildArguments(map, port, matchDir, settings.ManagerUrl, matchId, accessKey),
                    UseShellExecute = false,
                    // The server writes to -abslog; no console mixed with the manager's.
                    CreateNoWindow = true,
                    WorkingDirectory = Path.GetDirectoryName(exe)!,
                };
                var process = Process.Start(info) ?? throw new InvalidOperationException("Process.Start gave nothing.");
                servers.Add((process, port, matchId));
                return port;
            }
            catch (Exception e) when (e is Win32Exception or IOException or UnauthorizedAccessException or InvalidOperationException)
            {
                throw new LobbyException(LobbyErrors.ServerStartFailed, $"The server of {matchId} did not start: {e.Message}");
            }
        }
    }

    public bool HasExited(string matchId)
    {
        lock (gate)
        {
            Prune();
            return servers.All(s => s.MatchId != matchId);
        }
    }

    public void Stop(string matchId)
    {
        lock (gate)
        {
            foreach (var server in servers.Where(s => s.MatchId == matchId).ToList())
            {
                Kill(server.Process);
                servers.Remove(server);
            }
        }
    }

    private static void Kill(Process process)
    {
        try
        {
            if (!process.HasExited)
            {
                process.Kill(entireProcessTree: true);
                process.WaitForExit(5000);
            }
        }
        catch (Exception e) when (e is Win32Exception or InvalidOperationException)
        {
            // Already gone.
        }
        process.Dispose();
    }

    /** The arguments after the exe, built by hand: paths are quoted after the "=", as FParse::Value reads them. */
    internal static string BuildArguments(MapInfo map, int port, string matchDir, string managerUrl, string matchId,
        string? accessKey = null) =>
        $"{map.PackagePath} -Port={port} -MatchConfig=\"{Path.Combine(matchDir, "match.json")}\""
        + $" -ManagerUrl={managerUrl} -MatchId={matchId} -log -unattended -useperfthreads"
        + $" -abslog=\"{Path.Combine(matchDir, "server.log")}\""
        + (accessKey != null ? $" -HOLAccessKey={accessKey}" : "");

    /** The first port from FirstServerPort free in UDP (Unreal listens in UDP) and not given to a running server. */
    private int FreePort()
    {
        for (var port = settings.FirstServerPort; port < settings.FirstServerPort + PortsTried; port++)
        {
            if (servers.All(s => s.Port != port) && IsUdpFree(port))
            {
                return port;
            }
        }
        throw new LobbyException(LobbyErrors.ServerStartFailed, $"No free UDP port from {settings.FirstServerPort}.");
    }

    private static bool IsUdpFree(int port)
    {
        try
        {
            using var socket = new Socket(AddressFamily.InterNetwork, SocketType.Dgram, ProtocolType.Udp);
            socket.ExclusiveAddressUse = true;
            socket.Bind(new IPEndPoint(IPAddress.Any, port));
            return true;
        }
        catch (SocketException)
        {
            return false;
        }
    }

    /** A server that stopped is no longer counted (the store sends its lobby back, R2.10). */
    private void Prune()
    {
        foreach (var server in servers.Where(s => s.Process.HasExited).ToList())
        {
            server.Process.Dispose();
            servers.Remove(server);
        }
    }

    public void Dispose()
    {
        lock (gate)
        {
            foreach (var server in servers)
            {
                Kill(server.Process);
            }
            servers.Clear();
        }
    }
}
