namespace HOLLauncher.Tests;

/** The game's command line built from the launch options of the settings (L19). */
public class LaunchArgumentsTests
{
    [Fact]
    public void DefaultsGiveNoArgument()
    {
        Assert.Empty(LaunchArguments.For(LauncherSettings.Default));
        Assert.Empty(LaunchArguments.For(new LauncherSettings(Language: "en", GameDir: @"D:\HOL", ExtraArguments: "   ")));
    }

    [Theory]
    [InlineData("windowed", "-windowed")]
    [InlineData("fullscreen", "-fullscreen")]
    [InlineData("Windowed", "-windowed")]
    public void DisplayModeIsTheUnrealSwitch(string mode, string expected)
    {
        Assert.Equal([expected], LaunchArguments.For(new LauncherSettings(DisplayMode: mode)));
    }

    [Theory]
    [InlineData("borderless")]
    [InlineData("")]
    public void UnknownDisplayModeIsTheGamesOwn(string mode)
    {
        // A later launcher may add a mode: an older one leaves the choice to the game.
        Assert.Empty(LaunchArguments.For(new LauncherSettings(DisplayMode: mode)));
    }

    [Fact]
    public void ResolutionIsResXAndResY()
    {
        Assert.Equal(["-ResX=1920", "-ResY=1080"],
            LaunchArguments.For(new LauncherSettings(ResolutionWidth: 1920, ResolutionHeight: 1080)));
    }

    [Theory]
    [InlineData(1920, null)]
    [InlineData(null, 1080)]
    [InlineData(0, 1080)]
    [InlineData(-1920, 1080)]
    [InlineData(1920, 100000)]
    public void IncompleteOrOutOfRangeResolutionIsTheGamesOwn(int? width, int? height)
    {
        // Only a hand-edited file gets here (the Settings window checks the sizes): the launch never fails on it.
        Assert.Empty(LaunchArguments.For(new LauncherSettings(ResolutionWidth: width, ResolutionHeight: height)));
    }

    [Theory]
    [InlineData(1280, 720, true)]
    [InlineData(LaunchArguments.MinSize, LaunchArguments.MinSize, true)]
    [InlineData(LaunchArguments.MaxSize, LaunchArguments.MaxSize, true)]
    [InlineData(LaunchArguments.MinSize - 1, 720, false)]
    [InlineData(1280, LaunchArguments.MaxSize + 1, false)]
    [InlineData(0, 0, false)]
    public void ResolutionLimits(int width, int height, bool valid)
    {
        Assert.Equal(valid, LaunchArguments.IsValidResolution(width, height));
    }

    [Fact]
    public void CommonResolutionsAreValid()
    {
        Assert.Contains((1920, 1080), LaunchArguments.CommonResolutions);
        Assert.All(LaunchArguments.CommonResolutions, r => Assert.True(LaunchArguments.IsValidResolution(r.Width, r.Height)));
    }

    [Fact]
    public void ExtraArgumentsComeLastInOrder()
    {
        var settings = new LauncherSettings(DisplayMode: "windowed", ResolutionWidth: 1280, ResolutionHeight: 720,
            ExtraArguments: "  -nosound   -culture=en ");
        Assert.Equal(["-windowed", "-ResX=1280", "-ResY=720", "-nosound", "-culture=en"], LaunchArguments.For(settings));
    }

    [Theory]
    [InlineData("-a -b", new[] { "-a", "-b" })]
    [InlineData("-Key=\"a b\" -c", new[] { "-Key=\"a b\"", "-c" })]
    [InlineData("\"-Name=x y\"", new[] { "\"-Name=x y\"" })]
    [InlineData("-a\t-b\r\n-c", new[] { "-a", "-b", "-c" })]
    [InlineData("-Key=\"unclosed value", new[] { "-Key=\"unclosed value" })]
    [InlineData("", new string[0])]
    public void ExtraArgumentsAreSplitLikeACommandLineKeepingTheirQuotes(string text, string[] expected)
    {
        // Unreal reads the raw command line (FCommandLine, FParse::Value reads -Key="a b"): quotes stay in the token.
        Assert.Equal(expected, LaunchArguments.Split(text));
    }

    [Theory]
    [InlineData("-a \"b c\"", false)]
    [InlineData("-a \"b c", true)]
    [InlineData("-a", false)]
    public void UnclosedQuoteIsSeen(string text, bool unclosed)
    {
        Assert.Equal(unclosed, LaunchArguments.HasUnclosedQuote(text));
    }

    [Fact]
    public void ExtraArgumentsCannotContradictTheChosenOptions()
    {
        var settings = new LauncherSettings(DisplayMode: "fullscreen", ResolutionWidth: 1920, ResolutionHeight: 1080,
            ExtraArguments: "-WINDOWED /fullscreen -resx=800 -ResY=600 -nosound");
        Assert.Equal(["-fullscreen", "-ResX=1920", "-ResY=1080", "-nosound"], LaunchArguments.For(settings));
        Assert.Equal(["-WINDOWED", "/fullscreen", "-resx=800", "-ResY=600"], LaunchArguments.Rejected(settings));
    }

    [Fact]
    public void ExtraArgumentsMayCarryWhatTheGameDecides()
    {
        // Mode and resolution left to the game: the advanced player may still pass them by hand.
        var settings = new LauncherSettings(ExtraArguments: "-windowed -ResX=800 -ResY=600");
        Assert.Equal(["-windowed", "-ResX=800", "-ResY=600"], LaunchArguments.For(settings));
        Assert.Empty(LaunchArguments.Rejected(settings));
    }

    [Theory]
    [InlineData("-ServersFile=D:\\other.json")]
    [InlineData("-serversfile=\"D:\\a b.json\"")]
    public void ServersFileIsTheLaunchers(string extra)
    {
        // The launcher reads servers.json beside the game's exe (GameServers) and the game must read the same one.
        var settings = new LauncherSettings(ExtraArguments: extra + " -nosound");
        Assert.Equal(["-nosound"], LaunchArguments.For(settings));
        Assert.Equal([extra], LaunchArguments.Rejected(settings));
    }

    [Fact]
    public void QuotedReservedSwitchIsStillSeen()
    {
        var settings = new LauncherSettings(DisplayMode: "windowed", ExtraArguments: "\"-fullscreen\"");
        Assert.Equal(["-windowed"], LaunchArguments.For(settings));
    }

    [Fact]
    public void CommandLineIsTheTokensJoined()
    {
        Assert.Equal("-windowed -Key=\"a b\"", LaunchArguments.CommandLine(["-windowed", "-Key=\"a b\""]));
        Assert.Equal("", LaunchArguments.CommandLine([]));
    }
}
