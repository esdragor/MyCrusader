using System.IO.Compression;
using System.Net;
using System.Net.Http.Headers;
using System.Security.Cryptography;
using System.Text.Json.Nodes;
using HOLUpdate;
using Microsoft.AspNetCore.Mvc.Testing;

namespace HOLManager.Tests;

/**
 * L2: the routes that serve the game versions to the launcher, on a real versions folder laid out as in
 * LauncherInProgress.md: latest.txt, <v>/manifest.json (HOLUpdate), notes.txt, files/<path>.gz, patches/<from>/<path>.patch,
 * launcher/latest.txt and launcher/<lv>/HOLLauncher.exe.
 */
public sealed class VersionRoutesTests : IClassFixture<WebApplicationFactory<Program>>, IDisposable
{
    private const string Latest = "2026-10-25-b";
    private const string Older = "2026-10-25-a";
    private const string Launcher = "L3";

    private readonly string root = Path.Combine(Path.GetTempPath(), $"HOL versions {Guid.NewGuid():N}");
    private readonly WebApplicationFactory<Program> factory;
    private readonly HttpClient client;
    private readonly VersionManifest manifest;
    private readonly byte[] launcherExe = RandomBytes(5000, 7);

    public VersionRoutesTests(WebApplicationFactory<Program> factory)
    {
        var versions = Path.Combine(root, "Versions");
        manifest = Publish(versions, Latest, notes: "Archers plus rapides.", patchFrom: Older);
        Publish(versions, Older, notes: null, patchFrom: null);
        File.WriteAllText(Path.Combine(versions, "latest.txt"), Latest + "\r\n");
        Directory.CreateDirectory(Path.Combine(versions, "launcher", Launcher));
        File.WriteAllText(Path.Combine(versions, "launcher", "latest.txt"), Launcher);
        File.WriteAllBytes(Path.Combine(versions, "launcher", Launcher, "HOLLauncher.exe"), launcherExe);
        this.factory = factory;
        client = With(versions).CreateClient();
    }

    public void Dispose()
    {
        ProcessServerLauncherTests.DeleteDir(root);
    }

    private WebApplicationFactory<Program> With(string versionsDir) =>
        factory.WithWebHostBuilder(b => b.UseSetting("VersionsDir", versionsDir));

    private string VersionsDir => Path.Combine(root, "Versions");

    private static byte[] RandomBytes(int size, int seed)
    {
        var bytes = new byte[size];
        new Random(seed).NextBytes(bytes);
        return bytes;
    }

    /** A fake game folder, its list by HOLUpdate, and its files gzipped into <versions>/<v>/files, as L4 will publish them. */
    private VersionManifest Publish(string versions, string version, string? notes, string? patchFrom)
    {
        var game = Path.Combine(root, "Game " + version);
        Directory.CreateDirectory(Path.Combine(game, "HeroesOfLiberty", "Content", "Paks"));
        File.WriteAllText(Path.Combine(game, "Version.txt"), version);
        File.WriteAllBytes(Path.Combine(game, "HeroesOfLiberty.exe"), RandomBytes(200_000, version[^1]));
        File.WriteAllBytes(Path.Combine(game, "HeroesOfLiberty", "Content", "Paks", "HeroesOfLiberty-Windows.ucas"), RandomBytes(50_000, 3));
        File.WriteAllBytes(Path.Combine(game, "HeroesOfLiberty.pdb"), RandomBytes(1000, 4));
        var list = ManifestBuilder.Build(game);

        var dir = Path.Combine(versions, version);
        foreach (var file in list.Files)
        {
            var gz = Path.Combine(dir, "files", file.Path + ".gz");
            Directory.CreateDirectory(Path.GetDirectoryName(gz)!);
            using var output = File.Create(gz);
            using var zip = new GZipStream(output, CompressionLevel.Fastest);
            using var input = File.OpenRead(Path.Combine(game, file.Path));
            input.CopyTo(zip);
        }
        list.Save(Path.Combine(dir, "manifest.json"));
        if (notes != null)
        {
            File.WriteAllText(Path.Combine(dir, "notes.txt"), notes);
        }
        if (patchFrom != null)
        {
            Directory.CreateDirectory(Path.Combine(dir, "patches", patchFrom));
            File.WriteAllBytes(Path.Combine(dir, "patches", patchFrom, "HeroesOfLiberty.exe.patch"), RandomBytes(300, 9));
            Directory.CreateDirectory(Path.Combine(dir, "patches", "2026-10-24"));
        }
        // Next to the files but not in the list: must never be served.
        File.WriteAllText(Path.Combine(dir, "files", "secret.txt.gz"), "not in the list");
        return list;
    }

    private static async Task AssertError(HttpResponseMessage response, HttpStatusCode status, string error)
    {
        Assert.Equal(status, response.StatusCode);
        var body = JsonNode.Parse(await response.Content.ReadAsStringAsync())!;
        Assert.Equal(error, (string?)body["error"]);
        Assert.False(string.IsNullOrEmpty((string?)body["message"]));
    }

    private static string Sha256(byte[] bytes) => Convert.ToHexString(SHA256.HashData(bytes)).ToLowerInvariant();

    private static byte[] Gunzip(byte[] bytes)
    {
        using var zip = new GZipStream(new MemoryStream(bytes), CompressionMode.Decompress);
        using var output = new MemoryStream();
        zip.CopyTo(output);
        return output.ToArray();
    }

    [Fact]
    public async Task LatestGivesTheListNotesPatchesAndLauncher()
    {
        var response = await client.GetAsync("/versions/latest");
        Assert.Equal(HttpStatusCode.OK, response.StatusCode);
        var body = JsonNode.Parse(await response.Content.ReadAsStringAsync())!;
        Assert.Equal(1, (int?)body["format"]);
        Assert.Equal(Latest, (string?)body["version"]);
        Assert.Equal(manifest.ToJson(), VersionManifest.FromJson(body["manifest"]!.ToJsonString()).ToJson());
        Assert.Equal("Archers plus rapides.", (string?)body["notes"]);
        Assert.Equal(new[] { "2026-10-24", Older }, body["patchesFrom"]!.AsArray().Select(n => (string?)n).ToArray());
        Assert.Equal(Launcher, (string?)body["launcherVersion"]);
    }

    [Fact]
    public async Task LatestWithoutNotesNorLauncherGivesEmptyTexts()
    {
        File.WriteAllText(Path.Combine(VersionsDir, "latest.txt"), Older);
        File.Delete(Path.Combine(VersionsDir, "launcher", "latest.txt"));
        var body = JsonNode.Parse(await client.GetStringAsync("/versions/latest"))!;
        Assert.Equal(Older, (string?)body["version"]);
        Assert.Equal("", (string?)body["notes"]);
        Assert.Empty(body["patchesFrom"]!.AsArray());
        Assert.Equal("", (string?)body["launcherVersion"]);
    }

    [Fact]
    public async Task NoVersionPublishedIs404()
    {
        var empty = Path.Combine(root, "Empty");
        Directory.CreateDirectory(empty);
        await AssertError(await With(empty).CreateClient().GetAsync("/versions/latest"), HttpStatusCode.NotFound, "no_version");
        await AssertError(await With(Path.Combine(root, "Missing")).CreateClient().GetAsync("/versions/latest"),
            HttpStatusCode.NotFound, "no_version");
    }

    [Fact]
    public async Task LatestNamingAMissingVersionIs404()
    {
        File.WriteAllText(Path.Combine(VersionsDir, "latest.txt"), "2027-01-01");
        await AssertError(await client.GetAsync("/versions/latest"), HttpStatusCode.NotFound, "no_version");
    }

    [Fact]
    public async Task InvalidListIs500WithAMessage()
    {
        File.WriteAllText(Path.Combine(VersionsDir, Latest, "manifest.json"),
            """{ "format": 1, "version": "x", "files": [ { "path": "../evil.exe", "size": 1, "sha256": "00" } ] }""");
        await AssertError(await client.GetAsync("/versions/latest"), HttpStatusCode.InternalServerError, "bad_manifest");
    }

    [Fact]
    public async Task FileGivesTheGzipWhoseContentHasTheListHash()
    {
        foreach (var entry in manifest.Files)
        {
            var response = await client.GetAsync($"/versions/{Latest}/files/{entry.Path}");
            Assert.Equal(HttpStatusCode.OK, response.StatusCode);
            Assert.Equal("application/gzip", response.Content.Headers.ContentType?.MediaType);
            var bytes = await response.Content.ReadAsByteArrayAsync();
            Assert.Equal(File.ReadAllBytes(Path.Combine(VersionsDir, Latest, "files", entry.Path + ".gz")), bytes);
            var content = Gunzip(bytes);
            Assert.Equal(entry.Size, content.Length);
            Assert.Equal(entry.Sha256, Sha256(content));
        }
    }

    [Fact]
    public async Task FileResumesWithRange()
    {
        var gz = File.ReadAllBytes(Path.Combine(VersionsDir, Latest, "files", "HeroesOfLiberty.exe.gz"));
        var request = new HttpRequestMessage(HttpMethod.Get, $"/versions/{Latest}/files/HeroesOfLiberty.exe");
        request.Headers.Range = new RangeHeaderValue(1000, 1999);
        var response = await client.SendAsync(request);
        Assert.Equal(HttpStatusCode.PartialContent, response.StatusCode);
        Assert.Equal(gz.Length, response.Content.Headers.ContentRange?.Length);
        Assert.Equal(gz[1000..2000], await response.Content.ReadAsByteArrayAsync());

        var rest = new HttpRequestMessage(HttpMethod.Get, $"/versions/{Latest}/files/HeroesOfLiberty.exe");
        rest.Headers.Range = new RangeHeaderValue(2000, null);
        var tail = await client.SendAsync(rest);
        Assert.Equal(HttpStatusCode.PartialContent, tail.StatusCode);
        Assert.Equal(gz[2000..], await tail.Content.ReadAsByteArrayAsync());
    }

    [Theory]
    [InlineData("secret.txt")]
    [InlineData("HeroesOfLiberty.pdb")]
    [InlineData("heroesofliberty.exe")]
    [InlineData("..%2Fmanifest.json")]
    [InlineData("HeroesOfLiberty/..%2F..%2Fnotes.txt")]
    [InlineData("C:%2FWindows%2Fwin.ini")]
    public async Task FileOutsideTheListIs404(string path)
    {
        await AssertError(await client.GetAsync($"/versions/{Latest}/files/{path}"), HttpStatusCode.NotFound, "file_not_found");
    }

    /** Dot segments, even encoded, are removed from the URL before routing: the request never reaches a file. */
    [Theory]
    [InlineData("%2E%2E/%2E%2E/latest.txt")]
    [InlineData("../../latest.txt")]
    public async Task DotSegmentsNeverReachAFile(string path)
    {
        var response = await client.GetAsync($"/versions/{Latest}/files/{path}");
        Assert.Equal(HttpStatusCode.NotFound, response.StatusCode);
        Assert.DoesNotContain(Latest, await response.Content.ReadAsStringAsync());
    }

    [Fact]
    public async Task FileOfAnUnknownVersionIs404()
    {
        await AssertError(await client.GetAsync("/versions/2027-01-01/files/Version.txt"), HttpStatusCode.NotFound, "version_not_found");
    }

    [Theory]
    [InlineData("..")]
    [InlineData("%2E%2E")]
    [InlineData("a%20b")]
    [InlineData("a%5Cb")]
    [InlineData("v%3A1")]
    public async Task InvalidVersionNameIsRefused(string version)
    {
        var response = await client.GetAsync($"/versions/{version}/files/Version.txt");
        Assert.True(response.StatusCode is HttpStatusCode.BadRequest or HttpStatusCode.NotFound, $"{version}: {response.StatusCode}");
        var patch = await client.GetAsync($"/versions/{Latest}/patches/{version}/HeroesOfLiberty.exe");
        Assert.True(patch.StatusCode is HttpStatusCode.BadRequest or HttpStatusCode.NotFound, $"{version}: {patch.StatusCode}");
    }

    [Fact]
    public async Task InvalidVersionNameIs400WithTheErrorBody()
    {
        await AssertError(await client.GetAsync("/versions/a%20b/files/Version.txt"), HttpStatusCode.BadRequest, "invalid_version");
    }

    [Fact]
    public async Task PatchIsServedWithRange()
    {
        var file = File.ReadAllBytes(Path.Combine(VersionsDir, Latest, "patches", Older, "HeroesOfLiberty.exe.patch"));
        var response = await client.GetAsync($"/versions/{Latest}/patches/{Older}/HeroesOfLiberty.exe");
        Assert.Equal(HttpStatusCode.OK, response.StatusCode);
        Assert.Equal(file, await response.Content.ReadAsByteArrayAsync());

        var request = new HttpRequestMessage(HttpMethod.Get, $"/versions/{Latest}/patches/{Older}/HeroesOfLiberty.exe");
        request.Headers.Range = new RangeHeaderValue(100, null);
        var tail = await client.SendAsync(request);
        Assert.Equal(HttpStatusCode.PartialContent, tail.StatusCode);
        Assert.Equal(file[100..], await tail.Content.ReadAsByteArrayAsync());
    }

    [Fact]
    public async Task MissingPatchIs404()
    {
        // In the list but no patch for it (the launcher then takes the full file); unknown origin version; not in the list.
        await AssertError(await client.GetAsync($"/versions/{Latest}/patches/{Older}/Version.txt"), HttpStatusCode.NotFound, "patch_not_found");
        await AssertError(await client.GetAsync($"/versions/{Latest}/patches/2020-01-01/HeroesOfLiberty.exe"),
            HttpStatusCode.NotFound, "patch_not_found");
        await AssertError(await client.GetAsync($"/versions/{Latest}/patches/{Older}/..%2F..%2Fmanifest.json"),
            HttpStatusCode.NotFound, "file_not_found");
    }

    [Fact]
    public async Task LauncherIsServedWithVersionHashAndRange()
    {
        var response = await client.GetAsync("/launcher/latest");
        Assert.Equal(HttpStatusCode.OK, response.StatusCode);
        Assert.Equal(launcherExe, await response.Content.ReadAsByteArrayAsync());
        Assert.Equal(Launcher, response.Headers.GetValues("X-Launcher-Version").Single());
        Assert.Equal(Sha256(launcherExe), response.Headers.GetValues("X-Launcher-Sha256").Single());

        var request = new HttpRequestMessage(HttpMethod.Get, "/launcher/latest");
        request.Headers.Range = new RangeHeaderValue(4000, null);
        var tail = await client.SendAsync(request);
        Assert.Equal(HttpStatusCode.PartialContent, tail.StatusCode);
        Assert.Equal(launcherExe[4000..], await tail.Content.ReadAsByteArrayAsync());
        Assert.Equal(Sha256(launcherExe), tail.Headers.GetValues("X-Launcher-Sha256").Single());
    }

    [Fact]
    public async Task NoLauncherPublishedIs404()
    {
        File.Delete(Path.Combine(VersionsDir, "launcher", "latest.txt"));
        await AssertError(await client.GetAsync("/launcher/latest"), HttpStatusCode.NotFound, "launcher_not_found");
        File.WriteAllText(Path.Combine(VersionsDir, "launcher", "latest.txt"), "L9");
        await AssertError(await client.GetAsync("/launcher/latest"), HttpStatusCode.NotFound, "launcher_not_found");
    }
}
