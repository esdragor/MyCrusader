# Case de 4 m et cartes de 220 × 220 cases (D344)

Demande de l'utilisateur du 2026-10-22 : « une case dans le jeu fait 4 m et la plus grande carte possible fait 220 cases
sur 220 ». Session mycrusader-4b, worktree `HeroesOfLiberty-J5c`, branche `tile-4m` (depuis main 8408565).

## Choix de l'utilisateur (conformes aux recommandations)

- **Bâtiments gardés en cases** : forum 4 × 4 = 16 m, caserne 3 × 3 = 12 m, maison et camps 2 × 2 = 8 m.
- **Réseau** : positions sur 13 bits au pas de 11 cm (±450 m) au lieu de 14 bits au pas de 10 cm (D343) : 59 → 57 bits.

## Fait

- `Config/DefaultGame.ini` `TileSize=400` ; défaut de `UHOLBuildingSettings::TileSize` à 400.
- `Tools/Data/BuildingTypes.csv` : emprises 1600 / 800 / 1200 cm ; table refaite (`make_building_types.py`).
- `HOLUnitReplicationRules.h` : `HorizontalBits` 13, `PositionStep` 11 ; avertissement de la réplication à ±450 m ;
  `s5_report.py` `STATE_BITS = 57`.
- `AHOLMapBounds::MaxSideTiles = 220`, `FitsMaxSize`, avertissement au `BeginPlay` ; test dans `Map.StartSlots`
  (carte de match dans la limite, une case de plus refusée).
- Tests : bâtiments (16 m), table (4 m, caserne 12 m), réseau (57 bits, positions de test sur le pas de 11 cm).
  Compilation 0 avertissement, **tests 90/90 PASS**.
- GDD : D344 (journal, § 16.6), D311 et D343 annotés, prochain numéro D345.
- Autres sessions prévenues : mycrusader-c1 (stats des unités en cases : valeurs à 4 m, ne pas mettre à l'échelle
  `UnitTypes.csv` de mon côté ; elle prend D345), mycrusader-e9 (scénarios `blocked_bodies` / `formation_obstacles` :
  maisons collées à 8 m), mycrusader-43, mycrusader-80.

## En cours

- Scénario d'essai écrit dans le worktree : `Tools/Scenarios/tile_4m.txt` (+ `.json`) — deux maisons collées, une
  caserne, un camp de bûcherons, quatre Paysans sélectionnés ; pas encore lancé.

- **Sous-agent, boucle 1** (non commité) : `quick_check.ps1` adapté (`$Tile = 400`, `Get-SlotStart` arrondi comme
  `FMath::RoundToDouble` : départs (-14800, 0) et (15200, 0) ; nouvel outil `Get-NearSpot` ; maisons, camps, caserne,
  parties cut et gap replacées sur la grille de 4 m ; commentaires 4 m / 8 m / 16 m / 12 m). Scénarios `archers`,
  `j7_army`, `order_lines`, `placement`, `tile_4m` recalés. Dernière course `camera,build,net` : tout PASS sauf
  **build cut** (maison N : 40 corps remis dehors pour une limite de 20, 0 dedans, 0 nouveau chemin — le mur de la
  maison est plus long) et **net** (course entre les deux maisons des clients : le Paysan du slot 0 naît du côté de sa
  maison, antérieur à la case de 4 m ; correctif non essayé : Paysan d'abord envoyé à 15 m devant son centre).
- Limite de la partie **build cut** pour la maison N : 20 → **50** corps remis dehors (mur de 5,7 m au lieu de 3,7 ;
  25 à 40 mesurés, toujours 0 dedans et 0 nouveau chemin).
- **`quick_check -Parts tests,camera,gather,build,net` : PASS en 311 s** (tests 90/90, net 2 clients compris).
- **Essai de l'utilisateur** : `play.ps1 tile_4m -Root ..\HeroesOfLiberty-J5c`.
- Retour d'essai 1 : « c'est bon, mais les cases du sol ne correspondent pas (une maison entre deux cases) » : le sol
  tirait une ligne tous les 10 m. `make_matte_materials.py` `GRID_CELL = 400` (une ligne par case, sur les lignes de la
  grille des bâtiments), `M_MatteGround` refait ; doc de `create_match_map.py` à jour. Essai relancé.

- Essai relancé : **validé par l'utilisateur** (« super »). Commits sur `tile-4m` : 90d8dfe (D344), 2dd6a66 (scénarios
  `blocked_bodies` et `formation_obstacles` de mycrusader-e9, arrivés dans main entre-temps : maisons collées à 8 m,
  passage à y -2800). Fusion dans main par avance rapide après accord des sessions de la copie principale.

## Reste

- S5 (positions sur 13 bits) à mesurer à la prochaine série de mesures (doit baisser : 2 bits de moins par état).
- Peut-être : une ligne plus marquée tous les 5 cases (20 m) si le sol de 4 m fait trop chargé vu de loin.
