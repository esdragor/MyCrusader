using System.Collections.Concurrent;
using System.Net;
using HOLManager.Api;
using Microsoft.AspNetCore.Builder;
using Microsoft.AspNetCore.Hosting;
using Microsoft.AspNetCore.Mvc.Testing;
using Microsoft.Extensions.DependencyInjection;
using Microsoft.Extensions.Logging;

namespace HOLManager.Tests;

/** Collects the log lines written, for the tests that check what the manager says. */
public sealed class CapturingLoggerProvider : ILoggerProvider
{
    public ConcurrentQueue<(string Category, LogLevel Level, string Message)> Entries { get; } = new();

    public ILogger CreateLogger(string categoryName) => new Logger(this, categoryName);

    public IEnumerable<string> Messages(string category, LogLevel atLeast) =>
        Entries.Where(e => e.Category == category && e.Level >= atLeast).Select(e => e.Message);

    public void Dispose()
    {
    }

    private sealed class Logger(CapturingLoggerProvider provider, string category) : ILogger
    {
        public IDisposable? BeginScope<TState>(TState state) where TState : notnull => null;

        public bool IsEnabled(LogLevel logLevel) => true;

        public void Log<TState>(LogLevel logLevel, EventId eventId, TState state, Exception? exception,
            Func<TState, Exception?, string> formatter) => provider.Entries.Enqueue((category, logLevel, formatter(state, exception)));
    }
}

/** L12: the manager's request log (refusals limited per address, outside requests, loopback and polls quiet). */
public sealed class RequestLogTests : IClassFixture<WebApplicationFactory<Program>>, IDisposable
{
    private const string Key = "k3y-Of_The-Kit";
    private const string Remote = "192.168.1.42";

    private readonly WebApplicationFactory<Program> factory;
    private readonly string dir = Directory.CreateTempSubdirectory("hol-log-").FullName;
    private readonly CapturingLoggerProvider log = new();

    public RequestLogTests(WebApplicationFactory<Program> factory) => this.factory = factory;

    public void Dispose() => Directory.Delete(dir, true);

    private HttpClient Client()
    {
        var file = Path.Combine(dir, "access_key.txt");
        File.WriteAllText(file, Key);
        return factory.WithWebHostBuilder(b =>
        {
            b.UseSetting("AccessKeyFile", file);
            b.UseSetting("LogDir", "");
            b.ConfigureLogging(l => l.AddProvider(log).AddFilter(RequestLog.Category, LogLevel.Debug));
            b.ConfigureServices(s => s.AddSingleton<IStartupFilter>(new RemoteAddressFilter()));
        }).CreateClient();
    }

    private static HttpRequestMessage Get(string route, string remote = Remote, string? key = null)
    {
        var request = new HttpRequestMessage(HttpMethod.Get, route);
        request.Headers.Add("X-Test-Remote", remote);
        if (key != null)
        {
            request.Headers.Add(AccessKey.Header, key);
        }
        return request;
    }

    private string[] Lines(LogLevel atLeast = LogLevel.Information) => log.Messages(RequestLog.Category, atLeast).ToArray();

    [Fact]
    public async Task RefusalIsLoggedWithoutTheKey()
    {
        var response = await Client().SendAsync(Get("/lobbies?x=1", key: Key + "x"));
        Assert.Equal(HttpStatusCode.Unauthorized, response.StatusCode);
        var line = Assert.Single(Lines());
        Assert.Contains(Remote, line);
        Assert.Contains("GET /lobbies", line);
        Assert.Contains("bad_key", line);
        Assert.DoesNotContain(Key, line);
        Assert.DoesNotContain(AccessKey.Header, line);
        Assert.All(log.Entries, e => Assert.DoesNotContain(Key, e.Message));
    }

    [Fact]
    public async Task RefusalsAreLimitedPerAddress()
    {
        var client = Client();
        for (var i = 0; i < 5; i++)
        {
            await client.SendAsync(Get("/status"));
        }
        await client.SendAsync(Get("/status", remote: "10.0.0.7"));
        var lines = Lines();
        Assert.Equal(2, lines.Length);
        Assert.Single(lines, l => l.Contains(Remote));
        Assert.Single(lines, l => l.Contains("10.0.0.7"));
    }

    [Fact]
    public async Task AcceptedOutsideRequestIsLoggedAtInformation()
    {
        var response = await Client().SendAsync(Get("/versions/latest", key: Key));
        var line = Assert.Single(Lines());
        Assert.Contains($"GET /versions/latest from {Remote}: {(int)response.StatusCode}", line);
        Assert.DoesNotContain(Key, line);
    }

    [Theory]
    [InlineData("/status")]
    [InlineData("/lobbies")]
    [InlineData("/lobbies/L1")]
    [InlineData("/lobbies/L1/watch")]
    public async Task PollsAreLoggedAtDebug(string route)
    {
        await Client().SendAsync(Get(route, key: Key));
        Assert.Empty(Lines());
        Assert.Single(Lines(LogLevel.Debug));
    }

    [Fact]
    public async Task LoopbackIsNotLoggedAtInformation()
    {
        var client = Client();
        await client.SendAsync(Get("/versions/latest", remote: "127.0.0.1"));
        await client.SendAsync(Get("/no-such-route", remote: "::1"));
        Assert.Empty(Lines());
        Assert.Equal(2, Lines(LogLevel.Debug).Length);
    }

    [Theory]
    [InlineData("GET", "/status", true)]
    [InlineData("GET", "/lobbies", true)]
    [InlineData("GET", "/lobbies/L1", true)]
    [InlineData("GET", "/lobbies/L1/watch", true)]
    [InlineData("POST", "/lobbies", false)]
    [InlineData("POST", "/lobbies/L1/join", false)]
    [InlineData("GET", "/versions/latest", false)]
    public void FrequentReads(string method, string path, bool frequent) =>
        Assert.Equal(frequent, RequestLog.IsFrequentRead(method, path));

    [Theory]
    [InlineData("/versions/L11-v1/files/HeroesOfLiberty.exe.gz", true)]
    [InlineData("/versions/L11-v2/patches/L11-v1/a/b.pak.patch", true)]
    [InlineData("/launcher/latest", true)]
    [InlineData("/versions/latest", false)]
    [InlineData("/status", false)]
    public void Downloads(string path, bool download) => Assert.Equal(download, RequestLog.IsDownload("GET", path));

    [Fact]
    public void LimiterWritesOneLinePerAddressAndMinuteThenTheCount()
    {
        var clock = new FakeClock();
        var limiter = new RefusalLimiter(clock);
        Assert.Equal(("", true), Run(limiter.Note("1.2.3.4")));
        for (var i = 0; i < 12; i++)
        {
            Assert.Equal(("", false), Run(limiter.Note("1.2.3.4")));
        }
        Assert.Equal(("", true), Run(limiter.Note("5.6.7.8")));
        clock.UtcNow += TimeSpan.FromSeconds(59);
        Assert.Equal(("", false), Run(limiter.Note("1.2.3.4")));
        clock.UtcNow += TimeSpan.FromSeconds(1);
        // The window of 1.2.3.4 is over: its count first (13 kept silent); 5.6.7.8 had nothing more.
        Assert.Equal(("13 more refused from 1.2.3.4 in the last minute.", true), Run(limiter.Note("1.2.3.4")));
        clock.UtcNow += TimeSpan.FromMinutes(1);
        Assert.Equal(("", true), Run(limiter.Note("1.2.3.4")));
    }

    [Fact]
    public void LimiterTracksABoundedNumberOfAddresses()
    {
        var limiter = new RefusalLimiter(new FakeClock());
        for (var i = 0; i < RefusalLimiter.MaxTracked; i++)
        {
            Assert.True(limiter.Note($"10.0.{i / 256}.{i % 256}").Write);
        }
        // Beyond, the other addresses share one window.
        Assert.True(limiter.Note("11.0.0.1").Write);
        Assert.False(limiter.Note("11.0.0.2").Write);
    }

    private static (string, bool) Run((IReadOnlyList<string> Summaries, bool Write) note) => (string.Join("|", note.Summaries), note.Write);

    /** The test server has no remote address: the X-Test-Remote header gives it. */
    private sealed class RemoteAddressFilter : IStartupFilter
    {
        public Action<IApplicationBuilder> Configure(Action<IApplicationBuilder> next) => app =>
        {
            app.Use(async (context, nextMiddleware) =>
            {
                context.Connection.RemoteIpAddress = IPAddress.Parse(context.Request.Headers["X-Test-Remote"].ToString());
                await nextMiddleware(context);
            });
            next(app);
        };
    }
}
