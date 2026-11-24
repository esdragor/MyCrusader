using HOLUpdate;

namespace HOLLauncher;

/** The notes the "Nouveautés" box shows (L21); Headed: each one under its version's name. */
public sealed record ReleaseNotesSelection(IReadOnlyList<ReleaseNote> Notes, bool Headed, bool OlderMissing);

/**
 * One paragraph of the "What's new" box (L22): a version's name, a line of its notes, a bullet line ("- ", "* " or a
 * bullet sign, without its marker), a blank line, or the line saying older notes are gone. The window styles each kind
 * (NewsFormat); Gap has no text.
 */
public enum NewsKind
{
    Heading,
    Text,
    Bullet,
    Gap,
    OlderMissing,
}

public sealed record NewsBlock(NewsKind Kind, string Text = "");

/**
 * L21, pure: which versions' notes the window shows. A player several versions behind sees every version newer than
 * the installed one, newest first, each under its name (V 0.Y.X), not only the latest's. When the installed version is
 * not in the manager's history (pruned by publish.ps1 -Keep, or a name never published there), every kept version is
 * shown and a line says older notes are gone. Up to date or not installed: the latest's notes alone, as before L21 (a
 * new player has no "since" to measure from, and a list of past changes would bury what the game is today). A manager
 * older than L21 sends no history: the same. Versions without notes are left out; no notes at all hides the box.
 * L22: the notes are given as blocks (Blocks) so that the window can style them; Text is their plain form.
 */
public static class ReleaseNotes
{
    /** The markers a notes line may start with to be a bullet (the marker is dropped, the box draws its own bullet). */
    private static readonly string[] BulletMarkers = ["- ", "* ", "\u2022 "];

    public static ReleaseNotesSelection Select(string? installed, LatestVersion latest)
    {
        var alone = new ReleaseNotesSelection([new ReleaseNote(latest.Version, latest.Notes)], Headed: false, OlderMissing: false);
        var history = latest.History;
        if (installed is null || installed == latest.Version || history is null || history.Count == 0)
        {
            return alone;
        }
        var at = history.Select(n => n.Version).ToList().IndexOf(installed);
        var newer = at < 0 ? history : history.Take(at).ToList();
        return newer.Count == 0 ? alone : new ReleaseNotesSelection(newer, Headed: true, OlderMissing: at < 0);
    }

    /**
     * The box's paragraphs; none when no note has text. A blank line separates two versions, and stands for any run of
     * blank lines inside a note (those around it are dropped). The "older notes" line in the given language (null: the
     * one in use).
     */
    public static IReadOnlyList<NewsBlock> Blocks(string? installed, LatestVersion latest, string? language = null)
    {
        var selection = Select(installed, latest);
        var notes = selection.Notes.Where(n => !string.IsNullOrWhiteSpace(n.Notes)).ToList();
        var blocks = new List<NewsBlock>();
        if (notes.Count == 0)
        {
            return blocks;
        }
        foreach (var note in notes)
        {
            if (blocks.Count > 0)
            {
                blocks.Add(new NewsBlock(NewsKind.Gap));
            }
            if (selection.Headed)
            {
                blocks.Add(new NewsBlock(NewsKind.Heading, VersionName.Display(note.Version)));
            }
            blocks.AddRange(LinesOf(note.Notes));
        }
        if (selection.OlderMissing)
        {
            blocks.Add(new NewsBlock(NewsKind.Gap));
            blocks.Add(new NewsBlock(NewsKind.OlderMissing, Texts.Get("NotesOlderMissing", Texts.LanguageOr(language))));
        }
        return blocks;
    }

    /** The blocks as plain text, bullets written "- ", line ends \r\n ("" for none). */
    public static string PlainText(IEnumerable<NewsBlock> blocks)
        => string.Join("\r\n", blocks.Select(b => b.Kind == NewsKind.Bullet ? "- " + b.Text : b.Text));

    /** The plain text of Blocks (L21's text). */
    public static string Text(string? installed, LatestVersion latest, string? language = null)
        => PlainText(Blocks(installed, latest, language));

    private static IEnumerable<NewsBlock> LinesOf(string notes)
    {
        var gap = false;
        foreach (var raw in notes.Trim().ReplaceLineEndings("\n").Split('\n'))
        {
            var line = raw.TrimEnd();
            if (line == "")
            {
                gap = true;
                continue;
            }
            if (gap)
            {
                yield return new NewsBlock(NewsKind.Gap);
                gap = false;
            }
            var start = line.TrimStart();
            var marker = BulletMarkers.FirstOrDefault(m => start.StartsWith(m, StringComparison.Ordinal));
            yield return marker is null
                ? new NewsBlock(NewsKind.Text, line)
                : new NewsBlock(NewsKind.Bullet, start[marker.Length..].TrimStart());
        }
    }
}
