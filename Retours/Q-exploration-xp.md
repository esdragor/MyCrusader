# [À RÉPONDRE] XP d'exploration : qui la porte, et combien (B2, GDD § 9.4, D05, D06)

**Contexte** : la source « Exploration » du héros n'a encore aucun porteur (`HOLHeroXPRules.h`) ; la part Exploration + Événements vaut ~15 % de l'XP d'une partie (D05, ~500 XP sur les 3 300 du niveau 6) et continue pendant la récupération du héros (D06).
La règle pure est codée par **tranche** (carré de cases du brouillard qui paie une seule fois, à sa 1re case explorée) et par **lieu** (paie une seule fois quand sa case est explorée) ; il reste à choisir le porteur et la valeur. Ce qui est exploré au départ (vue initiale, mode « Carte explorée ») ne paie rien.

| Option | Porteur et valeur | Conséquences visibles |
|---|---|---|
| **A** Surface seule | tranche de 16 × 16 cases (64 m), 6 XP | tout éclaireur rapporte, en continu ; une carte plus grande rapporte plus (Petite 49 tranches, ~300 XP pour toute la carte ; la plus grande, 880 m, 196 tranches, ~1 200 XP) ; aucun lieu n'est mis en avant |
| **B** Lieux seuls | relais, camps neutres, autel : 40 XP chacun à leur 1re vue | l'éclairage vise des points précis, total fixé par la carte (~10 lieux = ~400 XP) ; une zone vide ne rapporte rien ; l'XP arrive par à-coups lisibles |
| **C** Les deux | tranche 3 XP + lieu 25 XP | le GDD cite les deux (« zones, lieux importants ») ; l'éclaireur rapporte un peu partout, beaucoup sur les lieux ; Petite ~150 + ~250 XP |
| **Autre** | ex. plafond d'XP d'exploration par partie, tranche plus grande sur les grandes cartes, autre valeur | — |

**Recommandation : C** (fidèle au § 9.4, l'XP de surface faible garde les grandes cartes dans la part de ~15 %, les lieux donnent un but à l'éclaireur).

**Défaut codé : C**, `HOLExplorationXP::Preset(0)` dans `Script\Heroes\HOLExplorationXPRules.as` (A = `Preset(1)`, B = `Preset(2)`, valeurs dans la structure `FHOLExplorationXPSettings`, une ligne à changer). Rien n'est branché au jeu (lecture du brouillard en C++ à faire) : réponse sans effet visible avant ce branchement.

Réponse : 
