namespace HOLLauncher.Tests;

public class CrashDetectorTests
{
    private static readonly DateTime LastCheck = new(2026, 11, 20, 12, 0, 0, DateTimeKind.Utc);
    private const string ClosedLog = "[2026.11.21-10.00.00:000][  0]LogInit: started\n"
        + "[2026.11.21-10.05.00:000][175]LogExit: Exiting.\n"
        + "[2026.11.21-10.05.00:010][175]Log file closed, 11/21/26 11:05:00\n";
    private const string CrashedLog = "[2026.11.21-10.00.00:000][  0]LogInit: started\n"
        + "[2026.11.21-10.05.00:000][264]LogOutputDevice: Error: [Callstack] 0x00007ff89b72caec ntdll.dll!UnknownFunction []\n"
        + "[2026.11.21-10.05.00:001][264]LogOutputDevice: Error: \n";

    [Fact]
    public void CrashFolderNewerThanTheLastCheckIsACrash()
    {
        using var dir = new TempDir();
        Crash(dir, "UECC-Windows-A_0000", "Crash", LastCheck.AddHours(1));
        Assert.True(CrashDetector.Check(dir.Root, LastCheck, gameRunning: false, NoAppData(dir)).Crashed);
    }

    [Fact]
    public void AssertFolderIsACrash()
    {
        using var dir = new TempDir();
        Crash(dir, "UECC-Windows-A_0000", "Assert", LastCheck.AddMinutes(5));
        Assert.True(CrashDetector.Check(dir.Root, LastCheck, gameRunning: false, NoAppData(dir)).Crashed);
    }

    [Fact]
    public void CrashFolderOlderThanTheLastCheckIsNotACrash()
    {
        using var dir = new TempDir();
        Crash(dir, "UECC-Windows-A_0000", "Crash", LastCheck.AddDays(-1));
        Assert.False(CrashDetector.Check(dir.Root, LastCheck, gameRunning: false, NoAppData(dir)).Crashed);
    }

    [Fact]
    public void FirstRunFlagsNothing()
    {
        using var dir = new TempDir();
        Crash(dir, "UECC-Windows-A_0000", "Crash", DateTime.UtcNow);
        Log(dir, CrashedLog, DateTime.UtcNow);
        Assert.False(CrashDetector.Check(dir.Root, null, gameRunning: false, NoAppData(dir)).Crashed);
    }

    [Fact]
    public void EnsureFolderIsNotACrash()
    {
        using var dir = new TempDir();
        // Unreal writes a folder for an ensure too; the game went on (seen in Saved\Crashes: <CrashType>Ensure).
        Crash(dir, "UECC-Windows-A_0000", "Ensure", LastCheck.AddHours(1));
        Assert.False(CrashDetector.Check(dir.Root, LastCheck, gameRunning: false, NoAppData(dir)).Crashed);
    }

    [Fact]
    public void CrashFolderWithoutContextIsACrash()
    {
        using var dir = new TempDir();
        var folder = dir.Write("HeroesOfLiberty/Saved/Crashes/UECC-Windows-B_0000/UEMinidump.dmp", new byte[] { 1, 2 });
        Directory.SetLastWriteTimeUtc(Path.GetDirectoryName(folder)!, LastCheck.AddHours(1));
        Assert.True(CrashDetector.Check(dir.Root, LastCheck, gameRunning: false, NoAppData(dir)).Crashed);
    }

    [Fact]
    public void LogEndingWithoutTheClosedLineIsACrash()
    {
        using var dir = new TempDir();
        Log(dir, CrashedLog, LastCheck.AddHours(1));
        Assert.True(CrashDetector.Check(dir.Root, LastCheck, gameRunning: false, NoAppData(dir)).Crashed);
    }

    [Fact]
    public void LogEndingWithTheClosedLineIsNotACrash()
    {
        using var dir = new TempDir();
        Log(dir, ClosedLog + "\r\n\r\n", LastCheck.AddHours(1));
        Assert.False(CrashDetector.Check(dir.Root, LastCheck, gameRunning: false, NoAppData(dir)).Crashed);
    }

    [Fact]
    public void UnclosedLogOfARunningGameIsNotACrash()
    {
        using var dir = new TempDir();
        Log(dir, CrashedLog, LastCheck.AddHours(1));
        Assert.False(CrashDetector.Check(dir.Root, LastCheck, gameRunning: true, NoAppData(dir)).Crashed);
    }

    [Fact]
    public void UnclosedLogOlderThanTheLastCheckIsNotACrash()
    {
        using var dir = new TempDir();
        Log(dir, CrashedLog, LastCheck.AddHours(-1));
        Assert.False(CrashDetector.Check(dir.Root, LastCheck, gameRunning: false, NoAppData(dir)).Crashed);
    }

    [Fact]
    public void LongLogIsReadFromItsEnd()
    {
        using var dir = new TempDir();
        Log(dir, new string('x', 100_000) + "\n" + ClosedLog, LastCheck.AddHours(1));
        Assert.False(CrashDetector.Check(dir.Root, LastCheck, gameRunning: false, NoAppData(dir)).Crashed);
    }

    [Fact]
    public void NoGameFilesIsNotACrash()
    {
        using var dir = new TempDir();
        Assert.False(CrashDetector.Check(dir.Root, LastCheck, gameRunning: false, NoAppData(dir)).Crashed);
        Assert.False(CrashDetector.Check(Path.Combine(dir.Root, "missing"), LastCheck, gameRunning: false, NoAppData(dir)).Crashed);
    }

    [Fact]
    public void ShippingGameIsLookedForUnderLocalAppData()
    {
        using var dir = new TempDir();
        // A Shipping package writes its Saved folder under %LOCALAPPDATA% (GameLogs, L17).
        Crash(dir, "UECC-Windows-A_0000", "Crash", LastCheck.AddHours(1), "LocalAppData/HeroesOfLiberty/Saved");
        dir.Write("LocalAppData/HeroesOfLiberty/Saved/Logs/HeroesOfLiberty.log", ClosedLog);
        Assert.True(CrashDetector.Check(Path.Combine(dir.Root, "game"), LastCheck, gameRunning: false, NoAppData(dir)).Crashed);
    }

    /** A %LOCALAPPDATA% of the test's own: the player's real one is never read. */
    private static string NoAppData(TempDir dir) => Path.Combine(dir.Root, "LocalAppData");

    private static void Crash(TempDir dir, string name, string type, DateTime timeUtc, string saved = "HeroesOfLiberty/Saved")
    {
        var context = dir.Write($"{saved}/Crashes/{name}/CrashContext.runtime-xml",
            "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n<FGenericCrashContext>\n\t<RuntimeProperties>\n"
            + $"\t\t<IsEnsure>{(type == "Ensure" ? "true" : "false")}</IsEnsure>\n\t\t<CrashType>{type}</CrashType>\n"
            + "\t</RuntimeProperties>\n</FGenericCrashContext>\n");
        dir.Write($"{saved}/Crashes/{name}/HeroesOfLiberty.log", CrashedLog);
        File.SetLastWriteTimeUtc(context, timeUtc);
        Directory.SetLastWriteTimeUtc(Path.GetDirectoryName(context)!, timeUtc);
    }

    private static void Log(TempDir dir, string text, DateTime timeUtc)
    {
        var file = dir.Write("HeroesOfLiberty/Saved/Logs/HeroesOfLiberty.log", text);
        File.SetLastWriteTimeUtc(file, timeUtc);
    }
}
