namespace HOLManager.Versions;

/** Settings of the versions served to the launcher (L2), apart from ManagerSettings: same sources (appsettings.json, --Key=Value). */
public sealed class VersionSettings
{
    /** The versions folder laid out as in LauncherInProgress.md, L2 (relative: from the manager's folder). */
    public string VersionsDir { get; init; } = "Versions";

    public string ResolveDir() => Path.GetFullPath(VersionsDir, AppContext.BaseDirectory);
}
