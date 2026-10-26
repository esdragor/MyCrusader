namespace HOLLauncher;

/**
 * Whether "Play" is enabled and what it says (L8). Confirmation: asked before starting (null: none); StateWarning: added
 * to the state line ("" if none).
 */
public sealed record PlayDecision(bool Enabled, string? Confirmation, string StateWarning);

/**
 * When "Play" is allowed (L8, pure): an installed game can always be played, even outdated (the server will refuse it,
 * solo still works: asked first) or with the server unreachable (solo: said in the state line); never without a game
 * nor during an update.
 */
public static class PlayRules
{
    public const string OutdatedConfirmation =
        "Le jeu n'est pas à jour : le serveur refusera cette version. Jouer quand même (en solo) ?";

    public const string SoloWarning = "Vous pouvez jouer en solo.";

    public static PlayDecision Decide(GameState state, string? installed, bool updating = false)
    {
        if (updating || installed is null || state == GameState.NotInstalled)
        {
            return new PlayDecision(false, null, "");
        }
        return state switch
        {
            GameState.Outdated => new PlayDecision(true, OutdatedConfirmation, ""),
            GameState.Unreachable => new PlayDecision(true, null, SoloWarning),
            _ => new PlayDecision(true, null, ""),
        };
    }
}
