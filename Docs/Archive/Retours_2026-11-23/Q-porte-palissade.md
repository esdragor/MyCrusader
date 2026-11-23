# [À RÉPONDRE] Porte de palissade : chiffres et passage (F9, S3-b)

Contexte : le GDD dit seulement que murs et portes sont solides (§ 197-240) ; aucun chiffre de porte. CELEBORN-1123j a posé une ligne de table provisoire `PalisadeGate` (BuildingTypes.csv) et la règle script `HOLWallLine::GateReplaces` / `GatePassesUnit` (une porte remplace une section de palissade posée du même camp ; elle laisse passer les unités du propriétaire).

Question (une seule, regroupée) : quel statut pour la porte de palissade ?

- A (défaut codé) : 1 case, 400 PV, 10 bois, 8 s, emplacement de grille 10 (touche C), passage pour les unités du propriétaire SEULEMENT ; une porte ne se glisse pas en ligne, elle se pose une à une sur une section de palissade existante.
- B : comme A mais les alliés passent aussi (règle de diplomatie : camps alliés).
- C : comme A mais plus chère et plus solide (par ex. 20 bois, 800 PV), pour qu'elle soit le point faible choisi d'un mur.
- Autre : à préciser (PV, prix, durée, touche, passage).

Conséquences en jeu : A garde la porte aussi fragile que la palissade ; B ouvre les bases aux alliés ; C rend l'attaque d'une porte rentable pour l'assaillant.

Recommandation : A (défaut), B à reprendre quand la diplomatie des unités au pathfinding sera tranchée.

Valeur par défaut codée : ligne `PalisadeGate` de `Tools\Data\BuildingTypes.csv` (PV, bois, durée, emplacement de grille : une ligne à changer) ; passage = `HOLWallLine::GatePassesUnit` (`Script\Buildings\HOLWallLineRules.as`, une ligne). Le branchement du passage au pathfinding / à la collision (C++ ou Mass) n'est PAS fait : plan seulement, voir FortificationsInProgress.md.
