using System.Text.Encodings.Web;
using System.Text.Json;
using System.Text.Json.Serialization;
using HOLManager.Lobbies;

namespace HOLManager.Matches;

/**
 * match.json, FHOLMatchConfig version 2 (Source/HeroesOfLiberty/Match/HOLMatchConfig.h) as FJsonObjectConverter
 * writes it: first letter in lower case (bool fields keep their "b"), enums by name, the map as an object path.
 */
public sealed class MatchConfig
{
    public const int CurrentVersion = 2;

    [JsonPropertyName("version")] public int Version { get; init; } = CurrentVersion;
    [JsonPropertyName("seed")] public int Seed { get; init; }
    [JsonPropertyName("map")] public string Map { get; init; } = "";
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

    /** The configuration of a lobby being started: its players by slot, open slots left empty become closed. */
    public static MatchConfig FromLobby(Lobby lobby, MapInfo map, int seed) => new()
    {
        Seed = seed,
        Map = map.ObjectPath,
        Slots = lobby.Slots.Select(slot => slot.Player is { } p
            ? new MatchSlot { State = MatchSlot.Human, Faction = p.Faction, ColorIndex = p.ColorIndex, Team = p.Team, PlayerName = p.Name }
            // Closed slots get the colour of their index, as FHOLMatchConfig::Normalize gives them.
            : new MatchSlot { ColorIndex = slot.Index % LobbyRules.ColorCount }).ToList(),
        Nomad = lobby.Settings.Nomad,
        Regicide = lobby.Settings.Regicide,
        Catastrophe = lobby.Settings.Catastrophe,
    };
}

public sealed class MatchSlot
{
    public const string Closed = "Closed";
    public const string Human = "Human";

    [JsonPropertyName("state")] public string State { get; init; } = Closed;
    [JsonPropertyName("faction")] public string Faction { get; init; } = LobbyRules.DefaultFaction;
    [JsonPropertyName("colorIndex")] public int ColorIndex { get; init; }
    [JsonPropertyName("team")] public int Team { get; init; }

    /** Version 2 (R3.1, FHOLMatchSlot::PlayerName): the expected player; "" for a closed slot or an AI. */
    [JsonPropertyName("playerName")] public string PlayerName { get; init; } = "";
}
