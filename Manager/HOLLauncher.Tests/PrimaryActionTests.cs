using HOLUpdate;

namespace HOLLauncher.Tests;

/** Launcher 5: the big button of the home window, one state per situation, in priority order. */
public class PrimaryActionTests
{
    private static StatusView View(string? installed, string? latest, LatestOutcome outcome = LatestOutcome.Ok)
    {
        var result = latest is null || outcome != LatestOutcome.Ok
            ? new LatestResult(outcome, null, "refused")
            : new LatestResult(LatestOutcome.Ok, new LatestVersion(latest, new VersionManifest(latest, []), "", [], "1"), "");
        return StatusView.From(LauncherStatus.Compute(installed, result, "1", "fr"), "fr");
    }

    private static PrimaryAction Fr(StatusView view, ProgressView? progress = null, bool running = false, bool busy = false)
        => PrimaryAction.From(view, progress, running, busy, "fr");

    [Fact]
    public void ReadingWaits()
    {
        Assert.Equal(new PrimaryAction(PrimaryKind.Loading, "Chargement…", false), Fr(StatusView.Loading(language: "fr")));
    }

    [Fact]
    public void NothingInstalledInstalls()
    {
        Assert.Equal(new PrimaryAction(PrimaryKind.Install, "Installer le jeu", true), Fr(View(null, "0.4.3")));
    }

    [Fact]
    public void OutdatedUpdates()
    {
        Assert.Equal(new PrimaryAction(PrimaryKind.Update, "Mettre à jour", true), Fr(View("0.4.2", "0.4.3")));
    }

    [Fact]
    public void UpToDatePlays()
    {
        Assert.Equal(new PrimaryAction(PrimaryKind.Play, "Jouer", true), Fr(View("0.4.3", "0.4.3")));
    }

    [Fact]
    public void UnreachableWithAGamePlaysSolo()
    {
        Assert.Equal(PrimaryKind.Play, Fr(View("0.4.3", null, LatestOutcome.Unreachable)).Kind);
    }

    [Fact]
    public void UnreachableWithoutGameCanDoNothing()
    {
        Assert.Equal(new PrimaryAction(PrimaryKind.None, "Installer le jeu", false), Fr(View(null, null, LatestOutcome.Unreachable)));
        Assert.Equal(new PrimaryAction(PrimaryKind.None, "Installer le jeu", false), Fr(StatusView.Error("x", null, language: "fr")));
    }

    [Fact]
    public void ARunWithABarPausesThenResumes()
    {
        var updating = StatusView.Updating(View("0.4.2", "0.4.3"));
        var line = ProgressView.Starting("fr");
        Assert.Equal(new PrimaryAction(PrimaryKind.Pause, "Pause", true), Fr(updating, line));
        Assert.Equal(new PrimaryAction(PrimaryKind.Pause, "Pause", false), Fr(updating, line.Pausing("fr")));
        Assert.Equal(new PrimaryAction(PrimaryKind.Pause, "Pause", false), Fr(updating, line.Cancelling("fr")));
        Assert.Equal(new PrimaryAction(PrimaryKind.Resume, "Reprendre", true), Fr(updating, line.Paused("fr")));
    }

    [Fact]
    public void ARunWithoutBarIsBusy()
    {
        Assert.Equal(new PrimaryAction(PrimaryKind.Busy, "Un instant…", false), Fr(StatusView.Updating(View("0.4.3", "0.4.3")), busy: true));
    }

    [Fact]
    public void TheRunningGameWinsOverEverything()
    {
        var expected = new PrimaryAction(PrimaryKind.Running, "En cours", false);
        Assert.Equal(expected, Fr(View("0.4.3", "0.4.3"), running: true));
        Assert.Equal(expected, Fr(StatusView.Loading(language: "fr"), ProgressView.Starting("fr"), running: true, busy: true));
    }

    [Theory]
    [InlineData("en", "Play", "Running", "Update")]
    [InlineData("ja", "プレイ", "プレイ中", "アップデート")]
    public void LabelsFollowTheLanguage(string language, string play, string running, string update)
    {
        var upToDate = View("0.4.3", "0.4.3");
        Assert.Equal(play, PrimaryAction.From(upToDate, null, false, false, language).Label);
        Assert.Equal(running, PrimaryAction.From(upToDate, null, true, false, language).Label);
        Assert.Equal(update, PrimaryAction.From(View("0.4.2", "0.4.3"), null, false, false, language).Label);
    }
}
