using System.Globalization;

namespace HOLLauncher;

/**
 * What the window shows during a game update (pure, tested): the progress line, the bar and the "Cancel" button.
 * The bar counts finished files; the line names the file being fetched (numbered from 1) and the bytes received so far
 * for the whole update, in MB (1 Mo = 1024 × 1024 bytes, French decimal comma).
 */
public sealed record ProgressView(string Text, int Percent, bool CancelEnabled)
{
    private const double BytesPerMegabyte = 1024 * 1024;
    private static readonly CultureInfo French = CultureInfo.GetCultureInfo("fr-FR");

    public static ProgressView Starting() => new("Préparation de la mise à jour…", 0, true);

    public static ProgressView From(DownloadProgress progress)
    {
        var count = progress.FileCount;
        var number = Math.Min(progress.FilesDone + 1, count);
        var megabytes = (progress.BytesReceived / BytesPerMegabyte).ToString("0.0", French);
        var percent = count == 0 ? 100 : (int)(100L * Math.Min(progress.FilesDone, count) / count);
        return new ProgressView($"Fichier {number}/{count} : {progress.CurrentPath} — {megabytes} Mo", percent, true);
    }

    public ProgressView Cancelling() => this with { Text = "Annulation…", CancelEnabled = false };
}
