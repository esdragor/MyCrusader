# GALADRIEL-1119ca : etape D.1, pont de 6 fonctions de plus (NON COMPILE)

Impact joueur : aucun a l'ecran tant que le plateau script repond comme le C++ (memes regles) ; une fois compile, les touches F, Tab, les puces de selection et les refus de commande lisent la regle script pour ces 6 fonctions.

## Commit
- `1bad9a4` sur `grid-g0` (worktree `C:\Users\baudo\Documents\HeroesOfLiberty-Galadriel`), sur 1ca94df. Pas de merge main, pas de push.

## Fonctions pontees (C++ : fichier, Ask* ; script : reponse du plateau)
Wrappers dans `Source\HeroesOfLiberty\Player\HOLCommandGrid.cpp` (corps natif renomme `*Native`, declare dans `HOLCommandGrid.h`, repli conserve) ; Ask* dans `HOLCommandGridBoard.h/.cpp` ; reponses dans `Script\Player\HOLCommandGridBoard.as` (classe `UHOLCommandGridBoardScript`), chacune deleguant a la regle existante.

| Fonction C++ | Ask* | Reponse script | Regle script appelee |
|---|---|---|---|
| IsAloneInGroup(FName) | AskIsAloneInGroup | IsAloneInGroupAnswer | HOLCommandGridGroups::IsAloneInGroup |
| IsRepeatPress(uint32 views, bool) | AskIsRepeatPress (uint32 -> int32 par static_cast) | IsRepeatPressAnswer | HOLCommandGridGroups::IsRepeatPress |
| GetCommandPriority(FHOLGroupMember) | AskGetCommandPriority (FHOLGroupMember -> FHOLGroupMemberView) | GetCommandPriorityAnswer | HOLCommandGridGroups::GetCommandPriority |
| IsSelectedByCategory(Row, Category) | AskIsSelectedByCategory(Type.Category, Category) | IsSelectedByCategoryAnswer | HOLCommandGridGroups::IsCategorySelectedByCategory |
| Blocks(FHOLCommandFeedback) | AskBlocks (FromRule du read library) | BlocksAnswer | HOLCommandGridKeys::Blocks |
| GetCycleRank(Row) | AskGetCycleRank(Type.IsBuildable(), Type.BuildGridSlot) | GetCycleRankAnswer | HOLCommandGridGroups::GetCycleRank |

Aussi : `HOLCommandGridReadLibrary.cpp` (7 lignes) lit maintenant les `*Native` (GetCycleRankOfType, IsTypeSelectedByCategory, IsCategorySelectedByCategory, Blocks, IsRepeatPress, IsAloneInGroup, GetCommandPriority), comme les 15 premieres : les tests d'accord comparent ainsi script et C++ vrai, pas script et script.

Appelants (inchanges, ils passent par le pont) : HOLPlayerController.cpp 2128, 2299, 2442, 2449, 2450, 2455, 2477, 2813, 3042, 3148 ; HOLSelectionChipsReadLibrary.cpp 153.

## Fonctions laissees
Aucune des 6 n'a ete laissee : toutes les signatures script passent avec des types deja exposes. GetCycleRank et IsSelectedByCategory : le C++ lit la ligne de table (FHOLBuildingTypeRow, non exposee) et passe les champs (bool, int, categorie). Non touchees comme demande : GetMouseCursor, GetAgePanelOpacity, Slate (HOLGameScreen, HOLGridCell), CheckResearch, specialisation. Aucun corps natif retire.

## Tests ajoutes (script, `Script\Tests\HOLCommandGridBoard_Test.as`, nouveau fichier)
Test_CGBIsAloneInGroup, Test_CGBIsRepeatPress, Test_CGBGetCommandPriority, Test_CGBIsSelectedByCategory, Test_CGBBlocks, Test_CGBGetCycleRank.
Acces au plateau : AUCUN acces de test existant (aucun test n'atteint le plateau ; pas de monde en test unitaire, donc `UHOLCommandGridBoard::FindAny` / les Ask* C++ ne sont pas atteignables). Les tests creent un plateau `NewObject(GetTransientPackage(), UHOLCommandGridBoardScript)` et appellent ses `*Answer` (celles que les Ask* C++ appellent). Valeurs attendues en clair (ex. centre seul : vrai ; maison : faux ; priorite [1,3,0,2] pour Mill, MainCentre, Barracks, MainCentre ; rang (true,3)=4, (true,0)=1, (false,3)=0 ; PopulationFull ne bloque pas), plus accord avec le C++ vrai (HOLCommandGridRead::*, sur les Native) sauf GetCycleRank (pas de jumeau de lecture a champs, valeurs en clair seulement).
Le franchissement reel Ask* C++ -> plateau n'est teste par aucun test (voir NON VERIFIE).

## FALSIFICATION pour le rouge a voir
1. `Script\Player\HOLCommandGridGroups.as`, `IsAloneInGroup` (ligne ~126) : `n"MainCentre"` -> `n"House"`. Rouge attendu : Test_CGBIsAloneInGroup, Test_CGGHardValues, Test_CGGSubgroupsSweep (accord script vs C++ Native). Aucun test C++ existant (Source\Tests) ne traverse ce pont pour ces 6 (grep : aucun appel de ces fonctions dans les tests C++) ; les tests d'accord script/C++ sont ce qui tombe.
2. `HOLCommandGridKeys.as` ligne ~178 `Blocks` : retirer `Feedback.Kind == EHOLCommandFeedbackKind::QueueFull ||` -> Test_CGBBlocks, Test_CGKYAgreementFeedback, Test_CGKYHardValues rouges.
3. `HOLCommandGridGroups.as` ligne ~34 `GetCycleRank` : `BuildGridSlot + 1` -> `BuildGridSlot` -> Test_CGBGetCycleRank, Test_CGGHardValues rouges.
Remettre la ligne ensuite (a faire dans le worktree).

## Commandes pour la session principale
1. `powershell -NoProfile -Command ". C:\Users\baudo\Documents\HeroesOfLiberty-Galadriel\Tools\Scenarios\LaunchCheck.ps1; Get-StaleBinaryProblem C:\Users\baudo\Documents\HeroesOfLiberty-Galadriel"`
2. `C:\Users\baudo\Documents\HeroesOfLiberty-Galadriel\Tools\build.ps1 -LiveCoding=0 -Wait` (UN build pour le lot G0 + D.1).
3. `quick_check -Parts tests` (script : Test_CGB*, Test_CGG*, Test_CGKY*, Test_CGR*, Test_CG*) puis la partie C++ qui couvre HOLCommandGrid.
4. Une falsification ci-dessus, rouge vu, remise, vert.

## Champs migration
- BUILDS_DEMANDES: 1 (le lot C++ de la seance, avec le lot G0 precedent non compile)
- TESTS_SCRIPT_AJOUTES: 6 (Test_CGBIsAloneInGroup, Test_CGBIsRepeatPress, Test_CGBGetCommandPriority, Test_CGBIsSelectedByCategory, Test_CGBBlocks, Test_CGBGetCycleRank)
- TESTS_CPP_RETIRES: 0
- LIGNES_CPP_SUPPRIMEES: 0 (le repli reste ; +~185 lignes ajoutees au total)
- TYPES_A_EXPOSER: FHOLBuildingTypeRow (les champs sont passes un a un par le C++ pour GetCycleRank et IsSelectedByCategory ; aucun nouveau type n'est requis pour ces 6)

## NON VERIFIE (rien n'est compile, ni UHT, ni le script)
- UHT : 6 nouveaux `UFUNCTION(BlueprintNativeEvent)` dans une classe `Abstract` protegee. Parametres : FName par valeur, `const TArray<int32>&`, `EHOLBuildingCategory` (suppose UENUM, il est utilise dans une UPROPERTY de FHOLFeedbackView), `const FHOLFeedbackView&`, `TArray<int32>` en retour. Risque : une `_Implementation` virtuelle doit avoir la signature exacte (ici sans `const` sur les valeurs dans le .h, `const` sur les valeurs dans le .cpp : meme forme que les 15 existantes).
- C++ : les wrappers appellent `*Native` declares dans le .h apres usage dans le .cpp (declares dans le .h, donc OK). `AskIsRepeatPress` avec `TConstArrayView<uint32>` : construit depuis `TConstArrayView<uint32>` deja vu dans la signature de l'ancien corps. Les tests C++ existants non relus : un test C++ appelant `HOLCommandGrid::Blocks` & co. hors monde reste sur le natif (FindAny null) ; un test PIE avec plateau script passerait par le script (grep : aucun appel direct).
- Script : `NewObject(GetTransientPackage(), UHOLCommandGridBoardScript)` jamais utilise ailleurs dans le depot (risque de syntaxe ou de refus pour une sous-classe de UWorldSubsystem hors collection) ; appeler un `*Answer` depuis un test suppose qu'une surcharge `BlueprintOverride` d'un evenement protege C++ est publique cote script (les surcharges existantes ne sont appelees que par le C++). Si le compilateur refuse, repli : appeler directement HOLCommandGridGroups::/Keys:: (deja teste ailleurs) et ne tester que l'accord. `"... " + I` (chaine + int) suppose valide. `NAME_None` suppose present dans le script.
- Test_CGBIsRepeatPress : cles negatives (-5) passent par int en script ; l'ordre uint32 est celui du C++ Native, accord compare.
- Le pont reel (FindAny avec un monde de jeu) n'est teste par aucun test automatise ; la verification se fait en jeu / PIE (touche F deux fois, Tab) apres le build.

## Anomalies
- Heure : debut 16:18, fin 16:22, environ 14 appels d'outils.
- Fichier de test cree en LF (git avertit du passage en CRLF) ; sans effet.
- Le journal `*InProgress.md`, EtapeDefini.md, Auto_Journey_File.md, IN_USE.txt non touches (regle de seance b).
