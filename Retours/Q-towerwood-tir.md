# [À RÉPONDRE] Tour de guet en bois : dégâts, portée, cadence de tir

Contexte : D878 fixe PV, coût, vision, garnison de TowerWood mais laisse « portée de tir, dégâts et cadence » à part. Aucune de ces valeurs n'est dans le GDD ; le tir de bâtiment n'existe pas encore dans le code (ELROND-1121sg).

Valeurs de départ proposées, par rapport à l'Archer (UnitTypes.csv : dégâts 5, cadence 1.625 s, portée 2000, vision 4000) :

- **A (recommandée)** : dégâts 5, cadence 1.625 s, portée 3000 (une tour voit 4800, tire plus loin qu'un archer), par flèche ; tir de base sans garnison. La garnison (3 places) multiplie le nombre de flèches (1 + 1 par occupant, à confirmer à part).
- **B** : comme A mais la tour ne tire QUE si elle est garnie (une flèche par occupant).
- **C** : dégâts 8, cadence 2 s, portée 3500, tir de base sans garnison (tour plus lourde, moins fréquente).
- **Autre** : tes valeurs.

Conséquences en jeu : A donne à une tour vide un rôle de défense légère (lancier seul qui s'en approche perd des PV) ; B oblige à garnir (plus de micro) ; C punit plus les petits groupes mais se contourne en restant hors de portée.

Recommandation : A. Valeur par défaut à coder : colonnes d'attaque de `Tools\Data\BuildingTypes.csv` (`BuildingAttack`, `BuildingAttackInterval`, `BuildingAttackRange`, noms à fixer dans le lot C++), ligne `TowerWood` seulement : une ligne à changer, aucun code.
