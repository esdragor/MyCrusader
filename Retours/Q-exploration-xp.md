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

**Précisions du branchement (2026-11-15, SAURON-1115d)** :
- Dans un camp à plusieurs sièges, chaque siège du camp est payé (défaut codé) ; partage entre alliés : à décider.
- Constat : la liste des lieux (relais, camps, autel) est vide : le préréglage B ne paie rien en jeu aujourd'hui.
- Côté C++ le préréglage se change par la console : `hol.HeroXP.ExplorationPreset` (0 = C défaut, 1 = A, 2 = B), lu au départ du brouillard.

**[À RÉPONDRE] Lieux : recouvrement avec les porteurs existants (2026-11-15, SAURON-1115e)** : relais et camps neutres créditent DÉJÀ la source Exploration : le relais paie l'XP de territoire à la minute tant qu'on le tient (`HOLRelaySubsystem`, D388), le camp paie sa récompense quand il est nettoyé (`HOLCampSubsystem`) ; l'autel n'existe que sur la carte Volcan. Payer aussi leur 1re vue (B : 40 XP, C : 25 XP) crédite deux fois la même source pour le même lieu et s'ajoute à la part ~15 % de D05. Lieux NON BRANCHÉS en attendant (liste vide : B ne paie rien, C ne paie que les tranches).

| Option | Ce qui paie à la 1re vue | Conséquences visibles |
|---|---|---|
| **A** Tous les lieux | relais + camps + autel | l'éclaireur est récompensé de trouver un relais ou un camp, puis le relais/camp paie encore à la prise ou au nettoyage ; la part Exploration grossit (~10 lieux x 25-40 XP en plus) |
| **B** Seulement les lieux sans autre paiement | autel (Volcan) et futurs lieux sans porteur | pas de double crédit ; hors Volcan, B ne paie rien et C se réduit aux tranches |
| **C** Aucun lieu | rien | la vue ne paie que la surface (tranches) ; les lieux paient par leur prise ou leur nettoyage, comme aujourd'hui |
| **Autre** | ex. vue d'un lieu payée mais retirée de la récompense de prise | — |

**Recommandation : B** (pas de double crédit, garde la part de D05). **Défaut en place : C** (liste vide, rien de branché, aucun changement de jeu).

Réponse : 
