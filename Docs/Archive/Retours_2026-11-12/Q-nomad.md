# [RÉPONDU 2026-11-12] Option Nomade : taille des groupes de départ et distance aux ennemis (NomadInProgress.md, HALDIR-1112g, 2026-11-12)

Contexte : en Nomade, les Paysans de départ sont « dispersés en petits groupes à des endroits aléatoires » (D207) avec une
« vigilance : distance minimale entre joueurs ». Le GDD ne chiffre ni la taille des groupes ni cette distance.

## Q1 : combien de Paysans par groupe de départ ?

- **A (recommandé, codé par défaut)** : **2 au plus** (5 Paysans = 2 + 2 + 1, trois groupes). En jeu : trois petits points à aller
  chercher, on regroupe vite, on peut perdre un groupe isolé sans perdre la partie.
- **B** : **1 par groupe** (5 Paysans seuls, comme le Nomade d'AoE2). En jeu : cinq points éparpillés, le début est plus long et plus
  fragile (un éclaireur ennemi en tue un facilement).
- **C** : **un groupe de 3 et un de 2**. En jeu : deux points seulement, départ plus calme, le forum se pose plus vite.
- **Autre** : à préciser.

Réglage : `GroupSize = 2` dans `Script/Nomad/HOLNomadRules.as` (ligne `nomad.group_size` de `Tools/Data/ValueSheet.csv`).

Reponse: B (1 Paysan par groupe, cinq Paysans seuls ; GroupSize à passer à 1) - 2026-11-12

## Q2 : distance minimale entre un groupe de départ et un groupe d'un autre camp ?

- **A (recommandé, codé par défaut)** : **40 cases (4 000 cm)**. En jeu : on ne voit pas l'ennemi au départ, mais on peut le croiser
  dans la première minute en explorant.
- **B** : **25 cases (2 500 cm)**. En jeu : rencontres très rapides, rush possible avant tout forum (défaite D354 plus fréquente).
- **C** : **60 cases (6 000 cm)**. En jeu : départ tranquille, mais sur les petites cartes à 8 joueurs le placement peut ne pas trouver de
  place (il faudra alors réduire la distance).
- **Autre** : à préciser.

Réglage : `MinEnemyDistanceCm = 4000` dans `Script/Nomad/HOLNomadRules.as` (ligne `nomad.min_enemy_distance_cm` de `ValueSheet.csv`).

Reponse: A (40 cases, 4 000 cm ; déjà codé) - 2026-11-12
