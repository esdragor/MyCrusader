# GLORFINDEL-1121c : emetteur script de Suivre (B5 etape 4), sans C++

NON COMPILE (aucun build, aucun quick_check, aucune syntaxe Angelscript verifiee).

APPELS_DECLARES: 19 (arret a 25 ; premier edit : IN_USE.txt au 7e appel)
HEURE_DEBUT/HEURE_FIN: 10:59:24 / 11:02:31 (arret dur 11:20)
PREMIER_BUILD: sans C++   BUILD: NON COMPILE   LAUNCHCHECK_DEBUT: sans C++
TEST_ROUGE: 2f88481 (test + bouchon ; rouge attendu 3/4 : Test_FollowOrdersAllyWalkingAwayIsFollowedByMoveOrders, Test_FollowOrdersEnemyTargetIsOneAttack, Test_FollowOrdersAllyBodyguardAttacksEnemyInRange ; Test_FollowOrdersManualOrderEndsIt passe contre le bouchon, c'est un test negatif) ; NON LANCE, rouge non vu a l'execution ; assertit l'effet : oui (contenu de la file d'ordres relue par ConsumeMoves, position finale d'un suiveur qui obeit aux Move)
COMMITS: 2 (2f88481 test + bouchon, b5e9e29 correctif), dernier a 11:01, merge_main_fait: non (worktree en integration, DLL non verifiee)
TESTS: aucun lance ; TESTS_TAUTOLOGIQUES: 0 (l'ecart final et les ordres viennent de la chaine HOLFollowOrders::Step -> HOLFollowRules::Decide -> HOLPlayerOrders:: -> file)
ESSAIS: aucun
QUESTIONS: aucune (D818 : Suivre = clic droit sur un allie ou un ennemi, aucune case ni touche ; rien a inventer ; Q-follow.md non cree)
BUILDS_DEMANDES: 0 ; TESTS_SCRIPT_AJOUTES: 4 (Test_FollowOrdersAllyWalkingAwayIsFollowedByMoveOrders, Test_FollowOrdersEnemyTargetIsOneAttack, Test_FollowOrdersAllyBodyguardAttacksEnemyInRange, Test_FollowOrdersManualOrderEndsIt)
TYPES_A_EXPOSER: (1) position d'une unite par id (le script ne lit ni sa position ni celle de la cible suivie : UHOLUnitView non expose) ; (2) ennemi a portee de l'unite (bool + id) ; (3) un pas d'ordre par unite suivante, ou un tick script, qui appelle HOLFollowOrders::Step ; (4) le clic droit sur une unite (OrderAtFraction, C++) doit appeler HOLFollowOrders::Begin (allie ou ennemi) ; (5) un porteur persistant du FHOLFollowBook (objet script ou sous-systeme). EHOLOrderKind n'est pas un UENUM mais n'est pas necessaire : Suivre s'emet en Move / Attack ordinaires.
DIAGNOSTICS_AS: non releves (rien compile)
FAIT: Script\Orders\HOLFollowOrders.as (FHOLFollowBook ; Begin ; OnManualOrder ; Step : Decide puis HOLPlayerOrders::Move vers la cible, ou ::Attack sur l'ennemi suivi / a portee, un seul Attack par ennemi) + HOLFollowOrders_Test.as (4 tests) dans le worktree Glorfindel (ability-columns).
RESTE: lot C++ d'exposition (liste ci-dessus) puis branchement ; quick_check -Parts tests ; focus ; case F Patrouille. Piege : le C++ qui rappelle HOLFollowOrders::Step ne doit pas appeler OnManualOrder pour les ordres que Step emet lui-meme (ils passent par les memes emetteurs HOLPlayerOrders que le joueur).
NON VERIFIE: tout (ni compilation, ni syntaxe, ni tests) ; deplacement Move relance a chaque pas tant que l'ecart depasse 300 cm (cout du chemin non mesure) ; FollowDistance 300 cm reste un reglage non valide en jeu ; l'appel depuis le jeu n'existe pas : la regle n'est PAS branchee.
PROCESSUS_DETACHES: aucun processus detache pour C:\Users\baudo\Documents\HeroesOfLiberty-Glorfindel
ANOMALIES_PROCESSUS: l'appel de la regle depuis le script d'ordres existant est impossible sans C++ : HOLPlayerOrders.as n'est qu'un jeu d'emetteurs appeles par le C++ (HOLPlayerOrders.cpp / Board), le clic droit et le pas d'ordre sont en C++ (HOLUnitOrderProcessor.cpp, HOLPlayerController), et le script n'a aucune lecture de position d'unite. Le worktree etait annonce en integration (meme remarque que 1121b : 3 fichiers neufs commites par nom).
GIT_STATUS_PRINCIPALE: docs seulement (EtapeDefini.md, Auto_Journey_File.md, OrdersPatrolInProgress.md, ce rapport) : oui

Commande de tests (session principale, apres compilation des scripts ; la syntaxe -Parts/-Root est celle donnee par 1121b, non verifiee) :
powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Glorfindel\Tools\Scenarios\quick_check.ps1 -Parts tests -Root C:\Users\baudo\Documents\HeroesOfLiberty-Glorfindel
(filtre attendu : Test_Follow*, 5 tests de la regle + 4 de l'emetteur ; rouge visible sur 2f88481 seul)
