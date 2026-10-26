using System.Diagnostics;

namespace HOLLauncher.Tests;

/** The real HDiffPatch binaries of the repository (Tools/ThirdParty/HDiffPatch), found from the test's bin folder. */
public static class HDiffPatchTool
{
    public static string Folder { get; } = Find();

    public static string Hpatchz => Path.Combine(Folder, "hpatchz.exe");

    /** A real patch from oldContent to newContent, made by hdiffz with HOLPublish's options. */
    public static byte[] Diff(byte[] oldContent, byte[] newContent)
    {
        using var work = new TempDir();
        var oldFile = work.Write("old", oldContent);
        var newFile = work.Write("new", newContent);
        var patch = Path.Combine(work.Root, "patch");
        var start = new ProcessStartInfo(Path.Combine(Folder, "hdiffz.exe"))
        {
            UseShellExecute = false,
            RedirectStandardOutput = true,
            RedirectStandardError = true,
            CreateNoWindow = true,
        };
        foreach (var arg in new[] { "-m-6", "-SD", "-c-zstd-21-24", oldFile, newFile, patch })
        {
            start.ArgumentList.Add(arg);
        }
        using var process = Process.Start(start)!;
        var output = process.StandardOutput.ReadToEndAsync();
        var error = process.StandardError.ReadToEnd();
        process.WaitForExit();
        Assert.True(process.ExitCode == 0, $"hdiffz exit {process.ExitCode}: {output.Result} {error}");
        return File.ReadAllBytes(patch);
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
