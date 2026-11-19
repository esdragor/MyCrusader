# GALADRIEL-1119r : pre-vol ecran_hud_triche (script, galadriel-tab 44395a9)

- Pre-vol : PASS au 1er passage ("preflight ecran_hud_triche : PASS"), aucune correction de scenario, aucun commit (sha = 44395a9, inchange). Seul WARN : `frames 60` ligne 8 du .txt (attente par images, sans effet).
- Journal : "HUD overlap: none" x3 ; `HUD cheats:` ecrit au 1er Report (apres 1er Refresh) : "cheats folded" puis "cheats open, fast build off, unit speed x1.0, gather speed x1.0" puis "cheats open, fast build on, unit speed x2.0, gather speed x1.0" (la derniere = ligne exacte attendue). Rect panneau plie 16,70,230,115 ; ouvert 16,70,232,440 ; aucune erreur Angelscript/LogHOL.
- Les doutes de l'auteur ne se sont pas realises : MakePanel place bien le panneau sous la barre ; clics sur boutons OK.

## PNG (Saved_Tests\Screenshots\WindowsEditor\HOL\, copies dans scratchpad\apres\ecran_hud_triche\)
- triche_pliee.png : seul bouton "Triches  +" dans un panneau sombre arrondi, sous la barre du haut a gauche (16,70) ; barre : Nourriture/Bois/Pierre/Or 5200.
- triche_ouverte.png : panneau ouvert, bouton "Triches  -", 7 boutons ("+1000 Nourriture", "+1000 Bois", "+1000 Pierre", "+1000 Or", "+1000 de tout", "Population +165", "Construction rapide : non") puis curseurs "Vitesse des unites : x1.0" et "Vitesse de collecte : x1.0" (curseur a gauche). Pas de ligne niveau IA visible sous le panneau (le .look en annonce une empilee ; seul le tableau des joueurs "IA Normal 2" en haut a droite apparait) : a verifier si l'ancien C++ la montrait ici.
- triche_nourriture.png : Nourriture 5200 -> 6200, reste inchange, panneau ouvert.
- triche_construction_rapide.png : bouton dit "Construction rapide : oui" ; ressources 6200/5200 ; point rouge supplementaire sur la minicarte (IA).
- triche_vitesse.png : curseur unites avance (x2.0), libelle "Vitesse des unites : x2.0", collecte x1.0 ; construction rapide oui.

## Ecarts du panneau
Aucun bloquant. Notes : (1) "Construction rapide : non" a 27 px du bord gauche, texte presque a la limite du bouton (largeur 190 px) ; (2) ligne niveau IA absente, voir ci-dessus ; (3) etat "Paysans immobiles" non pose (non teste).

## quick_check -Parts smoke,camera,production
sequence PASS, smoke PASS, camera 2p PASS, camera 8p PASS, production PASS ; "quick_check : PASS in 38 s".
