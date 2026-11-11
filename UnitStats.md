# Stats des unités

Produit par `python Tools/Data/make_unit_stats.py` à partir de `Tools/Data/UnitTypes.csv` (la source de l'asset `/Game/Units/UnitTypes`, D227) et des constantes de `HOLCombatRules.h`. Ne pas modifier à la main : changer le CSV, puis relancer le script. Valeurs provisoires, à équilibrer. Distances et vitesses en cases (une case = 4 m, D344), en mètres entre parenthèses.

## Combat

| Unité | PV | Attaque | Bonus | Cadence | Coups / min | Dégâts / s (sans bonus ni armure) | Élan du 1er coup | Type d'attaque | Portée | Armure mêlée | Armure distance | Armure siège | Armure héros | Catégories d'armure | Rayon d'agressivité |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| Infanterie légère | 100 | 8 | — | 1,875 s | 32 | 4,27 | 50 % (0,938 s) | Mêlée | 0,12 case (0,5 m) | 0 | 0 | 0 | 0 | Infanterie légère | 3 cases (12 m) |
| Infanterie légère (essai Nanite) | 100 | 8 | — | 1,875 s | 32 | 4,27 | 50 % (0,938 s) | Mêlée | 0,12 case (0,5 m) | 0 | 0 | 0 | 0 | Infanterie légère | 3 cases (12 m) |
| Archer | 70 | 5 | +5 contre Infanterie légère, +5 contre Infanterie à poudre | 1,625 s | 36,9 | 3,08 | 12 % (0,195 s) | Distance (tir) | 5 cases (20 m) | 0 | 0 | 0 | 0 | Distance | 5,33 cases (21,33 m) |
| Paysan | 50 | 3 | — | 3,7 s | 16,2 | 0,81 | 50 % (1,85 s) | Mêlée | 0,29 case (1,16 m) | 0 | 0 | 0 | 0 | Paysan | aucun (seulement sur ordre) |
| Zombie | 35 | 3 | — | 3,7 s | 16,2 | 0,81 | 50 % (1,85 s) | Mêlée | 0,29 case (1,16 m) | 0 | 0 | 0 | 0 | Paysan, Profane | aucun (seulement sur ordre) |
| Piquier | 80 | 8 | +17 contre Cavalerie | 1,875 s | 32 | 4,27 | 50 % (0,938 s) | Mêlée | 0,33 case (1,3 m) | 0 | 0 | 0 | 0 | Infanterie légère | 4 cases (16 m) |
| Homme d'armes | 155 | 12 | — | 1,375 s | 43,6 | 8,73 | 50 % (0,688 s) | Mêlée | 0,17 case (0,7 m) | 4 | 4 | 0 | 2 | Infanterie lourde, Lourd | 4 cases (16 m) |
| Damned Warrior | 130 | 16 | — | 1,5 s | 40 | 10,67 | 50 % (0,75 s) | Mêlée | 0,17 case (0,7 m) | 2 | 2 | 0 | 1 | Infanterie lourde, Lourd, Profane | 4 cases (16 m) |
| Roi | 250 | 50 | — | 1,375 s | 43,6 | 36,36 | 50 % (0,688 s) | Mêlée | 0,17 case (0,7 m) | 0 | 0 | 0 | 0 | Roi | aucun (riposte seulement) |
| Paladin-Commandant (héros) | 450 | 30 | — | 1,25 s | 48 | 24 | 50 % (0,625 s) | Héros | 0,17 case (0,7 m) | 0 | 0 | 0 | 0 | Héros | 4 cases (16 m) |
| Arbalétrier | 80 | 12 | +10 contre Lourd | 2,125 s | 28,2 | 5,65 | 12 % (0,255 s) | Distance (tir) | 5 cases (20 m) | 0 | 0 | 0 | 0 | Distance | 5,33 cases (21,33 m) |
| Moine | 80 | 0 | — | 1,875 s | 32 | 0 | 50 % (0,938 s) | Mêlée | 0,33 case (1,3 m) | 0 | 0 | 0 | 0 | Infanterie légère | aucun (seulement sur ordre) |
| Bélier | 400 | 0 | — | 2,5 s | 24 | 0 | 50 % (1,25 s) | Siège | 0,12 case (0,5 m) | 0 | 60 | 0 | 0 | Siège | aucun (seulement sur ordre) |
| Mangonneau | 200 | 45 | — | 5 s | 12 | 9 | 50 % (2,5 s) | Siège (tir) | 7,5 cases (30 m) | 0 | 60 | 0 | 0 | Siège | aucun (seulement sur ordre) |
| Cavalier léger | 120 | 9 | +10 contre Distance, +20 contre Siège | 1,5 s | 40 | 6 | 50 % (0,75 s) | Mêlée | 0,17 case (0,7 m) | 1 | 2 | 0 | 0 | Cavalerie | 5 cases (20 m) |
| Arquebusier | 70 | 40 | — | 4 s | 15 | 10 | 12 % (0,48 s) | Distance (tir) | 3,75 cases (15 m) | 0 | 0 | 0 | 0 | Distance, Infanterie à poudre | 4,08 cases (16,33 m) |

- Dégâts d'un coup = attaque + chaque bonus visant une des catégories de la cible (ils s'additionnent) − armure de la cible du type de l'attaque (mêlée, distance, siège ou héros), 1 au moins.
- Cadence : temps entre deux coups. Élan : part de la cadence qu'attend le premier coup sur une cible.
- Portée : entre les bords des deux corps. Au-delà de 0,5 case (2 m), les coups sont des tirs (flèche à 37,5 m/s, qui touche toujours) ; l'armure retirée vient du type d'attaque.
- Rayon d'agressivité : riposte et prise de cible automatiques, de bord à bord. Sur attaque-déplacement, une unité qui n'en a pas cherche à 4 cases (16 m).
- Siège (S2, D614, D615) : le Bélier n'attaque que des bâtiments (attaque contre unités nulle) ; le Mangonneau fait des dégâts de zone de 0,62 case (2,5 m) de rayon sur les unités (rayon : colonne AreaRadius), un coup unique sur un bâtiment. Armures, temps de production et vue de ces deux lignes : provisoires (SQ3).
- Soin (Q-chapelle-moine = A, GDD 7.1) : le Moine n'attaque pas ; il soigne 5 PV par seconde un allié blessé à la fois, à 2 cases (8 m) (colonnes HealRate et HealRange), jamais lui-même, jamais le Roi (D471) ni un engin de siège. Temps de production : provisoire.
- Cavalier léger (D672, CV-Q3 : stats validées en bloc, à retoucher à l'essai) : cheval provisoire (Mannequin agrandi, comme le Bélier). Sa charge (deuxième attaque à déclencheur, D345.3) n'est pas encore dans la table : les colonnes, le calcul et le marais viennent avec C2 (C++), donc les dégâts ci-dessus sont ceux du coup normal.
- Au plus 6 attaquants de mêlée vont sur une même cible hors de portée ; les autres en prennent une moins entourée.
- Une unité à 0 PV reste 20 pas (1 s à 20 Hz) pour sa chute.

## Attaque contre bâtiment

| Unité | Attaque | Type | Dégâts par coup (armure distance des bâtiments retirée) | Cadence | Dégâts / s | Portée (bord du corps au bâtiment) |
|---|---|---|---|---|---|---|
| Infanterie légère | — | — | — | — | — | — |
| Archer | 5 | Distance | 1 | 1,625 s | 0,62 | 5 cases (20 m) |
| Paysan | 10 | Mêlée | 10 | 2,1 s | 4,76 | 1 case (4 m) |
| Zombie | 10 | Mêlée | 10 | 2,1 s | 4,76 | 1 case (4 m) |
| Piquier | 20 | Mêlée | 20 | 2,1 s | 9,52 | 1 case (4 m) |
| Homme d'armes | 20 | Mêlée | 20 | 2,1 s | 9,52 | 1 case (4 m) |
| Damned Warrior | 25 | Mêlée | 25 | 2,1 s | 11,9 | 1 case (4 m) |
| Roi | — | — | — | — | — | — |
| Paladin-Commandant (héros) | 10 | Mêlée | 10 | 2,1 s | 4,76 | 1 case (4 m) |
| Arbalétrier | 12 | Distance | 1 | 2,125 s | 0,47 | 5 cases (20 m) |
| Moine | — | — | — | — | — | — |
| Bélier | 60 | Siège | 60 | 2,5 s | 24 | 1 case (4 m) |
| Mangonneau | 45 | Siège | 45 | 5 s | 9 | 7,5 cases (30 m) |
| Cavalier léger | 10 | Mêlée | 10 | 2,1 s | 4,76 | 1 case (4 m) |
| Arquebusier | 40 | Distance | 1 | 4 s | 0,25 | 3,75 cases (15 m) |

- Contre un bâtiment, l'unité se sert toujours de cette attaque (D345). Les bâtiments ont 40 d'armure distance, rien contre les autres types ; 1 dégât au moins.
- Cibles : sur ordre (clic droit), en attaque-déplacement quand aucune unité ennemie n'est à portée de recherche, au repos dans le rayon d'agressivité sans unité ennemie (le Paysan seulement sur ordre) ; une unité ennemie qui arrive fait lâcher le bâtiment.

## Déplacement et corps

| Unité | Vitesse | Rayon du corps | Distance d'évitement | Vue |
|---|---|---|---|---|
| Infanterie légère | 0,75 case/s (3 m/s) | 0,15 case (0,6 m) | 0,62 case (2,5 m) | 8 cases (32 m) |
| Infanterie légère (essai Nanite) | 0,75 case/s (3 m/s) | 0,15 case (0,6 m) | 0,62 case (2,5 m) | 8 cases (32 m) |
| Archer | 1,95 case/s (7,79 m/s) | 0,13 case (0,52 m) | 0,55 case (2,2 m) | 10 cases (40 m) |
| Paysan | 2 cases/s (8 m/s) | 0,15 case (0,6 m) | 0,62 case (2,5 m) | 8 cases (32 m) |
| Zombie | 2 cases/s (8 m/s) | 0,15 case (0,6 m) | 0,62 case (2,5 m) | 8 cases (32 m) |
| Piquier | 1,94 case/s (7,77 m/s) | 0,15 case (0,6 m) | 0,62 case (2,5 m) | 8 cases (32 m) |
| Homme d'armes | 1,74 case/s (6,97 m/s) | 0,15 case (0,6 m) | 0,62 case (2,5 m) | 8 cases (32 m) |
| Damned Warrior | 1,85 case/s (7,4 m/s) | 0,15 case (0,6 m) | 0,62 case (2,5 m) | 8 cases (32 m) |
| Roi | 1,74 case/s (6,97 m/s) | 0,15 case (0,6 m) | 0,62 case (2,5 m) | 8 cases (32 m) |
| Paladin-Commandant (héros) | 1,74 case/s (6,97 m/s) | 0,18 case (0,72 m) | 0,62 case (2,5 m) | 12 cases (48 m) |
| Arbalétrier | 1,8 case/s (7,2 m/s) | 0,13 case (0,52 m) | 0,55 case (2,2 m) | 10 cases (40 m) |
| Moine | 1,94 case/s (7,77 m/s) | 0,15 case (0,6 m) | 0,62 case (2,5 m) | 8 cases (32 m) |
| Bélier | 0,55 case/s (2,2 m/s) | 0,38 case (1,5 m) | 1 case (4 m) | 8 cases (32 m) |
| Mangonneau | 0,45 case/s (1,8 m/s) | 0,45 case (1,8 m) | 1,12 case (4,5 m) | 10 cases (40 m) |
| Cavalier léger | 2,19 cases/s (8,75 m/s) | 0,23 case (0,9 m) | 0,75 case (3 m) | 10 cases (40 m) |
| Arquebusier | 1,8 case/s (7,2 m/s) | 0,13 case (0,52 m) | 0,55 case (2,2 m) | 10 cases (40 m) |

- Vue : rayon du disque que l'unité découvre dans le brouillard (D383, D523), du centre de sa case ; la forêt la coupe derrière sa lisière. Au-delà de sa portée et de son rayon d'agressivité d'une case au moins.

## Production et économie

| Unité | Coût | Temps de production | Population | Récolte | Charge portée |
|---|---|---|---|---|---|
| Infanterie légère | — | 20 s | 1 | — | — |
| Infanterie légère (essai Nanite) | — | 20 s | 1 | — | — |
| Archer | 30 Nourriture + 50 Bois | 15 s | 1 | — | — |
| Paysan | 50 Nourriture | 20 s | 1 | Nourriture 0,7/s, Bois 0,7/s, Pierre 0,7/s, Or 0,7/s | 10 |
| Zombie | — | 20 s | 1 | Nourriture 0,49/s, Bois 0,49/s, Pierre 0,49/s, Or 0,49/s | 10 |
| Piquier | 60 Nourriture + 20 Bois | 15 s | 1 | — | — |
| Homme d'armes | 100 Nourriture + 20 Or | 22 s | 1 | — | — |
| Damned Warrior | 90 Nourriture + 25 Or | 24 s | 1 | — | — |
| Roi | — | 1 s | 0 | — | — |
| Paladin-Commandant (héros) | — | 1 s | 0 | — | — |
| Arbalétrier | 80 Nourriture + 40 Or | 22 s | 1 | — | — |
| Moine | 100 Nourriture + 100 Or | 20 s | 1 | — | — |
| Bélier | 150 Bois + 75 Or | 30 s | 3 | — | — |
| Mangonneau | 200 Bois + 150 Or | 40 s | 4 | — | — |
| Cavalier léger | 80 Nourriture + 40 Or | 20 s | 1 | — | — |
| Arquebusier | 60 Nourriture + 80 Or | 25 s | 1 | — | — |

## Affrontements

Ligne = l'attaquant, colonne = la cible. Chaque case : dégâts d'un coup · coups pour tuer · temps pour tuer une cible seule et immobile (élan du premier coup, puis un coup par cadence ; sans le vol de la flèche).

| Attaquant \ Cible | Infanterie légère | Archer | Paysan | Zombie | Piquier | Homme d'armes | Damned Warrior | Roi | Paladin-Commandant (héros) | Arbalétrier | Moine | Bélier | Mangonneau | Cavalier léger | Arquebusier |
|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|---|
| **Infanterie légère** | 8 · 13 · 23,4 s | 8 · 9 · 15,9 s | 8 · 7 · 12,2 s | 8 · 5 · 8,4 s | 8 · 10 · 17,8 s | 4 · 39 · 72,2 s | 6 · 22 · 40,3 s | 8 · 32 · 59,1 s | 2,4 · 188 · 351,6 s | 8 · 10 · 17,8 s | 8 · 10 · 17,8 s | 8 · 50 · 92,8 s | 8 · 25 · 45,9 s | 7 · 18 · 32,8 s | 8 · 9 · 15,9 s |
| **Archer** | 10 · 10 · 14,8 s | 5 · 14 · 21,3 s | 5 · 10 · 14,8 s | 5 · 7 · 9,9 s | 10 · 8 · 11,6 s | 1 · 155 · 250,4 s | 3 · 44 · 70,1 s | 5 · 50 · 79,8 s | 1,5 · 300 · 486,1 s | 5 · 16 · 24,6 s | 10 · 8 · 11,6 s | 1 · 400 · 648,6 s | 1 · 200 · 323,6 s | 3 · 40 · 63,6 s | 10 · 7 · 9,9 s |
| **Paysan** | 3 · 34 · 124 s | 3 · 24 · 87 s | 3 · 17 · 61,1 s | 3 · 12 · 42,6 s | 3 · 27 · 98 s | 1 · 155 · 571,7 s | 1 · 130 · 479,2 s | 3 · 84 · 309 s | 0,9 · 501 · 1851,8 s | 3 · 27 · 98 s | 3 · 27 · 98 s | 3 · 134 · 494 s | 3 · 67 · 246,1 s | 2 · 60 · 220,2 s | 3 · 24 · 87 s |
| **Zombie** | 3 · 34 · 124 s | 3 · 24 · 87 s | 3 · 17 · 61,1 s | 3 · 12 · 42,6 s | 3 · 27 · 98 s | 1 · 155 · 571,7 s | 1 · 130 · 479,2 s | 3 · 84 · 309 s | 0,9 · 501 · 1851,8 s | 3 · 27 · 98 s | 3 · 27 · 98 s | 3 · 134 · 494 s | 3 · 67 · 246,1 s | 2 · 60 · 220,2 s | 3 · 24 · 87 s |
| **Piquier** | 8 · 13 · 23,4 s | 8 · 9 · 15,9 s | 8 · 7 · 12,2 s | 8 · 5 · 8,4 s | 8 · 10 · 17,8 s | 4 · 39 · 72,2 s | 6 · 22 · 40,3 s | 8 · 32 · 59,1 s | 2,4 · 188 · 351,6 s | 8 · 10 · 17,8 s | 8 · 10 · 17,8 s | 8 · 50 · 92,8 s | 8 · 25 · 45,9 s | 24 · 5 · 8,4 s | 8 · 9 · 15,9 s |
| **Homme d'armes** | 12 · 9 · 11,7 s | 12 · 6 · 7,6 s | 12 · 5 · 6,2 s | 12 · 3 · 3,4 s | 12 · 7 · 8,9 s | 8 · 20 · 26,8 s | 10 · 13 · 17,2 s | 12 · 21 · 28,2 s | 3,6 · 125 · 171,2 s | 12 · 7 · 8,9 s | 12 · 7 · 8,9 s | 12 · 34 · 46,1 s | 12 · 17 · 22,7 s | 11 · 11 · 14,4 s | 12 · 6 · 7,6 s |
| **Damned Warrior** | 16 · 7 · 9,8 s | 16 · 5 · 6,8 s | 16 · 4 · 5,2 s | 16 · 3 · 3,8 s | 16 · 5 · 6,8 s | 12 · 13 · 18,8 s | 14 · 10 · 14,2 s | 16 · 16 · 23,2 s | 4,8 · 94 · 140,2 s | 16 · 5 · 6,8 s | 16 · 5 · 6,8 s | 16 · 25 · 36,8 s | 16 · 13 · 18,8 s | 15 · 8 · 11,2 s | 16 · 5 · 6,8 s |
| **Roi** | 50 · 2 · 2,1 s | 50 · 2 · 2,1 s | 50 · 1 · 0,7 s | 50 · 1 · 0,7 s | 50 · 2 · 2,1 s | 46 · 4 · 4,8 s | 48 · 3 · 3,4 s | 50 · 5 · 6,2 s | 15 · 30 · 40,6 s | 50 · 2 · 2,1 s | 50 · 2 · 2,1 s | 50 · 8 · 10,3 s | 50 · 4 · 4,8 s | 49 · 3 · 3,4 s | 50 · 2 · 2,1 s |
| **Paladin-Commandant (héros)** | 30 · 4 · 4,4 s | 30 · 3 · 3,1 s | 30 · 2 · 1,9 s | 30 · 2 · 1,9 s | 30 · 3 · 3,1 s | 28 · 6 · 6,9 s | 29 · 5 · 5,6 s | 30 · 9 · 10,6 s | 30 · 15 · 18,1 s | 30 · 3 · 3,1 s | 30 · 3 · 3,1 s | 30 · 14 · 16,9 s | 30 · 7 · 8,1 s | 30 · 4 · 4,4 s | 30 · 3 · 3,1 s |
| **Arbalétrier** | 12 · 9 · 17,3 s | 12 · 6 · 10,9 s | 12 · 5 · 8,8 s | 12 · 3 · 4,5 s | 12 · 7 · 13 s | 18 · 9 · 17,3 s | 20 · 7 · 13 s | 12 · 21 · 42,8 s | 3,6 · 125 · 263,8 s | 12 · 7 · 13 s | 12 · 7 · 13 s | 1 · 400 · 848,1 s | 1 · 200 · 423,1 s | 10 · 12 · 23,6 s | 12 · 6 · 10,9 s |
| **Moine** | 1 · 100 · 186,6 s | 1 · 70 · 130,3 s | 1 · 50 · 92,8 s | 1 · 35 · 64,7 s | 1 · 80 · 149,1 s | 1 · 155 · 289,7 s | 1 · 130 · 242,8 s | 1 · 250 · 467,8 s | 0,3 · 1501 · 2813,4 s | 1 · 80 · 149,1 s | 1 · 80 · 149,1 s | 1 · 400 · 749,1 s | 1 · 200 · 374,1 s | 1 · 120 · 224,1 s | 1 · 70 · 130,3 s |
| **Bélier** | 1 · 100 · 248,8 s | 1 · 70 · 173,8 s | 1 · 50 · 123,8 s | 1 · 35 · 86,2 s | 1 · 80 · 198,8 s | 1 · 155 · 386,2 s | 1 · 130 · 323,8 s | 1 · 250 · 623,8 s | 0,3 · 1501 · 3751,2 s | 1 · 80 · 198,8 s | 1 · 80 · 198,8 s | 1 · 400 · 998,8 s | 1 · 200 · 498,8 s | 1 · 120 · 298,8 s | 1 · 70 · 173,8 s |
| **Mangonneau** | 45 · 3 · 12,5 s | 45 · 2 · 7,5 s | 45 · 2 · 7,5 s | 45 · 1 · 2,5 s | 45 · 2 · 7,5 s | 45 · 4 · 17,5 s | 45 · 3 · 12,5 s | 45 · 6 · 27,5 s | 13,5 · 34 · 167,5 s | 45 · 2 · 7,5 s | 45 · 2 · 7,5 s | 45 · 9 · 42,5 s | 45 · 5 · 22,5 s | 45 · 3 · 12,5 s | 45 · 2 · 7,5 s |
| **Cavalier léger** | 9 · 12 · 17,2 s | 19 · 4 · 5,2 s | 9 · 6 · 8,2 s | 9 · 4 · 5,2 s | 9 · 9 · 12,8 s | 5 · 31 · 45,8 s | 7 · 19 · 27,8 s | 9 · 28 · 41,2 s | 2,7 · 167 · 249,8 s | 19 · 5 · 6,8 s | 9 · 9 · 12,8 s | 29 · 14 · 20,2 s | 29 · 7 · 9,8 s | 8 · 15 · 21,8 s | 19 · 4 · 5,2 s |
| **Arquebusier** | 40 · 3 · 8,5 s | 40 · 2 · 4,5 s | 40 · 2 · 4,5 s | 40 · 1 · 0,5 s | 40 · 2 · 4,5 s | 36 · 5 · 16,5 s | 38 · 4 · 12,5 s | 40 · 7 · 24,5 s | 12 · 38 · 148,5 s | 40 · 2 · 4,5 s | 40 · 2 · 4,5 s | 1 · 400 · 1596,5 s | 1 · 200 · 796,5 s | 38 · 4 · 12,5 s | 40 · 2 · 4,5 s |

Les lignes d'essai (Infanterie légère (essai Nanite)) sont laissées de côté : elles se battent comme le type qu'elles copient.
