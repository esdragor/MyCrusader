# ELROND-1117n : MIG-C-Construction (C++ de liaison, NON COMPILE)

NON COMPILE : le C++ (bibliotheque) et le script de test n'ont JAMAIS ete compiles ni lances. Aucun build, aucun quick_check.

APPELS_DECLARES: ~24 (arret a 25 vus)
HEURE_DEBUT/HEURE_FIN: 18:05:03 / ~18:10
PREMIER_BUILD: sans build (NON COMPILE)   BUILD: NON COMPILE   LAUNCHCHECK_DEBUT: non lance (worktree DLL perimee, pas de build)
TEST_ROUGE: aucun (rouge non vu) + assertit l'effet : oui (valeurs calculees par la regle C++ via la bibliotheque)
COMMITS: 2 sur tiers (d2413e9, 38dd6e7), dernier 18:09, merge_main_fait: non (DLL perimee)
TESTS: aucun lance ; TESTS_TAUTOLOGIQUES: 0 (chaque valeur est calculee par HOLConstruction::*)
ESSAIS: aucun
QUESTIONS: aucune
BUILDS_DEMANDES: 1 (lot C++ : Buildings/HOLConstructionReadLibrary.h/.cpp)
TESTS_CPP_RETIRES: 0 (gardes : tant que le rouge n'est pas vu)
TESTS_SCRIPT_AJOUTES: 14 : Test_Construction_Grid, Placement, Age, Payment, Damage, Builders, Population, States, Probes, BuildSpots, NextSite, CutPaths, SiteKeepOut, Body (Script\Tests\HOLConstruction_Test.as)
LIGNES_CPP_SUPPRIMEES: 0
LIGNES_MIGREES_PAR_LOT: environ 750 des 1174 (14 tests sur 20)

## Fait
- `Source\HeroesOfLiberty\Buildings\HOLConstructionReadLibrary.h/.cpp` : `UHOLConstructionReadLibrary` (namespace script `HOLConstructionRead`), `USTRUCT FHOLConstructionBox` (Min, Max, bValid ; bValid faux = pas de boite, p. ex. pas de limite de carte). Enums passes en int (etat 0 Foundation 1 UnderConstruction 2 Finished ; attaque 0 Melee 1 Ranged 2 Siege 3 Hero ; refus 0 None 1 OutOfMap 2 Building 3 Source 4 EnemyCentre 5 NavMesh). Sources de placement : FVector (X, Y, rayon Z). Noms de namespace anonyme prefixes `ConstructionRead_` (unity).
- `Script\Tests\HOLConstruction_Test.as` : memes vecteurs, un par un.
- Population utilise `HOLProductionRead::GetPopulationCap` (existe deja), pas de doublon.

## A faire par la session principale
1. Build : `C:\Users\baudo\Documents\HeroesOfLiberty-Elrond\Tools\build.ps1 -LiveCoding=0 -Wait` (sur tiers, apres merge main si utile). Fautes C++ probables a surveiller : `FBox2D::bIsValid` (uint8 : j'ai ecrit `!= 0`), conversion implicite `TArray<FBox2D>` vers `TArrayView<const FBox2D>` / `TConstArrayView`, `static_cast<EHOLConstructionState>` (enum defini dans Buildings/HOLBuildingTypes.h, EHOLAttackType dans Simulation/HOLUnitTypes.h, deja inclus).
2. Fautes script probables : concatenation `"texte" + int/double` dans les messages (si refusee, remplacer par `"" + ...` ou retirer), `Math::Fmod`, `string Where` (type string vs FString), `AssertEquals(bool, bool, ...)` (Age), `for (float Yaw : Yaws)`. Si une surcharge manque, corriger le .as sans toucher aux vecteurs.
3. Rouge vu : fausser UNE valeur attendue (p. ex. `T.AssertEquals(20, Spots.Num()` -> 19, ou Payment 33 -> 34), voir N-1/N, remettre ; puis vert ; puis retirer des tests C++ les 14 tests portes et leurs helpers inutilises (House, MakeQuery, IsOnGrid... restent tant que les 6 autres s'en servent).

## Etat exact pour l'agent suivant
- PORTES (14) : Grid, Placement, Age, Payment, Damage, Builders, Population, States, Probes, BuildSpots, NextSite, CutPaths, SiteKeepOut, Body. Body : la fin du test C++ (`UHOLBuildingSettings::GetBodyInset/GetBodySize/GetVisualInset/GetVisualSize`, 5 assertions) n'est PAS portee (reste dans le C++, ou a lire via les reglages si UHOLBuildingSettings est visible du script).
- RESTANTS (6) : Tables (l. 308-477, table des types de batiments : lignes FHOLBuildingTypeRow), BuildQueue (l. 618, template AddToBuildQueue), WorkQueue (l. 651, TWorkItem / AdvanceWorkQueue), CampSource (l. 731, FHOLCampSource, FindCampSource, GetSourceGap, SendsBuildersToGather), IdleWall (l. 872, FWallBody, EHOLUnitActivity, EHOLMovePhase, StandsClearOfVisibleWall, KeepOutOfVisibleWall, EaseOutOfVisibleWalls), FreeFootprint (l. 1083, FindObstacle / FindFreeFootprint avec TFunctionRef).
- TYPES_A_EXPOSER : FHOLCampSource (ou tableaux paralleles : centre, rayon, ressource int, debout), FWallBody (centre, demi-taille, cos, sin), EHOLUnitActivity / EHOLMovePhase (en int), FHOLBuildingTypeRow (Tables), un wrapper non template pour les files d'int (BuildQueue / WorkQueue : TArray<int32> avec ensemble de sites fermes en TArray<int32>), FindFreeFootprint avec un TFunctionRef rempli par un predicat cote C++ (ex. une liste de boites "hors navmesh" en entree).

NON VERIFIE : tout (C++ et script non compiles, aucun test lance, rouge non vu).
PROCESSUS_DETACHES: aucun processus detache pour C:\Users\baudo\Documents\HeroesOfLiberty-Elrond
ANOMALIES_PROCESSUS: aucune ; IN_USE.txt du worktree passe a "libre" (CPP: oui) ; ancienne ligne `IN_USE` principale non touchee.
GIT_STATUS_PRINCIPALE: hors docs voulus (EtapeDefini.md, Auto_Journey_File.md) inchange : oui
