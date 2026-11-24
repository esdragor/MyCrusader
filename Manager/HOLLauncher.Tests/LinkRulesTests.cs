using System.Diagnostics;

namespace HOLLauncher.Tests;

/** Launcher 5: only absolute http(s) addresses reach the shell (the home content comes over the network). */
public class LinkRulesTests
{
    [Theory]
    [InlineData("https://discord.gg/abc")]
    [InlineData("http://example.org/news?id=3")]
    [InlineData("  HTTPS://Example.org/  ")]
    public void HttpAndHttpsAreOpenable(string url)
    {
        Assert.NotNull(LinkRules.Openable(url));
    }

    [Theory]
    [InlineData(null)]
    [InlineData("")]
    [InlineData("   ")]
    [InlineData("javascript:alert(1)")]
    [InlineData("file:///C:/Windows/System32/cmd.exe")]
    [InlineData("C:\\Windows\\System32\\cmd.exe")]
    [InlineData("\\\\server\\share\\setup.exe")]
    [InlineData("ftp://example.org/x")]
    [InlineData("ms-settings:display")]
    [InlineData("/relative/path")]
    [InlineData("example.org")]
    [InlineData("mailto:someone@example.org")]
    public void AnythingElseIsNot(string? url)
    {
        Assert.Null(LinkRules.Openable(url));
    }

    [Fact]
    public void OpenHandsOnlyOpenableAddressesToTheShell()
    {
        var started = new List<ProcessStartInfo>();
        Process? Starter(ProcessStartInfo info)
        {
            started.Add(info);
            return null;
        }

        Assert.True(LinkRules.Open("https://example.org/a b", Starter));
        Assert.False(LinkRules.Open("file:///C:/x.exe", Starter));
        Assert.False(LinkRules.Open(null, Starter));

        var single = Assert.Single(started);
        Assert.Equal("https://example.org/a%20b", single.FileName);
        Assert.True(single.UseShellExecute);
    }

    [Fact]
    public void ARefusedOpenIsFalseNotAnException()
    {
        Assert.False(LinkRules.Open("https://example.org", _ => throw new System.ComponentModel.Win32Exception(2)));
    }
}
