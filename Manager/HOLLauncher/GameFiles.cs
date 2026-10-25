namespace HOLLauncher;

/**
 * The places of the packaged game's files the launcher relies on (L10a). The player's Windows\ folder of a package
 * (Tools/Package/package.ps1) holds HeroesOfLiberty.exe at its root (GameLauncher.ExeName) and the version in
 * HeroesOfLiberty\Version.txt, the file the game itself reads (HOLVersion.h); publish.ps1 gives it to HOLPublish with
 * --version, and the published list carries it as an ordinary file. A Version.txt at the root (L6-L9 trials) is not read.
 */
public static class GameFiles
{
    /** Relative to the game folder, '/' as in the version lists. */
    public const string VersionFile = "HeroesOfLiberty/Version.txt";
}
