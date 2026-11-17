# ELROND-1117p : MIG-C-Construction suite 2 (NON COMPILE : C++ et script jamais compiles ni lances)

APPELS_DECLARES: 25 (arret a 25 vus)
HEURE_DEBUT/HEURE_FIN: 18:48 / ~18:57 (Get-Date)
PREMIER_BUILD: sans build (NON COMPILE)   BUILD: NON COMPILE   LAUNCHCHECK_DEBUT: sans build
TEST_ROUGE: aucun (rouge non vu, rien compile) ; assertit l'effet : oui (valeurs calculees par les regles C++ via la bibliotheque)
COMMITS: 2 sur tiers (288433b, fe4202c), dernier 18:55, merge_main_fait: non (pas de build)
TESTS: aucun lance ; TESTS_TAUTOLOGIQUES: 0
ESSAIS: aucun. QUESTIONS: aucune.
BUILDS_DEMANDES: 1 (lot C++ : Buildings/HOLConstructionReadLibrary.h/.cpp)
TESTS_CPP_RETIRES: 2 (Construction.BuildQueue, Construction.WorkQueue ; 92 lignes) ; CampSource GARDE (FindCampSource n'est pas couvert par Test CampGap)
TESTS_SCRIPT_AJOUTES: 5 : Test_Construction_Tables, BodySettings, CampSource, IdleWall, FreeFootprint (Script\Tests\HOLConstruction_Test.as, memes vecteurs)
LIGNES_CPP_SUPPRIMEES: 92 ; LIGNES_MIGREES_PAR_LOT: ~430 (Tables 170, IdleWall 140, FreeFootprint 90, CampSource, Body)
TYPES_A_EXPOSER: aucun restant pour ce fichier (les C++ Tables, CampSource, IdleWall, Body, FreeFootprint restent jusqu'au rouge vu puis vert)

## Fait
- Tache 1 (288433b) : HOLConstructionTests.cpp 613 -> 521 lignes ; BuildQueue et WorkQueue retires (aucun helper devenu inutile : House, MakeQuery, IsOnGrid, TestTile servent encore) ; en-tete de HOLConstructionReadLibrary.h corrige (files d'int exposees).
- Tache 2 (fe4202c) : UHOLConstructionReadLibrary etendue (aucun nouveau nom de namespace anonyme hors un 2e bloc : ConstructionRead_FQueryData, _FillQuery, _ToResources) : USTRUCT FHOLConstructionCampSource (ressource en int) et FHOLConstructionWallBody ; GetSetting{TileSize,AntiRushRadius,BuilderExponent,BodyInset,BodySize,VisualInset,VisualSize} ; FindBuildingType(FName, ref FHOLBuildingTypeRow) + IsBuildingTypeBuildable / GetBuildingTypeCategory / GetBuildingTypeAccepts (ints) ; FindSourceTypeInfo ; FindCampSource ; StandsClearOfVisibleWall(int, int, bool) ; KeepOutOfVisibleWall(..., ref Local, ref bOutEased) ; EaseOutOfVisibleWalls(TArray<WallBody>, ...) ; SnapOutward ; FindObstacle et FindFreeFootprint : le TFunctionRef devient une liste de boites OffNavMesh (candidat dont le milieu est dans une boite = hors navmesh), la requete = boite, batiments, sources FVector(X, Y, rayon), carte, rayon anti-rush.
- Enums en int : categorie 0 Economic 1 Military 2 Technological ; ressource 0 Food 1 Wood 2 Stone 3 Gold ; activite 0 Idle 1 Moving 2 Gathering 3 GatherTrip 4 Building 5 Attacking 6 Dying ; phase 0 None 1 WaitingPath 2 Path 3 FinalLeg 4 FormUp 5 Block.

## A faire par la session principale
1. Build : `C:\Users\baudo\Documents\HeroesOfLiberty-Elrond\Tools\build.ps1 -LiveCoding=0 -Wait` (tiers). Fautes C++ probables : `FHOLPlacementCircle{ FVector2D, Source.Z }` (double depuis FVector.Z), `TArrayView` affecte depuis TArray, `FHOLCampSource{...}` agregat, `UHOLSourceSettings::FindType` (existe d'apres HOLSourceTypes.h), `UPARAM(ref) bool&`.
2. Script : fautes probables : champs de FHOLBuildingTypeRow lus depuis le script (Footprint, BuildGridSlot, PopulationProvided, Cost, BuildTime, Age, MaxHealth, Height, Walkable, Source, SightRadius, Produces : si un champ n'est pas visible, ajouter un accesseur), `Math::Floor(double)`, `Math::Sqrt`, `FUnitTest& T` en parametre d'un helper (ConCheckType), `FHOLPlaceHoldView` (de HOLFormationReadLibrary), parametres par defaut du helper ConSource, un `else` sans accolades apres `if/else`. Une variable `Disc` inutilisee dans FreeFootprint (a retirer si le compilateur la refuse).
3. Rouge vu : fausser UNE valeur (p. ex. Tables `Barracks.MaxHealth, 1500` -> 1501 ; FreeFootprint `Ring, 1` -> 2), voir N-1/N, remettre, vert ; puis retirer les C++ Tables, CampSource, IdleWall, FreeFootprint et la fin de Body (alors le fichier HOLConstructionTests.cpp disparait avec ses helpers) ; retirer de Source les lignes inutiles.

NON VERIFIE : tout (aucune compilation ni test).
PROCESSUS_DETACHES: aucun
ANOMALIES_PROCESSUS: heredoc python lance par erreur (bloque 2 min, arrete) ; Git Bash `date` pas utilise pour l'heure.
GIT_STATUS_PRINCIPALE: hors docs voulus (EtapeDefini.md, Auto_Journey_File.md) inchange : oui
