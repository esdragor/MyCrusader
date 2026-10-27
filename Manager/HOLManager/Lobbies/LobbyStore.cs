using System.Security.Cryptography;
using HOLManager.Matches;

namespace HOLManager.Lobbies;

/**
 * The lobbies and their rules, without HTTP (Manager/PROTOCOL.md). Every public call is atomic; a refused call
 * throws LobbyException and changes nothing.
 */
public sealed class LobbyStore(string gameVersion, IClock clock, IReadOnlyList<MapInfo>? maps = null,
    IServerLauncher? launcher = null)
{
    private readonly object gate = new();
    private readonly Dictionary<string, Lobby> lobbies = [];

    /** Matches whose server is followed (R2.10), until it stops; a finished match stays here until then. */
    private readonly Dictionary<string, (Lobby Lobby, LobbyMatch Match)> matches = [];
    private readonly IReadOnlyList<MapInfo> maps = maps ?? LobbyRules.DefaultMaps;
    private int nextLobbyId = 1;
    private long nextJoinOrder = 1;
    private int nextMatchId = 1;

    public string GameVersion => gameVersion;

    public IReadOnlyList<MapInfo> Maps => maps;

    public DateTimeOffset Now => clock.UtcNow;

    public int Count
    {
        get
        {
            lock (gate)
            {
                Sweep();
                return lobbies.Count;
            }
        }
    }

    /** GET /lobbies: the lobbies in order of creation (live objects: read them under Atomic). */
    public IReadOnlyList<Lobby> List()
    {
        lock (gate)
        {
            Sweep();
            return lobbies.Values.OrderBy(l => int.Parse(l.Id[1..])).ToList();
        }
    }

    public Lobby? Find(string lobbyId)
    {
        lock (gate)
        {
            Sweep();
            return lobbies.GetValueOrDefault(lobbyId);
        }
    }

    /** POST /lobbies: creates the lobby and enters it, the creator being the leader. */
    public JoinResult Create(string? lobbyName, string? playerName, string? clientVersion)
    {
        lock (gate)
        {
            Sweep();
            if (lobbyName == null)
            {
                throw new LobbyException(LobbyErrors.BadRequest, "Missing field: name.");
            }
            CheckEntry(playerName, clientVersion);
            if (!LobbyRules.IsValidLobbyName(lobbyName))
            {
                throw new LobbyException(LobbyErrors.InvalidName, $"Invalid lobby name: '{lobbyName}'.");
            }
            var lobby = new Lobby
            {
                Id = $"L{nextLobbyId++}",
                Name = LobbyRules.Clean(lobbyName),
                Settings = new LobbySettings { Map = maps[0].Name, Size = DefaultSize(maps[0]) },
            };
            lobbies.Add(lobby.Id, lobby);
            return Enter(lobby, LobbyRules.Clean(playerName!));
        }
    }

    /** POST /lobbies/{id}/join. */
    public JoinResult Join(string lobbyId, string? playerName, string? clientVersion)
    {
        lock (gate)
        {
            Sweep();
            var lobby = Get(lobbyId);
            CheckEntry(playerName, clientVersion);
            var name = LobbyRules.Clean(playerName!);
            if (lobby.State != LobbyState.Lobby)
            {
                throw new LobbyException(LobbyErrors.LobbyInGame, "The lobby is in a match.");
            }
            var held = lobby.Players.FirstOrDefault(p => LobbyRules.NameComparer.Equals(p.Name, name));
            if (held?.ReservedUntil > clock.UtcNow)
            {
                // Back from the match by name (R2.11): same line, new token; the old one is no longer valid.
                held.Token = NewToken();
                held.LastSeen = clock.UtcNow;
                MarkBack(lobby, held);
                return new JoinResult(lobby, held.Token, held.SlotIndex);
            }
            if (!lobby.Slots.Any(s => s.State == SlotState.Open))
            {
                throw new LobbyException(LobbyErrors.LobbyFull, "The lobby is full.");
            }
            if (held != null)
            {
                throw new LobbyException(LobbyErrors.NameTaken, $"The name '{name}' is already in the lobby.");
            }
            var leaderBack = lobby.AwayLeader != null && LobbyRules.NameComparer.Equals(lobby.AwayLeader, name)
                && !(lobby.AwayLeaderUntil <= clock.UtcNow);
            var entered = Enter(lobby, name);
            if (leaderBack)
            {
                // M6-Q14: the match's leader takes the lead again, ahead of those back before him.
                var leader = lobby.Players.Single(p => p.Token == entered.Token);
                leader.JoinOrder = lobby.Players.Min(p => p.JoinOrder) - 1;
                ClearAwayLeader(lobby);
            }
            else if (lobby.AwayLeader != null
                && (lobby.AwayLeaderUntil <= clock.UtcNow || !lobby.Slots.Any(s => s.State == SlotState.Open)))
            {
                // Gone while nobody was back, or the lobby is full without him: the first back leads.
                GiveUpAwayLeader(lobby);
            }
            return entered;
        }
    }

    /** PUT /lobbies/{id}/me: only the given fields change. */
    public Lobby UpdateMe(string lobbyId, string? token, int? colorIndex = null, int? team = null, bool? ready = null)
    {
        lock (gate)
        {
            var (lobby, me) = GetPlayer(lobbyId, token);
            if (colorIndex is < 0 or > LobbyRules.RandomColor || team is < 0 or > LobbyRules.MaxTeam)
            {
                throw new LobbyException(LobbyErrors.BadRequest,
                    $"Color is 0 to {LobbyRules.RandomColor} (random), team 0 to {LobbyRules.MaxTeam}.");
            }
            // E2-2: "Aléatoire" is never taken.
            if (colorIndex is { } color && color != LobbyRules.RandomColor && lobby.Players.Any(p => p != me && p.ColorIndex == color))
            {
                throw new LobbyException(LobbyErrors.ColorTaken, $"Color {color} is taken.");
            }
            var lineChanged = (colorIndex ?? me.ColorIndex) != me.ColorIndex || (team ?? me.Team) != me.Team;
            me.ColorIndex = colorIndex ?? me.ColorIndex;
            me.Team = team ?? me.Team;
            me.Ready = ready ?? (lineChanged ? false : me.Ready);
            return lobby;
        }
    }

    /** PUT /lobbies/{id}/settings (leader): only the given fields change; a change resets every "Ready". */
    public Lobby UpdateSettings(string lobbyId, string? token, string? map = null, bool? nomad = null, bool? regicide = null,
        bool? catastrophe = null, string? size = null, bool? teamsTogether = null)
    {
        lock (gate)
        {
            var lobby = GetLeaderLobby(lobbyId, token);
            var settings = lobby.Settings;
            var mapInfo = maps.FirstOrDefault(m => m.Name == (map ?? settings.Map))
                ?? throw new LobbyException(LobbyErrors.BadRequest, $"Unknown map: '{map}'.");
            // TC3: the size asked, else the current one if the map has it, else the map's default.
            var newSize = size != null
                ? mapInfo.FindSize(size)?.Key ?? throw new LobbyException(LobbyErrors.BadRequest, $"Unknown size: '{size}'.")
                : mapInfo.FindSize(settings.Size)?.Key ?? DefaultSize(mapInfo);
            var oldPlaces = Places(lobby);
            var places = mapInfo.Places(newSize);
            var extraSlots = lobby.Slots.Skip(places).ToList();
            var moving = extraSlots.Where(s => s.Player != null).ToList();
            // A map without sizes refuses a player past its places; a size (TC-Q4) moves him below, unless the players
            // are more than its places (such a size is greyed on the screen).
            if (moving.Count > 0 && (mapInfo.Sizes.Count == 0 || lobby.Players.Count() > places))
            {
                throw new LobbyException(LobbyErrors.SlotTaken, $"Map {mapInfo.Name} {newSize} has {places} places; a player holds a slot beyond.");
            }
            var sizeChanged = newSize != settings.Size;
            var changed = mapInfo.Name != settings.Map || sizeChanged || (nomad ?? settings.Nomad) != settings.Nomad
                || (regicide ?? settings.Regicide) != settings.Regicide || (catastrophe ?? settings.Catastrophe) != settings.Catastrophe
                || (teamsTogether ?? settings.TeamsTogether) != settings.TeamsTogether || extraSlots.Any(s => !s.Closed);
            settings.Map = mapInfo.Name;
            settings.Size = newSize;
            settings.Nomad = nomad ?? settings.Nomad;
            settings.Regicide = regicide ?? settings.Regicide;
            settings.Catastrophe = catastrophe ?? settings.Catastrophe;
            settings.TeamsTogether = teamsTogether ?? settings.TeamsTogether;
            // TC3 (FHOLMapSizeRules::FitSlotsToSize): each player past the places onto the first free slot below, in order.
            foreach (var from in moving)
            {
                var to = lobby.Slots.Take(places).First(s => s.Player == null);
                to.Player = from.Player;
                to.Closed = false;
                to.Player!.SlotIndex = to.Index;
                from.Player = null;
            }
            extraSlots.ForEach(s => s.Closed = true);
            // A bigger size opens the slots it adds (closed by the smaller one); a bigger map without sizes does not.
            if (sizeChanged && mapInfo.Sizes.Count > 0 && places > oldPlaces)
            {
                lobby.Slots.Skip(oldPlaces).Take(places - oldPlaces).ToList().ForEach(s => s.Closed = false);
            }
            if (changed)
            {
                ResetReady(lobby);
            }
            return lobby;
        }
    }

    /** PUT /lobbies/{id}/slots/{index} (leader): a change resets every "Ready". */
    public Lobby SetSlot(string lobbyId, string? token, int index, bool? open)
    {
        lock (gate)
        {
            var lobby = GetLeaderLobby(lobbyId, token);
            if (index < 0 || index >= LobbyRules.SlotCount || open == null)
            {
                throw new LobbyException(LobbyErrors.BadRequest,
                    $"Slot index is 0 to {LobbyRules.SlotCount - 1}, state \"open\" or \"closed\".");
            }
            var mapMax = Places(lobby);
            if (open == true && index >= mapMax)
            {
                throw new LobbyException(LobbyErrors.BadRequest, $"Map {lobby.Settings.Map} {lobby.Settings.Size} has {mapMax} slots.");
            }
            var slot = lobby.Slots[index];
            if (slot.Player != null)
            {
                throw new LobbyException(LobbyErrors.SlotTaken, $"Slot {index} is held by {slot.Player.Name}.");
            }
            if (slot.Closed == open)
            {
                slot.Closed = !open.Value;
                ResetReady(lobby);
            }
            return lobby;
        }
    }

    /**
     * POST /lobbies/{id}/start (leader): every player ready, MinPlayersToStart at least; writes the match
     * configuration, starts its server (when a launcher is given) and puts the lobby in the starting state.
     */
    public LobbyMatch Start(string lobbyId, string? token)
    {
        lock (gate)
        {
            var lobby = GetLeaderLobby(lobbyId, token);
            if (lobby.Players.Count() < LobbyRules.MinPlayersToStart)
            {
                throw new LobbyException(LobbyErrors.NotEnoughPlayers, $"A match needs {LobbyRules.MinPlayersToStart} players.");
            }
            if (lobby.Players.FirstOrDefault(p => !p.Ready) is { } notReady)
            {
                throw new LobbyException(LobbyErrors.NotAllReady, $"{notReady.Name} is not ready.");
            }
            var map = maps.First(m => m.Name == lobby.Settings.Map);
            var config = MatchConfig.FromLobby(lobby, map, Random.Shared.Next());
            var matchId = $"M{clock.UtcNow:yyyyMMdd-HHmmss}-{nextMatchId++}";
            // The launcher throws max_servers or server_start_failed: the lobby is then left as it was. TC3: the server
            // opens the file of the size (Plain_Small), match.json names the map and its size.
            var port = launcher?.Launch(matchId, map.File(lobby.Settings.Size), config) ?? 0;
            lobby.Match = new LobbyMatch(matchId, config, port, clock.UtcNow);
            lobby.State = LobbyState.Starting;
            matches.Add(matchId, (lobby, lobby.Match));
            return lobby.Match;
        }
    }

    /** GET /lobbies/{id}: with a token, the reading counts as presence; without one, it does not. */
    public Lobby Read(string lobbyId, string? token)
    {
        lock (gate)
        {
            Sweep();
            var lobby = Get(lobbyId);
            if (token != null)
            {
                FindPlayer(lobby, token);
            }
            return lobby;
        }
    }

    /** POST /lobbies/{id}/leave: allowed in any state; an empty lobby is deleted only at the lobby. */
    public void Leave(string lobbyId, string? token)
    {
        lock (gate)
        {
            Sweep();
            var lobby = Get(lobbyId);
            Remove(lobby, [FindPlayer(lobby, token)]);
        }
    }

    /** POST /matches/{id}/listening: the lobby goes in_game; port 0 or absent keeps the launch port. */
    public LobbyMatch ServerListening(string matchId, int? port) => OnMatch(matchId, (lobby, match) =>
    {
        // Repeated, or after "finished": changes nothing.
        if (lobby.State == LobbyState.Starting && lobby.Match == match)
        {
            match.Port = port is > 0 ? port.Value : match.Port;
            match.Listening = true;
            lobby.State = LobbyState.InGame;
        }
    });

    /** POST /matches/{id}/started: the minutes start (the first call only). */
    public LobbyMatch ServerStarted(string matchId) => OnMatch(matchId, (_, match) => match.StartedAt ??= clock.UtcNow);

    /**
     * POST /matches/{id}/finished (D2): the lobby goes back to the lobby with its settings, without the match's
     * players (each comes back by a plain join, the first back leads); kept, even empty, while its server runs.
     */
    public LobbyMatch ServerFinished(string matchId, IReadOnlyList<int>? winners) => OnMatch(matchId, (lobby, match) =>
    {
        if (!match.Finished)
        {
            match.Finished = true;
            match.Winners = winners ?? [];
            // M6-Q14: the leader leads while away, unless he already left the match's server.
            lobby.AwayLeader = lobby.Leader?.Name;
            lobby.AwayLeaderUntil = lobby.AwayLeader != null && match.LeftBeforeEnd.Contains(lobby.AwayLeader) ? clock.UtcNow : null;
            lobby.LastMatchColors = lobby.Players.ToDictionary(p => p.Name, p => p.ColorIndex, LobbyRules.NameComparer);
            lobby.State = LobbyState.Lobby;
            lobby.Match = null;
            foreach (var slot in lobby.Slots)
            {
                slot.Player = null;
            }
        }
    });

    /** POST /matches/{id}/empty: the server is stopped if it still runs EmptyStopDelay later. */
    public LobbyMatch ServerEmpty(string matchId) => OnMatch(matchId, (_, match) => match.EmptyAt ??= clock.UtcNow);

    /**
     * POST /matches/{id}/left (M6-Q14): a named player left the match's server. During the match, remembered for
     * "finished"; after it, the away leader leads LeaderReturnGrace longer. Any other name changes nothing.
     */
    public LobbyMatch ServerLeft(string matchId, string? playerName)
    {
        if (string.IsNullOrWhiteSpace(playerName))
        {
            throw new LobbyException(LobbyErrors.BadRequest, "playerName is required.");
        }
        var name = LobbyRules.Clean(playerName);
        return OnMatch(matchId, (lobby, match) =>
        {
            if (lobby.Match == match)
            {
                match.LeftBeforeEnd.Add(name);
                // R7.6: under his name in the match (the list shows it), a name absent from it is nobody.
                if (match.Config.Slots.FirstOrDefault(slot => LobbyRules.NameComparer.Equals(slot.PlayerName, name)) is { } slot)
                {
                    match.AwaySince[slot.PlayerName] = clock.UtcNow;
                }
            }
            else if (match.Finished && LobbyRules.NameComparer.Equals(lobby.AwayLeader, name))
            {
                lobby.AwayLeaderUntil ??= clock.UtcNow + LobbyRules.LeaderReturnGrace;
            }
        });
    }

    /**
     * POST /matches/{id}/back (M6-Q15): a named player came back on the match's server within R7's window, so he counts
     * as present. During the match, no longer remembered as gone for "finished"; after it, the away leader is back to
     * leading while away (no LeaderReturnGrace count, even one already over). Any other name changes nothing.
     */
    public LobbyMatch ServerBack(string matchId, string? playerName)
    {
        if (string.IsNullOrWhiteSpace(playerName))
        {
            throw new LobbyException(LobbyErrors.BadRequest, "playerName is required.");
        }
        var name = LobbyRules.Clean(playerName);
        return OnMatch(matchId, (lobby, match) =>
        {
            if (lobby.Match == match)
            {
                match.LeftBeforeEnd.Remove(name);
                match.AwaySince.Remove(name);
            }
            else if (match.Finished && LobbyRules.NameComparer.Equals(lobby.AwayLeader, name))
            {
                lobby.AwayLeaderUntil = null;
            }
        });
    }

    private LobbyMatch OnMatch(string matchId, Action<Lobby, LobbyMatch> action)
    {
        lock (gate)
        {
            Sweep();
            var (lobby, match) = matches.TryGetValue(matchId, out var followed)
                ? followed
                : throw new LobbyException(LobbyErrors.MatchNotFound, $"No match {matchId}.");
            action(lobby, match);
            return match;
        }
    }

    /** Housekeeping alone (the manager's timer, every second): see Sweep. */
    public void Housekeep()
    {
        lock (gate)
        {
            Sweep();
        }
    }

    /** Runs under the store's lock, so that a route builds its answer from a consistent state (calls nest). */
    public T Atomic<T>(Func<T> action)
    {
        lock (gate)
        {
            return action();
        }
    }

    /**
     * Housekeeping, run at the start of every public call (no background timer: whoever looks at the lobbies
     * calls the store first): removes the players silent for SilenceLimit at the lobby, clears old notices.
     */
    private void Sweep()
    {
        var now = clock.UtcNow;
        FollowServers(now);
        foreach (var lobby in lobbies.Values.ToList())
        {
            if (lobby.Notice != null && now >= lobby.NoticeUntil)
            {
                lobby.Notice = null;
            }
            if (lobby.State == LobbyState.Lobby)
            {
                Remove(lobby, lobby.Players.Where(p => now - p.LastSeen >= LobbyRules.SilenceLimit && !(p.ReservedUntil > now)).ToList());
                if (lobby.AwayLeaderUntil <= now && lobby.Players.Any())
                {
                    GiveUpAwayLeader(lobby);
                }
            }
        }
    }

    /**
     * R2.10, part of the housekeeping: a server gone, silent ListenTimeout after its launch, or still there
     * EmptyStopDelay after "empty" is forgotten (stopped first if it runs); its lobby, if still in the match, goes
     * back to the lobby with server_stopped. Without a launcher (store tests), servers never stop by themselves.
     */
    private void FollowServers(DateTimeOffset now)
    {
        foreach (var (matchId, (lobby, match)) in matches.ToList())
        {
            var tooLate = (!match.Listening && !match.Finished && now - match.LaunchedAt >= LobbyRules.ListenTimeout)
                || (match.EmptyAt is { } empty && now - empty >= LobbyRules.EmptyStopDelay);
            var exited = launcher?.HasExited(matchId) ?? false;
            if (!exited && !tooLate)
            {
                continue;
            }
            if (!exited)
            {
                launcher?.Stop(matchId);
            }
            matches.Remove(matchId);
            if (lobby.Match == match)
            {
                ReturnToLobby(lobby, match, LobbyNotices.ServerStopped);
            }
            else if (lobby.Leader == null && lobby.State == LobbyState.Lobby)
            {
                // A finished match's lobby nobody came back to (D2).
                lobbies.Remove(lobby.Id);
            }
            else if (lobby.AwayLeader != null)
            {
                // M6-Q14: its leader, not back, leads LeaderReturnGrace longer.
                lobby.AwayLeaderUntil ??= now + LobbyRules.LeaderReturnGrace;
            }
        }
    }

    /**
     * The lobby leaves its match (server stopped; "finished" empties it instead, D2): "Ready" reset; each player keeps his line, is not
     * removed by silence and keeps his name for ReturnReservation (R2.11), and is not back until his next call or
     * join by name. A lobby left by everyone during the match is deleted.
     */
    private void ReturnToLobby(Lobby lobby, LobbyMatch match, string? notice)
    {
        lobby.State = LobbyState.Lobby;
        lobby.Match = null;
        var now = clock.UtcNow;
        foreach (var player in lobby.Players)
        {
            player.Ready = false;
            player.LastSeen = now;
            player.ReservedUntil = now + LobbyRules.ReturnReservation;
            player.Back = false;
        }
        if (notice != null)
        {
            lobby.Notice = notice;
            lobby.NoticeUntil = now + LobbyRules.NoticeDuration;
        }
        if (lobby.Leader == null)
        {
            lobbies.Remove(lobby.Id);
        }
    }

    private void Remove(Lobby lobby, IReadOnlyCollection<LobbyPlayer> players)
    {
        if (players.Count == 0)
        {
            return;
        }
        var leader = lobby.Leader;
        foreach (var player in players)
        {
            lobby.Slots[player.SlotIndex].Player = null;
        }
        if (lobby.Leader == null)
        {
            // A finished match's lobby stays while its server runs (D2): its players may still come back.
            if (lobby.State == LobbyState.Lobby && !matches.Values.Any(followed => followed.Lobby == lobby))
            {
                lobbies.Remove(lobby.Id);
            }
        }
        else if (lobby.Leader != leader && lobby.AwayLeader == null)
        {
            lobby.Notice = LobbyNotices.LeaderChanged;
            lobby.NoticeUntil = clock.UtcNow + LobbyRules.NoticeDuration;
        }
    }

    /** M6-Q14: the away leader leads no more; the first back leads (leader_changed when others are there). */
    private void GiveUpAwayLeader(Lobby lobby)
    {
        ClearAwayLeader(lobby);
        if (lobby.Players.Count() > 1)
        {
            lobby.Notice = LobbyNotices.LeaderChanged;
            lobby.NoticeUntil = clock.UtcNow + LobbyRules.NoticeDuration;
        }
    }

    private static void ClearAwayLeader(Lobby lobby)
    {
        lobby.AwayLeader = null;
        lobby.AwayLeaderUntil = null;
    }

    private Lobby Get(string lobbyId) =>
        lobbies.GetValueOrDefault(lobbyId) ?? throw new LobbyException(LobbyErrors.LobbyNotFound, $"No lobby {lobbyId}.");

    /** The token's player; any call with a valid token counts as presence. */
    private LobbyPlayer FindPlayer(Lobby lobby, string? token)
    {
        var player = lobby.Players.FirstOrDefault(p => p.Token == token)
            ?? throw new LobbyException(LobbyErrors.NotInLobby, "Not in the lobby.");
        player.LastSeen = clock.UtcNow;
        MarkBack(lobby, player);
        return player;
    }

    /**
     * A player's first call after the match: the match's leader (earliest arrival) keeps his order, the others
     * arrive anew, so that the first back leads while the old leader is away (R2.11).
     */
    private void MarkBack(Lobby lobby, LobbyPlayer player)
    {
        if (player.Back)
        {
            return;
        }
        if (player != lobby.Players.First())
        {
            player.JoinOrder = nextJoinOrder++;
        }
        player.Back = true;
    }

    private static string NewToken() => Convert.ToHexString(RandomNumberGenerator.GetBytes(16)).ToLowerInvariant();

    private (Lobby, LobbyPlayer) GetPlayer(string lobbyId, string? token)
    {
        Sweep();
        var lobby = Get(lobbyId);
        var player = FindPlayer(lobby, token);
        if (lobby.State != LobbyState.Lobby)
        {
            throw new LobbyException(LobbyErrors.LobbyInGame, "The lobby is in a match.");
        }
        return (lobby, player);
    }

    private Lobby GetLeaderLobby(string lobbyId, string? token)
    {
        var (lobby, player) = GetPlayer(lobbyId, token);
        // M6-Q14: while the match's leader is away, nobody else sets or starts.
        if (player != lobby.Leader || lobby.AwayLeader != null)
        {
            throw new LobbyException(LobbyErrors.NotLeader, "Only the leader changes this.");
        }
        return lobby;
    }

    /** Order of the entry checks once the lobby is found: bad_request, version_mismatch, invalid_name. */
    private void CheckEntry(string? playerName, string? clientVersion)
    {
        if (playerName == null || clientVersion == null)
        {
            throw new LobbyException(LobbyErrors.BadRequest, "Missing field: playerName or gameVersion.");
        }
        if (clientVersion != gameVersion)
        {
            throw new LobbyException(LobbyErrors.VersionMismatch, $"Game version {clientVersion}, served {gameVersion}.");
        }
        if (!LobbyRules.IsValidPlayerName(playerName))
        {
            throw new LobbyException(LobbyErrors.InvalidName, $"Invalid player name: '{playerName}'.");
        }
    }

    private JoinResult Enter(Lobby lobby, string name)
    {
        var slot = lobby.Slots.First(s => s.State == SlotState.Open);
        var player = new LobbyPlayer
        {
            Name = name,
            Token = NewToken(),
            SlotIndex = slot.Index,
            // E2-1: back from the last match, his color again if free (E2-2: "Aléatoire" always is); else the first free one.
            ColorIndex = lobby.LastMatchColors.TryGetValue(name, out var color)
                    && (color == LobbyRules.RandomColor || lobby.Players.All(p => p.ColorIndex != color))
                ? color
                : Enumerable.Range(0, LobbyRules.ColorCount).First(c => lobby.Players.All(p => p.ColorIndex != c)),
            JoinOrder = nextJoinOrder++,
            LastSeen = clock.UtcNow,
        };
        slot.Player = player;
        return new JoinResult(lobby, player.Token, slot.Index);
    }

    /** TC3: the size of a new lobby on a map (DefaultSize, else its biggest); "" for a map without sizes. */
    private static string DefaultSize(MapInfo map) =>
        (map.FindSize(LobbyRules.DefaultSize) ?? map.Sizes.LastOrDefault())?.Key ?? "";

    /** The places of the lobby's map at its size. */
    private int Places(Lobby lobby) => maps.First(m => m.Name == lobby.Settings.Map).Places(lobby.Settings.Size);

    private static void ResetReady(Lobby lobby)
    {
        foreach (var player in lobby.Players)
        {
            player.Ready = false;
        }
    }
}
