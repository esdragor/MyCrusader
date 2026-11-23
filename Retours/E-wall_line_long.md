# [À ESSAYER] E-wall_line_long (BRANCHE integ-55 NON INTEGREE : EndWallDrag 68c86d0 absent de main, LOGIQUE SEULEMENT) : ligne de mur Palissade, une fondation par case (EndWallDrag, relâché sans sol)

GESTE: interne (touche D non jouée : hol.Build.Place arme le mode ; le glisser est un vrai glisser Slate hol.Mouse.Drag)
SON: non entendu
branches: integ-55
STATUT: LOGIQUE SEULEMENT (effet vu au journal et dans le PNG wall_line_long_after : ligne (-17420,-1100) -> (-17420,1800), 8 fondations, Bois 20200 -> 20160 = 8 x 5)

Voir : après le relâché, une file de fondations de Palissade sur toute la ligne affichée ; Bois baissé de 5 par case.
Limite connue : le fantôme s'arrête à 8 cases (le curseur perd son sol avant la fin du glisser en capture hors écran).

Commande : powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Sauron\Tools\Scenarios\play.ps1 wall_line_long -Root C:\Users\baudo\Documents\HeroesOfLiberty-Sauron
NETTOYAGE-1123g 2026-11-23 : RECETTE-1123a (3) avait vu FAIL (une seule fondation) ; la fiche a ete refaite apres (8 fondations) mais sa commande pointe Sauron = integ-55 ; manque pour devenir pret : fusion de 68c86d0 (non compile : NOT COMPILED dans le message du commit) puis pre-vol sur main, et le geste reel D + glisser (aujourd hui hol.Build.Place interne).
