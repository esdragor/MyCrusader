using System.Net;
using System.Text.RegularExpressions;
using HOLManager.Lobbies;

namespace HOLManager.Api;

/**
 * L12: what the manager, reachable from the Internet, says about its requests (PROTOCOL.md, Journal). A refusal (401
 * bad_key): one line per address and per minute, then a count (RefusalLimiter). Accepted from outside: one line at
 * Information, the polls of the game (GET /status, /lobbies, /lobbies/{id}, /lobbies/{id}/watch) at Debug; a file
 * served: one line with its bytes and 200/206. From the loopback (game servers, local trials): Debug only. Never the
 * key nor the headers.
 */
public static partial class RequestLog
{
    public const string Category = "HOLManager.Requests";

    /** Set by UseAccessKey on a request it refuses. */
    internal const string RefusedItem = "HOLManager.Refused";

    public static bool IsFrequentRead(string method, string path) =>
        HttpMethods.IsGet(method) && FrequentRead().IsMatch(path);

    public static bool IsDownload(string method, string path) =>
        HttpMethods.IsGet(method) && (path == "/launcher/latest" || Download().IsMatch(path));

    /** First of all middlewares: it sees the final status, also of the errors written by UseLobbyErrors. */
    public static void UseRequestLog(this WebApplication app)
    {
        var logger = app.Services.GetRequiredService<ILoggerFactory>().CreateLogger(Category);
        var limiter = new RefusalLimiter(app.Services.GetRequiredService<IClock>());
        app.Use(async (context, next) =>
        {
            await next(context);
            var request = context.Request;
            var path = request.Path.Value ?? "";
            var address = context.Connection.RemoteIpAddress;
            var remote = (address is { IsIPv4MappedToIPv6: true } ? address.MapToIPv4() : address)?.ToString() ?? "?";
            if (context.Items.ContainsKey(RefusedItem))
            {
                var (summaries, write) = limiter.Note(remote);
                foreach (var summary in summaries)
                {
                    logger.LogWarning("{Summary}", summary);
                }
                if (write)
                {
                    logger.LogWarning("Refused {Method} {Path} from {Remote}: {Error}.", request.Method, path, remote, LobbyErrors.BadKey);
                }
                return;
            }
            var status = context.Response.StatusCode;
            var level = AccessKey.IsExempt(address) || IsFrequentRead(request.Method, path) ? LogLevel.Debug : LogLevel.Information;
            if (IsDownload(request.Method, path) && status is StatusCodes.Status200OK or StatusCodes.Status206PartialContent)
            {
                logger.Log(level, "Served {Path} to {Remote}: {Status}, {Bytes} bytes.", path, remote, status,
                    context.Response.ContentLength);
            }
            else
            {
                logger.Log(level, "{Method} {Path} from {Remote}: {Status}.", request.Method, path, remote, status);
            }
        });
    }

    [GeneratedRegex("^/(status|lobbies(/[^/]+(/watch)?)?)$")]
    private static partial Regex FrequentRead();

    [GeneratedRegex("^/versions/[^/]+/(files|patches)/")]
    private static partial Regex Download();
}

/**
 * At most one refusal line per address and per Window: the others are counted and the count is written ("12 more
 * refused from 1.2.3.4 in the last minute.") with the first refusal, from any address, after the window is over. At
 * most MaxTracked addresses are followed; beyond, the others share one window.
 */
public sealed class RefusalLimiter(IClock clock)
{
    public static readonly TimeSpan Window = TimeSpan.FromMinutes(1);
    public const int MaxTracked = 1024;
    public const string OtherAddresses = "other addresses";

    private readonly object gate = new();
    private readonly Dictionary<string, (DateTimeOffset Start, int More)> windows = [];

    /** The counts due of the windows over, then whether this refusal gets its line. */
    public (IReadOnlyList<string> Summaries, bool Write) Note(string remote)
    {
        lock (gate)
        {
            var now = clock.UtcNow;
            var summaries = new List<string>();
            foreach (var (address, window) in windows.Where(w => now - w.Value.Start >= Window).ToList())
            {
                if (window.More > 0)
                {
                    summaries.Add($"{window.More} more refused from {address} in the last minute.");
                }
                windows.Remove(address);
            }
            var key = windows.Count >= MaxTracked && !windows.ContainsKey(remote) ? OtherAddresses : remote;
            if (windows.TryGetValue(key, out var current))
            {
                windows[key] = (current.Start, current.More + 1);
                return (summaries, false);
            }
            windows[key] = (now, 0);
            return (summaries, true);
        }
    }
}
