APPELS_DECLARES: ~22 (limite 20 depassee : lectures de PNG en parallele)
HEURE_DEBUT/HEURE_FIN: 10:45 / 10:50
PREMIER_BUILD: sans C++   BUILD: sans objet   LAUNCHCHECK_DEBUT: sans C++
TEST_ROUGE: sans objet (scenario)   TEST_ROUGE_SEPARE: non (pas de code)
COMMITS: 1 sur integ-41 (4d61f5d, 10:50), merge_main_fait: non
TESTS: preflight tower_wood 1 fois : check PASS 40 s ; global FAIL(1) = 2 fichiers non commites (corrige par le commit) ; TESTS_TAUTOLOGIQUES: 0
ESSAIS: tower_wood : preflight check PASS, PNG during/during2/during3/during4/after ouverts (before non ouvert), FLECHE VUE (petit trait orange + hampe sombre, pas en vol : plantee/en fin de vie), LOGIQUE SEULEMENT (geste interne, son non entendu). play.ps1 tower_wood -Root C:\Users\baudo\Documents\HeroesOfLiberty-Sauron
PREUVE RENDU ARCHERS : aucun PNG recent de cavalry_vs_archers / ai_* / siege_wave dans les worktrees (seuls des siege_wave du 2026-11-08 dans Sauron, non rouverts) ; pas rejoue. Preuve de remplacement : hol.Combat.ArrowReport dans tower_wood = "launched 1 landed 1" puis "launched 2 landed 2" (Source HOLArrowSubsystem.cpp:32) : le rendu des fleches de tour marche en -RenderOffScreen.
CAUSE : (1) HOLCombatProcessor.cpp:968-976 dessine la fleche de tour en solo/hote via UHOLArrowSubsystem::Launch ; la voie repliquee (RecordArrowShot / OnRep_ArrowShot, HOLBuilding.cpp:387-427) ne sert qu'aux clients : T7 dessine bien en standalone. (2) La fleche vole 6 pas (~0,3 s) a 12 m : l'ancienne capture sur "lands on" (:1665) tombait APRES l'impact ; a 30 m le trajet depassait aussi la bande utile. (3) Fleche minuscule a Zoom 2 (~20 px) mais visible. Aucun log "releases an arrow" pour les fleches de tour (:953/:2751) : lot C++ possible. Fleche en vol non capturee : a viser avec ArrowReport "in flight 1" (frame ~21, juste apres le lancer, avant "lands on" frame 45), pas de 2e preflight lance (consigne UNE fois).
FAIT: tower_wood.txt/.look (Lanciers a 12 m, ArrowReport, 4 captures + after), fiche E-tower_wood.md, commit 4d61f5d.
RESTE: capture a l'image du lancer (frame ~21 : capture juste apres ArrowReport sans waitlog lands on) ; agrandir la fleche (ArrowScale HOLArrowSubsystem.cpp:30) ; log "releases an arrow" pour les tours (lot C++).
NON VERIFIE: fleche en vol ; PV ; geste reel ; son ; before non ouvert ; Sauron contient integ-41 ?
QUESTIONS: aucune
ANOMALIES_PROCESSUS: limite d'appels depassee (~22/20) ; premier edit au 5e appel (IN_USE) ; Read sur PNG en parallele.