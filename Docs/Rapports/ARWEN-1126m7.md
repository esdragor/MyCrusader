# ARWEN-1126m7 (16:02 - 16:18) : clic reel sans effet = pixel de scenario (533,300 vs viewport 1280x720), pas le jeu

APPELS_DECLARES: ~40 (arret a 16:32 non atteint)
HEURE_DEBUT/HEURE_FIN: 16:02 / 16:18
PREMIER_BUILD: sans C++   BUILD: PASS (build de f887556 verifie : "Result: Succeeded", 16:01, avant mon debut ; aucun build lance par moi)   LAUNCHCHECK_DEBUT: sans C++
TEST_ROUGE: 71d892e (arwen_pick : timeout a l'etape 21 `waitlog Selection: Mangonel` apres hol.Mouse.Click 533 300, vu 16:14) ; assertit l'effet : oui (selection du Mangonneau)   TEST_ROUGE_SEPARE: oui
COMMITS: 2 sur la branche arwen-1126m7 (depuis main 77b857a, sans UE_LOG de diag) : 71d892e (test), ceeffd1 (correctif), dernier 16:15, merge_main_fait: non (branche prete)
TESTS: play.ps1 arwen_pick -Hidden vert (20 s) ; play.ps1 deploy_far -Hidden VERT (50 s, Deploy: phase 0->1 walk 3175 cm, puis 1->2, quit atteint). TESTS_TAUTOLOGIQUES: 0
ESSAIS: aucun
QUESTIONS: aucune
FAIT / RESTE:
- Log diag (f887556, build OK) : HandleSelectStarted frozen=0 placing=0 aim=-1 alt=0 ; HandleSelectCompleted pressed=1 : piste FactionAimIndex >= 0 (HOLPlayerController.cpp:5225) ECARTEE, le handler selectionne bien (SelectAt) mais au pixel clique.
- Cause : le jeu masque tourne en 1280x720 (r.setres) ; 533,300 est le centre d'un viewport 1066x600. Sonde : clic 640,360 -> "selected Mangonel #13", clic 960,540 -> none.
- Correctif : 533 300 -> 640 360 dans arwen_pick.txt, deploy_far.txt (1) et deploy.txt (2). arwen_pick.txt/json committes sur la branche propre (le .json est copie de integ-100b).
- RESTE : autres scenarios supposant une autre taille (alert_population/nomad_start cliquent 960 540 = 1920x1080 ; signals_altclick 1400 350) a verifier ; pixel fragile si la resolution change (une liaison C++ `hol.Mouse.Click` en fraction serait la vraie solution, lot C++ a decider). La branche integ-100b garde les UE_LOG de diag (f887556, NON COMPILED) : a ne pas fusionner.
NON VERIFIE: que l'utilisateur joue aussi en 1280x720 (son clic reel dans sa propre fenetre n'est pas concerne par ce correctif) ; mes 40 appels sont un decompte de memoire.
PROCESSUS_DETACHES: voir stray_processes ci-dessous
ANOMALIES_PROCESSUS: premier arwen_pick de la branche propre a echoue (timeout etape 8) faute de arwen_pick.json, copie depuis integ-100b ; 150 s perdus
GIT_STATUS_PRINCIPALE: docs seulement : oui (rien de code ecrit dans la copie principale)PROCESSUS_DETACHES (sortie finale): aucun processus detache pour C:\Users\baudo\Documents\HeroesOfLiberty-Arwen
