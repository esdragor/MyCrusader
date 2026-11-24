using HOLUpdate;

namespace HOLLauncher;

/** The notes the "Nouveautés" box shows (L21); Headed: each one under its version's name. */
public sealed record ReleaseNotesSelection(IReadOnlyList<ReleaseNote> Notes, bool Headed, bool OlderMissing);

/**
 * L21, pure: which versions' notes the window shows. A player several versions behind sees every version newer than
 * the installed one, newest first, each under its name (V 0.Y.X), not only the latest's. When the installed version is
 * not in the manager's history (pruned by publish.ps1 -Keep, or a name never published there), every kept version is
 * shown and a line says older notes are gone. Up to date or not installed: the latest's notes alone, as before L21 (a
 * new player has no "since" to measure from, and a list of past changes would bury what the game is today). A manager
 * older than L21 sends no history: the same. Versions without notes are left out; no notes at all hides the box.
 */
public static class ReleaseNotes
{
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

    /** The box's text; "" when no note has text. Line ends are \r\n: the box is a WinForms TextBox. */
    public static string Text(string? installed, LatestVersion latest)
    {
        var selection = Select(installed, latest);
        var notes = selection.Notes.Where(n => !string.IsNullOrWhiteSpace(n.Notes)).ToList();
        if (notes.Count == 0)
        {
            return "";
        }
        var blocks = notes.Select(n => selection.Headed
            ? VersionName.Display(n.Version) + "\r\n" + WindowsLines(n.Notes)
            : WindowsLines(n.Notes)).ToList();
        if (selection.OlderMissing)
        {
            blocks.Add(Texts.Get("NotesOlderMissing"));
        }
        return string.Join("\r\n\r\n", blocks);
    }

    private static string WindowsLines(string text) => text.Trim().ReplaceLineEndings("\r\n");
}
