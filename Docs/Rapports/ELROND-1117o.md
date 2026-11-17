# ELROND-1117o : MIG-C-Construction, suite (NON COMPILE : C++ ecrit, ni build ni quick_check)

APPELS_DECLARES: 17 (arret a 25 vus ; seuil 80 juge sur tool_uses)
HEURE_DEBUT/HEURE_FIN: 18:22 / 18:26 (Get-Date)
PREMIER_BUILD: sans build (C++ NON COMPILE)   BUILD: NON COMPILE   LAUNCHCHECK_DEBUT: sans build
TEST_ROUGE: tache 1 : rouge deja vu par ELROND-1117n sur Test_Construction_Grid (vert 1501/1503 au build groupe). Tache 2 : pas de rouge (non compile) ; assertit l'effet : oui, mais RIEN n'est prouve. BUILDS_DEMANDES: 0 (la session principale doit compiler le C++ de liaison).
COMMITS: 2 sur tiers, merge_main_fait: non (pas de build, DLL non verifiee) ; 2bbb60f, da6e05e
TESTS: aucun lance. TESTS_TAUTOLOGIQUES: 0 (les valeurs viennent des regles C++ ; les verifs d'encodage MakeWorkBuild(0) != MakeWorkGather(0) testent l'encodage int des wrappers)
ESSAIS: aucun. QUESTIONS: aucune.

## FAIT
- Tache 1 (2bbb60f) : HOLConstructionTests.cpp 1174 -> 613 lignes. Retires 13 tests : Grid, Placement, Age, Payment, Damage, Builders, Population, States, Probes, BuildSpots, NextSite, CutPaths, SiteKeepOut. Helper TestExponent supprime (House, MakeQuery, IsOnGrid, TestTile restent : Tables, CampSource, FreeFootprint, Body les utilisent). Body GARDE, reduit a sa fin (UHOLBuildingSettings::GetBodyInset/GetBodySize/GetVisualInset/GetVisualSize, non portee, non exposee) ; ses 14 assertions de regles pures sont dans Test_Construction_Body du script. Reste en C++ : Tables, BuildQueue, WorkQueue, CampSource, IdleWall, Body (reduit), FreeFootprint = 7 tests.
- Tache 2 (da6e05e) : UHOLConstructionReadLibrary etendue (.h/.cpp, namespace anonyme existant inchange, aucun nouveau nom anonyme) : GetSourceGap, SendsBuildersToGather(FName, TArray<int32> ressources 0 Food..3 Gold), AddToBuildQueue(TArray<int32>&, Item, bAppend), AdvanceBuildQueue(TArray<int32>&, ClosedSites), MakeWorkBuild/MakeWorkGather/MakeWorkDeposit (n*4, n*4+1, n*4+2), GetWorkItemKind, AdvanceWorkQueue(TArray<int32>&, bHeadDone, ClosedSites) (decode vers TWorkItem<int32>, appelle HOLConstruction::AdvanceWorkQueue, recode). Script\Tests\HOLConstruction_Test.as : Test_Construction_BuildQueue, Test_Construction_WorkQueue, Test_Construction_CampGap (SendsBuildersToGather x5 + GetSourceGap x3 de CampSource), helpers ConInts/ConSameInts.
- TESTS_CPP_RETIRES: 13 (Construction.Grid, Placement, Age, Payment, Damage, Builders, Population, States, Probes, BuildSpots, NextSite, CutPaths, SiteKeepOut)
- TESTS_SCRIPT_AJOUTES: 3 (Test_Construction_BuildQueue, Test_Construction_WorkQueue, Test_Construction_CampGap), NON COMPILES
- LIGNES_CPP_SUPPRIMEES: 561 (tests) ; LIGNES_MIGREES_PAR_LOT: 0 retire pour la tache 2 (+103 lignes de liaison C++ ajoutees)

## RESTE (pour la session principale)
1. Build groupe (`Tools\build.ps1 -LiveCoding=0 -Wait` dans HeroesOfLiberty-Elrond), puis `quick_check -Parts tests,Script` (ou le filtre HOLConstruction).
2. Rouge VU sur Test_Construction_BuildQueue et WorkQueue (fausser un attendu, p. ex. `ConInts(1, 2, 3)` -> `ConInts(1, 3, 2)`), puis vert ; ensuite retirer les C++ `Construction.BuildQueue` et `Construction.WorkQueue` (les lignes SendsBuildersToGather/GetSourceGap de CampSource peuvent etre retirees du C++ CampSource apres le rouge de CampGap, le reste de CampSource reste).
3. Si le build refuse : points a verifier d'abord (code jamais compile) : appel `HOLConstruction::AddToBuildQueue<int32>` / `AdvanceBuildQueue<int32>` depuis des statiques du meme nom dans la classe (qualifie, devrait passer) ; `UPARAM(ref) TArray<int32>&` ; Angelscript : `TArray<int32>` passe par valeur dans ConSameInts, `n"..."` pour les FName.
4. Docs : le commentaire d'en-tete de HOLConstructionReadLibrary.h dit encore les files non exposees (a corriger au build : seules les files d'int le sont maintenant).

## TYPES_A_EXPOSER
FHOLCampSource (FindCampSource : ou tableaux paralleles centre, rayon, ressource int, debout), FWallBody + EHOLUnitActivity / EHOLMovePhase (IdleWall), FHOLBuildingTypeRow (Tables), FindFreeFootprint / FindObstacle avec TFunctionRef (une liste de boites "hors navmesh" en entree), UHOLBuildingSettings (fin de Body : GetBodyInset, GetBodySize, GetVisualInset, GetVisualSize, ou les lire via une bibliotheque de reglages). Rien force.

NON VERIFIE: tout le C++ de liaison (aucune compilation), les 3 tests script (jamais parses), le retrait de 13 tests (aucune compilation du fichier de tests : variable `Peasant` du Body reduit, includes laisses tels quels).
PROCESSUS_DETACHES: aucun processus detache pour C:\Users\baudo\Documents\HeroesOfLiberty-Elrond
ANOMALIES_PROCESSUS: aucune. Hook : heredoc Bash casse (ecrit par Write puis ajoute par PowerShell) ; Git Bash `date` diverge (utilise Get-Date).
GIT_STATUS_PRINCIPALE: hors docs voulus : oui (seuls EtapeDefini.md, Auto_Journey_File.md modifies par moi ; Auto_Journey_Jour.md et les rapports sont d'autres sessions)
