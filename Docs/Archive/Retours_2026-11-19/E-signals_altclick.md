# [À ESSAYER] signals_altclick : marque au sol du signal (D825, lot H1)

VU À L'ÉCRAN (HALDIR-1116a, 2026-11-16 17:06, preflight PASS, 2 PNG ouverts)
GESTE: réel (vraie touche Alt tenue puis vrai clic gauche, hol.UI.Key + hol.Mouse.Click)
SON: non entendu (-nosound ; aucun son prévu pour ce point)
branches: maps

Commande (copiable telle quelle, durée ~1 min) :
powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Arwen\Tools\Scenarios\play.ps1 signals_altclick -Root C:\Users\baudo\Documents\HeroesOfLiberty-Arwen

Pré-vol rejoué le 2026-11-19 21:57 (session principale, worktree Arwen = main 7b9d6ec, binaire à jour, gate Essay sans FAIL à 22:00) : `preflight signals_altclick` PASS (34 s, audit Sanity OK). Les 2 captures ouvertes et vues : `signals_altclick` montre le losange blanc au sol à droite du forum, la ligne « Vous : Signal » du fil et le carré blanc sur la mini-carte ; `signals_altclick_after` : le losange et le carré ont disparu. (Ancienne commande, worktree Haldir : remplacée par celle d'Arwen ci-dessus.)

Ce que tu dois voir :
1. Maintiens Alt et fais un clic gauche bref sur le sol : pendant 5 s, un LOSANGE BARRÉ D'UNE CROIX (blanc = signal neutre ; couleur du type pour un glisser, D821), d'environ 5 m, apparaît AU SOL à l'endroit du clic ; le fil de messages dit « Vous : Signal ».
2. Après 5 s de jeu, la marque au sol disparaît.

Réglage : `hol.Signal.GroundMarkSize` (côté du losange, cm, défaut 500). Forme provisoire (bannière, « ! » ou pictogramme possibles plus tard).

Ce qui N'EST PAS dans cet essai (D825, lots suivants, non faits : arrêt du C++ du 2026-11-16) : marque de la minicarte ANIMÉE (H2), Alt + clic SUR la minicarte (H3).
Note de l'agent : sur les deux PNG, un petit carré blanc reste visible en bas de la minicarte au même endroit (le journal dit pourtant « Minimap signals: 0 » avant la 2e capture) : à regarder, NON VÉRIFIÉ que c'est la marque du signal.
Règle 19 : REGLAGE de l'utilisateur, à rejouer par l'intégrateur sur main fusionné avant de te donner la commande.

Reponse (2026-11-19 ~22:55) : OK mais REGLAGE (D860) : « OK mais REGLAGE: un animation serait bien + clic sur la minimap doit faire la meme chose que clic sur le sol ». Reporte dans SignalsInProgress.md (H2, H3) et Auto_Journey_File.md (ligne D860). Fiche archivee dans Docs\Archive\Retours_2026-11-19\.
