# ELROND-1124e : F4, D877 moitié A (engin démonté, cible lointaine), scénario deploy_far

Impact joueur : un Mangonneau démonté envoyé par clic droit sur une Maison ennemie lointaine marche, se monte tout seul à portée et tire : le défaut n'est PAS reproduit sur integ-25.

APPELS_DECLARES: 20 (arrêt à 25 vus)
HEURE_DEBUT/HEURE_FIN: 11:35:58 / 11:40 (Get-Date PowerShell ; arrêt dur 11:51)
PREMIER_BUILD: sans C++   BUILD: NON COMPILÉ (sans C++, aucun build)   LAUNCHCHECK_DEBUT: sans C++
TEST_ROUGE: 7c790ca (scénario deploy_far, assertions waitlog `Mangonel hits a building` + `D=Pack`), assertit l'effet : oui ; ROUGE NON VU : le scénario passe au vert (check 56 s). TEST_ROUGE_SEPARE: oui pour le scénario (commit seul) ; pas de correctif (aucun défaut à corriger).
COMMITS: 2 (7c790ca scénario, aac8127 journal), dernier à 11:40, merge_main_fait: non (pas demandé, sans C++)
TESTS: preflight.ps1 deploy_far -Root C:\Users\baudo\Documents\HeroesOfLiberty-Elrond : PASS (check 56 s, 0 warning, sanity OK, 0 HUD overlap, 4 PNG) ; TESTS_TAUTOLOGIQUES: 0 (effet = coups de siège calculés par la simulation)
ESSAIS: deploy_far : preflight PASS, 4 PNG ouverts et vus, VU À L'ÉCRAN (coups prouvés par le journal : PV non lisibles), geste réel (clic gauche, touche D x2, clic droit sol). Fiche Retours\E-deploy_far.md. Commande : powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Elrond\Tools\Scenarios\play.ps1 deploy_far -Root C:\Users\baudo\Documents\HeroesOfLiberty-Elrond
QUESTIONS: aucune
FAIT / RESTE:
- Fait : la ligne de journal « Deploy: » existe (HOLDeployProcessor.cpp:139, `Deploy: #%u phase %d -> %d (order %d, action %d, reason %d, distance %d cm, walk %d)`).
- Fait : scénario autonome deploy_far.txt / .look / .json (Mangonneau démonté après D monte puis D démonte ; Maison à 6500 cm > 2 x portée 3000).
- Résultat : marche 16 s, `Deploy: #13 phase 0 -> 1 (order 1, ..., distance 3173 cm, walk 2)`, `phase 1 -> 2 (delay over)`, 2 x `Mangonel hits a building for 45`. Même résultat que ELROND-1122a (main 1447448) : moitié A verte, la cause supposée de RECETTE-1121se (cc7c56e/ef6cbf2 ne couvrent que la marche coupée) n'est plus d'actualité sur integ-25 f426bd4 ; aucune cause ni correctif à proposer, aucun TYPES_A_EXPOSER.
- Reste : la session principale clôt la ligne « BUG D877 moitié A » ; rejouer par un autre agent sur main fusionné (règle 19).
NON VÉRIFIÉ: pas de PV de la Maison à l'écran (non sélectionnée) ; session_gate -Phase Essay non lancé ; son non entendu ; test C++ Mass ClickOnFarTargetWhilePackedComesInMountsAndFires (6baa6ba) non relancé ; ligne de file Auto_Journey_File.md non modifiée (lecture seule de ma part).
PROCESSUS_DETACHES: aucun processus detache pour C:\Users\baudo\Documents\HeroesOfLiberty-Elrond
ANOMALIES_PROCESSUS: aucune ; IN_USE.txt est ignoré par git (git add refusé, écrit sur disque). Une commande PowerShell a échoué au 2e appel (guillemet manquant, sans effet) ; la version de la ligne de journal dans deploy.txt l.166 contient un `r`n littéral (ancien, non touché).
GIT_STATUS_PRINCIPALE: copie principale : seuls docs de suivi modifiés (EtapeDefini.md, Retours\E-deploy_far.md, ce rapport) en plus des fichiers déjà modifiés par d'autres sessions ; aucun code ; inchangée hors docs voulus : oui
