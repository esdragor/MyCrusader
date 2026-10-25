using System.Net;
using System.Text;

namespace HOLLauncher.Tests;

/** Answers every request with the given function, and keeps the requested addresses. */
public sealed class FakeHandler(Func<HttpRequestMessage, HttpResponseMessage> answer) : HttpMessageHandler
{
    public List<Uri> Requests { get; } = [];

    public static FakeHandler Json(HttpStatusCode code, string body) => new(_ => new HttpResponseMessage(code)
    {
        Content = new StringContent(body, Encoding.UTF8, "application/json"),
    });

    public static FakeHandler Throwing(Exception e) => new(_ => throw e);

    protected override Task<HttpResponseMessage> SendAsync(HttpRequestMessage request, CancellationToken cancellationToken)
    {
        Requests.Add(request.RequestUri!);
        return Task.FromResult(answer(request));
    }
}
