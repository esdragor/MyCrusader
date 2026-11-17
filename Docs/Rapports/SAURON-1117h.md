# SAURON-1117h : rouge VU puis retrait des tests C++ Match.Stats.* et des cas EndBuildingTarget

Worktree `C:\Users\baudo\Documents\HeroesOfLiberty-Sauron`, branche `visual`, commit `c9083d2` (anglais, pas de push, pas de fusion). BUILDS_DEMANDES: 1 (voir fin : le retrait d'un .cpp ne se voit au total qu'apres un build).
Binaire a jour (Get-StaleBinaryProblem vide).

## Rouge puis vert (une seule passe chacun, `quick_check.ps1 -Root <worktree> -Parts tests`)
Une valeur faussee dans CHACUN des 8 tests script de SAURON-1117g :
- Test_MatchStats_Record : UnitsKilled slot 0, 2 -> 3
- Test_MatchStats_Samples : Gathered.Food, 30 -> 31
- Test_MatchStats_Send : Chunks, 4 -> 3
- Test_MatchStats_Curve : Curve[1].Y, 7.0 -> 8.0
- Test_OrderBuildingQueue_EndAfterMove : OrderedId, 0 -> 1
- Test_OrderBuildingQueue_EndTwoBuildings : TargetId, 2 -> 3
- Test_OrderBuildingQueue_EndThenMove : AssertTrue(bWalking) -> AssertFalse
- Test_OrderBuildingQueue_EndOwnAndOther : OrderedId, 4 -> 5
ROUGE VU : `tests 1398/1408 : FAIL (REGRESSION)`, les 8 noms ci-dessus rouges (plus les 2 Nomad connus), « scripts compiled » KO.
Valeurs remises par `git checkout -- Script/Tests/HOLMatchStats_Test.as Script/Tests/HOLOrderChain_Test.as` (git status propre).
VERT VU : `tests 1406/1408 : FAIL (rouge connu : NomadFirstForum, NomadReplicated)`, aucune regression.

## Registre C++ -> script
| Test C++ | Script | Lignes C++ supprimees |
|---|---|---|
| HeroesOfLiberty.Match.Stats.Rules (Sampling, deaths, trained/lost, samples, event) | Test_MatchStats_Sampling, _Record, _Samples | fichier entier (voir ci-dessous) |
| HeroesOfLiberty.Match.Stats.Send (CanReceiveStats, chunks, MergeChunk) | Test_MatchStats_CanReceive, _Send | idem |
| HeroesOfLiberty.Match.Stats.Curves (GetSeriesValue, MakeCurve, GetNiceStep, GetAxisEnd, FormatMatchTime) | Test_MatchStats_SeriesValues, _Curve, _Axes | idem |
| Orders.BuildingQueue, cas « After a move » + EndBuildingTarget | Test_OrderBuildingQueue_AfterMove + _EndAfterMove | 13 |
| Orders.BuildingQueue, cas « Two buildings » | _TwoBuildings + _EndTwoBuildings | 17 |
| Orders.BuildingQueue, cas « Then a move » | _ThenMove + _EndThenMove | 10 |
| Orders.BuildingQueue, 2 lignes `EndBuildingTarget(Rest...)` + assert « At rest: its building fallen » (cas Drop) | _EndOwnAndOther | 3 |

`HOLMatchStatsTests.cpp` (211 lignes) supprime par `git rm` (les 3 tests, plus rien dedans). `HOLOrderChainTests.cpp` : -43 lignes ; le test `Orders.BuildingQueue` reste (cas AfterUnitAttack, Drop sans fin, Step, Deposit : equivalents script AfterUnitAttack, Drop, Step, Deposit existants mais NON rejoues rouge par cette seance, donc gardes).
Total : 254 lignes C++ supprimees ; TESTS_CPP_RETIRES: 3 (HeroesOfLiberty.Match.Stats.Rules, .Send, .Curves). Total quick_check attendu apres le build : 1408 - 3 = 1405 (il reste 1408 tant que la dll contient encore les tests).

## Gardes et raisons
- FindOrAddPlayer, DescribePlayer, DescribeSample : non exposes (aucun test C++ ne les assertait directement ; FindOrAddPlayer exerce par Record*).
- Ecarts d'assertion acceptes : (1) Rules assertait `Gathered == (30,20,0,0)` : le script assertait Food et Wood, Stone/Gold = 0 (valeur par defaut) non assertes ; (2) Send comparait les echantillons entiers (`==`) : le script compare Workers et TimeSeconds de chaque echantillon (les seuls champs renseignes).
- Rouge de Sampling, CanReceive, SeriesValues, Axes (portes par ELROND-1117c, deja verts) : non rejoue ici ; vus verts seulement.

## Commande
`powershell -NoProfile -File C:\Users\baudo\Documents\HeroesOfLiberty-Sauron\Tools\build.ps1 -LiveCoding=0 -Wait` puis
`powershell -NoProfile -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Sauron\Tools\Tests\quick_check.ps1 -Root C:\Users\baudo\Documents\HeroesOfLiberty-Sauron -Parts tests` (attendu 1403/1405, 2 Nomad connus).
TESTS_SCRIPT_AJOUTES: 0 ; LIGNES_MIGREES_PAR_LOT: 254 ; TYPES_A_EXPOSER: aucun.
