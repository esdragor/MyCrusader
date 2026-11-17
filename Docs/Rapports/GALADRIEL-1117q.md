# GALADRIEL-1117q : MIG-C-Formation, suite (NON COMPILE : C++ de liaison + tests script)

NON COMPILE : ni la bibliotheque C++ etendue ni les tests script n'ont ete compiles ni lances (aucun build, aucun quick_check, aucun editeur). Branche galadriel-tab, 3 commits, aucun push, merge main non fait (main 77f22a7 deja fusionne au depart).

APPELS_DECLARES: ~23 (arret a 25 ; heure d'arret 19:05)
HEURE_DEBUT/HEURE_FIN: 18:48:40 / 18:53:28
PREMIER_BUILD: sans build   BUILD: NON COMPILE   LAUNCHCHECK_DEBUT: non lance (pas de build)
TEST_ROUGE: aucun (rien lance) ; assertit l'effet : oui (valeurs calculees par HOLFormation)
COMMITS: 3 (ea008f1, cd34adb, e3d3b73 ; dernier 18:53), merge_main_fait: non
TESTS: aucun lance ; TESTS_TAUTOLOGIQUES: 0
ESSAIS: aucun
QUESTIONS: aucune
BUILDS_DEMANDES: 1 (lot C++ de la session principale)
TESTS_CPP_RETIRES: 9 (Rows, Kinds, Classes, Places, Front, Speed, Return, YieldPlace, BreakAtMelee) + helper FormationText
TESTS_SCRIPT_AJOUTES: 4 (Test_FMAssignByDepth, Test_FMGroup, Test_FMBlock, Test_FMSettle) ; au total 15 Test_FM* pour 15 tests C++
LIGNES_CPP_SUPPRIMEES: 382 (HOLFormationTests.cpp 961 -> 579) ; ajoutees : ~95 lignes de liaison (HOLFormationReadLibrary.h/.cpp)
LIGNES_MIGREES_PAR_LOT: ~ 382 retirees + ~ 380 lignes de test C++ (AssignByDepth, Group, Block, Settle) portees
TYPES_A_EXPOSER: aucun (tous les types des 4 tests sont de base : TArray<FVector2D/int/bool>, FVector2D, double ; les lambdas ProjectPlace / IsFree sont remplaces par des variantes fixes)

## Fait
- ea008f1 : HOLFormationTests.cpp sans les 9 tests deterministes portes (Rows, Kinds, Classes, Places, Front, Speed, Return, YieldPlace, BreakAtMelee) ni le helper FormationText devenu inutile. GARDES en C++ : Assign et Corridor (aleatoire / flux different), AssignByDepth, Group, Block, Settle (a retirer apres vert). Helpers gardes : WidthsText (Group), Cross (Assign, AssignByDepth, Block). Demande de la session principale : retrait AVANT le rouge vu / vert du script (non compile).
- cd34adb : UHOLFormationReadLibrary etendue (noms de namespace anonyme inchanges : HOLFormationReadLibraryPrivate, aucun nouveau) : PlaceGroupClamped (ProjectPlace qui compte et borne X ; MaxPlaceX enorme = identite), PlaceGroupAvoidingDisc (surcharge IsFree : libre = a Clearance du disque, ou rien si bWalledIn ; repli = zero), SettleBlockedPlacesWalled, AssignPlacesByKind, GetLineLength, GetPointOnLine, GetDirectionOnLine, BlockOffsetToWorld, WorldToBlockOffset, GetSlotOnPath, GetBlockSoftPace, GetBlockBlendDistance, GetBlockBlend, FindPackets, FindStragglers, ReserveRearSlots ; GetConstant : + BlockTurnWindow, BlockLagDistance, BlockSoftHoldDistance, BlockBlendMargin, BlockJoinDistance, SettleMinGapInSpacings.
- e3d3b73 : Script\Tests\HOLFormation_Test.as : Test_FMAssignByDepth, Test_FMGroup, Test_FMBlock, Test_FMSettle aux memes vecteurs que le C++ (helpers FMDist, FMRowOfPlace, FMRunByKind, FMLocalSlots ; FMCross/FMNear/FMNearly/FMWidthsText deja la). L'en-tete du fichier dit encore « 11 sur 15, 4 non portes » : a corriger apres vert.

## NON VERIFIE (a la compilation, par ordre de risque)
1. `TArray<bool>` en parametre UFUNCTION (SettleBlockedPlacesWalled, FindStragglers, ReserveRearSlots) : si UHT ou la liaison refuse, passer a TArray<int32> 0/1 avec conversion dans le .cpp. Et `TArray<double>` deja signale en 1117p.
2. Fonctions TArray du script : SetNum, RemoveAt, Contains, Empty ; `FVector2D.GetSafeNormal()`, champs `.X` `.Y` ; ternaire `I < 3 ? 0 : 1` ; `Unit / 3` entier ; `1.0e12` / `1.0e-9` litteraux.
3. `T.AssertEquals(Places.Num(), Projected, ...)` : variables locales, pas de methode de classe en argument (piege du brief respecte).
4. Les 4 conversions `TArray<FVector2D>` -> `TConstArrayView` cote C++ (Positions passees a FindPackets etc.) : implicites, comme les autres liaisons.
5. Le test C++ Settle utilisait IsFree puis ProjectPlace separes ; ici PlaceGroupAvoidingDisc les fusionne (meme semantique : libre = hors du disque).

## Commande pour la session principale
Build groupe : `Tools\build.ps1 -LiveCoding=0 -Wait` dans C:\Users\baudo\Documents\HeroesOfLiberty-Galadriel, puis `quick_check -Parts Script`. Rouge vu : fausser une valeur attendue de Test_FMRows ("5/5" -> "5/6") puis la remettre. Si vert : retirer du C++ AssignByDepth, Group, Block, Settle (et Assign, Corridor si accord) de HOLFormationTests.cpp, puis les helpers WidthsText / Cross ; le total quick_check ne doit pas baisser (les 15 Test_FM* script remplacent les 15 tests C++).

PROCESSUS_DETACHES: aucun processus detache pour C:\Users\baudo\Documents\HeroesOfLiberty-Galadriel
ANOMALIES_PROCESSUS: aucune ; un heredoc Bash avec apostrophes a echoue (rien execute), refait par Write + cat.
GIT_STATUS_PRINCIPALE: Auto_Journey_File.md et EtapeDefini.md modifies (voulu) ; rapports CELEBORN-1117p, HALDIR-1117p et Priorite_Socle_Factions non touches par moi : oui (docs seulement)
