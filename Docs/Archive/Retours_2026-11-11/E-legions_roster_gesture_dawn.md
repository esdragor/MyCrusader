# [A ESSAYER] E-legions_roster_gesture_dawn : l'Aube (le joueur) garde ses noms d'Aube, le Lancier des Legions ennemi garde son nom Legions

STATUT : VU À L'ÉCRAN (rejoué par l'intégrateur sur main fusionné : 3 PNG ouverts en planche ; Caserne d'Aube : grille Lancier, Homme d'armes, Hallebardes trempées ; Lancier produit par A : bloc « Lancier » ; Lancier des Légions rouge cliqué : bloc « Lancier squelette »)
GESTE: interne   (hol.Select.* : pas de vraie souris ; l'objet de l'essai est l'AFFICHAGE des noms)
SON: sans objet
branches: aucune

Pre-vol (2026-11-11 13:57, agent Glorfindel-26 sans C++, worktree C:\Users\baudo\Documents\HeroesOfLiberty-Glorfindel, main = merge de af56ff8) : `preflight.ps1 legions_roster_gesture_dawn -Root C:\Users\baudo\Documents\HeroesOfLiberty-Glorfindel` PASS (35 s, 3 PNG). WARN : pas de ligne Sanity.
PNG OUVERTS et vus :
- legions_gesture_dawn_barracks : Caserne bleue selectionnee, grille A "Lancier", Z "Homme d'armes" (grise), W "Hallebardes trempees / Bientot" : noms d'Aube
- legions_gesture_dawn_unit : bloc "Lancier" (PV 80, attaque 8, vitesse 7,8 m/s), pop 4/70 ; message moteur "Shaders en preparation (1)" visible a l'ecran au moment de la capture (non lie au jeu)
- legions_gesture_dawn_enemy : unite rouge (IA Normal 2) : bloc "Lancier squelette" PV 80 : nom Legions en ennemi
NON VERIFIE : vrai clic de souris et son (gestes internes) ; l'aspect de la grille de l'Aube hors capture (W "Bientot" est un etat voulu, non juge).

Lancer UNE commande : powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Faramir\Tools\Scenarios\play.ps1 legions_roster_gesture_dawn -Root C:\Users\baudo\Documents\HeroesOfLiberty-Faramir  (rejoue sur le build de main 521adcb fusionne, worktree Faramir ; sans -Hidden = fenetre, reservee a l'utilisateur ; un agent ajoute -Hidden)
Duree : environ 30 s. Reponse attendue : `OK`, `BUG: ...` ou `REGLAGE: ...`.

## Integrateur Faramir 2026-11-11 15:09-15:16, main 521adcb, binaire 15:03 (session_gate -Phase Essay PASS)
Défaut constaté : dans legions_gesture_dawn_unit.png le message moteur « Shaders en préparation (3) » est affiché en haut à gauche (message d'éditeur, absent du jeu empaqueté). Même remarque de gate sur « branches: aucune ». Durée : environ 1 min.
