# RECETTE-1119c : grille de commandes 3x4 en script (worktree Sauron, envelope afd9c4a), 2026-11-19

## Pre-vols (lignes finales mot pour mot)
- ecran_hud_grille : `preflight ecran_hud_grille : PASS (list the trial only once every PNG above has been opened and seen)` (check ecran_hud_grille : PASS (31 s), tous les OK : aucun avertissement, aucun panneau ne en couvre un autre, 6 rapports HUD rects)
- peasant_grid : `preflight peasant_grid : FAIL (1)` ; check peasant_grid : PASS (26 s) ; le seul FAIL : `no peasant_grid.look: declare each point to look at as '<capture name> | <what the user must see>'` (fichier .look absent du scenario : pas un defaut du panneau)
- tech_grid : `preflight tech_grid : FAIL (2)` ; check tech_grid : PASS (37 s) ; FAIL : `map: tech_grid.json has no mapSize, the trial plays on the grey test map (D820: add mapSize Small to the .json and replay; -TestMap if the test map is wanted)` et `no tech_grid.look: declare each point to look at as '<capture name> | <what the user must see>'` (conventions de scenario, pas le panneau)
- Les deux lignes "engine smoke tests: 20 'Condition failed'" sont declarees non comptees par le pre-vol.

## Comparaison AVANT (C++) / APRES (script), zone x 0-1100, y 640-1080, echelle 1, AVANT a gauche, APRES a droite
Toutes les paires sont en 1920x1080. Ecart de pixels (> 30 de somme RVB, echantillon 1 sur 4 de la zone) : 0,16 a 0,36 % (decor 3D / herbe), rien de localise sur la grille.
- hud_grille_cachee : IDENTIQUE A L'OEIL (emplacement vide des deux cotes)
- hud_grille_paysan : IDENTIQUE A L'OEIL (S Arret et V Tenir allumes, autres cases sombres, lettres de touche, libelles, grille de construction a droite coupee de la meme facon)
- hud_grille_forum : IDENTIQUE A L'OEIL (A Paysan allumee, W "Cloche d'alarme amelioree / Bientot" grise, coupee en bas de la case des deux cotes)
- hud_grille_grisee : IDENTIQUE A L'OEIL (A Paysan attenuee, badge A present)
- hud_grille_infobulle : IDENTIQUE A L'OEIL (titre Paysan, pastille de cout 50, "Recolte et batit", meme place et meme taille ; case A en surbrillance)
- hud_grille_clic : IDENTIQUE A L'OEIL (panneau de production avec Paysan, barre, "Clic : annuler", grille et infobulle inchangees)

## Observation hors panneau (identique des deux cotes, donc pas une regression)
Le libelle de la case W du forum ("Cloche d'alarme amelioree / Bientot") deborde en bas de la case (coupe) AVANT comme APRES.

## Images cote a cote
C:\Users\baudo\AppData\Local\Temp\claude\82f06dab6ac779ae\7620cd0d-2798-4e7a-b440-a4d2cfd414d9\scratchpad\compare\ecran_hud_grille\hud_grille_cachee.png, hud_grille_paysan.png, hud_grille_forum.png, hud_grille_grisee.png, hud_grille_infobulle.png, hud_grille_clic.png (meme dossier). PNG APRES : ...\scratchpad\apres\ecran_hud_grille\.
