using HOLUpdate;

namespace HOLLauncher.Tests;

/** Launcher 5: the ⚙ menu of the big button, entries and what is enabled in each state. */
public class HomeMenuTests
{
    private static StatusView View(string? installed, string latest, string launcher = "1")
        => StatusView.From(LauncherStatus.Compute(installed,
            new LatestResult(LatestOutcome.Ok, new LatestVersion(latest, new VersionManifest(latest, []), "", [], launcher), ""),
            "1", "fr"), "fr");

    private static List<MenuCommand> Commands(IEnumerable<MenuEntry> entries) => entries.Select(e => e.Command).ToList();

    private static List<MenuCommand> Enabled(IEnumerable<MenuEntry> entries) => entries.Where(e => e.Enabled).Select(e => e.Command).ToList();

    [Fact]
    public void IdleUpToDateListsTheUsualEntriesAllEnabled()
    {
        var entries = HomeMenu.For(View("0.4.3", "0.4.3"), running: false, gameRunning: false, "fr");
        Assert.Equal([MenuCommand.Settings, MenuCommand.Repair, MenuCommand.OpenGameFolder, MenuCommand.OpenLogs,
            MenuCommand.SendReport, MenuCommand.Uninstall, MenuCommand.Refresh], Commands(entries));
        Assert.All(entries, e => Assert.True(e.Enabled, e.Command.ToString()));
        Assert.Equal(["Réglages", "Vérifier / réparer les fichiers", "Ouvrir le dossier du jeu", "Ouvrir le dossier des journaux",
            "Envoyer un rapport…", "Désinstaller le jeu…", "Actualiser"], entries.Select(e => e.Text));
        Assert.Equal([MenuCommand.Repair, MenuCommand.Refresh], entries.Where(e => e.SeparatorBefore).Select(e => e.Command));
    }

    [Fact]
    public void OutdatedOffersPlayAnywayFirst()
    {
        var entries = HomeMenu.For(View("0.4.2", "0.4.3"), false, false, "fr");
        Assert.Equal(new MenuEntry(MenuCommand.PlayAnyway, "Jouer quand même", true), entries[0]);
        Assert.True(entries[1].SeparatorBefore);
    }

    [Fact]
    public void NotInstalledHasNoPlayAnyway()
    {
        Assert.DoesNotContain(MenuCommand.PlayAnyway, Commands(HomeMenu.For(View(null, "0.4.3"), false, false, "fr")));
    }

    [Fact]
    public void NewerLauncherAddsItsUpdateBeforeRefresh()
    {
        var entries = HomeMenu.For(View("0.4.3", "0.4.3", launcher: "9"), false, false, "fr");
        Assert.Equal([MenuCommand.UpdateLauncher, MenuCommand.Refresh], Commands(entries)[^2..]);
        Assert.True(entries[^2].SeparatorBefore);
        Assert.False(entries[^1].SeparatorBefore);
        Assert.Equal("Mettre à jour le lanceur", entries[^2].Text);
    }

    [Fact]
    public void DuringARunOnlyTheFoldersStayEnabled()
    {
        var entries = HomeMenu.For(StatusView.Updating(View("0.4.2", "0.4.3", launcher: "9")), running: true, gameRunning: false, "fr");
        Assert.Equal([MenuCommand.OpenGameFolder, MenuCommand.OpenLogs], Enabled(entries));
    }

    [Fact]
    public void WhileReadingOnlyTheFoldersAndTheReport()
    {
        Assert.Equal([MenuCommand.OpenGameFolder, MenuCommand.OpenLogs, MenuCommand.SendReport],
            Enabled(HomeMenu.For(StatusView.Loading(language: "fr"), false, false, "fr")));
    }

    [Fact]
    public void WhileTheGameRunsNothingTouchesItsFiles()
    {
        var entries = HomeMenu.For(View("0.4.2", "0.4.3", launcher: "9"), running: false, gameRunning: true, "fr");
        Assert.DoesNotContain(MenuCommand.PlayAnyway, Commands(entries));
        Assert.Equal([MenuCommand.Settings, MenuCommand.OpenGameFolder, MenuCommand.OpenLogs, MenuCommand.SendReport,
            MenuCommand.Refresh], Enabled(entries));
    }

    [Fact]
    public void EnabledAgreesWithTheMenu()
    {
        var view = View("0.4.3", "0.4.3");
        Assert.True(HomeMenu.Enabled(MenuCommand.Settings, view, false, false));
        Assert.False(HomeMenu.Enabled(MenuCommand.Settings, view, true, false));
        Assert.False(HomeMenu.Enabled(MenuCommand.UpdateLauncher, view, false, false));
        Assert.True(HomeMenu.Enabled(MenuCommand.SendReport, view, false, true));
    }

    [Fact]
    public void TextsFollowTheLanguage()
    {
        Assert.Equal("設定", HomeMenu.For(View("0.4.3", "0.4.3"), false, false, "ja")[0].Text);
        Assert.Equal("Settings", HomeMenu.For(View("0.4.3", "0.4.3"), false, false, "en")[0].Text);
    }
}
