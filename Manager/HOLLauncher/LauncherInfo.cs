namespace HOLLauncher;

/**
 * The launcher's own version, compared with the "launcherVersion" of /versions/latest (the text of
 * <versions>/launcher/latest.txt). A constant rather than an assembly attribute: raised by hand with each launcher
 * published, and the same name is given to the folder launcher/<lv>/. Version name rules (PROTOCOL.md).
 */
public static class LauncherInfo
{
    public const string Version = "1";
}
