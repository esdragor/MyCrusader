# ELROND-1117b : S1g, bibliotheques de lecture OrderChain / MatchStats / Production

- Branche tiers, commit 9a31252 (« S1g: read libraries for OrderChain, MatchStats, Production »), branche prete, merge main non fait. PAS DE BUILD, PAS DE quick_check. IN_USE : CPP oui puis libre.
- BUILDS_DEMANDES: 0 (la session principale compile ensuite : `Tools\build.ps1 -LiveCoding=0 -Wait` sur le worktree Elrond, puis compile des scripts).
- TESTS_CPP_RETIRES: 0. TESTS_SCRIPT_AJOUTES: 3 tests de fumee dans Script\Tests\HOLS1gLibraries_Test.as (Test_OrderChainReadLibrarySmoke, Test_MatchStatsReadLibrarySmoke, Test_ProductionReadLibrarySmoke). Il ne compile qu'apres le build ; rouge non vu (rien n'a tourne). Le port complet = lot suivant.
- LIGNES_CPP_SUPPRIMEES: 0. Lignes C++ ajoutees : 346 (6 fichiers neufs, aucun fichier existant modifie). LIGNES_MIGREES_PAR_LOT: 0.
- Motif C++ : regles qui restent en C++ (Mass / serveur), lues par le script (D836, recette S1d).

## Bibliotheques
1. Simulation\HOLOrderChainReadLibrary.h/.cpp : UENUM EHOLOrderChainStepKind, USTRUCT FHOLOrderChainStep (Kind, Point, TargetId, SourceIndex), FHOLOrderChainUnitSteps, FHOLOrderChain ; GetMaxSteps, MakeMove/Attack/Build/AttackBuilding/Deposit/Gather/Rally, MakeChains, KindName, IsCurrentOrderOver (5 bool), MaySeek (5 bool, HOLCombat::MaySeek). Script : HOLOrderChainRead::.
2. Match\HOLMatchStatsReadLibrary.h/.cpp : 9 fonctions a arguments plats : GetSampleSteps, IsSampleStep, CanReceiveStats, GetMilitaryScore(UnitsKilled), GetEconomyScore(FHOLResources, UnitsTrained), GetSeriesValue (valeurs a plat + indice de serie int), GetNiceStep, GetAxisEnd, FormatMatchTime. Script : HOLMatchStatsRead::.
3. Buildings\HOLProductionReadLibrary.h/.cpp (PARTIELLE) : GetPopulationCap, GetProgress(StartStep, Step, BuildSteps), IsWaitingForRoom. Script : HOLProductionRead::.

## Non exposables (non fait, ne pas forcer)
- OrderChain : EndBuildingTarget (template sur TBuildingRef) ; TargetId/UnitId passes en int32 (le uint32 de la regle n'est pas expose a Blueprint/script).
- MatchStats : MakeChunk, MergeChunk, GetNextFromSample, FindOrAddPlayer, RecordUnitDeath/Trained/BuildingLost/Event, AddSample, MakeCurve, DescribePlayer/Sample : FHOLMatchStats, FHOLPlayerMatchStats, FHOLMatchStatsSample, FHOLMatchStatsChunk, FHOLMatchStatsEventEntry sont des USTRUCT() sans BlueprintType ; EHOLMatchStatsSeries n'est pas un UENUM (indice int32). Pour les exposer il faut modifier HOLMatchStats.h (BlueprintType + BlueprintReadWrite), interdit dans ce lot : lot suivant (2 des 4 tests C++ en dependent : Rules, Send).
- Production : TryEnqueue, Cancel, Advance, GetStartedPopulation : FHOLProductionItem (USTRUCT() non BlueprintType), FHOLProductionCost (struct simple), TFunctionRef pour Advance. Meme remede (BlueprintType sur FHOLProductionItem + un struct de cout) ou un miroir USTRUCT, a trancher apres le build.

## NON VERIFIE
Rien n'est compile : UHT (UFUNCTION int64/double/FVector par valeur, uint8 UENUM, USTRUCT TArray<USTRUCT>), les noms d'espace de nom script (supposes : classe UHOLxxxReadLibrary -> HOLxxxRead, comme S1d), la construction FHOLResources() / assignation des champs en script, la valeur 0.5f comparee en AssertEquals float, et les 3 tests de fumee. Si UHT refuse `bool` sans defaut ou une signature, corriger au build.