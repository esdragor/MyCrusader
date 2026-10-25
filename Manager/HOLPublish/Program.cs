using System.Globalization;

namespace HOLPublish;

/** One line, PASS or FAIL, and the exit code 0 / 1 (what Tools/Package/publish.ps1 will read, L4b). */
public static class Program
{
    /** UTF-8 out ("compressés"): a script that captures the line sets [Console]::OutputEncoding to UTF-8 as well. */
    public static int Main(string[] args)
    {
        Console.OutputEncoding = new System.Text.UTF8Encoding(false);
        return Run(args, Console.Out);
    }

    public static int Run(IReadOnlyList<string> args, TextWriter output)
    {
        try
        {
            var result = Publisher.Publish(CommandLine.Parse(args));
            foreach (var warning in result.Warnings)
            {
                output.WriteLine($"HOLPublish : warning: {warning}");
            }
            output.WriteLine($"HOLPublish : PASS {result.Version} ({result.FileCount} fichiers, {Mb(result.CompressedBytes)} Mo "
                + $"compressés, {result.PatchCount} correctifs, {Mb(result.PatchBytes)} Mo)");
            return 0;
        }
        catch (Exception e)
        {
            output.WriteLine($"HOLPublish : FAIL ({e.Message})");
            return 1;
        }
    }

    /** 1 Mo = 1 MiB, one decimal with a comma, like the journals. */
    private static string Mb(long bytes) =>
        (bytes / (1024.0 * 1024.0)).ToString("0.0", CultureInfo.InvariantCulture).Replace('.', ',');
}
