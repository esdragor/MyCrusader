# RECETTE-1118c : écran Nouvelle partie, AVANT (C++) contre APRÈS (script), même résolution

Agent indépendant, lecture seule (rien lancé). Début 22:01, fin avant 22:18. Je n'ai pas écrit cet écran.

## Contrôles préalables (PowerShell, 22:01)
- AVANT : np_avant (scratchpad), 5 fichiers du 18/11/2026 20:44:24 à 20:44:26 (pré-vol de 20:44).
- APRÈS : Sauron\Saved_Tests\Screenshots\WindowsEditor\HOL, 5 fichiers du 18/11/2026 22:00:44 à 22:00:47 (pré-vol PASS de 22:00).
- Résolution : les 10 PNG s'affichent en 1066 x 600 (même cadre, même échelle).
- Empreintes SHA-256 (8 premiers caractères) et taille en octets : IDENTIQUES pour les 5 paires.
  - ecran_np_ouverture 4D999A74 (45375 o), ecran_np_ia_niveau 1ECC4D04 (60060 o), ecran_np_couleur CD9CCBAF (73038 o),
    ecran_np_taille_liste 3D5F8856 (86407 o), ecran_np_retour 01C5F713 (63995 o).
  - Chaque paire est donc identique octet pour octet : zéro pixel d'écart, y compris notes jaunes et décor.
- Dates différentes (20:44 / 22:00), donc ce ne sont pas deux copies du même fichier : deux rendus distincts qui donnent le même PNG.

## Lecture des 10 PNG contre le .look (ecran_nouvelle_partie.look)
Les 10 captures ont été ouvertes (outil Read) et comparées élément par élément ; la comparaison visuelle confirme l'égalité des empreintes.

### Paire 1 : ecran_np_ouverture : identique
- Titre « Nouvelle partie » centré (y 85). Ligne du haut : Carte = Plaine, Taille = Petite (2 joueurs), Brouillard = Sans brouillard, Équipes ensemble = Oui.
- En-têtes Joueur / Niveau / Faction / Couleur / Équipe en italique gris.
- Place 1 « Vous » (Ordre de l'Aube, pastille bleue, Bleu, équipe 1) ; place 2 « IA Normal 2 », liste Normal grisée, Ordre de l'Aube, pastille rouge, Rouge, équipe 2 ; places 3 à 8 « Fermé » grisées, sans niveau/faction/couleur/équipe.
- Ligne du bas : Mode Standard actif, Nomade / Régicide / Catastrophe grisés « Non », Sites sacrés « Non » actif. Note « Grisé : pas encore dans le jeu ... » centrée. Retour et Lancer (cadre doré de focus sur Lancer). Rien de coupé, pas d'erreur.
- Conforme au .look.

### Paire 2 : ecran_np_ia_niveau : identique
- Taille = Moyenne (4 joueurs). Places 2 et 3 « IA Normal 2 / 3 » avec liste Normal grisée, pastilles Bleu / Rouge / Vert, équipes 1 / 2 / Aucune. Place 4 « Fermé » actif (chevron visible), places 5 à 8 grisées. Bas, note et boutons identiques. Notes jaunes identiques (1 seule, tronquée au bord droit dans les deux).
- Conforme au .look (place 3 en Vert, Taille Moyenne).

### Paire 3 : ecran_np_couleur : identique
- Places 2, 3, 4 sur IA avec niveau Normal grisé ; pastilles Bleu, Rouge, Jaune, Turquoise toutes différentes ; place 4 a sauté à Turquoise ; équipes 1 / 2 / Aucune / Aucune ; focus (fond brun-doré, cadre clair) sur la liste de couleur de la place 4 ; places 5 à 8 grisées ; bas et boutons identiques.
- Conforme au .look. La ligne du haut est masquée par les notes jaunes dans les DEUX séries (même superposition, donc pas un écart).

### Paire 4 : ecran_np_taille_liste : identique
- Liste Taille ouverte sous le champ (x 179-461, de y 168 à 337) : « — » (gris clair), « Petite (2 joueurs) » grisée, « Moyenne (4 joueurs) » (entrée courante, cadre clair/blanc), Normale, Grande, Immense en clair. Pas de 28 px entre entrées. Champ Taille en brun-doré. Lancer sans focus. Les cellules Joueur / Niveau / Faction des places 1 à 4 sont masquées par la liste de la même façon.
- Conforme au .look (Petite grisée, Moyenne et suivantes en clair).

### Paire 5 : ecran_np_retour : identique
- Écran titre « Heroes of Liberty » (doré), Jouer (cadre doré), Multijoueur, Réglages, Quitter ; boutons x 377-690 dans les deux ; étiquette « dev » en bas à droite ; notes jaunes et note cyan « Entrée : j'ai manipulé la grille » identiques.
- Conforme au .look.

## Sort des deux réserves de RECETTE-1118b
1. Cadre de la liste Taille déroulée / entrée « Moyenne » : NON, pas d'inversion. Les deux PNG sont octet pour octet identiques : dans les deux séries le cadre extérieur de la liste est doré (brun-doré) et l'entrée « Moyenne » est cernée d'un trait clair. La réserve venait de la comparaison entre deux résolutions (1280 x 720 contre 1066 x 600). LEVÉE.
2. Écran titre : étiquette « dev » et largeur des boutons : NON, aucun écart. « dev » est droite dans les deux, boutons à 377-690 dans les deux. L'italique et les 2 % de largeur venaient du changement de résolution. LEVÉE.

## Verdict global : IDENTIQUE
Aucun écart de pixel, de comportement, de grisé, de texte ni de couleur sur les 5 paires à résolution égale.

## Ce que cette recette ne prouve pas
- Un PNG ne dit pas quelle implémentation l'a dessiné : que l'APRÈS vienne bien de l'écran en script repose sur le pré-vol PASS de 22:00 (non relu ici). L'égalité octet pour octet est cohérente avec une reproduction exacte de la mise en page.
- Le salon réseau à deux instances, le clavier / la manette, les autres tailles et cartes, Lancer jusqu'à la carte, les bascules de liste (clic sur une ligne grisée, changement de faction) ne sont pas dans ces 5 captures.
