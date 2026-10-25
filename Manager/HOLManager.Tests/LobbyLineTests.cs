using HOLManager.Lobbies;

namespace HOLManager.Tests;

/** R2.5: each player's line, leader's settings and slots (Manager/PROTOCOL.md). */
public class LobbyLineTests
{
    private const string Version = LobbyEntryTests.Version;

    private static void AssertError(string error, Action action) => LobbyEntryTests.AssertError(error, action);

    private readonly LobbyStore store = new(Version, new FakeClock(), [new("Plain", 8), new("Duel", 2)]);
    private readonly Lobby lobby;
    private readonly string bob;
    private readonly string alice;

    public LobbyLineTests()
    {
        var created = store.Create("A", "Bob", Version);
        lobby = created.Lobby;
        bob = created.Token;
        alice = store.Join(lobby.Id, "Alice", Version).Token;
    }

    private LobbyPlayer Bob => lobby.Slots[0].Player!;
    private LobbyPlayer Alice => lobby.Slots[1].Player!;

    private void AllReady()
    {
        store.UpdateMe(lobby.Id, bob, ready: true);
        store.UpdateMe(lobby.Id, alice, ready: true);
        Assert.True(Bob.Ready && Alice.Ready);
    }

    [Fact]
    public void ColorAndTeamChange()
    {
        var state = store.UpdateMe(lobby.Id, alice, colorIndex: 7, team: 4);
        Assert.Same(lobby, state);
        Assert.Equal((7, 4), (Alice.ColorIndex, Alice.Team));
        store.UpdateMe(lobby.Id, alice, team: 0);
        Assert.Equal((7, 0), (Alice.ColorIndex, Alice.Team));
    }

    [Fact]
    public void ColorIsUniqueFirstComeKeepsIt()
    {
        AssertError(LobbyErrors.ColorTaken, () => store.UpdateMe(lobby.Id, alice, colorIndex: 0));
        Assert.Equal((0, 1), (Bob.ColorIndex, Alice.ColorIndex));
    }

    [Fact]
    public void SeveralPlayersCanChooseRandom()
    {
        // E2-2: "Aléatoire" is never taken; a new player still gets the first free real colour.
        store.UpdateMe(lobby.Id, bob, colorIndex: LobbyRules.RandomColor);
        store.UpdateMe(lobby.Id, alice, colorIndex: LobbyRules.RandomColor);
        Assert.Equal((LobbyRules.RandomColor, LobbyRules.RandomColor), (Bob.ColorIndex, Alice.ColorIndex));
        store.Join(lobby.Id, "Chloé", Version);
        Assert.Equal(0, lobby.Slots[2].Player!.ColorIndex);
    }

    [Fact]
    public void SameTeamIsAllowed()
    {
        store.UpdateMe(lobby.Id, bob, team: 2);
        store.UpdateMe(lobby.Id, alice, team: 2);
        Assert.Equal((2, 2), (Bob.Team, Alice.Team));
    }

    [Theory]
    [InlineData(-1, null)]
    [InlineData(9, null)]
    [InlineData(null, -1)]
    [InlineData(null, 5)]
    public void OutOfBoundsLineIsBadRequestAndChangesNothing(int? color, int? team)
    {
        AssertError(LobbyErrors.BadRequest, () => store.UpdateMe(lobby.Id, alice, colorIndex: color, team: team));
        Assert.Equal((1, 0), (Alice.ColorIndex, Alice.Team));
    }

    [Fact]
    public void ReadyAlone()
    {
        store.UpdateMe(lobby.Id, alice, ready: true);
        Assert.True(Alice.Ready);
        Assert.False(Bob.Ready);
        store.UpdateMe(lobby.Id, alice, ready: false);
        Assert.False(Alice.Ready);
    }

    [Fact]
    public void ChangingMyColorResetsOnlyMyReady()
    {
        AllReady();
        store.UpdateMe(lobby.Id, alice, colorIndex: 5);
        Assert.False(Alice.Ready);
        Assert.True(Bob.Ready);
    }

    [Fact]
    public void ChangingMyTeamResetsMyReady()
    {
        AllReady();
        store.UpdateMe(lobby.Id, alice, team: 3);
        Assert.False(Alice.Ready);
    }

    [Fact]
    public void ReadyInTheSameRequestWins()
    {
        store.UpdateMe(lobby.Id, alice, colorIndex: 5, team: 1, ready: true);
        Assert.True(Alice.Ready);
    }

    [Fact]
    public void SameValueIsNoChangeAndKeepsReady()
    {
        AllReady();
        store.UpdateMe(lobby.Id, alice, colorIndex: 1, team: 0);
        Assert.True(Alice.Ready);
    }

    [Fact]
    public void UnknownTokenIsNotInLobby()
    {
        AssertError(LobbyErrors.NotInLobby, () => store.UpdateMe(lobby.Id, "0123456789abcdef0123456789abcdef", ready: true));
        AssertError(LobbyErrors.NotInLobby, () => store.UpdateMe(lobby.Id, null, ready: true));
        var other = store.Create("B", "Carl", Version);
        AssertError(LobbyErrors.NotInLobby, () => store.UpdateMe(lobby.Id, other.Token, ready: true));
        AssertError(LobbyErrors.NotInLobby, () => store.UpdateSettings(lobby.Id, other.Token, nomad: true));
        AssertError(LobbyErrors.NotInLobby, () => store.SetSlot(lobby.Id, other.Token, 7, open: false));
    }

    [Fact]
    public void UnknownLobbyIsNotFound()
    {
        AssertError(LobbyErrors.LobbyNotFound, () => store.UpdateMe("L99", alice, ready: true));
        AssertError(LobbyErrors.LobbyNotFound, () => store.UpdateSettings("L99", bob, nomad: true));
        AssertError(LobbyErrors.LobbyNotFound, () => store.SetSlot("L99", bob, 7, open: false));
    }

    [Fact]
    public void NoChangeDuringAMatch()
    {
        lobby.State = LobbyState.InGame;
        AssertError(LobbyErrors.LobbyInGame, () => store.UpdateMe(lobby.Id, alice, colorIndex: 5));
        AssertError(LobbyErrors.LobbyInGame, () => store.UpdateSettings(lobby.Id, bob, nomad: true));
        AssertError(LobbyErrors.LobbyInGame, () => store.SetSlot(lobby.Id, bob, 7, open: false));
    }

    [Fact]
    public void LeaderChangesMapAndOptions()
    {
        var state = store.UpdateSettings(lobby.Id, bob, nomad: true, regicide: true);
        Assert.Same(lobby, state);
        Assert.Equal(("Plain", true, true, false),
            (lobby.Settings.Map, lobby.Settings.Nomad, lobby.Settings.Regicide, lobby.Settings.Catastrophe));
        store.UpdateSettings(lobby.Id, bob, map: "Duel", catastrophe: true, nomad: false);
        Assert.Equal(("Duel", false, true, true),
            (lobby.Settings.Map, lobby.Settings.Nomad, lobby.Settings.Regicide, lobby.Settings.Catastrophe));
    }

    [Fact]
    public void OnlyTheLeaderChangesSettingsAndSlots()
    {
        AssertError(LobbyErrors.NotLeader, () => store.UpdateSettings(lobby.Id, alice, nomad: true));
        AssertError(LobbyErrors.NotLeader, () => store.SetSlot(lobby.Id, alice, 7, open: false));
        Assert.False(lobby.Settings.Nomad);
        Assert.Equal(SlotState.Open, lobby.Slots[7].State);
    }

    [Fact]
    public void LeaderSettingResetsEveryReady()
    {
        AllReady();
        store.UpdateSettings(lobby.Id, bob, nomad: true);
        Assert.False(Bob.Ready || Alice.Ready);
    }

    [Fact]
    public void LeaderSlotResetsEveryReady()
    {
        AllReady();
        store.SetSlot(lobby.Id, bob, 7, open: false);
        Assert.False(Bob.Ready || Alice.Ready);
    }

    [Fact]
    public void UnchangedSettingKeepsReady()
    {
        AllReady();
        store.UpdateSettings(lobby.Id, bob, map: "Plain", nomad: false);
        store.SetSlot(lobby.Id, bob, 7, open: true);
        Assert.True(Bob.Ready && Alice.Ready);
    }

    [Fact]
    public void UnknownMapIsBadRequest()
    {
        AssertError(LobbyErrors.BadRequest, () => store.UpdateSettings(lobby.Id, bob, map: "Nowhere"));
        Assert.Equal("Plain", lobby.Settings.Map);
    }

    [Fact]
    public void SmallerMapClosesTheExtraSlots()
    {
        store.UpdateSettings(lobby.Id, bob, map: "Duel");
        Assert.All(lobby.Slots.Skip(2), s => Assert.Equal(SlotState.Closed, s.State));
        AssertError(LobbyErrors.LobbyFull, () => store.Join(lobby.Id, "Carl", Version));
        // A bigger map does not reopen them: the leader does.
        store.UpdateSettings(lobby.Id, bob, map: "Plain");
        Assert.Equal(SlotState.Closed, lobby.Slots[2].State);
        store.SetSlot(lobby.Id, bob, 2, open: true);
        Assert.Equal(2, store.Join(lobby.Id, "Carl", Version).Slot);
    }

    [Fact]
    public void SmallerMapRefusedWhenItsExtraSlotsAreHeld()
    {
        store.Join(lobby.Id, "Carl", Version);
        store.UpdateMe(lobby.Id, bob, ready: true);
        AssertError(LobbyErrors.SlotTaken, () => store.UpdateSettings(lobby.Id, bob, map: "Duel", nomad: true));
        Assert.Equal(("Plain", false, true), (lobby.Settings.Map, lobby.Settings.Nomad, Bob.Ready));
        Assert.Equal(SlotState.Open, lobby.Slots[3].State);
    }

    [Fact]
    public void LeaderClosesAndOpensASlot()
    {
        var state = store.SetSlot(lobby.Id, bob, 2, open: false);
        Assert.Same(lobby, state);
        Assert.Equal(SlotState.Closed, lobby.Slots[2].State);
        Assert.Equal(3, store.Join(lobby.Id, "Carl", Version).Slot);
        store.SetSlot(lobby.Id, bob, 2, open: true);
        Assert.Equal(SlotState.Open, lobby.Slots[2].State);
    }

    [Theory]
    [InlineData(0, false)]
    [InlineData(1, false)]
    [InlineData(1, true)]
    public void HeldSlotIsTaken(int index, bool open) =>
        AssertError(LobbyErrors.SlotTaken, () => store.SetSlot(lobby.Id, bob, index, open));

    [Theory]
    [InlineData(-1)]
    [InlineData(8)]
    public void SlotIndexOutOfBoundsIsBadRequest(int index) =>
        AssertError(LobbyErrors.BadRequest, () => store.SetSlot(lobby.Id, bob, index, open: false));

    [Fact]
    public void SlotBeyondTheMapCannotOpen()
    {
        store.UpdateSettings(lobby.Id, bob, map: "Duel");
        AssertError(LobbyErrors.BadRequest, () => store.SetSlot(lobby.Id, bob, 2, open: true));
        store.SetSlot(lobby.Id, bob, 2, open: false);
    }
}
