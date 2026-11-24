# [À RÉPONDRE] Q-grille-fine-structure : grille fine des murs (D898 + D904), 2 questions

Contexte : une case de 4 m devient 4 x 4 sous-cases de 1 m ; palissade, mur de bois et mur de pierre se posent par sous-case, au prix actuel par sous-case (D904).
Plan : Docs\Rapports\PLAN-GRILLE-FINE-1124.md. Écrit par SAURON-1124a le 2026-11-24 ; aucune réponse attendue pour continuer (les étapes G1 à G3 ne dépendent ni de A ni de B).

## Question A : une pièce = un acteur, ou un segment par ligne ?
- A : chaque sous-case est un bâtiment (acteur répliqué) : réutilise toute la chaîne (coût, construction, PV, attaque, navmesh) ; une ligne de 94 cases = 376 acteurs. En jeu : murs détruits et réparés pièce par pièce, trous naturels.
- B : un seul acteur par ligne posée, qui contient ses pièces : très peu d'acteurs ; chantier bien plus gros (construction, PV et destruction par pièce à réécrire).
- C : pièces par défaut (A), bascule vers B seulement si la mesure G9 dépasse un seuil à fixer.
- Autre : à préciser.
Recommandation : C (A d'abord, mesure ensuite). Valeur par défaut codée : A (aucune ligne de code à changer : c'est la structure actuelle).

## Question B : PV et durée de construction PAR SOUS-CASE
D904 fixe le prix (5 Bois la sous-case de palissade). Les lignes CSV `Palisade` (400 PV, 5 s), `StoneWall` (1200 PV, 8 s) ne disent rien de la sous-case.
- A : lignes inchangées : un mètre de palissade a 400 PV et se construit en 5 s ; une ligne de même longueur est 4 fois plus solide et 4 fois plus longue à bâtir.
- B : PV et durée divisés par 4 : une ligne de même longueur garde la solidité et la durée d'aujourd'hui, seul le prix est multiplié par 4 (comme D904).
- Autre : à préciser.
Recommandation : A pour G1 à G5, puis régler avec la mesure G9. Valeur par défaut codée : A (lignes de Tools\Data\BuildingTypes.csv inchangées, une ligne à changer par type).
