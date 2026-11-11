# [A ESSAYER] E-legions_roster_gesture_dawn : l'Aube (le joueur) garde ses noms d'Aube, le Lancier des Legions ennemi garde son nom Legions

STATUT : VU A L'ECRAN (objet de l'essai = l'AFFICHAGE des noms ; clics par hol.Select.ClickGround / ClickUnit, durcis en 798d1d3)
GESTE: interne      (commandes hol.Select.* : pas de vraie souris)
SON: sans objet
branches: aucune

Pre-vol (2026-11-11 13:57, agent Glorfindel-26 sans C++, worktree C:\Users\baudo\Documents\HeroesOfLiberty-Glorfindel, main = merge de af56ff8) : `preflight.ps1 legions_roster_gesture_dawn -Root C:\Users\baudo\Documents\HeroesOfLiberty-Glorfindel` PASS (35 s, 3 PNG). WARN : pas de ligne Sanity.
PNG OUVERTS et vus :
- legions_gesture_dawn_barracks : Caserne bleue selectionnee, grille A "Lancier", Z "Homme d'armes" (grise), W "Hallebardes trempees / Bientot" : noms d'Aube
- legions_gesture_dawn_unit : bloc "Lancier" (PV 80, attaque 8, vitesse 7,8 m/s), pop 4/70 ; message moteur "Shaders en preparation (1)" visible a l'ecran au moment de la capture (non lie au jeu)
- legions_gesture_dawn_enemy : unite rouge (IA Normal 2) : bloc "Lancier squelette" PV 80 : nom Legions en ennemi
NON VERIFIE : vrai clic de souris et son (gestes internes) ; l'aspect de la grille de l'Aube hors capture (W "Bientot" est un etat voulu, non juge).

Lancer UNE commande : powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Glorfindel\Tools\Scenarios\play.ps1 legions_roster_gesture_dawn -Root C:\Users\baudo\Documents\HeroesOfLiberty-Glorfindel
Duree : environ 30 s. Reponse attendue : `OK`, `BUG: ...` ou `REGLAGE: ...`.
