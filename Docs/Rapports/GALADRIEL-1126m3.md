GALADRIEL-1126m3 : Bug #19 de ai_tower (orbite d'un Lancier autour de sa place de bloc). Branche orbit-block-1126m3 (depuis main 77b857a), worktree HeroesOfLiberty-Galadriel. BUILD PASS (842 s), quick_check -Parts tests PASS 2340/2340 (ArrivalBrake vert). ai_tower NON rejoue (arret dur 16:06).

APPELS_DECLARES: ~24 (arret a 45 vus)
HEURE_DEBUT/HEURE_FIN: 15:37 / 16:03
PREMIER_BUILD: lance 15:40 (file : derriere Celeborn et Faramir)   BUILD: PASS (842 s)   LAUNCHCHECK_DEBUT: DLL UnrealEditor-HeroesOfLiberty.dll (14:17) plus vieille que les sources, UI dll (11:34) idem : build d'abord (c'est le build lance)
TEST_ROUGE: a99ab44 (HeroesOfLiberty.Movement.ArrivalBrake + bouchon ArrivalSpeed sans freinage) ; assertit l'effet : oui (l'unite doit ARRIVER sur sa place en 30 s, vitesse venant de la regle ArrivalSpeed, vitesse tournant a 50 deg/s) ; rouge VU : PAS a l'execution UE (un seul build, file occupee) mais par simulation Python hors UE du meme modele : sans freinage, depart a 200 cm et 800 cm = jamais arrive en 30 s (reste a 682 / 780 cm : orbite), a 3000 cm arrive ; avec freinage : arrive en 5,2 / 8,4 / 11,7 s.   TEST_ROUGE_SEPARE: oui (deux commits : a99ab44 test+bouchon, 5722a01 correctif)
COMMITS: 2 (a99ab44, 5722a01), dernier a 15:40, merge_main_fait: non (branche creee depuis main 77b857a)
TESTS: quick_check -Parts tests 2340/2340 PASS (16:01) ; TESTS_TAUTOLOGIQUES: 0 (la vitesse est calculee par ArrivalSpeed, la position par integration du virage a 50 deg/s)
ESSAIS: ai_tower non rejoue (build pas fini a l'arret)
QUESTIONS: aucune
FAIT / RESTE:
- Cause (lecture de HOLSimulationProcessors.cpp ~l.1327-1470, processeur de marche) : phase FinalLeg, la direction va droit a la place mais la vitesse reste DesiredSpeed (jusqu'a 680 cm/s) ; la vitesse tourne a ~50 deg/s (rayon de virage = v/w = ~7,8 m) : arrivant de cote a moins de ~8 m, l'unite tourne en orbite et la capture (distance <= vitesse x pas) n'a jamais lieu.
- Correctif C++ (raison : processeur Mass, code par entite et par pas) : HOLGroupMove::ArrivalSpeed(Speed, SlotDistance) = min(Speed, max(40 cm/s, w x distance x 0,5)), HOLGroupMoveRules.h/.cpp ; HOLSimulationProcessors.cpp : WalkSpeed = ArrivalSpeed seulement en phase FinalLeg (la marche en bloc FormUp/Block garde sa vitesse, sa place bouge avec elle).
- Test : Tests/HOLMovementTests.cpp FHOLMovementArrivalBrakeTest.
- RESTE : play.ps1 ai_tower -Hidden -Root C:\Users\baudo\Documents\HeroesOfLiberty-Galadriel : plus de Sanity FAIL #19. Risque : la detection de FinalLeg bloquee (FinalLegStuckSteps) pendant la fin freinee (40 cm/s) ; le modele de virage (50 deg/s) est celui du commentaire D290, non remesure.
NON VERIFIE: ai_tower (non rejoue), effet sur les autres scenarios de marche (groupes, diplomacy_qground_watch) ; rouge UE non vu.
BUILDS_DEMANDES: 1   TESTS_SCRIPT_AJOUTES: 0   TYPES_A_EXPOSER: aucun
PROCESSUS_DETACHES: stray_processes.ps1 -Stop lance en dernier geste
ANOMALIES_PROCESSUS: un build est en file/lance en tache de fond depuis ce worktree.
GIT_STATUS_PRINCIPALE: aucun fichier de code modifie : oui (docs : EtapeDefini.md, ce rapport)
