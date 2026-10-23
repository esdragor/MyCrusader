using System.Net;
using System.Net.Http.Json;
using HOLManager.Lobbies;
using Microsoft.AspNetCore.Mvc.Testing;
using Microsoft.Extensions.DependencyInjection;

namespace HOLManager.Tests;

public class StatusTests(WebApplicationFactory<Program> factory) : IClassFixture<WebApplicationFactory<Program>>
{
    private sealed record Status(int ProtocolVersion, string GameVersion, int Lobbies, int Servers, int MaxServers);

    [Fact]
    public async Task StatusGivesVersionsAndCounts()
    {
        var client = factory.WithWebHostBuilder(b => b.UseSetting("GameVersion", "2026-10-23-test")).CreateClient();
        var response = await client.GetAsync("/status");
        Assert.Equal(HttpStatusCode.OK, response.StatusCode);
        var status = await response.Content.ReadFromJsonAsync<Status>();
        Assert.Equal(new Status(1, "2026-10-23-test", 0, 0, 2), status);
    }

    [Fact]
    public async Task StatusWithoutVersionFileIsDev()
    {
        var status = await factory.CreateClient().GetFromJsonAsync<Status>("/status");
        Assert.Equal("dev", status?.GameVersion);
    }

    [Fact]
    public async Task StatusCountsTheLobbies()
    {
        var app = factory.WithWebHostBuilder(b => b.UseSetting("GameVersion", "2026-10-23-test"));
        var client = app.CreateClient();
        var store = app.Services.GetRequiredService<LobbyStore>();
        store.Create("A", "Bob", "2026-10-23-test");
        store.Create("B", "Bob", "2026-10-23-test");
        var status = await client.GetFromJsonAsync<Status>("/status");
        Assert.Equal(2, status?.Lobbies);
    }
}
