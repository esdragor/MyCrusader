using System.Net;
using System.Net.Http.Json;
using System.Text.Json.Nodes;
using HOLManager.Api;
using HOLManager.Lobbies;
using HOLManager.Matches;
using Microsoft.AspNetCore.Builder;
using Microsoft.AspNetCore.Hosting;
using Microsoft.AspNetCore.Mvc.Testing;
using Microsoft.Extensions.DependencyInjection;

namespace HOLManager.Tests;

/** L11: the shared access key (X-HOL-Key, 401 bad_key, loopback exempted, -HOLAccessKey= for the game servers). */
public sealed class AccessKeyTests : IClassFixture<WebApplicationFactory<Program>>, IDisposable
{
    private const string Key = "k3y-Of_The-Kit";
    private const string Remote = "192.168.1.42";

    private readonly WebApplicationFactory<Program> factory;
    private readonly string dir = Directory.CreateTempSubdirectory("hol-key-").FullName;

    public AccessKeyTests(WebApplicationFactory<Program> factory) => this.factory = factory;

    public void Dispose() => Directory.Delete(dir, true);

    /** The app with this key file content (null = no file); the remote address is X-Test-Remote, else 127.0.0.1. */
    private HttpClient Client(string? keyFileText)
    {
        var file = Path.Combine(dir, "access_key.txt");
        if (keyFileText != null)
        {
            File.WriteAllText(file, keyFileText);
        }
        return factory.WithWebHostBuilder(b =>
        {
            b.UseSetting("AccessKeyFile", file);
            b.ConfigureServices(s => s.AddSingleton<IStartupFilter>(new RemoteAddressFilter()));
        }).CreateClient();
    }

    private static HttpRequestMessage Status(string? remote = Remote, string? key = null)
    {
        var request = new HttpRequestMessage(HttpMethod.Get, "/status");
        if (remote != null)
        {
            request.Headers.Add("X-Test-Remote", remote);
        }
        if (key != null)
        {
            request.Headers.Add(AccessKey.Header, key);
        }
        return request;
    }

    private static async Task AssertBadKey(HttpResponseMessage response)
    {
        Assert.Equal(HttpStatusCode.Unauthorized, response.StatusCode);
        var body = await response.Content.ReadFromJsonAsync<JsonNode>();
        Assert.Equal("bad_key", (string?)body!["error"]);
        Assert.False(string.IsNullOrEmpty((string?)body["message"]));
        Assert.DoesNotContain(Key, body.ToJsonString());
    }

    [Theory]
    [InlineData(null)]
    [InlineData("")]
    [InlineData("  \r\n")]
    public async Task WithoutKeyEverythingPasses(string? keyFileText)
    {
        var response = await Client(keyFileText).SendAsync(Status());
        Assert.Equal(HttpStatusCode.OK, response.StatusCode);
    }

    [Fact]
    public async Task WithKeyNoHeaderIsBadKey() => await AssertBadKey(await Client(Key).SendAsync(Status()));

    [Fact]
    public async Task WithKeyWrongHeaderIsBadKey()
    {
        var client = Client(Key);
        await AssertBadKey(await client.SendAsync(Status(key: "nope")));
        await AssertBadKey(await client.SendAsync(Status(key: Key + "x")));
        await AssertBadKey(await client.SendAsync(Status(key: "")));
    }

    [Fact]
    public async Task WithKeyTheRightHeaderPasses()
    {
        // The file may end with a line break (written by hand or by make_kit.ps1).
        var response = await Client(Key + "\r\n").SendAsync(Status(key: Key));
        Assert.Equal(HttpStatusCode.OK, response.StatusCode);
    }

    [Fact]
    public async Task WithKeyEveryRouteIsGuarded()
    {
        var client = Client(Key);
        foreach (var route in new[] { "/lobbies", "/versions/latest", "/launcher/latest", "/no-such-route" })
        {
            var request = new HttpRequestMessage(HttpMethod.Get, route);
            request.Headers.Add("X-Test-Remote", Remote);
            await AssertBadKey(await client.SendAsync(request));
        }
    }

    [Theory]
    [InlineData("127.0.0.1")]
    [InlineData("::1")]
    [InlineData("::ffff:127.0.0.1")]
    public async Task WithKeyLoopbackIsExempted(string remote)
    {
        var response = await Client(Key).SendAsync(Status(remote: remote));
        Assert.Equal(HttpStatusCode.OK, response.StatusCode);
    }

    [Theory]
    [InlineData("127.0.0.1", true)]
    [InlineData("127.4.5.6", true)]
    [InlineData("::1", true)]
    [InlineData("::ffff:127.0.0.1", true)]
    [InlineData("192.168.1.19", false)]
    [InlineData("::ffff:192.168.1.19", false)]
    [InlineData("90.55.168.58", false)]
    [InlineData("fe80::1", false)]
    public void IsExemptOnlyForLoopback(string address, bool exempt) =>
        Assert.Equal(exempt, AccessKey.IsExempt(IPAddress.Parse(address)));

    [Fact]
    public void NoRemoteAddressIsNotExempted() => Assert.False(AccessKey.IsExempt(null));

    [Fact]
    public void ReadTrimsAndGivesNullWhenAbsentOrEmpty()
    {
        var file = Path.Combine(dir, "k.txt");
        Assert.Null(AccessKey.Read(file));
        File.WriteAllText(file, " \n");
        Assert.Null(AccessKey.Read(file));
        File.WriteAllText(file, $"\uFEFF{Key}\n");
        Assert.Equal(Key, AccessKey.Read(file));
    }

    [Fact]
    public void AccessKeyFileIsRelativeToTheManagerFolder()
    {
        Assert.Equal(Path.Combine(AppContext.BaseDirectory, "access_key.txt"), new ManagerSettings().ResolveAccessKeyFile());
        Assert.Equal(@"D:\k.txt", new ManagerSettings { AccessKeyFile = @"D:\k.txt" }.ResolveAccessKeyFile());
    }

    [Fact]
    public void ServerArgumentsCarryTheKey()
    {
        var map = LobbyRules.DefaultMaps[0];
        var without = ProcessServerLauncher.BuildArguments(map, 7778, @"C:\m\M1", "http://127.0.0.1:7700", "M1");
        Assert.DoesNotContain("HOLAccessKey", without);
        var with = ProcessServerLauncher.BuildArguments(map, 7778, @"C:\m\M1", "http://127.0.0.1:7700", "M1", Key);
        Assert.Equal(without + $" -HOLAccessKey={Key}", with);
        Assert.Equal(without, ProcessServerLauncher.BuildArguments(map, 7778, @"C:\m\M1", "http://127.0.0.1:7700", "M1", null));
    }

    /** The test server has no remote address: 127.0.0.1 unless the X-Test-Remote header gives another one. */
    private sealed class RemoteAddressFilter : IStartupFilter
    {
        public Action<IApplicationBuilder> Configure(Action<IApplicationBuilder> next) => app =>
        {
            app.Use(async (context, nextMiddleware) =>
            {
                context.Connection.RemoteIpAddress = context.Request.Headers.TryGetValue("X-Test-Remote", out var remote)
                    ? IPAddress.Parse(remote.ToString())
                    : IPAddress.Loopback;
                await nextMiddleware(context);
            });
            next(app);
        };
    }
}
