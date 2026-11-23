# [À RÉPONDRE] Mur de pierre : chiffres (F10, S3-b)

Contexte : le GDD donne le Mur de pierre (D691 : palier 1, grille § 11.1 ; D692 : seulement les dégâts de siège ; D24 : praticable, en dernier) mais aucun chiffre. CELEBORN-1123l a posé une ligne de table provisoire `StoneWall` (BuildingTypes.csv) et l'a listée dans `HOLWallLine::WallTypes()` (tracé en ligne comme la palissade). La Porte de bois ne remplace QUE la palissade (la Porte de pierre est un autre bâtiment, D691, non faite).

Question (une seule, regroupée) : quels chiffres pour le mur de pierre ?

- A (défaut codé) : 1 case, 1200 PV, 10 pierre (0 bois), 8 s, palier 1 (panneau II), emplacement de grille 7 (touche F), non praticable.
- B : comme A mais plus lent et plus solide (2000 PV, 15 pierre, 12 s) : un mur qui demande du siège pour tomber.
- C : comme A mais mêlée et tir ne le touchent pas du tout (D692 : colonne de famille d'attaque, non branchée aujourd'hui).
- Autre : à préciser (PV, prix, durée, touche).

Conséquences en jeu : A rend le mur de pierre trois fois plus solide que la palissade pour 10 pierre la case ; B ralentit la pose ; C n'a d'effet qu'une fois la colonne de familles d'attaque codée.

Recommandation : A (défaut).

Valeur par défaut codée : ligne `StoneWall` de `Tools\Data\BuildingTypes.csv` (PV, pierre, durée, emplacement de grille : une ligne à changer).
