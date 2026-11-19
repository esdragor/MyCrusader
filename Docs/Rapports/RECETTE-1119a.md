# RECETTE-1119a : fil de messages du HUD, C++ (avant) contre script (apres)

Worktree C:\Users\baudo\Documents\HeroesOfLiberty-Elrond, tip 772367a. Scenario ecran_hud_fil.

## Pre-vol (ligne finale, mot pour mot)
`preflight ecran_hud_fil : PASS (list the trial only once every PNG above has been opened and seen)`
(check ecran_hud_fil : PASS (34 s) ; aucun warning ni erreur au journal ; aucun panneau HUD ne en couvre un autre.)

## Clic sur la ligne du fil, version script : EFFET OUI
Journal Saved_Tests\Logs\scenario_ecran_hud_fil.log :
- l.2246 `LogHOLUI: Display: HUD messages: [Votre forum est tombé — 1 vie]`
- l.2262 `LogHOLUI: Display: HUD: alert of slot 0 clicked (camera on its forum)`
- l.2263 `LogHOLMatch: Display: Camera above the forum of slot 0`
- l.2264 `LogHOLUI: Display: Mouse click MessageLine0 x1 at 1780,131: on SBorder`
- l.2309 `HUD messages: []` (le fil se vide apres 8 s).
Remarque : l.2260 `Camera above the start of slot 1` precede le clic (comportement du scenario, pas du panneau).

## Resolution
Les 8 captures sont en 1920x1080 (memes dimensions avant et apres) : la comparaison vaut.

## Verdicts (zone x 1248-1920, y 0-432, agrandie x2, AVANT a gauche, APRES a droite)
- hud_fil_aucune : IDENTIQUE A L'OEIL (difference de pixels > 30 : 0).
- hud_fil_ligne : IDENTIQUE A L'OEIL. Ligne sombre a bords arrondis sous le tableau, texte "Votre forum est tombé — 1 vie" identique (taille, couleur, marges, position verticale ~y96 et horizontale, largeur ~184 px), fond identique. 64 pixels differents (>30) dans x1661-1676, y136-142 : dans le decor 3D sous la ligne, pas dans le panneau.
- hud_fil_apres_clic : IDENTIQUE A L'OEIL. Meme image que hud_fil_ligne des deux cotes ; 70 pixels differents, bbox x1471-1676 y0-142 : decor 3D (arbre en haut, objet sous la ligne).
- hud_fil_vide : IDENTIQUE A L'OEIL (0 pixel different > 30).

## Anomalies
- Sur les deux versions, hud_fil_apres_clic ressemble a hud_fil_ligne (la camera ne parait pas avoir bouge visiblement dans la zone haut droite) ; sans difference entre avant et apres, ce n'est pas une regression du panneau. Le journal confirme le recentrage camera.
- Aucun processus detache a la fin (stray_processes -Stop).

## Images cote a cote
Dossier : C:\Users\baudo\AppData\Local\Temp\claude\82f06dab6ac779ae\7620cd0d-2798-4e7a-b440-a4d2cfd414d9\scratchpad\compare\ecran_hud_fil\
- hud_fil_aucune.png, hud_fil_ligne.png, hud_fil_apres_clic.png, hud_fil_vide.png
Captures APRES copiees dans ...\scratchpad\apres\ecran_hud_fil\
