namespace HOLLauncher.Tests;

/** L23: the background update check's decision table and its timing. */
public class BackgroundUpdatePolicyTests
{
    private static readonly LauncherSettings NotifyMode = LauncherSettings.Default;
    private static readonly LauncherSettings InstallMode = new(AutoUpdate: "install");
    private static readonly LauncherSettings OffMode = new(AutoUpdate: "off");

    private static LauncherStatus Status(GameState state, string? installed = "0.1.0", string? latest = "0.2.0",
        string launcherAvailable = LauncherInfo.Version)
        => new(state, installed, latest, [], LauncherInfo.Version, launcherAvailable, "");

    private static readonly LauncherStatus Outdated = Status(GameState.Outdated);

    private static BackgroundDecision Decide(LauncherSettings settings, LauncherStatus? status, bool gameRunning = false,
        bool running = false, bool windowShown = false, BackgroundMemory? memory = null, bool manual = false)
        => BackgroundUpdatePolicy.Decide(settings, status, gameRunning, running, windowShown, memory ?? BackgroundMemory.Empty, manual);

    [Theory]
    [InlineData(null, AutoUpdateMode.Notify)]
    [InlineData("notify", AutoUpdateMode.Notify)]
    [InlineData("off", AutoUpdateMode.Off)]
    [InlineData("OFF", AutoUpdateMode.Off)]
    [InlineData("install", AutoUpdateMode.Install)]
    [InlineData("later-mode", AutoUpdateMode.Notify)]
    public void ModeOfTheSetting(string? setting, AutoUpdateMode mode)
    {
        Assert.Equal(mode, BackgroundUpdatePolicy.ModeOf(setting));
    }

    [Fact]
    public void TheDefaultModeIsNotStored()
    {
        Assert.Null(BackgroundUpdatePolicy.SettingFor(AutoUpdateMode.Notify));
        Assert.Equal("off", BackgroundUpdatePolicy.SettingFor(AutoUpdateMode.Off));
        Assert.Equal("install", BackgroundUpdatePolicy.SettingFor(AutoUpdateMode.Install));
    }

    [Theory]
    [InlineData(null, 30)]
    [InlineData(5, 5)]
    [InlineData(1, 5)]
    [InlineData(-3, 5)]
    [InlineData(90, 90)]
    [InlineData(100000, 1440)]
    public void IntervalIsBounded(int? minutes, int expected)
    {
        Assert.Equal(TimeSpan.FromMinutes(expected), BackgroundUpdatePolicy.Interval(minutes));
    }

    [Fact]
    public void NotifyModeNotifiesANewVersionOnce()
    {
        var first = Decide(NotifyMode, Outdated);
        Assert.Equal(BackgroundAction.Notify, first.Game);
        Assert.Equal("0.2.0", first.Version);

        var second = Decide(NotifyMode, Outdated, memory: first.Memory);
        Assert.Equal(BackgroundAction.None, second.Game);
        Assert.True(second.NothingToSay);

        var newer = Decide(NotifyMode, Status(GameState.Outdated, latest: "0.3.0"), memory: second.Memory);
        Assert.Equal(BackgroundAction.Notify, newer.Game);
        Assert.Equal("0.3.0", newer.Version);
    }

    [Fact]
    public void AGameNotInstalledIsNotifiedEvenInInstallMode()
    {
        var decision = Decide(InstallMode, Status(GameState.NotInstalled, installed: null));
        Assert.Equal(BackgroundAction.Notify, decision.Game);
    }

    [Fact]
    public void InstallModeInstallsAnOutdatedGameOnce()
    {
        var first = Decide(InstallMode, Outdated);
        Assert.Equal(BackgroundAction.Install, first.Game);
        Assert.Equal("0.2.0", first.Version);

        // Failed, paused or cancelled: the timer does not start it again, nor notify it (the failure was said).
        var again = Decide(InstallMode, Outdated, memory: first.Memory);
        Assert.Equal(BackgroundAction.None, again.Game);
    }

    [Fact]
    public void InstallModeInstallsEvenWithTheWindowShown()
    {
        Assert.Equal(BackgroundAction.Install, Decide(InstallMode, Outdated, windowShown: true).Game);
    }

    [Theory]
    [InlineData("notify")]
    [InlineData("install")]
    [InlineData("off")]
    public void NothingWhileTheGameRuns(string mode)
    {
        var settings = new LauncherSettings(AutoUpdate: mode);
        var decision = Decide(settings, Status(GameState.Outdated, launcherAvailable: "99"), gameRunning: true, manual: true);
        Assert.True(decision.NothingToSay);
        Assert.Equal(BackgroundMemory.Empty, decision.Memory);
    }

    [Fact]
    public void NothingWhileARunIsUnderWay()
    {
        Assert.Equal(BackgroundAction.None, Decide(InstallMode, Outdated, running: true).Game);
        Assert.Equal(BackgroundAction.None, Decide(NotifyMode, Outdated, running: true).Game);
        // Not remembered: once the run ends, the next check may still act.
        Assert.Equal(BackgroundMemory.Empty, Decide(InstallMode, Outdated, running: true).Memory);
    }

    [Fact]
    public void AShownWindowIsNotNotifiedUntilHidden()
    {
        var shown = Decide(NotifyMode, Outdated, windowShown: true);
        Assert.Equal(BackgroundAction.None, shown.Game);
        Assert.Equal(BackgroundAction.Notify, Decide(NotifyMode, Outdated, memory: shown.Memory).Game);
    }

    [Theory]
    [InlineData(GameState.UpToDate)]
    [InlineData(GameState.Unreachable)]
    public void NothingWithoutAnUpdate(GameState state)
    {
        Assert.True(Decide(InstallMode, Status(state, latest: state == GameState.UpToDate ? "0.1.0" : null)).NothingToSay);
    }

    [Fact]
    public void NothingWithoutAStatus()
    {
        Assert.True(Decide(InstallMode, null, manual: true).NothingToSay);
    }

    [Fact]
    public void OffDoesNothingUnlessAsked()
    {
        Assert.True(Decide(OffMode, Outdated).NothingToSay);
        var asked = Decide(OffMode, Outdated, manual: true);
        Assert.Equal(BackgroundAction.Notify, asked.Game);
    }

    [Fact]
    public void AManualCheckSaysAgainWhatWasAlreadyNotified()
    {
        var first = Decide(NotifyMode, Status(GameState.Outdated, launcherAvailable: "6"));
        var asked = Decide(NotifyMode, Status(GameState.Outdated, launcherAvailable: "6"), memory: first.Memory, manual: true);
        Assert.Equal(BackgroundAction.Notify, asked.Game);
        Assert.Equal("6", asked.LauncherVersion);
    }

    [Fact]
    public void AManualCheckInInstallModeTriesTheInstallAgain()
    {
        var first = Decide(InstallMode, Outdated);
        Assert.Equal(BackgroundAction.Install, Decide(InstallMode, Outdated, memory: first.Memory, manual: true).Game);
    }

    [Fact]
    public void ALauncherUpdateIsOnlyNotifiedAndOnce()
    {
        var status = Status(GameState.UpToDate, latest: "0.1.0", launcherAvailable: "6");
        var first = Decide(InstallMode, status);
        Assert.Equal(BackgroundAction.None, first.Game);
        Assert.Equal("6", first.LauncherVersion);
        Assert.Null(Decide(InstallMode, status, memory: first.Memory).LauncherVersion);
        Assert.Null(Decide(NotifyMode, status, windowShown: true).LauncherVersion);
    }

    [Fact]
    public void TheGameAndTheLauncherAreBothSaid()
    {
        var decision = Decide(NotifyMode, Status(GameState.Outdated, launcherAvailable: "6"));
        Assert.Equal(BackgroundAction.Notify, decision.Game);
        Assert.Equal("6", decision.LauncherVersion);
        Assert.Equal(new BackgroundMemory(NotifiedGame: "0.2.0", NotifiedLauncher: "6"), decision.Memory);
    }

    [Fact]
    public void ScheduleIsDueFirstThenAfterEachInterval()
    {
        var now = new DateTime(2026, 11, 24, 10, 0, 0, DateTimeKind.Utc);
        var schedule = new CheckSchedule(() => now);
        Assert.True(schedule.IsDue(30));

        schedule.Checked();
        Assert.False(schedule.IsDue(30));
        now = now.AddMinutes(29);
        Assert.False(schedule.IsDue(30));
        now = now.AddMinutes(1);
        Assert.True(schedule.IsDue(30));
    }

    [Fact]
    public void ScheduleFollowsAChangedIntervalAndItsBounds()
    {
        var now = new DateTime(2026, 11, 24, 10, 0, 0, DateTimeKind.Utc);
        var schedule = new CheckSchedule(() => now);
        schedule.Checked();
        now = now.AddMinutes(10);
        Assert.False(schedule.IsDue(null)); // 30 by default
        Assert.True(schedule.IsDue(10));
        Assert.True(schedule.IsDue(1)); // bounded to 5
    }

    [Fact]
    public void ScheduleIsDueAfterASleepOrAClockGoingBack()
    {
        var now = new DateTime(2026, 11, 24, 10, 0, 0, DateTimeKind.Utc);
        var schedule = new CheckSchedule(() => now);
        schedule.Checked();
        now = now.AddHours(9); // the PC slept
        Assert.True(schedule.IsDue(30));

        schedule.Checked();
        now = now.AddHours(-2);
        Assert.True(schedule.IsDue(30));
    }
}
