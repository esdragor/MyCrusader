# FARAMIR-1121sa : voir à l'écran le correctif des lignes d'ordre multi-blocs

APPELS_DECLARES: 17 (arrêt à 45 vus par toi ; le seuil de 80 se juge sur `tool_uses` relevé par la session principale)
HEURE_DEBUT/HEURE_FIN: 19:12 / 19:17 (Get-Date PowerShell)
PREMIER_BUILD: sans C++   BUILD: sans C++ (binaire à jour, preflight lancé tel quel)   LAUNCHCHECK_DEBUT: sans C++
TEST_ROUGE: sans objet (essai à l'écran ; test déjà dans main : 5658626 test + 904bd5e correctif) + « assertit l'effet : oui » (HOLOrderLines_Test.as, 5 tests, hors tâche)
COMMITS: 1 (c399c87 sur zone-effect : order_lines_blocks.txt/.look/.json), dernier à 19:14, merge_main_fait: oui (zone-effect = main 6b226f8 au départ)
TESTS: aucun quick_check (interdit par le brief) ; preflight order_lines_blocks : PASS (22 s) ; TESTS_TAUTOLOGIQUES: 0 ; BUILDS_DEMANDES: 0 ; TESTS_SCRIPT_AJOUTES: 0 ; TYPES_A_EXPOSER: aucun ; DIAGNOSTICS_AS: 0 erreur (aucun .as touché)
ESSAIS: order_lines_blocks : preflight PASS (1er passage FAIL : json sans mapSize, corrigé ; 2e : PASS), PNG order_lines_blocks_pendant ouvert, VU À L'ÉCRAN (deux lignes jaunes, une par bloc, partant de chaque bloc et se rejoignant au point d'ordre) ; commande : `powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Faramir\Tools\Scenarios\play.ps1 order_lines_blocks -Root C:\Users\baudo\Documents\HeroesOfLiberty-Faramir` ; fiche Retours\E-order_lines_blocks.md ([À ESSAYER], GESTE: interne, SON: sans objet)
QUESTIONS: aucune
FAIT / RESTE: FAIT : scénario 12 unités en 2 blocs à 30 m (6 Lanciers + 6 Hommes d'armes), hol.Select.Army puis hol.Select.OrderGround -13000 0 (coordonnées du MONDE), capture 20 frames après l'ordre ; fiche E ; BUG-order_lines_multi_blocs.md complété (« VU À L'ÉCRAN »). RESTE : geste réel (clic droit) à jouer par l'utilisateur ; commit du .look si retouché ; EtapeDefini.md et Placeholders.txt non touchés (pas dans les fichiers pris).
NON VÉRIFIÉ: vrai clic droit ; un seul bloc ; file Maj + clic droit à deux blocs ; mouvement dans le temps (une seule capture) ; le .look dit « nord/sud » alors que l'écran montre les blocs à gauche/droite (fiche E correcte).
PROCESSUS_DETACHES: aucun processus detache pour C:\Users\baudo\Documents\HeroesOfLiberty-Faramir
ANOMALIES_PROCESSUS: aucune. Preflight : WARN sans 'HUD overlap:' ni 'Sanity:' (non traités) ; 20 'Condition failed' des smoke tests moteur (UnifiedErrorTests, hors scénario).
GIT_STATUS_PRINCIPALE: modifié : Auto_Journey_File.md, Auto_Journey_Jour.md, EtapeDefini.md, GDD.md, MoraleInProgress.md (autres sessions), Retours/BUG-order_lines_multi_blocs.md (le mien) ; non suivis : Docs/*, Rapports d'autres agents, Retours/E-chat_input.md (autre agent), Retours/E-order_lines_blocks.md (le mien). Inchangée hors docs voulus : oui.
