namespace HOLLauncher;

/**
 * The game's command line built from the launch options of the settings (L19), pure. The game is an Unreal Engine 5
 * package: its standard switches -windowed / -fullscreen and -ResX= / -ResY= override the window settings the game saved
 * (the game's own Settings screen shows what they gave, HOLSettingsScreen). Defaults give no argument at all: the game
 * then keeps its own saved settings (L8).
 * Order: display mode, resolution, then the player's extra arguments (advanced, free text) split like a command line.
 * Unreal reads the raw command line itself (FCommandLine, FParse::Value reads -Key="a b"), not argv: a token keeps its
 * quotes and GameLauncher passes the tokens joined by a space, never re-quoted (CommandLine).
 * An extra token never contradicts what the launcher passes (Rejected): the display mode or resolution chosen in the
 * Settings window win over the same switch typed by hand, and -ServersFile= is the launcher's (the game must read the
 * servers.json the launcher reads, beside its exe: GameServers, HOLManagerClient::GetServersFilePath).
 */
public static class LaunchArguments
{
    /** LauncherSettings.DisplayMode values (null or any other value: the game's own). */
    public const string Windowed = "windowed";
    public const string Fullscreen = "fullscreen";

    /** Window sizes accepted, in pixels, both sides: below is unusable, above is beyond any screen (16K is 15360). */
    public const int MinSize = 320;
    public const int MaxSize = 16384;

    /** The usual sizes the Settings window offers before "Custom". */
    public static readonly IReadOnlyList<(int Width, int Height)> CommonResolutions =
    [
        (1280, 720), (1366, 768), (1600, 900), (1920, 1080), (2560, 1080), (2560, 1440), (3440, 1440), (3840, 2160),
    ];

    /** The tokens to pass to the game, in order; never throws (a hand-edited file only loses its odd values). */
    public static IReadOnlyList<string> For(LauncherSettings settings)
    {
        var arguments = new List<string>();
        if (ModeSwitch(settings.DisplayMode) is { } mode)
        {
            arguments.Add(mode);
        }
        if (Resolution(settings) is var (width, height))
        {
            arguments.Add($"-ResX={width}");
            arguments.Add($"-ResY={height}");
        }
        arguments.AddRange(Split(settings.ExtraArguments).Where(token => !IsReserved(token, settings)));
        return arguments;
    }

    /** The extra tokens For leaves out (they contradict the chosen options or are the launcher's): the window refuses them. */
    public static IReadOnlyList<string> Rejected(LauncherSettings settings)
        => Split(settings.ExtraArguments).Where(token => IsReserved(token, settings)).ToList();

    public static bool IsValidResolution(int width, int height)
        => width is >= MinSize and <= MaxSize && height is >= MinSize and <= MaxSize;

    /**
     * Splits like a command line: blanks outside double quotes separate tokens, the quotes stay in the token (Unreal
     * reads them). An unclosed quote runs to the end (HasUnclosedQuote lets the window refuse it first).
     */
    public static IReadOnlyList<string> Split(string? text)
    {
        var tokens = new List<string>();
        var current = new System.Text.StringBuilder();
        var quoted = false;
        foreach (var c in text ?? "")
        {
            if (c == '"')
            {
                quoted = !quoted;
            }
            if (!quoted && char.IsWhiteSpace(c))
            {
                if (current.Length > 0)
                {
                    tokens.Add(current.ToString());
                    current.Clear();
                }
                continue;
            }
            current.Append(c);
        }
        if (current.Length > 0)
        {
            tokens.Add(current.ToString());
        }
        return tokens;
    }

    public static bool HasUnclosedQuote(string text) => text.Count(c => c == '"') % 2 == 1;

    /** The process's argument string: the tokens as they are, a space between (see the class comment). */
    public static string CommandLine(IEnumerable<string> tokens) => string.Join(' ', tokens);

    private static string? ModeSwitch(string? mode)
    {
        if (string.Equals(mode, Windowed, StringComparison.OrdinalIgnoreCase))
        {
            return "-windowed";
        }
        return string.Equals(mode, Fullscreen, StringComparison.OrdinalIgnoreCase) ? "-fullscreen" : null;
    }

    private static (int Width, int Height)? Resolution(LauncherSettings settings)
        => settings is { ResolutionWidth: { } width, ResolutionHeight: { } height } && IsValidResolution(width, height)
            ? (width, height)
            : null;

    /** Unreal takes a switch after '-' or '/', any case; quotes around it do not hide it. */
    private static bool IsReserved(string token, LauncherSettings settings)
    {
        var bare = token.Replace("\"", "");
        if (bare.Length < 2 || bare[0] is not ('-' or '/'))
        {
            return false;
        }
        var name = bare[1..];
        bool Is(string word) => name.Equals(word, StringComparison.OrdinalIgnoreCase);
        bool Sets(string key) => name.StartsWith(key + "=", StringComparison.OrdinalIgnoreCase);
        return (ModeSwitch(settings.DisplayMode) is not null && (Is("windowed") || Is("fullscreen")))
            || (Resolution(settings) is not null && (Sets("ResX") || Sets("ResY")))
            || Sets("ServersFile");
    }
}
