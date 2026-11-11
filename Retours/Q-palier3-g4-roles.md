# [À RÉPONDRE] Palier 3 G4 : comment une faction remplace ou retire un rôle (Canon, Arquebusier)

**Contexte.** Aujourd'hui une faction remplace ou retire une unité par la colonne `Faction` de `UnitTypes.csv` (`Dawn`, `Legions`, `Dawn;Fire`, `Common`) : `HOLFactionRules::FilterProduces` (grille) et `IsUnitOpenTo` (serveur) filtrent la liste `Produces` du bâtiment ; une faction sans règle propre lit comme l'Aube. Il n'y a pas de table de rôles pour les unités de bâtiment (seuls Ouvrier, Héros, Centre ont des rôles dans `Factions.csv`).
Le *Cracheur de bile* des Légions et la *Bombarde* des Héritiers n'existent pas encore (le Canon non plus, G8) : on ne peut tester le remplacement du Canon qu'avec une ligne de substitution de test.

**Options**
- **A (codée, défaut)** : on garde la colonne `Faction` ; une variante de faction = une ligne d'unité avec sa `Faction` (ex. `Legions`), placée dans le `Produces` du même bâtiment. Aucun nouveau fichier. Conséquence en jeu : la grille du Canon (Atelier) montre l'unité de la faction du joueur, rien d'autre ; la même case (touche) est réutilisée seulement si on l'ordonne dans `Produces`. L'IA filtre par la même colonne.
- **B** : une table de rôles `Cannon` / `Gunner` dans `Factions.csv` (Dawn_Cannon -> Cannon, Legions_Cannon -> BileSpitter, Legions_Gunner -> vide) et une case de grille par rôle (même touche pour toutes les factions). Conséquence : une touche stable par rôle et un ordre garanti, mais un second mécanisme à maintenir à côté de `Faction`, et du C++ en plus (grille, IA, serveur) avant le premier jeu visible.
- **C** : A maintenant, B seulement si les variantes de Légions / Héritiers / Templiers montrent un besoin de touche stable.
- **Autre** : à préciser.

**Recommandation : C** (A maintenant). Le mécanisme existe, est testé côté serveur et grille, et G4 n'a de vrai trou que le filtre de l'IA (corrigé dans le lot). B n'apporte rien de visible avant G8 et les variantes.

**Valeur par défaut codée** : A (aucun réglage à changer ; passer à B = nouvelle étape).
