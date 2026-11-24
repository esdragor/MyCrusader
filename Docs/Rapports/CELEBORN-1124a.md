# CELEBORN-1124a : D901, la ligne de palissade saute les cases bloquees (sans C++, NON COMPILE)

Impact joueur : une case occupee sur le trace n'annule plus toute la ligne ; les autres cases sont posees et payees, la case occupee reste un trou, rouge dans le fantome.

```
APPELS_DECLARES: 17 (arret a 25 vus ; premier Edit a l'appel 8)
HEURE_DEBUT/HEURE_FIN: 10:50:02 / 10:54:44
PREMIER_BUILD: sans C++   BUILD: NON COMPILE   LAUNCHCHECK_DEBUT: sans C++
TEST_ROUGE: c285b65 (Test_WallLineValidateOccupiedCellIsSkippedNotRefused + 2 autres, bouchon = champ Skipped, Validate inchange) ; rouge NON VU a l'execution (non lance dans l'agent) ; assertit l'effet : oui (Cells.Num()==4, Skipped, Refusal Ok)   TEST_ROUGE_SEPARE: oui (c285b65 test+bouchon, 75025ee regle)
COMMITS: 5 (c285b65, 75025ee, 2fcb2d6, commentaire du drawer, 43ea46e docs), dernier 10:54, merge_main_fait: non (integ-41 = main 173d31b, rien a fusionner)
TESTS: aucun lance (quick_check a la charge de la session principale) ; TESTS_TAUTOLOGIQUES: 0
TESTS_SCRIPT_AJOUTES: 3 (Test_WallLineValidateOccupiedCellIsSkippedNotRefused, Test_WallLineValidateAllCellsOccupiedLaysNothing, Test_WallLineSkippedCellIsNotPaidNorQueued)
BUILDS_DEMANDES: 0   TYPES_A_EXPOSER: aucun
ESSAIS: aucun (wall_line_long.txt : commentaires seulement, geste inchange)
QUESTIONS: aucune
```

## FAIT
- `Script\Buildings\HOLWallLineRules.as` : `FHOLWallLinePlan.Skipped` (champ, c285b65) ; `Validate` : la boucle `Occupied` envoie les cases occupees vers `Skipped` et les libres vers `Cells` ; si `Cells` est vide et `Skipped` non vide : `Refusal = Occupied`, `BadCell = Skipped[0]` (75025ee). Commentaires « All or nothing » mis a jour (TooLong et OutOfMap restent tout-ou-rien).
- Cout et ordres : `HOLRpcPlayer.as ServerPlaceWallLine` (:1542-1543) boucle sur `Plan.Cells` : l'index de `BuildersForCell` / `QueueForCell` et le cout suivent donc les cases POSEES, sans modifier le fichier (aucun conflit avec E2). La premiere case posee obeit a Shift, meme si la case de depart est un trou.
- Fantome (« Suite » de l'enonce) : `HOLWallGhostRules.as` `Ghost.Skipped = Plan.Skipped`, `bValid` = ligne acceptee (verte) ; `HOLWallGhostDrawer.as` dessine les cases sautees dans la boite rouge (2fcb2d6). Pas de C++ (le drawer lit deja les cellules par script).
- Tests tout-ou-rien changes dans le meme lot (L22-3) : `Test_WallLineValidateOccupiedCellRefusesTheLine` renomme `...LeavesAHole` ; `Test_WallLinePlanCarriesTheRefusals` (cas Occupied : 4 cases + 1 sautee) ; `Test_WallGhostOfAnOccupiedLineIsRedAndNamesTheCell` renomme `...IsValidAndSkipsTheCell`. Grep de `quick_check.ps1` (Tools\Tests\, pas Tools\) : aucune attente sur WallLine / Occupied ; `Tools\Scenarios\` : seul `wall_line_long.txt` mentionnait le refus (commentaire :5 et note 4, corriges), aucune assertion.
- Journal FortificationsInProgress.md (43ea46e), EtapeDefini.md, Auto_Journey_File.md (ligne 28 cochee).

## RESTE
- Session principale : `quick_check.ps1 -Parts tests` du worktree Celeborn (rouge a viser : c285b65 doit echouer au premier cas ; vert a 75025ee / 2fcb2d6).
- Rejouer `wall_line_long` avec une case bloquee (geste reel : un glisser qui croise un batiment) pour voir le trou rouge ; non fait.
- HOLWallLineBridge.as : aucun changement necessaire (il ne renvoie que `DragCells`, la ligne complete) ; le trou rouge vient du ghost.
- Pas de log serveur du nombre de cases sautees (aurait touche HOLRpcPlayer.as, region voisine de E2).

## NON VERIFIE
Rien compile, aucun test lance ; le commit du test n'a pas ete vu rouge a l'execution ; diagnostics AS (serveur de langage) non relus (DIAGNOSTICS_AS: non verifie). Le drawer (`UInstancedStaticMeshComponent Box = bHole ? BadBoxes : Shown;`) est du code script non compile.

PROCESSUS_DETACHES: aucun processus detache pour C:\Users\baudo\Documents\HeroesOfLiberty-Celeborn
ANOMALIES_PROCESSUS: aucune
GIT_STATUS_PRINCIPALE: modifie par moi : EtapeDefini.md, Auto_Journey_File.md (lignes voulues) ; autres fichiers (Placeholders.txt, Retours/E-tower_wood.md, SessionEnd.txt, rapports d'autres agents) = autres sessions ; aucun code ; inchange hors docs voulus : oui
```
