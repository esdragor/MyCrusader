using HOLManager.Lobbies;
using HOLManager.Matches;

namespace HOLManager.Tests;

/** A launcher whose servers live until the test stops them. */
internal sealed class FakeLauncher : IServerLauncher
{
    public HashSet<string> Alive { get; } = [];
    public List<string> Stopped { get; } = [];
    public int Running => Alive.Count;

    public List<string?> Versions { get; } = [];

    public int Launch(string matchId, MapInfo map, MatchConfig config, string? gameVersion = null)
    {
        Alive.Add(matchId);
        Versions.Add(gameVersion);
        return 7778;
    }

    public bool HasExited(string matchId) => !Alive.Contains(matchId);

    public void Stop(string matchId)
    {
        Stopped.Add(matchId);
        Alive.Remove(matchId);
    }
}

/** R2.10: following a match's server (routes it calls, stopped server, 120 s without listening, empty). */
public class MatchFollowTests
{
    private const string Version = LobbyEntryTests.Version;
    private readonly FakeClock clock = new();
    private readonly FakeLauncher launcher = new();
    private readonly LobbyStore store;

    public MatchFollowTests() => store = new LobbyStore(Version, clock, launcher: launcher);

    /** Bob (leader) and Alice, both ready, match started: the lobby is starting. */
    private (Lobby Lobby, string Bob, string Alice, string MatchId) Started()
    {
        var created = store.Create("Salon de Bob", "Bob", Version);
        var id = created.Lobby.Id;
        var alice = store.Join(id, "Alice", Version).Token;
        store.UpdateMe(id, created.Token, ready: true);
        store.UpdateMe(id, alice, ready: true);
        var match = store.Start(id, created.Token);
        return (created.Lobby, created.Token, alice, match.Id);
    }

    private void AssertBackWithNotice(Lobby lobby, string? notice)
    {
        Assert.Equal(LobbyState.Lobby, lobby.State);
        Assert.Null(lobby.Match);
        Assert.Equal(notice, lobby.Notice);
        Assert.All(lobby.Players, p => Assert.False(p.Ready));
    }

    [Fact]
    public void ListeningPutsTheLobbyInGame()
    {
        var (lobby, _, _, matchId) = Started();
        store.ServerListening(matchId, 7790);
        Assert.Equal(LobbyState.InGame, lobby.State);
        Assert.Equal(7790, lobby.Match!.Port);
        Assert.Equal(0, lobby.Match.Minutes(clock.UtcNow));
    }

    [Fact]
    public void ListeningWithoutPortKeepsTheLaunchPort()
    {
        var (lobby, _, _, matchId) = Started();
        store.ServerListening(matchId, null);
        Assert.Equal(7778, lobby.Match!.Port);
    }

    [Fact]
    public void UnknownMatchIsMatchNotFound()
    {
        Started();
        LobbyEntryTests.AssertError(LobbyErrors.MatchNotFound, () => store.ServerListening("M99", 7778));
        LobbyEntryTests.AssertError(LobbyErrors.MatchNotFound, () => store.ServerStarted("M99"));
        LobbyEntryTests.AssertError(LobbyErrors.MatchNotFound, () => store.ServerFinished("M99", [0]));
        LobbyEntryTests.AssertError(LobbyErrors.MatchNotFound, () => store.ServerEmpty("M99"));
    }

    [Fact]
    public void MinutesCountFromStarted()
    {
        var (lobby, _, _, matchId) = Started();
        store.ServerListening(matchId, 7778);
        clock.UtcNow += TimeSpan.FromSeconds(50);
        store.ServerStarted(matchId);
        clock.UtcNow += TimeSpan.FromSeconds(179);
        Assert.Equal(2, lobby.Match!.Minutes(clock.UtcNow));
        clock.UtcNow += TimeSpan.FromSeconds(1);
        Assert.Equal(3, lobby.Match.Minutes(clock.UtcNow));
    }

    [Fact]
    public void ServerGoneInGameReturnsToTheLobbyWithTheNotice()
    {
        var (lobby, _, _, matchId) = Started();
        store.ServerListening(matchId, 7778);
        store.ServerStarted(matchId);
        clock.UtcNow += TimeSpan.FromMinutes(20);
        launcher.Alive.Remove(matchId);
        Assert.Same(lobby, store.Find(lobby.Id));
        AssertBackWithNotice(lobby, LobbyNotices.ServerStopped);
        Assert.Equal(["Bob", "Alice"], lobby.Players.Select(p => p.Name));
        LobbyEntryTests.AssertError(LobbyErrors.MatchNotFound, () => store.ServerFinished(matchId, []));
    }

    [Fact]
    public void PlayersBackFromTheMatchAreNotSilent()
    {
        var (lobby, bob, alice, matchId) = Started();
        store.ServerListening(matchId, 7778);
        clock.UtcNow += TimeSpan.FromMinutes(20);
        launcher.Alive.Remove(matchId);
        // The first call after the return keeps everyone; silence removes no one for 2 min (R2.11).
        Assert.Equal(2, store.Read(lobby.Id, bob).Players.Count());
        clock.UtcNow += TimeSpan.FromSeconds(6);
        Assert.Equal(2, store.Read(lobby.Id, bob).Players.Count());
        clock.UtcNow += LobbyRules.ReturnReservation - TimeSpan.FromSeconds(8);
        store.Read(lobby.Id, bob);
        clock.UtcNow += TimeSpan.FromSeconds(2);
        // Alice silent 2 min since the return: removed; Bob read 2 s ago: kept.
        Assert.Equal(["Bob"], store.Read(lobby.Id, bob).Players.Select(p => p.Name));
        _ = alice;
    }

    [Fact]
    public void ServerGoneBeforeListeningReturnsToTheLobbyWithTheNotice()
    {
        var (lobby, _, _, matchId) = Started();
        launcher.Alive.Remove(matchId);
        store.Housekeep();
        AssertBackWithNotice(lobby, LobbyNotices.ServerStopped);
    }

    [Fact]
    public void NoListeningIn120SecondsStopsTheServer()
    {
        var (lobby, _, _, matchId) = Started();
        clock.UtcNow += TimeSpan.FromSeconds(119);
        store.Housekeep();
        Assert.Equal(LobbyState.Starting, lobby.State);
        Assert.Empty(launcher.Stopped);
        clock.UtcNow += TimeSpan.FromSeconds(1);
        store.Housekeep();
        Assert.Equal([matchId], launcher.Stopped);
        AssertBackWithNotice(lobby, LobbyNotices.ServerStopped);
    }

    [Fact]
    public void ListeningInTimeIsNotStopped()
    {
        var (lobby, _, _, matchId) = Started();
        clock.UtcNow += TimeSpan.FromSeconds(100);
        store.ServerListening(matchId, 7778);
        clock.UtcNow += TimeSpan.FromMinutes(30);
        store.Housekeep();
        Assert.Empty(launcher.Stopped);
        Assert.Equal(LobbyState.InGame, lobby.State);
    }

    [Fact]
    public void NoticeIsClearedAfter10Seconds()
    {
        var (lobby, bob, alice, matchId) = Started();
        launcher.Alive.Remove(matchId);
        store.Housekeep();
        clock.UtcNow += TimeSpan.FromSeconds(4);
        store.Read(lobby.Id, bob);
        store.Read(lobby.Id, alice);
        clock.UtcNow += TimeSpan.FromSeconds(4);
        Assert.Equal(LobbyNotices.ServerStopped, store.Read(lobby.Id, bob).Notice);
        store.Read(lobby.Id, alice);
        clock.UtcNow += TimeSpan.FromSeconds(2);
        Assert.Null(store.Read(lobby.Id, bob).Notice);
    }

    [Fact]
    public void FinishedReturnsToTheLobbyWithoutNotice()
    {
        var (lobby, _, alice, matchId) = Started();
        store.ServerListening(matchId, 7778);
        store.ServerStarted(matchId);
        clock.UtcNow += TimeSpan.FromMinutes(25);
        var match = store.ServerFinished(matchId, [1]);
        Assert.Equal([1], match.Winners);
        AssertBackWithNotice(lobby, null);
        // D2: the match's players are out of the lobby; one comes back by joining.
        LobbyEntryTests.AssertError(LobbyErrors.NotInLobby, () => store.Read(lobby.Id, alice));
        var back = store.Join(lobby.Id, "Alice", Version).Token;
        // The server quitting afterwards gives no notice.
        launcher.Alive.Remove(matchId);
        store.Read(lobby.Id, back);
        AssertBackWithNotice(lobby, null);
        Assert.Empty(launcher.Stopped);
        LobbyEntryTests.AssertError(LobbyErrors.MatchNotFound, () => store.ServerEmpty(matchId));
    }

    [Fact]
    public void ALingeringServerIsStopped10SecondsAfterEmpty()
    {
        var (lobby, _, _, matchId) = Started();
        store.ServerListening(matchId, 7778);
        store.ServerFinished(matchId, [0]);
        store.ServerEmpty(matchId);
        clock.UtcNow += TimeSpan.FromSeconds(9);
        store.Housekeep();
        Assert.Empty(launcher.Stopped);
        clock.UtcNow += TimeSpan.FromSeconds(1);
        store.Housekeep();
        Assert.Equal([matchId], launcher.Stopped);
        Assert.Null(lobby.Notice);
    }

    [Fact]
    public void AServerQuittingAfterEmptyIsNotStopped()
    {
        var (_, _, _, matchId) = Started();
        store.ServerListening(matchId, 7778);
        store.ServerFinished(matchId, []);
        store.ServerEmpty(matchId);
        clock.UtcNow += TimeSpan.FromSeconds(2);
        launcher.Alive.Remove(matchId);
        clock.UtcNow += TimeSpan.FromSeconds(20);
        store.Housekeep();
        Assert.Empty(launcher.Stopped);
    }

    [Fact]
    public void EmptyInGameStopsTheServerWithTheNotice()
    {
        var (lobby, _, _, matchId) = Started();
        store.ServerListening(matchId, 7778);
        store.ServerEmpty(matchId);
        clock.UtcNow += TimeSpan.FromSeconds(10);
        store.Housekeep();
        Assert.Equal([matchId], launcher.Stopped);
        AssertBackWithNotice(lobby, LobbyNotices.ServerStopped);
    }

    [Fact]
    public void ALobbyLeftByEveryoneInGameIsDeletedAtTheEnd()
    {
        var (lobby, bob, alice, matchId) = Started();
        store.ServerListening(matchId, 7778);
        store.Leave(lobby.Id, bob);
        store.Leave(lobby.Id, alice);
        Assert.Same(lobby, store.Find(lobby.Id));
        store.ServerFinished(matchId, []);
        Assert.Same(lobby, store.Find(lobby.Id));
        launcher.Alive.Remove(matchId);
        store.Housekeep();
        Assert.Null(store.Find(lobby.Id));
    }

    [Fact]
    public void ALobbyBackFromAStoppedServerCanStartAgain()
    {
        var (lobby, bob, alice, matchId) = Started();
        launcher.Alive.Remove(matchId);
        store.UpdateMe(lobby.Id, bob, ready: true);
        store.UpdateMe(lobby.Id, alice, ready: true);
        var again = store.Start(lobby.Id, bob);
        Assert.NotEqual(matchId, again.Id);
        Assert.Equal(LobbyState.Starting, lobby.State);
    }

    [Fact]
    public void ListeningTwiceOrAfterFinishedChangesNothing()
    {
        var (lobby, _, _, matchId) = Started();
        store.ServerListening(matchId, 7790);
        store.ServerListening(matchId, 7790);
        Assert.Equal(LobbyState.InGame, lobby.State);
        store.ServerFinished(matchId, []);
        store.ServerListening(matchId, 7790);
        Assert.Equal(LobbyState.Lobby, lobby.State);
        Assert.Null(lobby.Match);
    }
}
