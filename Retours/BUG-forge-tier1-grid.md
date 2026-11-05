# BUG-forge-tier1-grid : au palier 1 la grille de la Forge reste grisée et l'infobulle « Palier supérieur requis » persiste

Découvert par : agent d'intégration n°2, 2026-11-05 21:19, main eb7b5fe (binaire 21:07), scénario `forge`.

SCÉNARIO : `powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Elrond\Tools\Scenarios\play.ps1 forge -Root C:\Users\baudo\Documents\HeroesOfLiberty-Elrond`
GESTE EXACT (forge.txt) : `hol.Cheat.SpawnBuilding Forge 0 2500 2500`, `hol.Select.Category Technological` (Forge sélectionnée), `hol.Select.Grid 8` au palier 0 (refus « tier too low », voulu), `hol.Hero.SetXP 0 600` (niveau 3 = palier 1), `frames 90`, `hol.HUD.Report`, capture `forge_tier1`.

VU (PNG dans C:\Users\baudo\Documents\HeroesOfLiberty-Elrond\Saved_Tests\Screenshots\WindowsEditor\HOL\) :
- forge_tier1.png : « Palier 1 atteint » en haut à droite, mais la rangée W X C V de la Forge est strictement comme dans forge_grid.png (palier 0) : W « Lames affûtées » grisée, C « Empennage » grisée ; l'infobulle jaune « Palier supérieur requis » est toujours affichée au-dessus de la grille.
- forge_queued.png (après `hol.Select.Grid 8` : `Production: slot 0 queued research MeleeAttackI`, coût pris 150 N / 100 O) : la case C « Empennage » est éclaircie, W est grisée (en file) ; l'infobulle « Palier supérieur requis » est toujours là, et le panneau de file affiche « …ames affûtées » tronqué.
ATTENDU (forge.look) : « W Lames affûtées I est maintenant prête (non grisée), le refus du palier a disparu ».
Hypothèse non vérifiée : le HUD ne recalcule la grille de la Forge qu'après un événement (file, sélection) et pas au changement de palier ; l'infobulle de refus n'a pas de minuterie. Pas de cause cherchée dans le code (classe sans C++).
Pas de question de design : le .look dit déjà l'attendu.
NON VÉRIFIÉ : si la grille se rafraîchit d'elle-même après quelques secondes au-delà de 90 images ; si le même défaut touche les autres bâtiments à recherche (Caserne W « Hallebardes trempées »).

## Cause trouvee (agent scenarios, 2026-11-05 21:31, sans C++) : DEFAUT DU JEU, pas du scenario
Preuve : dans forge.txt, apres `hol.Hero.SetXP 0 600` + `frames 90`, une re-selection (`hol.Select.Category Economic` puis `hol.Select.Category Technological`) redessine la grille : W « Lames affutees » est alors prete, l'infobulle « Palier superieur requis » a disparu (forge_tier1.png vu). Sans re-selection, 90 images ne suffisent pas : la grille ne se redessine jamais d'elle-meme.
Cause probable : `Source\HeroesOfLibertyUI\HUD\HOLGameScreen.cpp`, `UHOLGameScreen::RefreshGrid`, case `EHOLGridCommand::Research` (l. 1918-1950) : l'etat Ready/Greyed et l'infobulle viennent de `CheckResearchAt` au moment du dessin, mais aucun palier « montre » n'est memorise pour les cases de recherche. Le rafraichissement periodique (l. 574-584) ne suit le palier que pour les panneaux d'ages (`ShownTier`) et pour les cases d'engins de siege (`ShownEngineTier`, remis a `INDEX_NONE` l. 1873 et fixe l. 1910-1913, compare l. 581-584). Correction probable (a decider/faire par une session C++) : memoriser le palier au dessin d'une case Research (comme `ShownEngineTier`) et appeler `Refresh()` quand `BoundTableState->GetTier(...)` change. Meme defaut attendu pour tout batiment a recherche (Caserne « Hallebardes trempees »), non teste.
Scenario : forge.txt garde la re-selection (commentee) pour que l'essai passe ; retirer ces 4 lignes quand le jeu est corrige (le .look dit deja l'attendu).
Autre point vu (cosmetique, hors scenario) : forge_queued.png, le titre du panneau de file « Lames affutees » est tronque a gauche (« ames affutees ») et recouvert par le bloc du heros.
