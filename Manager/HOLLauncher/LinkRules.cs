using System.ComponentModel;
using System.Diagnostics;

namespace HOLLauncher;

/**
 * Which addresses the launcher opens in the browser (pure, tested): only absolute http and https ones. The home content
 * is written by the host, but it travels over the network and the shell would happily run a file: or a custom scheme
 * (a command line, an installer): anything else is never handed to it.
 */
public static class LinkRules
{
    /** The address as a Uri if it is absolute http(s) with a host; null otherwise (blank, relative, other scheme). */
    public static Uri? Openable(string? url)
    {
        if (string.IsNullOrWhiteSpace(url) || !Uri.TryCreate(url.Trim(), UriKind.Absolute, out var uri))
        {
            return null;
        }
        return (uri.Scheme == Uri.UriSchemeHttp || uri.Scheme == Uri.UriSchemeHttps) && uri.Host != "" ? uri : null;
    }

    /**
     * Opens an openable address in the default browser; false when it is not openable or the shell refused (nothing to
     * say to the player: the link simply did nothing). starter replaces Process.Start (tests).
     */
    public static bool Open(string? url, Func<ProcessStartInfo, Process?>? starter = null)
    {
        if (Openable(url) is not { } uri)
        {
            return false;
        }
        try
        {
            (starter ?? Process.Start)(new ProcessStartInfo(uri.AbsoluteUri) { UseShellExecute = true })?.Dispose();
            return true;
        }
        catch (Exception e) when (e is Win32Exception or IOException or InvalidOperationException)
        {
            return false;
        }
    }
}
