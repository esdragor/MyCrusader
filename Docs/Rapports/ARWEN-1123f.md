# ARWEN-1123f (2026-11-23 11:07-11:12, Sauron integ-55) : diag pose en ligne Palissade

NON COMPILE (correctif C++ de 3 lignes, la principale compile). Pas de test rouge (arret a 22 appels, rouge non ecrit).

## Cause : DEFAUT DU JEU (pas du scenario), HOLPlayerController.cpp:3451-3452 (EndWallDrag)
- Le geste du scenario est reel : `hol.Mouse.Drag` passe par Slate (journal : "Mouse drag 200,540 -> 1700,540 ... pressed on SViewport"), l'appui entre dans HandleSelectStarted -> BeginWallDrag ("Placement: Palisade wall drag started at (-17420, -1100)"), le pont script reconnait Palisade (HOLWallLineRules.as:197), le fantome croit ("Wall ghost: cells=1..8 valid=true"). `hol.Build.Place` arme bien le mode placement, et la Palissade est un type de ligne : armement interne OK (la touche D reste non prouvee).
- Au relache (journal scenario_wall_line_long.log l.2391) : "Palisade wall line (-17420, -1100) to (-17420, -1100)" : la fin = le DEBUT. EndWallDrag initialisait `Ground = WallDragStart` puis `GetCursorGround(Ground)` ; quand le curseur n'a pas de sol au relache (GetCursorFraction faux : fenetre non au premier plan, ou rayon sans sol), le repli etait le point de depart, donc une ligne de 1 case : "place wall line received ... queue false" puis UNE fondation (Bois -5).
- Aussi : le fantome s'arrete a 8 cases (Y 1800) a l'image 218 alors que le glisser dure 90 images : le curseur perd son sol avant la fin (cause du curseur non elucidee ; a regarder : IsForegroundWindow en capture hors ecran).

## Correctif (C++, NON COMPILE)
EndWallDrag : `FVector Ground = WallDragEnd;` (derniere fin affichee) au lieu de `WallDragStart`. Une ligne de 8 cases au lieu d'une.
Limite : la ligne posee fera 8 cases (pas 16+) tant que le curseur perd son sol avant la fin du glisser.

## A faire (principale)
1. Compiler (`Tools\build.ps1 -LiveCoding=0`), rejouer `preflight.ps1 wall_line_long`, ouvrir wall_line_long_after : attendu journal "wall line (-17420, -1100) to (-17400, ~1800)" et Bois baisse de 5 x cases.
2. Test rouge a ecrire (script ou C++) : appeler EndWallDrag avec GetCursorGround faux et verifier N fondations posees (non fait).
3. Corriger l'en-tete du .txt ("NON JOUABLE" obsolete) et, pour un essai [A ESSAYER], armer par la touche D.

## Champs
APPELS_DECLARES: 22 ; HEURE_DEBUT/FIN: 11:07 / 11:12 ; PREMIER_BUILD: sans C++ (NON COMPILE) ; TEST_ROUGE: non fait ; COMMITS: 1 ; TESTS: preflight wall_line_long PASS (29 s) rejoue une fois, effet FAIL (1 fondation) ; TESTS_TAUTOLOGIQUES: 0 ; ESSAIS: aucun liste ; QUESTIONS: aucune ; NON VERIFIE: correctif non compile, PNG after non reouvert (journal lu, pas de PNG), cause de la perte de sol du curseur ; ANOMALIES: 1er Edit apres le 7e appel (diag de 18 appels).
