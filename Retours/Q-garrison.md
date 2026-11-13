# [À RÉPONDRE] Q-garrison : la garnison des tours (F7, FortificationsInProgress.md) : combien de places, occupants à l'abri ou non, qui entre

Contexte : F7 ajoute la garnison (D693) : tes unités entrent dans TA tour (jamais celle d'un allié, D374, ni d'un ennemi), et la Tour de guet (F8) tirera plus avec ses occupants. Seule la règle pure est écrite (`Script/Garrison/HOLGarrisonRules.as`, branche `tiers`, sans effet en partie tant que l'ordre « garnir » n'existe pas) ; les défauts ci-dessous sont des réglages, une ligne à changer.

Réponds par exemple « Q1 A, Q2 A ».

## Q1. Places par tour et protection des occupants

- **A (recommandé)** : Tour de guet **3**, Tour de pierre **5**, Tour renforcée **8** ; les occupants sont **à l'abri** (ni ciblés ni touchés) tant que la tour tient, et sont éjectés vivants à sa destruction. En jeu : garnir = mettre quelques tireurs à l'abri ; l'ennemi doit abattre la tour (siège, mêlée sur le bois) avant de les atteindre ; une petite tour est vite pleine.
- **B** : mêmes abris, places **5 / 10 / 15**. En jeu : une tour cache une petite armée ; avec F8 (flèches par occupant) une tour pleine tire très fort : les tours deviennent le cœur de la défense.
- **C** : places **3 / 5 / 8**, occupants **non protégés** : chaque coup reçu par la tour blesse aussi un occupant. En jeu : garnir expose ; il faut sortir les blessés à temps ; la tour protège moins qu'elle n'aide à tirer.
- **Autre** : tes chiffres ou ta règle.

Défaut codé (A) : `hol.Garrison.SlotsWatchTower 3`, `hol.Garrison.SlotsStoneTower 5`, `hol.Garrison.SlotsReinforcedTower 8`, `hol.Garrison.OccupantsProtected 1` (0 = option C). À terme les places iront dans une colonne de `BuildingTypes.csv`.

## Q2. Quelles unités entrent

Les engins de siège n'entrent jamais (dans les trois options). Les Paysans (D351) et le Roi (D365) seront traités au chantier victoire, pas ici.

- **A (recommandé)** : l'**infanterie** (mêlée et tireurs à pied) et le **héros** entrent ; la **cavalerie** non. En jeu : le héros blessé peut se réfugier dans une tour (il échappe au coup de grâce tant qu'elle tient) ; la cavalerie reste une force de campagne.
- **B** : tout sauf le siège, **cavalerie comprise**. En jeu : plus simple à retenir ; une tour peut cacher des cavaliers pour une sortie surprise.
- **C** : seulement les **tireurs à pied** (archers, arbalétriers, arquebusiers) ; héros et mêlée restent dehors. En jeu : la garnison ne sert qu'au tir de la tour (F8) ; pas de refuge pour le héros.
- **Autre**.

Défaut codé (A) : `hol.Garrison.MeleeInfantry 1`, `hol.Garrison.Hero 1`, `hol.Garrison.Cavalry 0` (B : Cavalry 1 ; C : MeleeInfantry 0 et Hero 0).

Reponse:
