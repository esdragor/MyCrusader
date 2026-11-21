# GLORFINDEL-1121sg (2026-11-21, 20:17 -> 20:19)

NON COMPILE : script seul, aucun build, aucun quick_check, aucun test lance.

APPELS_DECLARES: ~17 vus (limite 25 ; premier Edit au 12e appel, avant-8e NON applique : 3 lectures de plus pour lire HOLFollowRules, le test et la file)
HEURE_DEBUT/HEURE_FIN: 20:17 / 20:19
PREMIER_BUILD: sans C++   BUILD: NON COMPILE   LAUNCHCHECK_DEBUT: sans C++
TEST_ROUGE: 93ec3d1 (3 tests + stubs StepBodyguard / StepUnit qui rendent false), assertit l'effet : oui (file d'ordres relue par ConsumeMoves : Attack sur l'ennemi puis Move de reprise ; rien en file avec un ordre du joueur). Rouge NON VU a l'execution (jamais lance).
COMMITS: 2 (93ec3d1 test rouge + stubs, ba2b62e StepBodyguard + StepUnit), dernier 20:18, merge_main_fait: non (DLL du worktree non verifiee ; la session principale synchronise)
TESTS: aucun lance ; TESTS_TAUTOLOGIQUES: 0
TESTS_SCRIPT_AJOUTES: 3 (Script\Orders\HOLFollowOrders_Test.as) : Test_FollowOrdersBodyguardWithoutPlayerOrderAttacksThenFollowsAgain, Test_FollowOrdersBodyguardWithPlayerOrderIsNotInterrupted, Test_FollowOrdersStepUnitUnknownUnitQueuesNothing
BUILDS_DEMANDES: 0 de plus (le lot C++ de 1121sd / sf couvre les liaisons ; ensuite `quick_check -Parts tests` dans le worktree Glorfindel)
TYPES_A_EXPOSER: aucun nouveau
DIAGNOSTICS_AS: non releves (serveur de langage non consulte)
ESSAIS: aucun   QUESTIONS: aucune

## Verification avant ecriture
Grep HOLFollowOrders / HOLFollowRules / FHOLFollowBook / Step dans Script\ (worktree et main) : HOLFollowOrders::Step n'est appele QUE par HOLFollowOrders_Test.as. Il n'existe donc AUCUN controleur de l'ordre Suivre qui le met a jour a chaque pas : rien de semblable dans main, et rien a brancher dedans. J'ai donc ecrit le point d'entree que ce controleur appellera.

## Fait (Script\Orders\HOLFollowOrders.as, decision D818 Q2 = B)
- `StepBodyguard(Commands, PlayerIndex, Book, UnitId, UnitLocation, TargetLocation, bPlayerOrderActive, EnemyId)` : regle de decision. Ordre du joueur en cours = return false, rien en file (pas d'interruption) ; sinon Step(..., EnemyId >= 0, EnemyId) : Attack sur l'ennemi (une fois), puis Move de reprise quand l'ennemi a disparu (Decide + remise a zero de LastAttackIds).
- `StepUnit(WorldContext, Commands, PlayerIndex, Book, UnitId, bPlayerOrderActive)` : lit l'unite et la cible par ReadUnit, l'ennemi par PickEnemyForUnit (seulement sans ordre du joueur), puis StepBodyguard ; false si l'unite ou sa cible est inconnue (l'appelant fait OnManualOrder).

## Rouges attendus nom par nom
- Sur 93ec3d1 (stubs) : ROUGE Test_FollowOrdersBodyguardWithoutPlayerOrderAttacksThenFollowsAgain (stub rend false, rien en file : AssertTrue(bAttackIssued) et les compteurs d'ordres a 1 echouent). VERTS par construction : Test_FollowOrdersBodyguardWithPlayerOrderIsNotInterrupted et Test_FollowOrdersStepUnitUnknownUnitQueuesNothing (le stub ne fait rien, comme la regle pour ces cas) : ils gardent la regle contre une regression, ne la prouvent pas.
- Sur ba2b62e (tip) : les 3 verts attendus ; les 10 autres tests de HOLFollowOrders_Test.as inchanges.
- Si le script ne compile pas a la synchro : premiers suspects, `FVector UnitLocation;` sans initialiseur passe en sortie `FVector&` de ReadUnit (ok en Angelscript, a confirmer), `nullptr` pour UObject WorldContext (deja utilise par 1121sd/sf).

## RESTE
1. Appelant (C++ ou script d'un controleur a creer) : appeler StepUnit a chaque pas d'ordre pour chaque suiveur du FHOLFollowBook, avec bPlayerOrderActive (source a lire : la file d'ordres du joueur pour l'unite, pas encore exposee), et OnManualOrder quand StepUnit rend false ou que le joueur donne un ordre (morceau 4 : clic droit sur unite -> Begin).
2. Test de simulation du cas positif (ennemi vivant a portee d'une unite nee), D838, en C++.
3. Placeholders.txt : aucun provisoire ajoute ou retire.

## NON VERIFIE
Compilation script, execution de tous les tests, DLL du worktree vs main, diagnostics AS. Le sens de « ordre du joueur en cours » : aujourd'hui tout ordre manuel TERMINE le suivi (OnManualOrder, Q2 B), donc bPlayerOrderActive ne sert que si le jeu garde le suivi actif pendant un ordre ponctuel ; non tranche, pas de question posee (la regle est derriere le parametre).
PROCESSUS_DETACHES: aucun (aucun processus lance)
ANOMALIES_PROCESSUS: aucune ; premier Edit apres le 8e appel (lectures de decouverte).
GIT_STATUS_PRINCIPALE: EtapeDefini.md et OrdersPatrolInProgress.md modifies par moi (docs voulus), le reste appartient a d'autres sessions ; aucun fichier de code touche dans la copie principale : oui.
