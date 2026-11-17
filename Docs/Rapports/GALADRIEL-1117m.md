# GALADRIEL-1117m : MIG-C-CommandGrid (C++ de liaison + tests script)

**NON COMPILÉ** : aucun build, aucun quick_check, aucun éditeur (machine saturée). Le C++ de la bibliothèque et le script de test n'ont jamais été vus par un compilateur.

APPELS_DECLARES: 22 (arrêt à 25 vus)
HEURE_DEBUT/HEURE_FIN: 17:47:04 / 17:52 (Get-Date PowerShell)
PREMIER_BUILD: sans build   BUILD: NON COMPILÉ   LAUNCHCHECK_DEBUT: non lancé (aucun build prévu ; DLL périmée, main fa1d9ab fusionné sans build)
TEST_ROUGE: aucun (non exécutable ce soir) ; assertit l'effet : oui (chaque test appelle la vraie règle C++ par la bibliothèque et compare au vecteur du test C++) ; rouge vu à faire par la session principale : fausser une valeur attendue une fois (p. ex. `GetTradeCell(Food, true)` attendu 1 au lieu de 0), puis la remettre.
COMMITS: 1 (053fc99 sur galadriel-tab, 17:5x), merge_main_fait: oui (main fa1d9ab déjà fusionné par la session principale, aucune fusion par moi)
TESTS: aucun lancé ; TESTS_TAUTOLOGIQUES: 0 (les valeurs assertées sortent de HOLCommandGrid::*, jamais d'une constante recopiée)
ESSAIS: aucun
QUESTIONS: aucune
BUILDS_DEMANDES: 1 (lot C++ groupé de la session principale)
TESTS_CPP_RETIRES: 0 (les tests C++ de HOLCommandGridTests.cpp sont intacts)
TESTS_SCRIPT_AJOUTES: 10 (Test_CGMarketGain, Test_CGMarket, Test_CGCommands, Test_CGFeedback, Test_CGCycle, Test_CGCategories, Test_CGGroup, Test_CGSubgroups, Test_CGDistribution, Test_CGTables = 1re moitié seulement)
LIGNES_CPP_SUPPRIMEES: 0
LIGNES_MIGREES_PAR_LOT: ~550 lignes de test C++ portées (sur 926)
TYPES_A_EXPOSER: HOLTechnology::FFacts + FHOLTechnologyRow (CheckResearch), FHOLGridContext::HeroCooldownSeconds / bDeployables / bDeployedAny / bRageUnit / Buildable.bAlwaysOpen / bPrimaryForum / Specialization*, FHOLSpecializationView (SetSpecialization, GetOpenSpecializationTier), EHOLHeroAbility (GetHeroAbilityCell, GetRageCell, GetSpecializationCell, GetReportName), GetMouseCursor, IsAgeOpen, GetAgePanelOpacity, FHOLGridCommand::Ability / CooldownSeconds / bPack / Tier / Option / RemainingMs, EHOLPlacementRefusal (n'est pas un UENUM : passé en int32 + GetPlacementRefusalBuilding), HOLTechnology::EBlock

## Fait (commit 053fc99, galadriel-tab)
- `Source\HeroesOfLiberty\Player\HOLCommandGridReadLibrary.h/.cpp` : `UHOLCommandGridReadLibrary` (namespace script `HOLCommandGridRead`), statiques BlueprintPure, aucune règle (copie C++ <-> USTRUCT « View » et miroirs UENUM `EHOLGridCommandKind`, `EHOLCommandFeedbackKind`, `EHOLGridKeyTargetKind`, `EHOLAgeEventKind`). Espace de noms unique `HOLCommandGridReadLibraryPrivate` (unity builds). Fonctions : GetNumCells / GetCancelCell / GetGridColumns, GetCategoryRow, GetTradeCell / GetTradeGain / GetTradeGainText, GetCommand, GetOrder, IsBuildGrid, IsAgeReached, GetAgeBuilding, PressKey, ClickAge, GetChosenAgeAfter, WithTableBuildables, GetProducesOf, IsTypeInTable, GetTypeCategory, GetCycleRankOfType, IsTypeSelectedByCategory, IsCategorySelectedByCategory, HasMissingResource / GetMissingResource, CheckProduction, CheckBuild, Blocks, IsPopulationBlocked, Describe, GetCellFace, IsRepeatPress, NextInCycle, IsAloneInGroup, SplitSubgroups, GetCommandPriority, MergeCommands (tableau aplati, buildings bout à bout : pas de TArray imbriqué), PickEvenly. Les clés uint32 passent en int32 (même bits).
- `Script\Tests\HOLCommandGrid_Test.as` : 10 tests portés aux mêmes vecteurs (voir plus haut). Le test C++ `Tables` n'est porté qu'en 1re moitié (grille du bâtisseur, pages I et II, Barracks, Archery range) ; la 2e moitié (Siege workshop, Chapel, test `Monk` etc.) n'est pas portée. Aucun `AssertTrue` n'est utilisé comme valeur de retour (réécrit en `bool bOkN = ...`, la signature de retour de `FUnitTest.Assert*` n'étant pas vérifiable ici).
- Docs : EtapeDefini.md (ligne 7), Auto_Journey_File.md (ligne 44, `[~]`), IN_USE.txt du worktree (libre, CPP: oui).

## Reste (état exact pour l'agent suivant)
1. Non portés : `UnitOrders` (lignes 577-?), HeroAbility, Research (CheckResearch), Specialization, Deploy cell, report names, la 2e moitié de `Tables` (Siege workshop L538-554, Chapel L555-569, TestFalse centre L570). Ils lisent `bSoldiers` / `bOwnUnits` (présents dans `FHOLGridContextView`, donc `UnitOrders` n'a besoin que de nouveaux champs si il touche bOnlyKings / Deploy / bRageUnit : ajouter ces champs à la vue et à ToRule). Lire `HOLCommandGridTests.cpp` à partir de la ligne 577.
2. Session principale : lot C++ groupé, `Tools\build.ps1 -LiveCoding=0 -Wait`, puis `quick_check -Parts tests`. Si la compilation échoue, les suspects sont : `FHOLCycleBuilding` / `FHOLQueueChoice` en `Rule.Add({ ... })` (agrégats avec initialiseurs de membres), `TArray<uint32>` -> `TConstArrayView`, la clause `using namespace HOLCommandGridReadLibraryPrivate;` (collision de `ToRule` / `FromRule` d'un autre .cpp en unity build : si oui, qualifier les appels).
3. Si une classe script ne compile pas : noms de types UENUM natifs utilisés tels quels (`EHOLResource`, `EHOLBuildingCategory` sont des `UENUM()` sans BlueprintType : le plugin les lie normalement, à confirmer) ; chaînes `n"House"`, `n"LumberCamp"`, `n"MiningCamp"`, `n"Mill"`, `n"Farm"`, `n"SiegeWorkshop"`, etc. à vérifier contre `HOLBuildingTypeNames` ; `T.AssertEquals(5, Num, msg)` (entiers seulement, aucune comparaison de FString/FName par AssertEquals).
4. Rouge vu (règle) : fausser une valeur attendue dans un test (ex. Test_CGCycle, première assertion 1 -> 2), vu rouge, remettre ; puis seulement retirer les tests C++ portés de `HOLCommandGridTests.cpp` (un test C++ n'est retiré qu'après rouge puis vert). Total de `quick_check` : 15 tests C++ - N retirés + N scripts, jamais moins.

## NON VÉRIFIÉ
Tout : compilation C++, compilation script, exécution des 10 tests, comptes (13 types buildables, noms d'Archery range / Chapel). Le script a une 2e coupe : « Tables » assume la même table de bâtiments dans le jeu script que dans le test C++ (settings chargés par l'éditeur).

PROCESSUS_DETACHES: aucun processus detache pour C:\Users\baudo\Documents\HeroesOfLiberty-Galadriel
ANOMALIES_PROCESSUS: aucune
GIT_STATUS_PRINCIPALE: ` M Auto_Journey_File.md`, ` M Auto_Journey_Jour.md`, ` M EtapeDefini.md`, `?? Docs/Priorite_Socle_Factions_2026-11-16.md`, `?? Docs/Rapports/{CELEBORN,HALDIR,SAURON}-1117m.md` ; hors docs voulus : inchangé (aucun code touché dans la copie principale) : oui. (Auto_Journey_Jour.md est modifié par une autre session.)
