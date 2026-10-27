using System.Text;

namespace HOLManager.Lobbies;

/**
 * TC3 (TC-Q3, TC-Q4): a named size of a map and its places; the map's file at this size is /Game/Maps/<Name>_<Key>.
 * Mirrors EHOLMapSize and FHOLMapSizeRules (Source/HeroesOfLiberty/Map/HOLMapSizeRules.h) and Tools/Data/MapSizes.csv.
 */
public sealed record MapSize(string Key, int Places);

/**
 * A map a lobby can play, with its number of start positions; its level is /Game/Maps/<Name>. With Sizes (TC3), the
 * map is a family: one file per size, the size sets the places; without, one file of MaxPlayers places (tests).
 */
public sealed record MapInfo(string Name, int MaxPlayers)
{
    /** TC3: the sizes of the map, smallest first; empty for a map without sizes. */
    public IReadOnlyList<MapSize> Sizes { get; init; } = [];

    /** Package path: the first argument of the server's command line. */
    public string PackagePath => $"/Game/Maps/{Name}";

    /** Object path (FSoftObjectPath): the map of match.json (the family's, the size being its own field). */
    public string ObjectPath => $"{PackagePath}.{Name}";

    /** The size of a key (case ignored), null if the map has no such size. */
    public MapSize? FindSize(string? key) => Sizes.FirstOrDefault(s => string.Equals(s.Key, key, StringComparison.OrdinalIgnoreCase));

    /** The places of the map at a size; MaxPlayers without sizes (or for an unknown size). */
    public int Places(string? size) => FindSize(size)?.Places ?? MaxPlayers;

    /** The file the server opens at a size (FHOLMapSizeRules::GetMapFile): Plain at Small is Plain_Small; the map itself without sizes. */
    public MapInfo File(string? size) => FindSize(size) is { } found ? new MapInfo($"{Name}_{found.Key}", found.Places) : this;
}

/** Values of the lobby rules (Manager/PROTOCOL.md, open choices): change them here only. */
public static class LobbyRules
{
    public const int SlotCount = 8;
    public const int ColorCount = 8;

    /**
     * E2-2: "Aléatoire", the colour index after the palette. Never taken (several players can choose it); each one
     * gets at launch a real colour nobody else has (MatchConfig.FromLobby).
     */
    public const int RandomColor = ColorCount;
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

    /** TC1 / TC3: the five named sizes (TC-Q5, TC-Q8), as Tools/Data/MapSizes.csv gives them (checked by a test). */
    public static readonly IReadOnlyList<MapSize> MapSizes =
        [new("Small", 2), new("Medium", 4), new("Normal", 6), new("Large", 8), new("Huge", 8)];

    /** TC3: the size of a new lobby, with its 8 places open to the players who come (the leader then chooses). */
    public const string DefaultSize = "Large";

    /** TC3 (TC-Q6): "Équipes ensemble", ticked in a new lobby. */
    public const bool DefaultTeamsTogether = true;

    /**
     * Maps served; the first one is the default of a new lobby. TC-Q11: Plaine in its five sizes; /Game/Maps/Plain itself,
     * the test map, is not offered.
     */
    public static readonly IReadOnlyList<MapInfo> DefaultMaps = [new("Plain", SlotCount) { Sizes = MapSizes }];

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
