using System.Text;

namespace HOLManager.Lobbies;

/** A map a lobby can play, with its number of start positions; its level is /Game/Maps/<Name>. */
public sealed record MapInfo(string Name, int MaxPlayers)
{
    /** Package path: the first argument of the server's command line. */
    public string PackagePath => $"/Game/Maps/{Name}";

    /** Object path (FSoftObjectPath): the map of match.json. */
    public string ObjectPath => $"{PackagePath}.{Name}";
}

/** Values of the lobby rules (Manager/PROTOCOL.md, open choices): change them here only. */
public static class LobbyRules
{
    public const int SlotCount = 8;
    public const int ColorCount = 8;
    public const int MaxTeam = 4;
    public const int PlayerNameMaxLength = 16;
    public const int LobbyNameMaxLength = 24;
    public const string DefaultFaction = "DawnOrder";
    public const int MinPlayersToStart = 2;

    /** At the lobby, a player whose token has not called for this long is removed. */
    public static readonly TimeSpan SilenceLimit = TimeSpan.FromSeconds(5);

    /** A notice of the lobby state is cleared after this. */
    public static readonly TimeSpan NoticeDuration = TimeSpan.FromSeconds(10);

    /** A server that has not called "listening" this long after its launch is stopped (server_stopped). */
    public static readonly TimeSpan ListenTimeout = TimeSpan.FromSeconds(120);

    /** After a match, its players keep their line this long without reading (choice 6, R2.11). */
    public static readonly TimeSpan ReturnReservation = TimeSpan.FromMinutes(2);

    /**
     * M6-Q14: the away leader still leads this long after leaving the finished match's server, or after it stopped
     * ("Retour au salon" leaves the server before the join).
     */
    public static readonly TimeSpan LeaderReturnGrace = TimeSpan.FromSeconds(30);

    /** A server still running this long after "empty" is stopped. */
    public static readonly TimeSpan EmptyStopDelay = TimeSpan.FromSeconds(10);

    /** Maps served; the first one is the default of a new lobby. */
    public static readonly IReadOnlyList<MapInfo> DefaultMaps = [new("Plain", SlotCount)];

    /** Player and lobby names are compared without case. */
    public static readonly StringComparer NameComparer = StringComparer.OrdinalIgnoreCase;

    /** Names are checked and kept trimmed and in composed form (é = one character). */
    public static string Clean(string name) => name.Trim().Normalize(NormalizationForm.FormC);

    public static bool IsValidPlayerName(string name) => IsValidName(name, PlayerNameMaxLength, allowApostrophe: false);

    public static bool IsValidLobbyName(string name) => IsValidName(name, LobbyNameMaxLength, allowApostrophe: true);

    private static bool IsValidName(string name, int maxLength, bool allowApostrophe)
    {
        var clean = Clean(name);
        return clean.Length >= 1 && clean.Length <= maxLength
            && clean.All(c => char.IsLetterOrDigit(c) || c == ' ' || c == '-' || c == '_' || (allowApostrophe && c == '\''));
    }
}
