# CELEBORN-1117m : MIG-C-Vision (liaison C++ + 7 tests purs en script)

**NON COMPILÉ** : le C++ et le `.as` ne sont ni compilés ni exécutés (machine saturée, consigne). La session principale compile (lot C++ groupé) et lance les tests.

```
APPELS_DECLARES: ~19 (arrêt à 25 vus)
HEURE_DEBUT/HEURE_FIN: 17:47 / ~17:50
PREMIER_BUILD: sans build (NON COMPILÉ)   BUILD: NON COMPILÉ   LAUNCHCHECK_DEBUT: non lancé (DLL périmée connue)
TEST_ROUGE: non vu (aucun build) + assertit l'effet : oui (valeurs calculées par la règle C++ : GetDisc / GetSeenCells / FHOLVisionMap)
COMMITS: 1 (3988703 sur hero-j8-close, 17:49), merge_main_fait: oui (main fa1d9ab déjà fusionné avant le lancement)
TESTS: aucun lancé ; TESTS_TAUTOLOGIQUES: 0 (sauf Grid/Height, vecteurs constants voulus comme le C++)
ESSAIS: aucun
QUESTIONS: aucune
```

BUILDS_DEMANDES: 1 (le lot C++ de la session principale) ; TESTS_CPP_RETIRES: 0 ; TESTS_SCRIPT_AJOUTES: 7 (Test_VisionGrid, Test_VisionDisc, Test_VisionForest, Test_VisionHeight, Test_VisionCounts, Test_VisionTreeCut, Test_VisionSightRadius) ; LIGNES_CPP_SUPPRIMEES: 0 ; LIGNES_MIGREES_PAR_LOT: ~230 (corps des 7 tests, HOLVisionTests.cpp l.59-263).

## FAIT
- `Source\HeroesOfLiberty\Map\HOLVisionRulesReadLibrary.h/.cpp` (module HeroesOfLiberty) : `UHOLVisionRulesReadLibrary` (statiques, namespace script `HOLVisionRulesRead`) et `UHOLVisionMapHolder` (UObject qui tient un `FHOLVisionMap`, `Create(Side, CellSize, NumCamps)`), car Counts et TreeCut ont besoin d'un état de carte qu'un script ne peut pas tenir en classe C++ simple. Wrappers seulement, aucune règle dupliquée. Aucune bibliothèque de vision n'existait (`UHOLMinimapReadLibrary` n'expose pas la vision). Namespace du .cpp : `HOLVisionRulesReadLibDetail` (unique, builds unity).
- Grille = `Side` + `CellSize` ; cellule = X, Y ; `Blocked` = `TArray<bool>` (index Y*Side+X) ; vision d'une cellule = 0 Unexplored / 1 Explored / 2 Visible.
- Fonctions : GetGridNum, ToCell (out X, Y), IsInsideGrid, GetCellCenter, GetDiscNum, DiscContains, GetSeenCellsNum, SeenCellsContains, IsInLineOfSight, GetSightRadius (tuning par défaut), GetMaxHeightBonus, GetTileSize, ReadUnitSightRows, ReadBuildingSightRows (SightRadius : lit les vraies tables, mêmes assertions que le C++ ; la boucle est dans le script).
- `Script\Tests\HOLVisionRules_Test.as` : 7 tests, mêmes vecteurs que le C++ (13 / 6 / 197 / 1 cellules de disque, forêts 8..8 et 8..10 + 14..15, hauteur 3200 +/- 800, observateurs Scout / équipe, forêt coupée puis replantée). Hors dossiers interdits (`Script\Tests\`).
- Tests C++ NON supprimés. Docs : ligne d'EtapeDefini.md, ligne `[~]` d'Auto_Journey_File.md, IN_USE.txt (CPP: oui puis libre).

## RESTE (session principale)
1. Lot C++ groupé : `Tools\build.ps1 -LiveCoding=0 -Wait`, puis `quick_check -Parts tests` (les 7 tests `HeroesOfLiberty.Vision.*` C++ restent actifs).
2. Rouge vu (fausser une valeur, p. ex. 197 -> 198 dans Test_VisionDisc), puis vert.
3. Seulement alors : retirer du `.cpp` les 7 tests (Grid, Disc, Forest, Height, Counts, TreeCut, SightRadius) ; les helpers de `UE::HeroesOfLiberty::Tests::Vision` (MakeGrid, MakeWall, Sees, CountUnion) servent peut-être encore à d'autres tests du fichier : vérifier avant de les ôter.

## NON VÉRIFIÉ
Tout : compilation C++ (UHT, `double` en paramètre UFUNCTION, `void` BlueprintPure avec sorties, `Row.ToConstants(Name).SightRadius`, includes `Buildings/HOLBuildingTypes.h` et `Simulation/HOLSimulationSettings.h`), compilation et résolution du script. Points de risque côté script : `UHOLVisionMapHolder Map = UHOLVisionMapHolder::Create(...)` (si la liaison veut un handle `UHOLVisionMapHolder@`, adapter), concaténation `string + double` dans les messages, `FVector2D(150.0, -150.0)` et `+` sur FVector2D, `TArray<bool>` passé en `const TArray<bool>&` (un tableau vide = aucune forêt, géré par `MakeBlocked`), objet créé par `NewObject` non référencé ailleurs que par une variable locale (risque GC faible).

## TYPES_A_EXPOSER
Pour les 12 autres tests (classe A, hors lot) : `FHOLGhost` / `EHOLBuildingSight` / `EHOLBuildingDraw` (GetBuildingSight, MakeGhostKey, UpdateGhosts avec `TFunctionRef`, BuildingDrawsFromMemory, GetBuildingDraw, SourceStaysDrawn, GhostActorWanted), `EHOLFogMode` (ResolveFogMode), GetReceivedCamps / GetViewerCamp / HasStandingTeammate (`TFunctionRef`), `HOLFogPicture` (Fill, Read, ReadSmooth, Blur, ToUV sur `TConstArrayView<uint8>`), GetExploredRuns / AddExploredRuns (`TArray<uint32>`), AddBlocker / RemoveBlocker / GetBlockers (non exposés dans le holder), ExploreAll, TakeNewlyExplored ; `FHOLVisionTuning` n'est pas exposé (le tuning par défaut est figé dans la bibliothèque).

PROCESSUS_DETACHES: aucun lancé par moi (aucun éditeur, aucun build) ; ANOMALIES_PROCESSUS: aucune ; GIT_STATUS_PRINCIPALE: non modifiée par le code (seuls EtapeDefini.md, Auto_Journey_File.md, IN_USE.txt et ce rapport) : oui.
