namespace HOLPublish;

/** HOLPublish --game <Windows\> --versions <folder> [--version <name>] [--notes <file>] [--keep <N>] [--hdiffz <path>]. */
public static class CommandLine
{
    public const string Usage =
        "HOLPublish --game <player folder Windows\\> --versions <versions folder> [--version <name>] [--notes <file>] "
        + "[--keep <N, default 3>] [--hdiffz <path>]";

    public static PublishOptions Parse(IReadOnlyList<string> args)
    {
        string? game = null, versions = null, version = null, notes = null, hdiffz = null;
        var keep = 3;
        for (var i = 0; i < args.Count; i += 2)
        {
            var name = args[i];
            if (i + 1 >= args.Count)
            {
                throw new ArgumentException($"{name} without a value. Usage: {Usage}");
            }
            var value = args[i + 1];
            switch (name)
            {
                case "--game": game = value; break;
                case "--versions": versions = value; break;
                case "--version": version = value; break;
                case "--notes": notes = value; break;
                case "--hdiffz": hdiffz = value; break;
                case "--keep":
                    if (!int.TryParse(value, out keep) || keep < 0)
                    {
                        throw new ArgumentException($"--keep wants a number >= 0, not '{value}'.");
                    }
                    break;
                default: throw new ArgumentException($"Unknown option '{name}'. Usage: {Usage}");
            }
        }
        if (game == null || versions == null)
        {
            throw new ArgumentException($"--game and --versions are required. Usage: {Usage}");
        }
        return new PublishOptions(game, versions, version, notes, keep, hdiffz);
    }
}
