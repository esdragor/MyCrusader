using System.Globalization;
using System.Net;
using System.Text.Json.Nodes;
using HOLUpdate;
using Microsoft.AspNetCore.Mvc.Testing;

namespace HOLManager.Tests;

/**
 * L21: /versions/latest gives the notes of every kept version (history), newest first in publication order
 * (published.txt, not the names), from latest.txt's version down: a version published after it (latest.txt set back by
 * hand) is not in it. A version without notes.txt is listed with empty notes.
 */
public sealed class VersionHistoryTests : IClassFixture<WebApplicationFactory<Program>>, IDisposable
{
    private readonly string root = Path.Combine(Path.GetTempPath(), $"HOL history {Guid.NewGuid():N}");
    private readonly WebApplicationFactory<Program> factory;

    public VersionHistoryTests(WebApplicationFactory<Program> factory)
    {
        this.factory = factory;
    }

    public void Dispose()
    {
        ProcessServerLauncherTests.DeleteDir(root);
    }

    private static DateTime Day(int day) => new(2026, 11, day, 10, 0, 0, DateTimeKind.Utc);

    private void Publish(string version, DateTime published, string? notes)
    {
        var dir = Path.Combine(root, version);
        Directory.CreateDirectory(dir);
        new VersionManifest(version, []).Save(Path.Combine(dir, "manifest.json"));
        File.WriteAllText(Path.Combine(dir, "published.txt"), published.ToString("O", CultureInfo.InvariantCulture));
        if (notes != null)
        {
            File.WriteAllText(Path.Combine(dir, "notes.txt"), notes);
        }
    }

    private async Task<JsonNode> Latest(string query = "")
    {
        var client = factory.WithWebHostBuilder(b => b.UseSetting("VersionsDir", root)).CreateClient();
        var response = await client.GetAsync("/versions/latest" + query);
        Assert.Equal(HttpStatusCode.OK, response.StatusCode);
        return JsonNode.Parse(await response.Content.ReadAsStringAsync())!;
    }

    [Fact]
    public async Task HistoryListsTheKeptVersionsNotesNewestFirst()
    {
        // Names against the order: 0.3.10 is newer than 0.3.9.
        Publish("0.3.9", Day(1), "Archers plus rapides.\r\n");
        Publish("0.3.10", Day(2), notes: null);
        Publish("0.3.11", Day(3), "Nouvelle carte.");
        Directory.CreateDirectory(Path.Combine(root, "0.3.12.tmp"));
        File.WriteAllText(Path.Combine(root, "latest.txt"), "0.3.11\r\n");

        var body = await Latest();

        Assert.Equal(1, (int?)body["format"]);
        Assert.Equal("Nouvelle carte.", (string?)body["notes"]);
        var history = body["history"]!.AsArray();
        Assert.Equal(["0.3.11", "0.3.10", "0.3.9"], history.Select(h => (string?)h!["version"]));
        Assert.Equal(["Nouvelle carte.", "", "Archers plus rapides."], history.Select(h => (string?)h!["notes"]));
        Assert.Equal(Day(1), DateTime.Parse((string)history[2]!["published"]!, CultureInfo.InvariantCulture,
            DateTimeStyles.AdjustToUniversal | DateTimeStyles.AssumeUniversal));
    }

    [Fact]
    public async Task VersionsPublishedAfterTheLatestAreNotInTheHistory()
    {
        Publish("0.3.9", Day(1), "Un.");
        Publish("0.3.10", Day(2), "Deux.");
        Publish("0.3.11", Day(3), "Trois.");
        File.WriteAllText(Path.Combine(root, "latest.txt"), "0.3.10");

        var history = (await Latest())["history"]!.AsArray();

        Assert.Equal(["0.3.10", "0.3.9"], history.Select(h => (string?)h!["version"]));
    }

    /**
     * Notes in the launcher's language: <v>/notes.<lang>.txt for ?lang=<lang>, else notes.txt (French, the language the
     * notes are written in); an unknown or malformed language is French too.
     */
    [Fact]
    public async Task NotesFollowTheAskedLanguageElseFrench()
    {
        Publish("0.3.9", Day(1), "Archers plus rapides.");
        File.WriteAllText(Path.Combine(root, "0.3.9", "notes.en.txt"), "Faster archers.\r\n");
        Publish("0.3.10", Day(2), "Nouvelle carte.");
        File.WriteAllText(Path.Combine(root, "0.3.10", "notes.ja.txt"), "新しいマップ。");
        File.WriteAllText(Path.Combine(root, "latest.txt"), "0.3.10");

        var english = await Latest("?lang=en");
        Assert.Equal("Nouvelle carte.", (string?)english["notes"]);
        Assert.Equal(["Nouvelle carte.", "Faster archers."], english["history"]!.AsArray().Select(h => (string?)h!["notes"]));

        var japanese = await Latest("?lang=ja");
        Assert.Equal("新しいマップ。", (string?)japanese["notes"]);
        Assert.Equal(["新しいマップ。", "Archers plus rapides."], japanese["history"]!.AsArray().Select(h => (string?)h!["notes"]));

        foreach (var query in new[] { "", "?lang=de", "?lang=../en", "?lang=EN" })
        {
            Assert.Equal(["Nouvelle carte.", "Archers plus rapides."],
                (await Latest(query))["history"]!.AsArray().Select(h => (string?)h!["notes"]));
        }
    }

    [Fact]
    public async Task SingleVersionHasItselfAsHistory()
    {
        Publish("0.3.9", Day(1), "Un.");
        File.WriteAllText(Path.Combine(root, "latest.txt"), "0.3.9");

        var entry = Assert.Single((await Latest())["history"]!.AsArray());

        Assert.Equal("0.3.9", (string?)entry!["version"]);
        Assert.Equal("Un.", (string?)entry["notes"]);
    }
}
