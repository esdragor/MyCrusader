# [À ESSAYER] signals_altclick : marque au sol du signal (D825, lot H1)

VU À L'ÉCRAN (HALDIR-1116a, 2026-11-16 17:06, preflight PASS, 2 PNG ouverts)
GESTE: réel (vraie touche Alt tenue puis vrai clic gauche, hol.UI.Key + hol.Mouse.Click)
SON: non entendu (-nosound ; aucun son prévu pour ce point)
branches: maps

Commande (copiable telle quelle, durée ~1 min) :
powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Haldir\Tools\Scenarios\play.ps1 signals_altclick -Root C:\Users\baudo\Documents\HeroesOfLiberty-Haldir

Ce que tu dois voir :
1. Maintiens Alt et fais un clic gauche bref sur le sol : pendant 5 s, un LOSANGE BARRÉ D'UNE CROIX (blanc = signal neutre ; couleur du type pour un glisser, D821), d'environ 5 m, apparaît AU SOL à l'endroit du clic ; le fil de messages dit « Vous : Signal ».
2. Après 5 s de jeu, la marque au sol disparaît.

Réglage : `hol.Signal.GroundMarkSize` (côté du losange, cm, défaut 500). Forme provisoire (bannière, « ! » ou pictogramme possibles plus tard).

Ce qui N'EST PAS dans cet essai (D825, lots suivants, non faits : arrêt du C++ du 2026-11-16) : marque de la minicarte ANIMÉE (H2), Alt + clic SUR la minicarte (H3).
Note de l'agent : sur les deux PNG, un petit carré blanc reste visible en bas de la minicarte au même endroit (le journal dit pourtant « Minimap signals: 0 » avant la 2e capture) : à regarder, NON VÉRIFIÉ que c'est la marque du signal.
Règle 19 : REGLAGE de l'utilisateur, à rejouer par l'intégrateur sur main fusionné avant de te donner la commande.
