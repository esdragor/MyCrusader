# DIAG-1121sg (integ-46, quick_check -Parts build)

APPELS_DECLARES: ~12 (arret a 25)
HEURE_DEBUT/HEURE_FIN: 21:16:14 / 21:19 (arret dur 21:32)
PREMIER_BUILD: sans C++   BUILD: sans objet (binaire deja compile)   LAUNCHCHECK_DEBUT: sans C++
TEST_ROUGE: sans objet (correctif d'une attente de quick_check.ps1) ; rouge = quick_check build 20 regressions (2 fois, session principale)
COMMITS: 1 (c9ac21c, 21:17), merge_main_fait: non necessaire (integ-46 = main + 4 verts)
TESTS: quick_check -Parts build : TOUT PASS en 112 s (sequence, camera 2p/8p, build + 29 sous-parties dont hero aura/hold/dawn/grid, dropoff*, minimap, minalert) ; TESTS_TAUTOLOGIQUES: 0

## Cause exacte
Premiere etape en echec : "sequence". Journal Saved_Tests\Logs\quick_game.log ligne 6182 :
`LogHOLTest: Error: Test sequence: timeout at step 1380: waitlog cries Serrez les rangs,`
Ce n'est PAS une erreur de script (Galadriel) ni D877 : la sequence de jeu s'arrete a l'etape 1380 (case "build hero aura", quick_check.ps1 ~2059), donc toutes les parties suivantes tombent en cascade (le jeu ferme sur "Closing by request").

Origine : Celeborn 5be21e4 (le cri est pose par le processeur de moral) a supprime de HOLCombatProcessor.cpp la ligne
`Combat: step %lld slot %d cries Serrez les rangs, %d unit(s) under it`. Le processeur de moral ne la logue pas.
quick_check attendait cette ligne (waitlog x3 : aura ~2059, hold ~2081, dawn ~2097 ; et l'analyse 4751 / 4763 / 4765, le compte "N units").
La ligne `Hero ability: slot 0 cries Serrez les rangs` (HOLHeroSubsystem.cpp:680, au lancement) existe toujours.

## Correctif (attente perimee, jeu conforme) : quick_check.ps1, BOM garde (EF), analyse PowerShell 0 erreur
- 3 waitlog : `cries Serrez les rangs,` -> `Hero ability: slot 0 cries Serrez les rangs` (+ `wait 0.3` apres celui de aura, pour que le pas de moral pose le cri, comme hold/dawn).
- CryAt (4751) cherche la ligne `Hero ability: ...`.
- Controle "the cry" : plus de compte d'unites (n'existe plus) ; garde "lance une fois, refuse pendant le refroidissement" ; l'EFFET du cri reste asserte par les coups de la phase suivante (Spearman 7.2, Hero 2.16, fleches 6).

## Commande rejouee
quick_check.ps1 -Parts build : PASS partout (voir TESTS). Une seule execution.

## Notes
- Recommandation facultative C++ (non fait) : logguer dans le processeur de moral "N unit(s) under it" si l'on veut retrouver ce compte.
- NON VERIFIE : parties net / tests complets sur integ-46 apres ce commit (inchange : le commit ne touche que quick_check.ps1).
- PROCESSUS_DETACHES: aucun (le seul quick_check a fini ; stray_processes non lance, pas de worktree edite ailleurs).
- ANOMALIES_PROCESSUS: aucune. GIT_STATUS_PRINCIPALE: non touche hors ce rapport.
