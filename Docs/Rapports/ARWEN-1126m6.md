# ARWEN-1126m6 (15:44 - 16:14) : vrai clic Slate sans effet sur le Mangonneau ; hypothese "Down+Up dans la meme image" ECARTEE ; log de diagnostic ecrit, build NON TERMINE a l'arret

APPELS_DECLARES: ~42 (arret anticipe a 16:0x, build en file)
HEURE_DEBUT/HEURE_FIN: 15:44 / voir fin
PREMIER_BUILD: 15:47 (`Tools\build.ps1 -LiveCoding=0 -Wait`, en file derriere les builds des autres worktrees, aucun cl.exe lance a 15:56)   BUILD: NON COMPILE (voir COMMITS)   LAUNCHCHECK_DEBUT: non lance (build direct)
TEST_ROUGE: aea5560 (arwen_pick.txt : `hol.Mouse.Drag 533 300 533 300 4` = appui tenu 4 images puis relache, Selection report: selected none) ; rouge de reference 281f3f7 (ARWEN-1126m5) ; assertit l'effet : oui (la selection du Mangonneau)   TEST_ROUGE_SEPARE: non (pas de correctif)
COMMITS: voir git log integ-100b (aea5560 scenario ; log de diagnostic C++ : commit NON COMPILE si fait), merge_main_fait: non
TESTS: play.ps1 arwen_pick -Hidden (28 s, 15:45) : Mouse.Click -> none ; Mouse.Drag tenu 4 images -> none ; Select.Click 0.5 0.5 -> Mangonel #13 ; ClickUnit -> idem. TESTS_TAUTOLOGIQUES: 0
ESSAIS: aucun
QUESTIONS: aucune
FAIT:
- Il n'existe pas de `hol.Mouse.Down` / `hol.Mouse.Up` (seuls Click, Drag, Hover, HOLUIMouse.cpp:339-350) ; `hol.Mouse.Drag` en est l'equivalent (appui, une image par pas, relache a la derniere) : un appui TENU 4 images au meme point ne selectionne rien non plus. Donc le piege "Started et Completed dans la meme image" (piste 2 de ARWEN-1126m5) est ECARTE.
- Le journal dit pourtant "Mouse click 533,300 ... on SViewport" : Slate trouve bien le viewport, mais rien n'atteint (ou ne passe) HandleSelectStarted, ou il sort tot.
- Diagnostic ecrit (HOLPlayerController.cpp, tete de HandleSelectStarted et de HandleSelectCompleted) : `ARWEN diag: HandleSelectStarted frozen/placing/aim/alt` et `HandleSelectCompleted pressed/wallDrag/signal` (LogHOLMatch Display).
- Observation de lecture : depuis 8d79aaa les modes armes (attaque-mouvement 100, banniere 101, Sceau 102) sont fusionnes dans FactionAimIndex (SetArmedMode -> SetFactionAim) ; HandleSelectStarted sort sans selectionner si FactionAimIndex >= 0 (HOLPlayerController.cpp:5225-5235, avec le log). Un mode arme laisse actif par le scenario (le Deploy du Mangonneau arme un mode ? Answer.Kind == Arm, :2680) expliquerait "rien selectionne" ; SelectAt (hol.Select.Click) ne lit pas FactionAimIndex, d'ou son succes. A VERIFIER par le log (champ aim=).
RESTE (pour un agent neuf) :
1. Quand le build passe : `play.ps1 arwen_pick -Hidden -Root C:\Users\baudo\Documents\HeroesOfLiberty-Arwen`, lire les lignes `ARWEN diag` apres l'etape Mouse.Click : absentes = l'action IA_Select ne se declenche pas (mapping/focus Slate) ; presentes avec aim>=0 = mode arme residuel (correctif : le scenario/script remet SetArmedMode(-1) ou la selection quitte le mode, a decider) ; presentes avec frozen/placing = autre cause.
2. Correctif en 2e commit, test rouge = arwen_pick (deja rouge), puis rejouer deploy_far.
NON VERIFIE: tout ce qui depend du log (le build n'a pas tourne) ; l'hypothese du mode arme (de lecture seule).
PROCESSUS_DETACHES: voir fin
ANOMALIES_PROCESSUS: build.ps1 (PID 68136) en file sans cl.exe pendant 9 min+ (file de build partagee)
GIT_STATUS_PRINCIPALE: docs seulement : oui
