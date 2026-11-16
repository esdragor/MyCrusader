# ELROND-1117c : L17-09 et L17-11, ports script (tiers 88685fe, sans C++, sans build)

BUILDS_DEMANDES: 0
TESTS_CPP_RETIRES: 0 (aucun supprime ; voir registre)
TESTS_SCRIPT_AJOUTES: 17 (HOLOrderChain_Test.as : Test_OrderChains_Group/Peasants/Mix/Rally/Bound, Test_OrderBuildingQueue_AfterMove/TwoBuildings/AfterUnitAttack/ThenMove/Drop/Step/Deposit ; HOLMatchStats_Test.as : Test_MatchStats_Sampling/CanReceive/Scores/SeriesValues/Axes)
LIGNES_CPP_SUPPRIMEES: 0
LIGNES_MIGREES_PAR_LOT: ~495 lignes de script (HOLOrderChain_Test.as ~290, HOLMatchStats_Test.as ~100) ; ~495 lignes C++ de tests couvertes en partie
TYPES_A_EXPOSER: FHOLMatchStats, FHOLPlayerMatchStats, FHOLMatchStatsSample, FHOLMatchStatsChunk, FHOLMatchStatsEvent + Record*/AddSample/MakeChunk/MergeChunk/GetNextFromSample/MakeCurve ; EndBuildingTarget (template TWeakObjectPtr) a rendre appelable (ou a laisser en C++)
TEST_ROUGE: vu a 00:44 : 6 valeurs attendues faussees (Group 4->5, AfterMove 7->8, Sampling 200->201, CanReceive false->true, Scores 39->40, Axes 300->301), quick_check tests 1387/1395 (6 rouges nouveaux + 2 connus), remises, vert 1393/1395 (2 rouges connus D831 : NomadFirstForum, NomadReplicated), 2 passes vertes (00:39, 00:46)
TESTS_TAUTOLOGIQUES: 0 (le modele d'unite du script appelle IsCurrentOrderOver / MaySeek de la bibliotheque ; seul Advance est local au script, comme la FUnitModel du test C++)
EDITS_REFUSES: 0
PROCESSUS_DETACHES: voir fin

## Registre : test C++ -> tests script
| Test C++ | Tests script | Retirable ? |
|---|---|---|
| HeroesOfLiberty.Orders.Chains (HOLOrderChainTests.cpp) | Test_OrderChains_Group, _Peasants, _Mix, _Rally, _Bound | OUI (tous les blocs portes, memes vecteurs ; Point des etapes non compare, comme avant dans Group/Peasants) |
| HeroesOfLiberty.Orders.BuildingQueue | Test_OrderBuildingQueue_AfterMove, _TwoBuildings, _AfterUnitAttack, _ThenMove, _Drop, _Step, _Deposit | NON : les assertions apres EndBuildingTarget restent seulement en C++ (AfterMove fin, TwoBuildings fin, ThenMove fin, Drop derniere ligne) |
| HeroesOfLiberty.Match.Stats.Rules (HOLMatchStatsTests.cpp) | Test_MatchStats_Sampling | NON : RecordUnitDeath/Trained/BuildingLost/AddSample/RecordEvent non exposes |
| HeroesOfLiberty.Match.Stats.Send | Test_MatchStats_CanReceive | NON : MakeChunk/MergeChunk/GetNextFromSample non exposes |
| HeroesOfLiberty.Match.Stats.Scores | Test_MatchStats_Scores | OUI en substance (la bibliotheque prend les comptes, pas FHOLPlayerMatchStats : les 5 vecteurs portes) ; a retirer quand la ligne 'nothing done' et 'unchanged' sont jugees equivalentes (elles le sont) |
| HeroesOfLiberty.Match.Stats.Curves | Test_MatchStats_SeriesValues, Test_MatchStats_Axes | NON : MakeCurve (FVector2D, FHOLPlayerMatchStats) non expose |

Retirables maintenant : Orders.Chains et Match.Stats.Scores. Non supprimes (consigne).

## Non porte (liste)
- EndBuildingTarget (BuildingQueue : 4 blocs, apres-End).
- Rules : RecordUnitDeath, RecordUnitTrained, RecordBuildingLost, AddSample (maxima, Gathered du dernier), RecordEvent.
- Send : MakeChunk, MergeChunk, GetNextFromSample, rejeu de 4 morceaux.
- Curves : MakeCurve (points FVector2D).

## Commande pour la session principale
`Tools\Tests\quick_check.ps1 -Parts tests` dans le worktree HeroesOfLiberty-Elrond (fait deux fois : 1393/1395, rouges connus seuls). Merge main non fait (DLL pas reconstruite sur main).