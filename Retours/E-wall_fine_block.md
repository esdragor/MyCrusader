# [À ESSAYER] E-wall_fine_block : une ligne de palissade en pièces de 1 m bloque un Paysan ; sur une ligne avec UN trou de 1 m, la mesure dit si le Paysan passe (résultat : il passe)

STATUT : LOGIQUE SEULEMENT
GESTE: interne      (ligne posée par `hol.Wall.PlaceLine` : appel de ServerPlaceWallLine, pas de glisser à la souris)
SON: sans objet
branches: walls-fine-1124s

Règle (L7-6) : un essai de GESTE ou de SON reste LOGIQUE SEULEMENT tant que la ligne dit `interne` ou `non entendu`.

Pré-vol (2026-11-24 20:34, agent CELEBORN-1124sh, worktree C:\Users\baudo\Documents\HeroesOfLiberty-Celeborn, branche walls-fine-1124s tip 621573a) : `preflight.ps1 wall_fine_block` PASS (47 s, 4 PNG).
Mesure (journal `WALLFINE gap crossed=1 unit #16 at (-18395, 249) line x -19450`) : sur la ligne B (pièces de 1 m, un trou d'une pièce à y -100..0), le Paysan parti de l'ouest (x -19800) est de l'autre côté de x -19450 après 5 s, au repos, sans détour par le bout de 15 m : le trou d'une sous-case de 1 m est FRANCHISSABLE. Ligne A (sans trou) : le Paysan de l'IA reste à (-17806, -3), il ne traverse pas.
PNG OUVERTS et vus :
- wall_fine_gap_after : une longue file de petites fondations de palissade (cadres), alignée ; le panneau du scénario cache le haut de l'écran, le trou de 1 m et le Paysan mesuré ne se distinguent pas à l'image (la mesure vient du journal, pas de l'image).
NON VÉRIFIÉ : 3 PNG sur 4 non ouverts (wall_fine_block_line, wall_fine_block_after, wall_fine_gap_line) ; geste réel du glisser non rejoué (hol.Wall.PlaceLine à la place) ; un Paysan passe un trou de 1 m, rien n'est dit d'une unité plus large (Hôte, engin) ; le design du trou (ligne B) n'est pas tranché ici (D898, D906).

Commande complète (règle 13) :
`powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Celeborn\Tools\Scenarios\play.ps1 wall_fine_block -Root C:\Users\baudo\Documents\HeroesOfLiberty-Celeborn`
Durée : environ 1 min. Réponse attendue : `OK`, `BUG: ...` ou `REGLAGE: ...`.
