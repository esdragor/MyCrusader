# [À ESSAYER] E-order_lines_blocks : deux blocs d'unités séparés, un clic droit sur le sol : DEUX lignes de mouvement (une par bloc), chacune partant de son bloc

STATUT : VU À L'ÉCRAN (capture « pendant » ouverte et vue ; geste de la commande = ordre interne, voir GESTE)
GESTE: reel (ELROND-1122i : le clic droit sur le sol est hol.Select.RightClickGround, curseur pose sous le point, journal 'Scripted right click at (0.500, 0.513)' puis 'Order: 12 units to (-13000, 0)' ; restent internes la mise en place : spawn, camera, hol.Select.Army pour la selection ; le glisser / Maj + clic droit n'est pas couvert)
SON: sans objet
branches: 96cff65 (main du 2026-11-22 22:05 ; zone-effect : tous ses commits hors merges sont dans main, dont c399c87, 5658626, 904bd5e)
recette: RECETTE-1121sa PASS sur main 9e7a324 (Sauron) : preflight PASS (21 s, WARN HUD overlap / Sanity absents, preexistants), PNG order_lines_blocks_pendant vu : 12 unites en 2 blocs, DEUX lignes jaunes, chacune de son bloc vers le point d ordre. Geste reste interne (hol.Select.OrderGround).
recette: RECETTE-1122b PASS VU A L ECRAN sur integ-50 7053828 : preflight PASS (28 s), 0 FAIL, 2 WARN (HUD overlap / Sanity, preexistants), 0 erreur Angelscript, 0 WorldContext ; PNG pendant ouvert : 12 unites en 2 blocs (6 + 6), DEUX lignes jaunes, chacune de son bloc vers le point d ordre, pas de ligne du milieu. Geste reste interne ; rapport Docs\Rapports\RECETTE-1122b.md
recette: ELROND-1122i 2026-11-22 18:05 (integ-25 0c8d5eb) : OrderGround remplace par hol.Select.RightClickGround -13000 0 + waitlog 'Scripted right click at' (camera 3500 0 posee avant, point a l'ecran) ; preflight PASS 18 s (1 passage), 0 FAIL, 2 WARN preexistants (HUD overlap / Sanity), 0 erreur Angelscript, 0 'requires WorldContext' ; PNG pendant ouvert : 12 unites en 2 blocs, DEUX lignes jaunes, chacune de son bloc vers le point d'ordre central, pas de ligne du milieu ; clic reel a atteint le sol (12 unites ordonnees). Geste reel pour le clic droit.
recette: RECETTE-1122c 2026-11-22 18:28 (integ-52 edc631a) : preflight order_lines_blocks PASS 22 s, 0 FAIL, 2 WARN (HUD overlap / Sanity absents, preexistants), 0 erreur Angelscript, 0 WorldContext ; PNG pendant ouvert : 12 unites en 2 blocs (gauche / droite a l ecran, le note dit nord et sud), DEUX lignes jaunes, chacune de son bloc vers le point au centre ; journal Scripted right click at (0.500, 0.513) puis Order: 12 units to (-13000, 0), rien d autre ; PASS VU A L ECRAN, GESTE reste interne (Select.Army, spawn, camera) ; Docs\Rapports\RECETTE-1122c.md
recette: pré-vol de la session principale 2026-11-22 22:0x sur main 96cff65 (worktree Sauron, code identique à main, binaire du 19:14) : preflight PASS (18 s), PNG pendant ouvert : 12 unités en 2 blocs, DEUX lignes jaunes, chacune de son bloc vers le point d'ordre

Pré-vol (2026-11-21 19:15, agent FARAMIR-1121sa, worktree C:\Users\baudo\Documents\HeroesOfLiberty-Faramir, main 6b226f8) : `preflight.ps1 order_lines_blocks -Root C:\Users\baudo\Documents\HeroesOfLiberty-Faramir` PASS (22 s, 1 PNG).
PNG OUVERTS et vus :
- order_lines_blocks_pendant : 12 unités sélectionnées (Tous 12 = Homme d'arme 6 + Lancier 6) en deux blocs distincts (un à gauche, un à droite de l'écran), DEUX lignes jaunes, chacune partant du centre de son bloc et se rejoignant au point d'ordre (point jaune au sol). Pas une ligne unique partant du milieu des deux blocs.
NON VÉRIFIÉ : un vrai clic droit (geste réel) ; le cas d'un seul bloc (une seule ligne, couvert par HOLOrderLines_Test.as) ; le tracé en file (Maj + clic droit) avec deux blocs ; le comportement ligne par ligne pendant la marche (une seule image, 20 frames après l'ordre).

Commande complète (règle 13, 2026-11-14) : chaque commande est écrite en entier, copiable telle quelle, jamais « même commande avec… ».
Lancer UNE commande : `powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Sauron\Tools\Scenarios\play.ps1 order_lines_blocks -Root C:\Users\baudo\Documents\HeroesOfLiberty-Sauron`
Durée : environ 1 min. Réponse attendue : `OK`, `BUG: ...` ou `REGLAGE: ...`. Geste à refaire : les 12 unités sont choisies, un clic droit sur le sol : deux lignes, une par bloc.

Reponse: OK (2026-11-22 soir, joué par l'utilisateur, dans la conversation)
