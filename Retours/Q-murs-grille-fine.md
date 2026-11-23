# [À RÉPONDRE] Q-murs-grille-fine : sur la grille fine des murs (une case = 4 sous-cases), 3 choses que ta réponse ne dit pas

Contexte (GDD D898) : tu as dit que pour les murs de bois et de pierre une case se divise en 4 cases plus petites, et que dans le tracé seuls le début et la fin comptent. Pour écrire le plan sans deviner, il manque trois réponses ; rien n'est codé tant que tu n'as pas répondu.

## Question 1 : la palissade suit-elle la grille fine ?
Ta phrase dit « mur en pierre ou en bois » ; la palissade est un mur en bois, mais tu ne l'as pas nommée.
- A. Oui : palissade, mur de bois et mur de pierre se posent tous sur la grille fine. En jeu : un seul geste de pose pour tous les murs, la palissade devient 4 fois plus fine qu'aujourd'hui (une case de 400 cm = 2 x 2 sous-cases de 200 cm).
- B. Non : la palissade reste sur la grille actuelle, seuls le mur de bois et le mur de pierre passent à la grille fine. En jeu : deux finesses de pose selon le mur, le joueur doit les distinguer.
Recommandation : A (probable d'après ta phrase, un seul geste pour tous les murs).

## Question 2 : le coût se compte-t-il par sous-case ?
Le Bois d'une palissade coûte aujourd'hui 5 par case (20200 -> 20155 pour 9 fondations à l'essai). Une sous-case est 4 fois plus petite.
- A. Par sous-case, au quart du prix actuel : une ligne de même longueur coûte le même total qu'avant. En jeu : le coût d'une ligne ne change pas, seule la finesse change.
- B. Par sous-case au prix actuel : une ligne de même longueur coûte 4 fois plus. En jeu : les murs deviennent beaucoup plus chers.
- C. Par case entière : une sous-case seule coûte une case. En jeu : poser un petit bout de mur coûte cher.
Recommandation : A (le coût d'une ligne reste celui que tu as vu à l'essai ; la valeur reste à régler en test).

## Question 3 : une sous-case bloquée sur le tracé
C'est la question `Retours\Q-ligne-case-bloquee.md` posée sur la grille fine : le tracé va du début à la fin (tu l'as dit), mais un arbre ou un bâtiment peut se trouver entre les deux.
- A. Refuser toute la ligne (comportement actuel). En jeu : rien n'est posé ni payé, le fantôme rouge montre l'obstacle ; tu retraces.
- B. Poser les sous-cases libres et sauter l'obstacle. En jeu : un trou dans le mur, les sous-cases posées sont payées.
- C. Couper la ligne avant l'obstacle. En jeu : le mur s'arrête devant l'obstacle, la fin du tracé est perdue.
Recommandation : A pour le prototype (rien de payé par surprise) ; c'est aussi la valeur par défaut codée aujourd'hui.

Valeurs par défaut codées en attendant : aucune (aucun code n'est écrit avant le plan, GDD D898). Le reste de ce qui est à préciser (épaisseur et collision d'une sous-case, portes sur la grille fine, alignement sur 8 directions) sera posé en questions au moment du plan.
