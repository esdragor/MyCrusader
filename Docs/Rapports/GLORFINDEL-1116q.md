# GLORFINDEL-1116q : pont serveur Garrison vers le script (net-playtest, commit 75fe739, NON COMPILE)

Worktree `C:\Users\baudo\Documents\HeroesOfLiberty-Glorfindel`, branche net-playtest, commit « Garrison: server bridge to the script rule ». Branche prete. Merge main non fait (non demande).

## Fait
- `Source\HeroesOfLiberty\Simulation\HOLGarrisonBoard.h/.cpp` (UHOLGarrisonBoard, UWorldSubsystem abstrait, modele UHOLChatSubsystem) : `GarrisonAdmitted` BlueprintNativeEvent, implementation par defaut = regle C++ existante (`HOLGarrison::AdmittedBySettings`, kind hors 0..4 = Siege) ; `AdmittedNow` (appelant public) ; `Find` ; `AdmittedForOrder(World, ...)` = reponse du pont si le board script existe, sinon C++ direct (comportement identique sans sous-classe script).
- `Script\Garrison\HOLGarrisonBoardScript.as` : `UHOLGarrisonBoardScript : UHOLGarrisonBoard`, override `GarrisonAdmitted` -> `HOLGarrison::Admitted`.
- `HOLGarrisonSubsystem.cpp` : include + 1 ligne (l. ~145) `AdmittedBySettings` -> `UHOLGarrisonBoard::AdmittedForOrder(World, ...)`. KindOf reste C++ (categories d armure non exposees). Rien du C++ existant retire.
- Test `Source\HeroesOfLiberty\Tests\HOLGarrisonBridgeTests.cpp` : `HeroesOfLiberty.Garrison.ServerBoard` (board non nul ; 2 places -> {0,3} ; 1 place libre -> {0} ; batiment -1 ; kind 9 ; accord avec UHOLGarrisonRuleLibrary::AdmittedBySettings ; chemin AdmittedForOrder).
- HOLPlayerController.* non touche. Les 8 tests Garrison.* non touches. Aucun placeholder ajoute.
- Suivi : ligne « bascule Garrison » de Auto_Journey_File.md, AngelscriptInProgress.md, EtapeDefini.md, IN_USE.txt (`libre`, CPP: oui).

## Champs
BUILDS_DEMANDES: 0 (pas de build, consigne) ; TESTS_CPP_RETIRES: 0 ; TESTS_SCRIPT_AJOUTES: 0 ; TESTS_CPP_AJOUTES: 1 (Garrison.ServerBoard) ; LIGNES_CPP_SUPPRIMEES: 0 ; LIGNES_MIGREES_PAR_LOT: 0 (pont seul) ; TYPES_A_EXPOSER: aucun nouveau (HOLGarrison:: deja visible du script) ; PROCESSUS_DETACHES: aucun lance.

## NON VERIFIE (a faire par la session principale)
1. Rien n est compile : UHT/compilation C++ (signatures TArray<int32> du BlueprintNativeEvent, `GetSubsystemArrayCopy`, test) et compilation script (override `GarrisonAdmitted` avec `TArray<int>`).
2. ROUGE PAS VU : avant le build, remplacer dans `HOLGarrisonBoardScript.as` le corps par `return TArray<int>();` (bouchon qui compile), build, lancer `HeroesOfLiberty.Garrison.ServerBoard` (attendu ROUGE : l implementation par defaut ne s execute pas, le bouchon renvoie vide), puis remettre `return HOLGarrison::Admitted(...)` et relancer (VERT). Le bouchon n est PAS dans le commit (le commit a la vraie regle).
3. Note : le test ne distingue le script du C++ que par ce bouchon (meme regle des deux cotes).
4. Le Board abstrait n est cree que par la sous-classe script ; la creation par UWorldSubsystem en monde de test (comme Chat.ServerBoard) est supposee.
5. Les 8 tests Garrison.* et le net (Net.Garrisoned) sont a relancer (quick_check -Parts garrison + net) : le chemin de l ordre passe maintenant par le pont.

## Reste (apres ce rouge/vert et la parite)
Supprimer la copie C++ : `HOLGarrisonRules.h` (80 l.), `HOLGarrisonRuleLibrary.h/.cpp` (28 + 33), le repli et l implementation par defaut du pont, les commentaires HOLCommandSubsystem.h:38 / HOLPlayerOrders.h:86, et retirer les tests de parite (Test_GarrisonScriptAndCppAdmitTheSame, Test_GarrisonScriptAndCppDefaultsAgree) une fois le C++ supprime. KindOf reste C++ tant que les categories d armure ne sont pas exposees.
