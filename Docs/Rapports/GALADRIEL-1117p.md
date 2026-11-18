# GALADRIEL-1117p : MIG-C-Formation (NON COMPILE, C++ de liaison + tests script)

NON COMPILE : ni la bibliotheque C++ ni le script n'ont ete compiles ni lances (aucun build, aucun quick_check, aucun editeur). Branche galadriel-tab, commit ef4c525, aucun push, merge main non fait.

APPELS_DECLARES: ~23 (arret a 25 ; heure d'arret 18:36)
HEURE_DEBUT/HEURE_FIN: 18:22:33 / ~18:28
PREMIER_BUILD: sans build   BUILD: NON COMPILE   LAUNCHCHECK_DEBUT: non lance (pas de build)
TEST_ROUGE: aucun (rouge non vu : rien lance) ; assertit l'effet : oui (valeurs calculees par HOLFormation, jamais recopiees)
COMMITS: 1 (ef4c525, 18:26), merge_main_fait: non
TESTS: aucun lance ; TESTS_TAUTOLOGIQUES: 0 (chaque valeur vient de HOLFormation::*)
ESSAIS: aucun
QUESTIONS: aucune
BUILDS_DEMANDES: 1 (lot C++ de la session principale) ; TESTS_CPP_RETIRES: 0 ; LIGNES_CPP_SUPPRIMEES: 0
TESTS_SCRIPT_AJOUTES: 11 (Test_FMRows, Test_FMKinds, Test_FMClasses, Test_FMPlaces, Test_FMAssign, Test_FMFront, Test_FMSpeed, Test_FMReturn, Test_FMYieldPlace, Test_FMCorridor, Test_FMBreakAtMelee)
LIGNES_MIGREES_PAR_LOT: ~ 640 lignes de test C++ couvertes (sur 961)

## Fait
- `Source\HeroesOfLiberty\Simulation\HOLFormationReadLibrary.h/.cpp` : UHOLFormationReadLibrary (script : `HOLFormationRead::`), statiques fines sur HOLFormation, namespace anonyme `HOLFormationReadLibraryPrivate` (unique), USTRUCT FHOLPlaceHoldView (miroir de FPlaceHold). Expose : GetConstant(FName) (priorites, ReturnRestSteps, MaxReturnTries, BlockBreakDistance, BlockMemberStuckSteps, FileSlideMaxAngle), MakePeasantRows/SquareRows/MixedRows/RowsOfWidth/GroupRows, PlaceKinds, GetRowPriority, GetRowClass (int : 0 Melee, 1 Ranged, 2 Siege, 3 Worker), MakePlaces, PlaceGroup (projection identite, sans IsFree), AssignPlaces, GetFrontWidth, GetFrontFacing, GetMarchSpeed, EndWalkToPlace, WatchPlace, ShouldYieldHeldPlace, ShouldBreakBlock, IsFightingFriend, IsMemberStuck, MakeCorridor, GetCorridorAt.
- `Script\Tests\HOLFormation_Test.as` : memes vecteurs un par un, du plus simple (Speed, BreakAtMelee, Front) au plus complexe (Return, Corridor, Assign avec FRandomStream(7), 40 essais).
- Pas de bibliotheque ni de test script Formation existants avant (grep : seul HOLFormationRules.h est lu par le C++).

## Etat exact des 15 tests C++
- PORTES (11) : Rows, Kinds, Classes, Places, Assign, Front, Speed, Return, YieldPlace, Corridor, BreakAtMelee.
- RESTANTS (4, non ouverts : lus ni portes) : AssignByDepth (l.343-431), Group (l.432-480), Block (l.625-780, le plus gros), Settle (l.781-860).
- TYPES_A_EXPOSER pour les 4 restants : AssignPlacesByKind, MakeGroupRows deja la, PlaceGroup avec IsFree, SettleBlockedPlaces (+ TArray<bool> IsBlocked/OutSettled), FindPackets, FindStragglers, ReserveRearSlots, GetBlockSoftPace, GetBlockBlendDistance, GetBlockBlend, GetLineLength, GetPointOnLine, GetDirectionOnLine, BlockOffsetToWorld, WorldToBlockOffset, GetSlotOnPath (a confirmer en lisant ces 4 tests).

## NON VERIFIE (a la compilation, par ordre de risque)
1. `TArray<double>` en parametre UFUNCTION (MakeCorridor/GetCorridorAt) : si UHT ou le binding refuse, passer a TArray<float> + conversion dans le .cpp.
2. Nom du namespace script `HOLFormationRead` (deduit de CommandGrid : classe sans U ni Library).
3. FRandomStream(int) / RandRange / FRandRange, TArray.Contains, Math::Tan, `int(float)` en script. Le C++ evalue `FVector2D(Random.FRandRange(), Random.FRandRange())` dans l'ordre du compilateur (MSVC : droite a gauche), le script de gauche a droite : les vecteurs aleatoires de Assign peuvent differer, les proprietes assertees sont les memes ; si un essai est rouge, regarder d'abord cette difference.
4. `UPARAM(ref)` sur TArray<int32>& (PlaceKinds) et FHOLPlaceHoldView& (copie dedans/dehors) : en script la reference doit rester modifiee apres l'appel.
5. Pieges du brief respectes : pas de TArray imbriquee (retiree), AssertTrue/AssertEquals sans if, FString + f"" seulement, methodes de classe script non passees en argument de Assert.

## Commande pour la session principale
Build groupe : `Tools\build.ps1 -LiveCoding=0 -Wait` dans C:\Users\baudo\Documents\HeroesOfLiberty-Galadriel, puis `quick_check -Parts Script` ; rouge vu = fausser une valeur attendue de Test_FMRows (ex. "5/5" -> "5/6"), le remettre ; puis seulement retirer les 11 tests C++ portes de HOLFormationTests.cpp.

PROCESSUS_DETACHES: aucun (aucun lancement)
ANOMALIES_PROCESSUS: aucune
GIT_STATUS_PRINCIPALE: docs seulement (EtapeDefini.md, Auto_Journey_File.md, ce rapport) : oui
