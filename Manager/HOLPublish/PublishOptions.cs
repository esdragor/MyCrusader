namespace HOLPublish;

/** The command line of HOLPublish (L4a). Hdiffz null: hdiffz.exe next to HOLPublish.exe. */
public sealed record PublishOptions(string GameDir, string VersionsDir, string? Version = null, string? NotesFile = null,
    int Keep = 3, string? Hdiffz = null);

/** What a publication wrote: files and their gzipped size, patches kept and their size, older versions removed. */
public sealed record PublishResult(string Version, int FileCount, long CompressedBytes, int PatchCount, long PatchBytes,
    IReadOnlyList<string> Pruned, IReadOnlyList<string> Warnings);

/** A publication refused or failed: nothing was published (no <v>\, latest.txt unchanged). */
public sealed class PublishException(string message, Exception? inner = null) : Exception(message, inner);
