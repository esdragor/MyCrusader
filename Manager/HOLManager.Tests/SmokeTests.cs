using Microsoft.AspNetCore.Mvc.Testing;

namespace HOLManager.Tests;

public class SmokeTests(WebApplicationFactory<Program> factory) : IClassFixture<WebApplicationFactory<Program>>
{
    [Fact]
    public async Task AppStartsAndAnswers()
    {
        var response = await factory.CreateClient().GetAsync("/no-such-route");
        Assert.Equal(System.Net.HttpStatusCode.NotFound, response.StatusCode);
    }
}
