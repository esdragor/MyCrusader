using System.Text.Encodings.Web;
using System.Text.Json;
using System.Text.Json.Serialization;
using HOLManager.Lobbies;

namespace HOLManager.Matches;

/**
 * match.json, FHOLMatchConfig version 3 (Source/HeroesOfLiberty/Match/HOLMatchConfig.h) as FJsonObjectConverter
 * writes it: first letter in lower case (bool fields keep their "b"), enums by name, the map as an object path.
 */
public sealed class MatchConfig
{
    public const int CurrentVersion = 3;

    [JsonPropertyName("version")] public int Version { get; init; } = CurrentVersion;
    [JsonPropertyName("seed")] public int Seed { get; init; }
    [JsonPropertyName("map")] public string Map { get; init; } = "";

    /** Version 3 (TC3, EHOLMapSize by name): the size, the file played being FHOLMapSizeRules::GetMapFile(map, mapSize). */
    [JsonPropertyName("mapSize")] public string MapSize { get; init; } = "None";

    /** Version 3 (TC-Q6): the places drawn by the server from the seed; always at a lobby (fixed: the test scenarios). */
    [JsonPropertyName("bDrawPlaces")] public bool DrawPlaces { get; init; } = true;

    /** Version 3 (TC-Q6): "Équipes ensemble". */
    [JsonPropertyName("bTeamsTogether")] public bool TeamsTogether { get; init; } = true;
    [JsonPropertyName("slots")] public IReadOnlyList<MatchSlot> Slots { get; init; } = [];
    [JsonPropertyName("mode")] public string Mode { get; init; } = "Standard";
    [JsonPropertyName("bNomad")] public bool Nomad { get; init; }
    [JsonPropertyName("bRegicide")] public bool Regicide { get; init; }
    [JsonPropertyName("bCatastrophe")] public bool Catastrophe { get; init; }

    private static readonly JsonSerializerOptions Options = new()
    {
        WriteIndented = true,
        Encoder = JavaScriptEncoder.UnsafeRelaxedJsonEscaping,
    };

    public string ToJson() => JsonSerializer.Serialize(this, Options);

    /**
     * The configuration of a lobby being started: its players and AIs by slot, open slots left empty become closed. A
     * player or an AI on "Aléatoire" (E2-2) gets a real colour nobody else has, chosen or already drawn, drawn from the
     * seed in slot order. IA0.6 L2: an AI slot is "AI" with its level ("difficulty", FHOLMatchSlot::Difficulty).
     */
    public static MatchConfig FromLobby(Lobby lobby, MapInfo map, int seed)
    {
        var colors = DrawRandomColors(lobby.Slots.Select(slot => slot.ColorIndex).ToList(), new Random(seed));
        return new()
        {
            Seed = seed,
            Map = map.ObjectPath,
            MapSize = map.FindSize(lobby.Settings.Size)?.Key ?? "None",
            DrawPlaces = true,
            TeamsTogether = lobby.Settings.TeamsTogether,
            Slots = lobby.Slots.Select((slot, position) => slot.Player is { } p
                ? new MatchSlot { State = MatchSlot.Human, Faction = p.Faction, ColorIndex = colors[position]!.Value, Team = p.Team, PlayerName = p.Name }
                : slot.AI is { } ai
                ? new MatchSlot { State = MatchSlot.AI, Faction = ai.Faction, ColorIndex = colors[position]!.Value, Team = ai.Team, Difficulty = ai.Level }
                // Closed slots get the colour of their index, as FHOLMatchConfig::Normalize gives them.
                : new MatchSlot { ColorIndex = slot.Index % LobbyRules.ColorCount }).ToList(),
            Nomad = lobby.Settings.Nomad,
            Regicide = lobby.Settings.Regicide,
            Catastrophe = lobby.Settings.Catastrophe,
        };
    }

    /** Per slot, its player's or AI's colour (null: nobody), RandomColor replaced in slot order by a free one. */
    private static List<int?> DrawRandomColors(List<int?> colors, Random random)
    {
        for (var index = 0; index < colors.Count; index++)
        {
            if (colors[index] == LobbyRules.RandomColor)
            {
                var free = Enumerable.Range(0, LobbyRules.ColorCount).Where(c => !colors.Contains(c)).ToList();
                // Eight colours for eight slots: one is always free.
                colors[index] = free[random.Next(free.Count)];
            }
        }
        return colors;
    }
}

public sealed class MatchSlot
{
    public const string Closed = "Closed";
    public const string Human = "Human";
    public const string AI = "AI";

    [JsonPropertyName("state")] public string State { get; init; } = Closed;
    [JsonPropertyName("faction")] public string Faction { get; init; } = LobbyRules.DefaultFaction;
    [JsonPropertyName("colorIndex")] public int ColorIndex { get; init; }
    [JsonPropertyName("team")] public int Team { get; init; }

    /** Version 2 (R3.1, FHOLMatchSlot::PlayerName): the expected player; "" for a closed slot or an AI. */
    [JsonPropertyName("playerName")] public string PlayerName { get; init; } = "";

    /**
     * IA0.6 L2 (FHOLMatchSlot::Difficulty, EHOLAIDifficulty by name): an AI slot's level; not written for a human or
     * closed slot (the game reads a missing one as Normal), so a lobby without AIs writes the file it wrote before.
     */
    [JsonPropertyName("difficulty")]
    [JsonIgnore(Condition = JsonIgnoreCondition.WhenWritingNull)]
    public string? Difficulty { get; init; }
}
