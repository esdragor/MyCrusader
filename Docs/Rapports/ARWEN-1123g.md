# ARWEN-1123g (11:17) : EndWallDrag, ligne de mur sans sol au relache

APPELS_DECLARES: 22 (arret par limite d'appels)
HEURE_DEBUT/HEURE_FIN: 11:10 / 11:17
PREMIER_BUILD: 11:11 BUILD: PASS (29 s, 68c86d0 compile)  LAUNCHCHECK_DEBUT: binaire plus vieux que HOLPlayerController.cpp (build lance ensuite)
TEST_ROUGE: aucun test C++ ecrit (EndWallDrag exige un controleur + camera + curseur : pas de test unitaire petit). Rouge = le scenario wall_line_long, vu rouge par ARWEN-1123f (journal : wall line (-17420,-1100) to (-17420,-1100), 1 fondation, Bois -5) ; vert apres 68c86d0 : assertit l'effet : oui (non separe d'un mutant temporaire). TEST_ROUGE_SEPARE: non (rouge hérité du rapport 1123f)
COMMITS: 0 nouveau (68c86d0 deja sur integ-55), merge_main_fait: non
TESTS: quick_check -Parts tests : 2132/2132 PASS (205 s) ; TESTS_TAUTOLOGIQUES: 0
ESSAIS: wall_line_long : preflight PASS (37 s), PNG after ouvert et vu (fondations en file, Bois 20160 = -8x5), journal "wall line (-17420,-1100) to (-17420,1800)" ; LOGIQUE SEULEMENT (geste interne). play.ps1 wall_line_long -Root C:\Users\baudo\Documents\HeroesOfLiberty-Sauron
QUESTIONS: aucune
CAUSE RACINE: GetCursorGround echoue car GetCursorFraction (HOLPlayerController.cpp:1004-1025) renvoie faux sans CVarTestCursor quand !Viewport->IsForegroundWindow() ou curseur hors fenetre ; en capture hors ecran le fantome gele a 8 cases (image 235) puis le relache n'a plus de sol. Hypothese forte, non tracee (aucun log ajoute). Ligne [ ] en S3-b.
FAIT: correctif 68c86d0 compile et verifie par scenario (8 fondations au lieu d'1). RESTE: tracer le foreground ; ligne 16+ cases ; touche D reelle.
NON VERIFIE: cause exacte de la perte de sol ; touche D reelle ; mutation rouge de 68c86d0 non rejouee.
PROCESSUS_DETACHES: aucun processus detache pour C:\Users\baudo\Documents\HeroesOfLiberty-Sauron
ANOMALIES_PROCESSUS: premier Edit au 5e appel (IN_USE via PowerShell) ; quick_check passe en arriere-plan apres 180 s ; arret a la limite de 22 appels.
GIT_STATUS_PRINCIPALE: M Auto_Journey_File.md
 M Auto_Journey_Jour.md
 M EtapeDefini.md
 M Placeholders.txt
?? Docs/Rapports/ARWEN-1123f.md
?? Docs/Rapports/BUILD-CELEBORN-1123j.txt
?? Docs/Rapports/CELEBORN-1123j.md
?? Docs/Rapports/CELEBORN-1123k.md
?? Docs/Rapports/DIAG-1123d.md
?? Docs/Rapports/FARAMIR-1123b-fix.ps1
?? Docs/Rapports/FARAMIR-1123g.md
?? Docs/Rapports/GALADRIEL-1123k.md
?? Docs/Rapports/GLORFINDEL-1123m.md
?? Docs/Rapports/INTEG-56-verts.txt
?? Docs/Rapports/RECETTE-1123a.md
?? Retours/E-wall_line_long.md
?? Retours/Q-porte-palissade.md