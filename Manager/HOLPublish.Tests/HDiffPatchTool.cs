using System.Diagnostics;

namespace HOLPublish.Tests;

/** The real HDiffPatch binaries of the repository (Tools/ThirdParty/HDiffPatch), found from the test's bin folder. */
public static class HDiffPatchTool
{
    public static string Folder { get; } = Find();

    public static string Hdiffz => Path.Combine(Folder, "hdiffz.exe");

    /** hpatchz <old> <patch> <new>, as the launcher will run it (L6). */
    public static void Apply(string oldFile, string patch, string newFile)
    {
        var start = new ProcessStartInfo(Path.Combine(Folder, "hpatchz.exe"))
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
        using var process = Process.Start(start)!;
        var output = process.StandardOutput.ReadToEndAsync();
        var error = process.StandardError.ReadToEnd();
        process.WaitForExit();
        Assert.True(process.ExitCode == 0, $"hpatchz exit {process.ExitCode}: {output.Result} {error}");
    }

    private static string Find()
    {
        for (var dir = new DirectoryInfo(AppContext.BaseDirectory); dir != null; dir = dir.Parent)
        {
            var folder = Path.Combine(dir.FullName, "Tools", "ThirdParty", "HDiffPatch");
            if (File.Exists(Path.Combine(folder, "hdiffz.exe")))
            {
                return folder;
            }
        }
        throw new FileNotFoundException($"Tools/ThirdParty/HDiffPatch/hdiffz.exe not found above {AppContext.BaseDirectory}.");
    }
}
