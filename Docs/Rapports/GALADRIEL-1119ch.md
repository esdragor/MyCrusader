# GALADRIEL-1119ch : lot L3c (fin du retrait du natif de la grille de commandes)

NON COMPILE (aucun build, aucun quick_check, aucun editeur). Worktree `C:\Users\baudo\Documents\HeroesOfLiberty-Galadriel`, branche NEUVE `grid-l3c` partie de `integ-20` (a070e50), commit `1b81038` (anglais, sans Co-Authored-By, pas de push, pas de fusion). Branche prete.

## 1. Appelants de GetCommand / GetOrder / GetCommandNative / GetOrderNative (Grep Source\ et Script\, .cpp .h .as)
Plus AUCUN appelant sans monde : `HOLRegicideTests.cpp` et `HOLVictoryTests.cpp` n'appellent plus rien (extraits retires par L2). `HOLDamnedWarriorTests.cpp` et `HOLStartingHeroTests.cpp` n'ont plus d'appel non plus (hors liste du Grep).
Appelants restants de `HOLCommandGrid::GetCommand` / `GetOrder` (C++) :
- `Player\HOLPlayerController.cpp:2264, 2277` (GetCommand), `:2377` (GetOrder) : monde de jeu, plateau script present en partie.
- `HeroesOfLibertyUI\Tests\HOLSpecializationGridTests.cpp:54-115` (12 appels GetCommand, test lignes 41-118) : AVEC monde (`UWorld::CreateWorld(Game)` + `CreateNewWorldContext(Game)`, lignes 41-42), mais il lui faut le plateau SCRIPT de la grille dans ce monde (`UHOLCommandGridBoard`, sous-systeme du monde). Sans lui, `GetCommand` rend une commande vide (avant L3c : repli natif). NON VERIFIE (voir plus bas).
- Script : `HOLCommandGridBoard.as:15, 21` appelle `HOLCommandGridRules::GetCommand/GetOrder` (la regle script, pas le natif) ; `Script\Tests\*` appellent la regle script (non touches).
Aucun autre appelant des deux `*Native`. Donc retrait fait : `GetCommand` et `GetOrder` sont maintenant le pont seul (`Ask* ? Answer : LogNoBoardOnce("<Fonction>") + FHOLGridCommand()` vide), ligne de journal `CommandGrid: GetCommand refused, no script grid board` / `... GetOrder refused ...`. Corps natifs supprimes (GetCommandNative ~80 l., GetOrderNative ~15 l.).

## 2. Formule de rang `slot + 1`
Rien a retirer : dans `HOLCommandGrid.cpp` il n'y a plus de formule (`GetCycleRankNative` a ete retire en L3a ; `GetCycleRank` est deja le pont : `AskGetCycleRank(Type.IsBuildable(), Type.BuildGridSlot, Answer)`, `HOLCommandGrid.cpp:~340`). `GetCycleRankOfType` n'existe plus dans `Source\` (L3b). La seule formule `BuildGridSlot + 1` est la regle script `Script\Player\HOLCommandGridGroups.as:34`. `GetCycleRank` (le pont) GARDE : appele par `HOLPlayerController.cpp:1903, 1904, 1909` (tri / cycle des batiments).

## 3. Nettoyage
- `HOLCommandGrid.h` : les 2 declarations `*Native` et leurs commentaires « KEPT » retirees ; le commentaire de `GetCommand` dit la regle (script seul, pas de plateau = commande vide + une ligne de journal).
- `HOLCommandGridBoard.h:14-16` : commentaire « keep their C++ body (GetCommandNative / GetOrderNative) ... removed in a later lot » remplace par la regle (script seul, defaut neutre + ligne de journal).
- `Script\Player\HOLCommandGridKeys.as:3-4, 9` : plus de « C++ file is NOT touched / two copies » ni `GetCycleRankOfType` ; cite `HOLCommandGridGroups::GetCycleRank`. Meme nettoyage des en-tetes de `HOLCommandGridGroups.as:3-4`, `HOLCommandGridRules.as:3`, `HOLCommandGridBoard.as:2-4` (commentaires seuls, aucune ligne de code script touchee).
- Fonctions d'aide statiques, tables, includes : rien de mort (verifie) : les includes du .cpp (Engine, World, ScopeLock, ObjectKey, IConsoleManager, HOLBuildingTypes, HOLCommandGridBoard) servent tous au journal, au cvar ou a GetCycleRank ; les constantes d'en-tete (`CancelCell`, `AttackMoveCell`, `GetSpecializationCell`, `GetRageCell`, `GetHeroAbilityCell`) sont lues par la bibliotheque de lecture / le script ; `HOLHeroAbilityRules.h` reste pour `EHOLHeroAbility` de l'en-tete.
- Non touches : `GetMouseCursor`, `GetAgePanelOpacity`, `SetSpecialization`, `CheckResearch`, `HOLCommandGridBoard.cpp`, `Script\Tests\*`.

LIGNES_CPP_SUPPRIMEES: 102 (`git diff --numstat integ-20 HEAD` : `.cpp` +8 / -98, `.h` +1 / -4, `HOLCommandGridBoard.h` +2 / -2 ; net C++ = -93 ; plus 9 lignes de commentaire script).

## C++ qui RESTE (`HOLCommandGrid.cpp` 433 l., `.h` 489 l.)
- `LogNoBoardOnce` (anonyme) + `LogHOLCommandGridBridge` : journal « pas de plateau », une fois par fonction et monde.
- `CVarSpecializationCell` + `GetSpecializationCell` : reglage hol.UI.SpecializationCell (cvar C++, lu par le script via la bibliotheque).
- `GetOpenSpecializationTier`, `SetSpecialization` : lit `FHOLSpecializationView` (struct C++), remplit le contexte (remis a plus tard, hors lot).
- `GetReportName` : nom de rapport d'une commande de specialisation.
- `GetCommand`, `GetOrder` : ponts seuls vers le plateau script.
- `GetRageCell`, `GetHeroAbilityCell`, `GetTradeCell`, `GetTradeGain`, `GetTradeGainText` : petites constantes / formats lus par le HUD et la bibliotheque.
- `GetMouseCursor`, `GetAgePanelOpacity` : appelees a chaque image, restent C++ (consigne).
- `IsBuildGrid`, `GetCategoryRow`, `GetCellFace`, `FindMissingResource`, `IsPopulationBlocked`, `CheckResearch` : regles simples sans jumeau script en pont ou lisant des structs C++ non exposees (CheckResearch : FHOLTechnologyRow).
- Ponts seuls (defaut neutre + ligne de journal) : IsAgeReached, IsAgeOpen, GetAgeBuilding, PressKey, ClickAge, GetChosenAgeAfter, CheckProduction, CheckBuild, Blocks, Describe, GetCycleRank, IsSelectedByCategory, IsRepeatPress, NextInCycle, IsAloneInGroup, SplitSubgroups, GetCommandPriority, MergeCommands, PickEvenly.

## Preuve a rejouer (session principale)
1. Build : plus aucun symbole `GetCommandNative` / `GetOrderNative` (Grep Source\ doit rendre 0). Sinon erreur de nom.
2. `quick_check -Parts tests` : `HOLSpecializationGridTests` (monde Game) doit rester VERT : s'il tourne ROUGE avec une commande `None` au lieu de Produce/Specialize, le plateau script de la grille n'existe pas dans ce monde de test (le journal montrera `CommandGrid: GetCommand refused, no script grid board`).
3. `tier_grid_client_run.ps1 -Root <worktree>` : cellule Peasant enabled, journal client et serveur SANS « CommandGrid: ... refused, no script grid board ».
4. Falsification : bouchon `return FHOLGridCommandView();` dans `GetCommandAnswer` (`HOLCommandGridBoard.as:13`) = rouge (cellule vide) ; ou test C++ sans monde appelant `GetOrder` : doit rendre vide + une ligne `CommandGrid: GetOrder refused, no script grid board`.

## NON VERIFIE
- Compilation (aucun build) : les deux ponts reprennent le patron des 19 autres, `LogNoBoardOnce` etait deja dans le fichier.
- `HOLSpecializationGridTests` : la presence du plateau script de la grille dans son monde `CreateWorld(Game)` (le plateau specialisation `UHOLSpecializationSubsystem` y est, pas necessairement `UHOLCommandGridBoard`).
- Appelants hors `Source\` et `Script\` (Plugins, Tools) non cherches.
- Le `.cpp` est en LF (git le normalise en CRLF) ; `eol_check.ps1 -Fix` si un Edit multi-lignes est refuse.

## Anomalies
- La tache supposait une formule de rang `slot + 1` en C++ vers la ligne 488 : elle n'existe plus (retiree en L3a, la ligne 488 est maintenant `SplitSubgroups`).
- `integ-20` : `GetCommand` / `GetOrder` etaient les derniers ponts avec repli ; la regle « pas de plateau = grille vide » est maintenant totale pour les 21 fonctions.

APPELS_DECLARES: 14 ; HEURE_DEBUT/HEURE_FIN: 17:59 / 18:02 ; PROCESSUS_DETACHES: aucun (rien lance) ; BUILDS_DEMANDES: 0 ; TESTS_CPP_RETIRES: 0 ; TESTS_SCRIPT_AJOUTES: 0 ; LIGNES_MIGREES_PAR_LOT: 0 ; TYPES_A_EXPOSER: aucun.
