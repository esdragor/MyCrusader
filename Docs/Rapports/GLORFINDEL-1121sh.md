# GLORFINDEL-1121sh (2026-11-21, 20:19 -> ~20:23)

NON COMPILE : script seul, aucun build, aucun quick_check, aucun test lance.

APPELS_DECLARES: ~21 vus (limite 25 ; premier Edit au ~11e appel, avant-8e NON applique : lectures de decouverte)
HEURE_DEBUT/HEURE_FIN: 20:19 / ~20:23
PREMIER_BUILD: sans C++   BUILD: NON COMPILE   LAUNCHCHECK_DEBUT: sans C++
TEST_ROUGE: 5841f40 (6 tests + stubs qui rendent Move / ne font rien), assertit l'effet : oui pour ApplyRightClick (le Book reel : followers, cibles, etat bActive) ; la classification est une regle pure (valeur calculee par le code teste). Rouge NON VU a l'execution.
COMMITS: 2 (5841f40 test rouge + stubs, 729165e regle), dernier ~20:22, merge_main_fait: non (DLL non verifiee ; la session principale synchronise)
TESTS: aucun lance ; TESTS_TAUTOLOGIQUES: 0
TESTS_SCRIPT_AJOUTES: 6 (Script\Orders\HOLFollowOrders_Test.as) : Test_RightClickOnAllyUnitIsFollow, Test_RightClickOnEnemyUnitIsAttack, Test_RightClickOnGroundIsMove, Test_RightClickOnSelectedAllyIsNotFollow, Test_ApplyRightClickOnAllyStartsFollowForTheOthers, Test_ApplyRightClickOnEnemyStartsNoFollow
BUILDS_DEMANDES: 0 ; apres le lot C++ : `quick_check -Parts tests` dans le worktree Glorfindel
TYPES_A_EXPOSER: aucun nouveau
DIAGNOSTICS_AS: non releves

## Verification avant ecriture
Grep RightClick / ClassifyRightClick / OrderGround / ClassifyTarget dans Script\ (worktree) et main : seul le clic droit SOL existe en script (HOLControllerCommandsC10a.as : commandes console hol.Select.OrderGround, RightClickGroundText). Aucun classement allie / ennemi / sol, rien de semblable dans main. Le clic droit reel sur une unite est dans HOLPlayerController (C++).

## Fait (Script\Orders\HOLFollowOrders.as, D818 Q1 C : clic droit sur un allie = Suivre, sur un ennemi = attaque = focus)
- enum EHOLRightClickOrder { Move, Attack, Follow }.
- HOLFollowOrders::ClassifyRightClick(bOnUnit, TargetId, bTargetIsAlly, SelectedIds) : sol = Move ; ennemi = Attack ; allie dans la selection = Move (pas de suivi de soi) ; sinon Follow.
- HOLFollowOrders::ApplyRightClick(Book, SelectedIds, bOnUnit, TargetId, bTargetIsAlly) : classe, et pour Follow fait Begin(..., true) pour chaque unite selectionnee sauf la cible ; Move / Attack laissent le Book intact (l'appelant envoie l'ordre ordinaire).
- bTargetIsAlly est decide par l'appelant (meme camp ou allie par Diplomatie).

## Rouges attendus nom par nom
- Sur 5841f40 (stubs : tout Move, Book inchange) ROUGES : Test_RightClickOnAllyUnitIsFollow, Test_ApplyRightClickOnAllyStartsFollowForTheOthers, Test_RightClickOnEnemyUnitIsAttack, Test_ApplyRightClickOnEnemyStartsNoFollow. VERTS par construction (le stub rend Move) : Test_RightClickOnGroundIsMove, Test_RightClickOnSelectedAllyIsNotFollow (gardes de regression).
- Sur 729165e (tip) : les 6 verts attendus ; les 13 autres tests de HOLFollowOrders_Test.as inchanges. Suspects si le script ne compile pas : `for (int UnitId : SelectedIds)` (syntaxe de boucle sur TArray, a confirmer), `TArray::Contains` sur `const TArray<int>&`.

## RESTE
1. C++ (HOLPlayerController, lot groupe) : au clic droit sur une unite, appeler HOLFollowOrders::ApplyRightClick ; Move / Attack = ordre ordinaire.
2. C++ : appelant de StepUnit a chaque pas + source de bPlayerOrderActive (ligne ajoutee a Auto_Journey_File.md l.135).
3. Placeholders.txt : aucun provisoire ajoute ou retire.

## NON VERIFIE
Compilation script, execution des tests, diagnostics AS, DLL du worktree vs main. Fins de ligne du test : ajoute par heredoc (LF), le depot le normalise en CRLF au prochain contact git (avertissement vu), non verifie par eol_check.
PROCESSUS_DETACHES: aucun (aucun processus lance)
ANOMALIES_PROCESSUS: premier Edit apres le 8e appel (lectures de decouverte) ; un appel PowerShell refuse par le classifieur (relance seul).
GIT_STATUS_PRINCIPALE: Auto_Journey_File.md (ligne l.135) et OrdersPatrolInProgress.md modifies par moi (docs voulus), le reste appartient a d'autres sessions ; aucun fichier de code touche dans la copie principale : oui.
