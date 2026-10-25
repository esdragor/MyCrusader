using System.Diagnostics;
using HOLUpdate;

namespace HOLLauncher;

/**
 * Applies an HDiffPatch patch (L6b) with hpatchz, run as a process like HOLPublish does: hpatchz -f <old> <patch> <new>,
 * then the new file's size and SHA-256 checked against the list. Any failure (hpatchz missing, exit code, mismatch)
 * returns its reason and leaves no output: the caller fetches the full file instead (L-Q2 bis).
 */
public sealed class PatchApplier(string hpatchz)
{
    /** hpatchz.exe beside the launcher's exe (UpdatePlan.HpatchzName), where the package puts it. */
    public static string DefaultHpatchz
        => Path.Combine(Path.GetDirectoryName(Environment.ProcessPath) ?? AppContext.BaseDirectory, UpdatePlan.HpatchzName);

    public string Hpatchz { get; } = Path.GetFullPath(hpatchz);

    /** null when newFile is in place and matches expected; else why not. */
    public async Task<string?> ApplyAsync(string oldFile, string patch, string newFile, FileEntry expected, CancellationToken cancel = default)
    {
        if (!File.Exists(Hpatchz))
        {
            return $"hpatchz not found: {Hpatchz}";
        }
        Directory.CreateDirectory(Path.GetDirectoryName(newFile)!);
        var start = new ProcessStartInfo(Hpatchz)
        {
            UseShellExecute = false,
            RedirectStandardOutput = true,
            RedirectStandardError = true,
            CreateNoWindow = true,
        };
        foreach (var arg in new[] { "-f", oldFile, patch, newFile })
        {
            start.ArgumentList.Add(arg);
        }
        string? failure;
        using (var process = Process.Start(start))
        {
            if (process is null)
            {
                return $"{Hpatchz} did not start";
            }
            var output = process.StandardOutput.ReadToEndAsync(cancel);
            var error = process.StandardError.ReadToEndAsync(cancel);
            try
            {
                await process.WaitForExitAsync(cancel);
            }
            catch (OperationCanceledException)
            {
                process.Kill();
                throw;
            }
            var text = ((await output) + " " + (await error)).Trim().ReplaceLineEndings(" ");
            failure = process.ExitCode == 0 ? Check(newFile, expected)
                : $"hpatchz exit {process.ExitCode}: {text[^Math.Min(text.Length, 300)..]}";
        }
        if (failure is not null && File.Exists(newFile))
        {
            File.Delete(newFile);
        }
        return failure;
    }

    private static string? Check(string file, FileEntry expected)
    {
        if (!File.Exists(file))
        {
            return "hpatchz wrote nothing";
        }
        var size = new FileInfo(file).Length;
        if (size != expected.Size)
        {
            return $"size {size} instead of {expected.Size}";
        }
        var hash = ManifestBuilder.Hash(file);
        return hash == expected.Sha256 ? null : $"SHA-256 {hash} instead of {expected.Sha256}";
    }
}
