# HALDIR-1119w : lot P1a (ordres du joueur, premiere moitie), worktree HeroesOfLiberty-Haldir, branche `selection`

Impact joueur : nul (le jeu ne change pas : rien n'appelle encore le script, `HOLPlayerOrders.cpp/.h` intacts). Tout est NON COMPILE.

## Fait
- `3f8a066` (liaison C++, raison D839 : liaison a exposer) : `Source\HeroesOfLiberty\Commands\HOLCommandScriptLibrary.h/.cpp`, +81 l.
  - 6 emetteurs `IssueMovePlayer / IssueAttackPlayer / IssueAttackMovePlayer / IssueAttackBuildingPlayer / IssueStopPlayer(bHold) / IssueToggleDeployPlayer` : `FPlayerOrderScope` ouverte DANS la fonction C++ (struct publique, HOLCommandSubsystem.h:109-116 lu) puis `Issue*Order` avec `ToUnitIds`.
  - `GetMaxSlots()` (BlueprintPure) = `FHOLMatchConfig::MaxSlots` ; ajout de `#include "Match/HOLMatchConfig.h"`.
  - `bChain` ajoute a `FHOLScriptOrderRead`, rempli dans `ConsumeMoves` depuis `FHOLMoveOrder::bChain` (HOLUnitCommand.h:94). Les autres `Consume*` ne le remplissent pas (inutile ici).
- `02fa4fb` : `Script\Commands\HOLPlayerOrders.as` (namespace `HOLPlayerOrders` : `GetMaxOrderedUnits`, `IsAccepted`, `Move`, `Attack`, `AttackBuilding`, `AttackMove`, `Stop`, `ToggleDeploy`, chacune renvoie bool ; garde = sous-systeme non nul + `IsAccepted` + batiment non nul pour AttackBuilding) et `Script\Commands\HOLPlayerOrders_Test.as` (8 tests).
- `SendNonWorkersTo` reste C++ (lit `HOLUnitView`).

## Tests script ajoutes (8, sur `MakeDetachedForTest`)
`Test_PlayerOrdersIsAccepted`, `Test_PlayerOrdersMoveEntersQueue`, `Test_PlayerOrdersRefusedForBadSlotOrTooMany`, `Test_PlayerOrdersToggleDeployKind`, `Test_PlayerOrdersStopVsHold`, `Test_PlayerOrdersAttackKindsAndTarget`, `Test_PlayerOrdersAttackBuildingNullRefused`, `Test_PlayerOrdersPlayerScopeDrawsChain`.
- Rouge prevu (non pose, non vu) : dans `Test_PlayerOrdersIsAccepted`, changer `201` en `200` dans `AssertFalse(IsAccepted(1, 201))` : 1 test rouge. Remettre 201, revoir vert. Alternative : `MaxOrderedUnitsValue = 200` -> `201` dans `HOLPlayerOrders.as` : rouge sur le meme test (et sur `GetMaxOrderedUnits == 200`, et `RefusedForBadSlotOrTooMany` sur 201 ids).
- Decouverte : `bChain` d'un ordre de combat = portee joueur ET Kind == Attack seulement (HOLCommandSubsystem.cpp:266) ; un Stop du joueur ne dessine pas de chaine. Le test `PlayerScopeDrawsChain` verifie : Move joueur = chaine, `IssueMove` brut = pas de chaine, Attack joueur = chaine, Stop joueur = pas de chaine.

## Champs
- BUILDS_DEMANDES: 0 (la session principale compile le lot C++ + scripts en un build)
- TESTS_CPP_RETIRES: 0
- TESTS_SCRIPT_AJOUTES: 8 (liste ci-dessus)
- LIGNES_CPP_SUPPRIMEES: 0 ; LIGNES_MIGREES_PAR_LOT: 0 supprimees (109 l. de HOLPlayerOrders.cpp:16-124 reecrites en script, retrait en P1b)
- TYPES_A_EXPOSER: aucun (`FVector2D`, `AHOLBuilding`, `UHOLCommandSubsystem` supposes deja lies : voir NON VERIFIE)
- TEST_ROUGE: non vu (pas de build autorise)
- TESTS_TAUTOLOGIQUES: 0 ; EDITS_REFUSES: 0 ; PROCESSUS_DETACHES: aucun lance

## Accord a prouver en P1b
Deux copies de la regle coexistent jusqu'a P1b (`HOLPlayerOrders.cpp:16-124` C++ qui decide, `HOLPlayerOrders.as` script). Les 8 tests couvrent les memes cas que le C++ (HOLAIOrdersTests.cpp:159-164 pour IsAccepted, refus, kinds). Pas de test d'accord direct C++/script : a ecrire en P1b (le pont rendra le C++ appelant du script, donc une seule source). Journal du chantier : le jumeau est temporaire, ce n'est pas un provisoire de jeu (Placeholders.txt non touche).

## Commande a lancer (session principale, un build groupe)
`Tools\build.ps1 -LiveCoding=0 -Wait` dans `C:\Users\baudo\Documents\HeroesOfLiberty-Haldir`, puis `Tools\Tests\quick_check.ps1 -Parts tests` : doit montrer les 8 `Test_PlayerOrders*` verts + les tests existants `Test_CommandLibrary*` toujours verts (la struct a un champ de plus).

## NON VERIFIE
- Rien n'est compile (C++ ni script).
- Angelscript : `UHOLCommandSubsystem` / `AHOLBuilding` utilises comme type handle sans `@` (comme `HOLCommandLibrary_Test.as`) ; `FVector2D(0.0, 0.0)` et `AHOLBuilding NoBuilding = nullptr` ; `const int` a l'interieur d'un `namespace` utilise dans le meme fichier (la valeur est aussi exposee par `GetMaxOrderedUnits()` car une constante n'est pas visible d'un autre fichier) ; que `HOLCommandScript::` expose bien les nouvelles statiques (nom de la bibliotheque sans le prefixe `UHOL` et `Command`... d'apres les tests existants : `HOLCommandScript::`).
- `FPlayerOrderScope` utilisable depuis la bibliotheque (struct publique, lu ; sa visibilite de `Guard` privee n'importe pas).
- Le comportement `IssueMoveOrder` sur un sous-systeme sans monde pour un joueur (slot 0/1) : les tests existants le font avec le slot 0 a 2 ; ici slots 0 et 1.
- Le test `ToggleDeploy` : `KindName` renvoie `ToggleDeploy` (lu dans le .cpp), mais que `IssueCombatOrder` mette l'ordre dans la file de mouvements (ConsumeMoves) n'est pas lu.
- Collision de nom possible : le namespace script `HOLPlayerOrders` homonyme du namespace C++ `HOLPlayerOrders` (aucun lien cote script attendu).

## Anomalies
- Aucune bloquante. `Crown.umap` non suivi laisse tel quel. Fichiers interdits non touches. Pas de merge de main (DLL non reconstruite).
