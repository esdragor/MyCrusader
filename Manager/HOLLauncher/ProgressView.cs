namespace HOLLauncher;

/**
 * What the window shows during a game or launcher update (pure, tested): the progress line, the bar and the "Cancel" button.
 * The bar counts finished files; the line names the file being fetched (numbered from 1) and the bytes received so far
 * for the whole update, in MB (1024 × 1024 bytes). L16: the texts in the given language (null: the one in use), the
 * numbers in its own format (12,4 Mo in French, 12.4 MB in English and Japanese).
 */
public sealed record ProgressView(string Text, int Percent, bool CancelEnabled)
{
    private const double BytesPerMegabyte = 1024 * 1024;

    public static ProgressView Starting(string? language = null)
        => new(Texts.Get("ProgressStarting", Texts.LanguageOr(language)), 0, true);

    public static ProgressView From(DownloadProgress progress, string? language = null)
    {
        var lang = Texts.LanguageOr(language);
        var count = progress.FileCount;
        var number = Math.Min(progress.FilesDone + 1, count);
        var percent = count == 0 ? 100 : (int)(100L * Math.Min(progress.FilesDone, count) / count);
        return new ProgressView(Texts.FormatIn(lang, "ProgressFile", number, count, progress.CurrentPath,
            Megabytes(progress.BytesReceived, lang)), percent, true);
    }

    /** The launcher's own update (L7): one file, the bar follows its bytes when the size is known. */
    public static ProgressView FromLauncher(LauncherProgress progress, string? language = null)
    {
        var lang = Texts.LanguageOr(language);
        var received = Megabytes(progress.BytesReceived, lang);
        if (progress.TotalBytes is not { } total || total <= 0)
        {
            return new ProgressView(Texts.FormatIn(lang, "ProgressLauncher", progress.Version, received), 0, true);
        }
        var percent = (int)(100L * Math.Min(progress.BytesReceived, total) / total);
        return new ProgressView(Texts.FormatIn(lang, "ProgressLauncherTotal", progress.Version, received, Megabytes(total, lang)),
            percent, true);
    }

    public ProgressView Cancelling(string? language = null)
        => this with { Text = Texts.Get("ProgressCancelling", Texts.LanguageOr(language)), CancelEnabled = false };

    private static string Megabytes(long bytes, string language) => Texts.Number(bytes / BytesPerMegabyte, "0.0", language);
}
