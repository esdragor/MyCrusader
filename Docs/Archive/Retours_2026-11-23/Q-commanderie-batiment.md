# [RÉPONDU] Q-commanderie-batiment : dans quel bâtiment se produisent les 3 unités de commanderie des Templiers ?

Reponse: A = bâtiment de l'unité copiée (utilisateur, 2026-11-23 18:1x, interface de choix, D902)

Contexte : les 3 unités du palier 1 (Sergent du Temple = Fer, Frère infirmier = Foi, Arbalétrier à pavois = Pierre) existent et ne peuvent plus être produites que par le joueur qui a choisi leur voie. Le GDD § 13.7 (D133, D683) dit « son bâtiment existant » sans le nommer, et aucun bâtiment ne les liste aujourd'hui (la liste de production est dans `BuildingTypes.csv`, colonne `Produces`, pas dans la table des unités).

Options (effet en jeu) :
- **A. Le bâtiment de l'unité copiée.** Le Sergent sort de la Caserne, le Frère infirmier du bâtiment du Moine (Sanctuaire), l'Arbalétrier à pavois du bâtiment de l'Arbalétrier. En jeu : la case de l'unité apparaît dans un bâtiment que le joueur a déjà, à côté de ses sœurs ; aucun bâtiment à construire de plus ; chaque Templier qui a choisi sa voie voit une case de plus dans 3 bâtiments différents.
- **B. Le Forum.** Les 3 unités sortent toutes du Forum, là où le choix est fait (D684). En jeu : tout est au même endroit, mais le Forum, qui ne produit que des paysans, devient un lieu de recrutement militaire et il est le bâtiment le plus visé en attaque.
- **C. Un bâtiment « commanderie » à créer.** Un bâtiment posé après le choix, qui produit la ou les unités de la voie. En jeu : le plus fidèle à l'idée « commanderie », mais il faut un modèle, un coût, une icône et du C++ de données ; rien n'est jouable avant.

Recommandation : A (D334 : « chaque unité sort du bâtiment de son type » ; c'est la plus cohérente et la moins coûteuse, rien à construire).

Défaut codé : A, donc le filtre de grille sera un simple filtre par voie sur des cases déjà dans les bâtiments existants. NON codé à ce jour : il faut (1) ajouter les 3 unités à la colonne `Produces` des 3 bâtiments (données, fichier hors des fichiers pris), (2) donner à la grille la voie choisie du joueur (champ du contexte de grille : C++).