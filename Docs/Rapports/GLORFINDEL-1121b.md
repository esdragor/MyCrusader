# GLORFINDEL-1121b : regle pure Suivre (B5 etape 4), sans C++

Tache : premiere ligne `[ ]` sans C++ de la section Glorfindel = ligne B5 Patrouille et Suivre (la patrouille pure est deja faite 3a636ce ; Suivre et focus n'existaient pas).

APPELS_DECLARES: 15 (arret a 25)
HEURE_DEBUT/HEURE_FIN: 10:56:58 / 10:58:44
PREMIER_BUILD: sans C++   BUILD: NON COMPILE   LAUNCHCHECK_DEBUT: sans C++
TEST_ROUGE: 77b30f8 (test + bouchon, rouge attendu 3/5 : Test_FollowAllyKeepsUpWithAWalkingAlly, Test_FollowAllyAttacksAnEnemyInRange, Test_FollowEnemyIsAnAttack ; NON LANCE, rouge non vu a l'execution) ; assertit l'effet : oui (position finale du suiveur apres une marche tick par tick, ordre tenu)
COMMITS: 2 (77b30f8 test + bouchon, e259262 correctif), dernier a 10:58, merge_main_fait: non (branche ability-columns en integration, non touchee)
TESTS: aucun lance (pas de build ni quick_check) ; TESTS_TAUTOLOGIQUES: 0 (la distance finale vient de la regle, pas d'une constante recopiee)
ESSAIS: aucun
QUESTIONS: aucune
FAIT: Script\Orders\HOLFollowRules.as (Start, Decide, Destination, OnManualOrder, FollowDistance 300 cm) + HOLFollowRules_Test.as (5 tests) dans le worktree Glorfindel.
RESTE: quick_check -Parts tests ; focus (une cible) ; branchement clic droit / case F / relance de la patrouille a chaque jambe (EHOLOrderKind::Patrol : C++ seulement si l'enum n'est pas exposee au script).
NON VERIFIE: rien compile, ni les tests ni la syntaxe Angelscript (DIAGNOSTICS_AS non releves) ; la valeur FollowDistance 300 cm est un choix de reglage non valide en jeu ; test 1 tolere un ecart de 430 cm (300 + un pas de l'allie 80 + marge).
ANOMALIES_PROCESSUS: le worktree Glorfindel etait annonce « en integration : n'y touche pas » ; seuls 2 fichiers neufs y ont ete commites par nom, au-dessus de f804471 (si l'integrateur a deja fige la branche, cherry-pick 77b30f8 + e259262).
PROCESSUS_DETACHES: aucun processus detache pour C:\Users\baudo\Documents\HeroesOfLiberty-Glorfindel
GIT_STATUS_PRINCIPALE: docs seulement (EtapeDefini.md, Auto_Journey_File.md, OrdersPatrolInProgress.md) : oui

Commande de tests (session principale, apres compilation des scripts) :
powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Glorfindel\Tools\Scenarios\quick_check.ps1 -Parts tests -Root C:\Users\baudo\Documents\HeroesOfLiberty-Glorfindel
(filtre attendu : Test_Follow*, 5 tests ; verifier la syntaxe exacte de quick_check.ps1 -Root, non verifiee)
