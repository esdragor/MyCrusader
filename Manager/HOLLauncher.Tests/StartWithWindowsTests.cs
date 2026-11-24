namespace HOLLauncher.Tests;

/** L23: "Start with Windows", against an in-memory Run key (the real HKCU key is never written by tests). */
public class StartWithWindowsTests
{
    private const string Exe = @"C:\Games\Heroes of Liberty\HOLLauncher.exe";

    private sealed class MemoryRunKey : IRunKey
    {
        public Dictionary<string, string> Values { get; } = new(StringComparer.OrdinalIgnoreCase);

        public int Writes { get; private set; }

        public string? Get(string name) => Values.GetValueOrDefault(name);

        public void Set(string name, string value)
        {
            Values[name] = value;
            Writes++;
        }

        public void Delete(string name) => Values.Remove(name);
    }

    [Fact]
    public void CommandLineStartsInTheTray()
    {
        Assert.Equal("\"C:\\Games\\Heroes of Liberty\\HOLLauncher.exe\" --tray", StartWithWindows.CommandLine(Exe, null));
        Assert.Equal("\"C:\\Games\\Heroes of Liberty\\HOLLauncher.exe\" --tray", StartWithWindows.CommandLine(Exe, "  "));
    }

    [Theory]
    [InlineData(@"D:\HOL", "\"D:\\HOL\"")]
    [InlineData(@"D:\My Games\HOL\", "\"D:\\My Games\\HOL\"")]
    [InlineData(@"D:\", "\"D:\\\\\"")]
    public void CommandLineRepeatsTheGivenGameFolder(string gameDir, string quoted)
    {
        Assert.Equal($"\"{Exe}\" --tray --game-dir {quoted}", StartWithWindows.CommandLine(Exe, gameDir));
    }

    [Fact]
    public void ARelativeGameFolderIsMadeFull()
    {
        var line = StartWithWindows.CommandLine(Exe, "HOL");
        Assert.EndsWith($"--game-dir \"{Path.GetFullPath("HOL")}\"", line);
    }

    [Theory]
    [InlineData(@"C:\a", "\"C:\\a\"")]
    [InlineData(@"C:\a b\", "\"C:\\a b\\\\\"")]
    [InlineData(@"C:\a\\", "\"C:\\a\\\\\\\\\"")]
    public void QuoteDoublesTrailingBackslashes(string argument, string expected)
    {
        Assert.Equal(expected, StartWithWindows.Quote(argument));
    }

    [Theory]
    [InlineData("\"C:\\Games\\Heroes of Liberty\\HOLLauncher.exe\" --tray", true)]
    [InlineData("\"c:\\games\\heroes of liberty\\hollauncher.exe\" --tray --game-dir \"D:\\HOL\"", true)]
    [InlineData("C:\\Games\\HOLLauncher.exe --tray", false)]
    [InlineData("\"E:\\Other\\HOLLauncher.exe\" --tray", false)]
    [InlineData("", false)]
    [InlineData(null, false)]
    [InlineData("\"", false)]
    public void StartsRecognisesThisExe(string? value, bool expected)
    {
        Assert.Equal(expected, StartWithWindows.Starts(value, Exe));
    }

    [Fact]
    public void BareExePathIsRecognised()
    {
        Assert.True(StartWithWindows.Starts(@"C:\HOL\HOLLauncher.exe --tray", @"C:\HOL\HOLLauncher.exe"));
    }

    [Fact]
    public void TurnedOnWritesTheValueOnce()
    {
        var key = new MemoryRunKey();
        Assert.Equal(RunKeyChange.Written, StartWithWindows.Sync(key, true, Exe, null));
        Assert.Equal($"\"{Exe}\" --tray", key.Values[StartWithWindows.ValueName]);
        Assert.Equal(RunKeyChange.None, StartWithWindows.Sync(key, true, Exe, null));
        Assert.Equal(1, key.Writes);
    }

    [Fact]
    public void AMovedLauncherRewritesTheValue()
    {
        var key = new MemoryRunKey();
        key.Set(StartWithWindows.ValueName, "\"C:\\Old\\HOLLauncher.exe\" --tray");
        Assert.Equal(RunKeyChange.Written, StartWithWindows.Sync(key, true, Exe, @"D:\HOL"));
        Assert.Equal($"\"{Exe}\" --tray --game-dir \"D:\\HOL\"", key.Values[StartWithWindows.ValueName]);
    }

    [Fact]
    public void TurnedOffRemovesOnlyThisLaunchersValue()
    {
        var key = new MemoryRunKey();
        key.Set(StartWithWindows.ValueName, $"\"{Exe}\" --tray");
        Assert.Equal(RunKeyChange.Removed, StartWithWindows.Sync(key, false, Exe, null));
        Assert.Empty(key.Values);
        Assert.Equal(RunKeyChange.None, StartWithWindows.Sync(key, false, Exe, null));

        // Another copy of the launcher (another game folder) owns it: left alone.
        key.Set(StartWithWindows.ValueName, "\"E:\\Other\\HOLLauncher.exe\" --tray");
        Assert.Equal(RunKeyChange.None, StartWithWindows.Sync(key, false, Exe, null));
        Assert.Single(key.Values);
    }

    [Fact]
    public void OtherRunValuesAreNeverTouched()
    {
        var key = new MemoryRunKey();
        key.Set("OneDrive", "\"C:\\OneDrive.exe\" /background");
        StartWithWindows.Sync(key, true, Exe, null);
        StartWithWindows.Sync(key, false, Exe, null);
        Assert.Equal(["OneDrive"], key.Values.Keys);
    }

    [Theory]
    [InlineData(@"D:\HOL", @"D:\HOL")]
    [InlineData(@"D:\My Games\HOL\", @"D:\My Games\HOL")]
    [InlineData(@"D:\", @"D:\")]
    public void ARunKeyStartIsParsedBackAsATrayStart(string gameDir, string expected)
    {
        // Split as Windows splits the Run value for the exe (CommandLineToArgvW), then parsed by the launcher.
        var argv = Split(StartWithWindows.CommandLine(Exe, gameDir));
        Assert.Equal(Exe, argv[0]);
        var options = LauncherOptions.Parse(argv.Skip(1).ToList(), @"C:\Games\HOL");
        Assert.True(options.StartInTray);
        Assert.Equal(expected, options.GameDirArgument);
        Assert.Equal(expected, options.GameDir);
    }

    private static string[] Split(string commandLine)
    {
        var argv = CommandLineToArgvW(commandLine, out var count);
        try
        {
            return Enumerable.Range(0, count)
                .Select(i => System.Runtime.InteropServices.Marshal.PtrToStringUni(
                    System.Runtime.InteropServices.Marshal.ReadIntPtr(argv, i * IntPtr.Size))!)
                .ToArray();
        }
        finally
        {
            LocalFree(argv);
        }
    }

    [System.Runtime.InteropServices.DllImport("shell32.dll", CharSet = System.Runtime.InteropServices.CharSet.Unicode)]
    private static extern IntPtr CommandLineToArgvW(string commandLine, out int count);

    [System.Runtime.InteropServices.DllImport("kernel32.dll")]
    private static extern IntPtr LocalFree(IntPtr memory);
}
