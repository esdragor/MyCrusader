# GALADRIEL-1119cc : lot L1a (tests script de la grille, groupe A) : NON COMPILÉ

Worktree `C:\Users\baudo\Documents\HeroesOfLiberty-Galadriel`, branche `grid-c6`. Départ HEAD 28dfc85. Début 17:02.

COMMIT : `3f54c3a` "Command grid script tests: Kings alone, forum down, rage button (group A extracts, script rule only)". Un seul fichier : `Script\Tests\HOLCommandGridRules_Test.as` (+62 lignes, fichier en CRLF, 3 tests ajoutés en fin, après `Test_CGRHardValues`).
BUILDS_DEMANDES: 0 ; TESTS_SCRIPT_AJOUTES: 3 (Test_CGKingsOnly, Test_CGForumDown, Test_CGRageButton) ; TESTS_CPP_RETIRES: 0 (c'est L2) ; TESTS_TAUTOLOGIQUES: 0 (valeurs attendues en clair, tout passe par `HOLCommandGridRules::GetOrder/GetCommand/IsBuildGrid`, aucun `*Answer`, aucune lecture du jumeau C++).

## Les 3 tests (fichier:ligne = lignes du `.as` après commit ; l'extrait C++ couvert)

1. `Test_CGKingsOnly` (couvre `HOLRegicideTests.cpp:288-299`).
   - Rois seuls (`bOwnUnits`, `bSoldiers`, `bOnlyKings`) : `GetOrder` cellule AttackMove (4) = None ; cellule Stop (5) = Stop ; cellule Hold (11) = None ; `IsBuildGrid` faux.
   - Roi + soldats (copie avec `bOnlyKings = false`) : AttackMove = AttackMove ; Hold = Hold.
   - Les cellules viennent de `HOLCommandGridRules::GetAttackMoveCell/GetStopCell/GetHoldCell` (les constantes de fichier ne sont pas visibles d'un autre fichier). Les 6 assertions de l'extrait C++ sont reprises (292, 293, 294, 295, 298, 299).
2. `Test_CGForumDown` (couvre `HOLVictoryTests.cpp:1197-1204`).
   - Forum debout (`bOwnBuilding`, `bBuildingFinished`, `Produces = [Peasant]`) : cellule 0 = Produce (précondition de la ligne 1201).
   - Puis `bForumDown = true` : cellule 0 = None ; `GetCancelCell()` (11) = None. (lignes 1203 et 1204)
3. `Test_CGRageButton` (couvre `HOLDamnedWarriorTests.cpp:230-235`).
   - `bOwnUnits` + `bRageUnit` : `GetCommand(Selection, GetRageCell())` = NecroticRage (ligne 233) ; `bRageUnit = false` : != NecroticRage (ligne 235).

Conversion : `IsNone()` du C++ = `Command == EHOLGridCommandKind::None` en script ; `EHOLGridCommand::X` = `EHOLGridCommandKind::X` (noms vus dans `Script\Player\HOLCommandGridRules.as`).

## FALSIFICATION (rouge à voir, un test à la fois, dans le worktree, à remettre ensuite)

Les lignes sont dans `Script\Tests\HOLCommandGridRules_Test.as` (cherche la chaîne du message d'assertion, les numéros exacts sont dans les 62 dernières lignes, 433-494).
- `Test_CGKingsOnly` : dans l'assertion « Kings alone: no hold (it would take on enemies in reach) », remplacer `EHOLGridCommandKind::None` par `EHOLGridCommandKind::Hold` (attendre Hold au lieu de vide pour les Rois). Attendu : ce test seul rouge, message « Kings alone: no hold ... ».
- `Test_CGForumDown` : dans l'assertion « a forum down: no production », remplacer `EHOLGridCommandKind::None` par `EHOLGridCommandKind::Produce` (attendre Produce en cellule 0 forum tombé). Attendu : rouge, message « a forum down: no production ».
- `Test_CGRageButton` : dans l'assertion « the grid shows the rage button ... », retirer `Selection.bRageUnit = true;` (rage attendue sans `bRageUnit`). Attendu : rouge, message « the grid shows the rage button for a selection with a Damned Warrior ».
Variante côté règle (plus forte, fait rougir aussi `Test_CGRHardValues`) : dans `Script\Player\HOLCommandGridRules.as` `GetOrder`, retirer `&& !Context.bOnlyKings` de la branche Hold.

## Commande à lancer par la session principale

Les 3 tests se découvrent par leur nom `Test_*` dans `Script\Tests` comme les autres du fichier (suites `HeroesOfLiberty.CommandGridRules.*`) ; pas de build C++ nécessaire, mais le script doit se recompiler au lancement. Lancer la partie `quick_check` qui exécute les tests script (nom exact non lu ; voir `Tools\Tests\quick_check.ps1`). Total attendu : +3 (1779 -> 1782).

## NON VÉRIFIÉ (rien n'est compilé ni exécuté)

- Compilation du script : syntaxe copiée sur `Test_CGRHardValues` (`FHOLGridContextView`, `FHOLGridCommandView`, `T.AssertTrue/AssertFalse`, `n"Peasant"`), mais aucune erreur de compilation Angelscript n'a pu être vue.
- Le nom de découverte des tests : j'ai suivi la convention du fichier (`// HeroesOfLiberty.CommandGridRules.X` au-dessus de `void Test_CGX(FUnitTest& T)`), non confirmée par un lancement ; le commentaire n'est qu'une étiquette si la découverte se fait par préfixe `Test_`.
- Que `HOLCommandGridRules::GetAttackMoveCell/GetStopCell/GetHoldCell/GetCancelCell/GetRageCell` soient bien appelables hors du fichier (lus en namespace, lignes 32-46 de `HOLCommandGridRules.as`, mais non exécutés).
- `FHOLGridContextView Mixed = Kings;` (copie par valeur d'un USTRUCT) : le piège « Null pointer access » concerne les méthodes en argument ; une copie de struct est attendue valide, non vue.
- Les 3 rouges de falsification ne sont pas vus.
- Les numéros de ligne C++ sont ceux de HEAD 28dfc85 (lus par Grep de `HOLCommandGrid::`).

## Anomalies

- Aucune dans le code. Le PowerShell de `Get-Date` final a été refusé par le classifieur (pas de verdict) ; heure de fin non relevée, environ 17:10.
- `EtapeDefini.md`, `Auto_Journey_File.md`, `IN_USE.txt` et les `*InProgress.md` non touchés (règle de séance). Pas de merge, pas de push, pas de build, pas d'éditeur lancé.
- `Test_CGForumDown` : en C++ `Grid.Produces = { Peasant }` ; en script `Produces.Add(n"Peasant")` (équivalent).
