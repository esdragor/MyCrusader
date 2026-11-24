using Microsoft.Win32;

namespace HOLLauncher;

/** The values of a "Run" registry key (L23): the real one is RegistryRunKey, tests give an in-memory one. */
public interface IRunKey
{
    string? Get(string name);

    void Set(string name, string value);

    void Delete(string name);
}

/** HKCU\Software\Microsoft\Windows\CurrentVersion\Run: what Windows starts at the user's logon (no admin rights needed). */
public sealed class RegistryRunKey : IRunKey
{
    public const string KeyPath = @"Software\Microsoft\Windows\CurrentVersion\Run";

    public string? Get(string name)
    {
        using var key = Registry.CurrentUser.OpenSubKey(KeyPath, writable: false);
        return key?.GetValue(name) as string;
    }

    public void Set(string name, string value)
    {
        using var key = Registry.CurrentUser.CreateSubKey(KeyPath, writable: true);
        key.SetValue(name, value, RegistryValueKind.String);
    }

    public void Delete(string name)
    {
        using var key = Registry.CurrentUser.OpenSubKey(KeyPath, writable: true);
        key?.DeleteValue(name, throwOnMissingValue: false);
    }
}

/** What StartWithWindows.Sync changed. */
public enum RunKeyChange
{
    None,
    Written,
    Removed,
}

/**
 * "Start with Windows" (L23): the value HeroesOfLibertyLauncher of the user's Run key, "<exe>" --tray [--game-dir
 * "<folder>"], so that the launcher starts hidden in the notification area at logon (and checks for updates there).
 * --game-dir is written only when this start was given one (else the settings' game folder is read at logon, as from
 * the desktop shortcut). Sync runs at every start and after "Save" in the settings: the value follows the exe (the
 * launcher's own update keeps the exe's name and folder, so the value stays right; a launcher moved elsewhere rewrites
 * it at its next start). Turned off, the value is removed only when it starts this exe: another copy of the launcher
 * (another game folder) may own it.
 */
public static class StartWithWindows
{
    public const string ValueName = "HeroesOfLibertyLauncher";
    public const string TrayOption = "--tray";

    /** The Run value's command line: the exe and each argument quoted for CommandLineToArgvW. */
    public static string CommandLine(string exePath, string? gameDirArgument)
    {
        var parts = new List<string> { Quote(exePath), TrayOption };
        if (!string.IsNullOrWhiteSpace(gameDirArgument))
        {
            parts.Add("--game-dir");
            // Made full: a relative --game-dir would be read against the folder Windows starts the Run key from.
            parts.Add(Quote(Path.TrimEndingDirectorySeparator(Path.GetFullPath(gameDirArgument.Trim()))));
        }
        return string.Join(' ', parts);
    }

    /**
     * Always quoted (a path may hold spaces). A path never holds a quote; its trailing backslashes are doubled so that the
     * closing quote is not read as an escaped one ("D:\" would otherwise end as D:" plus the rest of the line).
     */
    public static string Quote(string argument)
    {
        var trailing = argument.Length - argument.TrimEnd('\\').Length;
        return "\"" + argument + new string('\\', trailing) + "\"";
    }

    /** Whether a Run value starts this exe (its first, quoted or bare, token; case and form of the path ignored). */
    public static bool Starts(string? value, string exePath)
    {
        if (string.IsNullOrWhiteSpace(value))
        {
            return false;
        }
        var text = value.Trim();
        string first;
        if (text.StartsWith('"'))
        {
            var end = text.IndexOf('"', 1);
            first = end < 0 ? text[1..] : text[1..end];
        }
        else
        {
            var space = text.IndexOf(' ');
            first = space < 0 ? text : text[..space];
        }
        try
        {
            return string.Equals(Path.GetFullPath(first), Path.GetFullPath(exePath), StringComparison.OrdinalIgnoreCase);
        }
        catch (Exception e) when (e is ArgumentException or NotSupportedException or PathTooLongException)
        {
            return false;
        }
    }

    /**
     * Makes the Run key agree with the setting. IOException / UnauthorizedAccessException / SecurityException when the
     * registry refuses (the caller says it; nothing else depends on it).
     */
    public static RunKeyChange Sync(IRunKey key, bool enabled, string exePath, string? gameDirArgument)
    {
        var current = key.Get(ValueName);
        if (enabled)
        {
            var wanted = CommandLine(exePath, gameDirArgument);
            if (current == wanted)
            {
                return RunKeyChange.None;
            }
            key.Set(ValueName, wanted);
            return RunKeyChange.Written;
        }
        if (current is not null && Starts(current, exePath))
        {
            key.Delete(ValueName);
            return RunKeyChange.Removed;
        }
        return RunKeyChange.None;
    }
}
