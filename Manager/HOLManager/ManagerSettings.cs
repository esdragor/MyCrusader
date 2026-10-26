namespace HOLManager;

/** Settings of the manager, from appsettings.json, the command line (--Key=Value) or the tests. */
public sealed class ManagerSettings
{
    /** Version of the protocol in Manager/PROTOCOL.md. */
    public const int ProtocolVersion = 1;

    /** Game version served; empty = read version.txt next to ServerExe or HeroesOfLiberty\Version.txt below it, "dev" without them. */
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

    /** L11: the key shared with the friends' kit (relative: from the manager's folder); absent or empty = no key. */
    public string AccessKeyFile { get; init; } = "access_key.txt";

    public string ResolveAccessKeyFile() => Path.GetFullPath(AccessKeyFile, AppContext.BaseDirectory);

    public string ResolveGameVersion()
    {
        if (!string.IsNullOrWhiteSpace(GameVersion))
        {
            return GameVersion.Trim();
        }
        var dir = Path.GetDirectoryName(Path.GetFullPath(ServerExe, AppContext.BaseDirectory)) ?? "";
        // A package keeps it in the project folder, where the server reads it (HOLVersion.cpp).
        foreach (var file in new[] { Path.Combine(dir, "version.txt"), Path.Combine(dir, "HeroesOfLiberty", "Version.txt") })
        {
            if (File.Exists(file) && File.ReadAllText(file).Trim() is { Length: > 0 } version)
            {
                return version;
            }
        }
        return "dev";
    }
}
