# SAURON-1117g : S1g suite (MatchStats Record/chunks/curve, EndBuildingTarget)

Worktree `C:\Users\baudo\Documents\HeroesOfLiberty-Sauron`, branche `visual`, `git merge main` : deja a jour. Commits (anglais, pas de push, pas de fusion) :
- `4f22df8` tests script (rouge par absence : ne compilent qu'apres le build).
- `099531f` lot C++ minimal (enveloppes seules).

BUILDS_DEMANDES: 1 (a lancer par la session principale). PAS DE BUILD fait par l'agent.

## Etat de depart corrige
AngelscriptInProgress.md (ligne S1g) disait TryEnqueue/Cancel/Advance non exposes : c'est perime, `2c06b4f` (GLORFINDEL-1117g) les expose deja
(`FHOLProductionQueueState` en tableaux paralleles) avec leurs tests script. Il restait donc : MakeChunk/MergeChunk/GetNextFromSample/Record*/AddSample/MakeCurve
de MatchStats, et EndBuildingTarget d'OrderChain.

## Lot C++ (4 fichiers, +374 lignes de C++, enveloppes seules, aucune regle)
- `Match/HOLMatchStatsReadLibrary.h/.cpp` : les structs de `HOLMatchStats.h` sont des `USTRUCT()` simples (non BlueprintType) : on n'y touche pas ;
  copies plates BlueprintType `FHOLMatchStatsSampleState`, `FHOLPlayerMatchStatsState`, `FHOLMatchStatsEventState` (Kind en int : 0 Defeated, 1 ForumFell),
  `FHOLMatchStatsState`, `FHOLMatchStatsChunkState`, converties par valeur (ToStats / FromStats dans le .cpp). Fonctions : RecordUnitDeath,
  RecordUnitTrained, RecordBuildingLost, RecordEvent (Kind hors bornes : rien), AddSample, MakeChunk, MergeChunk, GetNextFromSample, MakeCurve
  (SeriesIndex hors bornes : Y 0). Les USTRUCT originaux ne sont pas modifies (une seule source des regles : `HOLMatchStats::*`).
- `Simulation/HOLOrderChainReadLibrary.h/.cpp` : `EndBuildingTarget(int32& TargetId, int32& OrderedId)` (ids, 0 = aucun) ; l'enveloppe passe par une petite
  reference locale (Reset, ==) au gabarit `HOLOrderChain::EndBuildingTarget`, qui reste la seule regle.

## Tests script (rouge d'abord : commit separe, ne compilent qu'apres le build)
- `Script/Tests/HOLMatchStats_Test.as` : Test_MatchStats_Record, _Samples, _Send (3 chunks + le dernier redemande, remplacement du dernier echantillon),
  _Curve. Les vecteurs du test C++ `HeroesOfLiberty.Match.Stats.Rules/Send/Curves`.
- `Script/Tests/HOLOrderChain_Test.as` : Test_OrderBuildingQueue_EndAfterMove, _EndTwoBuildings, _EndThenMove, _EndOwnAndOther (les moities du test C++
  apres EndBuildingTarget, plus un cas « autre batiment frappe : l'ordre de 4 reste » qui n'existait pas en C++).
- Une fonction exposee = au moins un test de comportement : les 10 + 1 fonctions ci-dessus sont toutes assertees sur leur effet (MakeCurve, GetNextFromSample, MergeChunk par la chaine serveur -> copie).

## Rouge vu : NON (impossible sans build)
Le rouge actuel est le rouge de compilation (symboles absents). Apres le build : `quick_check -Parts tests` (ou la partie script) doit passer, puis falsifier
UNE valeur (ex. `T.AssertEquals(4, Chunks, ...)` -> 3) pour voir le rouge d'execution, avant de retirer les tests C++ `HeroesOfLiberty.Match.Stats.*`
(Rules, Send, Curves) et les cas EndBuildingTarget de `HeroesOfLiberty.Orders.BuildingQueue`. Pas de test C++ retire par cette seance.

## Commande de build (session principale)
`powershell -NoProfile -File C:\Users\baudo\Documents\HeroesOfLiberty-Sauron\Tools\build.ps1 -LiveCoding=0 -Wait`
puis `quick_check` sur la partie tests (agent_rules : `tests` seul en cas de doute).
Risques a surveiller au build : (1) `int32&` en `UPARAM(ref)` appele avec un champ de struct script (`Unit.TargetId`) : precedent `FreePopulation` de
Production ; (2) `TArray<FVector2D>` retourne en script ; (3) imbrication `TArray<FHOLPlayerMatchStatsState>` -> `TArray<FHOLMatchStatsSampleState>` (precedent OrderChain).

## Ce qui reste hors script, et pourquoi
- `HOLMatchStats::FindOrAddPlayer` : non expose (interne ; les Record* l'exercent). `DescribePlayer` / `DescribeSample` : chaines de journal, non exposees (aucun test script utile ; a ajouter si un outil script les veut).
- Test C++ de la simulation Mass : le sous-systeme `UHOLMatchStatsSubsystem` (enregistrement par pas sur le serveur, lit la simulation) et les processeurs
  Mass d'ordres (`HOLCombatProcessor`, qui appelle `EndBuildingTarget` avec des `TWeakObjectPtr`) restent en C++ : Mass / code par entite et par pas (D838).
- Replication : l'envoi par morceaux (RPC / replication client du `FHOLMatchStatsChunk` par `AHOLGameState` ou le sous-systeme) reste en C++ : reseau. Seule la regle pure (MakeChunk / MergeChunk) est lisible du script.
- Slate : l'ecran de statistiques (graphiques de la Chronologie) reste en C++ pour l'instant ; ses regles pures (GetNiceStep, GetAxisEnd, FormatMatchTime, MakeCurve) sont maintenant toutes en script.
- La copie plate des structs est un jumeau de DONNEES (pas de regle) : si un champ est ajoute a `FHOLMatchStats*`, la copie `*State` et ses conversions doivent suivre (a la charge du lot qui ajoute le champ).

## Champs
TYPES_A_EXPOSER: aucun de plus (FindOrAddPlayer, DescribePlayer, DescribeSample optionnels)
TESTS_SCRIPT_AJOUTES: 8 (Test_MatchStats_Record, _Samples, _Send, _Curve ; Test_OrderBuildingQueue_EndAfterMove, _EndTwoBuildings, _EndThenMove, _EndOwnAndOther)
TESTS_CPP_RETIRES: 0
LIGNES_CPP_SUPPRIMEES: 0
TESTS_TAUTOLOGIQUES: 0
NON VERIFIE : tout (aucun build, aucune execution).
