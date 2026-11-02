# PerformanceInProgress — revue de performance du code (ouverte le 2026-11-01)

Copie principale (`main`), rien de commité : l'utilisateur valide d'abord (règle « committer après validation »).
Reprise = fin de « Suivi ».

## Constat de la revue (lecture, aucune mesure)

Hauts : **P1** `TActorIterator<AHOLBuilding>` dans des boucles par pas et par frame (le coût suit le nombre d'acteurs du
monde) ; **P2** terrain de la minimap (480 x 480 texels, flou du brouillard, envoi à la carte) refait deux fois par seconde
même sans changement.
Moyens, pas faits : **P3** `Selected.Contains(View.UnitId)` en O(vues x sélection) dans `HOLSelectionMarkers.cpp` (474, 565,
773) et `HOLPlayerController.cpp` (2289, 2750, 4822) ; **P4** `ColorOf` de `GatherEntities` parcourt `PlayerArray` pour chaque
unité ; **P5** tableaux de `UpdateRings` / `UpdateHealthBars` alloués à chaque frame.

## P1 — registre de bâtiments

- `FHOLBuildingIterator` (fin de `HOLBuilding.h`) : même usage que `TActorIterator<AHOLBuilding>`. Registre statique
  `AHOLBuilding::Registry` (bâtiment en `TWeakObjectPtr` + son monde), rempli par `PostRegisterAllComponents`, vidé par
  `Destroyed` et `EndPlay`. Un bâtiment qui part pendant une boucle laisse sa case vide (compteur `RegistryWalkers`), la
  boucle ne saute personne ; les cases mortes sont retirées à la prochaine inscription sans boucle en cours.
- **Ordre** : l'ordre d'apparition, celui que `TActorIterator` donnait (le tableau des acteurs du niveau). Pas de tri par
  `GetUniqueID()` : ces identifiants dépendent de l'ordre des objets de la machine, ils ne seraient pas plus communs.
- Sites passés à l'itérateur : tous ceux hors `Tests/` (25 fichiers : combat, gather, build, séparation, production, vision,
  HUD, minimap, marqueurs, contrôleur, IA, état de partie...). Les tests gardent `TActorIterator` : l'autre voie, indépendante.
- Test : `HeroesOfLiberty.Buildings.Registry` (`Tests/HOLBuildingRegistryTests.cpp`) : mêmes bâtiments et même ordre que
  `TActorIterator`, deux mondes, un destroy, une apparition, une boucle qui détruit ce qu'elle parcourt. **Rouge d'abord** : le
  type n'existait pas.

## P2 — cache du terrain de la minimap

- `UHOLMinimapView::RefreshTerrain` ne refait la couche que si la clé change : `UHOLFogRenderSubsystem::GetVersion()`,
  brouillard actif ou non, les deux coins du cadre, `HOLMinimapTerrain::GetSourcesSignature` (sources debout et leur place ;
  une source seulement entamée ne change rien). `Describe()` (ligne `HUD minimap:` de `hol.HUD.Report`) dit combien de fois
  la couche a été faite (`made N times`).
- Test : `HeroesOfLiberty.UI.Minimap.TerrainSources` (`HeroesOfLibertyUI/Tests/HOLMinimapTerrainTests.cpp`), rouge d'abord
  (la fonction n'existait pas).

## Suivi

- 2026-11-01 : P1 et P2 écrits (tests rouges puis code). Build PASS (95 s). `quick_check -Parts tests` : 795/795, dont
  `Buildings.Registry` et `UI.Minimap.TerrainSources` (Success). `-Parts production,gather,build,combat,sources,victory` :
  171 s, aucun FAIL (build minimap et build minalert compris). Compteur de la minimap dans `quick_game.log` : « made N times »
  de 1 à 18 sur toute la séance, au lieu d'une fois par demi-seconde.
- **Pas fait** : mesures S1/S2 avant/après (machine au repos demandée) : le gain n'est donc pas chiffré. Rien commité ; P3-P5 en réserve.
