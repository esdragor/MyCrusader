# [À RÉPONDRE] Technologies des Légions : dans quel bâtiment se cherchent-elles ?

Contexte : le GDD § 11.1 donne quatre technologies propres aux Légions mais, contrairement à l'Aube (D583), ne dit pas où on les cherche. `Technologies.csv` n'a aucune ligne `Legions` ; la colonne `Building` de chaque ligne dit le bâtiment. Les Légions n'ont pas de Chapelle ; la Nécropole (D338) n'existe pas encore. Aucune ligne n'est ajoutée tant que la réponse manque.

Ce que font les quatre technologies (GDD § 11.1) :
- *Os taillés* (palier 1) : les Squelettes gagnent de l'armure.
- *Chair putride* (palier 1) : les Zombies gagnent des PV.
- *Rites funèbres* (palier 2) : chaque levée d'un Nécromancien soigne les Squelettes proches d'environ 10 % de leurs PV (D317).
- *Lames empoisonnées* (palier 2) : les Guerriers Damnés infligent des dégâts sur la durée.

D'où sortent les unités (LegionsInProgress.md) : le **Zombie** remplace le Paysan, c'est le travailleur des Légions (D66) ; le **Squelette** n'est produit dans aucun bâtiment, un Nécromancien lève 1 cadavre en 1 Squelette (LQ-10) ; le **Guerrier Damné** s'entraîne à la Caserne (D395). Le Zombie se produit au **Forum** (le Centre principal) : confirmé par l'utilisateur le 2026-11-10.

Options :
- **A (recommandée, valeur par défaut)** : *Os taillés* et *Rites funèbres* à la **Nécropole** (le bâtiment du Nécromancien, D338, qui lève les Squelettes) ; *Chair putride* au **Forum** (là où se produisent les Zombies, travailleurs des Légions, comme *Milice de défense* de l'Aube au Centre principal) ; *Lames empoisonnées* à la **Caserne** (le Guerrier Damné s'y entraîne). Conséquence visible : une technologie à l'endroit de l'unité ou de la capacité qu'elle améliore ; tant que la Nécropole manque, *Os taillés* et *Rites funèbres* restent grisées « Bientôt » (colonne `Implemented`, D601).
- B : les quatre à la **Caserne**. Une seule grille à regarder, rien ne dépend de la Nécropole ; mais ni les Squelettes ni les Zombies n'en sortent, et le Nécromancien n'a pas « sa » technologie à son bâtiment.
- C : les quatre à la **Nécropole** (tout le « savoir noir » au même endroit). Rien à rechercher avant que la Nécropole existe (palier 1 en prototype), donc *Os taillés* arriverait plus tard que l'Aube n'a *Liturgie*.
- D : *Os taillés* et *Chair putride* à la **Caserne** (comme *Hallebardes trempées* de l'Aube, D583), *Rites funèbres* à la **Nécropole**, *Lames empoisonnées* à la **Caserne**. Ancienne recommandation, abandonnée le 2026-11-10 : elle supposait que la Caserne fabrique les Squelettes et les Zombies, ce que LegionsInProgress.md ne dit pas.
- Autre : un autre bâtiment (Forge commune, Ossuaire du palier 3...).

Recommandation : A (une technologie à l'endroit où se fabrique l'unité, ou la capacité, qu'elle améliore).

Valeur par défaut : A, codée derrière un réglage d'une ligne : la colonne `Building` des quatre lignes `Legions` de `Technologies.csv` (à ajouter par le chantier Légions L2/L4, `LegionsInProgress.md` ligne « lieu de recherche : à fixer »). Le Zombie sort du Forum (confirmé 2026-11-10), donc *Chair putride* y est cohérente.

Journal : `TechnologiesInProgress.md` (T9, 2026-11-10).
