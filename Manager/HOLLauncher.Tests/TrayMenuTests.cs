namespace HOLLauncher.Tests;

/** L23: the notification-area icon's menu, its tooltip, and what the close button does. */
public class TrayMenuTests
{
    private static StatusView View(GameState state, string? installed = "0.1.0", string? latest = "0.1.0")
        => StatusView.From(new LauncherStatus(state, installed, latest, [], LauncherInfo.Version, LauncherInfo.Version, "down"), "en");

    private static IReadOnlyList<TrayCommand> Commands(StatusView view, bool running = false, bool gameRunning = false)
        => TrayMenu.For(view, running, gameRunning, "en").Select(e => e.Command).ToList();

    [Fact]
    public void APlayableGameOffersPlay()
    {
        var entries = TrayMenu.For(View(GameState.UpToDate), running: false, gameRunning: false, "fr");
        Assert.Equal([TrayCommand.Open, TrayCommand.Play, TrayCommand.CheckForUpdates, TrayCommand.Quit], entries.Select(e => e.Command));
        Assert.Equal(["Ouvrir", "Jouer", "Rechercher une mise à jour", "Quitter"], entries.Select(e => e.Text));
        Assert.All(entries, e => Assert.True(e.Enabled));
        Assert.True(entries[0].Bold);
        Assert.Equal([false, false, true, true], entries.Select(e => e.SeparatorBefore));
    }

    [Fact]
    public void NoPlayWithoutAGameNorWhileItRuns()
    {
        Assert.DoesNotContain(TrayCommand.Play, Commands(View(GameState.NotInstalled, installed: null, latest: "0.2.0")));
        Assert.DoesNotContain(TrayCommand.Play, Commands(View(GameState.UpToDate), gameRunning: true));
        Assert.DoesNotContain(TrayCommand.Play, Commands(StatusView.Loading(language: "en")));
    }

    [Fact]
    public void AnOutdatedGameCanStillBePlayed()
    {
        Assert.Contains(TrayCommand.Play, Commands(View(GameState.Outdated, latest: "0.2.0")));
    }

    [Fact]
    public void DuringARunOnlyOpenAndQuit()
    {
        var view = View(GameState.UpToDate);
        Assert.False(TrayMenu.Enabled(TrayCommand.Play, view, running: true, gameRunning: false));
        Assert.False(TrayMenu.Enabled(TrayCommand.CheckForUpdates, view, running: true, gameRunning: false));
        Assert.True(TrayMenu.Enabled(TrayCommand.Open, view, running: true, gameRunning: false));
        Assert.True(TrayMenu.Enabled(TrayCommand.Quit, view, running: true, gameRunning: false));
    }

    [Fact]
    public void NoCheckWhileReadingNorWhilePlaying()
    {
        Assert.False(TrayMenu.Enabled(TrayCommand.CheckForUpdates, StatusView.Loading(language: "en"), false, false));
        Assert.False(TrayMenu.Enabled(TrayCommand.CheckForUpdates, View(GameState.UpToDate), false, gameRunning: true));
    }

    [Fact]
    public void TheMenuIsInEveryLanguage()
    {
        Assert.Equal(["開く", "プレイ", "アップデートを確認", "終了"],
            TrayMenu.For(View(GameState.UpToDate), false, false, "ja").Select(e => e.Text));
        Assert.Equal("Check for updates", TrayMenu.For(View(GameState.UpToDate), false, false, "en")[2].Text);
    }

    [Fact]
    public void TooltipIsTheNameThenTheLineCutToWindowsLimit()
    {
        Assert.Equal("Heroes of Liberty", TrayMenu.Tooltip("Heroes of Liberty", null));
        Assert.Equal("Heroes of Liberty", TrayMenu.Tooltip("Heroes of Liberty", "  "));
        Assert.Equal("Heroes of Liberty\nThe game is up to date.", TrayMenu.Tooltip("Heroes of Liberty", "The game is up to date. "));
        var cut = TrayMenu.Tooltip("Heroes of Liberty", new string('x', 300));
        Assert.Equal(TrayMenu.MaxTooltip, cut.Length);
        Assert.Equal(TrayMenu.Ellipsis, cut[^1]);
    }

    /** The user's choice (2026-11-24): the close button hides in the notification area by default; unticked, it quits. */
    [Fact]
    public void CloseHidesByDefaultAndQuitsWhenUnticked()
    {
        Assert.True(TrayMenu.CloseHides(LauncherSettings.Default, gameRunning: false, quitting: false));
        Assert.False(TrayMenu.CloseHides(new LauncherSettings(MinimizeToTray: false), false, false));
    }

    [Fact]
    public void CloseHidesWithMinimizeToTrayExceptWhilePlayingOrQuitting()
    {
        var tray = new LauncherSettings(MinimizeToTray: true);
        Assert.True(TrayMenu.CloseHides(tray, gameRunning: false, quitting: false));
        Assert.False(TrayMenu.CloseHides(tray, gameRunning: true, quitting: false));
        Assert.False(TrayMenu.CloseHides(tray, gameRunning: false, quitting: true));
    }
}
