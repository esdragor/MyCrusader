# [À ESSAYER] E-wall_line_long : ligne de mur Palissade, une fondation par case (EndWallDrag, relâché sans sol)

GESTE: interne (touche D non jouée : hol.Build.Place arme le mode ; le glisser est un vrai glisser Slate hol.Mouse.Drag)
SON: non entendu
branches: integ-55
STATUT: LOGIQUE SEULEMENT (effet vu au journal et dans le PNG wall_line_long_after : ligne (-17420,-1100) -> (-17420,1800), 8 fondations, Bois 20200 -> 20160 = 8 x 5)

Voir : après le relâché, une file de fondations de Palissade sur toute la ligne affichée ; Bois baissé de 5 par case.
Limite connue : le fantôme s'arrête à 8 cases (le curseur perd son sol avant la fin du glisser en capture hors écran).

Commande : powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Sauron\Tools\Scenarios\play.ps1 wall_line_long -Root C:\Users\baudo\Documents\HeroesOfLiberty-Sauron