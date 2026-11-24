# CELEBORN-1124d : grille fine, etape G2 (D898), occupation en sous-cases

NON COMPILE : aucun quick_check lance (consigne). A lancer par la session principale : `quick_check -Parts tests` avec le quick_check.ps1 du worktree C:\Users\baudo\Documents\HeroesOfLiberty-Celeborn (branche integ-41, tip 6f71391).

APPELS_DECLARES: 15 (arret a 25 vus)
HEURE_DEBUT/HEURE_FIN: 11:09:58 / 11:14:51
PREMIER_BUILD: sans C++   BUILD: NON COMPILE   LAUNCHCHECK_DEBUT: sans C++
TEST_ROUGE: 79d6817 (bouchon `ExpandOccupied` vide + `Test_WallLineFineGridOccupiedTileSkipsItsSubCells`) ; assertit l'effet : oui (Skipped, Cells, Refusal viennent de la regle). Rouge NON VU a l'execution : attendu avec le bouchon Skipped 0 au lieu de 4, Cells 17 au lieu de 13, Refusal Ok au lieu de Occupied. TEST_ROUGE_SEPARE: oui
COMMITS: 2 (79d6817 rouge 11:11, 6f71391 correctif 11:11), merge_main_fait: oui (Already up to date)
TESTS: aucun lance ; 1 test script ajoute. TESTS_TAUTOLOGIQUES: 0
TESTS_SCRIPT_AJOUTES: 1 (Test_WallLineFineGridOccupiedTileSkipsItsSubCells)   BUILDS_DEMANDES: 0   TYPES_A_EXPOSER: aucun
ESSAIS: aucun ; chargement du jeu : play.ps1 wall_line_skip -Hidden -Root C:\Users\baudo\Documents\HeroesOfLiberty-Celeborn -TimeoutSec 150 -> jeu quitte seul apres 30 s, log Saved\Logs\HeroesOfLiberty.log : 0 ligne "Angelscript ... rror" (L22-1). 1 avertissement preexistant (HOLWallLineRules.as:58 division entiere, hors de mes lignes).
QUESTIONS: aucune ; defaut hol.Wall.SubDivision = 1 inchange.

## Fait
- `Script\Buildings\HOLWallLineRules.as` : `ExpandOccupied(OccupiedTiles, SubDivision)` (chaque case de 4 m = SubDivision x SubDivision sous-cases ; SubDivision <= 1 = copie) ; `PlanFromRequestFine` appelle ExpandOccupied : son `Occupied` est desormais en CASES DE 4 M (G1 le lisait en sous-cases, aucun appelant branche). SubDivision 1 = `PlanFromRequest` inchange (28 tests existants non touches).
- Test : tuile (2,0) occupee, ligne de G1 -> 4 sautees (8..11, 0), 13 posees, 13 x 5 = 65 ; toutes les tuiles 0..4 occupees -> Refusal Occupied, 0 case ; SubDivision 1 -> 1 sautee.
- Auto_Journey_File.md : lignes [x] G1 (CELEBORN-1124c) et G2 ajoutees apres la ligne du plan D898 (les lignes G1..G10 de la section 5 du plan n'etaient pas dans la File). FortificationsInProgress.md, EtapeDefini.md, IN_USE.txt a jour.

## Reste
- G3 (PieceBox), G4 (lot C++), brancher SubDivision() dans ServerPlaceWallLine (G5). Les pieces de mur deja posees sont vues comme des cases de 4 m jusqu'a G4.

NON VERIFIE: execution des tests (rouge et vert) ; DIAGNOSTICS_AS via serveur de langage non consultes (seul le chargement du jeu est vu : 0 erreur).
PROCESSUS_DETACHES: aucun processus detache pour C:\Users\baudo\Documents\HeroesOfLiberty-Celeborn
ANOMALIES_PROCESSUS: aucune
GIT_STATUS_PRINCIPALE: M Auto_Journey_File.md |  M Docs/Rapports/GATE-1124-launch-Celeborn.txt |  M Docs/Rapports/GATE-1124-launch-Elrond.txt |  M Docs/Rapports/GATE-1124-launch-Faramir.txt |  M Docs/Rapports/GATE-1124-launch-arwen.txt |  M Docs/SessionEnd.txt |  M EtapeDefini.md |  M FortificationsInProgress.md |  M Placeholders.txt | ?? Docs/Rapports/ARWEN-1124a.md | ?? Docs/Rapports/ARWEN-1124b.md | ?? Docs/Rapports/ARWEN-1124c.md | ?? Docs/Rapports/CELEBORN-1124a.md | ?? Docs/Rapports/CELEBORN-1124b.md | ?? Docs/Rapports/CELEBORN-1124c.md | ?? Docs/Rapports/ELROND-1124a.md | ?? Docs/Rapports/ELROND-1124b.md | ?? Docs/Rapports/FARAMIR-1124a.md | ?? Docs/Rapports/FARAMIR-1124b.md | ?? Docs/Rapports/FARAMIR-1124c.md | ?? Docs/Rapports/GALADRIEL-1124a.md | ?? Docs/Rapports/GALADRIEL-1124b.md | ?? Docs/Rapports/GATE-1124-launch-Glorfindel.txt | ?? Docs/Rapports/GATE-1124-launch-galadriel.txt | ?? Docs/Rapports/GLORFINDEL-1124a.md | ?? Docs/Rapports/INTEG-64.txt | ?? Docs/Rapports/PLAN-GRILLE-FINE-1124.md | ?? Docs/Rapports/SAURON-1124a.md | ?? Retours/E-faction_aim_free.md | ?? Retours/E-wall_line_skip.md | ?? Retours/Q-grille-fine-structure.md ; hors docs voulus inchange par moi : oui