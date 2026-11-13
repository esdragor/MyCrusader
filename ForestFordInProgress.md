# Forêt et gué (B4, GDD § 8.1, reste de D33) : journal

Chapitre B4 de `C:\Users\baudo\Documents\MyCrusader\Auto_Journey_Jour.md` (« Chapitres du GDD non commencés »). Worktree Haldir,
branche `maps`. Règle D483 : règles pures en Angelscript (`Script\Terrain\HOLTerrainRules.as`) ; le combat (C++) lira le facteur ensuite.
Questions : `C:\Users\baudo\Documents\MyCrusader\Retours\Q-forest-cover.md` (2 au plus, défauts codés).

## Règles tirées du GDD (D33, D496)

1. Couvert en forêt : une cible en forêt reçoit moins de dégâts **à distance** ; la **mêlée ne change pas**.
2. Forêt lente pour la cavalerie et le siège (pas encore codé).
3. Défense réduite au gué (pas encore codé).

Pas dans le GDD (questions, défauts codés derrière `hol.Terrain.*`) :
- Q1 « être en forêt » : A zone peinte (défaut, `hol.Terrain.ForestByTrees 0`), B au moins 3 arbres à 6 m (`1`), C les deux.
- Q2 valeur : A 75 % (défaut, `hol.Terrain.ForestRangedPercent 75`), B 85 %, C 50 %.

## Plan (étapes de 15 min)

- F1 (fait 2026-11-13 soir) : règle pure `HOLTerrain::InForest`, `DamagePercentAt(bRanged)`, `ScaledDamage`, zone `Forest = 2`, arbres
  `FHOLTerrainTree`, `SettingsFromCVars`. Tests `HOLTerrainRules_Test.as` : `Test_TerrainForestZoneCoversFromRanged`,
  `Test_TerrainForestByTreesCovers`, `Test_TerrainForestCVarsDefaults` (effet : dégâts réellement infligés par une flèche de 20).
- F2 (C++, agent C++) : le combat lit le facteur : au coup à distance, `FHOLTerrainRules` (miroir C++ comme le marais) donne le
  pourcentage à la position de la cible ; test C++ ou `net_match` sur l'effet (PV perdus par une cible en forêt < hors forêt).
- F3 : la carte pose des zones `Forest` (styles Forêt de `Q-glade-style.md`) ou passe la liste des arbres (si Q1 = B).
- F4 : forêt lente pour cavalerie et siège (pourcentage par catégorie, même modèle que le marais). Question de valeur si besoin.
- F5 : gué : zone `Ford`, défense réduite (facteur de dégâts reçus > 100 % toute attaque). Question de valeur si besoin.

## Journal

- 2026-11-13 soir (HALDIR-1113c) : Q-forest-cover posée ; test rouge F1 (stubs : `InForest` faux, `DamagePercentAt` 100) commité.

## Appels vus (agent HALDIR-1113c)

|||||||||||||||||||||||
