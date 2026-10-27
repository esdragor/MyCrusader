# Stats des unités

Produit par `python Tools/Data/make_unit_stats.py` à partir de `Tools/Data/UnitTypes.csv` (la source de l'asset `/Game/Units/UnitTypes`, D227) et des constantes de `HOLCombatRules.h`. Ne pas modifier à la main : changer le CSV, puis relancer le script. Valeurs provisoires, à équilibrer. Distances et vitesses en cases (une case = 4 m, D344), en mètres entre parenthèses.

## Combat

| Unité | PV | Attaque | Bonus | Cadence | Coups / min | Dégâts / s (sans bonus ni armure) | Élan du 1er coup | Type d'attaque | Portée | Armure mêlée | Armure distance | Armure siège | Armure héros | Catégories d'armure | Rayon d'agressivité |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| Infanterie légère | 100 | 8 | — | 1,875 s | 32 | 4,27 | 50 % (0,938 s) | Mêlée | 0,12 case (0,5 m) | 0 | 0 | 0 | 0 | Infanterie légère | 3 cases (12 m) |
| Infanterie légère (essai Nanite) | 100 | 8 | — | 1,875 s | 32 | 4,27 | 50 % (0,938 s) | Mêlée | 0,12 case (0,5 m) | 0 | 0 | 0 | 0 | Infanterie légère | 3 cases (12 m) |
| Archer | 70 | 5 | +5 contre Infanterie légère, +5 contre Infanterie à poudre | 1,625 s | 36,9 | 3,08 | 12 % (0,195 s) | Distance (tir) | 5 cases (20 m) | 0 | 0 | 0 | 0 | Distance | 5,33 cases (21,33 m) |
| Paysan | 50 | 3 | — | 3,7 s | 16,2 | 0,81 | 50 % (1,85 s) | Mêlée | 0,29 case (1,16 m) | 0 | 0 | 0 | 0 | Paysan | aucun (seulement sur ordre) |
| Piquier | 80 | 8 | +17 contre Cavalerie | 1,875 s | 32 | 4,27 | 50 % (0,938 s) | Mêlée | 0,33 case (1,3 m) | 0 | 0 | 0 | 0 | Infanterie légère | 4 cases (16 m) |
| Homme d'armes | 155 | 12 | — | 1,375 s | 43,6 | 8,73 | 50 % (0,688 s) | Mêlée | 0,17 case (0,7 m) | 4 | 4 | 0 | 2 | Infanterie lourde, Lourd | 4 cases (16 m) |
| Roi | 250 | 50 | — | 1,375 s | 43,6 | 36,36 | 50 % (0,688 s) | Mêlée | 0,17 case (0,7 m) | 0 | 0 | 0 | 0 | Roi | aucun (riposte seulement) |
| Arbalétrier | 80 | 12 | +10 contre Lourd | 2,125 s | 28,2 | 5,65 | 12 % (0,255 s) | Distance (tir) | 5 cases (20 m) | 0 | 0 | 0 | 0 | Distance | 5,33 cases (21,33 m) |

- Dégâts d'un coup = attaque + chaque bonus visant une des catégories de la cible (ils s'additionnent) − armure de la cible du type de l'attaque (mêlée, distance, siège ou héros), 1 au moins.
- Cadence : temps entre deux coups. Élan : part de la cadence qu'attend le premier coup sur une cible.
- Portée : entre les bords des deux corps. Au-delà de 0,5 case (2 m), les coups sont des tirs (flèche à 37,5 m/s, qui touche toujours) ; l'armure retirée vient du type d'attaque.
- Rayon d'agressivité : riposte et prise de cible automatiques, de bord à bord. Sur attaque-déplacement, une unité qui n'en a pas cherche à 4 cases (16 m).
- Au plus 6 attaquants de mêlée vont sur une même cible hors de portée ; les autres en prennent une moins entourée.
- Une unité à 0 PV reste 20 pas (1 s à 20 Hz) pour sa chute.

## Attaque contre bâtiment

| Unité | Attaque | Type | Dégâts par coup (armure distance des bâtiments retirée) | Cadence | Dégâts / s | Portée (bord du corps au bâtiment) |
|---|---|---|---|---|---|---|
| Infanterie légère | — | — | — | — | — | — |
| Archer | 5 | Distance | 1 | 1,625 s | 0,62 | 5 cases (20 m) |
| Paysan | 10 | Mêlée | 10 | 2,1 s | 4,76 | 1 case (4 m) |
| Piquier | 20 | Mêlée | 20 | 2,1 s | 9,52 | 1 case (4 m) |
| Homme d'armes | 20 | Mêlée | 20 | 2,1 s | 9,52 | 1 case (4 m) |
| Roi | — | — | — | — | — | — |
| Arbalétrier | 12 | Distance | 1 | 2,125 s | 0,47 | 5 cases (20 m) |

- Contre un bâtiment, l'unité se sert toujours de cette attaque (D345). Les bâtiments ont 40 d'armure distance, rien contre les autres types ; 1 dégât au moins.
- Cibles : sur ordre (clic droit), en attaque-déplacement quand aucune unité ennemie n'est à portée de recherche, au repos dans le rayon d'agressivité sans unité ennemie (le Paysan seulement sur ordre) ; une unité ennemie qui arrive fait lâcher le bâtiment.

## Déplacement et corps

| Unité | Vitesse | Rayon du corps | Distance d'évitement |
|---|---|---|---|
| Infanterie légère | 0,75 case/s (3 m/s) | 0,15 case (0,6 m) | 0,62 case (2,5 m) |
| Infanterie légère (essai Nanite) | 0,75 case/s (3 m/s) | 0,15 case (0,6 m) | 0,62 case (2,5 m) |
| Archer | 1,95 case/s (7,79 m/s) | 0,13 case (0,52 m) | 0,55 case (2,2 m) |
| Paysan | 2 cases/s (8 m/s) | 0,15 case (0,6 m) | 0,62 case (2,5 m) |
| Piquier | 1,94 case/s (7,77 m/s) | 0,15 case (0,6 m) | 0,62 case (2,5 m) |
| Homme d'armes | 1,74 case/s (6,97 m/s) | 0,15 case (0,6 m) | 0,62 case (2,5 m) |
| Roi | 1,94 case/s (7,77 m/s) | 0,15 case (0,6 m) | 0,62 case (2,5 m) |
| Arbalétrier | 1,8 case/s (7,2 m/s) | 0,13 case (0,52 m) | 0,55 case (2,2 m) |

## Production et économie

| Unité | Coût | Temps de production | Population | Récolte | Charge portée |
|---|---|---|---|---|---|
| Infanterie légère | — | 20 s | 1 | — | — |
| Infanterie légère (essai Nanite) | — | 20 s | 1 | — | — |
| Archer | 30 Nourriture + 50 Bois | 15 s | 1 | — | — |
| Paysan | 50 Nourriture | 20 s | 1 | Nourriture 0,7/s, Bois 0,7/s, Pierre 0,7/s, Or 0,7/s | 10 |
| Piquier | 60 Nourriture + 20 Bois | 15 s | 1 | — | — |
| Homme d'armes | 100 Nourriture + 20 Or | 22 s | 1 | — | — |
| Roi | — | 1 s | 0 | — | — |
| Arbalétrier | 80 Nourriture + 40 Or | 22 s | 1 | — | — |

## Affrontements

Ligne = l'attaquant, colonne = la cible. Chaque case : dégâts d'un coup · coups pour tuer · temps pour tuer une cible seule et immobile (élan du premier coup, puis un coup par cadence ; sans le vol de la flèche).

| Attaquant \ Cible | Infanterie légère | Archer | Paysan | Piquier | Homme d'armes | Roi | Arbalétrier |
|---|---|---|---|---|---|---|---|
| **Infanterie légère** | 8 · 13 · 23,4 s | 8 · 9 · 15,9 s | 8 · 7 · 12,2 s | 8 · 10 · 17,8 s | 4 · 39 · 72,2 s | 8 · 32 · 59,1 s | 8 · 10 · 17,8 s |
| **Archer** | 10 · 10 · 14,8 s | 5 · 14 · 21,3 s | 5 · 10 · 14,8 s | 10 · 8 · 11,6 s | 1 · 155 · 250,4 s | 5 · 50 · 79,8 s | 5 · 16 · 24,6 s |
| **Paysan** | 3 · 34 · 124 s | 3 · 24 · 87 s | 3 · 17 · 61,1 s | 3 · 27 · 98 s | 1 · 155 · 571,7 s | 3 · 84 · 309 s | 3 · 27 · 98 s |
| **Piquier** | 8 · 13 · 23,4 s | 8 · 9 · 15,9 s | 8 · 7 · 12,2 s | 8 · 10 · 17,8 s | 4 · 39 · 72,2 s | 8 · 32 · 59,1 s | 8 · 10 · 17,8 s |
| **Homme d'armes** | 12 · 9 · 11,7 s | 12 · 6 · 7,6 s | 12 · 5 · 6,2 s | 12 · 7 · 8,9 s | 8 · 20 · 26,8 s | 12 · 21 · 28,2 s | 12 · 7 · 8,9 s |
| **Roi** | 50 · 2 · 2,1 s | 50 · 2 · 2,1 s | 50 · 1 · 0,7 s | 50 · 2 · 2,1 s | 46 · 4 · 4,8 s | 50 · 5 · 6,2 s | 50 · 2 · 2,1 s |
| **Arbalétrier** | 12 · 9 · 17,3 s | 12 · 6 · 10,9 s | 12 · 5 · 8,8 s | 12 · 7 · 13 s | 18 · 9 · 17,3 s | 12 · 21 · 42,8 s | 12 · 7 · 13 s |

Les lignes d'essai (Infanterie légère (essai Nanite)) sont laissées de côté : elles se battent comme le type qu'elles copient.
