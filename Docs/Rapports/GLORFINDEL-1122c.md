# GLORFINDEL-1122c : pont de Suivre NON PROUVÉ ACTIF (scénario follow_short ROUGE, le pont n'est jamais appelé)

pont prouvé actif : non
branche prête : oui (ability-columns, 2 commits, pas de C++ ; merge main non fait, non demandé ; scénario rouge, à ne pas lister [À ESSAYER])

APPELS_DECLARES: 25 (arrêt à 25 vus ; premier Edit au 11e appel environ : consigne « avant le 8e » non tenue, 10 appels de lecture)
HEURE_DEBUT/HEURE_FIN: 16:49 / 16:57 (Get-Date PowerShell)
PREMIER_BUILD: sans C++   BUILD: sans build   LAUNCHCHECK_DEBUT: sans C++
TEST_ROUGE: scénario follow_short (2e commit) : rouge vu à 15.52.07 UTC (journal : « Test sequence: timeout at step 22: waitlog FollowBridge: follow started ») ; assertit l'effet : oui (ligne émise par le script du pont)   TEST_ROUGE_SEPARE: non (aucun correctif : le C++ n'est pas à corriger, c'est la visée du scénario)
COMMITS: 2 (journal du pont ; scénario), dernier à 16:57, merge_main_fait: non
TESTS: 1 passage de `play.ps1 follow_short -Hidden -Root C:\Users\baudo\Documents\HeroesOfLiberty-Glorfindel -TimeoutSec 300` (a duré > 180 s, passé en arrière-plan : le jeu masqué n'a pas quitté après l'échec à 60 s) ; TESTS_TAUTOLOGIQUES: 0 ; TESTS_SCRIPT_AJOUTES: 0
ESSAIS: follow_short : preflight NON lancé, PNG start ouvert seul (walk, walk2, cancelled non produits : la séquence a échoué à l'étape 22) : LOGIQUE SEULEMENT, ROUGE
QUESTIONS: aucune
BUILDS_DEMANDES: 0   TYPES_A_EXPOSER: aucun

## (1) Journal du pont (HOLFollowBridge.as, commit 1)
Une ligne par ÉVÉNEMENT via HOLScriptLog::LogDisplay(n"LogHOLMatch", ...) : « FollowBridge: follow started, N units follow #T » (RightClickOnUnit, si le résultat est Follow) et « FollowBridge: manual order ended the follow of N units » (ManualOrder, seulement si au moins une unité du livre était concernée, via HOLFollowOrders::IndexOf). Rien dans Step. 0 erreur Angelscript au chargement ; la présence de la classe script du pont reste non prouvée (aucun appel).

## (2) Scénario (commit 2)
Caméra FocusNear 0 3500, 4 Hommes d'armes (SpawnHere 0 2500) + Paysan (SpawnHere 0 4500), hol.Select.Army, 3 relevés `hol.Units.Where` (avant, +4 s, +9 s) + 3 captures, `hol.Select.OrderNear 0 4500` (vrai clic droit visé sur le Paysan : chemin HOLPlayerController.cpp:1426 FindTargetAt puis :1465 NativeRightClickOnUnit), waitlog des deux lignes de suivi, puis `hol.Select.OrderNear 1500 3500` (ordre manuel), waitlog « FollowBridge: manual order ended », capture, note, `quit`. Durée prévue ~25 s.

## (3) Résultat de l'unique passage (journal, UTC)
Lignes d'événement « FollowBridge: » : AUCUNE (le seul « FollowBridge » du journal est l'écho du timeout).
Ligne C++ « Order: 4 units follow # » : AUCUNE. Le journal montre : « Right click near the centre at (-14800, 4500) » puis « Order: 4 units to (-14792, 4496) » puis « RpcOrder: move received by the server » = chemin SOL, pas chemin unité.
Relevé de position unique (le scénario n'a pas passé l'étape 22) : `Units where: 5 units: #1 (-12763, 2634); #2 (-14677, 3955); #3 (-13241, 3352); #4 (-13064, 4449); #5 (-17032, 1185)` tous Idle. Distance avant/après : NON MESURABLE (un seul relevé).
Erreurs : « Angelscript.*rror » = 0 ; « requires WorldContext » = 0.
Captures : follow_short_start ouvert ; les trois autres ne datent pas de ce passage (follow_short_walk et follow_short_cancelled du passage 1122b, follow_short_walk2 absente).

## CAUSE (ROUGE, lue dans le code et le journal)
Le pont n'a pas été appelé parce que le scénario a visé le vide : SpawnHere ne place pas les unités au point (dx, dy) demandé dans le cadre du centre : les hommes sont dispersés autour du point de focus de la caméra (x -12763 à -14677, y 2634 à 4449) et le Paysan est en (-17032, 1185), alors que le clic de `OrderNear 0 4500` a visé (-14800, 4500). FindTargetAt (HOLPlayerController.cpp:1426) n'y trouve aucune unité, UnderId = 0, la condition de la branche Suivre (:1459-1461) est fausse : ordre de déplacement au sol, NativeRightClickOnUnit (:1465) non atteint. Ce n'est PAS une preuve que le pont est cassé ; ce n'est pas non plus une preuve qu'il marche. Aucun défaut C++ identifié (HOLFollowBridge.cpp:1-80 : FindScriptClass puis GetFor créent bien l'instance si une classe script concrète existe).
Même cause pour 1122b : « OrderNear 1500 0 » y annulait un suivi jamais démarré par le pont.

## RESTE (agent neuf)
1. Viser le Paysan RÉEL : soit une position de spawn déterministe (verbe absolu de spawn, ou SpawnHere vérifié ; la dispersion vient de SpawnHere, à lire dans le C++/script de la commande), soit lire la position de #5 par `hol.Units.Where` puis `hol.Select.OrderNear <dx> <dy>` avec dx = X - (-14800), dy = Y (cadre du centre principal : OrderNear 0 4500 = (-14800, 4500)) ; ou, sans C++, le clic sur le Paysan par `hol.Select.ClickUnit` ne passe PAS par le pont (script C10a:256-259 appelle le livre directement).
2. Rejouer : attendre la ligne « FollowBridge: follow started » ; sinon la cause est dans HOLFollowBridge.cpp FindScriptClass.
3. play.ps1 : le jeu ne quitte pas sur échec de séquence (180 s+) : à vérifier (outil).

NON VÉRIFIÉ : le pont s'exécute-t-il, l'unité suit-elle, l'ordre manuel annule-t-il (aucune des deux lignes d'événement vue) ; follow_short_walk2 et les relevés suivants ; preflight.
DIAGNOSTICS_AS: non relus dans VS Code ; journal du jeu : 0 erreur Angelscript.
PROCESSUS_DETACHES: sortie de stray_processes en dernier geste (voir ci-dessous dans la réponse de séance)
ANOMALIES_PROCESSUS: play.ps1 passé en arrière-plan à 180 s (le jeu masqué n'a pas quitté après l'échec de séquence à 60 s)
GIT_STATUS_PRINCIPALE: inchangée hors EtapeDefini.md, Auto_Journey_File.md et ce rapport : oui (non relancé)