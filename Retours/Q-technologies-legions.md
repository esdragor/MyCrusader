# [À RÉPONDRE] Technologies des Légions : dans quel bâtiment se cherchent-elles ?

Contexte : le GDD § 11.1 donne quatre technologies propres aux Légions (*Os taillés*, *Chair putride* au palier 1 ; *Rites funèbres*, *Lames empoisonnées* au palier 2) mais, contrairement à l'Aube (D583), ne dit pas où on les cherche. `Technologies.csv` n'a aucune ligne `Legions` ; la colonne `Building` de chaque ligne dit le bâtiment. Les Légions n'ont pas de Chapelle ; le Zombie est le seul type d'unité des Légions déjà dans le jeu (L1) ; la Nécropole (D338) n'existe pas encore. Aucune ligne n'est ajoutée tant que la réponse manque.

Options :
- **A (recommandée, valeur par défaut)** : *Os taillés* et *Chair putride* à la **Caserne** (comme *Hallebardes trempées* de l'Aube, D583) ; *Rites funèbres* à la **Nécropole** (le bâtiment du Nécromancien, D338, dont le soin dépend) ; *Lames empoisonnées* à la **Caserne** (le Guerrier Damné s'y entraîne, D394/D395). Conséquence visible : la grille de la Caserne propose deux ou trois lignes, la Nécropole une ; tant que la Nécropole manque, *Rites funèbres* reste grisée « Bientôt » (colonne `Implemented`, D601).
- B : les quatre à la **Caserne**. Une seule grille à regarder, rien ne dépend de la Nécropole ; mais le Nécromancien n'a pas « sa » technologie à son bâtiment.
- C : les quatre à la **Nécropole** (tout le « savoir noir » au même endroit). Rien à rechercher avant que la Nécropole existe (palier 1 en prototype), donc *Os taillés* arriverait plus tard que l'Aube n'a *Liturgie*.
- Autre : un autre bâtiment (Forge commune, Ossuaire du palier 3...).

Recommandation : A (une technologie à l'endroit où se fabrique l'unité qu'elle améliore, comme l'Aube).

Valeur par défaut : A, codée derrière un réglage d'une ligne : la colonne `Building` des quatre lignes `Legions` de `Technologies.csv` (à ajouter par le chantier Légions L2/L4, `LegionsInProgress.md` ligne « lieu de recherche : à fixer »).

Journal : `TechnologiesInProgress.md` (T9, 2026-11-10).
