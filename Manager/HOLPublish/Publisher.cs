using System.IO.Compression;
using HOLUpdate;

namespace HOLPublish;

/**
 * Publishes a player folder Windows\ into the versions folder (L4a), laid out as VersionStore reads it: manifest.json
 * (HOLUpdate), notes.txt, files/<path>.gz, patches/<from>/<path>.patch. Everything is written into <v>.tmp, then renamed
 * <v>, then latest.txt is replaced, last: a launcher never sees half a version, and a failure publishes nothing.
 * Patches: from each of the `keep` newest versions already published, for each file changed (ManifestDiff), hdiffz on
 * the old file (its .gz unpacked, checked against its list) and the new one; kept only if smaller than the new .gz, and
 * only once hpatchz rebuilt the new file's SHA-256 from it. Then only `keep` versions are kept besides the new one.
 * D481: with a server folder (WindowsServer\), it is copied without its .pdb into <servers>\<v>.tmp, renamed <v> just
 * before the version's folder and latest.txt; old servers are never removed here (the manager removes them once no
 * match runs on them).
 */
public static class Publisher
{
    private const string WorkDir = "_work";
    public const string ServerExe = "HeroesOfLibertyServer.exe";

    public static PublishResult Publish(PublishOptions options)
    {
        var game = Path.GetFullPath(options.GameDir);
        if (!Directory.Exists(game))
        {
            throw new PublishException($"game folder not found: {game}");
        }
        if (options.Keep < 0)
        {
            throw new PublishException($"keep must be >= 0, not {options.Keep}");
        }
        var root = Path.GetFullPath(options.VersionsDir);
        var version = options.Version ?? ReadVersionTxt(game);
        if (!VersionFolder.IsPublishable(version))
        {
            throw new PublishException($"invalid version name '{version}' (letters, digits, '.', '_', '-'; not 'launcher' nor '*.tmp')");
        }
        var final = Path.Combine(root, version);
        if (Directory.Exists(final) || File.Exists(final))
        {
            throw new PublishException($"version {version} already published in {root}");
        }
        string? server = null, serverFinal = null;
        if (options.ServerDir != null || options.ServersDir != null)
        {
            if (options.ServerDir == null || options.ServersDir == null)
            {
                throw new PublishException("a server folder and a servers folder go together");
            }
            server = Path.GetFullPath(options.ServerDir);
            if (!File.Exists(Path.Combine(server, ServerExe)))
            {
                throw new PublishException($"no {ServerExe} in the server folder {server}");
            }
            serverFinal = Path.Combine(Path.GetFullPath(options.ServersDir), version);
            if (Directory.Exists(serverFinal) || File.Exists(serverFinal))
            {
                throw new PublishException($"server of version {version} already in {Path.GetDirectoryName(serverFinal)}");
            }
        }
        Directory.CreateDirectory(root);
        var previous = VersionFolder.Published(root).Take(options.Keep).ToList();
        var hdiff = new HDiffPatch(options.Hdiffz ?? Path.Combine(AppContext.BaseDirectory, "hdiffz.exe"));
        var tmp = final + VersionFolder.TmpSuffix;
        var warnings = new List<string>();

        var serverTmp = serverFinal + VersionFolder.TmpSuffix;
        var moved = false;
        var serverMoved = false;
        int fileCount, patchCount = 0;
        long compressed = 0, patchBytes = 0;
        try
        {
            DeleteDir(tmp);
            Directory.CreateDirectory(tmp);
            if (server != null)
            {
                DeleteDir(serverTmp);
                CopyWithoutPdb(server, serverTmp);
            }
            var manifest = ManifestBuilder.Build(game, version);
            manifest.Save(Path.Combine(tmp, VersionFolder.Manifest));
            if (options.NotesFile != null)
            {
                File.Copy(options.NotesFile, Path.Combine(tmp, VersionFolder.Notes));
            }
            foreach (var file in manifest.Files)
            {
                compressed += Gzip(VersionFolder.Under(game, file.Path), VersionFolder.Under(Path.Combine(tmp, VersionFolder.Files), file.Path, ".gz"));
            }
            fileCount = manifest.Files.Count;

            var work = Path.Combine(tmp, WorkDir);
            Directory.CreateDirectory(work);
            foreach (var from in previous)
            {
                foreach (var patch in MakePatches(root, from, manifest, game, tmp, work, hdiff, warnings))
                {
                    patchCount++;
                    patchBytes += patch;
                }
            }
            DeleteDir(work);
            VersionFolder.WritePublishedDate(tmp, DateTime.UtcNow);

            // The server first: a version named by latest.txt always has its server.
            if (server != null)
            {
                Directory.Move(serverTmp, serverFinal!);
                serverMoved = true;
            }
            Directory.Move(tmp, final);
            moved = true;
            VersionFolder.WriteLatest(root, version);
        }
        catch (Exception e)
        {
            TryDeleteDir(tmp);
            if (server != null)
            {
                TryDeleteDir(serverTmp);
            }
            if (serverMoved)
            {
                TryDeleteDir(serverFinal!);
            }
            if (moved)
            {
                TryDeleteDir(final);
            }
            throw e as PublishException ?? new PublishException(e.Message, e);
        }

        var pruned = Prune(root, version, options.Keep, warnings);
        return new PublishResult(version, fileCount, compressed, patchCount, patchBytes, pruned, warnings);
    }

    /** The patches from one older version, as their sizes; each one written to <tmp>/patches/<from>/<path>.patch. */
    private static IEnumerable<long> MakePatches(string root, string from, VersionManifest manifest, string game, string tmp,
        string work, HDiffPatch hdiff, List<string> warnings)
    {
        var oldDir = Path.Combine(root, from);
        var old = VersionManifest.Load(Path.Combine(oldDir, VersionFolder.Manifest));
        var oldEntries = old.Files.ToDictionary(f => f.Path, StringComparer.OrdinalIgnoreCase);
        var oldFile = Path.Combine(work, "old.bin");
        var patchFile = Path.Combine(work, "patch.bin");
        var checkFile = Path.Combine(work, "check.bin");
        foreach (var file in ManifestDiff.Compare(old, manifest).Changed)
        {
            var oldEntry = oldEntries[file.Path];
            var oldGz = VersionFolder.Under(Path.Combine(oldDir, VersionFolder.Files), oldEntry.Path, ".gz");
            if (!File.Exists(oldGz))
            {
                warnings.Add($"no patch of {file.Path} from {from}: {oldGz} missing");
                continue;
            }
            Gunzip(oldGz, oldFile);
            if (ManifestBuilder.Hash(oldFile) != oldEntry.Sha256)
            {
                throw new PublishException($"{oldGz} does not match the list of version {from}");
            }
            File.Delete(patchFile);
            hdiff.Diff(oldFile, VersionFolder.Under(game, file.Path), patchFile);
            var patchSize = new FileInfo(patchFile).Length;
            var gzSize = new FileInfo(VersionFolder.Under(Path.Combine(tmp, VersionFolder.Files), file.Path, ".gz")).Length;
            if (patchSize < gzSize)
            {
                hdiff.Apply(oldFile, patchFile, checkFile);
                if (ManifestBuilder.Hash(checkFile) != file.Sha256)
                {
                    throw new PublishException($"patch of {file.Path} from {from} rebuilds a wrong file (SHA-256 differs)");
                }
                var target = VersionFolder.Under(Path.Combine(tmp, VersionFolder.Patches, from), file.Path, ".patch");
                Directory.CreateDirectory(Path.GetDirectoryName(target)!);
                File.Move(patchFile, target);
                yield return patchSize;
            }
            File.Delete(oldFile);
            File.Delete(checkFile);
        }
    }

    /** Keeps the `keep` newest versions besides the new one; never the one named in latest.txt. */
    private static List<string> Prune(string root, string version, int keep, List<string> warnings)
    {
        var latest = VersionFolder.ReadLatest(root);
        var pruned = new List<string>();
        foreach (var old in VersionFolder.Published(root).Where(v => v != version && v != latest).Skip(keep))
        {
            try
            {
                DeleteDir(Path.Combine(root, old));
                pruned.Add(old);
            }
            catch (Exception e) when (e is IOException or UnauthorizedAccessException)
            {
                warnings.Add($"version {old} not removed: {e.Message}");
            }
        }
        return pruned;
    }

    /** Version.txt at the folder's root, as ManifestBuilder.Build reads it. */
    private static string ReadVersionTxt(string game)
    {
        var file = Path.Combine(game, ManifestBuilder.VersionFile);
        if (!File.Exists(file))
        {
            throw new PublishException($"no --version and no {ManifestBuilder.VersionFile} in {game}");
        }
        return File.ReadAllText(file).Trim();
    }

    /** The server folder as it is, minus its symbols (*.pdb: about 3 GB). */
    private static void CopyWithoutPdb(string source, string target)
    {
        Directory.CreateDirectory(target);
        foreach (var file in Directory.EnumerateFiles(source, "*", SearchOption.AllDirectories))
        {
            if (file.EndsWith(".pdb", StringComparison.OrdinalIgnoreCase))
            {
                continue;
            }
            var to = Path.Combine(target, Path.GetRelativePath(source, file));
            Directory.CreateDirectory(Path.GetDirectoryName(to)!);
            File.Copy(file, to);
        }
    }

    private static long Gzip(string source, string target)
    {
        Directory.CreateDirectory(Path.GetDirectoryName(target)!);
        using (var input = File.OpenRead(source))
        using (var output = File.Create(target))
        using (var gzip = new GZipStream(output, CompressionLevel.Optimal))
        {
            input.CopyTo(gzip, 1 << 20);
        }
        return new FileInfo(target).Length;
    }

    private static void Gunzip(string source, string target)
    {
        using var input = new GZipStream(File.OpenRead(source), CompressionMode.Decompress);
        using var output = File.Create(target);
        input.CopyTo(output, 1 << 20);
    }

    private static void DeleteDir(string dir)
    {
        if (Directory.Exists(dir))
        {
            Directory.Delete(dir, recursive: true);
        }
    }

    private static void TryDeleteDir(string dir)
    {
        try
        {
            DeleteDir(dir);
        }
        catch (Exception e) when (e is IOException or UnauthorizedAccessException)
        {
            // Left behind: the next publication of this version deletes <v>.tmp first.
        }
    }
}
