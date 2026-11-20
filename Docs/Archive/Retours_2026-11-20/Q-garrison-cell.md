# [À RÉPONDRE] Q-garrison-cell : où met-on la case « Sortir tous » de la grille d'un bâtiment garni ?

Contexte : D813 (Q1 C) veut une case de grille « Sortir tous » avec sa touche, comme les autres ordres. Le GDD ne dit ni la case ni la touche ; la grille d'un bâtiment est : A Z E R... production, ligne 2 (Q S D F) spécialisation du Forum, ligne du bas (W X C V) recherches ; la touche d'une case est celle de sa position.

Options :
A. Case 7 (F, ligne 2, dernière colonne) : libre sur tous les bâtiments sauf le Forum Templier (3 voies = cases 5 6 7, mais un Forum n'a pas de garnison au GDD actuel). Visible : un bâtiment garni montre « Sortir » sur F.
B. Case 3 (R) après les unités : cache une production à 4 unités (Caserne : 4 unités A Z E R) : à éviter.
C. Case 11 (V, ligne du bas) : prise par l'annulation d'un chantier (jamais en même temps qu'une garnison) et par la 4e recherche : risque de collision avec les recherches.
D. Autre (donne la touche).

Recommandation : A (F). Valeur par défaut codée : `const int UngarrisonAllCell = 7;` dans `Script\Player\HOLCommandGridRules.as` (une ligne à changer, le test lit `GetUngarrisonAllCell()`).

Reponse: A (2026-11-20 ~16:1x, interface de choix) : la case « Sortir tous » d'un bâtiment garni est la case 7, touche F. Valeur déjà codée par défaut sur `ability-columns` (d3fcb3e), rien à changer dans le code. Enregistré : GDD D876.