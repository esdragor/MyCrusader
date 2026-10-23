namespace HOLManager;

/** Settings of the manager, from appsettings.json, the command line (--Key=Value) or the tests. */
public sealed class ManagerSettings
{
    /** Version of the protocol in Manager/PROTOCOL.md. */
    public const int ProtocolVersion = 1;

    /** Game version served; empty = read version.txt next to ServerExe, "dev" without it. */
    public string GameVersion { get; init; } = "";

    /** The dedicated server started for each match (relative paths: from the manager's folder). */
    public string ServerExe { get; init; } = @"Server\HeroesOfLibertyServer.exe";

    /** Game servers running at once, at most (M6-Q6; measured in R5.5). */
    public int MaxServers { get; init; } = 2;

    /** Game servers listen on the first free UDP port from this one. */
    public int FirstServerPort { get; init; } = 7777;

    /** match.json and server.log of each match, in <MatchesDir>\<matchId> (relative: from the manager's folder). */
    public string MatchesDir { get; init; } = @"Saved\Matches";

    /** Address the game servers call back (-ManagerUrl=). */
    public string ManagerUrl { get; init; } = "http://127.0.0.1:7700";

    /** Log files manager-<date>.log (relative: from the manager's folder); empty = no log file. */
    public string LogDir { get; init; } = "Logs";

    public string ResolveGameVersion()
    {
        if (!string.IsNullOrWhiteSpace(GameVersion))
        {
            return GameVersion.Trim();
        }
        var exe = Path.GetFullPath(ServerExe, AppContext.BaseDirectory);
        var file = Path.Combine(Path.GetDirectoryName(exe) ?? "", "version.txt");
        return File.Exists(file) && File.ReadAllText(file).Trim() is { Length: > 0 } version ? version : "dev";
    }
}
