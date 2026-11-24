# [À ESSAYER] E-monk_seal_slow : le Moine pose le Sceau (touche de la grille puis clic sur le sol) ; un ennemi qui traverse la zone marche à environ 66 % de sa vitesse, et plus du tout ralenti quand il retraverse après les 10 s du Sceau

STATUT : VU À L'ÉCRAN
GESTE: réel
SON: non entendu
branches: ground-cast-1124s, monk-seal-1124s

Pré-vol (2026-11-24 21:01, agent ELROND-1124si, worktree C:\Users\baudo\Documents\HeroesOfLiberty-Elrond, branche ground-cast-1124s) : `preflight.ps1 monk_seal_slow -Root C:\Users\baudo\Documents\HeroesOfLiberty-Elrond -AllowUncommitted` PASS (38 s, 1 PNG) ; puis `monk_seal_slow_end` PASS (52 s, 1 PNG) et `monk_seal_slow_end_control` PASS (48 s, 1 PNG). Journal : `Seal of Light placeholder: pale blue ring of 600 cm for 10.0 s`, lignes `Units where` une par seconde ; `monk_seal_slow_check.ps1 -Log <journal>` donne :
- monk_seal_slow : pas dedans 423, 428 (moyenne 426) contre 709, 580 dehors (moyenne 644), rapport 0,66, PASS (attendu <= 0,75).
- monk_seal_slow_end (l'ennemi part 11 s après la pose, Sceau fini) : pas dedans 710, dehors 880, 540 (moyenne 710), rapport 1,00, `-Expect free` PASS.
- monk_seal_slow_end_control (l'ennemi part 4 s après la pose, Sceau actif) : rapport 0,66, `-Expect free` FAIL (rouge voulu : le même passage à 5 s est ralenti).
PNG OUVERTS et vus :
- monk_seal_slow_mid : l'anneau bleu pâle (double cercle, rayon 6 m) est posé, le Moine bleu en dessous, un ennemi rouge au bord haut de l'anneau ; case R « Sceau » dans la grille, panneau « recharge 36 s ».
- monk_seal_slow_end_mid : l'anneau a DISPARU (Sceau fini), le Moine seul, panneau « recharge 25 s », l'ennemi n'est plus à l'image (il a fini sa traversée).
- monk_seal_slow_end_control_mid : non rouvert par l'agent (preflight PASS, rapport 0,66 lu au journal).
NON VÉRIFIÉ : le ralentissement vu à l'œil (un humain ne mesure pas 66 %) ; partie réseau (client contre serveur dédié) ; aucun son (-nosound) ; capture hors écran (-RenderOffScreen), pas de fenêtre jouée par un humain ; le geste est le clic scripté (`hol.Select.LeftClickGround`) après `hol.Select.Grid 3`, pas un vrai appui clavier.

Commande complète :
`powershell -ExecutionPolicy Bypass -File C:\Users\baudo\Documents\HeroesOfLiberty-Elrond\Tools\Scenarios\play.ps1 monk_seal_slow -Root C:\Users\baudo\Documents\HeroesOfLiberty-Elrond`
Durée : environ 1 min. Réponse attendue : `OK`, `BUG: ...` ou `REGLAGE: ...`.
