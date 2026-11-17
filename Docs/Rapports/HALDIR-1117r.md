# HALDIR-1117r - tests purs de HOLGatherTests.cpp / HOLBuildingTests.cpp vers script

Worktree : C:\Users\baudo\Documents\HeroesOfLiberty-Haldir, branche maps, commit 385450d. NON COMPILÉ, NON LANCÉ, rien poussé, main non fusionné ("merge main non fait", non demandé).

BUILDS_DEMANDES: 0
LANCEMENT: 2026-11-17 22:08 ; fin ~22:12 (arrêt à 17 appels environ)
TESTS_SCRIPT_AJOUTES: 4 (Test_Building_TypesMainCentre, Test_Building_TypesHouseProvisional, Test_Building_WalkableTable, Test_Building_MainCentreAcceptsFourResources) dans Script\Tests\HOLBuilding_Test.as
TESTS_CPP_RETIRES: 0 (la session principale retire après rouge vu puis vert)
LIGNES_CPP_SUPPRIMEES: 0
LIGNES_MIGREES_PAR_LOT: ~80 lignes script
TEST_ROUGE: non vu (rien lancé : machine saturée). A faire : fausser une valeur (ex. 1600 -> 1601) une fois, rouge, puis vert.
TESTS_TAUTOLOGIQUES: 0

## Constat HOLGatherTests.cpp (12 tests) : AUCUN portable aujourd'hui
Aucune bibliothèque script n'expose HOLGather (Gather, Deposit, IsFull, FindNearestDeposit, GetDepositDistance/Spot, IsAtDeposit,
GetSanctuaryFarmPercent...), FHOLSourceMassifs, FHOLGatherLoad/Source/Deposit/Site, ni la table des types de source
(UHOLSourceSettings::FindType, FHOLSourceTypeRow) ni la table d'unités (FHOLUnitTypeRow, ToConstants). Pas de C++ écrit (consigne).
- Pure mais non exposée : Load, Deposit, Choice, Massif, NearbyMassif, Spots (rules HOLGather + FHOLSourceMassifs), Tables (source/unité), Sanctuary (1re moitié : GetSanctuaryFarmPercent ; 2e moitié monde).
- Monde / Mass : VeinSizes, LinkedSource, FarmField, Income, (2e moitié de Sanctuary) : restent en C++ (D838).
- Seul morceau portable : dernier bloc de Tables (MainCentre accepte les 4 ressources), porté dans HOLBuilding_Test.as.

## Constat HOLBuildingTests.cpp (11 tests)
Types (pure, table) : porté (MainCentre + House). Walkable : règle BlocksNavigation déjà dans HOLConstruction_Test.as ; moitié table (Farm Walkable / Mill non) portée ; moitié monde reste C++.
Appearance, ForumColour, ArcheryRange, Foundation, Selection, Damage, FarmField, FarmClick, FarmShape : monde / AHOLBuilding spawné : restent en C++.

## Tests C++ prêts à retirer (après rouge vu puis vert des 4 tests script)
- HeroesOfLiberty.Buildings.Types : COMPLET (FHOLBuildingTypesTest) -> retirable en entier.
- HeroesOfLiberty.Buildings.Walkable : PARTIEL (le bloc table + règle ; la suite monde reste) : ne pas retirer le test, au mieux retirer les lignes 'Farm->Walkable / Mill->Walkable' et 'rule:'.
- HeroesOfLiberty.Gather.Tables : PARTIEL (le dernier bloc MainCentre->AcceptsResource) : ne pas retirer le test.
Aucun autre test retirable.

## TYPES_A_EXPOSER (par ordre de gain)
1. HOLGather (Gather, Deposit, StartGathering, IsFull, IsFullOf, FindNearestDeposit, GetDepositDistance, GetDepositSpot, IsAtDeposit, GetEdgeDistance, IsInContact, GetSanctuaryFarmPercent) + structs FHOLGatherLoad / FHOLGatherSite / FHOLGatherDeposit / FHOLGatherSource : permettrait Load, Deposit, Choice, Spots, Sanctuary (partie règle) en script.
2. FHOLSourceMassifs (Build / Search / GetLinks / GetMassif / GetNearbyMassifs) : Massif, NearbyMassif (la recherche prend un lambda : passer par une liste d'index refusés ou un TFunction).
3. Tables : UHOLSourceSettings::FindType + FHOLSourceTypeRow, ligne d'unités (CarryCapacity, GatherRates) : Tables (source/unité), à faire en lecture seule.
4. FHOLBuildingTypeRow : les champs lus par HOLBuilding_Test.as (DisplayName, Footprint, Height, MaxHealth, Mesh, Walkable, Accepts) sont EditAnywhere seulement ; seul GarrisonSlots est BlueprintReadWrite. Si le script refuse un champ ("not declared"), ajouter BlueprintReadOnly à ces UPROPERTY (changement C++ minime, un build).

## NON VÉRIFIÉ
Tout : aucun build, rien lancé. Risques : (a) accès aux champs EditAnywhere de la ligne depuis le script (point 4) ; (b) `Row.Accepts` (TArray<EHOLResource>) lu par `int(...)` ; (c) `FText.IsEmpty()`, `TSoftObjectPtr.IsNull()`, `FVector2D` comparé champ par champ en script ; (d) une erreur de compilation dans ce fichier casse le module script entier : lancer d'abord le chargement script (HOLScriptSmoke_Test) avant tout le reste. Si ça casse, supprimer HOLBuilding_Test.as, rien d'autre n'en dépend.
Commande : à la session principale (build non requis, script seul) : `quick_check -Parts script` sur le worktree Haldir.

## Anomalies
Aucune. Crown.umap non suivi laissé intact. PROCESSUS_DETACHES: non lancé (aucun processus démarré).
