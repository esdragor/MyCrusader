using System.Text.Json;

namespace HOLPublish;

/**
 * L14: the number of the next publication, "0.Y.X": Y = steps validated by the user, X = update of the current step
 * (from 0). Kept in release.json beside latest.txt of the versions folder, so that every copy of the project publishing
 * to it counts together. The file holds the NEXT version: a publication that succeeded moves X up ("done"), the user
 * validating a step moves Y up and X back to 0 ("next-stage"). The name stored is "0.Y.X" (version names have no space);
 * the "V " is added to show it (Display).
 */
public sealed record ReleaseNumber(int Stage, int Update)
{
    public string Name => $"0.{Stage}.{Update}";

    /** "V 0.Y.X" for a numbered version, the name as it is for the others (old dated versions, "dev"). */
    public static string Display(string name) => HOLUpdate.VersionName.Display(name);

    public static ReleaseNumber Read(string file)
    {
        if (!File.Exists(file))
        {
            throw new InvalidDataException($"no {file}; create it: HOLPublish release set --file {file} --stage <Y> --update 0");
        }
        try
        {
            using var document = JsonDocument.Parse(File.ReadAllText(file));
            var root = document.RootElement;
            var stage = root.GetProperty("stage").GetInt32();
            var update = root.GetProperty("update").GetInt32();
            if (stage < 0 || update < 0)
            {
                throw new InvalidDataException($"{file}: stage and update want numbers >= 0.");
            }
            return new ReleaseNumber(stage, update);
        }
        catch (Exception e) when (e is JsonException or KeyNotFoundException or InvalidOperationException or FormatException)
        {
            throw new InvalidDataException($"{file} is not {{\"stage\":<Y>,\"update\":<X>}}: {e.Message}");
        }
    }

    /** Written beside, then moved over the file: a reader sees the old number or the new one, never half a file. */
    public void Write(string file)
    {
        Directory.CreateDirectory(Path.GetDirectoryName(Path.GetFullPath(file))!);
        var temporary = file + ".tmp";
        File.WriteAllText(temporary, JsonSerializer.Serialize(new { stage = Stage, update = Update }));
        File.Move(temporary, file, overwrite: true);
    }

    /** The number after a publication of published: X + 1 if it was this very one, the same number otherwise. */
    public ReleaseNumber After(string published) => published == Name ? this with { Update = Update + 1 } : this;

    public ReleaseNumber NextStage() => new(Stage + 1, 0);

    /**
     * HOLPublish release name|done|next-stage|set --file <release.json> [--version <published>] [--stage <Y> --update <X>].
     * One line, "HOLPublish : RELEASE <name>" (name: the next version), or FAIL; exit code 0 / 1.
     */
    public static int Run(IReadOnlyList<string> args, TextWriter output)
    {
        try
        {
            var verb = args.Count > 0 ? args[0] : "";
            var options = new Dictionary<string, string>();
            for (var i = 1; i < args.Count; i += 2)
            {
                if (i + 1 >= args.Count)
                {
                    throw new ArgumentException($"{args[i]} without a value. Usage: {Usage}");
                }
                options[args[i]] = args[i + 1];
            }
            var file = options.GetValueOrDefault("--file") ?? throw new ArgumentException($"--file is required. Usage: {Usage}");
            switch (verb)
            {
                case "name":
                    output.WriteLine($"HOLPublish : RELEASE {Read(file).Name}");
                    break;
                case "done":
                    var current = Read(file);
                    var published = options.GetValueOrDefault("--version") ?? throw new ArgumentException($"--version is required. Usage: {Usage}");
                    var next = current.After(published);
                    if (next == current)
                    {
                        output.WriteLine($"HOLPublish : RELEASE {current.Name} (unchanged: {published} was not the number of the release)");
                        break;
                    }
                    next.Write(file);
                    output.WriteLine($"HOLPublish : RELEASE {next.Name}");
                    break;
                case "next-stage":
                    var stage = Read(file).NextStage();
                    stage.Write(file);
                    output.WriteLine($"HOLPublish : RELEASE {stage.Name}");
                    break;
                case "set":
                    var set = new ReleaseNumber(Number(options, "--stage"), Number(options, "--update"));
                    set.Write(file);
                    output.WriteLine($"HOLPublish : RELEASE {set.Name}");
                    break;
                default:
                    throw new ArgumentException($"Unknown release command '{verb}'. Usage: {Usage}");
            }
            return 0;
        }
        catch (Exception e) when (e is ArgumentException or InvalidDataException or IOException)
        {
            output.WriteLine($"HOLPublish : FAIL ({e.Message})");
            return 1;
        }
    }

    private const string Usage = "HOLPublish release name|done|next-stage|set --file <release.json> [--version <published>] [--stage <Y> --update <X>]";

    private static int Number(Dictionary<string, string> options, string name) =>
        options.TryGetValue(name, out var value) && int.TryParse(value, out var number) && number >= 0
            ? number
            : throw new ArgumentException($"{name} wants a number >= 0. Usage: {Usage}");
}
