using System.Diagnostics;

namespace HOLPublish;

/**
 * The HDiffPatch tools run as processes (L3: HDiffPatch 5.1.3, Tools/ThirdParty/HDiffPatch): hdiffz makes a patch with
 * the options measured in L3, hpatchz (same folder) applies it, to check it the way the launcher will (L6).
 */
public sealed class HDiffPatch(string hdiffz)
{
    public static readonly IReadOnlyList<string> DiffOptions = ["-m-6", "-SD", "-c-zstd-21-24"];

    public string Hdiffz { get; } = Path.GetFullPath(hdiffz);

    public string Hpatchz => Path.Combine(Path.GetDirectoryName(Hdiffz)!, "hpatchz.exe");

    public void Diff(string oldFile, string newFile, string patch) => Run(Hdiffz, [.. DiffOptions, oldFile, newFile, patch]);

    public void Apply(string oldFile, string patch, string newFile) => Run(Hpatchz, ["-f", oldFile, patch, newFile]);

    private static void Run(string exe, IEnumerable<string> args)
    {
        if (!File.Exists(exe))
        {
            throw new PublishException($"{Path.GetFileNameWithoutExtension(exe)} not found: {exe}");
        }
        var start = new ProcessStartInfo(exe)
        {
            UseShellExecute = false,
            RedirectStandardOutput = true,
            RedirectStandardError = true,
            CreateNoWindow = true,
        };
        foreach (var arg in args)
        {
            start.ArgumentList.Add(arg);
        }
        using var process = Process.Start(start) ?? throw new PublishException($"{exe} did not start.");
        var output = process.StandardOutput.ReadToEndAsync();
        var error = process.StandardError.ReadToEnd();
        process.WaitForExit();
        if (process.ExitCode != 0)
        {
            var text = (output.Result + " " + error).Trim().ReplaceLineEndings(" ");
            throw new PublishException($"{Path.GetFileName(exe)} exit {process.ExitCode}: {text[^Math.Min(text.Length, 300)..]}");
        }
    }
}
