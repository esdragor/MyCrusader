namespace HOLUpdate;

/** One file of a version (L1): path relative to the game folder, always with '/', size in bytes, SHA-256 in lowercase hex. */
public sealed record FileEntry(string Path, long Size, string Sha256);
