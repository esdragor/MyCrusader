# [À RÉPONDRE] Q-ligne-case-bloquee : une case bloquée sur la ligne de palissade refuse-t-elle TOUTE la ligne ?

Contexte : D867 (pas de plafond) et D691 (pose au glisser) ne disent rien du cas d'une case occupée. Le code (HOLWallLineRules.as, Validate, « All or nothing: one refusal and no cell is laid ») refuse TOUTE la ligne dès qu'UNE case est occupée (`refused (3)` = Occupied) : le joueur qui trace sur un arbre ou près d'un bâtiment ne pose rien et ne voit que le fantôme rouge de la case fautive. SAURON-1123pa l'a vu : ligne partant du MainCentre, cellule (-42, -2), aucune fondation.

Options :
- A. Refuser toute la ligne (comportement actuel). En jeu : un seul arbre sur le tracé et rien n'est posé ; le joueur doit retracer ailleurs. Simple, prévisible, aucun coût payé par erreur.
- B. Poser les cases libres et sauter la bloquée. En jeu : la palissade a un trou à l'endroit de l'obstacle (parfois voulu pour passer, souvent un trou dans la défense) ; le joueur paie les cases posées.
- C. Couper la ligne à la première case bloquée. En jeu : la palissade s'arrête devant l'obstacle ; le reste du tracé est perdu, sans trou au milieu.

Recommandation : A pour le prototype (rien de payé par surprise, le fantôme rouge montre déjà la case fautive) ; C si les joueurs trouvent A trop sévère.
Valeur par défaut codée : A (comportement actuel, aucun changement).