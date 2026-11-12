# [À RÉPONDRE] Neutres N7 : le libellé au survol du point de camp sur la minicarte

Contexte : le point de camp sur la minicarte est codé (Q2 B : rouge vivant, gris nettoyé ; Q1 C : seulement les camps déjà vus, branche `d751-root`, agent ARWEN-1112a du 2026-11-12). Le GDD ne nomme aucun libellé de camp, et la minicarte n'a aujourd'hui aucun survol (rien ne s'affiche en passant la souris sur un point, ni pour les unités ni pour les bâtiments).

## Q. Que dit le survol d'un point de camp ?
- **A. Rien pour le prototype** (pas de survol sur la minicarte, comme les unités et les bâtiments). En jeu : la couleur suffit (rouge = camp à prendre, gris = déjà nettoyé) ; aucun code de plus.
- **B. Une infobulle courte** : « Camp neutre : 3 gardes » / « Camp neutre nettoyé » (le nombre vient de ce que le joueur a vu en dernier). En jeu : on sait combien il en reste avant d'y aller ; demande un survol de la minicarte en C++ (le premier de la minicarte) et deux textes fr/en/ja.
- **C. Le survol pour tous les points** (unités, bâtiments, camps, comme les infobulles de la carte d'AoE4 qui n'en a pas sur la minicarte : à inventer). En jeu : le plus complet, mais c'est un chantier d'interface à part.
- **Autre** : à préciser (autre texte, autre forme).

Recommandation : **A** pour le prototype (la minicarte d'AoE4, notre grille, n'a pas de survol ; la couleur porte l'information ; B si un essai montre qu'on veut le nombre de gardes).
Valeur par défaut codée : **A** (aucun survol ; le nombre de gardes vus est déjà transmis au poste du joueur, `FHOLMinimapCamp.Living`, prêt pour B sans toucher au réseau).
