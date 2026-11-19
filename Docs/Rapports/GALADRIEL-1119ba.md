# GALADRIEL-1119ba : lot G0 (grille de commandes, tests repointes sur le script) + devis du retrait de HOLCommandGrid.cpp

NON COMPILE (aucun build, aucun quick_check, par consigne). Worktree `C:\Users\baudo\Documents\HeroesOfLiberty-Galadriel`, branche `grid-g0`, partie de integ-17 848ce80.

APPELS_DECLARES: 14 (arret a 25 vus ; heure d'arret 15:41)
HEURE_DEBUT/HEURE_FIN: 15:22 / 15:27 (Get-Date)
PREMIER_BUILD: sans C++   BUILD: NON COMPILE   LAUNCHCHECK_DEBUT: sans C++
BUILDS_DEMANDES: 0 (la session principale compile les scripts : le module `Script\Tests\HOLCommandGrid_Test.as` n'a jamais compile)
TEST_ROUGE: aucun vu (rouge a jouer par la session principale, voir FALSIFICATION) ; assertit l'effet : oui (les 15 tests assertent des commandes / cellules / resultats calcules par la regle)
COMMITS: 1, dernier a 15:25 (1ca94df), merge_main_fait: non (DLL perimee, branche issue de integ-17 jamais compilee)
TESTS: aucun lance ; TESTS_TAUTOLOGIQUES: 0
ESSAIS: aucun
QUESTIONS: aucune
PROCESSUS_DETACHES: rien lance
TESTS_CPP_RETIRES: 0 ; TESTS_SCRIPT_AJOUTES: 0 (15 repointes) ; LIGNES_CPP_SUPPRIMEES: 0 ; TYPES_A_EXPOSER: voir devis ci-dessous

## (1) FAIT : 1ca94df

Etat avant : `HOLCommandGridRules_Test.as` (G1, 00a2800) existe deja avec les tests d'accord C++ / script (`Test_CGRAgreement*`, `Test_CGRHardValues`), et `HOLCommandGridKeys_Test.as` / `HOLCommandGridGroups_Test.as` idem. Mais les 15 `Test_CG*` de `Script\Tests\HOLCommandGrid_Test.as` (MarketGain, Market, Commands, Feedback, Cycle, Categories, Group, Subgroups, Distribution, Tables, UnitOrders, HeroAbilities, Ages, MouseCursor, Research) appelaient encore `HOLCommandGridRead::` (natif). `git log` : pas deja fait.

Fait dans `HOLCommandGrid_Test.as` (une seule modification, 248 + / 249 -) : tous les appels qui ont un jumeau script sont repointes :
- `HOLCommandGridRules::` : GetNumCells, GetGridColumns, GetCancelCell, GetResearchFirstCell, GetNumAges, GetAttackMoveCell, GetStopCell, GetHoldCell, GetHeroAbilityCell, GetCategoryRow, GetTradeCell, GetTradeGain, GetTradeGainText, GetCommand, GetOrder, IsBuildGrid, IsAgeReached, GetAgePanelOpacity, GetAgeBuilding.
- `HOLCommandGridKeys::` : PressKey, ClickAge, GetChosenAgeAfter, HasMissingResource, IsPopulationBlocked, CheckProduction, CheckBuild, Blocks, Describe, GetCellFace ; `GetMissingResource(..) == EHOLResource::X` devient `FindMissingResource(..) == int(EHOLResource::X)` (3 lignes, meme forme que `HOLCommandGridKeys_Test.as`).
- `HOLCommandGridGroups::` : IsRepeatPress, NextInCycle, IsAloneInGroup, SplitSubgroups, GetCommandPriority, MergeCommands, PickEvenly, IsCategorySelectedByCategory.
- `Test_CGMouseCursor` reecrit sur `HOLCommandGridRules::IsAttackCursor(bArmed, bPlacing)` (booleen : vrai = croix rouge) ; l'assertion `Crosshairs != Arrow` disparait (sans objet).
- RESTENT sur `HOLCommandGridRead::` (pas de jumeau script : lecture de TABLE C++, voir devis) : CheckResearch (12), GetCycleRankOfType (3), IsTypeSelectedByCategory (3), IsTypeInTable (4), GetTypeCategory, WithTableBuildables (2), GetProducesOf (5), GetBuildingResearches (5), IsTechnologyInTable (3), IsTechnologyImplemented, GetTechnologyFoodCost, GetBlock* (9 : numeros des blocs de recherche), GetHeroAbilityAura / CloseRanks / Banner (3 : numeros d'enum), GetPlacementRefusalBuilding.
- En-tete du fichier mis a jour (G0). Les tests d'accord C++ / script restent tels quels.

Risque de compilation (non compile) : signatures verifiees contre `HOLCommandGridReadLibrary.h` pour les 18 fonctions a signature douteuse (memes types de vues ; seules differences : GetMissingResource -> FindMissingResource renvoie un int, GetMouseCursor -> IsAttackCursor). Si un test tombe pour une valeur (pas une compilation), c'est un ecart reel entre la regle script et la table C++ : le test d'accord `Test_CGRAgreement*` dit lequel.

## FALSIFICATION a jouer (rouge a voir)

Fichier `Script\Player\HOLCommandGridRules.as`, ligne 14 : `const int GridColumns = 4;` -> `5`. Effet : `GetTradeCell` (vente = GridColumns + Good) et le calcul `Cell % GridColumns` du Marche.
Doivent tomber : `Test_CGMarket` (certain : `T.AssertEquals(4 + I, GetTradeCell(Good, false))` ligne ~134, et les cellules du marche), `Test_CGRHardValues` (Stone vente = 6 en dur, ligne 363 de `HOLCommandGridRules_Test.as`), `Test_CGRAgreementSelections` / `Test_CGRAgreementTables` / la comparaison des constantes (accord avec le natif). Probables : `Test_CGTables`, `Test_CGUnitOrders` (lisent GetGridColumns). Compte exact non connu (non compile) : la session principale note N rouges vus puis remet 4 et note N = 0. Au moins 4 attendus, dont 1 des 15 `Test_CG*` (CGMarket).
Alternative a un seul test : `CancelCell = 11` -> `10` ne fait tomber que ce qui compare a 11 en dur (CGCommands ligne 200, CGRHardValues ligne 372) car les autres lisent `GetCancelCell()` (coherent des deux cotes) : moins bon rouge.

## (2) DEVIS : appelants restants du natif `HOLCommandGrid::` (Source\HeroesOfLiberty\Player\HOLCommandGrid.cpp, 619+ lignes)

Etat du pont (HOLCommandGridBoard.h:29-47, HOLCommandGrid.cpp:60-616) : 15 fonctions du natif demandent d'abord au script (`UHOLCommandGridBoard::Ask*`) et gardent leur corps `*Native` si aucun plateau script n'existe : GetCommand, GetOrder, IsAgeReached, IsAgeOpen, GetAgeBuilding, PressKey, ClickAge, GetChosenAgeAfter, CheckProduction, CheckBuild, Describe, NextInCycle, PickEvenly, SplitSubgroups, MergeCommands. Les corps `*Native` + les jumeaux `*Native` de la bibliotheque ne disparaissent qu'avec le dernier appelant C++ (etape C6/C7 du plan, controleur).

A. Appelants qui LISENT DEJA le script (via Ask*, fonction pontee) : rien a faire pour eux, seulement a conserver tant que le corps natif existe.
- HOLPlayerController.cpp : 1273, 3711, 5364, 5470, 5475 (GetChosenAgeAfter) ; 2143, 2253 (SplitSubgroups) ; 2498 (NextInCycle) ; 2810, 2823 (GetCommand) ; 2813 (MergeCommands) ; 2923 (GetOrder) ; 2952 (PressKey) ; 2965 (IsAgeReached), 2966 (ClickAge) ; 3035 (CheckProduction) ; 3147, 3624, 3661 (CheckBuild) ; 3255 (PickEvenly) ; 3285 (Describe).
- Tests C++ qui appellent des fonctions pontees (lisent le script dans un monde de jeu avec plateau, le corps natif sinon : a verifier, ils peuvent donc prouver l'une ou l'autre copie selon le monde) : HOLRegicideTests.cpp:292-299 (GetOrder, IsBuildGrid non ponte), HOLVictoryTests.cpp:1201-1204 (GetCommand), HOLDamnedWarriorTests.cpp:233,235 (GetCommand + GetRageCell), HOLStartingHeroTests.cpp:655-661, 726 (PressKey, ClickAge, GetAgeBuilding). Avant le retrait : les passer sur la bibliotheque de lecture, ou les convertir en script (D838 : seuls les tests Mass restent en C++).

B. Appelants C++ DIRECTS, fonction NON pontee mais ayant DEJA un jumeau script (il manque un Ask* ou le passage du caller en script) :
- HOLPlayerController.cpp:1438 GetMouseCursor -> `Rules::IsAttackCursor` (a ponter ; ou le curseur passe en script avec le controleur).
- :2128, :2299 IsAloneInGroup ; HOLSelectionChipsReadLibrary.cpp:153 IsAloneInGroup -> `Groups::IsAloneInGroup` (ponter : 1 Ask).
- :2477 IsRepeatPress -> `Groups::IsRepeatPress` (ponter).
- :2813 GetCommandPriority (argument de MergeCommands) -> `Groups::GetCommandPriority` (ponter).
- :2442 IsSelectedByCategory -> `Groups::IsCategorySelectedByCategory(Type.Category, Category)` (ponter ; entree = 1 champ de ligne : Category).
- :2449, :2450, :2455 GetCycleRank -> `Groups::GetCycleRank(bBuildable, BuildGridSlot)` (voir C ci-dessous).
- :3042, :3148 Blocks -> `Keys::Blocks` (ponter, ou la logique `Feedback.Kind` reste lue en C++ : 4 valeurs d'enum).
- :590 GetHeroAbilityCell(Banner) ; HOLControllerCommandLibrary.cpp:412 HoldCell / StopCell : constantes (`Rules::GetHeroAbilityCell`, GetHoldCell, GetStopCell existent) ; rester C++ (constexpr) tant que le controleur est C++, la valeur est dans 2 copies (test d'accord `Test_CGRAgreementConstants` couvre).
- UI : HOLGameScreen.cpp:1069 GetReportName -> `Rules::GetReportName` ; :1213 GetTradeGain -> `Rules::GetTradeGain` ; :1460, :1541 IsPopulationBlocked -> `Keys::IsPopulationBlocked` ; HOLGridCell.cpp:193 GetCellFace -> `Keys::GetCellFace`, :228 GetTradeGainText -> `Rules::GetTradeGainText`. Raison de les laisser C++ aujourd'hui : widgets Slate/UMG de base (CLAUDE.md : C++ pour Slate/widget de base) ; pour lire le script il faut soit que le panneau script (HOLCommandGridPanel.as) porte la valeur, soit un Ask* par fonction.
- Constantes de taille (NumCells, NumAges, GridColumns, CancelCell...) : HOLGameScreen.cpp:247, 261, 329-331, 1663, 1681, 1764, 1791 ; HOLCommandGridBoard.cpp:251, 254 ; HOLPlayerController.h:250 (`NumGridCells = HOLCommandGrid::NumCells`). A reprendre dans un petit header de constantes C++ (une ligne par constante, test d'accord avec `Rules::Get*` deja la) : c'est la derniere raison de garder `HOLCommandGrid.h` ; NumCells alimente un `constexpr` et des tableaux de taille fixe, donc reste C++ (taille de tableau a la compilation).
- HOLCommandGridReadLibrary.cpp:145-434 : ~60 transferts vers le natif (la bibliotheque de lecture elle-meme) : disparait par morceaux, ne garde que les lectures de TABLE (voir C).

C. Appelants qui RESTENT C++ (raison) :
- HOLPlayerController.cpp:2767 `SetSpecialization` et HOLSpecializationGridTests.cpp:31-115 (SetSpecialization, GetSpecializationCell, GetCommand, GetReportName) : la vue `FHOLSpecializationView` n'est pas exposee au script (non portee en G1) ; a exposer : `FHOLSpecializationView` (tier ouvert, option 0/1, restant en ms) ; portage de `GetOpenSpecializationTier` (HOLCommandGrid.cpp:23-37) + `GetSpecializationCell` (:18). Tant que non faite, ces 2 lignes + 1 test gardent un petit HOLCommandGrid.cpp.
- HOLPlayerController.cpp:3280 `CheckResearch(*Row, Facts, Stock, Queue, MaxQueue)` (aussi HOLCommandGridReadLibrary.cpp:434) : a exposer au script = la ligne de TABLE `FHOLTechnologyRow` : Cost (4 ressources, pour `Stock.Covers`), Implemented, MinTier, MinHeroLevel (lus par `HOLTechnology::Check`, HOLTechnologyRules.cpp) et le reste de `Check` que je n'ai pas lu au-dela de MinHeroLevel (NON VERIFIE : autres champs eventuels) ; plus `FHOLTechFactsView` (deja une USTRUCT : HeroTier, bRequiresDone, bBuildingFinished, bResearched, bQueued ; HeroLevel absent de la vue). Corps de CheckResearch (HOLCommandGrid.cpp:384-405) : `bCanPay = Stock.Covers(Cost)` ; bloc CannotPay -> NotEnoughResources + FindMissingResource ; autre bloc -> ResearchRefused + Block ; sinon file pleine -> QueueFull. Pas de jumeau script : un portage de `HOLTechnology::Check` est un lot a part (la regle est utilisee par le serveur : C++ pour la simulation).
- GetCycleRank (HOLCommandGrid.cpp:474-477, appelants HOLPlayerController.cpp:2449, 2450, 2455) : le jumeau script existe (`Groups::GetCycleRank(bBuildable, BuildGridSlot)`) ; lignes de table a exposer : `FHOLBuildingTypeRow::BuildGridSlot` (IsBuildable() = `BuildGridSlot != INDEX_NONE`, HOLBuildingTypes.h:141) et `Category` (pour IsSelectedByCategory). Le controleur C++ les lit deja sur ses lignes : le cout est un `Ask*` (ou un passage au script de la fonction qui trie, HOLPlayerController.cpp:2440-2500) ; la ligne de table n'a pas a etre exposee tant que l'appelant est en C++.
- HOLCommandGridBoard.cpp:251,254 (NumCells) : constante, voir B.
- HOLAttackCursor.h:13, HOLUnitView.h:44, HOLCommandGrid.h:41,111, HOLPlayerController.h (179-309) : commentaires seulement, a mettre a jour au retrait.

D. Ordre pour retirer `HOLCommandGrid.cpp` (devis, rien retire) :
1. Lot C++ unique : Ask* pour IsAloneInGroup, IsRepeatPress, GetCommandPriority, IsCategorySelectedByCategory, GetMouseCursor/IsAttackCursor, Blocks, GetCycleRank (7 fonctions, meme patron que les 15 existants) ; header de constantes ; exposer FHOLSpecializationView + SetSpecialization script OU garder ces 2 fonctions.
2. G3 (switch de RefreshGrid), puis C6/C7 (controleur) : dernier appelant du natif ; les `*Native` et les tests C++ du groupe A disparaissent ensemble.
3. Restent apres : specialisation (si non portee), CheckResearch, constantes de taille = environ 80 lignes sur 619 : HOLCommandGrid.cpp devient `HOLCommandGridTables.cpp`.

NON VERIFIE : compilation du module de tests ; les 18 signatures (relues, pas compilees) ; la liste « lisent deja le script » suppose qu'un plateau script existe dans le monde de chaque appelant (les Ask* retournent false sans plateau) ; champs de `HOLTechnology::Check` au-dela de MinHeroLevel ; comportement des tests C++ du groupe A selon le monde.
ANOMALIES : aucune. Fins de ligne conservees (CRLF) ; `git diff` : 1 fichier.
