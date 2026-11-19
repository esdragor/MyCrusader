# AVANT-1119a : captures AVANT (panneaux C++, main, worktree Arwen) du bloc de selection du HUD

Jeu joue : `C:\Users\baudo\Documents\HeroesOfLiberty-Arwen` (gele ; 9 fichiers copies puis supprimes, `git status --short` vide a la fin).
Scenarios d'origine : `C:\Users\baudo\Documents\HeroesOfLiberty-Haldir\Tools\Scenarios\` (branche selection).
PNG AVANT : `C:\Users\baudo\AppData\Local\Temp\claude\82f06dab6ac779ae\7620cd0d-2798-4e7a-b440-a4d2cfd414d9\scratchpad\avant\<scenario>\`

Pre-vol : seul FAIL residuel de chaque scenario = « user's launch : 3 uncommitted file(s) » (attendu : fichiers copies non suivis dans Arwen, pas un defaut du scenario). Tout le reste OK (journal, audit Sanity, pas de recouvrement HUD, un PNG par point).

## ecran_hud_selection_batiment : PASS (1er passage, aucune correction)
- hud_sel_forum_ami : Centre principal, Vous, PV 2400/2400, Vies 2, bouton Paysan.
- hud_sel_fondation : « Fondation : Maison », Vous, PV 1/600, barre 0 %, Batisseurs 0, bouton Annuler.
- hud_sel_chantier : « Chantier : Maison », PV 191/600, barre 31 %, Batisseurs 2.
- hud_sel_arbre : « Arbre », « Reste : 100 » (un rocher passe derriere le coin du bloc, sans gene).
- hud_sel_forum_ennemi : Centre principal, proprietaire « IA Normal 2 » (rouge), PV, Vies, aucune commande.

## ecran_hud_selection_unite : PASS apres 1 correction
- Echec 1er passage : `hol.Select.ClickUnit -1 enemy` rejete par le pre-vol (« write slot=<S> »). Corrige ligne 87 en `slot=1`. Commit Haldir 3390196.
- hud_sel_lancier : Lancier, Vous, PV 80/80, Attaque 8, Armure melee 0, Armure distance 0, Armure heros 0, Vitesse 7,8 m/s (5 puces visibles dans le bloc, pas 6 : a verifier cote script).
- hud_sel_lancier_survol : identique a la precedente (le survol de puce ne se voit pas sur la capture, pas de bulle).
- hud_sel_paysan_recolte : Paysan, 50/50, stats, « Recolte — Bois 2/10 », grille de construction I a IV visible a droite.
- hud_sel_groupe_paysans : puce « Paysan (5) », Vous, « 1 en recolte · 4 inactifs ».
- hud_sel_unite_ennemie : l'unite ennemie cliquee est un **Paysan** « IA Normal 2 » (Recolte — Nourriture 0/10), pas le Lancier pose par le scenario : `ClickUnit -1 slot=1` prend le dernier ennemi de l'index, ici un Paysan de l'IA. Le bloc montre bien un proprietaire ennemi + stats ; non corrige (hors budget), comparaison AVANT/APRES valable tant que l'index reste le meme.

## ecran_hud_selection_puces : PASS (1er passage) + 1 correction de libelle
- hud_puces_tous : « Tous (8) » allume + « Archer (3) », « Homme d'arm. (2) », « Paysan (3) », Vous, « 3 inactifs ».
- hud_puces_archers : « Archer (3) » allume.
- hud_puces_hommes_darmes : « Homme d'arm. (2) » allume.
- hud_puces_batiments_tous : « Tous (4) » allume, « Centre prin. », « Maison (3) », Vous.
- hud_puces_batiments_sous : « Centre prin. » allume (sous-groupe forum seul).
- Correction du `.look` : le libelle reel est « Homme d'arm. (2) » (pas « Homme d'armes »). Commit Haldir 9b8d0cc.

## Etat final
- Arwen : `git status --short` vide (9 fichiers supprimes, PNG hud_*.png du dossier Saved_Tests supprimes).
- stray_processes.ps1 -Stop : aucun processus detache.
- Commits Haldir (branche selection) : 3390196 (slot=1), 9b8d0cc (libelle .look). Pas de push.
- Aucun build, quick_check, ni edition de EtapeDefini.md / Auto_Journey_*.md / IN_USE.txt.
