# [À RÉPONDRE] Chapelle : à quelle faction appartient-elle ? (ARWEN-1125e)

Contexte : `Tools\Data\BuildingTypes.csv` ligne `Chapel` a `Faction=Dawn` et produit `("Monk","InfirmaryBrother")` ; le Frère infirmier est `Faction=Templars` (UnitTypes.csv) et le Moine `Dawn`.
Depuis ARWEN-1125e, `Factions.csv` a des lignes Templars : `HasFaction(Templars)` est vrai, donc un siège Templars ne se lit plus comme l'Aube (`ResolveKey`) et `IsBuildingOpenTo(Chapel)` devient FAUX pour lui (avant : vrai par accident du repli sur l'Aube). Il peut former le Frère mais ne peut plus poser la Chapelle.

Options :
- A : `Chapel Faction = Dawn;Templars` (les deux factions bâtissent la Chapelle ; le Moine reste filtré par sa propre colonne `Dawn`, le Frère par `Templars`). En jeu : un siège Templars pose la Chapelle et y forme le Frère ; l'Aube la pose et y forme le Moine.
- B : `Chapel Faction = Dawn` inchangé + une Chapelle des Templiers à part (nouveau type, nouveau modèle). En jeu : le Templier a son bâtiment propre ; coût : un type de plus, des données et un visuel à faire.
- C : `Chapel Faction = Templars` (Chapelle propre aux Templiers) ; l'Aube perd la Chapelle et le Moine. À éviter sans autre bâtiment pour le Moine.
- Autre : à préciser.

Recommandation : A (le plus petit changement, rétablit ce que le repli sur l'Aube permettait par accident).
Valeur par défaut codée : AUCUNE (la classe de permission a refusé l'édition de la colonne, qui est une décision de design) ; la ligne à changer est `Tools\Data\BuildingTypes.csv`, ligne `Chapel`, colonne `Faction` (`Dawn` -> `Dawn;Templars`), puis régénérer `Content\Buildings\BuildingTypes.uasset` depuis le CSV (éditeur, file de build vide).
