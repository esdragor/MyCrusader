# Vitesse des tests (quick_check en pas fixe)

Demande du 2026-10-26 : « comment on pourrait optimiser les tests ? », puis « go faire 1 ».

## Constat (2026-10-26)

- Défaut de `quick_check` : ~31 s, dont ~17 s de démarrage du moteur ; rien à gagner.
- `-Full` : ~8 min de jeu, dont **build ~4 min 30** (cut 44 s, camps ~60 s, barracks 24 s…), combat ~1 min 45, orders
  ~1 min 20, move ~1 min 10 : c'est le temps du jeu qui passe (marches, chantiers, combats), pas le calcul.
- 226 `wait N` fixes dans `quick_check.ps1` (~450 s écrits).
- Hors écran, le jeu tourne vers 300 images/s mais le temps du jeu suit l'horloge.
- La simulation est déjà à pas fixe (20 Hz, `MaxStepsPerFrame=4`) : elle fait les mêmes pas quelle que soit la cadence.

## Option retenue : piste 1, le temps du jeu accéléré

- La partie solo est lancée avec `-UseFixedTimeStep -FPS=<GameFps>` : chaque image avance le jeu de 1/GameFps s, sans
  attendre l'horloge (moteur : `UEngine::UpdateTimeAndHandleMaxTickRate`).
- `wait N` du lanceur de séquences (`HOLTestSequence.cpp`) compte le temps du jeu (`FApp::GetCurrentTime`), les délais
  d'abandon (60 s) restent en temps réel.
- La ligne « Test sequence: done in X s (game time Y s, xZ, fixed step …) » donne le gain ; `quick_check` l'affiche.
- `-RealTime` : l'ancien comportement. La partie réseau (serveur et clients) reste en temps réel.

## Suivi

- Worktree `HeroesOfLiberty-Elrond`, branche `test-speed` (depuis `main` efb046c).
- Code écrit : `HOLTestSequence.cpp` (wait en temps du jeu, ligne de vitesse), `quick_check.ps1` (`-GameFps`, défaut 30,
  `-RealTime`, ligne « game speed »).
- Mesures de `-Parts build` (Elrond, 2026-10-26) :

| Cadence | Durée totale | Séquence réelle / temps du jeu | Résultat |
|---|---|---|---|
| temps réel (avant) | 306 s | — | PASS |
| `-GameFps 30` | 88 s | 61,6 s / 307,7 s (×5,0) | PASS |
| `-GameFps 20` | **72 s** | 44,3 s / 328 s (×7,4) | PASS |
| `-GameFps 10` | 58 s | 36,2 s / 398,6 s (×11) | FAIL : build cut, maison K (0 marcheur relancé, le recalcul de la navigation arrive trop tard en images) |

- Retenu : **20 par défaut** = un pas de simulation par image, comme en jeu réel (jamais moins d'images que de pas).
- Premier `-Full` à 20 : 242 s (au lieu de ~9-11 min), 2 échecs (victory forum, orders). Puis `-Parts victory,combat` :
  échecs en cascade (délai dépassé à la file d'ordres : 2 Hommes d'armes sélectionnés au lieu de 3). En temps réel,
  la même combinaison échoue aussi (« no hit while walking away », Lancier introuvable) : fragilités déjà là.
- Causes et corrections :
  1. **Chaque étape de la séquence coûtait 1/20 s de jeu** (une image par étape ; ~3 ms en temps réel) : les unités
     bougeaient entre deux commandes. Correction : en pas fixe, l'image ne dure que 5 ms de jeu (`CommandFrameSeconds`)
     sauf pendant une attente non encore satisfaite (`wait`, `waitlog`, `waitunits`, `waitidle`, `waitmap`,
     `waitplayers`), où elle reprend 1/GameFps.
  2. **Le HUD rafraîchissait sa table, ses groupes et ses messages au temps de Slate** (l'horloge) : en pas fixe, 3 s
     de jeu passaient sans rafraîchissement (« hold 0 s »). Correction : `HOLGameScreen::NativeTick` compte avec
     `UWorld::DeltaRealTimeSeconds` (identique en jeu normal, suit le pas fixe en test).
  3. Contrôle « orders » : le Lancier reconnu comme la cible des Paysans, plus par sa distance au point d'apparition.
  4. Contrôle « victory forum released » : `wait 0.6` au lieu de 0.4 (à 2 s par seconde, la seconde entière affichée
     baisse toujours).
- `-Parts victory,combat` après corrections : **PASS en 69 s** (temps réel : 300 s, 2 échecs).
- **`-Full` avec les corrections : tout PASS en 310 s** (séquence 262 s réelles pour 929 s de jeu, ×3,5 ; réseau et
  186/186 tests compris). Avant : ~9-11 min.
- `-Parts build` : PASS en 86 s (séquence 53,8 s, 6 808 images, ~8 ms par image) contre 306 s avant.
- Défaut : PASS en 62 s, mais machine chargée (deux autres éditeurs Unreal ouverts : démarrage des tests unitaires
  35 s au lieu de 18) ; la partie courte ne gagne rien (×0,9, 7,5 s de jeu), le changement n'y touche pas.
- Ce qui coûte encore : le rendu hors écran de chaque image (~8 ms). Piste suivante possible : résolution plus basse
  hors captures, ou `-nullrhi` pour les parties sans capture.
- `-GameFps 10` non retenu : deux pas de simulation par image changent le comportement (build cut).
- La ligne « game speed » donne aussi le nombre d'images.

## État

Validé par l'utilisateur le 2026-10-26 (« go ») et fusionné dans `main` ; Elrond libéré.

## Suite : rendu plus léger (demandée le 2026-10-26 soir, « fait optimisation facultative »)

- Elrond, branche `render-light` (depuis `main` 6ff5ae6).
- Mesures de `-Parts build` (séquence seule) : rendu complet ~51-54 s ; `ShowFlag.Rendering 0` partout 40,1 s ;
  `r.ScreenPercentage 25` 51,3 s (rien). Le rendu 3D pèse ~2 ms sur ~8 ms par image ; le reste est la logique du jeu
  et l'interface. Sauter le dessin de Slate saute aussi le Tick des widgets (logique du HUD) : écarté.
- Fait : `hol.Test.SkipWaitRendering 1` (`HOLTestSequence.cpp`) : en pas fixe, le monde n'est pas dessiné pendant
  une attente non satisfaite (`GameViewport->bDisableWorldRendering`) ; dessiné pour les commandes, `frames`, `waitui`
  et les captures ; une capture attend 4 images dessinées (historique de l'anticrénelage temporel). `quick_check`
  l'active en pas fixe hors écran (pas avec `-Visible` ni `-RealTime`) ; `-GameExec` ajoute des commandes au lancement.
- Captures : 84 comparées pixel à pixel avec rendu complet : identiques (< 1 d'écart moyen), sauf `cam_8p_*` où la
  course avec rendu complet avait encore des shaders en préparation.
- Mesures alternées de `-Parts build` (séquence) : sans dessin pendant les attentes 40,1 / 47,4 / 49,0 / 45,9 s
  (~46 s) ; rendu complet 51,3 / 50,7 / 53,8 / 62,9 s (~55 s) : ~15-20 % de gain, machine bruitée (autres sessions).
- **`-Full` : tout PASS en 258 s** (310 s avant), séquence 193,5 s au lieu de 262 s (25 218 images) ; captures de
  combat vérifiées à l'œil (flèches en vol, corps au sol).
- Validé par l'utilisateur (« oui, vas y ») et fusionné dans `main` ; Elrond libéré.

## Reste possible (non demandé)

- Le reste d'une image (~6 ms) est la logique du jeu et de l'interface : plus rien de gros à gagner côté rendu.
- Les `wait N` fixes du script (226) restent : en pas fixe ils ne coûtent plus que leur temps de jeu, divisé par ~5.
