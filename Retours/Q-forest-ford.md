# [À RÉPONDRE] Forêt lente et gué (F4 / F5, GDD § 8.1, reste de D33)

**Contexte.** D33 : la forêt ralentit la cavalerie et le siège, le gué réduit la défense des unités dans l'eau ; le couvert a été supprimé (D796), donc « être en forêt » n'a plus de définition codée.
La règle pure est écrite (`Script\Terrain\HOLTerrainRules.as`, worktree Galadriel, branche `galadriel-tab`) avec les valeurs A ; rien n'est branché au jeu (le mouvement et le combat C++ liront les facteurs ensuite).

## Q1 — Qu'est-ce qu'« être en forêt » (et « sur un gué ») ?
- **A (recommandé)** : une **zone peinte** par la carte (comme le marais et la route). Visible : la cavalerie ralentit d'un coup en passant le bord de la zone, partout pareil ; une clairière laissée par les bûcherons reste lente tant que la zone n'est pas repeinte.
- **B** : la **proximité d'arbres** (par exemple au moins 3 arbres à 6 m). Visible : un bois abattu par les bûcherons cesse de ralentir ; la limite suit les arbres, plus irrégulière ; plus coûteux en calcul.
- **C** : les deux (zone peinte ET arbres encore debout).
- **Autre** : ...
- Le gué : une zone peinte dans tous les cas (les gués sont posés par la carte).

## Q2 — Valeurs (modérées : « se sentir sans décider seule d'une bataille », § 8.1)
| Option | Cavalerie en forêt | Siège en forêt | Dégâts reçus sur un gué |
|---|---|---|---|
| **A (recommandé, codé)** | 80 % de la vitesse | 70 % | 120 % (défend comme à 83 %) |
| B (plus fort) | 70 % | 60 % | 130 % |
| C (plus léger) | 90 % | 85 % | 110 % |
| Autre | ... | ... | ... |

Conséquences visibles : avec A, une cavalerie qui traverse 10 m de forêt met 12,5 m de temps ; une unité attaquée sur un gué perd 1 PV de plus sur 5 ; l'infanterie n'est pas ralentie en forêt.
Note : sur la carte **Les Gués**, D496 dit « gués sans ralentissement » ; la défense réduite y est-elle gardée ? (défaut codé : oui, la règle ne connaît pas la carte).

**Réglage** : une ligne chacune dans `FHOLTerrainSettings` (`ForestCavalrySpeedPercent`, `ForestSiegeSpeedPercent`, `FordDamageTakenPercent`).
