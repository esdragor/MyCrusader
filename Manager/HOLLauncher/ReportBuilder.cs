using System.Globalization;
using System.IO.Compression;
using System.Runtime.InteropServices;
using System.Text;
using System.Text.Json;
using System.Text.Json.Nodes;
using System.Text.RegularExpressions;

namespace HOLLauncher;

/**
 * What the report is made of: the game folder, the launcher's folder (its own servers.json may hold another key), the
 * settings in use, the time (UTC), the launcher's status lines and errors to tell (unreadable settings...) and the
 * %LOCALAPPDATA% where a Shipping game keeps its Saved folder (GameLogs).
 */
public sealed record ReportInput(
    string GameDir,
    string LauncherDir,
    LauncherSettings Settings,
    DateTime NowUtc,
    IReadOnlyList<string> LauncherErrors,
    string LocalAppData);

/** ReportBuilder.Build: the zip's entries and what was left out, with why (also written in report.txt). */
public sealed record ReportSummary(IReadOnlyList<string> Entries, IReadOnlyList<string> LeftOut);

/**
 * "Send a report" (L20): a zip the player gives the developer by hand (nothing is uploaded). It holds
 * - logs/: the game's newest logs (GameLogs, Unreal's <Saved>\Logs\*.log: HeroesOfLiberty.log is the last run),
 * - crashes/<UECC-...>/: the newest crash folders Unreal wrote (<Saved>\Crashes, CrashDetector.CrashesDir: context,
 *   minidump, the log at the crash),
 * - servers.json with every "key" replaced by "***" (left out when unreadable),
 * - report.txt: versions, Windows, processors, memory, .NET, the settings, the launcher's status and errors, the contents and
 *   what was left out. In English whatever the launcher's language: it is read by the developer, not the player.
 * Size: logs and crash folders, newest first, up to sizeLimit bytes in all (uncompressed); at most MaxLogs logs and
 * MaxCrashes crash folders. One that no longer fits is left out (an older, smaller one may still fit), except a log,
 * whose end is kept (the crash is at the end) from a line start, which fills the limit. Every left out is said in
 * report.txt.
 * The access key (L11) never leaves the machine: every key found in the servers.json of the game folder and of the
 * launcher's folder is replaced by as many '*' in every file put in the zip, as UTF-8 and as UTF-16 (the minidump
 * holds the game's memory), same length so that a minidump keeps its layout.
 */
public static class ReportBuilder
{
    public const long DefaultSizeLimit = 50L * 1024 * 1024;
    public const int MaxLogs = 10;
    public const int MaxCrashes = 5;
    /** Below this, a log's end is not worth keeping. */
    public const long MinLogEnd = 16 * 1024;
    private const string Masked = "***";

    /** The name the save dialog proposes, with the player's local time. */
    public static string FileNameFor(DateTime localNow)
        => $"HOL-report-{localNow.ToString("yyyyMMdd-HHmmss", CultureInfo.InvariantCulture)}.zip";

    /**
     * Writes the zip to zipPath (beside it first, then moved over: never half written, an old file replaced).
     * IOException / UnauthorizedAccessException when it cannot be written; a game file that cannot be read is left
     * out and said.
     */
    public static ReportSummary Build(string zipPath, ReportInput input, long sizeLimit = DefaultSizeLimit)
    {
        var part = zipPath + ".part";
        try
        {
            ReportSummary summary;
            using (var file = new FileStream(part, FileMode.Create, FileAccess.Write))
            using (var zip = new ZipArchive(file, ZipArchiveMode.Create))
            {
                summary = Write(zip, input, sizeLimit);
            }
            File.Move(part, zipPath, overwrite: true);
            return summary;
        }
        catch
        {
            File.Delete(part);
            throw;
        }
    }

    private static ReportSummary Write(ZipArchive zip, ReportInput input, long sizeLimit)
    {
        var keys = AccessKeys(input.GameDir).Concat(AccessKeys(input.LauncherDir)).Distinct().ToList();
        var entries = new List<string>();
        var leftOut = new List<string>();

        void Add(string name, byte[] content)
        {
            var entry = zip.CreateEntry(name, CompressionLevel.Optimal);
            using var stream = entry.Open();
            stream.Write(Scrub(content, keys));
            entries.Add($"{name} ({Size(content.Length)})");
        }

        var servers = Path.Combine(input.GameDir, GameServers.FileName);
        if (File.Exists(servers))
        {
            if (MaskedServers(servers) is { } masked)
            {
                Add(GameServers.FileName, Encoding.UTF8.GetBytes(masked));
            }
            else
            {
                leftOut.Add($"{GameServers.FileName}: unreadable (it may hold the access key)");
            }
        }

        var logsDir = GameLogs.FindDir(input.GameDir, input.LocalAppData);
        var crashesDir = CrashDetector.CrashesDir(input.GameDir, input.LocalAppData);
        var items = Newest(logsDir, crashesDir, leftOut);
        var budget = sizeLimit;
        foreach (var item in items)
        {
            try
            {
                if (item.Size <= budget)
                {
                    foreach (var (name, path) in item.Files)
                    {
                        var content = ReadShared(path, long.MaxValue);
                        budget -= content.Length;
                        Add(name, content);
                    }
                }
                else if (item.IsLog && budget >= MinLogEnd)
                {
                    var (name, path) = item.Files[0];
                    var content = ReadShared(path, budget);
                    budget = 0; // full: what the cut left over is too small to be worth a file
                    Add(name, content);
                    leftOut.Add($"{item.Label}: only its end kept ({Size(content.Length)} of {Size(item.Size)}, size limit)");
                }
                else
                {
                    leftOut.Add($"{item.Label} ({Size(item.Size)}): size limit");
                }
            }
            catch (Exception e) when (e is IOException or UnauthorizedAccessException)
            {
                leftOut.Add($"{item.Label}: unreadable ({e.Message})");
            }
        }

        var report = ReportText(input, logsDir, crashesDir, entries, leftOut, sizeLimit);
        Add("report.txt", Encoding.UTF8.GetBytes(report));
        return new ReportSummary(entries, leftOut);
    }

    private sealed record Item(string Label, DateTime TimeUtc, long Size, bool IsLog, IReadOnlyList<(string Name, string Path)> Files);

    /** Logs and crash folders, newest first, within the counts (the others said in leftOut). */
    private static List<Item> Newest(string? logsDir, string? crashesDir, List<string> leftOut)
    {
        var logs = new List<Item>();
        if (logsDir is not null)
        {
            logs = new DirectoryInfo(logsDir).EnumerateFiles("*.log")
                .OrderByDescending(f => f.LastWriteTimeUtc)
                .Select(f => new Item($"logs/{f.Name}", f.LastWriteTimeUtc, f.Length, true, [($"logs/{f.Name}", f.FullName)]))
                .ToList();
        }
        var crashes = new List<Item>();
        if (crashesDir is not null)
        {
            crashes = new DirectoryInfo(crashesDir).EnumerateDirectories()
                .OrderByDescending(d => d.LastWriteTimeUtc)
                .Select(d =>
                {
                    var files = d.EnumerateFiles("*", SearchOption.AllDirectories).ToList();
                    return new Item($"crashes/{d.Name}", d.LastWriteTimeUtc, files.Sum(f => f.Length), false,
                        files.Select(f => ($"crashes/{d.Name}/{Path.GetRelativePath(d.FullName, f.FullName).Replace('\\', '/')}", f.FullName)).ToList());
                })
                .ToList();
        }
        leftOut.AddRange(logs.Skip(MaxLogs).Select(i => $"{i.Label}: older than the {MaxLogs} newest logs"));
        leftOut.AddRange(crashes.Skip(MaxCrashes).Select(i => $"{i.Label}: older than the {MaxCrashes} newest crash folders"));
        return logs.Take(MaxLogs).Concat(crashes.Take(MaxCrashes)).OrderByDescending(i => i.TimeUtc).ToList();
    }

    /**
     * The file, or its last `limit` bytes from the first line start in them, read while the game may still write it.
     * Cut at a line start so that no partial key is left at the cut.
     */
    private static byte[] ReadShared(string path, long limit)
    {
        using var stream = new FileStream(path, FileMode.Open, FileAccess.Read, FileShare.ReadWrite | FileShare.Delete);
        var length = stream.Length;
        if (length <= limit)
        {
            using var all = new MemoryStream();
            stream.CopyTo(all);
            return all.ToArray();
        }
        stream.Seek(length - limit, SeekOrigin.Begin);
        var tail = new byte[limit];
        stream.ReadExactly(tail);
        var start = Array.IndexOf(tail, (byte)'\n') + 1;
        return tail[start..];
    }

    /** Every "key" string of a servers.json (any row), as parsed and as written: an unreadable file still gives its keys. */
    private static IEnumerable<string> AccessKeys(string dir)
    {
        var file = Path.Combine(dir, GameServers.FileName);
        string text;
        try
        {
            if (!File.Exists(file))
            {
                return [];
            }
            text = File.ReadAllText(file);
        }
        catch (Exception e) when (e is IOException or UnauthorizedAccessException)
        {
            return [];
        }
        var keys = Regex.Matches(text, "\"key\"\\s*:\\s*\"((?:[^\"\\\\]|\\\\.)*)\"", RegexOptions.IgnoreCase)
            .Select(m => m.Groups[1].Value).ToList();
        try
        {
            keys.AddRange(KeyNodes(JsonNode.Parse(text)).Select(v => v.GetValue<string>()));
        }
        catch (JsonException)
        {
            // Unreadable JSON: the keys as written are scrubbed all the same.
        }
        return keys.Where(k => k != "");
    }

    /** servers.json with every "key" string replaced by "***"; null when it cannot be read as JSON. */
    private static string? MaskedServers(string file)
    {
        try
        {
            var root = JsonNode.Parse(File.ReadAllText(file));
            foreach (var value in KeyNodes(root).ToList())
            {
                value.ReplaceWith(JsonValue.Create(Masked));
            }
            return root?.ToJsonString(new JsonSerializerOptions { WriteIndented = true }) + "\n";
        }
        catch (Exception e) when (e is JsonException or IOException or UnauthorizedAccessException)
        {
            return null;
        }
    }

    private static IEnumerable<JsonValue> KeyNodes(JsonNode? node)
    {
        switch (node)
        {
            case JsonObject row:
                foreach (var (name, value) in row)
                {
                    if (string.Equals(name, "key", StringComparison.OrdinalIgnoreCase) && value is JsonValue v
                        && v.TryGetValue<string>(out _))
                    {
                        yield return v;
                    }
                    else
                    {
                        foreach (var inner in KeyNodes(value))
                        {
                            yield return inner;
                        }
                    }
                }
                break;
            case JsonArray rows:
                foreach (var value in rows)
                {
                    foreach (var inner in KeyNodes(value))
                    {
                        yield return inner;
                    }
                }
                break;
        }
    }

    /** Each key, as UTF-8 and as UTF-16, replaced by as many '*' (same length). */
    internal static byte[] Scrub(byte[] content, IReadOnlyList<string> keys)
    {
        var data = content.ToArray();
        foreach (var key in keys)
        {
            foreach (var (bytes, mask) in new[]
                     {
                         (Encoding.UTF8.GetBytes(key), (byte[])[(byte)'*']),
                         (Encoding.Unicode.GetBytes(key), (byte[])[(byte)'*', 0]),
                     })
            {
                var span = data.AsSpan();
                for (var at = span.IndexOf(bytes); at >= 0;)
                {
                    for (var i = 0; i < bytes.Length; i++)
                    {
                        span[at + i] = mask[i % mask.Length];
                    }
                    var next = span[(at + bytes.Length)..].IndexOf(bytes);
                    at = next < 0 ? -1 : at + bytes.Length + next;
                }
            }
        }
        return data;
    }

    private static string ReportText(ReportInput input, string? logsDir, string? crashesDir, List<string> entries,
        List<string> leftOut, long sizeLimit)
    {
        var text = new StringBuilder();
        text.AppendLine("Heroes of Liberty - launcher report");
        text.AppendLine($"Created (UTC): {input.NowUtc.ToString("yyyy-MM-dd HH:mm:ss", CultureInfo.InvariantCulture)}");
        text.AppendLine($"Launcher version: {LauncherInfo.Version}");
        text.AppendLine($"Game version: {Safe(() => InstalledVersion.Read(input.GameDir) ?? "not installed")}");
        text.AppendLine($"Game folder: {input.GameDir}");
        text.AppendLine($"Launcher folder: {input.LauncherDir}");
        text.AppendLine($"Manager: {Safe(() => GameServers.ReadFirst(input.GameDir) is var s ? $"{s.Name} {s.Url}" : "")}");
        text.AppendLine($"Windows: {RuntimeInformation.OSDescription} ({Environment.OSVersion.VersionString}), {RuntimeInformation.OSArchitecture}");
        text.AppendLine($"Logical processors: {Environment.ProcessorCount}");
        text.AppendLine($"Memory: {Size(GC.GetGCMemoryInfo().TotalAvailableMemoryBytes)} (as seen by .NET)");
        text.AppendLine($".NET: {RuntimeInformation.FrameworkDescription}");
        text.AppendLine($"Launcher language: {Texts.Language}");
        text.AppendLine(logsDir is null ? "No game log folder found." : $"Game logs: {logsDir}");
        text.AppendLine(crashesDir is null ? "No game crash folder found." : $"Game crashes: {crashesDir}");
        text.AppendLine();
        text.AppendLine($"Launcher settings ({LauncherSettings.FileName}):");
        text.AppendLine(input.Settings.ToJson());
        text.AppendLine();
        text.AppendLine("Launcher status and errors:");
        var errors = input.LauncherErrors.Where(e => !string.IsNullOrWhiteSpace(e)).ToList();
        text.AppendLine(errors.Count == 0 ? "- none" : string.Join(Environment.NewLine, errors.Select(e => $"- {e}")));
        text.AppendLine();
        text.AppendLine("Files:");
        foreach (var entry in entries)
        {
            text.AppendLine($"- {entry}");
        }
        text.AppendLine("- report.txt");
        text.AppendLine();
        text.AppendLine(leftOut.Count == 0
            ? $"Nothing left out (size limit {Size(sizeLimit)} for logs and crash folders, uncompressed)."
            : $"Left out (size limit {Size(sizeLimit)} for logs and crash folders, uncompressed, newest kept first):");
        foreach (var line in leftOut)
        {
            text.AppendLine($"- {line}");
        }
        return text.ToString();
    }

    private static string Safe(Func<string> read)
    {
        try
        {
            return read();
        }
        catch (Exception e) when (e is IOException or UnauthorizedAccessException or InvalidDataException)
        {
            return $"unreadable ({e.Message})";
        }
    }

    private static string Size(long bytes) => bytes >= 1024L * 1024 * 1024
        ? (bytes / (1024.0 * 1024 * 1024)).ToString("0.0", CultureInfo.InvariantCulture) + " GB"
        : bytes >= 1024 * 1024
        ? (bytes / (1024.0 * 1024)).ToString("0.0", CultureInfo.InvariantCulture) + " MB"
        : (bytes / 1024.0).ToString("0.0", CultureInfo.InvariantCulture) + " KB";
}
