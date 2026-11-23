# [À ESSAYER] E-wall_line_long : branche integ-55, pas encore dans main : pose en ligne de la palissade au geste réel (Paysan, touches A puis D, glisser de souris), 9 fondations, Bois débité

STATUT : VU À L'ÉCRAN
GESTE: réel
SON: non entendu
branches: integ-55

Règle (L7-6) : touche de la grille (hol.Select.Grid 0 puis 6 = A puis D, comme le clavier) + vrai glisser de souris (hol.Mouse.Drag) = geste réel.

Pré-vol (2026-11-23 15:28, agent SAURON-1123pb, worktree C:\Users\baudo\Documents\HeroesOfLiberty-Sauron, tip 0e7144e) : `preflight.ps1 wall_line_long` PASS (47 s, 2 PNG).
PNG OUVERTS et vus :
- wall_line_long_during : le Paysan est sélectionné (panneau « Paysan », grille de l'âge I ouverte), la ligne se dessine pendant le glisser : 4 boîtes vertes plates alignées (environ 100 px de large, pas d'environ 100 px) à gauche, au sud-ouest du MainCentre, aucune boîte rouge ; le Bois affiche 20200 (rien payé encore). La capture tombe au tiers du glisser, d'où 4 boîtes et pas la ligne entière.
- wall_line_long_after : 9 fondations de palissade (cadres bleu-gris) alignées sur la ligne, plus aucune boîte verte ; le Bois est passé de 20200 à 20155 (9 x 5).
Journal : `Placement: Palisade (cost F 0 W 5 S 0 G 0)`, `wall drag started at (-17711, -1375)`, `Drag end: ... testCursor='off' ... ground=1 (-17711, 1982)`, `Placement: Palisade wall line (-17711, -1375) to (-17711, 1982)`.
NON VÉRIFIÉ : la construction des fondations par les Paysans (non attendue dans l'essai) ; un glisser en diagonale ou croisant un obstacle (une case bloquée refuse toute la ligne : question Retours\Q-ligne-case-bloquee.md) ; le son ; la pose avec la souris matérielle d'un vrai joueur (ici hol.Mouse.Drag injecte la souris, viewport caché 1066x600).

Commande complète : `powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Sauron\Tools\Scenarios\play.ps1 wall_line_long -Root C:\Users\baudo\Documents\HeroesOfLiberty-Sauron`
Durée : environ 1 min. Réponse attendue : `OK`, `BUG: ...` ou `REGLAGE: ...`.