# [À ESSAYER] E-signals_altclick : signal neutre par Alt maintenu + clic gauche bref sur le sol (SG3, D784, D445)

STATUT : VU À L'ÉCRAN (preflight PASS, 2 PNG ouverts : le carré du signal est là à 1 s, parti après 5 s de jeu)
GESTE: réel (Alt tenu par hol.UI.Key LeftAlt 0 3, passé par Slate ; vrai clic gauche hol.Mouse.Click 1400 350 sur le SViewport)
SON: non entendu (-nosound)
branches: maps

Pré-vol (2026-11-15 14:26, agent HALDIR-1115j, worktree C:\Users\baudo\Documents\HeroesOfLiberty-Haldir, branche maps) : `preflight.ps1 signals_altclick -Root C:\Users\baudo\Documents\HeroesOfLiberty-Haldir -MaxSeconds 240 -AllowUncommitted` PASS (25 s, 2 PNG).
Journal : « Signal: received kind 0 from slot 0 at (-13767, 1950) », « Minimap signal mark at (131.7, 202.6) px of 240.0 », « Minimap signals: 1 (world 2.32 s) », puis « Minimap signals: 0 (world 7.28 s) » (5 s de jeu).
PNG OUVERTS et vus :
- signals_altclick (1 s après le clic) : minicarte en bas à droite : le carré BLANC de 10 px à droite du point bleu de ton forum, dans le cadre de la caméra = la marque du signal (neutre = blanc, défaut A de Q-signals-ui : une couleur par type, neutre blanc) ; fil « Vous : Signal ».
- signals_altclick_after (prise une fois la marque partie) : le carré blanc a DISPARU ; le reste de la minicarte inchangé.
CAUSE du « non vu » de HALDIR-1115i : aucun défaut du jeu. Le carré blanc ÉTAIT le signal. Le témoin `wait 6` compte l'horloge de l'application (FApp), plus rapide ici que le temps du monde (5 s de jeu = 7,3 s d'horloge) : la 2e capture tombait 1 image avant le départ de la marque. Le scénario attend maintenant l'effet (`waitlog Minimap signals: 0`) avant la 2e capture.
NON VÉRIFIÉ : pourquoi le temps du monde court à ~0,68 de l'horloge en -RenderOffScreen (en jeu normal, 5 s de jeu ≈ 5 s) ; un signal coloré (types 1-5, roue) non joué.

Commande complète (règle 13, 2026-11-14) :
`powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Haldir\Tools\Scenarios\play.ps1 signals_altclick -Root C:\Users\baudo\Documents\HeroesOfLiberty-Haldir`
Durée : environ 40 s, aucune touche. Réponse attendue : `OK`, `BUG: ...` ou `REGLAGE: ...`.
