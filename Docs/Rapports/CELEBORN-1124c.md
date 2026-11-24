# CELEBORN-1124c : grille fine, etape G1 (D898)

NON COMPILE : aucun build ni quick_check lance (consigne : pas de quick_check dans un agent). A lancer par la session principale : `quick_check -Parts tests` avec le quick_check.ps1 du worktree `C:\Users\baudo\Documents\HeroesOfLiberty-Celeborn` (branche integ-41).

APPELS_DECLARES: 13 (arret a 25 vus)
HEURE_DEBUT/HEURE_FIN: 11:05:13 / 11:07:48 (horloge Get-Date)
PREMIER_BUILD: sans C++   BUILD: NON COMPILE   LAUNCHCHECK_DEBUT: sans C++
TEST_ROUGE: 6a8b750 (bouchon `PlanFromRequestFine` + `Test_WallLineFineGridLineIsSeventeenCells`) ; assertit l'effet : oui (le nombre de cases et le cout viennent de la regle, pas d'une constante). Rouge NON VU a l'execution : attendu `Plan.Cells.Num()` 5 au lieu de 17 (bouchon = plan de 4 m). TEST_ROUGE_SEPARE: oui (rouge seul dans 6a8b750)
COMMITS: 2 (6a8b750 rouge, e9cf204 correctif), dernier a ~11:07, merge_main_fait: non (integ-41 fige pour l'integration)
TESTS: aucun lance ; 1 test script ajoute `Test_WallLineFineGridLineIsSeventeenCells` (HOLWallLineRules_Test.as). TESTS_TAUTOLOGIQUES: 0
TESTS_SCRIPT_AJOUTES: 1 (Test_WallLineFineGridLineIsSeventeenCells)   BUILDS_DEMANDES: 0   TYPES_A_EXPOSER: aucun
ESSAIS: aucun
QUESTIONS: aucune neuve ; Retours\Q-grille-fine-structure.md (SAURON-1124a) non tranchee : defaut = comportement actuel derriere `hol.Wall.SubDivision` = 1.

## Recalcul des nombres
Start (50,50), End (1650,50), Tile 400, SubDivision 4 : pas = 100. Case de x = 50 : floor(50/100) = 0 ; case de x = 1650 : floor(16.5) = 16. Meme rangee (Y 0). Cases 0..16 = 17 ; 17 x 5 Bois = 85. A SubDivision 1 : pas 400, cases 0 et 4 = 5 cases (le bouchon rend 5, comme annonce).
Le test verifie aussi : premiere (0,0), derniere (16,0), cout `Cost(Cells[0], Cells[16], 1000, 5)` = 85, et SubDivision 1 = 5 cases (garde du defaut).

## Fait
- `Script\Buildings\HOLWallLineRules.as` : `PlanFromRequestFine(Slot, Start, End, Tile, SubDivision, MapMin, MapMax, Occupied, MaxCells)` : SubDivision <= 1 = `PlanFromRequest` inchange ; sinon pas = Tile / SubDivision, bornes MapMin * Sub et MapMax * Sub + Sub - 1 (MapMin / MapMax restent en cases de 4 m pour l'appelant). Cvar `hol.Wall.SubDivision` (defaut 1) + accesseur `HOLWallLine::SubDivision()` (aucun appelant branche : HOLRpcPlayer.as hors fichiers pris, G5).
- Les 28 tests existants ne sont pas touches (SubDivision 1 par defaut).

## Reste
- G2 (occupation en sous-cases, `ExpandOccupied`) : `Occupied` est lu tel quel en sous-cases ici.
- Brancher `SubDivision()` dans `ServerPlaceWallLine` (G5) ; bascule du defaut a 4 en G10.

NON VERIFIE: compilation Angelscript et execution du test (rouge et vert) ; `const FConsoleVariable` en tete de fichier suit le modele de `CVar_WallMaxLineCells`. DIAGNOSTICS_AS: non relus (serveur de langage non consulte).
PROCESSUS_DETACHES: aucun processus detache pour C:\Users\baudo\Documents\HeroesOfLiberty-Celeborn
ANOMALIES_PROCESSUS: aucune. Worktree : fichiers non suivis preexistants (CELEBORN-1123d_tests.ps1, Docs/Integrate_*), non touches. IN_USE.txt et Auto_Journey_File.md non mis a jour (hors temps).
GIT_STATUS_PRINCIPALE: docs seulement (FortificationsInProgress.md, EtapeDefini.md par moi) ; reste inchange par moi : oui
