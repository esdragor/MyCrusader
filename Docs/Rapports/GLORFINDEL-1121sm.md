# GLORFINDEL-1121sm (2026-11-21, 20:54 -> 20:57)

NON COMPILE (aucun build, aucun quick_check). Worktree HeroesOfLiberty-Glorfindel, branche ability-columns.
EN TETE : commit separe "fix: HOLFollowBridge UHT signatures" = e39bcf4. Cause : UHT genere les parametres TArray d'un BlueprintImplementableEvent en `const TArray<>&` ; mon .h les declarait par valeur (BP_RightClickOnUnit, BP_ManualOrder) => C2511 puis C2352. Corrige : `const TArray<int32>&` (BP_Step n'a pas de TArray). HOLWallLineBridge.h n'a pas de TArray en parametre, d'ou son silence. L'override AS (`TArray<int>`) ne change pas.

APPELS_DECLARES: ~24 (arret 21:10 non atteint)
HEURE_DEBUT/HEURE_FIN: 20:54 / 20:57 (Get-Date)
PREMIER_BUILD: sans build   BUILD: NON COMPILE   LAUNCHCHECK_DEBUT: non lance
TEST_ROUGE: 7aa7efa (assertit l'effet : oui) ; rouge NON VU a l'execution
COMMITS: 3 (e39bcf4 fix UHT, 7aa7efa test, 9247652 branchements), dernier 20:56, merge_main_fait: non
TESTS: aucun lance ; TESTS_TAUTOLOGIQUES: 0
ESSAIS: aucun ; QUESTIONS: aucune
BUILDS_DEMANDES: 1 (HOLFollowBridge.h + HOLPlayerController.cpp, lot de la session)
TESTS_SCRIPT_AJOUTES: 3 (Script/Orders/HOLFollowBridgeRequests_Test.as : Test_FollowEndsOnRequestMove, Test_FollowEndsOnRequestAttackMove, Test_FollowEndsOnRequestMoveInLine)
TYPES_A_EXPOSER: aucun ; DIAGNOSTICS_AS: non releves

## Fait
- Point commun : les chemins convergent dans les Request* du controleur (raccourcis clavier, clic mini-carte HOLMinimapView.cpp:664, attaque-deplacement, glisse en ligne, soin, attaque d'unite, Stop/Hold). NativeManualOrder(UnitIds) ajoute en tete du bloc `if (UnitIds.Num() > 0)` de RequestMoveUnits, RequestAttackUnit, RequestHealUnit, RequestAttackMove, RequestStopUnits, RequestMoveUnitsInLine (HOLPlayerController.cpp ~2891-2990). L'appel de la branche Stop/Hold d'OrderGrid (doublon) est retire ; celui de OrderAt (avant la branche Suivre) reste : la branche Suivre recommence son suivi apres.
- Le Step du suivi n'utilise pas les Request* (UHOLCommandSubsystem direct) : pas de boucle ou le suivi se termine lui-meme.
- Patrouille : AUCUN chemin dans HOLPlayerController.cpp (grep Patrol/Patrouille vide), rien a brancher ; a faire avec la case F (OrdersPatrolInProgress.md).

## Rouges attendus nom par nom (sur 7aa7efa, apres build de e39bcf4)
Test_FollowEndsOnRequestMove, Test_FollowEndsOnRequestAttackMove, Test_FollowEndsOnRequestMoveInLine : ROUGES (assert "unit 1 no longer follows after ..."). Verts attendus sur 9247652. Test_FollowBridgeManualOrderEndsFollowOfThoseUnits : vert apres le build. Commande : Tools\build.ps1 -LiveCoding=0 dans le worktree Glorfindel, puis quick_check -Parts tests.

## NON VERIFIE
- Tout. Les tests supposent qu'un AHOLPlayerController se cree par SpawnActor(AHOLPlayerController::StaticClass(), ...) depuis un test script (pas de precedent dans Script\*_Test.as ; si refuse : meme test en C++ comme HOLSignalGroundMarkTests.cpp, avec NativeIsFollowing a exposer) et que RequestMoveUnits sans PlayerState passe jusqu'a l'appel du pont (l'appel est avant la route RPC).
- RequestAttackUnitIds / RequestHealUnit non testes ; les clics qui rendent la main avant le bloc Own d'OrderAt (chantier, ferme, source) ne terminent toujours pas le suivi (non traite).
- Diagnostics Angelscript non releves.

ANOMALIES_PROCESSUS: aucune
PROCESSUS_DETACHES: stray_processes lance (sortie vide = aucun)
GIT_STATUS_PRINCIPALE: non releve (aucun fichier touche dans la copie principale hors ce rapport)
