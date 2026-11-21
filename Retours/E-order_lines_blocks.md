# [À ESSAYER] E-order_lines_blocks : deux blocs d'unités séparés, un clic droit sur le sol : DEUX lignes de mouvement (une par bloc), chacune partant de son bloc

STATUT : VU À L'ÉCRAN (capture « pendant » ouverte et vue ; geste de la commande = ordre interne, voir GESTE)
GESTE: interne (commande `hol.Select.OrderGround`, pas un vrai clic droit : à refaire à la main pendant l'essai, Maj + clic droit pour la file)
SON: sans objet
branches: zone-effect (scénario c399c87 ; le correctif 5658626 + 904bd5e est déjà dans main 6b226f8, zone-effect = main + le scénario)
recette: RECETTE-1121sa PASS sur main 9e7a324 (Sauron) : preflight PASS (21 s, WARN HUD overlap / Sanity absents, preexistants), PNG order_lines_blocks_pendant vu : 12 unites en 2 blocs, DEUX lignes jaunes, chacune de son bloc vers le point d ordre. Geste reste interne (hol.Select.OrderGround).

Pré-vol (2026-11-21 19:15, agent FARAMIR-1121sa, worktree C:\Users\baudo\Documents\HeroesOfLiberty-Faramir, main 6b226f8) : `preflight.ps1 order_lines_blocks -Root C:\Users\baudo\Documents\HeroesOfLiberty-Faramir` PASS (22 s, 1 PNG).
PNG OUVERTS et vus :
- order_lines_blocks_pendant : 12 unités sélectionnées (Tous 12 = Homme d'arme 6 + Lancier 6) en deux blocs distincts (un à gauche, un à droite de l'écran), DEUX lignes jaunes, chacune partant du centre de son bloc et se rejoignant au point d'ordre (point jaune au sol). Pas une ligne unique partant du milieu des deux blocs.
NON VÉRIFIÉ : un vrai clic droit (geste réel) ; le cas d'un seul bloc (une seule ligne, couvert par HOLOrderLines_Test.as) ; le tracé en file (Maj + clic droit) avec deux blocs ; le comportement ligne par ligne pendant la marche (une seule image, 20 frames après l'ordre).

Commande complète (règle 13, 2026-11-14) : chaque commande est écrite en entier, copiable telle quelle, jamais « même commande avec… ».
Lancer UNE commande : `powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Sauron\Tools\Scenarios\play.ps1 order_lines_blocks -Root C:\Users\baudo\Documents\HeroesOfLiberty-Sauron`
Durée : environ 1 min. Réponse attendue : `OK`, `BUG: ...` ou `REGLAGE: ...`. Geste à refaire : les 12 unités sont choisies, un clic droit sur le sol : deux lignes, une par bloc.
