using HOLUpdate;

namespace HOLLauncher.Tests;

public class StatusViewTests
{
    private static LauncherStatus Status(string? installed, string latest, string notes = "", string launcher = "")
        => LauncherStatus.Compute(installed,
            new LatestResult(LatestOutcome.Ok, new LatestVersion(latest, new VersionManifest(latest, []), notes, [], launcher), ""),
            "1");

    [Fact]
    public void UpToDate()
    {
        var view = Fr(Status("v2", "v2", launcher: "1"));
        Assert.Equal("Version installée : v2", view.InstalledText);
        Assert.Equal("Dernière version : v2", view.LatestText);
        Assert.Equal("Le jeu est à jour.", view.StateText);
        Assert.Equal("Lanceur : version 1", view.LauncherText);
        Assert.False(view.UpdateGameEnabled);
        Assert.False(view.UpdateLauncherEnabled);
        Assert.True(view.RefreshEnabled);
    }

    [Fact]
    public void NumberedVersionsAreShownWithTheV()
    {
        var outdated = Fr(Status("0.3.0", "0.3.1"));
        Assert.Equal("Version installée : V 0.3.0", outdated.InstalledText);
        Assert.Equal("Dernière version : V 0.3.1", outdated.LatestText);
        Assert.Equal("Une mise à jour du jeu est disponible (V 0.3.1).", outdated.StateText);
        var missing = Fr(Status(null, "0.3.1"));
        Assert.Equal("Le jeu n'est pas installé ; la version V 0.3.1 est disponible.", missing.StateText);
        Assert.Equal("Version installée : V 0.3.0", StatusView.Error("x", "0.3.0", language: "fr").InstalledText);
    }

    [Fact]
    public void OutdatedEnablesGameUpdate()
    {
        var view = Fr(Status("v1", "v2"));
        Assert.Equal("Version installée : v1", view.InstalledText);
        Assert.Equal("Une mise à jour du jeu est disponible (v2).", view.StateText);
        Assert.True(view.UpdateGameEnabled);
        Assert.Equal("Mettre à jour le jeu", view.UpdateGameText);
    }

    [Fact]
    public void NotInstalledEnablesGameUpdate()
    {
        var view = Fr(Status(null, "v2"));
        Assert.Equal("Version installée : pas installé", view.InstalledText);
        Assert.Equal("Le jeu n'est pas installé ; la version v2 est disponible.", view.StateText);
        Assert.True(view.UpdateGameEnabled);
        Assert.Equal("Installer le jeu", view.UpdateGameText);
    }

    [Fact]
    public void UnreachableShowsReason()
    {
        var status = LauncherStatus.Compute("v1", new LatestResult(LatestOutcome.Unreachable, null, "connection refused"), "1");
        var view = Fr(status);
        Assert.Equal("Version installée : v1", view.InstalledText);
        Assert.Equal("Dernière version : inconnue", view.LatestText);
        Assert.Equal("Serveur injoignable : connection refused Vous pouvez jouer en solo.", view.StateText);
        Assert.False(view.UpdateGameEnabled);
        Assert.False(view.UpdateLauncherEnabled);
        Assert.True(view.RefreshEnabled);
        Assert.False(view.NotesVisible);
        Assert.True(view.PlayEnabled);
        Assert.Null(view.PlayConfirmation);
    }

    /** L21: three versions published, the first one installed: the box shows the two newer notes, newest first. */
    [Fact]
    public void OutdatedShowsTheNotesOfEveryNewerVersion()
    {
        ReleaseNote[] history = [new("0.3.11", "Nouvelle carte."), new("0.3.10", "Archers plus rapides."), new("0.3.9", "Un.")];
        var latest = new LatestVersion("0.3.11", new VersionManifest("0.3.11", []), "Nouvelle carte.", [], "", history);

        var view = StatusView.From(LauncherStatus.Compute("0.3.9", new LatestResult(LatestOutcome.Ok, latest, ""), "1"));

        Assert.True(view.NotesVisible);
        Assert.Equal("V 0.3.11\r\nNouvelle carte.\r\n\r\nV 0.3.10\r\nArchers plus rapides.", view.Notes);
    }

    [Fact]
    public void NotesShownOnlyWhenPresent()
    {
        Assert.False(Fr(Status("v2", "v2")).NotesVisible);
        var view = Fr(Status("v2", "v2", notes: "Archers plus rapides."));
        Assert.True(view.NotesVisible);
        Assert.Equal("Archers plus rapides.", view.Notes);
    }

    [Fact]
    public void NewerLauncherEnablesLauncherUpdate()
    {
        var view = Fr(Status("v2", "v2", launcher: "2"));
        Assert.True(view.UpdateLauncherEnabled);
        Assert.Equal("Lanceur : version 1 (version 2 disponible)", view.LauncherText);
    }

    [Fact]
    public void PlayFollowsPlayRules()
    {
        var upToDate = Fr(Status("v2", "v2"));
        Assert.True(upToDate.PlayEnabled);
        Assert.Null(upToDate.PlayConfirmation);
        var outdated = Fr(Status("v1", "v2"));
        Assert.True(outdated.PlayEnabled);
        Assert.Equal("Le jeu n'est pas à jour : le serveur refusera cette version. Jouer quand même (en solo) ?",
            outdated.PlayConfirmation);
        Assert.False(Fr(Status(null, "v2")).PlayEnabled);
    }

    [Fact]
    public void UnreachableWithoutGameCannotPlay()
    {
        var view = Fr(LauncherStatus.Compute(null, new LatestResult(LatestOutcome.Unreachable, null, "x"), "1"));
        Assert.False(view.PlayEnabled);
        Assert.Equal("Serveur injoignable : x", view.StateText);
    }

    [Fact]
    public void LoadingDisablesEverything()
    {
        var view = StatusView.Loading("1", "fr");
        Assert.Equal("Lecture en cours…", view.StateText);
        Assert.False(view.PlayEnabled || view.UpdateGameEnabled || view.UpdateLauncherEnabled || view.RefreshEnabled);
        Assert.Equal("Lanceur : version 1", view.LauncherText);
        Assert.False(view.NotesVisible);
    }

    [Fact]
    public void ErrorKeepsRefresh()
    {
        var view = StatusView.Error("servers.json: bad JSON", "v1", "1", "fr");
        Assert.Equal("Erreur : servers.json: bad JSON", view.StateText);
        Assert.Equal("Version installée : v1", view.InstalledText);
        Assert.Equal("Dernière version : inconnue", view.LatestText);
        Assert.True(view.RefreshEnabled);
        Assert.False(view.UpdateGameEnabled || view.UpdateLauncherEnabled || view.PlayEnabled);
    }

    [Fact]
    public void UpdatingDisablesEveryButtonAndKeepsTheTexts()
    {
        var view = StatusView.Updating(Fr(Status("v1", "v2", launcher: "2")));
        Assert.Equal("Version installée : v1", view.InstalledText);
        Assert.Equal("Une mise à jour du jeu est disponible (v2).", view.StateText);
        Assert.False(view.PlayEnabled || view.UpdateGameEnabled || view.UpdateLauncherEnabled || view.RefreshEnabled);
    }

    /** L16: the same status in English and Japanese. */
    [Fact]
    public void TheViewSpeaksEnglishAndJapanese()
    {
        var en = StatusView.From(Status("v1", "v2", launcher: "2"), "en");
        Assert.Equal("Installed version: v1", en.InstalledText);
        Assert.Equal("Latest version: v2", en.LatestText);
        Assert.Equal("A game update is available (v2).", en.StateText);
        Assert.Equal("Launcher: version 1 (version 2 available)", en.LauncherText);
        Assert.Equal("Update the game", en.UpdateGameText);
        Assert.Equal("The game is not up to date: the server will refuse this version. Play anyway (solo)?", en.PlayConfirmation);
        Assert.Equal("Install the game", StatusView.From(Status(null, "v2"), "en").UpdateGameText);

        var ja = StatusView.From(Status("v1", "v2", launcher: "2"), "ja");
        Assert.Equal("インストール済みのバージョン：v1", ja.InstalledText);
        Assert.Equal("最新バージョン：v2", ja.LatestText);
        Assert.Equal("ゲームのアップデートがあります（v2）。", ja.StateText);
        Assert.Equal("ランチャー：バージョン 1（バージョン 2 が利用可能）", ja.LauncherText);
        Assert.Equal("ゲームを更新", ja.UpdateGameText);
        Assert.Equal("ゲームをインストール", StatusView.From(Status(null, "v2"), "ja").UpdateGameText);
    }

    [Fact]
    public void LoadingErrorAndUnreachableInEnglishAndJapanese()
    {
        Assert.Equal("Loading…", StatusView.Loading("1", "en").StateText);
        Assert.Equal("Installed version: …", StatusView.Loading("1", "en").InstalledText);
        Assert.Equal("読み込み中…", StatusView.Loading("1", "ja").StateText);
        Assert.Equal("Update the game", StatusView.Loading("1", "en").UpdateGameText);
        Assert.Equal("Error: bad JSON", StatusView.Error("bad JSON", null, "1", "en").StateText);
        Assert.Equal("Installed version: not installed", StatusView.Error("bad JSON", null, "1", "en").InstalledText);
        Assert.Equal("エラー：bad JSON", StatusView.Error("bad JSON", null, "1", "ja").StateText);
        var unreachable = LauncherStatus.Compute("v1", new LatestResult(LatestOutcome.Unreachable, null, "x"), "1");
        Assert.Equal("Server unreachable: x You can play solo.", StatusView.From(unreachable, "en").StateText);
        Assert.Equal("Latest version: unknown", StatusView.From(unreachable, "en").LatestText);
        Assert.Equal("サーバーに接続できません：x ソロでプレイできます。", StatusView.From(unreachable, "ja").StateText);
    }

    /** The refused key is known by the outcome, not by comparing texts: it is said in every language. */
    [Theory]
    [InlineData("fr", "Clé d'accès refusée : le kit n'est pas le bon. Redemandez-le.")]
    [InlineData("en", "Access key refused: this is not the right kit. Ask for it again.")]
    [InlineData("ja", "アクセスキーが拒否されました。キットが正しくありません。もう一度入手してください。")]
    public void BadKeyIsSaidInEveryLanguage(string language, string expected)
    {
        var status = LauncherStatus.Compute("v1", new LatestResult(LatestOutcome.BadKey, null, "401 bad_key"), "1");
        Assert.True(status.KeyRefused);
        Assert.StartsWith(expected, StatusView.From(status, language).StateText);
    }

    private static StatusView Fr(LauncherStatus status) => StatusView.From(status, "fr");
}
