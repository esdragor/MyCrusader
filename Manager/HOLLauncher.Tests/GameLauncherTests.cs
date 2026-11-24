namespace HOLLauncher.Tests;

public class GameLauncherTests
{
    [Fact]
    public void FindsTheGameExeInTheGameFolder()
    {
        using var game = new TempDir();
        var exe = game.Write(GameLauncher.ExeName, "x");
        Assert.Equal(exe, GameLauncher.FindExe(game.Root));
    }

    [Fact]
    public void MissingExeIsAClearMessage()
    {
        using var game = new TempDir();
        Assert.Null(GameLauncher.FindExe(game.Root));
        var result = GameLauncher.Start(game.Root, language: "fr");
        Assert.False(result.Success);
        Assert.Equal($"Le jeu est introuvable : {Path.Combine(game.Root, GameLauncher.ExeName)} n'existe pas.", result.Message);
        var exe = Path.Combine(game.Root, GameLauncher.ExeName);
        Assert.Equal($"Game not found: {exe} does not exist.", GameLauncher.Start(game.Root, language: "en").Message);
        Assert.Equal($"ゲームが見つかりません：{exe} が存在しません。", GameLauncher.Start(game.Root, language: "ja").Message);
    }

    [Fact]
    public void StartsTheExeInTheGameFolder()
    {
        using var game = new TempDir();
        // A copy of cmd.exe under the game's name: "/c cd > started.txt" writes its working folder and exits at once.
        var exe = Path.Combine(game.Root, GameLauncher.ExeName);
        File.Copy(Path.Combine(Environment.SystemDirectory, "cmd.exe"), exe);
        var result = GameLauncher.Start(game.Root, ["/c", "cd > started.txt"]);
        Assert.True(result.Success, result.Message);
        var marker = Path.Combine(game.Root, "started.txt");
        for (var i = 0; i < 100 && !(File.Exists(marker) && new FileInfo(marker).Length > 0); i++)
        {
            Thread.Sleep(50);
        }
        Thread.Sleep(100);
        Assert.Equal(Path.TrimEndingDirectorySeparator(Path.GetFullPath(game.Root)),
            Path.TrimEndingDirectorySeparator(File.ReadAllText(marker).Trim()));
    }

    [Fact]
    public void LaunchOptionsArePassedToTheGame()
    {
        using var game = new TempDir();
        game.Write(GameLauncher.ExeName, "x");
        var settings = new LauncherSettings(DisplayMode: "fullscreen", ResolutionWidth: 2560, ResolutionHeight: 1440,
            ExtraArguments: "-nosound -Key=\"a b\"");
        System.Diagnostics.ProcessStartInfo? started = null;
        var result = GameLauncher.Start(game.Root, LaunchArguments.For(settings), info =>
        {
            started = info;
            return null;
        });
        Assert.True(result.Success, result.Message);
        Assert.NotNull(started);
        Assert.Equal(Path.Combine(game.Root, GameLauncher.ExeName), started.FileName);
        Assert.Equal(game.Root, started.WorkingDirectory);
        Assert.Equal("-fullscreen -ResX=2560 -ResY=1440 -nosound -Key=\"a b\"", started.Arguments);
        Assert.Empty(started.ArgumentList);
    }

    [Fact]
    public void DefaultSettingsStartTheGameWithoutArgument()
    {
        using var game = new TempDir();
        game.Write(GameLauncher.ExeName, "x");
        System.Diagnostics.ProcessStartInfo? started = null;
        GameLauncher.Start(game.Root, LaunchArguments.For(LauncherSettings.Default), info =>
        {
            started = info;
            return null;
        });
        Assert.Equal("", started!.Arguments);
    }

    [Fact]
    public void QuotedArgumentsReachTheGameVerbatim()
    {
        // Unreal reads the raw command line: -Key="a b" must arrive as typed, not re-quoted as "-Key=\"a b\"".
        using var game = new TempDir();
        File.Copy(Path.Combine(Environment.SystemDirectory, "cmd.exe"), Path.Combine(game.Root, GameLauncher.ExeName));
        var result = GameLauncher.Start(game.Root, ["/c", "echo", "-Key=\"a b\"", "-windowed>", "args.txt"]);
        Assert.True(result.Success, result.Message);
        var marker = Path.Combine(game.Root, "args.txt");
        for (var i = 0; i < 100 && !(File.Exists(marker) && new FileInfo(marker).Length > 0); i++)
        {
            Thread.Sleep(50);
        }
        Thread.Sleep(100);
        Assert.Equal("-Key=\"a b\" -windowed", File.ReadAllText(marker).Trim());
    }

    [Fact]
    public void GameReadsServersJsonFromTheFolderTheLauncherReads()
    {
        // The packaged game reads FPaths::RootDir()/servers.json, RootDir being the folder of HeroesOfLiberty.exe
        // (Windows\ of the package); the launcher reads <game dir>/servers.json, the same folder as the exe it starts.
        using var game = new TempDir();
        game.Write(GameLauncher.ExeName, "x");
        Assert.Equal(Path.GetDirectoryName(GameLauncher.FindExe(game.Root)),
            Path.GetDirectoryName(Path.Combine(game.Root, GameServers.FileName)));
    }
}
