# Q-terrain-formation : la forêt et un bloc de formation mixte (HALDIR-1117f, 2026-11-17)

Contexte : D810 est branché au Mass pour les marcheurs isolés (ordre de déplacement : catégorie pied / cavalerie / siège, forêt F4, gué F5).
Le bloc de formation (GetTerrainShare du bloc, HOLUnitOrderProcessor, ~l.2597) ne tient compte que du marais : un bloc mélange des unités à pied
et de la cavalerie, il n'a donc pas de catégorie unique, et la forêt n'y ralentit personne aujourd'hui (incohérence visible : une cavalerie isolée est
ralentie en forêt, la même cavalerie en formation ne l'est pas).

Question : quelle vitesse la forêt donne-t-elle à un bloc mixte ?

- Option A : le bloc marche à la vitesse du MEMBRE LE PLUS LENT de sa composition (la plus petite part de vitesse de ses catégories présentes : un
  bloc avec du siège en forêt = 70, avec de la cavalerie seule = 80, tout à pied = 100). Cohérent avec « le plus lent gagne » (marais + forêt) et avec
  la cohésion d'un bloc. Coût : calculer la plus petite part sur les catégories présentes, une seule fois par bloc et par pas.
- Option B : le bloc ignore la forêt (marais seul, comme aujourd'hui) ; seules les unités isolées sont ralenties. Aucun code neuf, mais la forêt
  ne ralentit plus la cavalerie dès qu'elle marche en bloc (contourne F4).

Recommandation : A (le bloc suit son membre le plus lent), car B vide F4 de son sens pour la cavalerie, la seule unité visée avec le siège.

Rien n'est fait dans ce bloc tant que la question n'est pas tranchée.
