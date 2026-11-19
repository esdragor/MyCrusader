# AVANT-1119b : captures AVANT (panneaux C++, jeu d'Arwen = main) de fil, grille, triche

Date 2026-11-19. Jeu joue : C:\Users\baudo\Documents\HeroesOfLiberty-Arwen (gele, aucun fichier suivi touche). Un seul jeu a la fois.
PNG AVANT : C:\Users\baudo\AppData\Local\Temp\claude\82f06dab6ac779ae\7620cd0d-2798-4e7a-b440-a4d2cfd414d9\scratchpad\avant\<scenario>\
Seul FAIL de chaque pre-vol : "3 uncommitted file(s)" (attendu, fichiers copies). Tous les autres controles OK (sanity compris).

## 1. ecran_hud_fil
- Pre-vol passage 1 : check FAIL, timeout au pas 36 `waitlog HUD: alert of slot 0 clicked`. Cause : `hol.HUD.Click MessageFeed` repond "hidden, greyed or unknown" sur le C++ (le widget MessageFeed n'existe que dans la version script ; le survol de MessageLine0 marche, 1780,131).
- Correction : `hol.HUD.Click MessageFeed` remplace par `hol.Mouse.Click MessageLine0` (clic reel, meme widget que le survol), commit **772367a** dans le worktree **Elrond** (branche `tiers`), fichier Tools/Scenarios/ecran_hud_fil.txt seul. A confirmer cote script : la commande Mouse.Click MessageLine0 doit aussi declencher le clic sur la version script (non rejoue sur script ici).
- Pre-vol passage 2 : PASS (31 s), tous les controles OK.
- hud_fil_aucune.png : NON OUVERT par moi (pas de budget d'appels), a ouvrir.
- hud_fil_ligne.png : haut droite, sous le tableau des joueurs (Vous 1 vie Ruine / IA Normal 2 2 vies), UNE ligne a fond sombre a bords legerement arrondis, texte gris clair "Votre forum est tombé — 1 vie" en petit ; PAS de nom de joueur en couleur dans la ligne (le seul "Vous" bleu est dans le tableau des joueurs au-dessus) ; la banniere d'alerte du meme texte est en haut centre. Le panneau de triche est ouvert en haut gauche (ouvert par defaut ici).
- hud_fil_apres_clic.png : NON OUVERT, a ouvrir.
- hud_fil_vide.png : plus de ligne sous le tableau des joueurs, tableau seul en haut droite ; camera sur le forum en ruine.

## 2. ecran_hud_grille (Sauron, jamais joue avant)
- Pre-vol passage 1 : PASS (28 s), AUCUNE correction : `hol.Mouse.Hover GridCell00` et `hol.Mouse.Click GridCell00` sans coordonnees marchent (infobulle apparue, clic = "queued Peasant (1 in" vu, un Paysan en file). Le rouvrage du panneau de notes par `note` n'a pas gene (les 6 captures sont propres, pas de panneau de notes). Seul WARN : `frames 40` (> 30).
- La grille est en BAS A GAUCHE (a droite du bloc d'info), pas en bas a droite.
- hud_grille_cachee.png : NON OUVERT, a ouvrir.
- hud_grille_paysan.png : bloc d'info "Paysan (5)" ; grille 3x4 avec A Z E R / Q S D F / W X C V en petit badge en haut a gauche de chaque case, cases sombres sauf S "Arret" et V "Tenir" claires, pas de Q ; a droite le menu de construction par ages (I A Maison, Z Camp de bucherons, E Camp de mineurs, R Moulin, Q Caserne, S Champ de tir, W Ferme, X Sanctuaire de l'Aube ; II/III/IV grises).
- hud_grille_forum.png : "Centre principal" ; grille : A "Paysan" allumee, W "Cloche d'alarme amelioree" avec le mot "Bientot" COUPE en bas de la case (texte qui deborde), le reste sombre.
- hud_grille_grisee.png : ressources a 0 ; case A "Paysan" attenuee (texte gris terne), meme place.
- hud_grille_infobulle.png : case A surlignee brun/dore ; infobulle "Paysan / pastille nourriture 50 / Recolte et batit" juste sous-droite de la case, recouvre les cases Q et S voisines.
- hud_grille_clic.png : panneau de production "Paysan" avec barre de progression et "Clic : annuler" au-dessus de la grille ; nourriture 4950 ; infobulle toujours visible, case A surlignee.

## 3. ecran_hud_triche (Galadriel, recopie juste avant de jouer)
- Pre-vol passage 1 : PASS (28 s), aucune correction. Seul WARN : `frames 60`.
- triche_pliee.png : haut gauche sous la barre de ressources, un seul bouton "Triches  +" dans un panneau sombre a bords arrondis (16..222 px de large environ, y 70..115).
- triche_ouverte.png : panneau ouvert, bouton du haut "Triches  -", puis colonne de 7 boutons : +1000 Nourriture, +1000 Bois, +1000 Pierre, +1000 Or, +1000 de tout, Population +165, "Construction rapide : non" ; puis "Vitesse des unites : x1.0" et "Vitesse de collecte : x1.0" avec curseur (poignee doree a gauche). Panneau de 16 a 224 px, y 70 a 440. Le tableau des joueurs reste en haut droite.
- triche_nourriture, triche_construction_rapide, triche_vitesse : NON OUVERTS, a ouvrir.

## Etat d'Arwen
Fichiers copies (fil, grille, triche : .txt .look .json) supprimes par nom ; `git status --short` vide (voir sortie de la session). stray_processes.ps1 -Stop : "aucun processus detache".
