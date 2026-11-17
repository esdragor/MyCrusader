# GLORFINDEL-1117g : L17-12 reste (HOLProduction Queue / Advance / Tables)

Worktree `C:\Users\baudo\Documents\HeroesOfLiberty-Glorfindel`, branche net-playtest, commit 2c06b4f (pas de push, pas de fusion).
Binaire PERIME (`Get-StaleBinaryProblem` : DLL 10:24 < HOLSiegeTests.cpp 10:49) et du C++ neuf a compiler : le rouge n'a pas pu etre vu.

## Registre C++ -> script

| Test C++ (HOLProductionTests.cpp) | Test script (Script\Tests\) | Etat |
|---|---|---|
| Production.Queue | Test_Production_Queue (HOLProductionQueue_Test.as) | ecrit, rouge a voir apres build |
| Production.Advance, moitie file (Advance, StartStep, Done, FreePopulation) | Test_Production_AdvanceQueue | ecrit, rouge a voir apres build |
| Production.Advance, moitie GetProgress / IsWaitingForRoom / plafond de population | Test_Production_Progress / _WaitingForRoom / _PopulationCap (HOLProduction_Test.as, ELROND-1117d) | rouge vu 1386/1389, vert 1387/1389 |
| Production.Tables | Test_Production_Tables | ecrit, rouge a voir apres build |

Aucun test C++ supprime (la suppression suit le rouge puis le vert). Les 3 tests C++ Queue / Advance / Tables restent donc.

## C++ ajoute (un petit lot, enveloppes seules, aucune regle)

- `Buildings\HOLProductionReadLibrary.h/.cpp` : `FHOLProductionQueueState` (USTRUCT BlueprintType : `Types` + `StartSteps`, deux tableaux paralleles) ; `TryEnqueue`, `Cancel`, `Advance` (un seul cout pour tous les types : `FHOLResources`, `BuildSteps`, `Population`) ; l'enveloppe convertit vers `TArray<FHOLProductionItem>` et appelle `HOLProduction::*`. `TryEnqueue` rend le refus en int (0 None, 1 NotProducible, 2 QueueFull, 3 NotEnoughResources).
- `Buildings\HOLProductionTablesLibrary.h/.cpp` (neuf) : `FHOLUnitTypeSummary`, `FHOLBuildingProductionSummary`, `FindUnitType`, `FindBuildingType`, `GetStartingResources` (espace de noms script `HOLProductionTables`). Les lignes `FHOLUnitTypeRow` et `FHOLBuildingTypeRow` n'ont que des UPROPERTY EditAnywhere (invisibles du script), d'ou ces copies plates ; enums rendus en int.

## A faire par la session principale

1. `Tools\build.ps1 -LiveCoding=0 -Wait` dans le worktree Glorfindel (un build, lot C++ unique ; risques de compilation : `UPARAM(ref)` sur `TArray<FName>&` et `int32&`, `int64` en UPROPERTY BlueprintReadWrite, include `Match/HOLResources.h` dans le .h de la bibliotheque de lecture).
2. `quick_check -Parts tests` : les 3 nouveaux tests script doivent passer (vert), puis faux une fois (ex. `20` -> `21` dans Test_Production_Queue, `70` dans le cancel) pour voir le rouge, puis retablir.
3. Apres rouge puis vert : supprimer `FHOLProductionQueueTest`, `FHOLProductionAdvanceTest`, `FHOLProductionTablesTest` de HOLProductionTests.cpp (le total quick_check ne doit pas baisser : 3 tests script ajoutes).
4. Hors lot : HOLProductionPopulationEndTests.cpp et HOLUnitTierProductionTests.cpp n'etaient pas dans cette tache et restent a analyser.

## Champs
BUILDS_DEMANDES: 1 (lot C++, par la session principale) ; TESTS_CPP_RETIRES: 0 ; TESTS_SCRIPT_AJOUTES: 3 (Test_Production_Queue, Test_Production_AdvanceQueue, Test_Production_Tables) ; LIGNES_CPP_SUPPRIMEES: 0 ; TYPES_A_EXPOSER: aucun de plus (FHOLProductionQueueState, FHOLUnitTypeSummary, FHOLBuildingProductionSummary ajoutes) ; TEST_ROUGE: non vu (binaire perime + C++ neuf).

## Suite GLORFINDEL-1117h (2026-11-17, net-playtest) : rouge vu, tests C++ retires

Rouge : une valeur fausse par test (`20`->`21` dans Test_Production_Queue "cost taken when queued", `10`->`11` dans Test_Production_AdvanceQueue "head started", `50`->`51` dans Test_Production_Tables "50 food") ; `Get-StaleBinaryProblem` vide ; quick_check tests 1391/1396 avec les 3 tests script en rouge. Valeurs remises : 1394/1396 (seuls NomadFirstForum / NomadReplicated, connus). Le rouge n'est pas commite.

| Test C++ supprime | Test script | Lignes C++ supprimees |
|---|---|---|
| FHOLProductionQueueTest (HeroesOfLiberty.Production.Queue) | Test_Production_Queue | 27 (l.36-62) |
| FHOLProductionAdvanceTest (HeroesOfLiberty.Production.Advance) | Test_Production_AdvanceQueue + Test_Production_Progress / _WaitingForRoom / _PopulationCap | 43 (l.64-106) |
| FHOLProductionTablesTest (HeroesOfLiberty.Production.Tables) | Test_Production_Tables | 98 (l.108-205) |

Le reste du fichier (includes, namespace Tests::Production : Flags, noms de types, MakeCost) n'etait utilise que par ces 3 tests (aucune autre reference) : `Source\HeroesOfLiberty\Tests\HOLProductionTests.cpp` supprime en entier (207 lignes au total). Total quick_check attendu : -3 tests (a verifier apres compilation par la session principale).
