using System.Text;

namespace HOLLauncher.Tests;

/** A real temporary folder for a test (a game folder), deleted at the end. */
public sealed class TempDir : IDisposable
{
    public TempDir()
    {
        Root = Path.Combine(Path.GetTempPath(), "HOLLauncher.Tests", Guid.NewGuid().ToString("N"));
        Directory.CreateDirectory(Root);
    }

    public string Root { get; }

    /** Writes a file at a relative path ('/' or '\'), creating its folders. */
    public string Write(string relativePath, byte[] content)
    {
        var file = Path.Combine(Root, relativePath.Replace('/', Path.DirectorySeparatorChar));
        Directory.CreateDirectory(Path.GetDirectoryName(file)!);
        File.WriteAllBytes(file, content);
        return file;
    }

    public string Write(string relativePath, string content) => Write(relativePath, Encoding.UTF8.GetBytes(content));

    public void Dispose()
    {
        try
        {
            Directory.Delete(Root, recursive: true);
        }
        catch (IOException)
        {
            // Left in %TEMP%: never fails a test.
        }
    }
}
