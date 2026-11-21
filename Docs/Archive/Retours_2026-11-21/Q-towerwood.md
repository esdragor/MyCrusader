# [À RÉPONDRE] Tour de guet en bois (TowerWood, F8) : chiffres de la ligne de table

Contexte : le GDD (D691, D693, D694, D793) donne seulement le palier 0, 3 places de garnison, non praticable, bois (mêlée et siège la détruisent, pas le tir). PV, taille, coût, temps, case de grille et vision sont « à proposer » (FortificationsInProgress.md F8) : la ligne `TowerWood` de `Tools\Data\BuildingTypes.csv` (commit 482e26b, branche integ-25) porte une PROPOSITION.

Valeur par défaut codée (une ligne du CSV à changer, puis `regen_tables.ps1`) : empreinte 2 x 2 cases (800 x 800), hauteur 800, 800 PV, 100 bois, 20 s, case 7 de la grille du palier 0, vision 4800 (la tour éclaire plus loin qu'une Maison, 2000, et que la Caserne, 3200), catégorie Militaire, 3 places.

## Question : quels chiffres pour la Tour de guet en bois ?

- **A (par défaut, recommandé)** : 2 x 2 cases, 800 PV, 100 bois, 20 s, vision 4800. Fragile (moins qu'une Caserne, 1500 PV) : une escouade de mêlée la fait tomber vite, ce qui garde la valeur du mur de pierre du palier 1 ; coût d'un Camp de bois pour pouvoir en poser plusieurs.
- **B** : 3 x 3 cases (comme la Forge), 1500 PV, 150 bois, 25 s, vision 4800. Plus massive et plus chère : une seule tour est un vrai point fort, on en pose peu.
- **C** : 2 x 2 cases, 500 PV, 75 bois, 15 s, vision 4800. Très bon marché et très cassante : de la tour d'alerte plutôt que de la défense.
- **Autre** : donne tes chiffres (empreinte, PV, bois, temps, vision).

Conséquences visibles : A est entre les deux ; la portée de tir, les dégâts et la cadence (colonnes d'attaque, C++ de F8) se règlent à part et ne sont pas dans cette question.

Autre point signalé (pas une question) : la case de grille 7 est prise provisoirement ; si la Palissade (F5) ou le Centre secondaire veulent la case 7 du palier 0, une des deux lignes change de case (une cellule du CSV).

Recommandation : **A**.
Reponse: A (2026-11-21 14:09, interface de choix)
