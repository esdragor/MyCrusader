using System.IO.Compression;
using System.Text;

namespace HOLLauncher.Tests;

public class ReportBuilderTests
{
    private const string Key = "S3cret-Access-Key-4242";
    private const string OtherKey = "Launcher-Dir-Key-777";
    private static readonly DateTime Now = new(2026, 11, 24, 9, 30, 15, DateTimeKind.Utc);

    [Fact]
    public void ReportHoldsLogsCrashesServersAndReportText()
    {
        using var game = new TempDir();
        using var output = new TempDir();
        game.Write("HeroesOfLiberty/Version.txt", "0.4.3\n");
        game.Write("servers.json", ServersJson("http://hol.example:7700", Key));
        Log(game, "HeroesOfLiberty.log", "last run\n", Now.AddMinutes(-5));
        Log(game, "HeroesOfLiberty-backup-2026.11.23-10.00.00.log", "older run\n", Now.AddDays(-1));
        Crash(game, "UECC-Windows-A_0000", Now.AddMinutes(-6));

        var zip = Path.Combine(output.Root, "report.zip");
        var summary = ReportBuilder.Build(zip, Input(game, new LauncherSettings(Language: "ja", DisplayMode: "windowed"),
            ["Serveur injoignable : timeout"]));

        var entries = Entries(zip);
        Assert.Equal(
            new[]
            {
                "crashes/UECC-Windows-A_0000/CrashContext.runtime-xml",
                "crashes/UECC-Windows-A_0000/HeroesOfLiberty.log",
                "crashes/UECC-Windows-A_0000/UEMinidump.dmp",
                "logs/HeroesOfLiberty-backup-2026.11.23-10.00.00.log",
                "logs/HeroesOfLiberty.log",
                "report.txt",
                "servers.json",
            },
            entries.Keys.Order(StringComparer.Ordinal));
        Assert.Empty(summary.LeftOut);
        Assert.Equal("last run\n", Encoding.UTF8.GetString(entries["logs/HeroesOfLiberty.log"]));

        var report = Encoding.UTF8.GetString(entries["report.txt"]);
        Assert.Contains($"Launcher version: {LauncherInfo.Version}", report);
        Assert.Contains("Game version: 0.4.3", report);
        Assert.Contains("Created (UTC): 2026-11-24 09:30:15", report);
        Assert.Contains("Windows: ", report);
        Assert.Contains($"Logical processors: {Environment.ProcessorCount}", report);
        Assert.Contains("Memory: ", report);
        Assert.Contains(".NET: ", report);
        Assert.Contains("\"language\": \"ja\"", report);
        Assert.Contains("\"displayMode\": \"windowed\"", report);
        Assert.Contains("Serveur injoignable : timeout", report);
        Assert.Contains("http://hol.example:7700", report);
        Assert.Contains("logs/HeroesOfLiberty.log", report);
        Assert.Contains("Nothing left out", report);
    }

    [Fact]
    public void AccessKeyNeverEntersTheReport()
    {
        using var game = new TempDir();
        using var launcher = new TempDir();
        using var output = new TempDir();
        game.Write("servers.json", ServersJson("http://hol.example:7700", Key));
        launcher.Write("servers.json", ServersJson("http://other.example:7700", OtherKey));
        Log(game, "HeroesOfLiberty.log", $"LogHOL: GET /versions/latest X-HOL-Key: {Key} and {OtherKey}\n", Now.AddMinutes(-1));
        Crash(game, "UECC-Windows-A_0000", Now.AddMinutes(-2));
        // The key in a crash's log, and in the minidump as the game holds it in memory (UTF-16) and as read (UTF-8).
        game.Write("HeroesOfLiberty/Saved/Crashes/UECC-Windows-A_0000/HeroesOfLiberty.log", $"key={Key}\n");
        game.Write("HeroesOfLiberty/Saved/Crashes/UECC-Windows-A_0000/UEMinidump.dmp",
            [0, 1, .. Encoding.Unicode.GetBytes(Key), 2, 3, .. Encoding.UTF8.GetBytes(Key), 4]);
        var settings = new LauncherSettings(ExtraArguments: $"-Token={Key}");

        var zip = Path.Combine(output.Root, "report.zip");
        ReportBuilder.Build(zip, Input(game, settings, [], launcher.Root));

        var entries = Entries(zip);
        foreach (var (name, bytes) in entries)
        {
            foreach (var key in new[] { Key, OtherKey })
            {
                Assert.False(Contains(bytes, Encoding.UTF8.GetBytes(key)), $"{key} in {name} (UTF-8)");
                Assert.False(Contains(bytes, Encoding.Unicode.GetBytes(key)), $"{key} in {name} (UTF-16)");
            }
        }
        var servers = Encoding.UTF8.GetString(entries["servers.json"]);
        Assert.Contains("\"key\": \"***\"", servers);
        Assert.Contains("http://hol.example:7700", servers);
        // Same length: the minidump keeps its layout.
        Assert.Equal(new FileInfo(Path.Combine(game.Root, "HeroesOfLiberty/Saved/Crashes/UECC-Windows-A_0000/UEMinidump.dmp")).Length,
            entries["crashes/UECC-Windows-A_0000/UEMinidump.dmp"].Length);
        Assert.DoesNotContain(launcher.Root + Path.DirectorySeparatorChar + "servers.json", entries.Keys);
    }

    [Fact]
    public void UnreadableServersFileIsLeftOutButItsKeyStillScrubbed()
    {
        using var game = new TempDir();
        using var output = new TempDir();
        game.Write("servers.json", "{ \"servers\": [ { \"url\": \"http://x\", \"key\": \"" + Key + "\" } ");
        Log(game, "HeroesOfLiberty.log", $"key {Key}\n", Now.AddMinutes(-1));

        var zip = Path.Combine(output.Root, "report.zip");
        var summary = ReportBuilder.Build(zip, Input(game, LauncherSettings.Default, []));

        var entries = Entries(zip);
        Assert.DoesNotContain("servers.json", entries.Keys);
        Assert.Contains(summary.LeftOut, line => line.Contains("servers.json"));
        Assert.False(Contains(entries["logs/HeroesOfLiberty.log"], Encoding.UTF8.GetBytes(Key)));
    }

    [Fact]
    public void SizeLimitKeepsTheNewestFirstAndSaysWhatWasLeftOut()
    {
        using var game = new TempDir();
        using var output = new TempDir();
        const int size = 400_000;
        Log(game, "HeroesOfLiberty.log", Lines(size), Now.AddMinutes(-1));
        Crash(game, "UECC-Windows-Big_0000", Now.AddMinutes(-2), dumpSize: 700_000);
        Log(game, "HeroesOfLiberty-backup-1.log", Lines(size), Now.AddHours(-1));
        Log(game, "HeroesOfLiberty-backup-2.log", Lines(size) + "end of backup 2\n", Now.AddHours(-2));
        Log(game, "HeroesOfLiberty-backup-3.log", "small\n", Now.AddHours(-3));
        const long limit = 1_000_000;

        var zip = Path.Combine(output.Root, "report.zip");
        var summary = ReportBuilder.Build(zip, Input(game, LauncherSettings.Default, []), limit);

        var entries = Entries(zip);
        // The newest log; the crash folder no longer fits, the older, smaller log still does; then the end of the next.
        Assert.Contains("logs/HeroesOfLiberty.log", entries.Keys);
        Assert.DoesNotContain(entries.Keys, k => k.StartsWith("crashes/UECC-Windows-Big_0000"));
        Assert.Contains("logs/HeroesOfLiberty-backup-1.log", entries.Keys);
        Assert.EndsWith("end of backup 2\n", Encoding.UTF8.GetString(entries["logs/HeroesOfLiberty-backup-2.log"]));
        Assert.DoesNotContain("logs/HeroesOfLiberty-backup-3.log", entries.Keys);
        var gathered = entries.Where(e => e.Key.StartsWith("logs/") || e.Key.StartsWith("crashes/")).Sum(e => (long)e.Value.Length);
        Assert.True(gathered <= limit, $"{gathered} > {limit}");
        Assert.True(gathered > limit - 1_000, $"{gathered}: the limit is filled");
        Assert.Contains(summary.LeftOut, line => line.Contains("UECC-Windows-Big_0000") && line.Contains("size limit"));
        Assert.Contains(summary.LeftOut, line => line.Contains("HeroesOfLiberty-backup-2.log") && line.Contains("end"));
        Assert.Contains(summary.LeftOut, line => line.Contains("HeroesOfLiberty-backup-3.log"));
        var report = Encoding.UTF8.GetString(entries["report.txt"]);
        Assert.Contains("UECC-Windows-Big_0000", report);
        Assert.Contains("Left out", report);
    }

    [Fact]
    public void LogLargerThanTheLimitKeepsItsEnd()
    {
        using var game = new TempDir();
        using var output = new TempDir();
        var text = Lines(300_000) + "the very last line\n";
        Log(game, "HeroesOfLiberty.log", text, Now.AddMinutes(-1));
        const long limit = 100_000;

        var zip = Path.Combine(output.Root, "report.zip");
        var summary = ReportBuilder.Build(zip, Input(game, LauncherSettings.Default, []), limit);

        var log = Encoding.UTF8.GetString(Entries(zip)["logs/HeroesOfLiberty.log"]);
        Assert.True(log.Length <= limit);
        Assert.True(log.Length > limit / 2);
        Assert.EndsWith("the very last line\n", log);
        Assert.StartsWith("line ", log); // cut at a line start
        Assert.Contains(summary.LeftOut, line => line.Contains("HeroesOfLiberty.log") && line.Contains("end"));
    }

    [Fact]
    public void OldestBeyondTheCountsAreLeftOut()
    {
        using var game = new TempDir();
        using var output = new TempDir();
        for (var i = 0; i < ReportBuilder.MaxLogs + 2; i++)
        {
            Log(game, $"HeroesOfLiberty-backup-{i:00}.log", "x\n", Now.AddHours(-i));
        }
        for (var i = 0; i < ReportBuilder.MaxCrashes + 1; i++)
        {
            Crash(game, $"UECC-Windows-{i:00}_0000", Now.AddHours(-i).AddMinutes(-1));
        }

        var zip = Path.Combine(output.Root, "report.zip");
        var summary = ReportBuilder.Build(zip, Input(game, LauncherSettings.Default, []));

        var entries = Entries(zip);
        Assert.Equal(ReportBuilder.MaxLogs, entries.Keys.Count(k => k.StartsWith("logs/")));
        Assert.Equal(ReportBuilder.MaxCrashes, entries.Keys.Where(k => k.StartsWith("crashes/")).Select(k => k.Split('/')[1]).Distinct().Count());
        Assert.DoesNotContain("logs/HeroesOfLiberty-backup-11.log", entries.Keys);
        Assert.Contains(summary.LeftOut, line => line.Contains("HeroesOfLiberty-backup-11.log"));
        Assert.Contains(summary.LeftOut, line => line.Contains($"UECC-Windows-{ReportBuilder.MaxCrashes:00}_0000"));
    }

    [Fact]
    public void GameNotInstalledStillGivesAReport()
    {
        using var game = new TempDir();
        using var output = new TempDir();
        var zip = Path.Combine(output.Root, "report.zip");
        ReportBuilder.Build(zip, Input(game, LauncherSettings.Default, []));

        var entries = Entries(zip);
        Assert.Equal(["report.txt"], entries.Keys);
        var report = Encoding.UTF8.GetString(entries["report.txt"]);
        Assert.Contains("Game version: not installed", report);
        Assert.Contains("No game log", report);
    }

    [Fact]
    public void ExistingFileIsReplacedAndNoPartLeft()
    {
        using var game = new TempDir();
        using var output = new TempDir();
        var zip = output.Write("report.zip", "old");
        ReportBuilder.Build(zip, Input(game, LauncherSettings.Default, []));
        Assert.Contains("report.txt", Entries(zip).Keys);
        Assert.Equal(["report.zip"], Directory.GetFiles(output.Root).Select(Path.GetFileName));
    }

    [Fact]
    public void FileNameCarriesTheDate()
    {
        Assert.Equal("HOL-report-20261124-093015.zip", ReportBuilder.FileNameFor(new DateTime(2026, 11, 24, 9, 30, 15)));
    }

    [Fact]
    public void ShippingGameFilesAreReadUnderLocalAppData()
    {
        using var game = new TempDir();
        using var output = new TempDir();
        var file = game.Write("LocalAppData/HeroesOfLiberty/Saved/Logs/HeroesOfLiberty.log", "shipping run\n");
        File.SetLastWriteTimeUtc(file, Now.AddMinutes(-1));
        Crash(game, "UECC-Windows-S_0000", Now.AddMinutes(-2), saved: "LocalAppData/HeroesOfLiberty/Saved");

        var zip = Path.Combine(output.Root, "report.zip");
        ReportBuilder.Build(zip, Input(game, LauncherSettings.Default, []));

        var entries = Entries(zip);
        Assert.Contains("logs/HeroesOfLiberty.log", entries.Keys);
        Assert.Contains("crashes/UECC-Windows-S_0000/UEMinidump.dmp", entries.Keys);
    }

    /** The game's %LOCALAPPDATA% is a folder of the test (the player's real one is never read). */
    private static ReportInput Input(TempDir game, LauncherSettings settings, IReadOnlyList<string> errors, string? launcherDir = null)
        => new(game.Root, launcherDir ?? game.Root, settings, Now, errors, Path.Combine(game.Root, "LocalAppData"));

    private static string ServersJson(string url, string key)
        => "{\n  \"servers\": [\n    { \"name\": \"Test\", \"url\": \"" + url + "\", \"key\": \"" + key + "\" }\n  ]\n}\n";

    private static string Lines(int size)
    {
        var text = new StringBuilder();
        for (var i = 0; text.Length < size; i++)
        {
            text.Append("line ").Append(i).Append(" of the game log, some words to fill it\n");
        }
        return text.ToString();
    }

    private static void Log(TempDir dir, string name, string text, DateTime timeUtc)
    {
        var file = dir.Write($"HeroesOfLiberty/Saved/Logs/{name}", text);
        File.SetLastWriteTimeUtc(file, timeUtc);
    }

    private static void Crash(TempDir dir, string name, DateTime timeUtc, int dumpSize = 16, string saved = "HeroesOfLiberty/Saved")
    {
        var folder = $"{saved}/Crashes/{name}";
        dir.Write($"{folder}/CrashContext.runtime-xml", "<FGenericCrashContext><RuntimeProperties><CrashType>Crash</CrashType></RuntimeProperties></FGenericCrashContext>");
        dir.Write($"{folder}/HeroesOfLiberty.log", "crash log\n");
        dir.Write($"{folder}/UEMinidump.dmp", new byte[dumpSize]);
        Directory.SetLastWriteTimeUtc(Path.Combine(dir.Root, folder), timeUtc);
    }

    private static Dictionary<string, byte[]> Entries(string zip)
    {
        using var archive = ZipFile.OpenRead(zip);
        return archive.Entries.ToDictionary(e => e.FullName, e =>
        {
            using var stream = e.Open();
            using var copy = new MemoryStream();
            stream.CopyTo(copy);
            return copy.ToArray();
        });
    }

    private static bool Contains(byte[] data, byte[] part)
        => data.AsSpan().IndexOf(part) >= 0;
}
