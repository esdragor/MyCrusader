# [À RÉPONDRE] Q-ai-towers : combien de tours de guet l'IA construit-elle, et à quel niveau ?

Contexte : l'IA ne construisait aucune tour (aucun `TowerWood` dans `Script\AI`), donc son appel de garnison (S4-b) n'avait rien à lire en partie
(`HOLAITower.as`, FARAMIR-1124sg ; la ligne `TowerWood` de `BuildingTypes.csv` a bien 3 places depuis 482e26b).

Options :
- A : Facile 0, Normal 1, Difficile 2 tours (valeur codée). En jeu : l'IA Normale / Difficile pose une tour côté ennemi après 4 soldats ; Facile reste sans tour.
- B : toutes les difficultés 1 tour, Difficile 2. En jeu : même Facile garnit une tour attaquée.
- C : pas de tour (tout à 0). En jeu : la garnison de l'IA reste inerte, comme avant.
- Autre.

Recommandation : A (la quantité seule varie avec le niveau, comme la cavalerie).

Valeur par défaut codée : A, dans `FHOLAITowerSettings` (`EasyMax`, `NormalMax`, `HardMax`, `MinSoldiers`, `BuildTier`) de `Script\AI\HOLAITower.as` : une ligne à changer.

Reponse: B (2026-11-25 13:0x, DOCS-1125me, GDD D918)
